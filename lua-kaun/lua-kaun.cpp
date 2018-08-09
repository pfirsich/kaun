extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}
#include <LuaBridge/LuaBridge.h>
namespace lb = luabridge;

#include <kaun.hpp>

#define EXPORT __declspec(dllexport)

template <typename T>
T luax_check(lua_State* L, int index);

template <>
float luax_check<float>(lua_State* L, int index) {
    return static_cast<float>(luaL_checknumber(L, index));
}

template <>
glm::vec2 luax_check<glm::vec2>(lua_State* L, int index) {
    return glm::vec2(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1));
}

template <>
glm::vec3 luax_check<glm::vec3>(lua_State* L, int index) {
    return glm::vec3(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1),
                     luax_check<float>(L, index + 2));
}

template <>
glm::vec4 luax_check<glm::vec4>(lua_State* L, int index) {
    return glm::vec4(luax_check<float>(L, index + 0), luax_check<float>(L, index + 1),
                     luax_check<float>(L, index + 2), luax_check<float>(L, index + 3));
}

// checks if stack[index] is a table with num numbers and pushes them on to the stack
// true on success, false if malformed
bool luax_getnumtable(lua_State* L, int index, int num) {
    if(index < 0) index = lua_gettop(L) + index + 1;
    if(lua_istable(L, index)) {
        int len = lua_objlen(L, index);
        if(len == num) {
            luaL_checkstack(L, num, "Cannot grow stack to appropriate size.");
            for(int i = 1; i <= num; ++i) {
                lua_rawgeti(L, index, i);
                if(!lua_isnumber(L, -1)) {
                    luaL_error(L, "Expected number for element #%d in table.", i);
                    lua_pop(L, i); // clean up pushed elements
                    return false;
                }
            }
            return true;
        } else {
            luaL_error(L, "Expecting table of length %d", num);
            return false;
        }
    } else {
        luaL_typerror(L, index, "table");
        return false;
    }
}

template <typename T>
T luax_checkvectable(lua_State* L, int index) {
    int len = T::length();
    if(luax_getnumtable(L, index, len)) {
        T v = luax_check<T>(L, -len);
        lua_pop(L, len);
        return v;
    }
    return T();
}

void luax_pushvec3(lua_State* L, const glm::vec3& v) {
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
}

void luax_pushvec4(lua_State* L, const glm::vec4& v) {
    lua_pushnumber(L, v.x);
    lua_pushnumber(L, v.y);
    lua_pushnumber(L, v.z);
    lua_pushnumber(L, v.w);
}

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

int setTrafos(lua_State* L) {
    TransformWrapper* t1 = lb::Userdata::get<TransformWrapper>(L, 1, false);
    TransformWrapper* t2 = lb::Userdata::get<TransformWrapper>(L, 2, false);
    t1->setPosition(50.0f, 50.0f, 50.0f);
    t2->setPosition(50.0f, 50.0f, 50.0f);
    return 0;
}

const char* loveRun = R"(function love.run()
    local width, height, flags = love.window.getMode()
    love.resize(width, height)
    assert(flags.depth > 0, "Window needs a depth buffer! Add 't.window.depth = 24' to conf.lua.")

    if love.load then love.load(love.arg.parseGameArguments(arg), arg) end

    -- We don't want the first frame's dt to include time taken by love.load.
    if love.timer then love.timer.step() end

    local dt = 0

    -- Main loop time.
    return function()
        -- Process events.
        if love.event then
            love.event.pump()
            for name, a,b,c,d,e,f in love.event.poll() do
                if name == "quit" then
                    if not love.quit or not love.quit() then
                        return a or 0
                    end
                end
                love.handlers[name](a,b,c,d,e,f)
            end
        end

        -- Update dt, as we'll be passing it to update
        if love.timer then dt = love.timer.step() end

        -- Call update and draw
        if love.update then love.update(dt) end -- will pass 0 if love.timer is disabled

        if love.graphics.isActive() then
            if love.draw then love.draw() end
            kaun.flush()
            love.graphics.present()
        end
    end
end)";

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

        .addCFunction("setTrafos", setTrafos)

        .beginClass<MeshWrapper>("Mesh")
        .endClass()
        .addCFunction("newBoxMesh", MeshWrapper::newBoxMesh)

        .beginClass<ShaderWrapper>("Shader")
        .endClass()
        .addCFunction("newShader", ShaderWrapper::newShader)

        .beginClass<TextureWrapper>("Texture")
        .endClass()
        .addCFunction("newCheckerTexture", TextureWrapper::newCheckerTexture)

        .addCFunction("clear", clear)
        .addCFunction("clearDepth", clearDepth)
        .addFunction("setViewport", (void (*)(int, int, int, int))&kaun::setViewport)
        .addCFunction("setProjection", setProjection)
        .addFunction("setViewTransform", setViewTransform)
        .addFunction("setModelTransform", setModelTransform)
        .addCFunction("draw", draw)
        .addFunction("flush", kaun::flush)

        .endNamespace();

    kaun::init(true);

    luaL_dostring(L, loveRun);
    // make love.graphics untouchable?

    lb::push(L, lb::getGlobal(L, "kaun"));
    return 1;
}
