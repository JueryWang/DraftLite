#include "UI/Components/HmiTemplateWebViewer.h"
#include <QQuickWindow>
#include <QQuickWidget>
#include <QEvent>
#include <QApplication>

QString HmiTemplateWebViewer::url;
QObject* HmiTemplateWebViewer::webObject;

const QString qmlSource = R"(
import QtCore
import QtQml
import QtQuick
import QtQuick.Controls.Fusion
import QtQuick.Layouts
import QtQuick.Window
import QtWebEngine

Window {
    visible: true          
    width: 1024
    height: 768

    
    // 定义WebView组件
    Component {
        id: tabComponent
        WebEngineView {
            id: webEngineView
            objectName: "WebEngineView"
            focus: true
            url: "http://192.168.6.6:8080/webvisu.htm"
            anchors.fill: parent

            states: [
                State {
                    name: "FullScreen"
                    PropertyChanges {
                        target: tabBar
                        visible: false
                        height: 0
                    }
                    PropertyChanges {
                        target: navigationBar  // navigationBar需在外部定义
                        visible: false
                    }
                }
            ]

            // WebEngineView设置（appSettings需提前定义，否则会报错）
            settings.localContentCanAccessRemoteUrls: true
            settings.localContentCanAccessFileUrls: false
            // 暂时注释依赖appSettings的配置（避免未定义导致的错误）
            // settings.autoLoadImages: appSettings.autoLoadImages
            // settings.javascriptEnabled: appSettings.javaScriptEnabled
            // ... 其他依赖appSettings的配置同理

            // 重新加载定时器
            Timer {
                id: reloadTimer
                interval: 0
                running: false
                repeat: false
                onTriggered: webEngineView.reload()  // 修正为当前WebView的id
            }
        }
    }

    // 实例化WebView组件（否则组件仅定义不显示）
    Loader {
        id: webLoader
        sourceComponent: tabComponent  // 加载上面定义的tabComponent
        anchors.fill: parent           // 充满整个窗口
    }
}
)";

void HmiTemplateWebViewer::SetUrl(const QString _url)
{
	HmiTemplateWebViewer::url = _url;
}

QWidget* HmiTemplateWebViewer::GetWidget()
{
	QQuickWidget* quickWidget = new QQuickWidget();
	QQmlEngine* engine = quickWidget->engine();

	QQmlComponent* conponent = new QQmlComponent(engine);
	conponent->setData(qmlSource.toUtf8(), QUrl());

	if (quickWidget)
	{
		quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
		quickWidget->setSource(QUrl("./plugins/WebViewer.qml"));
		quickWidget->hide();
	}

	return quickWidget;
}

void HmiTemplateWebViewer::Reload()
{

}
