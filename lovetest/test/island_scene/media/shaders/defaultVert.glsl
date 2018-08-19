out VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 eye;
} vsOut;

layout(location = KAUN_ATTR_POSITION) in vec3 attrPosition;
layout(location = KAUN_ATTR_NORMAL) in vec3 attrNormal;
layout(location = KAUN_ATTR_TEXCOORD0) in vec2 attrTexCoord;

void main() {
    vsOut.texCoord = attrTexCoord;
    vsOut.normal = normalize(kaun_normal * attrNormal);
    vsOut.worldPos = vec3(kaun_model * vec4(attrPosition, 1.0));
    vsOut.eye = vec3(-kaun_modelView * vec4(attrPosition, 1.0));
    gl_Position = kaun_modelViewProjection * vec4(attrPosition, 1.0);
}
