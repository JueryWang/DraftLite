#include "Auth/CounterDownManager.h"
#include "Common/ProgressInfo.h"
#include <QCoreApplication>
#include <QApplication>
#include <QQmlContext>
#include <QQuickWindow>
#include <QDateTime>
#include <QTimeZone>
#include <QUrl>
#include <QBuffer>
#include <QQmlComponent>

const QString PopUpHintQmlSource = R"(
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Rectangle {
    id: mainWindow
    visible: true
    width: 380
    height: 220
    color: "transparent"

    signal popUpClose() 
    function onReload()
    {
        unauthorizedPopup.startWarning(5);
    }
    
    function formatTime(totalSeconds) {
        // 1. 确保输入是数字且非负
        if (typeof totalSeconds !== 'number' || totalSeconds < 0) {
            return "00:00";
        }
        
        // 2. 核心算法：计算分钟和秒
        var minutes = Math.floor(totalSeconds / 60);
        var seconds = totalSeconds % 60;
    
        var formattedMinutes = String(minutes).padStart(2, '0');
        var formattedSeconds = String(seconds).padStart(2, '0');
    
        return formattedMinutes + ":" + formattedSeconds;
        }
        
    // --- 自定义警告 PopUp 组件 ---
    Popup {
        id: unauthorizedPopup
        
        // --- 属性设置 ---
        property int totalSeconds: 5
        property int currentSeconds: 5
        
        // 设置弹窗位置（居中显示更具强制性）
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 380
        height: 220
        modal: true // 模态窗口，阻止用户操作背景
        closePolicy: Popup.NoAutoClose // 禁止手动关闭

        // 自定义背景（深红色警示风格）
        background: Rectangle {
            color: "#AA3300" // 深红色背景
            border.color: "#FF5555"
            border.width: 2
            radius: 8
            // 添加一个微妙的阴影
            layer.enabled: true
        }

        // --- 弹窗内容布局 ---
        contentItem: Item {
            // 1. 左上角的警告图标 (使用 Unicode 字符代替图片，方便运行)
            Text {
                text: "⚠️"
                font.pixelSize: 32
                color: "#FFD700" // 金黄色图标
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 5
                anchors.leftMargin: 5
            }

            // 2. 主要文本内容区域 (垂直排列)
                ColumnLayout { // 👈 关键改动：从 RowLayout 改为 ColumnLayout
                anchors.centerIn: parent
                spacing: 20
                
                // 提示信息文字
                Text {
                    // Layout.maximumWidth: parent.width * 0.9 (在 ColumnLayout 中不再必需，但可以保留作为最大宽度限制)
                    // 移除 Layout.fillWidth: true
                    
                    text: "警告：由于您是未授权用户，\n软件将在倒计时结束后自动关闭。"
                    color: "white"
                    font.pixelSize: 16
                    font.bold: true
                    wrapMode: Text.WordWrap
                    
                    // 垂直布局时，内容需要水平居中
                    horizontalAlignment: Text.AlignHCenter 
                    Layout.alignment: Qt.AlignHCenter // 确保 Text 组件在 ColumnLayout 中居中
                }

                // 3. 巨大、加粗、半透明的倒计时数字
                Text {
                    id: countdownNumberText
                    
                    // 确保数字在 ColumnLayout 中居中
                    Layout.alignment: Qt.AlignHCenter 
                    
                    // 将毫秒转换为 "M:SS" 格式 (避免调用 remainTime() 函数)
                    text: formatTime(countdownTimer.remainTime)
                    color: "white"
                    font.pixelSize: 80  // 加大
                    font.bold: true     // 加粗
                    opacity: 0.3        // 半透明
                    
                    // 可选：添加一个简单的缩放动画让数字跳动
                    Behavior on text {
                        SequentialAnimation {
                            NumberAnimation { 
                                target: countdownNumberText
                                property: "scale"
                                from: 1.2
                                to: 1.0
                                duration: 200
                                easing.type: Easing.OutQuad 
                            }
                        }
                    }
                }
            }
        }

        // --- 核心逻辑控制器 ---

        // 定时器：负责每秒更新数字
        Timer {
            id: countdownTimer
            interval: 1000 // 1秒间隔
            repeat: true
            property int remainTime: counterDownManager.remainTime

            onTriggered: {
                unauthorizedPopup.currentSeconds -= 1;
                remainTime -= 1;    
                // 当倒计时只剩 1 秒时，开始执行渐隐动画
                if (unauthorizedPopup.currentSeconds === 1) {
                    finalFadeOutAnim.start();
                }
                
                // 倒计时结束 (实际上在变为0之前动画已经接管了)
                if (unauthorizedPopup.currentSeconds <= 0) {
                    countdownTimer.stop();
                }
            }
        }

        // 最终的渐隐退出动画 (耗时 1 秒)
        SequentialAnimation {
            id: finalFadeOutAnim
            // 在 1 秒内将整个弹窗透明度变为 0
            NumberAnimation {
                target: unauthorizedPopup
                property: "opacity"
                to: 0.0
                duration: 1000
                easing.type: Easing.InQuad
            }
            // 动画完成后执行退出软件
            ScriptAction {
                script: {
                    mainWindow.visible = false
                    mainWindow.popUpClose();
                }
            }
        }

        // --- 对外接口函数 ---
        function startWarning(seconds) {
            // 重置状态
            totalSeconds = seconds;
            currentSeconds = seconds;
            opacity = 1.0; // 确保开始时是不透明的
            countdownTimer.stop();
            finalFadeOutAnim.stop();
            
            // 打开弹窗并启动定时器
            open();
            countdownTimer.start();
        }
    }

    Component.onCompleted: {
        counterDownManager.reloadSignal.connect(mainWindow.onReload);
    }
}
)";

