#pragma once
#include <QWidget>
#include <QList>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QContextMenuEvent>
#include <QTimer>
#include <QAction>
#include <variant>
#include <vector>
#include "Common/CraftParamConfig.h"
#include "Controls/GlobalPLCVars.h"

class ConfigVariablesPage;

namespace CNCSYS
{
	class SketchGPU;
}

class ConfigVariablesItem : public QWidget
{
	Q_OBJECT
public:
	ConfigVariablesItem();
	~ConfigVariablesItem();
	bool ToParamConfig(CraftParamConfig& config);

public slots:
	void OnValidateInput();

private:
	void ParamSelectionChanged(const QString& text);
	bool CheckInputValid();
public:
	static ConfigVariablesPage* configPage;

public:
	QLineEdit* variable;
	QComboBox* varTypeCombo;
	QLineEdit* preSetValue;
	QCheckBox* preSetBool;
	QLineEdit* remarks;
	QTimer* delayTimer = nullptr;
	PLCParam_ProtocalOpc* attachedPLCAdress = nullptr;
	QHBoxLayout* hlay;
	QPushButton* btnDelete;
	QAction* invalidInputAct;
	std::variant<bool, uint16_t, uint32_t, uint64_t, int16_t, int32_t, int64_t, float, double, std::string> setVal;
	bool validInput = true;
};

class ConfigVariablesPage : public QWidget
{
	friend class ConfigVariablesItem;

	Q_OBJECT
public:
	static ConfigVariablesPage* GetInstance();
	void BindSketch(CNCSYS::SketchGPU* sketch);
	CNCSYS::SketchGPU* attachedSketch;

private:
	ConfigVariablesPage();
	~ConfigVariablesPage();

	void AddNewItem();
	void DeleteItem(QListWidgetItem* item);
private slots:
	void OnConfirm();

private:
	static ConfigVariablesPage* s_instance;
	QListWidget* itemLists = nullptr;
	QPushButton* btnAdd = nullptr;
	QPushButton* btnConfirm = nullptr;

	std::vector<ConfigVariablesItem*> itemWidgets;
};