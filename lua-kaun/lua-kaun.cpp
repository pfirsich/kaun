extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}
#include <LuaBridge/LuaBridge.h>
namespace lb = luabridge;

#include <kaun.hpp>

#include "luax.hpp"
#include "glstate.hpp"

static const char kaunLua[] =
#include "kaun.lua"
;

#define EXPORT __declspec(dllexport)

template <typename T>
int __gc(lua_State* L) {
    T* obj = lb::Userdata::get<T>(L, 1, false);
    delete obj;
    return 0;
}

template <typename T>
void pushWithGC(lua_State* L, T* obj) {
    lb::push(L, obj);
    lua_getmetatable(L, -1);
    lb::push(L, "__gc");
    lb::push(L, __gc<T>);
    lua_rawset(L, -3);
    lua_pop(L, 1); // pop metatable
}

std::pair<const uint8_t*, int> getFileData(lua_State* L, const char* filename) {
    lua_getglobal(L, "_kaun");
    lua_pushstring(L, "getFileData");
    lua_rawget(L, -2);
    lua_pushstring(L, filename);
    if(lua_pcall(L, 1, 2, 0)) {
        luaL_error(L, "Could not get file data: %s", lua_tostring(L, -1));
        return std::make_pair(nullptr, 0);
    } else {
        const uint8_t* fileData = reinterpret_cast<uint8_t*>(lua_touserdata(L, -2));
        int fileSize = luaL_checkint(L, -1);
        return std::make_pair(fileData, fileSize);
    }
}

struct TransformWrapper : public kaun::Transform {
    void setPosition(float x, float y, float z) {
        Transform::setPosition(glm::vec3(x, y, z));
    }

    int getPosition(lua_State* L) {
        luax_pushvec3(L, Transform::getPosition());
        return 3;
    }

    int getForward(lua_State* L) {
        luax_pushvec3(L, Transform::getForward());
        return 3;
    }

    int getUp(lua_State* L) {
        luax_pushvec3(L, Transform::getUp());
        return 3;
    }

    int getRight(lua_State* L) {
        luax_pushvec3(L, Transform::getRight());
        return 3;
    }

    void rotate(float angle, float axisX, float axisY, float axisZ) {
        Transform::rotate(angle, glm::vec3(axisX, axisY, axisZ));
    }

    void rotateWorld(float angle, float axisX, float axisY, float axisZ) {
        Transform::rotateWorld(angle, glm::vec3(axisX, axisY, axisZ));
    }

    int localDirToWorld(lua_State* L) {
        luax_pushvec3(L, Transform::localDirToWorld(luax_check<glm::vec3>(L, 2)));
        return 3;
    }

    int lookAt(lua_State* L) {
        int nargs = lua_gettop(L);
        if(nargs == 4) {
            Transform::lookAt(luax_check<glm::vec3>(L, 2));
        } else if(nargs == 7) {
            Transform::lookAt(luax_check<glm::vec3>(L, 2), luax_check<glm::vec3>(L, 5));
        } else {
            luaL_error(L, "Number of arguments to Transform:lookAt must be 7 or 10. Got %d", nargs);
        }
        return 0;
    }

    int lookAtPos(lua_State* L) {
        int args = lua_gettop(L);
	    if(args == 7) {
            Transform::lookAtPos(luax_check<glm::vec3>(L, 2), luax_check<glm::vec3>(L, 5));
	    } else if(args == 10) {
            Transform::lookAtPos(luax_check<glm::vec3>(L, 2), luax_check<glm::vec3>(L, 5), luax_check<glm::vec3>(L, 8));
	    } else {
		    luaL_error(L, "Number of arguments to Transform:lookAtPos must be 7 or 10. Got %d", args);
	    }
        return 0;
    }

    static TransformWrapper newTransform() {
        return TransformWrapper();
    }
};

class Trash {
private:
    int x;
public:
    Trash() : x(0) { std::printf("construct %p\n", this); }
    Trash(const Trash& other) : x(other.x) { std::printf("copy construct %p\n", this); }
    ~Trash() { std::printf("destruct %p\n", this); }
    void set(int v) { x = v; }
    int get() const { return x; }

    static Trash newTrash() {
        return Trash();
    }

    static Trash* newTrashP() {
        return new Trash();
    }

    static int newTrashS(lua_State* L) {
        pushWithGC(L, new Trash());
        return 1;
    }
};

