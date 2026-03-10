#include "Common/OpenGLContext.h"
#include <random>

namespace CNCSYS
{
	GLuint circleMaskTex;
	CanvasGPU* g_canvasInstance = nullptr;
	Shader* g_mirrorShader = nullptr;
	Shader* g_lineShader = nullptr;
	Shader* g_dashedLineShader = nullptr;
	Shader* g_pointShader = nullptr;
	Shader* g_showArrowShader = nullptr;

	glm::vec4 g_whiteColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	glm::vec4 g_redColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	glm::vec4 g_greenColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 g_darkGreen = glm::vec4(0.06f, 0.28f, 0.06f, 1.0f);
	glm::vec4 g_yellowColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	glm::vec4 g_workBlankColor = glm::vec4(0.878f,0.640f,0.459f,1.0f);
	glm::vec4 g_highlightColor = glm::vec4(171.0f / 255.0f, 228.0f / 255.0f, 243.0f / 255.0f, 1.0f);

	float g_lineWidth = 2.0f;

	bool InitializeOpenGL()
	{
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		GLFWwindow* driverTest = glfwCreateWindow(50, 50, "", NULL, NULL);
		if (driverTest == NULL)
		{
			glfwTerminate();
			glfwDestroyWindow(driverTest);
			return false;
		}
		glfwDestroyWindow(driverTest);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		return true;
	}
	glm::vec4 GetRandomColor()
	{
		static std::random_device rd;
		static std::mt19937 gen(rd());

		// 2. ����ֲ���Χ [0.0, 1.0)
		// �����Ҫ���� 1.0������ʹ�� std::nextafter(1.0, 2.0)
		std::uniform_real_distribution<double> dis(0.0, 1.0);

		return glm::vec4(dis(gen), dis(gen), dis(gen), 1.0f);
	}
}
