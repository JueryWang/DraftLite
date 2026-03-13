#define GLFW_EXPOSE_NATIVE_WIN32
#include "Graphics/Canvas.h"
#include "Graphics/OCS.h"
#include "Graphics/Sketch.h"
#include "UI/GLWidget.h"
#include "UI/TransformBaseHint.h"
#include "UI/MainLayer.h"
#include "UI/GCodeEditor.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Graphics/Anchor.h"
#include "Common/MathUtils.h"
#include "Common/HistoryCmds.h"
#include "IO/ObjectSerializer.h"
#include "ModalEvent/EntityRotateModal.h"
#include "ModalEvent/EntityScaleModal.h"
#include "ModalEvent/EntityMirrorModal.h"
#include "ModalEvent/EvCanvasSetNewScene.h"
#include "ModalEvent/EvSendCanvasTag.h"
#include "ModalEvent/MeasureDimension.h"
#include "Controls/ScadaScheduler.h"
#include <UI/CanvasGuide.h>
#include <QMessageBox>
#include <Windows.h>
#include <QMenu>
#include <QFile>
#include <QMouseEvent>
#include <QGestureEvent>
#include <QPinchGesture>
#include <QBuffer>
#include <chrono>
#include "Graphics/SelectionBox.h"

#include "GLFW/glfw3.h"
#include "ft2build.h"
#include "freetype/freetype.h"

struct Character
{
	unsigned int TextureID;
	glm::vec2 Size;
	glm::vec2 Bearing;
	unsigned int Advance;
};

std::map<GLchar, Character> Characters;

namespace CNCSYS
{

	CanvasGPU::CanvasGPU(std::shared_ptr<SketchGPU> sketch, OCSGPU* ocs, int width, int height, bool isMainCanvas) : OpenGLRenderWindow(width, height, ""), isMainCanvas(isMainCanvas)
	{
		firstResize = true;
		ocsSys = ocs;
		m_currentSketch = sketch;
		sketch->SetCanvas(this);
		UpdateOCS();
		mouseHint = new TransformBaseHint();
		mouseHint->hide();
		sketch->mainCanvas = this;

		this->Resize(QSize(width, height));

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_DEPTH);