LuaEnum<kaun::AttributeType> attributeType("attribute type", {
    {"POSITION", kaun::AttributeType::POSITION},
    {"NORMAL", kaun::AttributeType::NORMAL},
    {"TANGENT", kaun::AttributeType::TANGENT},
    {"BITANGENT", kaun::AttributeType::BITANGENT},
    {"COLOR0", kaun::AttributeType::COLOR0},
    {"COLOR1", kaun::AttributeType::COLOR1},
    {"BONEINDICES", kaun::AttributeType::BONEINDICES},
    {"BONEWEIGHTS", kaun::AttributeType::BONEWEIGHTS},
    {"TEXCOORD0", kaun::AttributeType::TEXCOORD0},
    {"TEXCOORD1", kaun::AttributeType::TEXCOORD1},
    {"TEXCOORD2", kaun::AttributeType::TEXCOORD2},
    {"TEXCOORD3", kaun::AttributeType::TEXCOORD3},
    {"CUSTOM0", kaun::AttributeType::CUSTOM0},
    {"CUSTOM1", kaun::AttributeType::CUSTOM1},
    {"CUSTOM2", kaun::AttributeType::CUSTOM2},
    {"CUSTOM3", kaun::AttributeType::CUSTOM3},
    {"CUSTOM4", kaun::AttributeType::CUSTOM4},
    {"CUSTOM5", kaun::AttributeType::CUSTOM5},
    {"CUSTOM6", kaun::AttributeType::CUSTOM6},
    {"CUSTOM7", kaun::AttributeType::CUSTOM7},
});

LuaEnum<kaun::AttributeDataType> attributeDataType("attribute data type", {
    {"I8", kaun::AttributeDataType::I8},
    {"UI8", kaun::AttributeDataType::UI8},
    {"I16", kaun::AttributeDataType::I16},
    {"UI16", kaun::AttributeDataType::UI16},
    {"I32", kaun::AttributeDataType::I32},
    {"UI32", kaun::AttributeDataType::UI32},
    {"F32", kaun::AttributeDataType::F32},
});

struct VertexFormatWrapper : public kaun::VertexFormat {
    static int newVertexFormat(lua_State* L) {
        int nargs = lua_gettop(L);
        if(nargs == 0) luaL_error(L, "Vertex format needs at least one attribute.");
        VertexFormatWrapper* format = reinterpret_cast<VertexFormatWrapper*>(new kaun::VertexFormat());

        for(int arg = 1; arg <= nargs; ++arg) {
            if(!lua_istable(L, arg)) luaL_typerror(L, 1, "table");
            int len = lua_objlen(L, arg);
            if(len < 3) luaL_error(L, "Vertex attribute table needs at least 3 elements: type, count, data type");

            for(int i = 1; i <= 3; ++i) lua_rawgeti(L, arg, i);
            kaun::AttributeType type = attributeType.check(L, -3);
            int count = luaL_checkint(L, -2);
            if(count < 1 || count >= 4) luaL_error(L, "count has to be 1, 2, 3 or 5");
            kaun::AttributeDataType dataType = attributeDataType.check(L, -1);
            lua_pop(L, 3);

            bool normalized = false;
            if(len >= 4) {
                lua_rawgeti(L, arg, 4);
                if(!lua_isboolean(L, -1)) luaL_error(L, "Fourth element in attribute table ('normalized') has to be boolean.");
                normalized = lua_toboolean(L, -1);
                lua_pop(L, 1);
            }

            unsigned int divisor = 0;
            if(len >= 5) {
                lua_rawgeti(L, arg, 5);
                divisor = luaL_checkint(L, -1);
                lua_pop(L, 1);
            }

            format->add(type, count, dataType, normalized, divisor);
        }

        pushWithGC(L, format);
        return 1;
    }
};

LuaEnum<kaun::Mesh::DrawMode> meshDrawMode("mesh draw mode", {
    {"points", kaun::Mesh::DrawMode::POINTS},
    {"lines", kaun::Mesh::DrawMode::LINES},
    {"line_loop", kaun::Mesh::DrawMode::LINE_LOOP},
    {"line_strip", kaun::Mesh::DrawMode::LINE_STRIP},
    {"triangles", kaun::Mesh::DrawMode::TRIANGLES},
    {"triangle_fan", kaun::Mesh::DrawMode::TRIANGLE_FAN},
    {"triangle_strip", kaun::Mesh::DrawMode::TRIANGLE_STRIP},
});

LuaEnum<kaun::UsageHint> usageHint("usage hint", {
    {"static", kaun::UsageHint::STATIC},
    {"stream", kaun::UsageHint::STREAM},
    {"dynamic", kaun::UsageHint::DYNAMIC},
});

