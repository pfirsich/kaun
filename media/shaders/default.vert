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
layout(location = 2) in vec2 attrTexCoord;

uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    vsOut.texCoord = attrTexCoord;
    vsOut.normal = normalize(normalMatrix * attrNormal);
    // TODO: Don't use modelMatrix, but inverse(transpose(modelMatrix)) as a uniform to remove scaling!
    vsOut.worldNormal = normalize(modelMatrix * vec4(attrNormal, 0.0)).xyz;
    vsOut.worldPos = vec3(modelMatrix * vec4(attrPosition, 1.0));
    vsOut.eye = vec3(-viewMatrix * modelMatrix * vec4(attrPosition, 1.0));
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(attrPosition, 1.0);
}
