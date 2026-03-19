#include "UI/GCodeEditor.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/MainLayer.h"
#include "Controls/GCodeController.h"
#include "Controls/ScadaScheduler.h"
#include "Graphics/Primitives.h"
#include "Graphics/Sketch.h"
#include "Controls/ScadaScheduler.h"
#include "Controls/GlobalPLCVars.h"
#include <QApplication>
#include <Graphics/Anchor.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonParseError>
#include <QDragEnterEvent>
#include <qscrollbar.h>
#include <QDropEvent>
#include <QClipboard>
#include <QMimeData>
#include <QFile>
#include <QFileInfo>
#include <chrono>


QStringList getIntellisense(const QString& intellisenseFile);
bool font_inited = false;
QFont font_global;

GCodeEditor* GCodeEditor::s_instance = nullptr;
QsciLexerCPP* GCodeEditor::s_cpplexer = nullptr;
QsciAPIs* GCodeEditor::s_api = nullptr;

using namespace CNCSYS;

GCodeEditor::GCodeEditor(QWidget* parent)
{
	GCodeEditor::initFont(global_font_mp["Cascadia"], 10);
	GCodeEditor::initIntellisense();
	this->setUtf8(true);
	this->setFont(font_global);
	this->setBraceMatching(QsciScintilla::SloppyBraceMatch);

	this->setMarginWidth(0, 15);
	this->setTabWidth(4);
	this->setIndentationsUseTabs(false);
	this->setAutoIndent(true);
	//autocomplete
	this->setAutoCompletionSource(QsciScintilla::AcsAll);
	this->setAutoCompletionThreshold(1);
	this->setAutoCompletionCaseSensitivity(false);
	this->setAutoCompletionUseSingle(QsciScintilla::AcusNever);

	//caret settings
	this->setCaretForegroundColor(QColor("#dcdcdc"));
	this->setCaretLineVisible(true);
	this->setCaretWidth(2);
	this->setCaretLineBackgroundColor(QColor(242, 161, 141, 55));
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// 设置标记背景色
	this->markerDefine(QsciScintilla::Background, HIGHLIGHT_MARKER_ID);
	this->setMarkerBackgroundColor(QColor(255, 200, 200), HIGHLIGHT_MARKER_ID);

	GCodeEditor::s_cpplexer->setColor(QColor(0, 0, 128), QsciLexerCPP::Keyword);
	// 关键字2颜色 (深绿色)
	GCodeEditor::s_cpplexer->setColor(QColor(0, 128, 0), QsciLexerCPP::KeywordSet2);
	// 字符串颜色 (棕色)
	GCodeEditor::s_cpplexer->setColor(QColor(163, 21, 21), QsciLexerCPP::UserLiteral);
	// 注释颜色 (绿色)
	GCodeEditor::s_cpplexer->setColor(QColor(0, 128, 0), QsciLexerCPP::Comment);
	GCodeEditor::s_cpplexer->setColor(QColor(0, 128, 0), QsciLexerCPP::CommentLine);
	// 数字颜色 (紫色)
	GCodeEditor::s_cpplexer->setColor(QColor(128, 0, 128), QsciLexerCPP::Number);
	// 预处理指令颜色 (深蓝色)
	GCodeEditor::s_cpplexer->setColor(QColor(128, 0, 0), QsciLexerCPP::TaskMarker);
	// 操作符颜色 (黑色)
	GCodeEditor::s_cpplexer->setColor(QColor(0, 0, 0), QsciLexerCPP::Operator);

	this->setEolMode(QsciScintilla::EolWindows);
	this->setEolVisibility(false);

	this->setLexer(GCodeEditor::s_cpplexer);

	this->setMarginType(0, QsciScintilla::NumberMargin);
	this->setMarginWidth(0, "0000");
	this->setMarginsForegroundColor(QColor("#F8A959"));
	this->setMarginsBackgroundColor(QColor("#80838A"));
	QFont FiraSans(FONTPATH(FiraSans - Book.otf));
	this->setMarginsFont(FiraSans);

	m_idleTimer = new QTimer();
	m_idleTimer->setSingleShot(true);
	m_idleTimer->setInterval(1000);

	connect(this, &GCodeEditor::textChanged, [this]()
		{
			m_idleTimer->start();
		});
	connect(m_idleTimer, &QTimer::timeout, this, &GCodeEditor::onEditingFinished);

	int lineFrom, indexFrom, lineTo, indexTo;
	this->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

	connect(this, &GCodeEditor::linesChanged, this, &GCodeEditor::updateLineNumberMargin);
	connect(this, &GCodeEditor::cursorPositionChanged, this, &GCodeEditor::onCursorPositionChanged);
	connect(this, &GCodeEditor::selectionChanged, this, &GCodeEditor::onSelectionChanged);
}

