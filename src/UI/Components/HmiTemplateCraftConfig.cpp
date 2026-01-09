#include "UI/Components/HmiTemplateCraftConfig.h"
#include "Common/ProgressInfo.h"
#include "NetWork/MessageValidtor.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include "Graphics/Sketch.h"
#include <QMenu>
#include <QLabel>
#include <QHBoxLayout>
#include <QCompleter>
#include <QStringListModel>
#include <QMessageBox>

ConfigVariablesPage* ConfigVariablesItem::configPage = nullptr;
ConfigVariablesPage* ConfigVariablesPage::s_instance = nullptr;

ConfigVariablesItem::ConfigVariablesItem()
{
	hlay = new QHBoxLayout();

	variable = new QLineEdit();
	variable->setObjectName("CraftConfig");
	variable->setPlaceholderText(tr("预设变量.."));

	QStringList options;
	for (auto& key : g_ConfigableKeys)
	{
		options << QString::fromStdString(key.first);
	}
	QStringListModel* model = new QStringListModel(options);
	QCompleter* completer = new QCompleter(model, variable);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setFilterMode(Qt::MatchContains);
	variable->setCompleter(completer);
	connect(variable, &QLineEdit::textChanged, this, &ConfigVariablesItem::ParamSelectionChanged);

	hlay->addWidget(variable);
	varTypeCombo = new QComboBox();
	varTypeCombo->setObjectName("CraftConfig");
	QStringList varTypes = { "BOOL", "WORD","DWORD","LWORD","INT","DINT","LINT","REAL","LREAL","STRING" };
	varTypeCombo->insertItems(0, varTypes);
	varTypeCombo->setCurrentIndex(2);
	hlay->addWidget(varTypeCombo);
	preSetValue = new QLineEdit();
	preSetValue->setObjectName("CraftConfig");
	hlay->addWidget(preSetValue);
	preSetValue->setPlaceholderText(tr("预设值.."));
	connect(preSetValue, &QLineEdit::editingFinished, this, &ConfigVariablesItem::OnValidateInput);
	preSetBool = new QCheckBox();
	preSetBool->setObjectName("CraftConfig");
	remarks = new QLineEdit();
	remarks->setPlaceholderText(tr("备注.."));
	remarks->setObjectName("CraftConfig");
	hlay->addWidget(remarks);

	btnDelete = new QPushButton(tr("删除"));
	btnDelete->setFixedSize(40, 25);
	btnDelete->setObjectName("CraftConfig");
	hlay->addWidget(btnDelete);
	connect(btnDelete, &QPushButton::clicked, [&]() {
		QListWidget* listWidget = ConfigVariablesItem::configPage->itemLists;
		QPoint posInViewport = this->mapToParent(QPoint(0, 0));
		QListWidgetItem* item = listWidget->itemAt(posInViewport);
		ConfigVariablesItem::configPage->DeleteItem(item);
		});

	hlay->setContentsMargins(10, 0, 10, 0);
	hlay->setSpacing(10);
	this->setLayout(hlay);

	delayTimer = new QTimer(this);
	delayTimer->setSingleShot(true);
	delayTimer->setInterval(500);

	invalidInputAct = new QAction(this);
	invalidInputAct->setIcon(QIcon(ICOPATH(InputWarning.png)));
	invalidInputAct->setIconVisibleInMenu(false);
	preSetValue->addAction(invalidInputAct, QLineEdit::TrailingPosition);
	invalidInputAct->setVisible(false);
}

void ConfigVariablesItem::OnValidateInput()
{
	QString content = preSetValue->text();
	bool isValid = CheckInputValid();

	if (!isValid) {
		preSetValue->setProperty("validation_error", true);
		preSetValue->style()->unpolish(preSetValue);
		preSetValue->style()->polish(preSetValue);
		invalidInputAct->setVisible(true);

		preSetValue->setToolTip(tr("输入值不符合当前类型"));
	}
	else
	{
		preSetValue->setProperty("validation_error", false);
		preSetValue->style()->unpolish(preSetValue);
		preSetValue->style()->polish(preSetValue);
		invalidInputAct->setVisible(false);

		preSetValue->setToolTip(tr(""));
	}
}

ConfigVariablesItem::~ConfigVariablesItem()
{
	delete variable;
	delete varTypeCombo;
	delete preSetValue;
	delete preSetBool;
	delete remarks;
	delete btnDelete;
	delete delayTimer;
	delete invalidInputAct;
}

