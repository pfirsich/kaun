#include <string>

#include "shader.hpp"

std::string kaun::Shader::globalShaderPreamble = R"(#version 330 core

// Attribute Indices
#define KAUN_ATTR_POSITION    0
#define KAUN_ATTR_NORMAL      1
#define KAUN_ATTR_TANGENT     2
#define KAUN_ATTR_BITANGENT   3
#define KAUN_ATTR_COLOR0      4
#define KAUN_ATTR_COLOR1      5
#define KAUN_ATTR_BONEINDICES 6
#define KAUN_ATTR_BONEWEIGHTS 7
#define KAUN_ATTR_TEXCOORD0   8
#define KAUN_ATTR_TEXCOORD1   9
#define KAUN_ATTR_TEXCOORD2   10
#define KAUN_ATTR_TEXCOORD3   11
#define KAUN_ATTR_CUSTOM0     12
#define KAUN_ATTR_CUSTOM1     13
#define KAUN_ATTR_CUSTOM2     14
#define KAUN_ATTR_CUSTOM3     15
#define KAUN_ATTR_CUSTOM4     16
#define KAUN_ATTR_CUSTOM5     17
#define KAUN_ATTR_CUSTOM6     18
#define KAUN_ATTR_CUSTOM7     19

// Built-In Uniforms
uniform ivec4 kaun_viewport;
uniform mat4 kaun_view;
uniform mat4 kaun_invView;
uniform mat4 kaun_projection;
uniform mat4 kaun_invProjection;
uniform mat4 kaun_viewProjection;
uniform mat4 kaun_invViewProjection;
uniform mat4 kaun_model;
uniform mat3 kaun_normal;
uniform mat4 kaun_modelView;
uniform mat4 kaun_modelViewProjection;

//TODO: Gamma correction functions?
)";

std::string kaun::Shader::fragmentShaderPreamble = R"(
#define FRAGMENT
)";

std::string kaun::Shader::vertexShaderPreamble = R"(
#define VERTEX
)";
