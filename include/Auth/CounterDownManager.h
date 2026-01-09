#pragma once

#include <QTimer>
#include <QTime>
#include <QQmlApplicationEngine>
#include <QQuickView> 

class CounterDownManager : public QObject
{
	Q_OBJECT
		Q_PROPERTY(qint64 remainTime READ remainTime NOTIFY remainTimeChanged)
		Q_PROPERTY(QString remainTimeFormatted READ remainTimeFormatted NOTIFY remainTimeChanged)

public:
	explicit CounterDownManager(QObject* parent = nullptr);
	~CounterDownManager();

	qint64 remainTime() const;
	QString remainTimeFormatted() const;
	Q_INVOKABLE void updateTime();

signals:
	void remainTimeChanged();
	void reloadSignal();

public slots:
	void hidePopUp();

private:
	QDateTime initTime;
	QDateTime endTime;
	QTimer* countdownTimer = nullptr;
	QQuickView* viewer = nullptr;
	QObject* qmlRoot = nullptr;
};