struct MeshWrapper : public kaun::Mesh {
    // This function assumes the element at idx is already a table
    int setVerticesInternal(lua_State* L, int idx) {
        auto vertexBuffer = getVertexBuffers()[0];
        auto format = vertexBuffer->getVertexFormat();
        auto attributes = format.getAttributes();
        int components = 0;
        for(auto& attr : attributes) components += attr.num;

        // verfiy vertex table format
        size_t vertexCount = lua_objlen(L, idx);
        if(vertexBuffer->getNumVertices() < vertexCount) vertexBuffer->reallocate(vertexCount);
        std::vector<float> data;
        for(size_t v = 1; v <= vertexCount; ++v) {
            lua_rawgeti(L, idx, v);
            if(!lua_istable(L, -1))
                luaL_error(L, "Each vertex has to be a table");
            if(lua_objlen(L, -1) != components)
                luaL_error(L, "Each vertex table needs a number of values equal to the number of components in the vertex format (here: %d)", components);

            for(int i = 1; i <= components; ++i) {
                lua_rawgeti(L, -1, i);
                data.push_back(luax_check<float>(L, -1));
                lua_pop(L, 1);
            }

            lua_pop(L, 1);
        }

        int c = 0;
        for(auto& attr : attributes) {
            switch(attr.num) {
                case 1: {
                    auto accessor = getAccessor<float>(attr.type);
                    for(size_t v = 0; v < vertexCount; ++v) {
                        accessor.set(v, data[v * components + c]);
                    }
                    break;
                }
                case 2: {
                    auto accessor = getAccessor<glm::vec2>(attr.type);
                    for(size_t v = 0; v < vertexCount; ++v) {
                        accessor.set(v, glm::make_vec2(&data[0] + v * components + c));
                    }
                    break;
                }
                case 3: {
                    auto accessor = getAccessor<glm::vec3>(attr.type);
                    for(size_t v = 0; v < vertexCount; ++v) {
                        accessor.set(v, glm::make_vec3(&data[0] + v * components + c));
                    }
                    break;
                }
                case 4: {
                    auto accessor = getAccessor<glm::vec4>(attr.type);
                    for(size_t v = 0; v < vertexCount; ++v) {
                        accessor.set(v, glm::make_vec4(&data[0] + v * components + c));
                    }
                    break;
                }
            }
            c += attr.num;
        }

        return 0;
    }

    int setVertices(lua_State* L) {
        if(!lua_istable(L, 1)) luaL_typerror(L, 1, "table");
        return setVerticesInternal(L, 1);
    }

    static int newMesh(lua_State* L) {
        // mode, vertexFormat
        // mode, vertexFormat, vertices, (usage)
        // mode, vertexFormat, vertexCount, (usage)

        kaun::Mesh::DrawMode mode = meshDrawMode.check(L, 1);
        VertexFormatWrapper* format = lb::Userdata::get<VertexFormatWrapper>(L, 2, true);
        kaun::UsageHint usage = kaun::UsageHint::STATIC;
        if(usageHint.is(L, 4)) usage = usageHint.check(L, 4);

        MeshWrapper* mesh = reinterpret_cast<MeshWrapper*>(new kaun::Mesh(mode));

        int nargs = lua_gettop(L);
        if(nargs == 2) {
            mesh->addVertexBuffer(*format, usage);
        } else if(nargs >= 3) {
            if(lua_isnumber(L, 3)) {
                int vertexCount = luaL_checkint(L, 3);
                mesh->addVertexBuffer(*format, vertexCount, usage);
            } else if(lua_istable(L, 3)) {
                int vertexCount = lua_objlen(L, 3);
                mesh->addVertexBuffer(*format, vertexCount, usage);
                mesh->setVerticesInternal(L, 3);
            } else {
                luaL_typerror(L, 3, "table or integer");
            }
        }

        pushWithGC(L, mesh);
        return 1;
    }

    static int newBoxMesh(lua_State* L) {
        int args = lua_gettop(L);
        if(args == 3 || args == 4) {
            float width = luax_check<float>(L, 1);
            float height = luax_check<float>(L, 2);
            float depth = luax_check<float>(L, 3);
            if(args == 4) {
                VertexFormatWrapper* format = lb::Userdata::get<VertexFormatWrapper>(L, 4, true);
                pushWithGC(L, reinterpret_cast<MeshWrapper*>(kaun::Mesh::box(width, height, depth, *format)));
            } else {
                pushWithGC(L, reinterpret_cast<MeshWrapper*>(kaun::Mesh::box(width, height, depth, kaun::defaultVertexFormat)));
            }
            return 1;
        } else {
            luaL_error(L, "Number of arguments to kaun.newBoxMesh must be 3 or 4. Got %d", args);
            return 0;
        }
    }

