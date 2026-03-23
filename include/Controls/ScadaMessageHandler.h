#pragma once

#include "UI/Components/HmiTemplateMsgBox.h"
#include <mutex>
#include <QPushButton>
#include <QObject>
#include <QWidget>

class OPClient;
class MenuLayerTop;

class ScadaMessageHandler : public QObject
{
	Q_OBJECT
public:
	static ScadaMessageHandler* GetInstance();

public slots:
	void handleOpcDisconnected(OPClient* client);
	void handleOpcConnectSuccess(OPClient* client);
	void handleOpcConnectFailed(OPClient* client);
	void handleOpcReconnectFailed(OPClient* client);
	void handleOpcWriteFailed(OPClient* client,const QString& targetNode,const QString& targetValue,const QString& errorCode);
	void handleDogPullOut();
	void handleAuthNotMatch(MenuLayerTop* menu,const std::string& origin_chipId,const std::string& local_chipId,const std::string& origin_uuid,const std::string& local_uuid);
	void handleResoreHistory();
	void showAuthWindow();
private:
	ScadaMessageHandler();
	~ScadaMessageHandler();

private:
	static ScadaMessageHandler* instance;
	static std::mutex mtx;
	QWidget* AuthInformationWindow;
	QWidget* InputActivationCodeWindow;
	QPushButton* activationBtn;
	QPushButton* showAuthenInfoBtn;
};