layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 eye;
} gsIn[];

out VSOUT {
    vec2 texCoord;
    vec3 normal;
    vec3 worldPos;
    vec3 eye;
} gsOut;

void main() {
    vec3 rel10 = gsIn[0].worldPos - gsIn[1].worldPos;
    vec3 rel12 = gsIn[2].worldPos - gsIn[1].worldPos;
    vec3 normal = normalize(kaun_normal * cross(rel12, rel10));

    for(int i = 0; i < 3; ++i) {
        gsOut.texCoord = gsIn[i].texCoord;
        gsOut.worldPos = gsIn[i].worldPos;
        gsOut.eye = gsIn[i].eye;
        gsOut.normal = normal;
        gl_Position = gl_in[i].gl_Position;
        EmitVertex();
    }
    EndPrimitive();
}
