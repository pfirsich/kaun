#version 330 core

in VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 worldNormal;
    vec3 eye;
} vsOut;

out vec4 fragColor;

uniform sampler2D base;

void main() {
    fragColor = vec4(1.0, 0.0, 0.0, 1.0); //texture(base, vsOut.texCoord);
}
