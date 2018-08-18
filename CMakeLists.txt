cmake_minimum_required(VERSION 3.7)

include(ExternalProject)

project(kaun)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 17)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive")
endif()

include_directories(dependencies/glm)

include_directories(dependencies/stb)

include_directories(dependencies/LuaBridge/Source)

include_directories(dependencies/expected/tl)

include_directories(dependencies/tinyobjloader)

add_subdirectory(dependencies/glad)
include_directories(dependencies/glad/include)

add_subdirectory(dependencies/SDL2)
include_directories(dependencies/SDL2/include)

add_subdirectory(dependencies/lua)
include_directories(dependencies/lua/src)
# luaconf.h seems to be created during cmake configuration and copied to the build dir
include_directories(build/dependencies/lua)

include_directories(kaun/include)
add_compile_definitions(NOMINMAX)
set(KAUN_SOURCE kaun/log.cpp kaun/mesh.cpp kaun/mesh_buffers.cpp kaun/mesh_vertexaccessor.cpp
    kaun/mesh_vertexformat.cpp kaun/render.cpp kaun/renderstate.cpp kaun/shader.cpp
    kaun/shader_preambles.cpp kaun/texture.cpp kaun/transform.cpp kaun/utility.cpp
    kaun/window.cpp kaun/kaun.cpp kaun/renderattachment.cpp kaun/rendertarget.cpp)
add_library(libkaun STATIC ${KAUN_SOURCE})
target_link_libraries(libkaun SDL2main SDL2 glad)

set(LUA_KAUN_SOURCE lua-kaun/lua-kaun.cpp lua-kaun/glstate.cpp)
add_library(kaun ${LUA_KAUN_SOURCE})
target_link_libraries(kaun libkaun liblua)

add_executable(kauntest kaun/main.cpp)
target_link_libraries(kauntest libkaun)

set_target_properties(kauntest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
