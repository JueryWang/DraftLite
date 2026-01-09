#pragma once

#include "UI/Components/HmiInterfaceDefines.h"
#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <functional>
#include <QLabel>

class HmiTemplateMsgBox : public QDialog {
	Q_OBJECT
public:
	HmiTemplateMsgBox(QWidget* parent, const QString& mainMsg, const QString& attachedMsg);
	~HmiTemplateMsgBox();
	static HmiTemplateMsgBox* question(QWidget* parent, const QString& title, const QString& text, const QStringList& btnTexts, const std::vector<std::function<void(void)>>& callbacks);
	static HmiTemplateMsgBox* warning(QWidget* parent, const QString& title, const QString& text, const QStringList& btnTexts, const std::vector<std::function<void(void)>>& callbacks);
	static HmiTemplateMsgBox* success(QWidget* parent, const QString& title, const QString& text, const QStringList& btnTexts, const std::vector<std::function<void(void)>>& callbacks);
	void setTitle(const QString& title);
	void setContent(const QString& content);
	void setButtonTexts(const QStringList& texts);
	void setCallbacks(const std::vector<std::function<void(void)>>& callbacks);
	void showEvent(QShowEvent* event) override;
private:
	void on_clcBtn1();
	void on_clcBtn2();
	void on_clcBtn3();
	bool eventFilter(QObject* obj, QEvent* event) override;
private:
	QWidget* handler;
	QLabel* icon;

	QPushButton* btn1;
	QPushButton* btn2;
	QPushButton* btn3;

	std::function<void(void)> cb1;
	std::function<void(void)> cb2;
	std::function<void(void)> cb3;
	bool isDraging = false;
	QPoint offsetPoint = QPoint(0, 0);
	int exec_res = -1;

	QLabel* maindisp = nullptr;
	QLabel* attachedDisp = nullptr;
};