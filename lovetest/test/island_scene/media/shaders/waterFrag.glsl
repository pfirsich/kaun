in VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 eye;
} vsOut;

out vec4 fragColor;

uniform float texScale = 1.0;
uniform samplerCube skyboxTexture;
uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;

const float R0 = 0.15;

#pragma include pcfShadows

void main() {
    vec3 worldEye = normalize(kaun_invView * vec4(vsOut.eye, 0.0)).xyz;

    vec3 reflection = texture(skyboxTexture, reflect(-worldEye, vsOut.normal)).rgb;
    vec3 refraction = texture(skyboxTexture, refract(-worldEye, vsOut.normal, R0)).rgb;
    float R = R0 + (1.0 - R0) * pow(1.0 - max(0.0, dot(worldEye, vsOut.normal)), 5);

    //This would be correct, but our refraction is lame, so I cheat instaed
    //fragColor = vec4(mix(refraction, reflection, R), 1.0);

    vec2 depthUV = gl_FragCoord.xy / kaun_viewport.zw;
    float depth = texture(depthTexture, depthUV).r;

    // https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
    vec3 ndcPos;
    ndcPos.xy = depthUV * 2.0 - 1.0;
    ndcPos.z = (2.0 * depth - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
    vec4 clipPos;
    clipPos.w = kaun_projection[3][2] / (ndcPos.z - (kaun_projection[2][2] / kaun_projection[2][3]));
    clipPos.xyz = ndcPos * clipPos.w;
    vec4 groundPos = kaun_invProjection * clipPos;

    float foamAmount = max(0, 1.0 - abs(length(groundPos.xyz) - length(vsOut.eye)) * 2.5);
    foamAmount *= texture(noiseTexture, vsOut.texCoord * 135.0).r;

    float shadow = getShadowValue();
    vec4 foam = vec4(mix(vec3(0.4), vec3(1.0), shadow), 1.0);
    reflection *= mix(0.5, 1.0, shadow);
    //fragColor = vec4(vec3(shadow), 1.0);
    fragColor = mix(vec4(reflection, min(1, R + 0.3)), foam, foamAmount);
}