		tickerRuleRenderShader = new Shader("Shader/drawTick.vert", "Shader/drawTick.frag");
		tickerTextRenderShader = new Shader("Shader/drawTickText.vert", "Shader/drawTickText.frag");
		if (isMainCanvas)
		{
			sketch.get()->mainCanvas = this;
			glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->width()), 0.0f, static_cast<float>(this->height()));
			tickerTextRenderShader->use();
			glUniformMatrix4fv(glGetUniformLocation(tickerTextRenderShader->ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

			FT_Library ft;
			if (FT_Init_FreeType(&ft))
			{
				std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
			}

			FT_Face face;
			if (FT_New_Face(ft, "C:/Windows/Fonts/comic.ttf", 0, &face))
			{
				std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
			}
			else
			{
				FT_Set_Pixel_Sizes(face, 0, 48);
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

				for (unsigned char c = 0; c < 128; c++)
				{
					if (FT_Load_Char(face, c, FT_LOAD_RENDER))
					{
						std::cout << "ERROR::FREETYP: Failed to load Glyph" << std::endl;
						continue;
					}

					unsigned int texture;
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);

					glTexImage2D(
						GL_TEXTURE_2D,
						0,
						GL_RED,
						face->glyph->bitmap.width,
						face->glyph->bitmap.rows,
						0,
						GL_RED,
						GL_UNSIGNED_BYTE,
						face->glyph->bitmap.buffer
					);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

					Character character = {
						texture,
						glm::ivec2(face->glyph->bitmap.width,face->glyph->bitmap.rows),
						glm::ivec2(face->glyph->bitmap_left,face->glyph->bitmap_top),
						static_cast<unsigned int>(face->glyph->advance.x)
					};

					Characters.insert(std::pair<char, Character>(c, character));
				}
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			FT_Done_Face(face);
			FT_Done_FreeType(ft);

			glGenVertexArrays(1, &tickerRenderVAO);
			glBindVertexArray(tickerRenderVAO);

			glGenBuffers(1, &tickerRenderVBO);
			glBindBuffer(GL_ARRAY_BUFFER, tickerRenderVBO);
			glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(glm::vec3), NULL, GL_DYNAMIC_DRAW);
			glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(glm::vec3), (void*)0);
			glEnableVertexAttribArray(0);

			glGenVertexArrays(1, &textRenderVAO);
			glGenBuffers(1, &textRenderVBO);
			glBindVertexArray(textRenderVAO);
			glBindBuffer(GL_ARRAY_BUFFER, textRenderVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		g_lineShader = new Shader("Shader/drawEntityModeLine.vert", "Shader/drawEntityModeLine.frag");
		g_showArrowShader = new Shader("Shader/drawEntityModeLine.vert", "Shader/drawEntityModeLine.frag", "Shader/addArrow.geom");
		g_dashedLineShader = new Shader("Shader/drawEntityModeLine.vert", "Shader/drawEntityModeLine.frag", "Shader/addDashed.geom");
		g_pointShader = new Shader("Shader/drawEntityModePoint.vert", "Shader/drawEntityModePoint.frag");
		g_mirrorShader = new Shader("Shader/drawMirroredEntity.vert", "Shader/drawEntityModeLine.frag");

		selectBox = new SelectionBox(m_currentSketch);
		hoverPoint = new Point2DGPU();
		hoverPoint->isVisible = false;

		modalCallbacks[ModalState::EntityMove] = std::bind(&CanvasGPU::handleEventEntityMove, this, std::placeholders::_1, std::placeholders::_2);
		modalCallbacks[ModalState::EntityRotate] = std::bind(&CanvasGPU::handleEventEntityRoatate, this, std::placeholders::_1, std::placeholders::_2);
		modalCallbacks[ModalState::EntityScale] = std::bind(&CanvasGPU::handleEventEntityScale, this, std::placeholders::_1, std::placeholders::_2);
		modalCallbacks[ModalState::EntityMirror] = std::bind(&CanvasGPU::handleEventEntityMirror, this, std::placeholders::_1, std::placeholders::_2);
		modalCallbacks[ModalState::MeasureDimension] = std::bind(&CanvasGPU::handleEventMeasureDimension, this, std::placeholders::_1, std::placeholders::_2);
		HistoryRecorder::GetInstance()->SetSketch(m_currentSketch.get());

		//guide = CanvasGuide::GetInstance();
		//connect(guide->longPressTimerViewLeft, &QTimer::timeout, this, &CanvasGPU::OnPaceLeft);
		//connect(guide->longPressTimerViewRight, &QTimer::timeout, this, &CanvasGPU::OnPaceRight);
		//connect(guide->longPressTimerViewUp, &QTimer::timeout, this, &CanvasGPU::OnPaceUp);
		//connect(guide->longPressTimerViewDown, &QTimer::timeout, this, &CanvasGPU::OnPaceDown);
		//connect(guide->longPressTimerZoomIn, &QTimer::timeout, this, &CanvasGPU::OnZoomIn);
		//connect(guide->longPressTimerZoomOut, &QTimer::timeout, this, &CanvasGPU::OnZoomOut);

		toolAnchor = Anchor::GetInstance();
		toolAnchor->SetCoordinateSystem(ocsSys);
		toolAnchor->SetCurrentCanvas(this);
		toolAnchor->animatorOpen = true;
		ScadaScheduler::GetInstance()->AddNode(toolAnchor);
		g_canvasInstance = this;
	}

	CanvasGPU::~CanvasGPU()
	{
		m_currentSketch.reset();
		for (EntityVGPU* ent : assistantEnt)
		{
			delete ent;
		}
		delete selectBox;
		delete mouseHint;
		delete hoverPoint;
		delete ocsSys;
		delete tickerTextRenderShader;
		delete tickerRuleRenderShader;
	}

	void CanvasGPU::UpdateOCS()
	{
		//更新画面
		ocsSys->ComputeScaleFitToCanvas();
		ocsSys->FitToZero();
		if (isMainCanvas)
		{
			ocsSys->UpdateTickers();
		}
		for (EntityVGPU* ent : m_currentSketch.get()->entities)
		{
			ent->Move(ocsSys->translationToZero);
			ent->UpdatePaintData();
			m_currentSketch.get()->UpdateEntityBox(ent, ent->bbox);
		}
	}

	bool CanvasGPU::eventFilter(QObject* obj, QEvent* event)
	{
		GLWidget* glwgt = (GLWidget*)this->parent();
		if (event->type() == EvCanvasSetNewScene::eventType)
		{
			GCodeEditor::GetInstance()->SetAutoSendFTP(false);
			EvCanvasSetNewScene* evSetScene = static_cast<EvCanvasSetNewScene*>(event);
			this->SetScene(evSetScene->m_sketch, evSetScene->m_ocs);
			event->accept();
			return true;
		}
		if (event->type() == EvSendCanvasTag::eventType)
		{
			EvSendCanvasTag* evSendTag = static_cast<EvSendCanvasTag*>(event);
			frontWidget->AddTag(Tag(evSendTag->tagPos, evSendTag->tagLabel, evSendTag->tagSize));
			event->accept();
			return true;
		}
		switch (event->type())
		{
		case QEvent::Resize:
		{
			int width, height;
			glfwGetWindowSize(m_window.get(), &width, &height);
			if (ocsSys != nullptr)
			{
				ocsSys->SetCanvasSizae(width, height);
				if (firstResize)
				{
					UpdateOCS();
					firstResize = false;
				}
			}
			break;
		}
		case QEvent::MouseButtonPress:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->buttons() & Qt::MiddleButton)
			{
				isDragging = true;
				lastMousePos = mouseEvent->pos();
			}
			if (mouseEvent->button() == Qt::RightButton)
			{
				QMenu menu(nullptr);
				if (captureType == CaptureMode::Point)
				{
					if (hoverPoint->isVisible)
					{
						QAction* actionSetStart = menu.addAction("设置为加工起点");
						connect(actionSetStart, &QAction::triggered, [this]() {
							if (lastHoverEntity->ringParent != nullptr)
							{
									lastHoverEntity->ringParent->SetStartPoint(lastHoverEntity, lastHoverEntity->editPointIndex);
							}
							m_currentSketch.get()->UpdateGCode(true);
						});
						QAction* actionSetEnd = menu.addAction("设置为加工终点");
						connect(actionSetEnd, &QAction::triggered, [this]() {
								if (lastHoverEntity->ringParent != nullptr)
								{
									lastHoverEntity->ringParent->SetEndPoint(lastHoverEntity, lastHoverEntity->editPointIndex);
								}
							m_currentSketch->UpdateGCode(true);
							});
						QAction* actionSetOrigin = menu.addAction("设置为图形原点");
						connect(actionSetOrigin, &QAction::triggered, [this]() {
							m_currentSketch.get()->SetOrigin(glm::vec3(this->hoverPoint->point.x, this->hoverPoint->point.y, 0.0f));
						});
					}
				}
				if (selectedItems.size() )
				{
					QAction* clearSelect = menu.addAction("取消选择");
					connect(clearSelect, &QAction::triggered, [this]() {
						for (EntityVGPU* ent : selectedItems)
						{
							ent->isSelected = false;
						}
						selectedItems.clear();
						});
					QAction* reverse = menu.addAction("反向");
					connect(reverse, &QAction::triggered, [this]()
						{
							std::set<EntRingConnection*> groups = m_currentSketch.get()->GetParentRings(selectedItems);
							for (EntRingConnection* group : groups)
							{
								group->Reverse();
							}
							m_currentSketch->UpdateGCode();
						});
				}
				if (static_cast<uint64_t>(operationState & ModalState::EntityRotate))
				{
					QAction* cancelRotate = menu.addAction("取消旋转");
					connect(cancelRotate, &QAction::triggered, [this]() {this->CancelModal(); });
				}
				if (static_cast<uint64_t>(operationState & ModalState::EntityScale))
				{
					QAction* cancelScale = menu.addAction("取消缩放");
					connect(cancelScale, &QAction::triggered, [this]() {this->CancelModal(); });
				}
				if (static_cast<uint64_t>(operationState & (ModalState::CreatePoint | ModalState::CreateLine | ModalState::CreatePolyline | ModalState::CreateCircle
					| ModalState::CreateArc | ModalState::CreateRectangle | ModalState::CreateSpline)))
				{
					QAction* actionConfirm = menu.addAction("确定");
					QAction* actionCancel = menu.addAction("取消");

					connect(actionConfirm, &QAction::triggered, this, &CanvasGPU::OnComfirmCreateElements);
					connect(actionCancel, &QAction::triggered, this, &CanvasGPU::OnCancelCreateElements);
				}
				if (static_cast<uint64_t>(operationState & ModalState::MeasureDimension))
				{
					QAction* actionCancelMeasure = menu.addAction("取消测量");
					connect(actionCancelMeasure, &QAction::triggered, [this]() {this->CancelModal(); });
				}
				if (menu.actions().size() > 0)
					menu.exec(QCursor().pos());
			}
			frontWidget->repaint();
			break;
		}
		case QEvent::MouseMove:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint currentMousePos = mouseEvent->pos();

			offsetMove = currentMousePos - lastMousePos;

			if (isDragging)
			{
				ocsSys->OnMouseMove(glm::vec2(-offsetMove.x(), offsetMove.y()));
			}

			lastMousePos = currentMousePos;
			if (static_cast<uint64_t>(operationState & (ModalState::NormalInteract | ModalState::MeasureDimension)))
			{
				handleEventCapture(glwgt, mouseEvent->pos());
			}
			frontWidget->repaint();
			break;
		}
		case QEvent::Wheel:
		{
			if (!isDragging)
			{
				QWheelEvent* wheelEvent = static_cast<QWheelEvent*>(event);
				float delta = -wheelEvent->angleDelta().y() * 0.01;
				lastMousePos = wheelEvent->position().toPoint();

				ocsSys->OnMouseScroll(delta, glm::vec2(lastMousePos.x(), lastMousePos.y()));
			}
			if (operationState == ModalState::NormalInteract)
			{
				handleEventCapture(glwgt, lastMousePos);
			}
			frontWidget->repaint();
			break;
		}
		case QEvent::MouseButtonRelease:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			if (mouseEvent->button() & Qt::MiddleButton)
			{
				isDragging = false;
			}
			break;
		}
		case QEvent::KeyPress:
		{
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Delete)
			{
				EraseSelectedEntitys();
			}
			frontWidget->repaint();
			break;
		}
		}
		handleEventSelection(glwgt, obj, event);
		if (modalCallbacks[operationState] != nullptr)
		{
			modalCallbacks[operationState](glwgt, event);
		}
		handleEventCreateEntity(glwgt, obj, event);
		handleEventShortCut(glwgt, event);
		this->updateGL();

		return glwgt->eventFilter(obj, event);
	}

	void CanvasGPU::DrawTickers()
	{
		for (auto& ticker : ocsSys->tickers)
		{
			glm::vec2 tickTextCoord = ticker.tickCoord[1];
			tickTextCoord /= 2;
			tickTextCoord += glm::vec2(0.5f, 0.5f);
			if (ticker.Atype == AxisType::X)
			{
				tickTextCoord += glm::vec2(0.005f, -0.01f);
			}
			else
			{
				tickTextCoord += glm::vec2(-0.01f, 0.01f);
			}

			tickTextCoord.x *= this->width();
			tickTextCoord.y *= this->height();

			if (ticker.Ttype == TickType::Sub)
			{
				glLineWidth(1.0f);
				tickerRuleRenderShader->use();
				std::vector<glm::vec3> vertices = { glm::vec3(ticker.tickCoord[0],0.0),glm::vec3(ticker.tickCoord[1],0.0) };
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, sizeof(glm::vec3), vertices.data());
				glDrawArrays(GL_LINES, 0, 2);
				glDisableClientState(GL_VERTEX_ARRAY);
			}
			else if (ticker.Ttype == TickType::Main)
			{
				tickerRuleRenderShader->use();
				glLineWidth(1.5);
				std::vector<glm::vec3> vertices = { glm::vec3(ticker.tickCoord[0],0.0),glm::vec3(ticker.tickCoord[1],0.0) };
				glBindVertexArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, sizeof(glm::vec3), vertices.data());
				glDrawArrays(GL_LINES, 0, 2);
				glDisableClientState(GL_VERTEX_ARRAY);
			}
		}
	}

	void CanvasGPU::handleEventCreateEntity(GLWidget* glwgt, QObject* obj, QEvent* event)
	{
		if (static_cast<uint64_t>(operationState & (ModalState::CreatePoint | ModalState::CreateLine | ModalState::CreatePolyline | ModalState::CreateCircle
			| ModalState::CreateArc | ModalState::CreateRectangle | ModalState::CreateSpline)))
		{
			switch (event->type())
			{
			case QEvent::MouseButtonPress:
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				if (mouseEvent->button() == Qt::LeftButton)
				{
					if (operationState != ModalState::NormalInteract)
					{
						glm::vec2 ocsPosition = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mouseEvent->pos().x(), mouseEvent->pos().y()));
						modalClickPos.push_back(glm::vec3(ocsPosition, 0.0f));
						switch (operationState)
						{
						case ModalState::CreatePoint:
						{
							Point2DGPU* pt = static_cast<Point2DGPU*>(tempEntityCreated);
							pt->SetParameter(modalClickPos[0]);

							auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), pt);
							if (find != m_currentSketch.get()->entities.end())
							{
								m_currentSketch.get()->UpdateEntityBox(pt, pt->bbox);
							}
							this->EndModal();
							modalClickPos.clear();
							break;
						}
						case ModalState::CreateLine:
						{
							if (modalClickPos.size() == 2)
							{
								glm::vec3 start = modalClickPos[0];
								glm::vec3 end = modalClickPos[1];
								Line2DGPU* line = static_cast<Line2DGPU*>(tempEntityCreated);
								line->SetParameter(start, end);

								auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), line);
								if (find != m_currentSketch.get()->entities.end())
								{
									m_currentSketch.get()->UpdateEntityBox(line, line->bbox);
								}
								this->EndModal();
								modalClickPos.clear();
							}
							break;
						}
						case ModalState::CreatePolyline:
						{
							break;
						}
						case ModalState::CreateCircle:
						{
							if (modalClickPos.size() == 2)
							{
								glm::vec3 center = modalClickPos[0];
								glm::vec3 ptAtCircle = modalClickPos[1];
								Circle2DGPU* circle = static_cast<Circle2DGPU*>(tempEntityCreated);

								float radius = glm::distance(center, ptAtCircle);
								circle->SetParameter(center, radius);

								auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), circle);
								if (find != m_currentSketch.get()->entities.end())
								{
									m_currentSketch.get()->UpdateEntityBox(circle, circle->bbox);
								}
								this->EndModal();
								modalClickPos.clear();
							}
							break;
						}
						case ModalState::CreateArc:
						{
							if (modalClickPos.size() == 3)
							{
								glm::vec3 p1 = glm::vec3(modalClickPos[0]);
								glm::vec3 p2 = glm::vec3(modalClickPos[1]);
								glm::vec3 p3 = glm::vec3(modalClickPos[2]);
								glm::vec3 center;
								float angleStart, angleEnd, radius;

								auto tuple = MathUtils::CalculateCircleByThreePoints(p1, p2, p3);
								std::tie(center, angleStart, angleEnd, radius) = tuple;

								Arc2DGPU* arc = static_cast<Arc2DGPU*>(tempEntityCreated);
								arc->SetParameter(center, angleStart, angleEnd, radius);

								auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), arc);
								if (find != m_currentSketch.get()->entities.end())
								{
									m_currentSketch.get()->UpdateEntityBox(arc, arc->bbox);
								}

								for (EntityVGPU* assistant : assistantEnt)
								{
									delete assistant;
								}
								assistantEnt.clear();
								this->EndModal();
								modalClickPos.clear();
							}
							break;
						}
						case ModalState::CreateRectangle:
						{
							if (modalClickPos.size() == 2)
							{
								glm::vec3 p1 = modalClickPos[0];
								glm::vec3 p2 = modalClickPos[1];

								Polyline2DGPU* rectangle = static_cast<Polyline2DGPU*>(tempEntityCreated);
								std::vector<glm::vec3> polynodes;
								polynodes.push_back(p1);
								polynodes.push_back(glm::vec3(p1.x, p2.y, 0.0f));
								polynodes.push_back(p2);
								polynodes.push_back(glm::vec3(p2.x, p1.y, 0.0f));
								rectangle->SetParameter(polynodes, true, { 0,0,0 });

								auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), rectangle);
								if (find != m_currentSketch.get()->entities.end())
								{
									m_currentSketch.get()->UpdateEntityBox(rectangle, rectangle->bbox);
								}
								this->EndModal();
							}
							break;
						}
						case ModalState::CreateSpline:
						{
							glm::vec3 p = glm::vec3(ocsPosition, 0.0f);
							Point2DGPU* ptEnt = new Point2DGPU(p);
							ptEnt->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
							assistantEnt.push_back(ptEnt);

							if (modalClickPos.size() >= 4)
							{
								Spline2DGPU* spline = static_cast<Spline2DGPU*>(tempEntityCreated);
								std::vector<glm::vec3> controlPoints = modalClickPos;

								std::vector<float> knots = MathUtils::GenerateClampedKnots(controlPoints.size(), 3);
								spline->SetParameter(controlPoints, knots, false);

								auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), spline);
								if (find != m_currentSketch.get()->entities.end())
								{
									m_currentSketch.get()->UpdateEntityBox(spline, spline->bbox);
								}
							}

							break;
						}
						default:
							break;
						}
					}
					else
					{
						isSelecting = true;
					}

				}
				break;

			}
			case QEvent::MouseMove:
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				QPoint currentMousePos = mouseEvent->pos();

				switch (operationState)
				{
				case ModalState::NormalInteract:
				{
					break;
				}
				case ModalState::CreateLine:
				{
					if (modalClickPos.size() == 1)
					{
						Line2DGPU* line = static_cast<Line2DGPU*>(tempEntityCreated);

						line->SetParameter(modalClickPos[0], ocsSys->GetOCSPosWithPixelPos(glm::vec2(currentMousePos.x(), currentMousePos.y())));
						auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), line);
						if (find == m_currentSketch.get()->entities.end())
						{
							m_currentSketch.get()->AddEntity(line);
						}
					}
					break;
				}
				case ModalState::CreatePolyline:
				{
					if (modalClickPos.size() >= 1)
					{
						Polyline2DGPU* poly = static_cast<Polyline2DGPU*>(tempEntityCreated);

						std::vector<glm::vec3> polynodes = modalClickPos;

						polynodes.push_back(ocsSys->GetOCSPosWithPixelPos(glm::vec2(currentMousePos.x(), currentMousePos.y())));
						std::vector<float> bulges(polynodes.size() - 1, 0);
						poly->SetParameter(polynodes, false, bulges);

						auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), poly);
						if (find == m_currentSketch.get()->entities.end())
						{
							m_currentSketch.get()->AddEntity(poly);
						}

						break;
					}
					break;
				}
				case ModalState::CreateCircle:
				{
					if (modalClickPos.size() > 0)
					{
						Circle2DGPU* circle = static_cast<Circle2DGPU*>(tempEntityCreated);

						glm::vec3 center = modalClickPos[0];
						glm::vec3 ptAtCircle = ocsSys->GetOCSPosWithPixelPos(glm::vec2(currentMousePos.x(), currentMousePos.y()));

						float radius = glm::distance(center, ptAtCircle);
						auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), circle);
						circle->SetParameter(center, radius);

						if (find == m_currentSketch.get()->entities.end())
						{
							m_currentSketch.get()->AddEntity(circle);
						}
					}
					break;
				}
				case ModalState::CreateArc:
				{
					if (modalClickPos.size() == 1)
					{
						glm::vec3 p1 = modalClickPos[0];
						glm::vec3 p2 = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mouseEvent->pos().x(), mouseEvent->pos().y()));
						if (assistantEnt.size() < 1)
						{
							Line2DGPU* line = new Line2DGPU(p1, p2);
							line->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
							assistantEnt.push_back(line);
						}
						else
						{
							Line2DGPU* line = static_cast<Line2DGPU*>(assistantEnt[assistantEnt.size() - 1]);
							line->SetParameter(p1, p2);
						}
					}
					else if (modalClickPos.size() == 2)
					{
						glm::vec3 p1 = modalClickPos[0];
						glm::vec3 p2 = modalClickPos[1];
						glm::vec3 p3 = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mouseEvent->pos().x(), mouseEvent->pos().y()));
						glm::vec3 center;
						float angleStart, angleEnd, radius;

						std::tie(center, angleStart, angleEnd, radius) = MathUtils::CalculateCircleByThreePoints(p1, p2, p3);

						if (!std::isnan(center.x) && !std::isnan(center.y) && !std::isnan(angleStart) && !std::isnan(angleEnd) && !std::isnan(radius))
						{
							if (assistantEnt.size() < 2)
							{
								Line2DGPU* line = new Line2DGPU(p2, p3);
								line->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
								assistantEnt.push_back(line);
							}
							else
							{
								Line2DGPU* line = static_cast<Line2DGPU*>(assistantEnt[assistantEnt.size() - 1]);
								line->SetParameter(p2, p3);
							}

							Arc2DGPU* arc = static_cast<Arc2DGPU*>(tempEntityCreated);
							if (!isnan(angleStart) && !isnan(angleEnd))
								arc->SetParameter(center, angleStart, angleEnd, radius);
							auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), arc);
							if (find == m_currentSketch.get()->entities.end())
							{
								m_currentSketch.get()->AddEntity(arc);
							}
						}

					}
					break;
				}
				case ModalState::CreateRectangle:
				{
					if (modalClickPos.size() == 1)
					{
						glm::vec3 p1 = modalClickPos[0];
						glm::vec3 p2 = ocsSys->GetOCSPosWithPixelPos(glm::vec2(currentMousePos.x(), currentMousePos.y()));

						Polyline2DGPU* rectangle = static_cast<Polyline2DGPU*>(tempEntityCreated);
						std::vector<glm::vec3> polylinenodes = { p1,glm::vec3(p1.x,p2.y,0.0f),p2,glm::vec3(p2.x,p1.y,0.0f) };
						std::vector<float> bulges(polylinenodes.size() - 1, 0);
						rectangle->SetParameter(polylinenodes, true, bulges);

						auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), rectangle);
						if (find == m_currentSketch.get()->entities.end())
						{
							m_currentSketch.get()->AddEntity(rectangle);
						}
					}
					break;
				}
				case ModalState::CreateSpline:
				{
					if (modalClickPos.size() >= 3)
					{
						glm::vec3 p = ocsSys->GetOCSPosWithPixelPos(glm::vec2(currentMousePos.x(), currentMousePos.y()));
						Spline2DGPU* spline = static_cast<Spline2DGPU*>(tempEntityCreated);
						std::vector<glm::vec3> controlPoints;

						for (auto& pt : modalClickPos)
						{
							controlPoints.push_back(pt);
						}
						controlPoints.push_back(p);

						std::vector<float> knots = MathUtils::GenerateClampedKnots(controlPoints.size(), 3);
						spline->SetParameter(controlPoints, knots, false);

						auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), spline);
						if (find == m_currentSketch.get()->entities.end())
						{
							m_currentSketch.get()->AddEntity(spline);
						}
					}
					break;
				}
				default:
					break;
				}
			}
			break;
			}
		}
	}
	void CanvasGPU::handleEventSelection(GLWidget* glwgt, QObject* obj, QEvent* event)
	{
		static bool dragEntity = false;
		if (!static_cast<uint64_t>(operationState & (ModalState::CreatePoint | ModalState::CreateLine | ModalState::CreatePolyline | ModalState::CreateCircle
			| ModalState::CreateArc | ModalState::CreateRectangle | ModalState::CreateSpline | ModalState::EntityRotate | ModalState::EntityScale
			| ModalState::EntityMirror | ModalState::EntityMove | ModalState::MeasureDimension)) && captureType != CaptureMode::Point)
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			switch (event->type())
			{
			case QEvent::MouseButtonPress:
			{
				if (mouseEvent->button() == Qt::LeftButton)
				{
					if (lastHoverEntity != nullptr && lastHoverEntity->isHover)
					{
						auto find = std::find(selectedItems.begin(), selectedItems.end(), lastHoverEntity);
						if (find == selectedItems.end())
						{
							selectedItems.push_back(lastHoverEntity);
							lastHoverEntity->isSelected = true;
							return;
						}
					}
					glm::vec3 anchorPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mouseEvent->pos().x(), mouseEvent->pos().y()));
					float precision = std::max(ocsSys->canvasRange->MaxRange() * 0.01f, 1.0f);
					double distance = m_currentSketch.get()->GetDistanceToSelectedItems(anchorPos);

					if (distance < precision)
					{
						EnterModal(ModalState::EntityMove);
						return;
					}
					if (selectBox->anchorPoints.size() == 0)
					{
						selectBox->anchorPoints.push_back(anchorPos);
						isSelecting = true;
					}


					operationState = ModalState::NormalInteract;
				}
				break;
			}
			case QEvent::MouseMove:
			{
				if (isSelecting)
				{
					glm::vec3 pos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mouseEvent->pos().x(), mouseEvent->pos().y()));

					if (selectBox->anchorPoints.size() == 1)
					{
						selectBox->anchorPoints.push_back(ocsSys->GetOCSPosWithPixelPos(glm::vec2(mouseEvent->pos().x(), mouseEvent->pos().y())));
					}
					else if (selectBox->anchorPoints.size() == 2)
					{
						selectBox->anchorPoints[1] = pos;

						float minX = qMin(selectBox->anchorPoints[0].x, selectBox->anchorPoints[1].x);
						float maxX = std::max(selectBox->anchorPoints[0].x, selectBox->anchorPoints[1].x);
						float minY = qMin(selectBox->anchorPoints[0].y, selectBox->anchorPoints[1].y);
						float maxY = std::max(selectBox->anchorPoints[0].y, selectBox->anchorPoints[1].y);
						selectBox->selectionRegion.min = glm::vec3(minX, minY, 0.0f);
						selectBox->selectionRegion.max = glm::vec3(maxX, maxY, 0.0f);
					}
				}
				break;
			}
			case QEvent::MouseButtonRelease:
			{
				if (isSelecting && operationState != ModalState::EntityMove)
				{
					if (selectBox->anchorPoints.size() > 1)
					{
						selectBox->anchorPoints[1] = (ocsSys->GetOCSPosWithPixelPos(glm::vec2(mouseEvent->pos().x(), mouseEvent->pos().y())));
						float minX = qMin(selectBox->anchorPoints[0].x, selectBox->anchorPoints[1].x);
						float maxX = std::max(selectBox->anchorPoints[0].x, selectBox->anchorPoints[1].x);
						float minY = qMin(selectBox->anchorPoints[0].y, selectBox->anchorPoints[1].y);
						float maxY = std::max(selectBox->anchorPoints[0].y, selectBox->anchorPoints[1].y);
						selectBox->selectionRegion.min = glm::vec3(minX, minY, 0.0f);
						selectBox->selectionRegion.max = glm::vec3(maxX, maxY, 0.0f);

						//先将之前选中的Items取消选择
						for (EntityVGPU* ent : selectedItems)
						{
							ent->isSelected = false;
						}

						selectedItems = m_currentSketch.get()->QueryBatchSelection(selectBox->selectionRegion);

						std::set<EntityVGPU*> uniqueSelection;
						for (EntityVGPU* ent : selectedItems)
						{
							if (ent->ringParent != nullptr)
							{
								for (EntityVGPU* conponent : ent->ringParent->conponents)
								{
									uniqueSelection.insert(conponent);
								}
							}
							else
							{
								uniqueSelection.insert(ent);
							}
						}

						selectedItems.clear();

						for (EntityVGPU* ent : uniqueSelection)
						{
							selectedItems.push_back(ent);
							ent->isSelected = true;
						}

						isSelecting = false;
						lastSelectionRegion = AABB(selectBox->selectionRegion.min, selectBox->selectionRegion.max);
						selectBox->selectionRegion.min = glm::vec3(0);
						selectBox->selectionRegion.max = glm::vec3(0);
						selectBox->anchorPoints.clear();
					}
				}
			}
			}
		}
	}

	void CanvasGPU::handleEventCapture(GLWidget* glwgt, const QPointF& mousePos)
	{
		glm::vec3 capturePos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));

		float precision = ocsSys->canvasRange->MinRange() * 0.05f;
		if (captureType == CaptureMode::Entity)
		{
			EntityVGPU* ent = m_currentSketch.get()->QueryNearsetEntity(capturePos, precision);

			if (lastHoverEntity)
			{
				lastHoverEntity->isHover = false;
				lastHoverEntity->ResetColor();
			}

			if (ent != nullptr)
			{
				if (!ent->isSelected)
				{
					lastHoverEntity = ent;
					ent->isHover = true;
					ent->color = g_yellowColor;
				}
			}
			else
			{
			}
		}
		else if (captureType == CaptureMode::Point)
		{
			EntityVGPU* searchEntity;
			int pointIndx;
			glm::vec2 pointPos;

			std::tie(searchEntity, pointIndx, pointPos) = m_currentSketch.get()->QueryNearestPoint(capturePos, precision);
			if (searchEntity == nullptr)
			{
				hoverPoint->isHover = false;
				hoverPoint->isVisible = false;

				if (lastHoverEntity)
				{
					lastHoverEntity->isHover = false;
					lastHoverEntity->ResetColor();
				}
				lastHoverEntity = nullptr;
				return;
			}

			lastHoverEntity = searchEntity;
			lastHoverEntity->isHover = false;
			lastHoverEntity->ResetColor();

			if (abs(pointIndx - searchEntity->indexRange.first) < abs(pointIndx - searchEntity->indexRange.second))
			{
				glm::vec3 editPos = searchEntity->GetStart();
				float distance = glm::distance(editPos, capturePos);
				if (distance < precision)
				{
					searchEntity->editPointIndex = searchEntity->indexRange.first;
					hoverPoint->isHover = true;
					hoverPoint->isVisible = true;
					hoverPoint->SetParameter(glm::vec3(pointPos.x, pointPos.y, 1.0f));
				}
			}
			else
			{
				glm::vec3 editPos = searchEntity->GetEnd();
				float distance = glm::distance(editPos, capturePos);
				if (distance < precision)
				{
					searchEntity->editPointIndex = searchEntity->indexRange.second;
					hoverPoint->isHover = true;
					hoverPoint->isVisible = true;
					hoverPoint->SetParameter(glm::vec3(pointPos.x, pointPos.y, 1.0f));
				}
			}
		}
	}

	void CanvasGPU::handleEventEntityMove(GLWidget* glwgt, QEvent* event)
	{
		static bool entityMoveDragging = false;
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
		{
			entityMoveDragging = true;
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			lastMousePos = mouseEvent->pos();
			break;
		}
		case QEvent::MouseMove:
		{
			if (entityMoveDragging)
			{
				QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
				glm::vec3 lastCanvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(lastMousePos.x() - offsetMove.x(), lastMousePos.y() - offsetMove.y()));
				glm::vec3 currentCanvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(lastMousePos.x(), lastMousePos.y()));

				for (EntityVGPU* ent : selectedItems)
				{
					ent->Move(currentCanvasPos - lastCanvasPos);
				}
			}
			break;
		}
		case QEvent::MouseButtonRelease:
		{
			entityMoveDragging = false;
			//更新场景RTree
			for (EntityVGPU* ent : selectedItems)
			{
				ent->UpdatePaintData();
				m_currentSketch.get()->UpdateEntityBox(ent, ent->bbox);
			}
			this->EndModal();
			break;
		}
		}
	}

	void CanvasGPU::handleEventEntityRoatate(GLWidget* glwgt, QEvent* event)
	{

		EntityRotateModal* rotateModal = static_cast<EntityRotateModal*>(modalEv);
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePos = mouseEvent->pos();
			glm::vec3 canvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));
			modalClickPos.push_back(canvasPos);
			if (rotateModal->processStep == 0)
			{
				rotateModal->TransitionToProcess1(canvasPos);
			}
			else if (rotateModal->processStep == 1)
			{
				rotateModal->TransitionToProcess2(canvasPos);
			}
			else if (rotateModal->processStep == 2)
			{
				rotateModal->TransitionToProcess3(canvasPos);
			}
			break;
		}
		case QEvent::MouseMove:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePos = mouseEvent->pos();
			glm::vec3 canvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));

			if (rotateModal->processStep == 0)
			{
				handleEventCapture(glwgt, mouseEvent->pos());
				if (hoverPoint->isVisible)
				{
					mouseHint->mouseSpinX->setValue(hoverPoint->point.x);
					mouseHint->mouseSpinY->setValue(hoverPoint->point.y);
					rotateModal->param.rotateTask->base = hoverPoint->point;
				}
				else
				{
					mouseHint->mouseSpinX->setValue(canvasPos.x);
					mouseHint->mouseSpinY->setValue(canvasPos.y);
					rotateModal->param.rotateTask->base = canvasPos;
				}
			}
			else if (rotateModal->processStep == 1)
			{
				handleEventCapture(glwgt, mouseEvent->pos());
				if (hoverPoint->isVisible)
				{
					mouseHint->mouseSpinX->setValue(hoverPoint->point.x);
					mouseHint->mouseSpinY->setValue(hoverPoint->point.y);
				}
				else
				{
					mouseHint->mouseSpinX->setValue(canvasPos.x);
					mouseHint->mouseSpinY->setValue(canvasPos.y);
				}
				rotateModal->param.rotateTask->startPoint = canvasPos;
			}
			else if (rotateModal->processStep == 2)
			{
				glm::vec3 base = rotateModal->param.rotateTask->base;
				glm::vec3 start = rotateModal->param.rotateTask->startPoint;
				rotateModal->param.rotateTask->endPoint = canvasPos;

				float angle = MathUtils::CounterClockwiseAngle(start - base, canvasPos - base);
				mouseHint->mouseSpinX->setValue(angle);
				float deltaRotate = angle - rotateModal->param.rotateTask->angle;
				rotateModal->param.rotateTask->angle = angle;

				for (EntityVGPU* ent : selectedItems)
				{
					ent->Rotate(base, deltaRotate);
				}
			}
			mouseHint->move(mouseEvent->pos());
			break;
		}
		}

	}

	void CanvasGPU::handleEventEntityScale(GLWidget* glwgt, QEvent* event)
	{
		EntityScaleModal* scaleModal = static_cast<EntityScaleModal*>(modalEv);
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePos = mouseEvent->pos();
			glm::vec3 canvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));
			modalClickPos.push_back(canvasPos);
			if (scaleModal->processStep == 0)
			{
				scaleModal->TransitionToProcess1(canvasPos);
			}
			else if (scaleModal->processStep == 1)
			{
				mouseHint->hide();

				scaleModal->TransitionToProcess2(canvasPos);
			}
			break;
		}
		case QEvent::MouseMove:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePos = mouseEvent->pos();
			glm::vec3 canvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));

			if (scaleModal->processStep == 0)
			{
				handleEventCapture(glwgt, mouseEvent->pos());
				if (hoverPoint->isVisible)
				{
					mouseHint->mouseSpinX->setValue(hoverPoint->point.x);
					mouseHint->mouseSpinY->setValue(hoverPoint->point.y);
					scaleModal->param.scaleTask->base = hoverPoint->point;
				}
				else
				{
					mouseHint->mouseSpinX->setValue(canvasPos.x);
					mouseHint->mouseSpinY->setValue(canvasPos.y);
					scaleModal->param.scaleTask->base = canvasPos;
				}
			}
			else if (scaleModal->processStep == 1)
			{
				scaleModal->param.scaleTask->mouse = canvasPos;

				float scaleAnchor = scaleModal->param.scaleTask->selectionBox.MinRange() / 2.0f;
				float hoverDistance = glm::distance(scaleModal->param.scaleTask->mouse, scaleModal->param.scaleTask->base);
				scaleModal->param.scaleTask->scale = hoverDistance / scaleAnchor;
				float scale = scaleModal->param.scaleTask->scale;
				mouseHint->mouseSpinX->setValue(scale);

				for (EntityVGPU* ent : selectedItems)
				{
					ent->Scale(glm::vec3(scale, scale, scale), scaleModal->param.scaleTask->selectionBox.Center());
				}
			}
			mouseHint->move(mouseEvent->pos());
			break;
		}
		}
	}

	void CanvasGPU::handleEventEntityMirror(GLWidget* glwgt, QEvent* event)
	{
		EntityMirrorModal* mirrorModal = static_cast<EntityMirrorModal*>(modalEv);
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePoint = mouseEvent->pos();
			glm::vec3 canvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePoint.x(), mousePoint.y()));
			if (mirrorModal->processStep == 0)
			{
				mirrorModal->TransitionToProcess1(canvasPos);
			}
			else if (mirrorModal->processStep == 1)
			{
				mouseHint->hide();
				mirrorModal->TransitionToProcess2(canvasPos);
			}
			break;
		}
		case QEvent::MouseMove:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePos = mouseEvent->pos();
			glm::vec3 canvasPos = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));

			if (mirrorModal->processStep == 0)
			{
				handleEventCapture(glwgt, mouseEvent->pos());
				if (hoverPoint->isVisible)
				{
					mouseHint->mouseSpinX->setValue(hoverPoint->point.x);
					mouseHint->mouseSpinY->setValue(hoverPoint->point.y);
					mirrorModal->param.mirrorTask->start = hoverPoint->point;
				}
				else
				{
					mouseHint->mouseSpinX->setValue(canvasPos.x);
					mouseHint->mouseSpinY->setValue(canvasPos.y);
					mirrorModal->param.mirrorTask->start = canvasPos;
				}
			}
			else if (mirrorModal->processStep == 1)
			{
				handleEventCapture(glwgt, mouseEvent->pos());
				if (hoverPoint->isVisible)
				{
					mouseHint->mouseSpinX->setValue(hoverPoint->point.x);
					mouseHint->mouseSpinY->setValue(hoverPoint->point.y);
					mirrorModal->param.mirrorTask->end = hoverPoint->point;
				}
				else
				{
					mouseHint->mouseSpinX->setValue(canvasPos.x);
					mouseHint->mouseSpinY->setValue(canvasPos.y);
					mirrorModal->param.mirrorTask->end = canvasPos;
				}
			}
			mouseHint->move(mouseEvent->pos());
			break;
		}
		default:
			break;
		}
	}

	void CanvasGPU::handleEventShortCut(GLWidget* glwgt, QEvent* event)
	{
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

			if (static_cast<uint64_t>(operationState & (ModalState::CreatePoint | ModalState::CreateLine | ModalState::CreateArc |
				ModalState::CreateCircle | ModalState::CreatePolyline | ModalState::CreateRectangle | ModalState::CreateSpline)))
			{
				tempEntityCreated = nullptr;
				modalClickPos.clear();
				assistantEnt.clear();
				operationState = ModalState::NormalInteract;
			}
			else
			{
				bool isUndoKey = (keyEvent->modifiers() & Qt::ControlModifier) && (keyEvent->key() == Qt::Key_Z);
				if (isUndoKey)
				{
					HistoryRecorder::GetInstance()->Revoke();
					return;
				}

				bool isRedoKey = (keyEvent->modifiers() & Qt::ControlModifier) && (keyEvent->key() == Qt::Key_Y);
				if (isRedoKey)
				{
					HistoryRecorder::GetInstance()->Restore();

				}
			}
		}
	}

	void CanvasGPU::handleEventMeasureDimension(GLWidget* glwgt, QEvent* event)
	{
		MeasureDimensionModal* measureModal = static_cast<MeasureDimensionModal*>(modalEv);
		switch (event->type())
		{
		case QEvent::MouseButtonPress:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePos = mouseEvent->pos();
			if ((mouseEvent->button() == Qt::LeftButton))
			{
				if (measureModal->processStep == 0)
				{
					if (lastHoverEntity != nullptr)
					{
						if (lastHoverEntity->editPointIndex >= 0)
						{
							measureModal->point1 = lastHoverEntity->GetTransformedNodes()[lastHoverEntity->editPointIndex];
						}
						else if (lastHoverEntity->editPointIndex == -1)
						{
							measureModal->point1 = lastHoverEntity->centroid;
						}
					}
					else
					{
						measureModal->point1 = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));
					}
					measureModal->processStep = (measureModal->processStep + 1) % 2;
					measureModal->allInited = false;
				}
				else if (measureModal->processStep == 1)
				{
					if (lastHoverEntity != nullptr)
					{
						if (lastHoverEntity->editPointIndex >= 0)
						{
							measureModal->point2 = lastHoverEntity->GetTransformedNodes()[lastHoverEntity->editPointIndex];
						}
						else if (lastHoverEntity->editPointIndex == -1)
						{
							measureModal->point2 = lastHoverEntity->centroid;
						}
					}
					else
					{
						measureModal->point2 = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));
					}
					measureModal->processStep = (measureModal->processStep + 1) % 2;
				}
			}
			break;
		}
		case QEvent::MouseMove:
		{
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
			QPoint mousePos = mouseEvent->pos();

			if (measureModal->processStep == 1)
			{
				if (lastHoverEntity != nullptr)
				{
					if (lastHoverEntity->editPointIndex >= 0)
					{
						measureModal->point2 = lastHoverEntity->GetTransformedNodes()[lastHoverEntity->editPointIndex];
					}
					else if (lastHoverEntity->editPointIndex == -1)
					{
						measureModal->point2 = lastHoverEntity->centroid;
					}
				}
				else
				{
					measureModal->point2 = ocsSys->GetOCSPosWithPixelPos(glm::vec2(mousePos.x(), mousePos.y()));
				}
				measureModal->allInited = true;
			}
		}
		default:
			break;
		}
	}

	void CanvasGPU::PushToHistory()
	{
		switch (operationState)
		{
		case ModalState::EntityMove:
		case ModalState::EntityRotate:
		case ModalState::EntityScale:
		case ModalState::EntityMirror:
		{
			HistoryRecord rec;
			std::vector<OperationCommand> opCmds;
			for (EntityVGPU* ent : selectedItems)
			{
				std::string s = serilize_to_string(ent);
				OperationCommand cmd = std::make_pair(ent, s);
				opCmds.push_back(cmd);
			}
			rec.commands = opCmds;
			rec.cleanFunc = [&](HistoryRecorder* recorder) {
				this->EndModal();
				};
			HistoryRecorder::GetInstance()->PushRecord(rec);
			break;
		}
		case ModalState::CreatePoint:
		case ModalState::CreateLine:
		case ModalState::CreateArc:
		case ModalState::CreateCircle:
		case ModalState::CreatePolyline:
		case ModalState::CreateRectangle:
		case ModalState::CreateSpline:
		{
			HistoryRecord rec;
			std::vector<OperationCommand> opCmds;
			OperationCommand cmd = std::make_pair(tempEntityCreated, "");
			opCmds.push_back(cmd);
			rec.commands = opCmds;
			rec.cleanFunc = [&](HistoryRecorder* recorder)
				{
					this->EndModal();
				};
			HistoryRecorder::GetInstance()->PushRecord(rec);
			break;
		}
		}
	}

	void CanvasGPU::SetScene(std::shared_ptr<SketchGPU> sketch, OCSGPU* ocs)
	{
		ocsSys = ocs;
		ocs->SetSketch(sketch);
		if (frontWidget != nullptr)
		{
			frontWidget->SetOCSystem(ocs);
		}
		toolAnchor->SetCoordinateSystem(ocs);
		m_currentSketch = sketch;
		sketch->mainCanvas = this;
		sketch->UpdateGCode();
		ocsSys->UpdateTickers();
		this->updateGL();
	}

	void CanvasGPU::SetFrontWidget(GLWidget* widget)
	{
		mouseHint->setParent(widget);
		frontWidget = widget;
		frontWidget->SetOCSystem(ocsSys);
	}

	void CanvasGPU::ModalEventDraw(const glm::vec2& viewport, const glm::mat4& projection, const glm::mat4& view)
	{
		if (modalEv != nullptr)
		{
			switch (modalEv->state)
			{
			case ModalState::EntityRotate:
			{
				EntityRotateModal* rotateModal = static_cast<EntityRotateModal*>(modalEv);
				rotateModal->modalFunc.ptr_RotateEntity(rotateModal, this, viewport, projection, view);
				break;
			}
			case ModalState::EntityScale:
			{
				EntityScaleModal* scaleModal = static_cast<EntityScaleModal*>(modalEv);
				scaleModal->modalFunc.ptr_ScaleEntity(scaleModal, this, viewport, projection, view);
				break;
			}
			case ModalState::EntityMirror:
			{
				EntityMirrorModal* mirrorModal = static_cast<EntityMirrorModal*>(modalEv);
				mirrorModal->modalFunc.ptr_MirrorEntity(mirrorModal, this, viewport, projection, view);
			}
			case ModalState::MeasureDimension:
			{
				MeasureDimensionModal* measureModal = static_cast<MeasureDimensionModal*>(modalEv);
				measureModal->modalFunc.ptr_MeasureDimension(measureModal, this, viewport, projection, view);
				break;
			}
			default:
				break;
			}
		}
	}

	void CanvasGPU::EnterModal(ModalState modal)
	{
		operationState = modal;
		modalClickPos.clear();

		switch (modal)
		{
		case ModalState::CreatePoint:
		{
			tempEntityCreated = new Point2DGPU();
			std::vector<OperationCommand> opCmds;
			break;
		}
		case ModalState::CreateLine:
		{
			tempEntityCreated = new Line2DGPU();
			break;
		}
		case ModalState::CreatePolyline:
		{
			tempEntityCreated = new Polyline2DGPU();
			break;
		}
		case ModalState::CreateCircle:
		{
			tempEntityCreated = new Circle2DGPU();
			break;
		}
		case ModalState::CreateArc:
		{
			tempEntityCreated = new Arc2DGPU();
			break;
		}
		case ModalState::CreateRectangle:
		{
			tempEntityCreated = new Polyline2DGPU();
			break;
		}
		case ModalState::CreateSpline:
		{
			tempEntityCreated = new Spline2DGPU();
			break;
		}
		case ModalState::EntityMove:
		{
			PushToHistory();
			break;
		}
		case ModalState::EntityRotate:
		{
			modalEv = new EntityRotateModal(this);
			modalEv->modalFunc.ptr_RotateEntity = EntityRotateModal::ModalRotateEntityFunc;

			if (!selectedItems.size())
			{
				this->EndModal();
				return;
			}

			for (EntityVGPU* selectedEnt : selectedItems)
			{
				selectedEnt->modelMatrixStash = selectedEnt->worldModelMatrix;
			}

			connect(mouseHint->mouseSpinX, &MouseHoverEditSpin::ConfirmData, this, [&](const glm::vec3& p) {
				EntityRotateModal* rotateModal = static_cast<EntityRotateModal*>(modalEv);
				rotateModal->TransitionToProcess1(p);
				});
			connect(mouseHint->mouseSpinY, &MouseHoverEditSpin::ConfirmData, this, [&](const glm::vec3& p) {
				EntityRotateModal* rotateModal = static_cast<EntityRotateModal*>(modalEv);
				rotateModal->TransitionToProcess1(p);
				});

			mouseHint->setParent(frontWidget);
			mouseHint->ResetToRotate();
			mouseHint->show();
			PushToHistory();
			break;
		}
		case ModalState::EntityScale:
		{
			modalEv = new EntityScaleModal(this);
			modalEv->modalFunc.ptr_ScaleEntity = EntityScaleModal::ModalScaleEntityFunc;

			if (!selectedItems.size())
			{
				this->EndModal();
				return;
			}

			connect(mouseHint->mouseSpinX, &MouseHoverEditSpin::ConfirmData, this, [&](const glm::vec3& p) {
				EntityScaleModal* scaleModal = static_cast<EntityScaleModal*>(modalEv);
				scaleModal->TransitionToProcess1(p);
				});
			connect(mouseHint->mouseSpinY, &MouseHoverEditSpin::ConfirmData, this, [&](const glm::vec3& p) {
				EntityScaleModal* scaleModal = static_cast<EntityScaleModal*>(modalEv);
				scaleModal->TransitionToProcess1(p);
				});

			AABB box = selectedItems[0]->bbox;
			for (EntityVGPU* selectedEnt : selectedItems)
			{
				selectedEnt->modelMatrixStash = selectedEnt->worldModelMatrix;
				box.Union(selectedEnt->bbox);
			}

			modalEv->param.scaleTask->selectionBox = box;
			mouseHint->setParent(frontWidget);
			mouseHint->ResetToScale();
			mouseHint->show();
			PushToHistory();
			break;
		}
		case ModalState::EntityMirror:
		{
			modalEv = new EntityMirrorModal(this);
			modalEv->modalFunc.ptr_MirrorEntity = EntityMirrorModal::ModalMirrorEntityFunc;

			if (!selectedItems.size())
			{
				this->EndModal();
				return;
			}

			connect(mouseHint->mouseSpinX, &MouseHoverEditSpin::ConfirmData, this, [&](const glm::vec3& p) {
				EntityMirrorModal* mirrorModal = static_cast<EntityMirrorModal*>(modalEv);
				mirrorModal->TransitionToProcess1(p);
				});
			connect(mouseHint->mouseSpinY, &MouseHoverEditSpin::ConfirmData, this, [&](const glm::vec3& p) {
				EntityMirrorModal* mirrorModal = static_cast<EntityMirrorModal*>(modalEv);
				mirrorModal->TransitionToProcess1(p);
				});

			for (EntityVGPU* selectedEnt : selectedItems)
			{
				selectedEnt->modelMatrixStash = selectedEnt->worldModelMatrix;
			}

			mouseHint->setParent(frontWidget);
			mouseHint->ResetToMirror();
			mouseHint->show();
			PushToHistory();
			break;
		}
		case ModalState::MeasureDimension:
		{
			modalEv = new MeasureDimensionModal(this);
			modalEv->modalFunc.ptr_MeasureDimension = MeasureDimensionModal::ModalMeasureDimensionFunc;
			SetCaptureMode(CaptureMode::Point);
			PushToHistory();

			break;
		}
		default:
			break;
		}
	}
	void CanvasGPU::EndModal()
	{
		if (modalEv != nullptr)
		{
			switch (modalEv->state)
			{
			case ModalState::EntityRotate:
			{
				delete (EntityRotateModal*)modalEv;
				break;
			}
			case ModalState::EntityScale:
			{
				delete (EntityScaleModal*)modalEv;
				break;
			}
			case ModalState::EntityMirror:
			{
				delete (EntityMirrorModal*)modalEv;
				break;
			}
			case ModalState::MeasureDimension:
			{
				delete (MeasureDimensionModal*)modalEv;
				break;
			}
			default:
				break;
			}
			m_currentSketch.get()->UpdateGCode();
		}
		switch (operationState)
		{
		case ModalState::CreatePoint:
		case ModalState::CreateLine:
		case ModalState::CreateRectangle:
		case ModalState::CreateArc:
		case ModalState::CreateCircle:
		case ModalState::CreatePolyline:
		case ModalState::CreateSpline:
		{
			PushToHistory();
			tempEntityCreated = nullptr;
			modalClickPos.clear();
			assistantEnt.clear();
			break;
		}
		case ModalState::EntityMove:
		{
			break;
		}
		default:
			break;
		}
		operationState = ModalState::NormalInteract;
		modalClickPos.clear();
	}

	void CanvasGPU::CancelModal()
	{
		mouseHint->hide();

		for (EntityVGPU* ent : selectedItems)
		{
			ent->worldModelMatrix = ent->modelMatrixStash;
		}
		EndModal();
	}

	void CanvasGPU::EraseSelectedEntitys()
	{
		HistoryRecord rec;
		std::vector<OperationCommand> opCmds;
		for (EntityVGPU* ent : selectedItems)
		{
			std::string s = serilize_to_string(ent);
			OperationCommand cmd = std::make_pair(nullptr, s);
			opCmds.push_back(cmd);
		}
		rec.commands = opCmds;
		rec.cleanFunc = [&](HistoryRecorder* recorder) {
			std::string NcProgram = m_currentSketch.get()->ToNcProgram();
			GCodeEditor::GetInstance()->setText(QString::fromStdString(NcProgram));
			m_currentSketch.get()->UpdateGCode();
			};
		HistoryRecorder::GetInstance()->PushRecord(rec);

		int count = 0;
		for (EntityVGPU* ent : selectedItems)
		{
			m_currentSketch.get()->EraseEntity(ent);
		}
		m_currentSketch.get()->UpdateGCode();
	}

	void CanvasGPU::ResetCanvas()
	{
		lastHoverEntity = nullptr;
		modalClickPos.clear();
		this->EndModal();
		tempEntityCreated = nullptr;
	}

	void CanvasGPU::SetCaptureMode(CaptureMode mode)
	{
		captureType = mode;
	}

	QImage CanvasGPU::GrabImage(SketchGPU* sketch, OCSGPU* ocs, int imageWidth, int imageHeight, const glm::vec4& bgcolor)
	{
		memset(m_windowbuf, 255, this->width() * this->height() * bit_per_pixel);

		ocs->SetCanvasSizae(this->width(), this->height());
		ocs->ComputeScaleFitToCanvas();
		ocs->FitToZero();


		for (EntityVGPU* entity : sketch->GetEntities())
		{
			entity->Move(ocs->translationToZero);
			entity->UpdatePaintData();
			sketch->UpdateEntityBox(entity, entity->bbox);
		}

		int widthStash, heightStash;
		GLFWwindow* windowInst = m_window.get();
		QImage image;

		if (windowInst)
		{
			glfwMakeContextCurrent(windowInst);
			glClearColor(bgcolor.x, bgcolor.y, bgcolor.z, bgcolor.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			g_lineWidth = 5.0f;

			glm::mat4 ortho = ocs->camera->GetOrthoGraphicMatrix();
			glm::mat4 view = ocs->camera->GetViewMatrix();

			g_lineShader->use();
			g_lineShader->setMat4("projection", ortho);
			g_lineShader->setMat4("view", view);

			for (EntityVGPU* ent : sketch->entities)
			{
				ent->color = g_darkGreen;
				ent->Paint(g_lineShader, ocs, RenderMode::Line);
				ent->ResetColor();
			}
			glReadPixels(0, 0, this->width(), this->height(), GL_RGB, GL_UNSIGNED_BYTE, m_windowbuf);
			glfwSwapBuffers(m_window.get());

			g_lineWidth = 2.0f;
			image = QImage(m_windowbuf, this->width(), this->height(), QImage::Format_RGB888).mirrored(false, true);
		}
		return image.scaled(imageWidth, imageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	}

	void CanvasGPU::RenderText(std::string& text, float x, float y, float scale, const glm::vec3& color)
	{
		size_t dotPos = text.find('.');
		if (dotPos != std::string::npos) {
			if (text.size() > dotPos + 5) {
				text = text.substr(0, dotPos + 5); // 保留小数点后两位
			}
		}

		tickerTextRenderShader->use();
		glUniform3f(glGetUniformLocation(tickerTextRenderShader->ID, "textColor"), color.x, color.y, color.z);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(textRenderVAO);

		std::string::const_iterator c;
		for (c = text.begin(); c != text.end(); c++)
		{
			Character ch = Characters[*c];
			float xpos = x + ch.Bearing.x * scale;
			float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

			float w = ch.Size.x * scale;
			float h = ch.Size.y * scale;

			float vertices[6][4] =
			{
				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos,     ypos,       0.0f, 1.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },

				{ xpos,     ypos + h,   0.0f, 0.0f },
				{ xpos + w, ypos,       1.0f, 1.0f },
				{ xpos + w, ypos + h,   1.0f, 0.0f }
			};

			glBindTexture(GL_TEXTURE_2D, ch.TextureID);
			glBindBuffer(GL_ARRAY_BUFFER, textRenderVBO);
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			x += (ch.Advance >> 6) * scale;
		}

		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void CanvasGPU::OnComfirmCreateElements()
	{
		switch (operationState)
		{
		case ModalState::CreateLine:
		{
			glm::vec3 start = modalClickPos[0];
			glm::vec3 end = modalClickPos[1];
			Line2DGPU* line = static_cast<Line2DGPU*>(tempEntityCreated);
			line->SetParameter(start, end);


			auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), line);
			if (find != m_currentSketch.get()->entities.end())
			{
				m_currentSketch.get()->UpdateEntityBox(line, line->bbox);
			}
			break;
		}
		case ModalState::CreatePolyline:
		{
			Polyline2DGPU* poly = static_cast<Polyline2DGPU*>(tempEntityCreated);
			std::vector<float> bulges(modalClickPos.size() - 1, 0);
			poly->SetParameter(modalClickPos, false, bulges);

			auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), poly);
			if (find != m_currentSketch.get()->entities.end())
			{
				m_currentSketch.get()->UpdateEntityBox(poly, poly->bbox);
			}

			break;
		}
		}

		for (EntityVGPU* assistant : assistantEnt)
		{
			delete assistant;
		}
		assistantEnt.clear();
		this->EndModal();
		modalClickPos.clear();

	}
	void CanvasGPU::OnCancelCreateElements()
	{
		auto find = std::find(m_currentSketch.get()->entities.begin(), m_currentSketch.get()->entities.end(), tempEntityCreated);
		if (find != m_currentSketch.get()->entities.end())
		{
			m_currentSketch.get()->entities.erase(find);
		}
		delete tempEntityCreated;

		for (EntityVGPU* assistant : assistantEnt)
		{
			delete assistant;
		}
		assistantEnt.clear();
		this->EndModal();
		modalClickPos.clear();
	}

	void CanvasGPU::OnPaceLeft()
	{
		ocsSys->OnMouseMove(glm::vec2(0.005f * this->m_width, 0));
	}

	void CanvasGPU::OnPaceRight()
	{
		ocsSys->OnMouseMove(glm::vec2(-0.005f * this->m_width, 0));
	}

	void CanvasGPU::OnPaceUp()
	{
		ocsSys->OnMouseMove(glm::vec2(0, -0.005f * this->m_height));
	}

	void CanvasGPU::OnPaceDown()
	{
		ocsSys->OnMouseMove(glm::vec2(0, 0.005f * this->m_height));
	}

	void CanvasGPU::OnZoomIn()
	{
		ocsSys->ScrollByCenter(0.01);
	}

	void CanvasGPU::OnZoomOut()
	{
		ocsSys->ScrollByCenter(-0.01);;
	}

	void CanvasGPU::updateGL()
	{
		GLFWwindow* windowInst = m_window.get();

		if (windowInst)
		{
			glfwMakeContextCurrent(windowInst);

			glClearColor(background.x, background.y, background.z, background.w);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glLineWidth(2);
			g_lineShader->use();
			int width, height;
			glfwGetWindowSize(windowInst, &width, &height);

			glm::mat4 ortho = ocsSys->camera->GetOrthoGraphicMatrix();
			glm::mat4 view = ocsSys->camera->GetViewMatrix();

			g_lineShader->setMat4("projection", ortho);
			g_lineShader->setMat4("view", view);

			g_mirrorShader->use();
			g_mirrorShader->setMat4("projection", ortho);
			g_mirrorShader->setMat4("view", view);

			g_showArrowShader->use();
			g_showArrowShader->setMat4("projection", ortho);
			g_showArrowShader->setMat4("view", view);
			g_showArrowShader->setVec2("viewportSize", glm::vec2(width, height));

			Shader* dashedLineRenderShader = g_dashedLineShader;
			dashedLineRenderShader->use();
			dashedLineRenderShader->setMat4("projection", ortho);
			dashedLineRenderShader->setMat4("view", view);
			dashedLineRenderShader->setVec2("viewportSize", glm::vec2(width, height));

			Shader* pointRender = g_pointShader;
			pointRender->use();
			pointRender->setMat4("projection", ortho);
			pointRender->setMat4("view", view);
			pointRender->setVec2("viewportSize", glm::vec2(width, height));
			pointRender->setFloat("PointSize", 10);

			//绘制Entity实体
			for (EntityVGPU* ent : m_currentSketch.get()->entities)
			{
				if (!ent->isSelected)
				{
					if (showArrow)
					{
						g_showArrowShader->use();
						ent->Paint(g_showArrowShader, ocsSys, RenderMode::Line);
					}
					else
					{
						g_lineShader->use();
						ent->Paint(g_lineShader, ocsSys, RenderMode::Line);
					}
					//g_lineShader->use();
				}
				else
				{
					if (!static_cast<uint64_t>(operationState & (ModalState::EntityRotate | ModalState::EntityScale | ModalState::EntityMirror
						| ModalState::EntityShift)))
					{
						dashedLineRenderShader->use();

						ent->Paint(dashedLineRenderShader, ocsSys, RenderMode::DashedLine);
					}
				}

				if (captureType == CaptureMode::Point)
				{
					pointRender->use();
					ent->Paint(pointRender, ocsSys, RenderMode::Point);
				}
			}

			//绘制空走路径
			for (Path2D* path : m_currentSketch.get()->paths)
			{
				if (path->visiable)
				{
					dashedLineRenderShader->use();
					dashedLineRenderShader->setMat4("model", glm::mat4(1.0f));
					dashedLineRenderShader->setVec4("PaintColor", g_yellowColor);
					dashedLineRenderShader->setFloat("minVisibleLength", 1.0f);
					std::vector<glm::vec3> nodes = path->GetTransformedNodes();
					glBindVertexArray(0);
					glBindBuffer(GL_ARRAY_BUFFER, 0);
					glEnableClientState(GL_VERTEX_ARRAY);
					glVertexPointer(3, GL_FLOAT, sizeof(glm::vec3), nodes.data());
					glDrawArrays(GL_LINES, 0, 2);

					g_showArrowShader->use();
					glm::vec3 dir = nodes[1] - nodes[0];
					glm::vec3 center = (nodes[0] + nodes[1]) / 2.0f;
					std::vector<glm::vec3> arrowPos = { center,center + 0.01f * dir };
					g_showArrowShader->setMat4("projection", ortho);
					g_showArrowShader->setMat4("model", glm::mat4(1.0f));
					g_showArrowShader->setVec4("PaintColor", g_yellowColor);
					glVertexPointer(3, GL_FLOAT, sizeof(glm::vec3), arrowPos.data());
					glDrawArrays(GL_LINES, 0, 2);
					glDisableClientState(GL_VERTEX_ARRAY);
				}
			}
			//绘制捕捉点
			if (captureType == CaptureMode::Point)
			{
				pointRender->use();
				hoverPoint->color = g_yellowColor;
				hoverPoint->Paint(pointRender, ocsSys, RenderMode::Point);
			}

			//绘制辅助线
			for (EntityVGPU* ent : assistantEnt)
			{
				ent->Paint(g_lineShader, ocsSys, RenderMode::Line);
			}

			//绘制模态逻辑
			if (operationState != ModalState::NormalInteract)
			{
				ModalEventDraw(glm::vec2(width, height), ortho, view);
			}

			////绘制坐标刻度
			if (drawTickers)
			{
				DrawTickers();
			}
			//绘制框选框
			if (isSelecting)
			{
				selectBox->Paint();
			}
			
			//绘制刀具锚点
			toolAnchor->Paint();

			glfwSwapInterval(1);
			glfwSwapBuffers(m_window.get());
			glReadPixels(0, 0, m_width, m_height, GL_RGB, GL_UNSIGNED_BYTE, m_windowbuf);
		}
	}
}