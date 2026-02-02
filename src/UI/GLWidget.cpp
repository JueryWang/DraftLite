#include "UI/GLWidget.h"
#include "UI/OpenGLRenderWindow.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "Common/OpenGLContext.h"
#include "Graphics/Sketch.h"
#include "Graphics/OCS.h"
#include <QRegularExpression>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>

bool firstShow = true;
using namespace CNCSYS;

std::string double_to_string_trim(double value) {
	std::ostringstream oss;
	oss << std::fixed << value;

	std::string str = oss.str();
	str.erase(str.find_last_not_of('0') + 1, std::string::npos);
	if (str.back() == '.') {
		str.pop_back();
	}
	return str;
}

namespace CNCSYS
{
	GLWidget::GLWidget(OpenGLRenderWindow* glwindow, SketchGPU* sketch, WindowState state)
	{
		setAttribute(Qt::WA_AcceptTouchEvents);
		m_context = new OpenGLWindowContext(glwindow, state);
		setFocusPolicy(Qt::StrongFocus);
		glwindow->setParent(this);
		this->setMouseTracking(true);

		this->setMinimumSize(m_context->window->GetSize());
		m_context->window->SetReciverWidget(this);

		connect(&m_updateTimer, &QTimer::timeout, this, &GLWidget::update);
		this->setFixedSize(glwindow->GetSize());
		this->update();

		font = QFont(global_font_mp["Comic"], 9);

		this->installEventFilter(m_context->window);
	}
	GLWidget::~GLWidget()
	{
		delete m_context;
	}

	void GLWidget::SetWindowStatus(WindowState state)
	{
		m_context->state = state;
		this->update();
	}

	void GLWidget::update()
	{
		if (m_context->state == INACTIVE)
		{
			m_updateTimer.stop();
			return;
		}
		if (m_context->state == STATIC_DRAW)			//OpenGL双缓冲必须运行两次才能得到正确结果
		{
			for (int i = 0; i < 2; i++)
			{
				m_context->window->updateGL();
			}
		}
		if (m_context->state == DYNAMIC_DRAW)
		{
			if (firstShow)
			{
				m_updateTimer.start(0);
				try
				{
					m_context->window->updateGL();
					QMetaObject::invokeMethod(this, "repaint");
				}
				catch (...)
				{

				}
				firstShow = false;
			}

			if (this->isVisible())
			{
				try
				{
					m_context->window->updateGL();
					QMetaObject::invokeMethod(this, "repaint");
				}
				catch (...)
				{

				}
			}
		}
	}

	void GLWidget::paintEvent(QPaintEvent* event)
	{
		QPainter painter;
		painter.begin(this);
		QSize size = m_context->window->GetSize();
		painter.drawImage(QRect(0, 0, size.width(), size.height()), m_context->window->grabImage());
		if (ocsSys)
		{
			QPen pen;
			pen.setColor(QColor(255, 255, 190));
			painter.setPen(pen);
			painter.setFont(font);
			for (auto& ticker : ocsSys->tickers)
			{
				if (ticker.Ttype == TickType::Main)
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
					tickTextCoord.y = 1.0f - tickTextCoord.y;

					tickTextCoord.x *= m_context->window->width();
					tickTextCoord.y *= m_context->window->height();

					QString str = QString::number(ticker.value, 'f', 3);
					QRegularExpression trailingZeros("(?<=\\.)0+$");
					str.replace(trailingZeros, "");

					QRegularExpression trailingDot("\\.$");
					str.replace(trailingDot, "");
					painter.drawText(tickTextCoord.x, tickTextCoord.y, str);
				}
			}

			pen.setColor(QColor(255,255,255));
			QFont font = painter.font();
			font.setPointSize(20);
			font.setBold(true);
			painter.setFont(font);
			OCSGPU* ocsSys  = g_canvasInstance->GetOCSSystem();

			for (Tag& tag : canvasTags)
			{
				glm::vec2 pixelPos = ocsSys->GetPixelPosWithOCSPos(tag.pixelPos);
				painter.setPen(pen);
				painter.drawText(pixelPos.x,pixelPos.y,tag.label);
			}
		}
		painter.end();
	}
}
