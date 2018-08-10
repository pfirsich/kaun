local shaders = {}

shaders.frag = [[
in VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 worldNormal;
    vec3 eye;
} vsOut;

out vec4 fragColor;

uniform sampler2D baseTexture;
uniform vec4 color;

void main() {
    float NdotL = max(0.0, dot(vsOut.normal, normalize(vsOut.eye)));
    //fragColor = vec4(0.0, 0.0, 1.0, 1.0);
    //fragColor = vec4(color.rgb * NdotL, color.a);
    fragColor = vec4(color.rgb * NdotL * texture(baseTexture, vsOut.texCoord).rgb, color.a);
}
]]

shaders.vert = [[
out VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 worldNormal;
    vec3 eye;
} vsOut;

layout(location = KAUN_ATTR_POSITION) in vec3 attrPosition;
layout(location = KAUN_ATTR_NORMAL) in vec3 attrNormal;
layout(location = KAUN_ATTR_TEXCOORD0) in vec2 attrTexCoord;

void main() {
    vsOut.texCoord = attrTexCoord;
    vsOut.normal = normalize(kaun_normal * attrNormal);
    vsOut.worldNormal = normalize(kaun_normal * attrNormal).xyz;
    vsOut.worldPos = vec3(kaun_model * vec4(attrPosition, 1.0));
    vsOut.eye = vec3(-kaun_view * kaun_model * vec4(attrPosition, 1.0));
    gl_Position = kaun_projection * kaun_view * kaun_model * vec4(attrPosition, 1.0);
}
]]

return shaders
