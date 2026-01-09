#include "UI/ArrayGenerationDlg.h"
#include "UI/ClickableLabel.h"
#include "UI/VirtualKeyBoard.h"
#include "UI/MainLayer.h"
#include "UI/Components/HmiInterfaceDefines.h"
#include <QApplication>
#include <QMainWindow>
#include <QGridLayout>
#include <QSpacerItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>


namespace CNCSYS
{

	RectArrayGenDlg* RectArrayGenDlg::instance = nullptr;
	RingArrayGenDlg* RingArrayGenDlg::instance = nullptr;

	RectArrayGenDlg* RectArrayGenDlg::GetInstance()
	{
		QList<QWidget*> topLevelWidgets = QApplication::topLevelWidgets();
		MainLayer* topWidget = nullptr;

		for (QWidget* wgt : topLevelWidgets)
		{
			if ((topWidget = dynamic_cast<MainLayer*>(wgt)) != nullptr)
				break;
		}

		if (topWidget && instance == nullptr)
		{
			RectArrayGenDlg::instance = new RectArrayGenDlg(topWidget);
		}

		if (topWidget && RectArrayGenDlg::instance) {
			RectArrayGenDlg::instance->rowCountEdit->clearFocus();
			RectArrayGenDlg::instance->colCountEdit->clearFocus();
			RectArrayGenDlg::instance->rowOffsetBox->clearFocus();
			RectArrayGenDlg::instance->colOffsetBox->clearFocus();
			RectArrayGenDlg::instance->setFocus();

			QRect mainRect = topWidget->geometry();
			QRect dlgRect = RectArrayGenDlg::instance->geometry();

			int x = mainRect.x() + (mainRect.width() - dlgRect.width()) / 2;
			int y = mainRect.y() + (mainRect.height() - dlgRect.height()) / 2;

			RectArrayGenDlg::instance->move(x, y);
		}

		return RectArrayGenDlg::instance;
	}
	