    static int newPlaneMesh(lua_State* L) {
        int nargs = lua_gettop(L);
        float width = luax_check<float>(L, 1);
        float depth = luax_check<float>(L, 2);
        int segmentsX = 1;
        int segmentsZ = 1;
        if(nargs >= 3) {
            segmentsZ = segmentsX = luaL_checkint(L, 3);
            if(nargs >= 4) segmentsZ = luaL_checkint(L, 4);
        }
        if(nargs >= 5) {
            VertexFormatWrapper* format = lb::Userdata::get<VertexFormatWrapper>(L, 5, true);
            pushWithGC(L, reinterpret_cast<MeshWrapper*>(kaun::Mesh::plane(width, depth, segmentsX, segmentsZ, *format)));
        } else {
            pushWithGC(L, reinterpret_cast<MeshWrapper*>(kaun::Mesh::plane(width, depth, segmentsX, segmentsZ, kaun::defaultVertexFormat)));
        }
        return 1;
    }
};

struct ShaderWrapper : public kaun::Shader {
    static int newShader(lua_State* L) {
        size_t fragLen;
        const char* fragStr = luaL_checklstring(L, 1, &fragLen);
        size_t vertLen;
        const char* vertStr = luaL_checklstring(L, 2, &vertLen);
        ShaderWrapper* shader = new ShaderWrapper();
        shader->compileAndLinkStrings(fragStr, vertStr);
        pushWithGC(L, shader);
        return 1;
    }
};

LuaEnum<kaun::RenderState::DepthFunc> depthFunc("depth func", {
    {"disabled", kaun::RenderState::DepthFunc::DISABLED},
    {"never", kaun::RenderState::DepthFunc::NEVER},
    {"less", kaun::RenderState::DepthFunc::LESS},
    {"equal", kaun::RenderState::DepthFunc::EQUAL},
    {"lequal", kaun::RenderState::DepthFunc::LEQUAL},
    {"greater", kaun::RenderState::DepthFunc::GREATER},
    {"notequal", kaun::RenderState::DepthFunc::NOTEQUAL},
    {"gequal", kaun::RenderState::DepthFunc::GEQUAL},
    {"always", kaun::RenderState::DepthFunc::ALWAYS},
});

LuaEnum<kaun::RenderState::FaceDirections> faceDirections("face direction", {
    {"none", kaun::RenderState::FaceDirections::NONE},
    {"front", kaun::RenderState::FaceDirections::FRONT},
    {"back", kaun::RenderState::FaceDirections::BACK},
});

LuaEnum<kaun::RenderState::FaceOrientation> faceOrientation("face winding", {
    {"cw", kaun::RenderState::FaceOrientation::CW},
    {"ccw", kaun::RenderState::FaceOrientation::CCW},
});

LuaEnum<kaun::RenderState::BlendFactor> blendFactor("blend factor", {
    {"zero", kaun::RenderState::BlendFactor::ZERO},
    {"one", kaun::RenderState::BlendFactor::ONE},
    {"src_color", kaun::RenderState::BlendFactor::SRC_COLOR},
    {"one_minus_src_color", kaun::RenderState::BlendFactor::ONE_MINUS_SRC_COLOR},
    {"dst_color", kaun::RenderState::BlendFactor::DST_COLOR},
    {"one_minus_dst_color", kaun::RenderState::BlendFactor::ONE_MINUS_DST_COLOR},
    {"src_alpha", kaun::RenderState::BlendFactor::SRC_ALPHA},
    {"one_minus_src_alpha", kaun::RenderState::BlendFactor::ONE_MINUS_SRC_ALPHA},
    {"dst_alpha", kaun::RenderState::BlendFactor::DST_ALPHA},
    {"one_minus_dst_alpha", kaun::RenderState::BlendFactor::ONE_MINUS_DST_ALPHA},
    {"constant_color", kaun::RenderState::BlendFactor::CONSTANT_COLOR},
    {"one_minus_constant_color", kaun::RenderState::BlendFactor::ONE_MINUS_CONSTANT_COLOR},
    {"constant_alpha", kaun::RenderState::BlendFactor::CONSTANT_ALPHA},
    {"one_minus_constant_alpha", kaun::RenderState::BlendFactor::ONE_MINUS_CONSTANT_ALPHA},
});

LuaEnum<kaun::RenderState::BlendEq> blendEquation("blend equation", {
    {"add", kaun::RenderState::BlendEq::ADD},
    {"subtract", kaun::RenderState::BlendEq::SUBTRACT},
    {"reverse_subtract", kaun::RenderState::BlendEq::REVERSE_SUBTRACT},
    {"min", kaun::RenderState::BlendEq::MIN},
    {"max", kaun::RenderState::BlendEq::MAX},
});

enum class BlendingMode {
    DISABLED,
    ALPHA,
    ADDITIVE,
    SCREEN,
};

LuaEnum<BlendingMode> blendingMode("blending mode", {
    {"replace", BlendingMode::DISABLED},
    {"alpha", BlendingMode::ALPHA},
    {"additive", BlendingMode::ADDITIVE},
    {"screen", BlendingMode::SCREEN},
});

