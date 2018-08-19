uniform sampler2DShadow shadowMap;
uniform mat4 lightTransform;
uniform float pcfRadius = 0.001;
const int pcfSamples = 16;
const int pcfEarlyBailSamples = 4;

const vec2 poissonDisk[16] = vec2[](
    vec2(-0.94201624, -0.39906216),
    vec2( 0.94558609, -0.76890725),
    vec2(-0.09418410, -0.92938870),
    vec2( 0.34495938,  0.29387760),
    vec2(-0.91588581,  0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543,  0.27676845),
    vec2( 0.97484398,  0.75648379),
    vec2( 0.44323325, -0.97511554),
    vec2( 0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2( 0.79197514,  0.19090188),
    vec2(-0.24188840,  0.99706507),
    vec2(-0.81409955,  0.91437590),
    vec2( 0.19984126,  0.78641367),
    vec2( 0.14383161, -0.14100790)
);

float rand3(in vec3 seed) {
    return fract(sin(dot(seed,vec3(53.1215, 21.1352, 9.1322))) * 2105.2354);
}

float rand2(in vec2 seed) {
    return fract(sin(dot(seed,vec2(12.9898,78.233))) * 43758.5453);
}

float poissonShadowValue(vec3 shadowCoords) {
    vec2 radius = vec2(pcfRadius);

    // Think about storing these rotations (sin(alpha) and cos(alpha)) in the RG channel of a texture and tile it over the screen
    // I could get rid of a sqrt, fract, sin, dot for a texture fetch. I don't know if this might be worth it.
    // Also I could store the whole poisson disk coordinate, if I don't plan on sorting my offsets for early bailing!
    // Storing this in a texture might be bad because we only fetch once into it and therefore effectively
    // trade two cache misses with some instructions. What I have now is good enough for now though.
    mat2 offsetRotation = mat2(1.0);
    // Interestingly using sin/cos seems slower than c = rand, s = sqrt(1-c*c), even though on newer hardware with a special functions unit
    // (like Kepler) that is idle mos of the time, sin/cos should be almost free. Seems like a reason to use a texture?
    // But it might be that it's slower, because here we use a lot of special functions anyways. Test this someday!

    // In theory using the world position as the variable for the rotation is the better way to do it, to avoid creeping noise
    // when the camera is moving, but in practice there is no visible difference. Don't forget this in case I decide to use a texture, because
    // if I would use the world position, I would need a 3D texture! (which I can then just not do)
    //float c = rand2(gl_FragCoord.xy);
    float c = rand3(vsOut.worldPos * 1000.0);
    float s = sqrt(1.0 - c*c);
    offsetRotation[0][0] = c;
    offsetRotation[1][1] = c;
    offsetRotation[0][1] = -s;
    offsetRotation[1][0] = s;

    float sum = 0.0;
    vec2 offset;

    for(int i = 0; i < pcfEarlyBailSamples; ++i) {
        offset = poissonDisk[i] * radius;
        sum += texture(shadowMap, shadowCoords + vec3(offsetRotation * offset, 0.0));
    }

    float bailShadowFactor = sum / pcfEarlyBailSamples;
    if(pcfEarlyBailSamples == 0 || bailShadowFactor > 0.1 && bailShadowFactor < 0.9) {
        for(int i = pcfEarlyBailSamples; i < pcfSamples; ++i) {
            offset = poissonDisk[i] * radius;
            sum += texture(shadowMap, shadowCoords + vec3(offsetRotation * offset, 0.0));
        }
        return sum / pcfSamples;
    } else {
        return bailShadowFactor;
    }

}

float getShadowValue() {
    vec4 coords = lightTransform * vec4(vsOut.worldPos, 1.0);
    coords /= coords.w;
    coords.xyz = coords.xyz * 0.5 + 0.5; // [-1, 1] to [0, 1]
    float bias = 0.001; // don't just use a constant bias
    coords.z -= bias;
    if(coords.z > 1.0) {
        return 1.0;
    } else {
        return poissonShadowValue(coords.xyz);
    }
}
