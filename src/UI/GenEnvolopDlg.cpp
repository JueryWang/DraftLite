#include "UI/GenEnvolopDlg.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QCheckBox>

namespace CNCSYS
{
	GenEnvolopDialog::GenEnvolopDialog(QWidget* parent)
	{
		this->setParent(parent);
		this->setWindowFlags(Qt::Tool | Qt::WindowTitleHint);

		QVBoxLayout* vlay = new QVBoxLayout();
		QLabel* lbTopLayout = new QLabel("包络线");
		QString style = R"(
			QLabel{
				font-family: 'Microsoft YaHei';
				font-size: 16px;
				font-weight: bold;
				color:white;
				background-color: #1a3052;
				padding: 20px;
			}
		)";
		lbTopLayout->setStyleSheet(style);
		QWidget* container= new QWidget(this);
		QVBoxLayout* innerLayout = new QVBoxLayout(container);
		innerLayout->setContentsMargins(30, 30, 30, 30);
		QLabel* label = new QLabel("生成包络线,零件间距越大,包络线包含的范围越大");
		innerLayout->addWidget(label);
		innerLayout->addSpacing(20);

		vlay->addWidget(lbTopLayout);
		
	}

	GenEnvolopDialog::~GenEnvolopDialog()
	{

	}
}