struct RenderStateWrapper : public kaun::RenderState {
    int setDepthWrite(lua_State* L) {
        RenderState::setDepthWrite(luax_check<bool>(L, 2));
        return 0;
    }

    int getDepthTest(lua_State* L) {
        lua_pushstring(L, depthFunc.getValue(RenderState::getDepthTest()).c_str());
        return 1;
    }

    int setDepthTest(lua_State* L) {
        RenderState::setDepthTest(depthFunc.check(L, 2));
        return 0;
    }

    int setBlendEnabled(lua_State* L) {
        RenderState::setBlendEnabled(luax_check<bool>(L, 2));
        return 0;
    }

    int getBlendFactors(lua_State* L) {
        auto factors = RenderState::getBlendFactors();
        lua_pushstring(L, blendFactor.getValue(factors.first).c_str());
        lua_pushstring(L, blendFactor.getValue(factors.second).c_str());
        return 2;
    }

    int setBlendFactors(lua_State* L) {
        RenderState::setBlendFactors(blendFactor.check(L, 2), blendFactor.check(L, 3));
        return 0;
    }

    int getCullFaces(lua_State* L) {
        lua_pushstring(L, faceDirections.getValue(RenderState::getCullFaces()).c_str());
        return 1;
    }

    int setCullFaces(lua_State* L) {
        RenderState::setCullFaces(faceDirections.check(L, 2));
        return 0;
    }

    int setBlending(lua_State* L) {
        BlendingMode mode = blendingMode.check(L, 2);
        switch(mode) {
            case BlendingMode::DISABLED:
                RenderState::setBlendEnabled(false);
                break;
            case BlendingMode::ALPHA:
                RenderState::setBlendEnabled(true);
                RenderState::setBlendEquation(RenderState::BlendEq::ADD);
                RenderState::setBlendFactors(RenderState::BlendFactor::SRC_ALPHA,
                                             RenderState::BlendFactor::ONE_MINUS_SRC_ALPHA);
                break;
            case BlendingMode::ADDITIVE:
                RenderState::setBlendEnabled(true);
                RenderState::setBlendEquation(RenderState::BlendEq::ADD);
                RenderState::setBlendFactors(RenderState::BlendFactor::SRC_ALPHA,
                                             RenderState::BlendFactor::ONE);
                break;
            case BlendingMode::SCREEN:
                RenderState::setBlendEnabled(true);
                RenderState::setBlendEquation(RenderState::BlendEq::ADD);
                RenderState::setBlendFactors(RenderState::BlendFactor::SRC_ALPHA,
                                             RenderState::BlendFactor::ONE_MINUS_SRC_COLOR);
                break;
        }
        return 0;
    }

    static int newRenderState(lua_State* L) {
        RenderStateWrapper* state = new RenderStateWrapper;
        pushWithGC(L, state);
        return 1;
    }
};

LuaEnum<kaun::Texture::WrapMode> wrapMode("wrap mode", {
    {"clamp_to_edge", kaun::Texture::WrapMode::CLAMP_TO_EDGE},
    {"clamp_to_border", kaun::Texture::WrapMode::CLAMP_TO_BORDER},
    {"mirrored_repeat", kaun::Texture::WrapMode::MIRRORED_REPEAT},
    {"repeat", kaun::Texture::WrapMode::REPEAT},
});

LuaEnum<kaun::Texture::MinFilter> minFilter("min filter", {
    {"nearest", kaun::Texture::MinFilter::NEAREST},
    {"linear", kaun::Texture::MinFilter::LINEAR},
    {"nearest_mipmap_nearest", kaun::Texture::MinFilter::NEAREST_MIPMAP_NEAREST},
    {"linear_mipmap_nearest", kaun::Texture::MinFilter::LINEAR_MIPMAP_NEAREST},
    {"nearest_mipmap_linear", kaun::Texture::MinFilter::NEAREST_MIPMAP_LINEAR},
    {"linear_mipmap_linear", kaun::Texture::MinFilter::LINEAR_MIPMAP_LINEAR},
});

LuaEnum<kaun::Texture::MagFilter> magFilter("mag filter", {
    {"nearest", kaun::Texture::MagFilter::NEAREST},
    {"linear", kaun::Texture::MagFilter::LINEAR},
});

struct TextureWrapper : public kaun::Texture {
    int getDimensions(lua_State* L) {
        lua_pushinteger(L, Texture::getWidth());
        lua_pushinteger(L, Texture::getHeight());
        return 2;
    }

