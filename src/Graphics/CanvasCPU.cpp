#include "Graphics/Canvas.h"
#include "Graphics/OCS.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Sketch.h"
#include <QDebug>
#include <QGraphicsScene>

namespace CNCSYS
{
    CanvasCPU::CanvasCPU(std::shared_ptr<SketchCPU> sketch, int width, int height, bool isMainCanvas)
    {
        ocsSys = new OCSCPU(sketch);
        m_scene = new QGraphicsScene();
        this->setScene(m_scene);
        m_sketch = sketch;
        m_sketch.get()->mainCanvas = this;
        ocsSys->genTickers = isMainCanvas;
        ocsSys->canvasWidth = width;
        ocsSys->canvasHeight = height;
        UpdateOCS();
        ResetView();
        this->resize(QSize(width, height));
    }
    CanvasCPU::~CanvasCPU()
    {
        m_sketch.reset();
        delete ocsSys;
        delete m_scene;
    }

    void CanvasCPU::UpdateOCS()
    {
        ocsSys->ComputeScaleFitToCanvas();
        if (isMainCanvas)
        {
            ocsSys->UpdateTickers();
        }

        QRectF sceneRect = QRectF(QPointF(ocsSys->canvasRange->min.x, ocsSys->canvasRange->min.y), QSize(ocsSys->canvasRange->XRange(), ocsSys->canvasRange->YRange()));
        this->setSceneRect(sceneRect);
    }

    void CanvasCPU::ResetView()
    {
        m_scene->clear();

        for (EntityVCPU* ent : m_sketch.get()->entities)
        {
            ent->setVisible(true);
            m_scene->addItem(ent);
        }
        UpdateOCS();
        QTransform transform;
        transform.scale(1, -1);  // Y轴取反
        this->setTransform(transform);
        
        this->viewport()->update();
    }

    void CanvasCPU::EnterModal(ModalState modal)
    {

    }
    void CanvasCPU::EndModal()
    {

    }
    bool CanvasCPU::eventFilter(QObject* obj, QEvent* event)
    {
        return QGraphicsView::eventFilter(obj, event);
    }
}