bool ConfigVariablesItem::ToParamConfig(CraftParamConfig& config)
{
	config.alias = variable->text().toStdString();
	config.plcInfo = attachedPLCAdress;
	switch (attachedPLCAdress->dataType)
	{
	case AtomicVarType::BOOL:
	{
		config.preSetVal = preSetBool->isChecked();
		break;
	}
	case AtomicVarType::WORD:
	{
		uint16_t value;
		if (stringToUint16(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		else
		{
			return false;
		}
		break;
	}
	case AtomicVarType::DWORD:
	{
		uint32_t value;
		if (stringToUint32(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		else
		{
			return false;
		}
	}
	case AtomicVarType::LWORD:
	{
		uint64_t value;
		if (stringToUint64(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		else
		{
			return false;
		}
	}
	case AtomicVarType::INT:
	{
		int16_t value;
		if (stringToInt16(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		else
		{
			return false;
		}
		break;
	}
	case AtomicVarType::DINT:
	{
		int32_t value;
		if (stringToInt32(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		else
		{
			false;
		}
		break;
	}
	case AtomicVarType::LINT:
	{
		int64_t value;
		if (stringToInt64(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		else
		{
			return false;
		}
		break;
	}
	case AtomicVarType::REAL:
	{
		float value;
		if (stringToFloat(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		break;
	}
	case AtomicVarType::LREAL:
	{
		double value;
		if (stringToDouble(preSetValue->text(), value))
		{
			config.preSetVal = value;
		}
		break;
	}
	case AtomicVarType::STRING:
	{
		config.preSetVal = preSetValue->text().toStdString();
		break;
	}
	default:
		break;
	}
	config.remark = remarks->text().toStdString();
	return true;
}


void ConfigVariablesItem::ParamSelectionChanged(const QString& text)
{
	varTypeCombo->setEnabled(true);
	connect(delayTimer, &QTimer::timeout, [=]()
		{
			std::string identifier = g_ConfigableKeys[text.toStdString()];
			if (identifier.empty())
			{
				return;
			}
			for (auto& pair : g_PLCVariables)
			{
				PLCParam_ProtocalOpc* plcInfo = static_cast<PLCParam_ProtocalOpc*>(pair.second);
				if (plcInfo)
				{
					if (identifier == pair.first)
					{
						attachedPLCAdress = plcInfo;
						varTypeCombo->setCurrentIndex(static_cast<int>(plcInfo->dataType) - 1);
						varTypeCombo->setEnabled(false);
						if (varTypeCombo->currentIndex() == 0)
						{
							preSetValue->hide();
							preSetBool->show();
							hlay->replaceWidget(preSetValue, preSetBool);
						}
						else
						{
							preSetValue->show();
							preSetBool->hide();
							hlay->replaceWidget(preSetBool, preSetValue);
						}
						break;
					}
				}
			}
			disconnect(delayTimer, &QTimer::timeout, nullptr, nullptr);
		});

	delayTimer->stop();
	delayTimer->start();
}

bool ConfigVariablesItem::CheckInputValid()
{
	bool valid = true;
	AtomicVarType type = static_cast<AtomicVarType>(varTypeCombo->currentIndex() + 1);
	if (preSetBool->isVisible())
	{
		setVal = preSetBool->isChecked();
		return true;
	}
	const QString input = preSetValue->text();

	switch (type)
	{
	case AtomicVarType::WORD:
	{
		uint16_t value;
		valid = stringToUint16(input, value);
		setVal = value;
		break;
	}
	case AtomicVarType::DWORD:
	{
		uint32_t value;
		valid = stringToUint32(input, value);
		setVal = value;
		break;
	}
	case AtomicVarType::LWORD:
	{
		uint64_t value;
		valid = stringToUint64(input, value);
		setVal = value;
		break;
	}
	case AtomicVarType::INT:
	{
		int16_t value;
		valid = stringToInt16(input, value);
		setVal = value;
		break;
	}
	case AtomicVarType::DINT:
	{
		int32_t value;
		valid = stringToInt32(input, value);
		setVal = value;
		break;
	}
	case AtomicVarType::LINT:
	{
		int64_t value;
		valid = stringToInt64(input, value);
		setVal = value;
		break;
	}
	case AtomicVarType::REAL:
	{
		float value;
		valid = stringToFloat(input, value);
		setVal = value;;
		break;
	}
	case AtomicVarType::LREAL:
	{
		double value;
		valid = stringToDouble(input, value);
		setVal = value;
		break;
	}
	case AtomicVarType::STRING:
	{
		valid = true;
		setVal = input.toStdString();
		break;
	}
	default:
		break;
	}

	return valid;
}

ConfigVariablesPage* ConfigVariablesPage::GetInstance()
{
	if (ConfigVariablesPage::s_instance == nullptr)
	{
		ConfigVariablesPage::s_instance = new ConfigVariablesPage();
	}

	return ConfigVariablesPage::s_instance;
}

void ConfigVariablesPage::BindSketch(CNCSYS::SketchGPU* sketch)
{
	attachedSketch = sketch;
	while (itemLists->count())
	{
		DeleteItem(itemLists->item(0));
	}
	for (int i = 0; i < sketch->attachedConfig.size(); i++)
	{
		CraftParamConfig& config = sketch->attachedConfig[i];

		QListWidgetItem* itemNew = new QListWidgetItem();
		itemNew->setSizeHint(QSize(this->width(), 50));
		ConfigVariablesItem* configItem = new ConfigVariablesItem();
		configItem->variable->setText(QString::fromStdString(config.alias));
		configItem->varTypeCombo->setCurrentIndex(static_cast<int>(config.plcInfo->dataType) - 1);
		if (config.plcInfo->dataType == AtomicVarType::BOOL)
		{
			configItem->preSetBool->setChecked(std::get<bool>(config.preSetVal));
		}
		else if (config.plcInfo->dataType == AtomicVarType::WORD)
		{
			configItem->preSetValue->setText(QString::number(std::get<uint16_t>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::DWORD)
		{
			configItem->preSetValue->setText(QString::number(std::get<uint32_t>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::LWORD)
		{
			configItem->preSetValue->setText(QString::number(std::get<uint64_t>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::INT)
		{
			configItem->preSetValue->setText(QString::number(std::get<int16_t>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::DINT)
		{
			configItem->preSetValue->setText(QString::number(std::get<int32_t>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::LINT)
		{
			configItem->preSetValue->setText(QString::number(std::get<int64_t>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::REAL)
		{
			configItem->preSetValue->setText(QString::number(std::get<float>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::LREAL)
		{
			configItem->preSetValue->setText(QString::number(std::get<double>(config.preSetVal)));
		}
		else if (config.plcInfo->dataType == AtomicVarType::STRING)
		{
			configItem->preSetValue->setText(QString::fromStdString(std::get<std::string>(config.preSetVal)));
		}
		itemWidgets.push_back(configItem);
		itemLists->addItem(itemNew);
		itemLists->setItemWidget(itemNew, configItem);
	}
}

ConfigVariablesPage::ConfigVariablesPage()
{
	this->setWindowFlags(Qt::Tool | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	QVBoxLayout* vlay = new QVBoxLayout();
	itemLists = new QListWidget();
	itemLists->setMinimumSize(700, 400);
	vlay->addWidget(itemLists);
	QHBoxLayout* hlay = new QHBoxLayout();
	btnAdd = new QPushButton(tr("添加项"));
	btnAdd->setObjectName("CraftConfig");
	btnAdd->setFixedSize(80, 40);
	hlay->addWidget(btnAdd, Qt::AlignRight);
	btnConfirm = new QPushButton(tr("确认"));
	btnConfirm->setObjectName("CraftConfig");
	btnConfirm->setFixedSize(80, 40);
	hlay->addWidget(btnConfirm, Qt::AlignRight);
	vlay->addLayout(hlay);
	this->setWindowTitle(tr("工艺参数配置"));
	this->setLayout(vlay);

	ConfigVariablesItem::configPage = this;

	connect(btnAdd, &QPushButton::clicked, this, &ConfigVariablesPage::AddNewItem);
	connect(btnConfirm, &QPushButton::clicked, this, &ConfigVariablesPage::OnConfirm);
}


ConfigVariablesPage::~ConfigVariablesPage()
{

}

void ConfigVariablesPage::AddNewItem()
{
	QListWidgetItem* itemNew = new QListWidgetItem();
	itemNew->setSizeHint(QSize(this->width(), 50));
	ConfigVariablesItem* configItem = new ConfigVariablesItem();
	itemWidgets.push_back(configItem);
	itemLists->addItem(itemNew);
	itemLists->setItemWidget(itemNew, configItem);
}

void ConfigVariablesPage::DeleteItem(QListWidgetItem* item)
{
	if (item)
	{
		int row = itemLists->row(item);
		itemWidgets.erase(itemWidgets.begin() + row);
		QListWidgetItem* takenItem = itemLists->takeItem(row);
		delete takenItem;
	}
}

void ConfigVariablesPage::OnConfirm()
{
	if (attachedSketch)
	{
		attachedSketch->attachedConfig.clear();

		for (ConfigVariablesItem* item : itemWidgets)
		{
			CraftParamConfig config;
			if (item->ToParamConfig(config))
			{
				attachedSketch->attachedConfig.push_back(config);
			}
			else
			{
				attachedSketch->attachedConfig.clear();
				QMessageBox::warning(nullptr, tr("错误"), tr("变量值和类型不匹配,请检查后重新设置!"));
			}
		}
	}
	this->close();
}