GCodeEditor::~GCodeEditor()
{
	delete m_idleTimer;
}

void GCodeEditor::onCursorPositionChanged(int line, int index)
{
	int lineFrom, indexFrom, lineTo, indexTo;
	this->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
	if ((lineFrom == -1 && lineTo == -1) || line == this->lines() - 1)
	{
		if (lastSelection.first >= 0 && lastSelection.second >= 0)
		{
			for (int i = lastSelection.first; i < lastSelection.second; i++)
			{
				GCodeRecord& curRec = GCodeController::GetController()->records[i];
				if (curRec.attachedEntity != NULL)
				{
					curRec.attachedEntity->SetHighLight(false);
				}
				if (curRec.attachedPath != NULL)
				{
					curRec.attachedPath->visiable = false;
				}
			}
		}

		if (lastHoverEntity)
		{
			lastHoverEntity->SetHighLight(false);
			lastHoverEntity = nullptr;
		}
		if (lastHoverPath)
		{
			lastHoverPath->visiable = false;
			lastHoverPath = nullptr;
		}

		GCodeController* controller = GCodeController::GetController();

		if (controller->records.size() && line < controller->records.size())
		{
			GCodeRecord& curRec = GCodeController::GetController()->records[line];
			if (curRec.attachedPath)
			{
				Path2D* path = curRec.attachedPath;
				path->visiable = true;
				controller->lastHighlightPath = path;
				lastHoverPath = path;
			}
			else
			{
				while (line >= 0)
				{
					GCodeRecord& lastRec = GCodeController::GetController()->records[line];
					if (lastRec.attachedEntity)
					{
						EntityVGPU* entlast = static_cast<EntityVGPU*>(lastRec.attachedEntity);
						EntityVGPU* entCur = static_cast<EntityVGPU*>(curRec.attachedEntity);
						if (entlast != nullptr && entCur != nullptr)
						{
							entCur->highlightSection = { lastRec.sampleIndex,curRec.sampleIndex };
							entCur->SetHighLight(true);
							controller->lastHighlightEntity = entCur;
							lastHoverEntity = entCur;
						}
						break;
					}
					line--;
				}
			}
		}
	}
}

void GCodeEditor::onSelectionChanged()
{
	int lineFrom, indexFrom, lineTo, indexTo;
	this->getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);
	GCodeController* controller = GCodeController::GetController();


	if (lineFrom > -1 && lineTo > -1)
	{
		int size = GCodeController::GetController()->records.size();
		for (int line = lineFrom; line < lineTo; line++)
		{

			if (controller->records.size() && line < controller->records.size())
			{
				GCodeRecord& curRec = GCodeController::GetController()->records[line];
				if (curRec.attachedPath != NULL)
				{
					curRec.attachedPath->visiable = true;
				}

				auto temp = line - 1;
				while (temp >= 0)
				{
					GCodeRecord& lastRec = GCodeController::GetController()->records[temp];
					if (lastRec.attachedEntity)
					{
						EntityVGPU* entlast = lastRec.attachedEntity;
						EntityVGPU* entCur = curRec.attachedEntity;
						if (entlast != nullptr && entCur != nullptr)
						{
							entCur->highlightSection.first = qMax(0, qMin(entCur->highlightSection.first, curRec.sampleIndex));
							entCur->highlightSection.second = std::max(entCur->highlightSection.second, curRec.sampleIndex);
							entCur->SetHighLight(true);
							controller->lastHighlightEntity = entCur;
							break;
						}
					}
					temp--;
				}
			}
		}
		lastSelection = { lineFrom, qMin(lineTo,(int)GCodeController::GetController()->records.size()) };
	}
	else
	{
		for (int i = lastSelection.first; i < lastSelection.second; i++)
		{
			GCodeRecord& curRec = GCodeController::GetController()->records[i];
			if (curRec.attachedEntity != NULL)
			{
				curRec.attachedEntity->SetHighLight(false);
			}
			if (curRec.attachedPath != NULL)
			{
				curRec.attachedPath->visiable = false;
			}
		}
	}
}

