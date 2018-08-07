extern "C" {
    #include <lua.h>
    #include <lauxlib.h>
    #include <lualib.h>
}
#include <LuaBridge/LuaBridge.h>
namespace lb = luabridge;

#include <kaun/kaun.hpp>

// init()
// clear(float r, float g, float b, float a)
// clear()
// clearDepth()
// setProjection(mat11, mat12, mat13, ..., mat44)
// setViewTransform(Transform&)
// setModelTransform(Transform&)
// draw(Mesh&, Shader&, {name = value})
// flush()
// swap()

// newTransform()
//   setPosition(posX, posY, posZ)
//   getPosition() => x, y, z
//   rotate(angle, axisX, axisY, axisZ)
//   lookAtPos(posX, posY, posZ, lookAtX, lookAtY, lookAtZ)

// newBoxMesh(width, height, depth) => Mesh*

// newShader(fragmentStr, vertexStr)

// newCheckerTexture(int resX, int resY, int checkerSize) => Texture*

#define EXPORT __declspec(dllexport)

int func(float a, float b) {
    return static_cast<int>(a*b);
}

extern "C" EXPORT int luaopen_kaun(lua_State* L) {
    lb::getGlobalNamespace(L)
        .beginNamespace("kaun")
            .addFunction("test", func)
        .endNamespace();

    lb::push(L, lb::getGlobal(L, "kaun"));
    return 1;
}