    int getWrap(lua_State* L) {
        auto wrap = Texture::getWrap();
        lua_pushstring(L, wrapMode.getValue(wrap.first).c_str());
        lua_pushstring(L, wrapMode.getValue(wrap.second).c_str());
        return 2;
    }

    int setWrap(lua_State* L) {
        Texture::setWrap(wrapMode.check(L, 2), wrapMode.check(L, 3));
        return 0;
    }

    int getFilter(lua_State* L) {
        lua_pushstring(L, minFilter.getValue(Texture::getMinFilter()).c_str());
        lua_pushstring(L, magFilter.getValue(Texture::getMagFilter()).c_str());
        return 2;
    }

    int setFilter(lua_State* L) {
        Texture::setMinFilter(minFilter.check(L, 2));
        Texture::setMagFilter(magFilter.check(L, 3));
        return 0;
    }

    static int newTexture(lua_State* L) {
        const char* path = luaL_checklstring(L, 1, nullptr);
        auto fileData = getFileData(L, path);
        if(fileData.first != nullptr) {
            TextureWrapper* texture = new TextureWrapper;
            texture->loadEncodedFromMemory(fileData.first, fileData.second, false);
            pushWithGC(L, texture);
            return 1;
        } else {
            luaL_error(L, "Could not load file %s", path);
            return 0;
        }
    }

    static int newCheckerTexture(lua_State* L) {
        int args = lua_gettop(L);
        if(args == 3 || args == 9 || args == 11) {
            int width = luaL_checkint(L, 1);
            int height = luaL_checkint(L, 2);
            int checkerSize = luaL_checkint(L, 3);
            if(args == 11) {
                glm::vec4 colA = luax_check<glm::vec4>(L, 4);
                glm::vec4 colB = luax_check<glm::vec4>(L, 8);
                pushWithGC(L, reinterpret_cast<TextureWrapper*>(kaun::Texture::checkerBoard(width, height, checkerSize, colA, colB)));
            } else if(args == 9) {
                glm::vec4 colA = glm::vec4(luax_check<glm::vec3>(L, 4), 1.0f);
                glm::vec4 colB = glm::vec4(luax_check<glm::vec3>(L, 7), 1.0f);
                pushWithGC(L, reinterpret_cast<TextureWrapper*>(kaun::Texture::checkerBoard(width, height, checkerSize, colA, colB)));
            } else {
                pushWithGC(L, reinterpret_cast<TextureWrapper*>(kaun::Texture::checkerBoard(width, height, checkerSize)));
            }
            return 1;
        } else {
            luaL_error(L, "Number of arguments to kaun.newCheckerTexture must be 3 or 9. Got %d", args);
            return 0;
        }
    }
};

struct RenderBufferWrapper : public kaun::RenderBuffer {

};

int clear(lua_State* L) {
	int args = lua_gettop(L);
	if(args == 0) {
		kaun::clear();
	} else if(args == 4) {
		kaun::clear(luax_check<glm::vec4>(L, 1));
	} else if(args == 5) {
        kaun::clear(luax_check<glm::vec4>(L, 1), luaL_checkinteger(L, 5));
	} else {
		luaL_error(L, "Number of arguments to kaun.clear must be 0, 4 or 5. Got %d", args);
	}
	return 0;
}

int clearDepth(lua_State* L) {
	int args = lua_gettop(L);
	if(args == 0) {
		kaun::clearDepth();
	} else if(args == 1) {
		kaun::clearDepth(luax_check<float>(L, 1));
	} else {
		luaL_error(L, "Number of arguments to kaun.clearDepth must be 0 or 1. Got %d", args);
	}
	return 0;
}

int setProjection(lua_State* L) {
    int args = lua_gettop(L);
    if(args == 4) {
        kaun::setProjection(glm::perspective(glm::radians(luax_check<float>(L, 1)),
                                             luax_check<float>(L, 2),
                                             luax_check<float>(L, 3),
                                             luax_check<float>(L, 4)));
    } else if(args == 6) {
        // ortho
    } else if(args == 16) {
        float mat[16];
        for(int i = 0; i < 16; ++i) {
            mat[i] = luax_check<float>(L, i);
        }
        kaun::setProjection(glm::make_mat4(mat));
    } else {
        luaL_error(L, "Number of arguments to kaun.setProjection must be 4, 6, or 16. Got %d", args);
    }
    return 0;
}

void setViewTransform(const TransformWrapper* trafo) {
    kaun::setViewTransform(*trafo);
}

void setModelTransform(const TransformWrapper* trafo) {
    kaun::setModelTransform(*trafo);
}

const char* typeNames[9] = {
    "nil", "boolean", "lightuserdata", "number", "string", "table", "function", "userdata", "thread"
};

