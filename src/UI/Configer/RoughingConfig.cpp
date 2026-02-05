#include "UI/Configer/RoughingConfig.h"
#include "UI/Configer/WorkBlankConfig.h"
#include "UI/GCodeEditor.h"
#include "Common/ProgressInfo.h"
#include "Algorithm/RoughingAlgo.h"
#include "Graphics/DrawEntity.h"
#include "QGroupBox"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDoubleValidator>
#include <QMessageBox>

RoughingConfigPage* RoughingConfigPage::s_instance = nullptr;
RoughingParamSettings RoughingConfigPage::s_setting;

RoughingConfigPage* RoughingConfigPage::GetInstance()
{
	if (RoughingConfigPage::s_instance == nullptr)
	{
		RoughingConfigPage::s_instance = new RoughingConfigPage();
	}
	return RoughingConfigPage::s_instance;
}

RoughingConfigPage::RoughingConfigPage()
{
	QVBoxLayout* layout = new QVBoxLayout();

	this->setWindowTitle("开粗配置:");
	this->setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	QHBoxLayout* row1 = new QHBoxLayout();
	row1->setSpacing(10);
	labelDirection = new QLabel("切削方向:");
	labelDirection->setMinimumWidth(30);
	labelDirection->setObjectName("CraftConfig");
	row1->addWidget(labelDirection);
	
	comboDirection = new QComboBox();
	comboDirection->setObjectName("CraftConfig");
	QStringList directions = {"顺铣","逆铣","任意"};
	comboDirection->setMinimumWidth(120);
	comboDirection->addItems(directions);
	row1->addWidget(comboDirection,Qt::AlignRight);
	connect(comboDirection, &QComboBox::currentIndexChanged, this, [&](int index)
	{
			if (index == 0)
				s_setting.direction = MillingDirection::CW;
			else if (index == 1)
				s_setting.direction = MillingDirection::CCW;
			else
				s_setting.direction = MillingDirection::Any;
	});

	QHBoxLayout* row2 = new QHBoxLayout();
	row2->setSpacing(10);
	labelRemain = new QLabel("余量:");
	labelRemain->setMinimumWidth(30);
	labelRemain->setObjectName("CraftConfig");
	row2->addWidget(labelRemain);

	editRemain = new QLineEdit();
	QDoubleValidator* validator = new QDoubleValidator(0.0, 999.0,10, this);
	editRemain->setValidator(validator);
	editRemain->setObjectName("CraftConfig");
	editRemain->setText("0.0");
	connect(editRemain, &QLineEdit::editingFinished, this, [&]()
		{
			s_setting.allowance = editRemain->text().toDouble();
		});
	row2->addWidget(editRemain);

	labelLineSpacing = new QLabel("行距:");
	labelLineSpacing->setMinimumWidth(30);
	labelLineSpacing->setObjectName("CraftConfig");
	row2->addWidget(labelLineSpacing);

	editLineSpacing = new QLineEdit();
	editLineSpacing->setText("5.0");
	editLineSpacing->setValidator(validator);
	editLineSpacing->setObjectName("CraftConfig");
	connect(editLineSpacing, &QLineEdit::editingFinished, this, [&]()
		{
			s_setting.stepover = editLineSpacing->text().toDouble();
		});
	row2->addWidget(editLineSpacing);

	QHBoxLayout* row3 = new QHBoxLayout();
	row3->setSpacing(10);
	labelTolerance = new QLabel("公差:");
	labelTolerance->setMinimumWidth(30);
	labelTolerance->setObjectName("CraftConfig");
	row3->addWidget(labelTolerance);

	QDoubleValidator* toleranceValidator = new QDoubleValidator(0.0,1.0,10,this);
	editTolerance = new QLineEdit();
	editTolerance->setText("0.01");
	editTolerance->setValidator(toleranceValidator);
	editTolerance->setObjectName("CraftConfig");
	connect(editTolerance, &QLineEdit::editingFinished, this, [&]()
		{
			s_setting.tolerance = editTolerance->text().toDouble();
		});
	row3->addWidget(editTolerance);

	QHBoxLayout* row4 = new QHBoxLayout();
	row4->setSpacing(10);
	
	QWidget* placeholder = new QWidget();
	row4->addWidget(placeholder);

	btnExec = new QPushButton("执行");
	btnExec->setObjectName("CraftConfig");
	btnExec->setMinimumWidth(40);
	row4->addWidget(btnExec);
	connect(btnExec, &QPushButton::clicked, this, [&]()
		{
			if (WorkBlankConfigPage::s_attachedRing == nullptr)
			{
				QMessageBox::warning(nullptr,"错误","未指定工件");
				return;
			}
			if (WorkBlankConfigPage::s_workBlank== nullptr)
			{
				QMessageBox::warning(nullptr, "错误", "未指定毛坯");
				return;
			}
			g_MScontext.ncstep = 0;
			
			std::string gcode;
			char buffer[256];
			std::sprintf(buffer, "N%03d G81 X%f Y%f\n", g_MScontext.ncstep, g_MScontext.toolPos.x - g_MScontext.wcsAnchor, g_MScontext.toolPos.y - g_MScontext.wcsAnchor);
			gcode += buffer;
			gcode = RoughingAlgo::GetRoughingPath(WorkBlankConfigPage::s_attachedRing, WorkBlankConfigPage::s_workBlank->bbox, RoughingConfigPage::s_setting);
			GCodeEditor::GetInstance()->setText(QString::fromStdString(gcode));
			this->close();
		});

	btnComfirm = new QPushButton("确认");
	btnComfirm->setObjectName("CraftConfig");
	btnComfirm->setMinimumWidth(40);
	row4->addWidget(btnComfirm);
	connect(btnComfirm, &QPushButton::clicked, this, [&]()
		{


			this->close();
		});

	layout->addLayout(row1);
	layout->addLayout(row2);
	layout->addLayout(row3);
	layout->addLayout(row4);

	this->setLayout(layout);
}

RoughingConfigPage::~RoughingConfigPage()
{

}
