out VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 eye;
} vsOut;

layout(location = KAUN_ATTR_POSITION) in vec3 attrPosition;
layout(location = KAUN_ATTR_NORMAL) in vec3 attrNormal;
layout(location = KAUN_ATTR_TEXCOORD0) in vec2 attrTexCoord;

uniform float time;

// https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch01.html
struct wave {
    vec3 dir;
    float freq;
    float speed;
    float amplitude;
    float Q;
};
wave waves[4];

float amplitude = 0.075;

void main() {
    waves[0].dir = normalize(vec3(0.0, 0.0, 1.0));
    waves[0].freq = 2.0 / 10.0; // 2/L, L = wavelength
    waves[0].speed = 2.7;
    waves[0].amplitude = 1.0;
    waves[0].Q = 0.1;

    waves[1].dir = normalize(vec3(1.0, 0.0, 0.1));
    waves[1].freq = 2.0 / 8.0;
    waves[1].speed = 2.5;
    waves[1].amplitude = 0.5;
    waves[1].Q = 0.1;

    waves[2].dir = normalize(vec3(1.0, 0.0, 1.0));
    waves[2].freq = 2.0 / 2.0;
    waves[2].speed = 2.4123;
    waves[2].amplitude = 0.07;
    waves[2].Q = 0.8;

    waves[3].dir = normalize(vec3(0.5, 0.0, 1.0));
    waves[3].freq = 2.0 / 0.8;
    waves[3].speed = 2.1524;
    waves[3].amplitude = 0.025;
    waves[3].Q = 0.95;

    vec3 P = attrPosition;
    P.y = 0.0;
    for(int i = 0; i < 4; ++i) {
        float phase = (dot(waves[i].dir, P) + waves[i].speed * time) * waves[i].freq;
        P.xz += waves[i].dir.xz * waves[i].Q * waves[i].amplitude * cos(phase);
        P.y += amplitude * waves[i].amplitude * sin(phase);
    }

    vsOut.texCoord = attrTexCoord;
    vsOut.normal = normalize(kaun_normal * attrNormal);
    vsOut.worldPos = vec3(kaun_model * vec4(P, 1.0));
    vsOut.eye = vec3(-kaun_modelView * vec4(P, 1.0));
    gl_Position = kaun_modelViewProjection * vec4(P, 1.0);
}
