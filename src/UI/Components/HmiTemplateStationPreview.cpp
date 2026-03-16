#include "UI/Components/HmiTemplateStationPreview.h"
#include "Graphics/Sketch.h"
#include "UI/MainLayer.h"

PreviewItem::PreviewItem(GLWidget* preview, int id) : canvas(preview)
{
    QHBoxLayout* hlay = new QHBoxLayout(this);
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->setSpacing(2);

    QVBoxLayout* vlay = new QVBoxLayout();
    vlay->setContentsMargins(0, 0, 0, 0);

    editor = GCodeEditor::GetInstance();
    editor->setText(QString::fromUtf8(preview->GetCanvas()->GetSketchShared()->ToNcProgram()));
    MenuLayerTop* menu = new MenuLayerTop(preview, editor);
    vlay->addWidget(menu);
    vlay->addWidget(preview);
    vlay->addWidget(editor);

 
    QVBoxLayout* vlay2 = new QVBoxLayout();
    vlay2->setContentsMargins(10, 10, 10, 10);

    infoPanel = new SketchInfoPanel(this);
    infoPanel->updateStats(preview->attachedSketch);

    vlay2->addWidget(infoPanel);
    vlay2->addStretch(); 

    hlay->addLayout(vlay, 1);
    hlay->addLayout(vlay2, 0);
}

PreviewItem::~PreviewItem()
{

}

StationSwitchTab::StationSwitchTab(MainLayer* parent) : holder(parent)
{
    this->setElideMode(Qt::ElideNone);

    this->addTab(QString("预览"));
    this->addTab(QString("工位1"));
    this->addTab(QString("工位2"));
    this->addTab(QString("工位3"));
    this->addTab(QString("工位4"));
    this->addTab(QString("工位5"));
    this->addTab(QString("工位6"));
    this->addTab(QString("工位7"));
    this->addTab(QString("工位8"));
    this->addTab(QString("工位9"));
    this->addTab(QString("工位10"));
    this->setShape(QTabBar::RoundedEast);

    this->setStyleSheet(R"(
        QTabBar {
            min-width: 80px;
            white-space: nowrap;
        }
        QTabBar::tab {
            padding: 10px 6px;
            text-align: left;
            /* 关键：取消左侧标签的默认竖排 */
            writing-mode: lr-tb;  /* 左到右、上到下（默认竖排是 tb-rl） */
        }
        QTabWidget::tab-bar:west {
            alignment: left;
        }
    )");

    connect(this, &QTabBar::currentChanged,this,&StationSwitchTab::SwitchStation);
}

StationSwitchTab::~StationSwitchTab()
{

}

void StationSwitchTab::SwitchStation(int index)
{
    g_mainWindow->currentSketchIndex = index;
    holder->preview->attachedSketch = holder->sketchLists[index].get();
    holder->preview->GetCanvas()->SetScene(holder->sketchLists[index], holder->sketchLists[index]->attachedOCS);
    holder->infoPanel->updateStats(holder->preview->attachedSketch);
}