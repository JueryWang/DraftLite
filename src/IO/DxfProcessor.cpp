#include "IO/DxfProcessor.h"
#include "Graphics/DrawEntity.h"
#include "Graphics/Primitives.h"
#include "Common/MathUtils.h"
#include "Graphics/Sketch.h"
#include "Algorithm/RingDetector.h"
#include "Algorithm/PathOptimizer.h"
#include "Algorithm/PartClassifier.h"
#include <QMessageBox>
#include <boost/unordered_set.hpp>
#include <QApplication>

static std::vector<glm::vec3> vertexs;
static std::vector<glm::vec3> splineControls;
static std::vector<float> bulges;
static std::vector<float> splineKnots;
static int vertexSize = 0;
static int knotsSize;
static bool polylineClose;
static bool openRingDetect = true;

static std::vector<CNCSYS::EntityVGPU*> entityStorage;
//static int splineCount = 0;

namespace CNCSYS
{
	class DxfReader : public DL_CreationAdapter
	{
	public:
		DxfReader(std::weak_ptr<SketchGPU> sketch) : sketchGPU(sketch)
		{
			entityStorage.clear();
			sketchGPU.lock()->ClearEntities();
		}
		DxfReader(std::weak_ptr<SketchCPU> sketch) : sketchCPU(sketch)
		{
			sketchCPU.lock()->ClearEntities();
		}
		~DxfReader()
		{
			blockItems.clear();
		}
		virtual void addLayer(const DL_LayerData& data)
		{
		}
		virtual void addLinetype(const DL_LinetypeData& data)
		{
		}
		virtual void addLinetypeDash(double data)
		{
		}
		virtual void endSection()
		{
		}
		virtual void addBlock(const DL_BlockData& data)
		{
			inBlock = true;
			useBlock = !data.flags;
			blockName = data.name;
			if (useBlock)
			{
				//$ 通常用于 DXF 系统内部定义的特殊块（如 $MODEL_SPACE 表示模型空间块、$PAPER_SPACE 表示图纸空间块)
				auto find = std::find(blockName.begin(), blockName.end(), '$');
				if (find != blockName.end())
				{
					useBlock = false;
				}
			}
			blockItems.clear();
		}
		virtual void endBlock()
		{
			if (blockItems.size())
			{
				static int i = 0;
				if (sketchGPU.lock())
				{
					if (useBlock)
					{
						std::vector<EntRingConnection*> rings = RingDetector::RingDetect(blockItems);
						PartClassifier classifier(rings);
						std::vector<EntGroup*> groups = classifier.Execute();
						
						int ringsize = 0;
						for (EntGroup* group : groups)
						{
							sketchGPU.lock()->AddEntityGroup(group);
							ringsize += group->rings.size();
						}
						sketchGPU.lock()->enitiySize += sketchGPU.lock()->GetEntities().size();
						sketchGPU.lock()->contourSize += ringsize;
					}
				}
				blockItems.clear();
			}
			inBlock = false;
		}
		virtual void addTextStyle(const DL_StyleData& data)
		{
		}
		virtual void addPoint(const DL_PointData& data)
		{
			if (sketchGPU.lock())
			{
				Point2DGPU* point = new Point2DGPU(glm::vec3(data.x, data.y, 0.0f));
				if (inBlock)
				{
					blockItems.push_back(point);
				}
				else
				{
					if (sketchGPU.lock())
					{
						if (openRingDetect)
						{
							entityStorage.push_back(point);
						}
						else
						{
							sketchGPU.lock()->AddEntity(point);
						}
					}
				}
			}
			else if (sketchCPU.lock())
			{
				Point2DCPU* point = new Point2DCPU(glm::vec3(data.x, data.y, 0.0f));
				sketchCPU.lock()->AddEntity(point);
			}
		}
		virtual void addLine(const DL_LineData& data)
		{
			if (sketchGPU.lock())
			{
				Line2DGPU* line = new Line2DGPU(glm::vec3(data.x1, data.y1, 0.0f), glm::vec3(data.x2, data.y2, 0.0f));
				if (inBlock)
				{
					blockItems.push_back(line);
				}
				else
				{
					if (openRingDetect)
					{
						entityStorage.push_back(line);
					}
					else
					{
						sketchGPU.lock()->AddEntity(line);
					}
				}
			}
			else if (sketchCPU.lock())
			{
				Line2DCPU* line = new Line2DCPU(glm::vec3(data.x1, data.y1, 0.0f), glm::vec3(data.x2, data.y2, 0.0f));
				sketchCPU.lock()->AddEntity(line);
			}
		}
		virtual void addXLine(const DL_XLineData& data)
		{
		}
		virtual void addRay(const DL_RayData& data)
		{
		}
		virtual void addArc(const DL_ArcData& data)
		{
			if (sketchGPU.lock())
			{
				Arc2DGPU* arc = new Arc2DGPU(glm::vec3(data.cx, data.cy, 0.0f), data.radius, data.angle1, data.angle2);
				if (inBlock)
				{
					blockItems.push_back(arc);
				}
				else
				{
					if (openRingDetect)
					{
						entityStorage.push_back(arc);
					}
					else
					{
						sketchGPU.lock()->AddEntity(arc);
					}
				}
			}
			else if (sketchCPU.lock())
			{
				Arc2DCPU* arc = new Arc2DCPU(glm::vec3(data.cx, data.cy, 0.0f), data.radius, data.angle1, data.angle2);
				sketchCPU.lock()->AddEntity(arc);
			}
		}
		virtual void addEllipse(const DL_EllipseData& data)
		{
			float radiusX = glm::length(glm::vec3(data.mx, data.my, 0.0f));
			if (sketchGPU.lock())
			{
				Ellipse2DGPU* ellipse = new Ellipse2DGPU(glm::vec3(data.cx, data.cy, 0.0f), radiusX, radiusX * data.ratio);
				if (inBlock)
				{
					blockItems.push_back(ellipse);
				}
				else
				{
					if (openRingDetect)
					{
						entityStorage.push_back(ellipse);
					}
					else
					{
						sketchGPU.lock()->AddEntity(ellipse);
					}
				}
			}
			else if (sketchCPU.lock())
			{
				Ellipse2DCPU* ellipse = new Ellipse2DCPU(glm::vec3(data.cx, data.cy, 0.0f), radiusX, radiusX * data.ratio);
				sketchCPU.lock()->AddEntity(ellipse);
			}
		}
		virtual void addCircle(const DL_CircleData& data)
		{
			if (sketchGPU.lock())
			{
				Circle2DGPU* circle = new Circle2DGPU(glm::vec3(data.cx, data.cy, 0.0f), data.radius);
				if (inBlock)
				{
					blockItems.push_back(circle);
				}
				else
				{
					if (openRingDetect)
					{
						entityStorage.push_back(circle);
					}
					else
					{
						sketchGPU.lock()->AddEntity(circle);
					}
				}
			}
			else if (sketchCPU.lock())
			{
				Circle2DCPU* circle = new Circle2DCPU(glm::vec3(data.cx, data.cy, 0.0f), data.radius);
				sketchCPU.lock()->AddEntity(circle);
			}
		}
		virtual void addPolyline(const DL_PolylineData& data)
		{
			if (vertexSize != 0)
			{
				if (sketchGPU.lock())
				{
					Polyline2DGPU* polyline = new Polyline2DGPU(vertexs, polylineClose, bulges);
					if (inBlock)
					{
						blockItems.push_back(polyline);
					}
					else
					{
						if (openRingDetect)
						{
							entityStorage.push_back(polyline);
						}
						else
						{
							sketchGPU.lock()->AddEntity(polyline);
						}
					}
				}
				else if (sketchCPU.lock())
				{
					Polyline2DCPU* polyline = new Polyline2DCPU(vertexs, polylineClose);
					sketchCPU.lock()->AddEntity(polyline);
				}
			}
			vertexSize = data.number;
			polylineClose = data.flags;
			vertexs.clear();
			bulges.clear();
		}
		virtual void addVertex(const DL_VertexData& data)
		{
			vertexs.push_back(glm::vec3(data.x, data.y, 0.0f));
			vertexSize++;

			bulges.push_back(data.bulge);
		}
		virtual void addSpline(const DL_SplineData& data)
		{
			splineControls.clear();
			splineKnots.clear();
			vertexSize = data.nControl;
			knotsSize = data.nKnots;
			//splineCount++;
		}
		virtual void addControlPoint(const DL_ControlPointData& data)
		{
			splineControls.push_back(glm::vec3(data.x, data.y, 0.0f));
			vertexSize--;
		}
		virtual void addFitPoint(const DL_FitPointData& data)
		{

		}
		virtual void addKnot(const DL_KnotData& data)
		{
			splineKnots.push_back(data.k);
			knotsSize--;
			if (knotsSize == 0)
			{
				bool rescale = splineKnots[splineKnots.size() - 1] > 1;
				if (rescale)
				{
					for (int i = 0; i < splineKnots.size(); i++)
					{
						splineKnots[i] = (float)splineKnots[i] / splineKnots.size();
					}
				}
				if (sketchGPU.lock())
				{
					if (splineKnots[splineKnots.size() - 1] > 1.0f)
					{
						for (int i = 0; i < splineKnots.size(); i++)
						{
							splineKnots[i] = (float)splineKnots[i] / splineKnots[splineKnots.size() - 1];
						}
					}
					Spline2DGPU* spline = new Spline2DGPU(splineControls, splineKnots, false);
					if (inBlock)
					{
						blockItems.push_back(spline);
					}
					else
					{
						if (openRingDetect)
						{
							entityStorage.push_back(spline);
						}
						else
						{
							sketchGPU.lock()->AddEntity(spline);
						}
					}
				}
				else if (sketchCPU.lock())
				{
					Spline2DCPU* spline = new Spline2DCPU(splineControls, splineKnots, false);
					sketchCPU.lock()->AddEntity(spline);
				}
			}
		}
		virtual void addInsert(const DL_InsertData& data)
		{
		}
		virtual void addMText(const DL_MTextData& data)
		{
		}
		virtual void addText(const DL_TextData& data)
		{
		}
		virtual void addArcAlignedText(const DL_ArcAlignedTextData& data)
		{
		}
		virtual void addAttribute(const DL_AttributeData& data)
		{
		}
		virtual void addDimAlign(const DL_DimensionData& dimData, const DL_DimAlignedData& dimAlignData)
		{
		}
		virtual void addDimLinear(const DL_DimensionData& dimData, const DL_DimLinearData& dimLinearData)
		{
		}
		virtual void addDimRadial(const DL_DimensionData&, const DL_DimRadialData&)
		{
		}
		virtual void addDimDiametric(const DL_DimensionData&, const DL_DimDiametricData&)
		{
		}
		virtual void addDimAngular(const DL_DimensionData&, const DL_DimAngular2LData&)
		{
		}
		virtual void addDimAngular3P(const DL_DimensionData&, const DL_DimAngular3PData&)
		{
		}
		virtual void addDimOrdinate(const DL_DimensionData&, const DL_DimOrdinateData&)
		{
		}
		virtual void endEntity()
		{
		}
		virtual void addComment(const std::string& comment)
		{
		}
		virtual void addLeader(const DL_LeaderData& data)
		{
		}
		virtual void addLeaderVertex(const DL_LeaderVertexData& data)
		{
		}
		virtual void addHatch(const DL_HatchData& data)
		{
		}
		virtual void addTrace(const DL_TraceData& data)
		{
		}
		virtual void add3dFace(const DL_3dFaceData& data)
		{
		}
		virtual void addSolid(const DL_SolidData& data)
		{
		}
		virtual void addImage(const DL_ImageData& data)
		{
		}
		virtual void linkImage(const DL_ImageDefData& data)
		{
		}
		virtual void addHatchLoop(const DL_HatchLoopData& data)
		{
		}
		virtual void addHatchEdge(const DL_HatchEdgeData& data)
		{
		}
		virtual void addXRecord(const std::string& record)
		{
		}
		virtual void addXRecordString(int index, const std::string& record)
		{
		}
		virtual void addXRecordReal(int index, double data)
		{
		}
		virtual void addXRecordInt(int index, int data)
		{
		}
		virtual void addXRecordBool(int index, bool data)
		{
		}
		virtual void addXDataApp(const std::string& app)
		{
		}
		virtual void addXDataString(int index, const std::string& data)
		{
		}
		virtual void addXDataReal(int index, double real)
		{
		}
		virtual void addXDataInt(int index, int data)
		{
		}
		virtual void addDictionary(const DL_DictionaryData& data)
		{
		}
		virtual void addDictionaryEntry(const DL_DictionaryEntryData& data)
		{
		}

