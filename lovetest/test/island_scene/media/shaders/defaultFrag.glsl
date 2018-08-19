in VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 eye;
} vsOut;

out vec4 fragColor;

uniform sampler2D baseTexture;
uniform vec4 color;
uniform vec3 ambientColor;
uniform vec3 lightDir; // normalized
uniform float texScale = 1.0;
uniform float detailTexScale;
uniform vec2 detailMapDistance;

#pragma include pcfShadows

void main() {
    float NdotL = max(0.0, dot(vsOut.normal, lightDir));
    vec2 tc = vsOut.texCoord;
    tc.y = 1.0 - tc.y;
    vec4 tex = texture(baseTexture, vsOut.texCoord * texScale);
    if(detailTexScale > 0.0) {
        vec4 detail = texture(baseTexture, vsOut.texCoord * texScale * detailTexScale);
        tex = mix(detail, tex,
            smoothstep(detailMapDistance[0], detailMapDistance[1], length(vsOut.eye)));
    }
    vec3 col = color.rgb * tex.rgb;

    float shadow = getShadowValue();

    fragColor = vec4(col * (vec3(1.0) * NdotL * shadow + ambientColor), color.a);
}
