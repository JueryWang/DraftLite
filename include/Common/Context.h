#pragma once
#include <string>

#include "ModalEvent/ModalEvent.h"
#include "glm/glm.hpp"

namespace CNCSYS
{
    enum LineType
    {
        Continious,
        DashedLine,
        DashedLineWithInterp,
    };

    struct Layer
    {
        std::string name;
        float lineWeight;
        glm::vec4 color;
        LineType lineType;
    };

    static ModalState operator|(ModalState a, ModalState b) {
        return static_cast<ModalState>(
            static_cast<uint64_t>(a) | static_cast<uint64_t>(b)
        );
    }

    static ModalState operator&(ModalState a, ModalState b) {
        return static_cast<ModalState>(
            static_cast<uint64_t>(a) & static_cast<uint64_t>(b)
            );
    }
}
