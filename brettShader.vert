#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 halmaCoord;
layout (location = 2) in vec3 normal;

out vec3 Normal;
out vec2 HalmaCoord;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
	// We swap the y-axis by substracing our coordinates from 1. This is done because most images have the top y-axis inversed with OpenGL's top y-axis.
	HalmaCoord = vec2(halmaCoord.x, 1.0 - halmaCoord.y);
	FragPos = vec3(model * vec4(position, 1.0f));
	Normal = mat3(transpose(inverse(model))) * normal;
}