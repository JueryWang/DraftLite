#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <qt_windows.h>                 
#include <QApplication>
#include <QLineEdit>
#include <QFile>
#include <QScreen>
#include <thread>
#include "Controls/ScadaScheduler.h"
#include "Common/OpenGLContext.h"
#include "Common/ProgressInfo.h"
#include "UI/GLWidget.h"
#include "Graphics/Canvas.h"
#include "Graphics/Primitives.h"
#include "UI/OverallWindow.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "UI/Components/HmiTemplateMonitorTool.h"
#include "UI/ToDoListItem.h"
#include "UI/TaskListWindow.h"
#include "UI/DigitalHUD.h"
#include "UI/TaskFlowGuide.h"
#include "UI/CanvasGuide.h"
#include "Controls/GlobalPLCVars.h"
#include "NetWork/OPClient.h"
#include "NetWork/FtpClient.h"
#include "IO/ExcelProcessor.h"
#include <QMessageBox>
#include <QFile>
#include <QHboxLayout>
#include <QQmlApplicationEngine>
#include <QtWebEngineQuick>
#include <QProcessEnvironment>
#include <UI/Components/HmiTemplateCraftConfig.h>
#include <algorithm>
#include <string>
#include <Windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "Dbghelp.lib")
using namespace CNCSYS;

bool containsChinese(const QString& str)
{
	// 中文 Unicode 编码范围（覆盖常用场景）
	const uint basicStart = 0x4E00;    // 基本汉字区起始
	const uint basicEnd = 0x9FFF;      // 基本汉字区结束
	const uint extAStart = 0x3400;     // 扩展区A起始
	const uint extAEnd = 0x4DBF;       // 扩展区A结束
	const uint extBStart = 0x20000;    // 扩展区B起始
	const uint extBEnd = 0x2A6DF;      // 扩展区B结束
	// 可根据需要添加更多扩展区（如 extC/extD/extE/extF）

	for (const QChar& ch : str) {
		uint unicode = ch.unicode(); // 获取字符的 Unicode 编码
		// 判断是否在中文编码区间
		if ((unicode >= extAStart && unicode <= extAEnd) ||
			(unicode >= basicStart && unicode <= basicEnd) ||
			(unicode >= extBStart && unicode <= extBEnd)) {
			return true;
		}
	}
	return false;
}

LONG WINAPI CrashHandler(EXCEPTION_POINTERS* pException) {
	// 创建minidump文件
	HANDLE hFile = CreateFile(L"crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION expInfo;
		expInfo.ThreadId = GetCurrentThreadId();
		expInfo.ExceptionPointers = pException;
		expInfo.ClientPointers = TRUE;
		// 生成minidump
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &expInfo, NULL, NULL);
		CloseHandle(hFile);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

int main(int argc, char* argv[])
{
	SetUnhandledExceptionFilter(CrashHandler);

	QApplication app(argc, argv);

	QString appDir = QCoreApplication::applicationDirPath();
	QString webEngineResPath = appDir + "/plugins/qml";

	webEngineResPath.replace('\\', '/');

	qputenv("QTWEBENGINE_RESOURCES_PATH", webEngineResPath.toUtf8());

	qDebug() << "QTWEBENGINE_RESOURCES_PATH:" << webEngineResPath;

	QByteArray qmlPath = "./plugins/qml";
	if (qgetenv("QML2_IMPORT_PATH").isDetached()) {
		qmlPath = qgetenv("QML2_IMPORT_PATH") + ";" + qmlPath;
	}
	qputenv("QML2_IMPORT_PATH", qmlPath);

	// 2. 检测目录是否包含中文字符
	bool hasChinese = containsChinese(appDir);
	if (hasChinese) {
		QMessageBox::critical(NULL, QStringLiteral("错误"), QStringLiteral("检测到中文路径,程序运行可能失败,请手动修改为英文"));
	}


	QtWebEngineQuick::initialize();
	InitUIEnvironment();
	InitProgressContext();

	OverallWindow* window = new OverallWindow();
	window->show();
	TaskListWindow::GetInstance()->setParent(window);
	TaskListWindow::GetInstance()->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);

	TaskFlowGuide* guide = new TaskFlowGuide(window);
	guide->show();

	ScadaScheduler::GetInstance()->Start();

	return app.exec();
}