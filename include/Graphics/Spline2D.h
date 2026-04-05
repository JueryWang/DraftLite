#pragma once
#include "Graphics/DrawEntity.h"
#include "Path/Path.h"

using namespace CNCSYS;

class Spline2DGPU : public EntityVGPU {
public:
    friend class boost::serialization::access;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar& controlPoints;
        ar& knots;
        ar& worldModelMatrix;
        ar& isVisible;
        ar& createGCode;
        ar& ringId;
    }

    Spline2DGPU();
    Spline2DGPU(Spline2DGPU* other);
    Spline2DGPU(const std::vector<glm::vec3>& controlPoints, const std::vector<float> knots, bool isPassTrough);
    ~Spline2DGPU();
    void Copy(Spline2DGPU* other);
    virtual void UpdatePaintData() override;
    virtual EntityType GetType() const override { return EntityType::Spline; }
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
    virtual QString Description() override;
    virtual std::string ToNcInstruction(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
    virtual std::string GenNcSection(SimulateStatus* Mstatus, bool createRecord = false, SketchGPU* sketch = nullptr) override;
    void SetParameter(const std::vector<glm::vec3>& controlpoints, const std::vector<float>& knots, bool isPassthrough);
private:
    void GenerateSplineSamplePoints(const std::vector<glm::vec3>& controlPoints, std::vector<glm::vec3>& samples);
    float cumulativeLength(double t0, double t1, double eps = 1e-4);
    float findT(const float lastT, const int precision);
public:
    GLuint vao_cntl;
    GLuint vbo_cntl;
    std::vector<glm::vec3> controlPoints;
    std::vector<glm::vec3> splineSamples;
	std::vector<PathNode> pathNodes;
    std::vector <float> samplesT;
    std::vector<float> knots;
    bool isPassthrough = false;
    const int degree = 3;

    float maxSectionRatio = FLT_MIN;
};