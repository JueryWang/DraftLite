#pragma once
#include "glad/glad.h"
#include "glm/glm.hpp"
#include <vector>
#include <iostream>
#include <vector>

namespace CNCSYS
{
    enum TickType
    {
        Main,
        Sub
    };

    enum AxisType
    {
        X,
        Y
    };

    class AxisTicker
    {
    public:
        float value;
        glm::vec2 tickCoord[2];
        TickType Ttype;
        AxisType Atype;

    public:
        AxisTicker(float _value,const std::vector<glm::vec2>& oglPosition, TickType _Ttype, AxisType _AType)
        {
            tickCoord[0] = oglPosition[0];
            tickCoord[1] = oglPosition[1];
            value = _value;
            Ttype = _Ttype;
            Atype = _AType;
        }
    };
}