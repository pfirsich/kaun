in VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 eye;
} vsOut;

out vec4 fragColor;

uniform sampler2D base;

void main() {
    float NdotL = max(0.0, dot(vsOut.normal, normalize(vsOut.eye)));
    fragColor = NdotL * texture(base, vsOut.texCoord);
}
