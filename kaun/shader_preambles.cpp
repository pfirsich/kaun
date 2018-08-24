#include <string_view>

#include "shader.hpp"

// Gamma Correction Helpers are from https://bitbucket.org/rude/love (wrap_Graphics.lua)
// and  http://chilliant.blogspot.com.au/2012/08/srgb-approximations-for-hlsl.html?m=1

std::string_view kaun::Shader::globalShaderPreamble = R"(#version 330 core

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

// Gamma Correction Helpers
float gammaToLinearPrecise(float c) {
	return c <= 0.04045 ? c * 0.077399380804954 : pow((c + 0.055) * 0.9478672985782, 2.4);
}
vec3 gammaToLinearPrecise(vec3 c) {
	bvec3 leq = lessThanEqual(c, vec3(0.04045));
	c.r = leq.r ? c.r * 0.077399380804954 : pow((c.r + 0.055) * 0.9478672985782, 2.4);
	c.g = leq.g ? c.g * 0.077399380804954 : pow((c.g + 0.055) * 0.9478672985782, 2.4);
	c.b = leq.b ? c.b * 0.077399380804954 : pow((c.b + 0.055) * 0.9478672985782, 2.4);
	return c;
}
vec4 gammaToLinearPrecise(vec4 c) { return vec4(gammaToLinearPrecise(c.rgb), c.a); }

float linearToGammaPrecise(float c) {
	return c < 0.0031308 ? c * 12.92 : 1.055 * pow(c, 1.0 / 2.4) - 0.055;
}
vec3 linearToGammaPrecise(vec3 c) {
	bvec3 lt = lessThanEqual(c, vec3(0.0031308));
	c.r = lt.r ? c.r * 12.92 : 1.055 * pow(c.r, 1.0 / 2.4) - 0.055;
	c.g = lt.g ? c.g * 12.92 : 1.055 * pow(c.g, 1.0 / 2.4) - 0.055;
	c.b = lt.b ? c.b * 12.92 : 1.055 * pow(c.b, 1.0 / 2.4) - 0.055;
	return c;
}
vec4 linearToGammaPrecise(vec4 c) { return vec4(linearToGammaPrecise(c.rgb), c.a); }

mediump float gammaToLinearFast(mediump float c) { return c * (c * (c * 0.305306011 + 0.682171111) + 0.012522878); }
mediump vec3 gammaToLinearFast(mediump vec3 c) { return c * (c * (c * 0.305306011 + 0.682171111) + 0.012522878); }
mediump vec4 gammaToLinearFast(mediump vec4 c) { return vec4(gammaToLinearFast(c.rgb), c.a); }

mediump float linearToGammaFast(mediump float c) { return max(1.055 * pow(max(c, 0.0), 0.41666666) - 0.055, 0.0); }
mediump vec3 linearToGammaFast(mediump vec3 c) { return max(1.055 * pow(max(c, vec3(0.0)), vec3(0.41666666)) - 0.055, vec3(0.0)); }
mediump vec4 linearToGammaFast(mediump vec4 c) { return vec4(linearToGammaFast(c.rgb), c.a); }

#define gammaToLinear gammaToLinearFast
#define linearToGamma linearToGammaFast
)";

std::string_view kaun::Shader::fragmentShaderPreamble = R"(
#define FRAGMENT
#line 1
)";

std::string_view kaun::Shader::vertexShaderPreamble = R"(
#define VERTEX
#line 1
)";

std::string_view kaun::Shader::geometryShaderPreamble = R"(
#define GEOMETRY
#line 1
)";
