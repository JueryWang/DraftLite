#pragma once
#include <QEvent>
#include "Graphics/OCS.h"
#include "Graphics/Sketch.h"

using namespace CNCSYS;

class EvCanvasSetNewScene : public QEvent
{
public:
	EvCanvasSetNewScene(std::shared_ptr<SketchGPU> sketch, OCSGPU* ocs);
	~EvCanvasSetNewScene();
	static Type eventType;

public:
	std::shared_ptr<SketchGPU> m_sketch;
	OCSGPU* m_ocs;
};