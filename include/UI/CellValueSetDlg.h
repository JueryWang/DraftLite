#pragma once
#include "Common/AtomicVariables.h"
#include <UI/Components/HmiTemplateLabel.h>
#include <QLineEdit>
#include <string>

class CellValueSetDlg : public QWidget
{
public:
	CellValueSetDlg();
	~CellValueSetDlg();

	void SetValueTag(const std::string& tag);

private:
	void closeEvent(QCloseEvent* event) override;
	void showEvent(QShowEvent* event) override;

public:
	AtomicVarType currentHandleType = AtomicVarType::None;

private:
	std::string bindTag;
	HmiTemplateLabel* labelCurValue = nullptr;
	QLineEdit* inputEdit = nullptr;
};