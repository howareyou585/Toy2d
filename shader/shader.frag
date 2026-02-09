#version 450 core
layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec4 outColor;
layout(set = 1, binding = 0) uniform sampler2D Sampler;
void main() {
    outColor = vec4(0,1.0,0,1.0);
}