		virtual void endSequence()
		{
		}
	private:
		std::weak_ptr<SketchGPU> sketchGPU;
		std::weak_ptr<SketchCPU> sketchCPU;
		bool useBlock = false;
		bool inBlock = false;
		std::vector<EntityVGPU*> blockItems;
		std::string blockName;
	};


	DXFProcessor::DXFProcessor(std::shared_ptr<SketchGPU> sketch) : psketchGPU(sketch)
	{

	}

	DXFProcessor::DXFProcessor(std::shared_ptr<SketchCPU> sketch) : psketchCPU(sketch)
	{

	}


	DXFProcessor::~DXFProcessor()
	{
	}

	void handlePoint(Point2DGPU* shape, DL_Dxf* writer, DL_WriterA* dw, int handle, const char* layername)
	{
		std::vector<glm::vec3> nodes = shape->GetTransformedNodes();

		writer->writePoint(*dw,
			DL_PointData(nodes[0].x, nodes[0].y, 0),
			DL_Attributes(layername, 256, 0xFFDEAD, -1, "BYLAYER", handle)
		);
	}

	void handleLine(Line2DGPU* shape, DL_Dxf* writer, DL_WriterA* dw, int handle, const char* layername)
	{
		std::vector<glm::vec3> nodes = shape->GetTransformedNodes();

		writer->writeLine(*dw,
			DL_LineData(nodes[0].x, nodes[0].y, 0.0, nodes[1].x, nodes[1].y, 0.0),
			DL_Attributes(layername, 256, 0xFFDEAD, -1, "BYLAYER", handle)
		);
	}

