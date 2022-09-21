#version 150


in vec4 particleColor
out vec4 fragColor;

void main()
{
    
	fragColor = vec4(particleColor, 1.0);
}