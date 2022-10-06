#version 430


in vec3 normal;

out vec4 fragColor;

void main()
{
	fragColor = vec4(normalize(normal), 1.0);
}