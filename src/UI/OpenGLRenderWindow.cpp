#include "UI/OpenGLRenderWindow.h"
#include "Graphics/Sketch.h"
#include "UI/SizeDefines.h"

namespace CNCSYS
{
	int bit_per_pixel = 3;

	uchar* opengl_swap_buffer = nullptr;

	GLuint createCircleTexture(int size = 64, float radius = 0.5f);

	OpenGLRenderWindow::OpenGLRenderWindow(int width, int height, const char* title) : m_width(width), m_height(height)
	{
		if (opengl_swap_buffer == nullptr)
		{
			opengl_swap_buffer = (uchar*)malloc(m_width * m_height * bit_per_pixel);
		}
		if (m_windowbuf == nullptr)
		{
			m_windowbuf = (uchar*)malloc(m_width * m_height * bit_per_pixel);
		}

		GLFWwindow* glfwWindow = glfwCreateWindow(width, height, title, NULL, NULL);
		if (glfwWindow == nullptr)
		{
			glfwTerminate();
			return;
		}
		m_window.reset(glfwWindow);
		glfwMakeContextCurrent(m_window.get());
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "No OpenGL Driver" << std::endl;
		}
		glEnable(GL_PROGRAM_POINT_SIZE);  // 允许在顶点着色器中设置点大小
		glEnable(GL_POINT_SPRITE);        // 启用点精灵
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		circleMaskTex = createCircleTexture();
	}

	OpenGLRenderWindow::~OpenGLRenderWindow()
	{
		m_window.reset();
		free(m_windowbuf);
		free(opengl_swap_buffer);
	}

	void OpenGLRenderWindow::SetReciverWidget(GLWidget* widget)
	{
		m_reciver = widget;
	}

	void OpenGLRenderWindow::Resize(const QSize& size)
	{
		m_width = size.width(); m_height = size.height();
		glfwSetWindowSize(m_window.get(), m_width, m_height);
	}

	QImage OpenGLRenderWindow::grabImage()
	{
		return QImage(m_windowbuf, m_width, m_height, QImage::Format_RGB888).mirrored(false, true);
	}

	GLuint createCircleTexture(int size, float radius)
	{
		GLuint texID;
		glGenTextures(1, &texID);
		glBindTexture(GL_TEXTURE_2D, texID);

		// 设置纹理参数（避免边缘锯齿）
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		// 生成圆形纹理数据（单通道，存储Alpha值）
		std::vector<unsigned char> data(size * size, 0);
		float center = size / 2.0f;  // 纹理中心坐标
		float maxDistSq = radius * size * radius * size;  // 最大距离平方（避免开方）

		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				// 计算当前像素到中心的距离平方
				float dx = x - center;
				float dy = y - center;
				float distSq = dx * dx + dy * dy;

				// 距离越近，Alpha值越高（0~255）
				if (distSq <= maxDistSq) {
					float alpha = 1.0f - sqrt(distSq) / (radius * size);  // 边缘渐变
					data[y * size + x] = static_cast<unsigned char>(alpha * 255);
				}
				else {
					data[y * size + x] = 0;  // 圆形外透明
				}
			}
		}

		// 将数据上传到GPU纹理
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RED,  // 单通道（RED对应Alpha）
			size, size, 0, GL_RED, GL_UNSIGNED_BYTE,
			data.data()
		);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		return texID;
	}
}