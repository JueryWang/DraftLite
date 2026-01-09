#pragma once

#include <QTimer>
#include <QString>
#include <Qsci/qsciapis.h>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciscintilla.h>
#include <Path/Path.h>
#include <map>

const int HIGHLIGHT_MARKER_ID = 0;
class CNCSYS::EntityVGPU;
class Path2D;

class GCodeEditor : public QsciScintilla
{
	Q_OBJECT
public:
	static void initFont(const QString& fontFamily, int fontSize);
	static void initIntellisense();
	static GCodeEditor* GetInstance()
	{
		if(!s_instance)
			s_instance = new GCodeEditor(nullptr);

		return s_instance;
	}
	void CleanCache();
	void SetSketch(CNCSYS::SketchGPU* _sketch);
	void SetAutoSendFTP(bool autoSend) { AutoSendFTP = autoSend; }

protected:
	void keyPressEvent(QKeyEvent* e) override;

private:
	GCodeEditor(QWidget* parent = nullptr);
	~GCodeEditor();

public slots:
	void SetMarkLine(int line);
	void updateLineNumberMargin();
	void onCursorPositionChanged(int line,int index);
	void onSelectionChanged();
	void onEditingFinished();
	void onFTPDone();

private slots:
	void handleGCodePaste();

private:
	static GCodeEditor* s_instance;
	static QsciLexerCPP* s_cpplexer;
	static QsciAPIs* s_api;
	QTimer *m_idleTimer;
	std::pair<int, int> lastSelection;
	CNCSYS::EntityVGPU* lastHoverEntity = nullptr;
	CNCSYS::SketchGPU* holdSketch;
	Path2D* lastHoverPath = nullptr;
	bool AutoSendFTP = true;
};