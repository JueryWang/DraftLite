#include "ModalEvent/EvSendCanvasTag.h"

QEvent::Type EvSendCanvasTag::eventType = (QEvent::Type)QEvent::registerEventType(QEvent::User + 1);

EvSendCanvasTag::EvSendCanvasTag(const glm::vec3& canvasPos, const QString& _label, int _size) : QEvent(Type(eventType))
{
	tagPos = canvasPos;
	tagLabel = _label;
	tagSize = _size;
}

EvSendCanvasTag::~EvSendCanvasTag()
{

}