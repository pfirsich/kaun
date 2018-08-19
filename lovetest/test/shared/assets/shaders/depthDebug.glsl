in vec2 texCoord;

out vec4 fragColor;

uniform sampler2D depthTexture;
uniform vec2 range = vec2(0.0, 1.0);

void main() {
    float depth = texture(depthTexture, texCoord).r;
    depth = max(0, depth - range.x) / (range.y - range.x);
    fragColor = vec4(vec3(depth), 1.0);
}