void GCodeEditor::onEditingFinished()
{
}

void GCodeEditor::handleGCodePaste()
{

	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();
	//  if (mimeData->hasText()) {
	//      QString text = mimeData->text();
		  //std::vector<EntityVGPU*> outEntities;
		  //GCodeController::GetController()->ParseGCodeToEntities(text, outEntities);
	//  }
}


void GCodeEditor::SetSketch(CNCSYS::SketchGPU* _sketch)
{
	holdSketch = _sketch;
	GCodeController::GetController()->SetSketch(_sketch);
}

void GCodeEditor::onFTPDone()
{
	PLC_TYPE_BOOL uploadDone = true;
	WritePLC_OPCUA(g_ConfigableKeys["PCFileFTPDone"].c_str(), &uploadDone, AtomicVarType::BOOL);
}

void GCodeEditor::CleanCache()
{
	lastSelection = { -1,-1 };
	lastHoverEntity = nullptr;
	lastHoverPath = nullptr;
}

void GCodeEditor::keyPressEvent(QKeyEvent* e)
{
	CleanCache();
	if (e->matches(QKeySequence::Paste))
	{
		handleGCodePaste();
	}

	QsciScintilla::keyPressEvent(e);
}

void GCodeEditor::SetMarkLine(int line)
{
	this->setCursorPosition(line, 0);
	this->markerDeleteAll(HIGHLIGHT_MARKER_ID);
	this->markerAdd(line, HIGHLIGHT_MARKER_ID);
	this->ensureLineVisible(line);
}

//void GCodeEditor::onScintillaModified(int pos, int mtype, const char* text, int len,
//    int added, int line, int foldNow, int foldPrev,
//    int token, int annotationLinesAdded)
//{
//	std::cout << "Modified at line: " << line << std::endl;
//}

void GCodeEditor::initFont(const QString& fontFamily, int fontSize)
{
	font_global.setFamily(fontFamily);
	font_global.setPointSize(fontSize);
}

void GCodeEditor::initIntellisense()
{
	s_cpplexer = new QsciLexerCPP;
	s_cpplexer->setDefaultFont(font_global);
	s_api = new QsciAPIs(s_cpplexer);
	QStringList buildin_keys = getIntellisense(EDITOR_LANG_CONFIG_PATH(gcode - intellisense.json));
	for (const QString& key : buildin_keys)
	{
		s_api->add(key);
	}
	s_api->prepare();
}

void GCodeEditor::updateLineNumberMargin()
{
	int lineCount = this->lines();
	int digitCount = QString::number(lineCount).length();
	QString filled;
	this->setMarginWidth(0, filled.fill('0', digitCount + 1));
}

QStringList getIntellisense(const QString& intellisenseFile)
{
	QStringList result;

	QFile file(intellisenseFile);
	if (!file.open(QIODevice::ReadOnly))
	{
		return result;
	}

	QTextStream stream(&file);
	stream.setEncoding(QStringConverter::Encoding::Utf8);
	QString str = stream.readAll();
	file.close();

	QJsonParseError jsonError;
	QJsonDocument doc = QJsonDocument::fromJson(str.toUtf8(), &jsonError);
	if (jsonError.error != QJsonParseError::NoError && !doc.isNull())
	{

	}

	QJsonObject rootObj = doc.object();
	QStringList kwords;

	static QStringList type_to_parse = { "g-code-commands","M-code-commands","parameters",
										"modifiers","functions","comment",
										"preprocessor","macro"
	};

	for (int i = 0; i < type_to_parse.size(); i++)
	{
		QJsonValue array = rootObj.value(type_to_parse[i]);

		if (array.type() == QJsonValue::Array) {
			QJsonArray arrayVal = array.toArray();
			int arrsize = arrayVal.size();
			for (int j = 0; j < arrsize; j++)
			{
				QJsonValue key = arrayVal.at(j);
				result.append(key.toString());
			}
		}
	}

	return result;
}