	void handleCircle(Circle2DGPU* shape, DL_Dxf* writer, DL_WriterA* dw, int handle, const char* layername)
	{
		glm::vec3 transformedCenter = shape->worldModelMatrix * glm::vec4(shape->center, 1.0f);

		writer->writeCircle(*dw,
			DL_CircleData(transformedCenter.x, transformedCenter.y, 0.0, shape->radius),
			DL_Attributes(layername, 256, 0xFFDEAD, -1, "BYLAYER", handle)
		);
	}

	void handleArc(Arc2DGPU* shape, DL_Dxf* writer, DL_WriterA* dw, int handle, const char* layername)
	{
		glm::vec3 transformedCenter = shape->worldModelMatrix * glm::vec4(shape->center, 1.0f);

		writer->writeArc(*dw,
			DL_ArcData(transformedCenter.x, transformedCenter.y, 0.0f, shape->radius, shape->startAngle, shape->endAngle),
			DL_Attributes(layername, 256, 0xFFDEAD, -1, "BYLAYER", handle)
		);
	}

	void handleEllipse(Ellipse2DGPU* shape, DL_Dxf* writer, DL_WriterA* dw, int handle, const char* layername)
	{
		glm::vec3 transformedCenter = shape->worldModelMatrix * glm::vec4(shape->center, 1.0f);

		//writer->writeEllipse(*dw,
		//	DL_EllipseData(transformedCenter.x,transformedCenter.y,0,
		//		shape->a
		//	)
	}

