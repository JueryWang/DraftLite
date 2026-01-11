#pragma once
#include "Graphics/DrawEntity.h"

using namespace CNCSYS;

class Polyline2DGPU : public EntityVGPU {
public:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& nodes;
        ar& bulges;
        ar& worldModelMatrix;
        ar& isVisible;
        ar& isClosed;
        ar& createGCode;
        ar& ringId;
    }

    Polyline2DGPU();
    Polyline2DGPU(Polyline2DGPU* other);
    Polyline2DGPU(const std::vector<glm::vec3>& points, bool isClosed, const std::vector<float> bulges = {});
    ~Polyline2DGPU();
    void Copy(Polyline2DGPU* other);
    virtual void UpdatePaintData() override;
    virtual EntityType GetType() const override { return EntityType::Polyline; }
    virtual glm::vec3 GetStart() override;
    virtual glm::vec3 GetEnd() override;
    virtual void Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode) override;
    virtual void Move(const glm::vec3& offset) override;
    virtual void MoveTo(const glm::vec3& pos) override;
    virtual void Rotate(const glm::vec3& center, float angle) override;
    virtual void Scale(const glm::vec3& scalar, const glm::vec3& center) override;
    virtual void Mirror(const glm::vec3& linePt1, const glm::vec3& linePt2) override;
    virtual void SetStartPoint(int index) override;
    virtual void Reverse() override;
    virtual glm::vec3 Evaluate(float t) override;
    virtual glm::vec3 Derivative(float t) override;
    virtual float Curvature(float t) override;
    virtual float CurvatureRadius(float t) override;
    virtual std::vector<glm::vec3> GetTransformedNodes();
    virtual std::vector<glm::vec3> SplitToSection(float precision) override;
    virtual std::string ToNcInstruction(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
    virtual std::string GenNcSection(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
    virtual QString Description() override;
    void SetParameter(const std::vector<glm::vec3>& nodes, bool isClosed, const std::vector<float> bulges);
    void Simplify(float epsilon);
    void Smooth(float epsilon);

private:
    float cumulativeLength(double t0, double t1, double eps = 1e-4);
    float findT(const float lastT, const int precision);
    std::vector<glm::vec3> GetBulgeSamples(const glm::vec3& node1, const glm::vec3& node2, float bulge);
    void GenerateBulgeSamples(std::vector<glm::vec3> nodes,const std::vector<float> bulges,std::vector<glm::vec3>& bulgedSamples);

public:
    std::vector<glm::vec3> nodes;
    std::vector<float> bulges;
    std::vector<glm::vec3> polylineBulgedSamples;
    //nodes的索引到bulgeSamples的映射
    std::map<int, int> bulgeIndexMapper;

    bool isLineBulged;
    bool isClosed;
};