	RectArrayGenDlg::RectArrayGenDlg(QWidget* parent) : QDialog(parent)
	{
		qRegisterMetaType<RectArrayParam>("RectArrayParam");

		this->setStyleSheet("background-color: #f0f2f5;color:#24395a;font-size:14px;");
		this->setWindowFlags(Qt::FramelessWindowHint |Qt::Dialog
			| Qt::Tool
			| Qt::WindowTitleHint);
		this->setFixedSize(380, 500);

		QVBoxLayout* layout = new QVBoxLayout();
		layout->setSpacing(20);
		layout->setContentsMargins(0, 0, 0, 10);

		{
			QHBoxLayout* layTitleClose = new QHBoxLayout();
			layTitleClose->addSpacing(10);
			QWidget* wrapper = new QWidget();
			wrapper->setFixedSize(380, 35);
			wrapper->setStyleSheet("background-color: #1a3052");

			QLabel* title = new QLabel("矩形阵列");
			title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			title->setStyleSheet(".QLabel{font-size: 14px;color: white;font-weight: bold;}");
			ClickableLabel* closeBtn = new ClickableLabel("icon/close-white.png", 12);
			closeBtn->setFixedSize(20, 20);
			QObject::connect(closeBtn, &ClickableLabel::clicked, [&]() {VirtualKeyBoard::GetInstance()->close(); this->close(); });
			layTitleClose->addWidget(title, Qt::AlignLeft);
			layTitleClose->addWidget(closeBtn, Qt::AlignRight);
			layTitleClose->setContentsMargins(0, 0, 0, 0);
			layTitleClose->addSpacing(10);
			wrapper->setLayout(layTitleClose);
			layout->addWidget(wrapper);
		}

		QVBoxLayout* mainBoxLayout = new QVBoxLayout();
		mainBoxLayout->setContentsMargins(20, 0, 20, 20);

		QLabel* description = new QLabel("本功能根据给定的行数和列数进行快速复制。");
		mainBoxLayout->addWidget(description);

		{
			QGroupBox* arryGroupBox = new QGroupBox("阵列数量");
			arryGroupBox->setMinimumHeight(90);
			arryGroupBox->setStyleSheet(GroupBoxStyle);
			QHBoxLayout* arrayGroupLayout = new QHBoxLayout();
			arrayGroupLayout->setContentsMargins(20, 10, 20, 10);
			QLabel* label1 = new QLabel("行数:");
			arrayGroupLayout->addWidget(label1);
			rowCountEdit = new SpinEditInput("行数", 50, {1,100}, "", 0, 2);
			QLineEdit* lineEdit = rowCountEdit->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			rowCountEdit->setValue(2);
			arrayGroupLayout->addWidget(rowCountEdit);
			arrayGroupLayout->addSpacing(20);
			QLabel* label2 = new QLabel("列数:");
			arrayGroupLayout->addWidget(label2);
			colCountEdit = new SpinEditInput("列数", 50, {1,100}, "", 0, 2);
			colCountEdit->setStyleSheet(SpinBoxInputStyele);
			lineEdit = colCountEdit->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			arrayGroupLayout->addWidget(colCountEdit);
			arryGroupBox->setLayout(arrayGroupLayout);
			mainBoxLayout->addWidget(arryGroupBox);
		}

		{
			QGroupBox* OffsetGroupBox = new QGroupBox("偏移方式");
			OffsetGroupBox->setMinimumHeight(110);
			OffsetGroupBox->setStyleSheet(GroupBoxStyle);
			QVBoxLayout* overallLayout = new QVBoxLayout();
			QHBoxLayout* offsetGroupLayout = new QHBoxLayout();
			checkOffset = new QRadioButton("偏移量");
			checkOffset->setChecked(true);
			checkOffset->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			checkSpacing = new QRadioButton("间距");
			offsetGroupLayout->addWidget(checkOffset, Qt::AlignLeft);
			offsetGroupLayout->addWidget(checkSpacing, Qt::AlignLeft);
			QLabel* label1 = new QLabel("行偏移:");
			rowOffsetBox = new SpinEditInput("行偏移", 50, {0,9999}, "", 0, 10);
			rowOffsetBox->setSuffix("mm");
			QLineEdit* lineEdit = rowOffsetBox->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			QLabel* label2 = new QLabel("列偏移");
			colOffsetBox = new SpinEditInput("行偏移", 50, {0,9999}, "mm", 0, 10);
			lineEdit = colOffsetBox->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			QHBoxLayout* offsetGroupLayout2 = new QHBoxLayout();
			offsetGroupLayout2->addWidget(label1);
			offsetGroupLayout2->addWidget(rowOffsetBox);
			offsetGroupLayout2->addWidget(label2);
			offsetGroupLayout2->addWidget(colOffsetBox);
			overallLayout->addLayout(offsetGroupLayout);
			overallLayout->addLayout(offsetGroupLayout2);
			OffsetGroupBox->setLayout(overallLayout);

			mainBoxLayout->addWidget(OffsetGroupBox);
		}


		{
			QGroupBox* rowDirGroupBox = new QGroupBox("行方向");
			rowDirGroupBox->setChecked(true);
			rowDirGroupBox->setMinimumHeight(100);
			rowDirGroupBox->setStyleSheet(GroupBoxStyle);
			QHBoxLayout* overallLayout = new QHBoxLayout();
			QVBoxLayout* rowDirLayout = new QVBoxLayout();
			lineDirUp = new QRadioButton("向上");
			lineDirDown = new QRadioButton("向下");
			lineDirUp->setChecked(true);
			rowDirLayout->addWidget(lineDirUp);
			rowDirLayout->addWidget(lineDirDown);
			rowDirGroupBox->setLayout(rowDirLayout);
			QGroupBox* colDirGroupBox = new QGroupBox("列方向");
			colDirGroupBox->setChecked(true);
			colDirGroupBox->setMinimumHeight(100);
			colDirGroupBox->setStyleSheet(GroupBoxStyle);
			QVBoxLayout* colDirLayout = new QVBoxLayout();
			lineDirLeft = new QRadioButton("向左");
			lineDirRight = new QRadioButton("向右");
			lineDirLeft->setChecked(true);
			colDirLayout->addWidget(lineDirLeft);
			colDirLayout->addWidget(lineDirRight);
			colDirGroupBox->setLayout(colDirLayout);
			overallLayout->addWidget(rowDirGroupBox);
			overallLayout->addWidget(colDirGroupBox);
			mainBoxLayout->addLayout(overallLayout);
		}

		layout->addLayout(mainBoxLayout);

		QFrame* frame = new QFrame();
		frame->setAttribute(Qt::WA_StyledBackground, true);
		frame->setStyleSheet("QFrame {"
			"   background-color: rgba(155, 155, 155, 20); /* 边框半透明 */"
			"   border: none;"
			"}");
		frame->setFrameShape(QFrame::HLine); // 水平线条
		frame->setLineWidth(1); // 线条宽度

		layout->addWidget(frame);

		QSpacerItem* placeHolder = new QSpacerItem(180, 25, QSizePolicy::Expanding);
		QPushButton* btnOK = new QPushButton("确定");
		QPushButton* btnCancel = new QPushButton("取消");
		QHBoxLayout* btnLayout = new QHBoxLayout();
		btnLayout->setContentsMargins(5, 5, 10, 5);
		btnLayout->addSpacerItem(placeHolder);
		btnLayout->addWidget(btnOK, Qt::AlignRight);
		btnOK->setFixedSize(80, 25);
		btnLayout->addWidget(btnCancel, Qt::AlignRight);
		btnCancel->setFixedSize(80, 25);

		layout->addLayout(btnLayout);
		this->setLayout(layout);

		connect(btnOK, &QPushButton::clicked, [this]()
			{
				RectArrayParam param;
				param.colCount = colCountEdit->value();
				param.colDir = lineDirLeft->isChecked() ? ColDirection::LEFT : ColDirection::RIGHT;
				param.offsetCol = colOffsetBox->value();
				param.rowCount = rowCountEdit->value();
				param.rowDir = lineDirUp->isChecked() ? RowDirection::UP : RowDirection::DOWN;
				param.offsetRow = rowOffsetBox->value();
				emit ConfirmData(param);
				VirtualKeyBoard::GetInstance()->close();
				this->close();
			});
		connect(btnCancel, &QPushButton::clicked, [this]() {this->close(); });

		this->installEventFilter(this);
	}

