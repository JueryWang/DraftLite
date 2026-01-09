#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"

namespace CNCSYS
{
	class OpenGLRenderWindow;
	class CanvasGPU;

	extern CanvasGPU* g_canvasInstance;
	extern GLuint circleMaskTex;
	extern glm::vec4 g_whiteColor;
	extern glm::vec4 g_yellowColor;
	extern glm::vec4 g_highlightColor;
	extern glm::vec4 g_redColor;
	extern glm::vec4 g_greenColor;
	extern glm::vec4 g_darkGreen;
	extern Shader* g_mirrorShader;
	extern Shader* g_lineShader;
	extern Shader* g_dashedLineShader;
	extern Shader* g_pointShader;
	extern Shader* g_showArrowShader;
	extern float g_lineWidth;

	enum WindowState
	{
		DYNAMIC_DRAW = 0,			//鼠标激活的当前窗口(实时渲染)
		STATIC_DRAW = 1,			//静态激活窗口(只渲染当前一帧)
		INACTIVE = 2				//未激活窗口(未销毁,但是隐藏的窗口)
	};

	enum class RenderMode
	{
		Line,
		Point,
		DashedLine,
		ShowArrow,
		DrawTickers,
		Mirror
	};

	static int g_RenderWindowID = 0;

	struct OpenGLWindowContext
	{
	public:
		OpenGLWindowContext(OpenGLRenderWindow* _window, WindowState _state) : window(_window), state(_state), id(g_RenderWindowID++)
		{

		}

		~OpenGLWindowContext()
		{
			delete window;
		}

		bool operator==(const struct OpenGLWindowContext& other)
		{
			return id == other.id;
		}

		int id;
		OpenGLRenderWindow* window = nullptr;
		WindowState state;
	};

	bool InitializeOpenGL();
}
