#include "ModalEvent/EvCanvasSetNewScene.h"

QEvent::Type EvCanvasSetNewScene::eventType = (QEvent::Type)QEvent::registerEventType(QEvent::User + 1);

EvCanvasSetNewScene::EvCanvasSetNewScene(std::shared_ptr<SketchGPU> sketch, OCSGPU* ocs) : m_sketch(sketch), m_ocs(ocs), QEvent(Type(eventType))
{

}

EvCanvasSetNewScene::~EvCanvasSetNewScene()
{

}