	void handlePolyline(Ellipse2DGPU* shape, DL_Dxf* writer, DL_WriterA* dw, int handle, const char* layername)
	{
		std::vector<glm::vec3> nodes = shape->GetTransformedNodes();
		writer->writePolyline(*dw,
			DL_PolylineData(nodes.size(), 0, 0, 0, 0),
			DL_Attributes(layername, 256, 0xFFDEAD, -1, "BYLAYER", handle)
		);

		for (int i = 0; i < nodes.size(); i++)
		{
			writer->writeVertex(*dw,
				DL_VertexData(nodes[i].x, nodes[i].y, 0, 0));
		}
		writer->writePolylineEnd(*dw);
	}

	void handleSpline(Spline2DGPU* shape, DL_Dxf* writer, DL_WriterA* dw, int handle, const char* layername)
	{
		std::vector<glm::vec3> nodes = shape->GetTransformedNodes();

		std::vector<glm::vec3> transformedControls;
		transformedControls.reserve(shape->controlPoints.size());
		std::transform(shape->controlPoints.begin(), shape->controlPoints.end(),
			std::back_inserter(transformedControls),
			[&](glm::vec3& v) {return shape->worldModelMatrix * glm::vec4(v, 1.0f); });



		DL_SplineData spline = DL_SplineData(3, 0, transformedControls.size(), 0, 0);
		glm::vec3 dirStart = glm::normalize(nodes[1] - nodes[0]);
		glm::vec3 dirEnd = glm::normalize(nodes[nodes.size() - 1] - nodes[nodes.size() - 2]);

		spline.tangentStartX = dirStart.x;
		spline.tangentStartY = dirStart.y;
		spline.tangentEndX = dirEnd.x;
		spline.tangentEndY = dirEnd.y;

		for (auto& knot : shape->knots)
		{
			writer->writeKnot(*dw,
				DL_KnotData(knot));
		}

		int sampleStep = nodes.size() / transformedControls.size();

		int i = 0;
		while (i < nodes.size())
		{
			writer->writeFitPoint(*dw,
				DL_FitPointData(nodes[i].x, nodes[i].y, 0)
			);
			i += sampleStep;
		}

		for (auto& controlPoint : transformedControls)
		{
			writer->writeControlPoint(*dw,
				DL_ControlPointData(controlPoint.x, controlPoint.y, 0, 1)
			);
		}
	}

