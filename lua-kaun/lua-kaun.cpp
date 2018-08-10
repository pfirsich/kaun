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

    void rotate(float angle, float axisX, float axisY, float axisZ) {
        Transform::rotate(angle, glm::vec3(axisX, axisY, axisZ));
    }

    int lookAtPos(lua_State* L) {
        int args = lua_gettop(L);
	    if(args == 7) {
            Transform::lookAtPos(luax_check<glm::vec3>(L, 2), luax_check<glm::vec3>(L, 5));
	    } else if(args == 10) {
            Transform::lookAtPos(luax_check<glm::vec3>(L, 2), luax_check<glm::vec3>(L, 5), luax_check<glm::vec3>(L, 8));
	    } else {
		    luaL_error(L, "Number of arguments to kaun.lookAtPos must be 7 or 10. Got %d", args);
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

struct VertexFormatWrapper : public kaun::VertexFormat {

};

struct MeshWrapper : public kaun::Mesh {
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

struct RenderStateWrapper : public kaun::RenderState {

};

struct TextureWrapper : public kaun::Texture {
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
        .addCFunction("lookAtPos", &TransformWrapper::lookAtPos)
        .endClass()
        .addFunction("newTransform", TransformWrapper::newTransform)

        .beginClass<Trash>("Trash")
        .addFunction("set", &Trash::set)
        .addFunction("get", &Trash::get)
        .endClass()
        .addFunction("newTrash", Trash::newTrash)
        .addFunction("newTrashP", Trash::newTrashP)
        .addCFunction("newTrashS", Trash::newTrashS)

        .beginClass<MeshWrapper>("Mesh")
        .endClass()
        .addCFunction("newBoxMesh", MeshWrapper::newBoxMesh)

        .beginClass<ShaderWrapper>("Shader")
        .endClass()
        .addCFunction("newShader", ShaderWrapper::newShader)

        .beginClass<TextureWrapper>("Texture")
        .endClass()
        .addCFunction("newTexture", TextureWrapper::newTexture)
        .addCFunction("newCheckerTexture", TextureWrapper::newCheckerTexture)

        .addCFunction("clear", clear)
        .addCFunction("clearDepth", clearDepth)
        .addFunction("setViewport", (void(*)(int, int, int, int))&kaun::setViewport)
        .addCFunction("setProjection", setProjection)
        .addFunction("setViewTransform", setViewTransform)
        .addFunction("setModelTransform", setModelTransform)
        .addCFunction("draw", draw)
        .addFunction("flush", kaun::flush)

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