	RectArrayGenDlg::~RectArrayGenDlg()
	{

	}

	RingArrayGenDlg* RingArrayGenDlg::GetInstance()
	{
		QList<QWidget*> topLevelWidgets = QApplication::topLevelWidgets();
		MainLayer* topWidget = nullptr;

		for (QWidget* wgt : topLevelWidgets)
		{
			if ((topWidget = dynamic_cast<MainLayer*>(wgt)) != nullptr)
				break;
		}

		if (topWidget && instance == nullptr)
		{
			RingArrayGenDlg::instance = new RingArrayGenDlg(topWidget);
		}
		if (topWidget && RingArrayGenDlg::instance) {
			
			QRect mainRect = topWidget->geometry();
			QRect dlgRect = RingArrayGenDlg::instance->geometry();

			int x = mainRect.x() + (mainRect.width() - dlgRect.width()) / 2;
			int y = mainRect.y() + (mainRect.height() - dlgRect.height()) / 2;

			RingArrayGenDlg::instance->move(x, y);
		}
		return instance;
	}

	RingArrayGenDlg::RingArrayGenDlg(QWidget* parent) : QDialog(parent)
	{
		this->setParent(parent);
		qRegisterMetaType<RingArrayParam>("RingArrayParam");

		this->setStyleSheet("background-color: #f0f2f5;color:#24395a;font-size:14px;");
		this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog
			| Qt::Tool
			| Qt::WindowTitleHint);
		this->setFixedSize(380, 500);

		QVBoxLayout* layout = new QVBoxLayout();
		layout->setSpacing(20);
		layout->setContentsMargins(0, 0, 0, 10);
		{
			QHBoxLayout* layTitleClose = new QHBoxLayout();
			layTitleClose->addSpacing(10);
			QWidget* wrapper = new QWidget();
			wrapper->setFixedHeight(35);
			wrapper->setStyleSheet("background-color: #1a3052");
			QLabel* title = new QLabel("环形阵列");
			title->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			title->setStyleSheet(".QLabel{font-size: 14px;color: white;font-weight: bold;}");
			ClickableLabel* closeBtn = new ClickableLabel("icon/close-white.png", 12);
			closeBtn->setFixedSize(20, 20);
			QObject::connect(closeBtn, &ClickableLabel::clicked, [&]() {this->close(); });
			layTitleClose->addWidget(title, Qt::AlignLeft);
			layTitleClose->addWidget(closeBtn, Qt::AlignRight);
			layTitleClose->setContentsMargins(0, 0, 0, 0);
			layTitleClose->addSpacing(10);
			wrapper->setLayout(layTitleClose);
			layout->addWidget(wrapper);
		}

		QVBoxLayout* mainBoxLayout = new QVBoxLayout();
		mainBoxLayout->setContentsMargins(40, 20, 40, 0);

		QLabel* description = new QLabel("本功能用于将图形绕中心点旋转阵列。");
		layout->addSpacing(20);

