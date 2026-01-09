#pragma once

#include "UI/Components/HmiInterfaceDefines.h"
#include <QWidget>
#include <QQmlApplicationEngine>
#include <QtWebEngineQuick>

class HmiTemplateWebViewer
{
public:
	static void SetUrl(const QString url);
	static QWidget* GetWidget();
	static void Reload();

private:
	static QString url;
	static QObject* webObject;

};