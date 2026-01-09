#pragma once
#include "Graphics/DrawEntity.h"

using namespace CNCSYS;

class Arc2DGPU : public EntityVGPU {
public:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& start;
        ar& end;
        ar& center;
        ar& radius;
        ar& startAngle;
        ar& endAngle;
        ar& worldModelMatrix;
        ar& isVisible;
        ar& ringId;
    }

    Arc2DGPU();
    Arc2DGPU(Arc2DGPU* other);
    Arc2DGPU(glm::vec3 center, float radius, float startAngle, float endAngle);
    ~Arc2DGPU();
    void Copy(Arc2DGPU* other);
    virtual void UpdatePaintData() override;
    virtual EntityType GetType() const override { return EntityType::Arc; }
    virtual void Paint(Shader* shader, OCSGPU* ocsSys, RenderMode mode) override;
    virtual glm::vec3 GetStart() override;
    virtual glm::vec3 GetEnd() override;
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
    virtual std::vector<glm::vec3> SplitToSection(float precision) override;
    virtual std::vector<glm::vec3> GetTransformedNodes();
    virtual std::string ToNcInstruction(SimulateStatus* Mstatus, bool createRecord = false,SketchGPU* sketch = nullptr) override;
    virtual std::string GenNcSection(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
    virtual QString Description() override;
    void SetParameter(const glm::vec3& center, float startAngle, float endAngle, float radius);
    void GenerateArcSamples(float startAngle, float endAngle, const glm::vec3& center, std::vector<glm::vec3>& samples);

public:
    glm::vec3 start;
    glm::vec3 end;
    glm::vec3 center;
    float radius;
    float startAngle;
    float endAngle;
    std::vector<glm::vec3> arcSamples;
};