		mainBoxLayout->addWidget(description);
		QGridLayout* gridLayout = new QGridLayout();
		{
			QLabel* label1 = new QLabel("图形数量:");
			itemCountEdit = new SpinEditInput("图形数量", 60, {1,360}, "", 0, 4);
			gridLayout->addWidget(label1, 0, 0,1,4);
			gridLayout->addWidget(itemCountEdit, 0, 4, 1, 2);
			QLabel* label2 = new QLabel("阵列方式:");
			gridLayout->addWidget(label2, 1, 0, 1, 4);
			checkBySpacingBtn = new QRadioButton("按角度间距:");
			checkBySpacingBtn->setChecked(true);
			QLineEdit* lineEdit = itemCountEdit->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			gridLayout->addWidget(checkBySpacingBtn, 1, 4, 1, 5);
			angleSpacingEdit = new SpinEditInput("按角度间距", 100, {1,360}, "°", 0, 20);
			gridLayout->addWidget(angleSpacingEdit, 1, 9, 1, 5);
			lineEdit = angleSpacingEdit->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			checkByRangeBtn = new QRadioButton("按阵列范围:");
			gridLayout->addWidget(checkByRangeBtn, 2, 4, 1, 5);
			angleRangeEdit = new SpinEditInput("按阵列范围", 100, {1,360}, "°", 0, 360);
			lineEdit = angleRangeEdit->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			angleRangeEdit->setReadOnly(true);
			gridLayout->addWidget(angleRangeEdit, 2, 9, 1, 5);

			setCenterCbx = new QCheckBox("设置阵列中心参数");
			setCenterCbx->setChecked(true);
			setCenterCbx->setStyleSheet(".QCheckBox{ color:#24395a; font-weight: bold;}");
			gridLayout->addWidget(setCenterCbx, 3, 0, 1, 12);

			QLabel* label4 = new QLabel("阵列中心半圆:");
			radiusEdit = new SpinEditInput("阵列中心半圆", 100, {1,9999}, "mm", 0, 20);
			lineEdit = radiusEdit->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			gridLayout->addWidget(label4,4, 4, 1, 5);
			gridLayout->addWidget(radiusEdit,4, 9, 1, 5);

			QLabel* label5 = new QLabel("图形相对中心起始角:");
			startupAngleEdit = new SpinEditInput("圆形相对中心起始角", 100, {1,360}, "°", 0, 30);
			lineEdit = startupAngleEdit->findChild<QLineEdit*>();
			if (lineEdit) {
				lineEdit->setAlignment(Qt::AlignCenter);
			}
			gridLayout->addWidget(label5, 5, 2, 1, 7);
			gridLayout->addWidget(startupAngleEdit, 5,9, 1, 5);

			mainBoxLayout->addLayout(gridLayout);
		}
		layout->addLayout(mainBoxLayout);

		QFrame* frame = new QFrame();
		frame->setAttribute(Qt::WA_StyledBackground, true);
		frame->setStyleSheet("QFrame {"
			"   background-color: rgba(155, 155, 155, 20); /* 边框半透明 */"
			"   border: none;"
			"}");
		frame->setFrameShape(QFrame::HLine); // 水平线条
		frame->setLineWidth(1); // 线条宽度

		layout->addWidget(frame);

		QSpacerItem* placeHolder = new QSpacerItem(180, 25, QSizePolicy::Expanding);
		QPushButton* btnOK = new QPushButton("确定");
		connect(btnOK, &QPushButton::clicked, [this](){
			RingArrayParam param;
			param.itemCount = itemCountEdit->value();
			param.angleSpacing = angleSpacingEdit->value();
			param.angleRange = angleRangeEdit->value();
			param.radius = radiusEdit->value();
			param.startupAngle = startupAngleEdit->value();
			param.setCenterParam = setCenterCbx->isChecked();
			if (checkBySpacingBtn->isChecked())
			{
				param.RingType = RingArrayType::BaseOnSpacing;
			}
			else if (checkBySpacingBtn)
			{
				param.RingType = RingArrayType::BaseOnRange;
			}
			emit ConfirmData(param);
			this->close();
		});
		QPushButton* btnCancel = new QPushButton("取消");
		connect(btnCancel, &QPushButton::clicked, [this]() {
			this->close();
		});
		QHBoxLayout* btnLayout = new QHBoxLayout();
		btnLayout->setContentsMargins(5, 5, 10, 5);
		btnLayout->addSpacerItem(placeHolder);
		btnLayout->addWidget(btnOK, Qt::AlignRight);
		btnOK->setFixedSize(80, 25);
		btnLayout->addWidget(btnCancel, Qt::AlignRight);
		btnCancel->setFixedSize(80, 25);

		layout->addLayout(btnLayout);
		this->setLayout(layout);
	}

	RingArrayGenDlg::~RingArrayGenDlg()
	{
		VirtualKeyBoard::GetInstance()->close();
	}
}