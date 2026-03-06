#include "UI/Components/HmiTemplateStationPreview.h"

HmiTemplateStationPreview::HmiTemplateStationPreview()
{
	this->setViewMode(QListView::IconMode);
	this->setResizeMode(QListView::Adjust);
	this->setWrapping(true);
	this->setSpacing(20);
	this->setFlow(QListView::LeftToRight);      // ´Ó×óÍùÓÒÅÅÁÐ
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
	previewlists.push_back(preview);
	item->setSizeHint(QSize(200,200));

	this->setModel(model);
	this->setIndexWidget(index, preview);
}
