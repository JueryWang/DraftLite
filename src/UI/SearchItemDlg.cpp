#include "UI/SearchItemDlg.h"
#include <QPixmap>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

SearchItemDialog::SearchItemDialog(const QStringList& key) : items(key)
{
	this->setWindowTitle("查找变量");
	this->setWindowFlags(Qt::Tool);
	
	QVBoxLayout* vlay = new QVBoxLayout();

	QHBoxLayout* hlay = new QHBoxLayout();
	QLabel* searchImg = new QLabel(this);
	searchImg->setFixedSize(48,48);
	searchImg->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
	searchImg->setScaledContents(false);
	searchImg->setAlignment(Qt::AlignCenter);

	QPixmap pix("Resources/icon/search.png");
	pix.scaled(
		searchImg->size(),
		Qt::KeepAspectRatio, // 保持宽高比
		Qt::SmoothTransformation // 平滑缩放（避免锯齿）
	);
	searchImg->setPixmap(pix);
	hlay->addWidget(searchImg);

	keyEdit = new QLineEdit(this);
	hlay->addWidget(keyEdit);

	model = new QStringListModel(items);
	completer = new QCompleter(model, keyEdit);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	completer->setFilterMode(Qt::MatchContains);

	keyEdit->setCompleter(completer);

	QHBoxLayout* btnLayout = new QHBoxLayout();
	btnLayout->setContentsMargins(5, 5, 10, 5);
	QPushButton* btnConfirm = new QPushButton("确定");
	QObject::connect(btnConfirm, &QPushButton::clicked, [&]() {
		if (callback)
		{
			callback(this->keyEdit->text());
		}
		this->keyEdit->clear();
		this->hide();
	});
	
	QPushButton* btnCancel = new QPushButton("取消");
	QObject::connect(btnCancel, &QPushButton::clicked, [&]() {this->close(); });

	QSpacerItem* placeHolder = new QSpacerItem(70,25, QSizePolicy::Expanding);
	btnLayout->addSpacerItem(placeHolder);
	btnLayout->addWidget(btnConfirm);
	btnLayout->addWidget(btnCancel);

	vlay->addLayout(hlay);
	vlay->addLayout(btnLayout);

	this->setMinimumSize(350,50);
	this->setLayout(vlay);
}

SearchItemDialog::~SearchItemDialog()
{

}

void SearchItemDialog::SetNewKeys(const QStringList& newKey)
{
	items = newKey;
	model->setStringList(newKey);
}
