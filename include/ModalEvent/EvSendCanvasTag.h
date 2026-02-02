#pragma once

#include <QEvent>
#include <QPoint>
#include <QString>
#include <tuple>
#include "glm/glm.hpp"

class EvSendCanvasTag : public QEvent
{
public:
	EvSendCanvasTag(const glm::vec3& canvasPos, const QString& _label, int _size);
	~EvSendCanvasTag();

	static Type eventType;

	glm::vec3 tagPos;
	QString tagLabel;
	int tagSize;
};