cmake_minimum_required(VERSION 3.7)

include(ExternalProject)

project(kaun)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(CMAKE_CXX_STANDARD 11)

include_directories(dependencies/glm)

include_directories(dependencies/stb)

include_directories(dependencies/expected/tl)

add_subdirectory(dependencies/glad)
include_directories(dependencies/glad/include)

add_subdirectory(dependencies/SDL2)
include_directories(dependencies/SDL2/include)

# LuaJIT
# taken from here: https://bitbucket.org/rude/megasource/src/default/CMakeLists.txt
if(MSVC)
    # Copy LuaJIT source to binary dir. LuaJIT builds in-source,
    # and we don't want to modify the original source tree, so
    # we copy it.
    set(LUAJIT_SOURCE_DIR ${CMAKE_BINARY_DIR}/LuaJIT)
    file(COPY dependencies/LuaJIT DESTINATION ${CMAKE_BINARY_DIR})

    set(LUAJIT_BUILD_BAT "
        @echo off
        REM call \"${MEGA_MSVC_VCVARSALL_BAT}\" ${MEGA_MSVC_VCVARSALL_BAT_ARG}
        cd \"${LUAJIT_SOURCE_DIR}/src\"
        msvcbuild.bat
    ")

    file(WRITE ${LUAJIT_SOURCE_DIR}/megabuild.bat ${LUAJIT_BUILD_BAT})

    ExternalProject_Add(luajit
        SOURCE_DIR ${LUAJIT_SOURCE_DIR}
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ""
        BUILD_COMMAND "${LUAJIT_SOURCE_DIR}/megabuild.bat"
        INSTALL_COMMAND cmake -E make_directory ${CMAKE_BINARY_DIR}/bin
            COMMAND cmake -E copy ${LUAJIT_SOURCE_DIR}/src/lua51.dll ${CMAKE_BINARY_DIR}
            COMMAND cmake -E copy ${LUAJIT_SOURCE_DIR}/src/lua51.lib ${CMAKE_BINARY_DIR})

    #set(LUAJIT_LIB ${CMAKE_BINARY_DIR}/bin/lua51.lib)
    #set(LUAJIT_DLL ${CMAKE_BINARY_DIR}/bin/lua51.dll)

    include_directories(${LUAJIT_SOURCE_DIR}/src)
endif()

include_directories(include)
include_directories(include/kaun)
add_compile_definitions(NOMINMAX)
set(KAUN_SOURCE src/log.cpp src/mesh.cpp src/mesh_buffers.cpp src/mesh_vertexaccessor.cpp
    src/mesh_vertexformat.cpp src/render.cpp src/renderstate.cpp src/shader.cpp
    src/texture.cpp src/transform.cpp src/utility.cpp src/window.cpp src/kaun.cpp)
add_library(kaun ${KAUN_SOURCE})
target_link_libraries(kaun SDL2main SDL2 glad)
add_executable(kauntest src/main.cpp)
target_link_libraries(kauntest kaun)

set_target_properties(kauntest PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
