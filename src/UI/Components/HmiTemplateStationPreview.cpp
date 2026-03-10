#include "UI/Components/HmiTemplateStationPreview.h"
#include "Graphics/Sketch.h"

HmiTemplateStationPreview::HmiTemplateStationPreview()
{
	this->setViewMode(QListView::IconMode);
	this->setResizeMode(QListView::Adjust);
	this->setWrapping(true);
	this->setSpacing(20);
	this->setFlow(QListView::LeftToRight);      // ������������
	this->setMovement(QListView::Static);

	model = new QStandardItemModel();
}

HmiTemplateStationPreview::~HmiTemplateStationPreview()
{

}

void HmiTemplateStationPreview::AddPreview(GLWidget* preview)
{
	QStandardItem* item = new QStandardItem();
	model->appendRow(item);
	QModelIndex index = model->index(previewlists.size(), 0);
	item->setSizeHint(QSize(preview->width(), preview->height() + 200));

	this->setModel(model);
	PreviewItem* previewItem = new PreviewItem(preview,previewlists.size()+1);
	this->setIndexWidget(index, previewItem);
	previewlists.push_back(preview);
}

PreviewItem::PreviewItem(GLWidget* preview, int id)
{
	QVBoxLayout* vlay = new QVBoxLayout();
	vlay->addWidget(preview);
	QHBoxLayout* hlay1 = new QHBoxLayout();
	QLabel* stationHint = new QLabel("工位号:");
	stationId = new QLabel(QString::number(id));
	hlay1->addWidget(stationHint);
	hlay1->addWidget(stationId);
	vlay->addLayout(hlay1);
	QLabel* fileSourceHint = new QLabel("文件源:");
	QHBoxLayout* hlay2 = new QHBoxLayout();
	fileSource = new QLabel();
	fileSource->setText(QString::fromUtf8(preview->GetCanvas()->GetSketchShared()->source));
	hlay2->addWidget(fileSourceHint);
	hlay2->addWidget(fileSource);
	vlay->addLayout(hlay2);
	QLabel* gcodeHint = new QLabel("G代码预览:");
	vlay->addWidget(gcodeHint);
	editor = new QsciScintilla();
	editor->setText(QString::fromUtf8(preview->GetCanvas()->GetSketchShared()->ToNcProgram()));
	vlay->addWidget(editor);
	vlay->setContentsMargins(0, 0, 0, 0);
	this->setLayout(vlay);
}

PreviewItem::~PreviewItem()
{

}
