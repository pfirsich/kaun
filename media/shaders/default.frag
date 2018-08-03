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
    float NdotL = max(0.0, dot(vsOut.normal, normalize(vsOut.eye)));
    fragColor = NdotL * vec4(1.0, 0.0, 0.0, 1.0); //texture(base, vsOut.texCoord);
}
