#include "kaun.hpp"

namespace kaun {
    VertexFormat defaultVertexFormat;
    RenderState defaultRenderState;

    void init(bool loadGl) {
        setupDefaultLogging();

        if(loadGl) {
            if(!gladLoadGL()) {
                LOG_CRITICAL("Failed to initialize GLAD!");
            }
        }

        defaultVertexFormat
            .add(kaun::AttributeType::POSITION, 3, kaun::AttributeDataType::F32)
            .add(kaun::AttributeType::NORMAL, 3, kaun::AttributeDataType::F32)
            .add(kaun::AttributeType::TEXCOORD0, 2, kaun::AttributeDataType::F32);
    }
}