void printStackInfo(lua_State* L) {
    for(int i = 1; i <= lua_gettop(L); ++i) {
        int type = lua_type(L, i);
        std::printf("Arg %d: type = %s\n", i, lua_typename(L, type));
        if(type == LUA_TSTRING) {
            std::printf("String: %s\n", luaL_checklstring(L, i, nullptr));
        }
    }
}

int draw(lua_State* L) {
    int args = lua_gettop(L);
    if(args == 3 || args == 4) {
        MeshWrapper* mesh = lb::Userdata::get<MeshWrapper>(L, 1, false);
        ShaderWrapper* shader = lb::Userdata::get<ShaderWrapper>(L, 2, false);

        // build uniform list
        std::vector<kaun::Uniform> uniforms;

        if(lua_istable(L, 3)) {
            lua_pushnil(L);
            while(lua_next(L, 3) != 0) {
                // lua_next pops key from stack, then pushes new key and value
                // => key is at -2, value at -1 (top)
                const char* name = luaL_checklstring(L, -2, nullptr);
                const kaun::UniformInfo& uniformInfo = shader->getUniformInfo(name);
                int uniformSize = uniformInfo.getSize();
                if(uniformSize > 1) {
                    if(lua_istable(L, -1)) {
                        int num = lua_objlen(L, -1);
                        if(num != uniformSize) {
                            luaL_error(L, "Number of elements in uniform table is not equal to the size of the uniform array (%d)", uniformSize);
                            return 0;
                        } else {
                            luaL_error(L, "Uniform arrays are not yet implemented yet.");
                            return 0;
                        }
                    } else {
                        luaL_typerror(L, 3, "table");
                        return 0;
                    }
                } else {
                    if(uniformInfo.exists()) {
                        switch(uniformInfo.getType()) {
                            case kaun::UniformInfo::UniformType::INT:
                                uniforms.emplace_back(name, luaL_checkint(L, -1));
                                break;
                            case kaun::UniformInfo::UniformType::FLOAT:
                                uniforms.emplace_back(name, luax_check<float>(L, -1));
                                break;
                            case kaun::UniformInfo::UniformType::VEC2:
                                uniforms.emplace_back(name, luax_checkvectable<glm::vec2>(L, -1));
                                break;
                            case kaun::UniformInfo::UniformType::VEC3:
                                uniforms.emplace_back(name, luax_checkvectable<glm::vec3>(L, -1));
                                break;
                            case kaun::UniformInfo::UniformType::VEC4:
                                uniforms.emplace_back(name, luax_checkvectable<glm::vec4>(L, -1));
                                break;
                            case kaun::UniformInfo::UniformType::MAT2:
                            case kaun::UniformInfo::UniformType::MAT3:
                            case kaun::UniformInfo::UniformType::MAT4:
                                luaL_error(L, "Uniform matrices are not yet implemented yet.");
                                return 0;
                            case kaun::UniformInfo::UniformType::SAMPLER2D: {
                                TextureWrapper* tex = lb::Userdata::get<TextureWrapper>(L, lua_gettop(L), false);
                                uniforms.emplace_back(name, *reinterpret_cast<kaun::Texture*>(tex));
                                break;
                            }
                            default:
                                luaL_error(L, "Attempting to set uniform of unsupported type.");
                                return 0;
                        }
                    } else {
                        // do nothing for now?
                    }
                }
                lua_pop(L, 1); // pop value
            }
        } else {
            luaL_typerror(L, 3, "table");
            return 0;
        }

        if(args == 4) {
            RenderStateWrapper* state = lb::Userdata::get<RenderStateWrapper>(L, 4, false);
            kaun::draw(*mesh, *shader, uniforms, *state);
        } else {
            kaun::draw(*mesh, *shader, uniforms);
        }
    } else {
        luaL_error(L, "Number of arguments to kaun.draw has to be 3 or 4. Got %d", args);
    }
    return 0;
}

void flush() {
    kaun::flush();
}

LoveGlState loveState;

// As it is now, we only do a bunch of glGet in endLoveGraphics,
// which is mostly called right before love.graphics.present
// i.e. before buffers are swapped.
// Bufferswaps usually involve syncing too (glFinish-like?),
// so that the hit we take from the sync (glFlush-like) induced by glGet
// is lessened.

void beginLoveGraphics() {
    kaun::flush();
    // we don't need to do save kaun's gl state here, since we
    // KNOW the state kaun has saved internally
    restoreLoveGlState(loveState);
}

void endLoveGraphics() {
    loveState = saveLoveGlState();
    // kaun's internal gl state is already saved in kaun, restore it
    kaun::ensureGlState();
}

void beginLoveEventPump() {
    restoreLoveViewportState(loveState);
}

void endLoveEventPump() {
    saveLoveViewportState(loveState);
    kaun::setViewport();
}

