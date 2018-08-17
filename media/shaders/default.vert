layout(location = KAUN_ATTR_POSITION) in vec3 attrPosition;

void main() {
    gl_Position = kaun_modelViewProjection * vec4(attrPosition, 1.0);
}
