#include "kaun.hpp"

namespace kaun {
    VertexFormat defaultVertexFormat;

    void init() {
        setupDefaultLogging();

        defaultVertexFormat
            .add(kaun::AttributeType::POSITION, 3, kaun::AttributeDataType::F32)
            .add(kaun::AttributeType::NORMAL, 3, kaun::AttributeDataType::F32)
            .add(kaun::AttributeType::TEXCOORD0, 2, kaun::AttributeDataType::F32);
    }
}
