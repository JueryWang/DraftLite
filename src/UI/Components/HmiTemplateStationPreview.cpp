#include "UI/Components/HmiTemplateStationPreview.h"
#include "Graphics/Sketch.h"
PreviewItem::PreviewItem(GLWidget* preview, int id) : canvas(preview)
{
    QHBoxLayout* hlay = new QHBoxLayout(this);
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->setSpacing(2);

    QVBoxLayout* vlay = new QVBoxLayout();
    vlay->setContentsMargins(0, 0, 0, 0);

    editor = new QsciScintilla();
    editor->setText(QString::fromUtf8(preview->GetCanvas()->GetSketchShared()->ToNcProgram()));
    MenuLayerTop* menu = new MenuLayerTop(preview, editor);
    vlay->addWidget(menu);
    vlay->addWidget(preview);
    vlay->addWidget(editor);

 
    QVBoxLayout* vlay2 = new QVBoxLayout();
    vlay2->setContentsMargins(10, 10, 10, 10);

    infoPanel = new SketchInfoPanel(this);
    infoPanel->setFixedWidth(200);
    int draftWidth = preview->attachedSketch->attachedOCS->objectRange->XRange();
    int drafHeight = preview->attachedSketch->attachedOCS->objectRange->YRange();
    infoPanel->updateStats(preview->attachedSketch->enitiySize,
        preview->attachedSketch->contourSize,
        g_MScontext.totalPath,
        g_MScontext.idlePath,QSize(draftWidth,drafHeight));

    vlay2->addWidget(infoPanel);
    vlay2->addStretch(); 

    hlay->addLayout(vlay, 1);
    hlay->addLayout(vlay2, 0);
}

PreviewItem::~PreviewItem()
{

}
