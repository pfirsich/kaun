#version 330 core

out VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 worldNormal;
    vec3 eye;
} vsOut;

layout(location = 0) in vec3 attrPosition;
layout(location = 1) in vec3 attrNormal;
layout(location = 8) in vec2 attrTexCoord;

uniform mat4 kaun_model;
uniform mat3 kaun_normal;
uniform mat4 kaun_view;
uniform mat4 kaun_projection;

void main() {
    vsOut.texCoord = attrTexCoord;
    vsOut.normal = normalize(kaun_normal * attrNormal);
    // TODO: Don't use modelMatrix, but inverse(transpose(kaun_model)) as a uniform to remove scaling!
    vsOut.worldNormal = normalize(kaun_model * vec4(attrNormal, 0.0)).xyz;
    vsOut.worldPos = vec3(kaun_model * vec4(attrPosition, 1.0));
    vsOut.eye = vec3(-kaun_view * kaun_model * vec4(attrPosition, 1.0));
    gl_Position = kaun_projection * kaun_view * kaun_model * vec4(attrPosition, 1.0);
}