CounterDownManager::CounterDownManager(QObject* parent)
	: QObject(parent)
{
	initTime = QDateTime::currentDateTime();
	endTime = initTime.addSecs(30 * 60);

	countdownTimer = new QTimer(this);
	countdownTimer->setInterval(30 * 60 * 1000); // 每秒更新一次
	connect(countdownTimer, &QTimer::timeout, this, &CounterDownManager::updateTime);
	countdownTimer->start();

	viewer = new QQuickView();
	Qt::WindowFlags flags = Qt::Window; // 基础窗口标志
	flags |= Qt::FramelessWindowHint;   // 无边框
	flags |= Qt::WindowStaysOnTopHint;  // 始终置顶
	viewer->setFlags(flags);
	QQmlComponent* component = new QQmlComponent(viewer->engine());
	component->setData(PopUpHintQmlSource.toUtf8(), QUrl());
	viewer->engine()->rootContext()->setContextProperty("counterDownManager", this); // 传递上下文属性

	if (component->isError()) {
		for (const auto& err : component->errors()) {
			qWarning() << "QML 错误：" << err;
		}
		return;
	}

	qmlRoot = component->create();
	connect(qmlRoot, SIGNAL(popUpClose()), this, SLOT(hidePopUp()));
	// 3. 将 Rectangle 设置为 QQuickView 的根对象
	viewer->setContent(QUrl(), component, qmlRoot);
	viewer->show();
	emit reloadSignal();
}

CounterDownManager::~CounterDownManager()
{

}

qint64 CounterDownManager::remainTime() const
{
	QDateTime currentTime = QDateTime::currentDateTime();
	// 计算从现在到 endTime 的毫秒数（正值表示剩余时间）
	qint64 ms = currentTime.secsTo(endTime);
	if (ms < 0) ms = 0;
	return ms;
}

void CounterDownManager::hidePopUp()
{
	viewer->hide();
}

QString CounterDownManager::remainTimeFormatted() const
{
	qint64 totalSeconds = remainTime();
	qint64 minutes = totalSeconds / 60;
	qint64 seconds = totalSeconds % 60;
	QString res = QString("%1:%2")
		.arg(minutes, 2, 10, QChar('0'))
		.arg(seconds, 2, 10, QChar('0'));

	return res;
}

Q_INVOKABLE void CounterDownManager::updateTime()
{
	bool auhtorized = CheckAuth(false);
	if (auhtorized)
	{
		countdownTimer->stop();
	}
	if (remainTime() > 0)
	{
		emit reloadSignal();
		viewer->show();
	}
	else
	{
		QApplication::exit();
	}

}