#version 450 core
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 0) out vec2 outTexCoord;
layout(set = 0, binding = 0) uniform UniformBuffer
{
	mat4 model;
	mat4 projection;
}ubo;
void main() {
    gl_Position = vec4(inPosition.x, inPosition.y,0.0,1.0);
	outTexCoord = inTexCoord;
}