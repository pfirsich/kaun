in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D tex;
uniform bool flipY;

void main() {
    vec2 tc = texCoord;
    if(flipY) tc.y = 1.0 - tc.y;
    fragColor = texture(tex, tc);
}