extern "C" EXPORT int luaopen_kaun(lua_State* L) {
    lb::getGlobalNamespace(L)
        .beginNamespace("kaun")

        .beginClass<TransformWrapper>("Transform")
        .addFunction("setPosition", &TransformWrapper::setPosition)
        .addCFunction("getPosition", &TransformWrapper::getPosition)
        .addFunction("rotate", &TransformWrapper::rotate)
        .addFunction("rotateWorld", &TransformWrapper::rotateWorld)
        .addCFunction("localDirToWorld", &TransformWrapper::localDirToWorld)
        .addCFunction("lookAt", &TransformWrapper::lookAt)
        .addCFunction("lookAtPos", &TransformWrapper::lookAtPos)
        .addCFunction("getForward", &TransformWrapper::getForward)
        .addCFunction("getUp", &TransformWrapper::getUp)
        .addCFunction("getRight", &TransformWrapper::getRight)
        .endClass()
        .addFunction("newTransform", TransformWrapper::newTransform)

        .beginClass<Trash>("Trash")
        .addFunction("set", &Trash::set)
        .addFunction("get", &Trash::get)
        .endClass()
        .addFunction("newTrash", Trash::newTrash)
        .addFunction("newTrashP", Trash::newTrashP)
        .addCFunction("newTrashS", Trash::newTrashS)

        .beginClass<VertexFormatWrapper>("VertexFormat")
        .endClass()
        .addCFunction("newVertexFormat", VertexFormatWrapper::newVertexFormat)

        .beginClass<MeshWrapper>("Mesh")
        .addCFunction("setVertices", &MeshWrapper::setVertices)
        .endClass()
        .addCFunction("newMesh", MeshWrapper::newMesh)
        .addCFunction("newBoxMesh", MeshWrapper::newBoxMesh)
        .addCFunction("newPlaneMesh", MeshWrapper::newPlaneMesh)

        .beginClass<ShaderWrapper>("Shader")
        .endClass()
        .addCFunction("newShader", ShaderWrapper::newShader)

        .beginClass<TextureWrapper>("Texture")
        .addFunction("getWidth", &kaun::Texture::getWidth)
        .addFunction("getHeight", &kaun::Texture::getHeight)
        .addCFunction("getDimensions", &TextureWrapper::getDimensions)
        .addCFunction("getWrap", &TextureWrapper::getWrap)
        .addCFunction("setWrap", &TextureWrapper::setWrap)
        .addCFunction("getFilter", &TextureWrapper::getFilter)
        .addCFunction("setFilter", &TextureWrapper::setFilter)
        .endClass()
        .addCFunction("newTexture", TextureWrapper::newTexture)
        .addCFunction("newCheckerTexture", TextureWrapper::newCheckerTexture)

        .beginClass<RenderStateWrapper>("RenderState")
        .addFunction("getDepthWrite", &kaun::RenderState::getDepthWrite)
        .addCFunction("setDepthWrite", &RenderStateWrapper::setDepthWrite)
        .addCFunction("getDepthTest", &RenderStateWrapper::getDepthTest)
        .addCFunction("setDepthTest", &RenderStateWrapper::setDepthTest)
        .addFunction("getBlendEnabled", &kaun::RenderState::getBlendEnabled)
        .addCFunction("setBlendEnabled", &RenderStateWrapper::setBlendEnabled)
        .addCFunction("getBlendFactors", &RenderStateWrapper::getBlendFactors)
        .addCFunction("setBlendFactors", &RenderStateWrapper::setBlendFactors)
        .addCFunction("setBlending", &RenderStateWrapper::setBlending)
        .endClass()
        .addCFunction("newRenderState", RenderStateWrapper::newRenderState)

        .addCFunction("clear", clear)
        .addCFunction("clearDepth", clearDepth)
        .addFunction("setViewport", (void(*)(int, int, int, int))&kaun::setViewport)
        .addCFunction("setProjection", setProjection)
        .addFunction("setViewTransform", setViewTransform)
        .addFunction("setModelTransform", setModelTransform)
        .addCFunction("draw", draw)
        .addFunction("flush", flush)

        .addFunction("beginLoveGraphics", beginLoveGraphics)
        .addFunction("endLoveGraphics", endLoveGraphics)
        .addFunction("beginLoveEventPump", beginLoveEventPump)
        .addFunction("endLoveEventPump", endLoveEventPump)

        .endNamespace();

    kaun::init(true);

    loveState = saveLoveGlState();

    if(luaL_dostring(L, kaunLua)) {
        luaL_error(L, "Error: %s", lua_tostring(L, -1));
    }
    // make love.graphics untouchable?

    lb::push(L, lb::getGlobal(L, "kaun"));
    return 1;
}