	int DXFProcessor::read(const std::string& dxfFile)
	{
		psketchGPU.lock()->source = dxfFile;
		QApplication::setOverrideCursor(Qt::WaitCursor);
		DxfReader* dxfReader;
		if (psketchGPU.lock())
		{
			//重置索引计数器
			EntityVGPU::counter = 0;
			psketchGPU.lock()->source = dxfFile;
			dxfReader = new DxfReader(psketchGPU);
		}
		else
		{
			dxfReader = new DxfReader(psketchCPU);
		}

		DL_Dxf* dxf = new DL_Dxf();
		if (!dxf->in(dxfFile, dxfReader))
		{
			QApplication::restoreOverrideCursor();
			return 1;
		}

		{//收尾工作
			if (vertexSize != 0)
			{
				if (psketchGPU.lock())
				{
					Polyline2DGPU* polyline = new Polyline2DGPU(vertexs, polylineClose, bulges);
					if (openRingDetect)
					{
						entityStorage.push_back(polyline);
					}
					else
					{
						psketchGPU.lock()->AddEntity(polyline);
					}
				}
				else if (psketchCPU.lock())
				{
					Polyline2DCPU* polyline = new Polyline2DCPU(vertexs, polylineClose);
					psketchCPU.lock()->AddEntity(polyline);
				}
			}
			vertexSize = 0;
			vertexs.clear();
			bulges.clear();
		}
		


		if (openRingDetect)
		{
			std::vector<EntRingConnection*> rings = RingDetector::RingDetect(entityStorage);
			PartClassifier classifier(rings);
			std::vector<EntGroup*> groups = classifier.Execute();
			int ringsize = 0;

			for (EntGroup* group : groups)
			{
				psketchGPU.lock()->AddEntityGroup(group);
				ringsize += group->rings.size();
			}

			entityStorage.clear();
			psketchGPU.lock()->UpdateSketch();

			psketchGPU.lock()->enitiySize += psketchGPU.lock()->GetEntities().size();
			psketchGPU.lock()->contourSize += ringsize;
			//路径优化算法
			PathOptimizer optimizer(groups);
			optimizer.Run();

		}



		if (psketchGPU.lock()->GetEntities().size() == 0)
		{
			QMessageBox::information(NULL, QObject::tr("错误"), QObject::tr("dxf格式有误"));
		}

		if (psketchCPU.lock())
		{
			psketchCPU.lock()->GetCanvas()->ResetView();
		}
		if (onComplete)
			onComplete();

		delete dxf;
		delete dxfReader;
		QApplication::restoreOverrideCursor();
		return 0;
	}
	int DXFProcessor::write(const std::string& dxfFile)
	{
		DL_Dxf* dxf = new DL_Dxf();
		DL_Codes::version exportVersion = DL_Codes::DL_VERSION_2000;
		DL_WriterA* dw = dxf->out(dxfFile.c_str(), exportVersion);
		if (dw == NULL)
		{
			return 1;
		}

		dxf->writeHeader(*dw);
		dw->sectionEnd();
		dw->sectionTables();
		dxf->writeVPort(*dw);

		dw->tableLinetypes(3);
		dxf->writeLinetype(*dw, DL_LinetypeData("BYBLOCK", "BYBLOCK", 0, 0, 0.0));
		dxf->writeLinetype(*dw, DL_LinetypeData("BYLAYER", "BYLAYER", 0, 0, 0.0));
		dw->tableEnd();

		dxf->writeLayer(*dw,
			DL_LayerData("0", 0),
			DL_Attributes(
				std::string(""),      // leave empty
				DL_Codes::black,        // default color
				100,                  // default width
				"CONTINUOUS", 1.0));       // default line style

		dxf->writeLayer(*dw,
			DL_LayerData("mainlayer", 0),
			DL_Attributes(
				std::string(""),
				DL_Codes::red,
				100,
				"CONTINUOUS", 1.0));

		dxf->writeLayer(*dw,
			DL_LayerData("anotherlayer", 0),
			DL_Attributes(
				std::string(""),
				DL_Codes::black,
				100,
				"CONTINUOUS", 1.0));

		dw->tableEnd();

		dw->tableStyle(1);
		dxf->writeStyle(*dw, DL_StyleData("standard", 0, 2.5, 1.0, 0.0, 0, 2.5, "txt", ""));
		dw->tableEnd();

		dxf->writeView(*dw);
		dxf->writeUcs(*dw);

		dw->tableAppid(1);
		dxf->writeAppid(*dw, "WG");
		dw->tableEnd();

		dxf->writeDimStyle(*dw, 1, 1, 1, 1, 1);

		if (psketchGPU.lock())
		{
			for (EntityVGPU* ent : psketchGPU.lock()->GetEntities())
			{
				switch (ent->GetType())
				{
				case EntityType::Point:
				{

				}
				}
			}
		}

		return 0;
	}
}