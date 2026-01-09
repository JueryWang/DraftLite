#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QCompleter>
#include <QStringListModel>
#include <QStringList>
#include <functional>

class SearchItemDialog : public QWidget
{
public:
	SearchItemDialog(const QStringList& key);
	~SearchItemDialog();

	void SetNewKeys(const QStringList& newKey);
	void SetCallback(const std::function<void(const QString&)>& cb) { this->callback = cb; }

private:
	QStringList items;
	QLineEdit* keyEdit;
	std::function<void(const QString&)> callback;
	QCompleter* completer;
	QStringListModel* model;
};