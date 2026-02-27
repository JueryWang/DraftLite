#include "UI/AboutVersion.h"
#include <QPixmap>
#include <QVBoxLayout>
#include "UI/Components/HmiInterfaceDefines.h"
#include <QDateTime>
#include <QApplication>

AboutVersion::AboutVersion()
{
	QVBoxLayout* vlay = new QVBoxLayout();
	this->setWindowTitle("WG DraftLite");
	poster = new QLabel(this);
	QPixmap posterPixmap("./Resources/icon/WG-poster.png");
	if (posterPixmap.isNull()) {
		poster->setText("Í¼±ê¼ÓÔØÊ§°Ü!");
	}
	else
	{
		int width = (580 * screen_resolution_x / 1920.0);
		int height = (260 * screen_resolution_y / 1080.0);
		QPixmap scaledPixmap = posterPixmap.scaled(
			width,
			height,
			Qt::KeepAspectRatioByExpanding,
			Qt::SmoothTransformation
		);
		poster->setPixmap(scaledPixmap);
	}
	vlay->addWidget(poster);

	textDescription = new QLabel();
	QDateTime utcTime = QDateTime::currentDateTimeUtc();
	QString versionMessage = QString("WG DraftLite 2026\nVersion:2025.0.1.250035017\nBuildDate:%1\nStatus:Full Release\n\n(c) Copyright 2025 WG Automation, Inc. All rights reserved.").arg(utcTime.toString("yyyy-MM-dd UTC HH:mm:ss"));
	textDescription->setText(versionMessage);

	textArea = new QScrollArea();
	textArea->setStyleSheet(R"(
        QScrollArea {
            border: 1px solid #1E90FF;
            padding: 8px;
        }
    )");
	textArea->setStyle(QApplication::style());
	textArea->setWidget(textDescription);
	textDescription->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	vlay->addWidget(textArea);

	this->setFixedSize((580 * screen_resolution_x / 1920.0), (460 * screen_resolution_y / 1080.0));
	this->setLayout(vlay);
}

AboutVersion::~AboutVersion()
{

}
