#version 120

void main() // color of the sun
{
	float r = 253.0/256.0;
	float g = 184.0/256.0;
	float b = 19.0/256.0;
	gl_FragColor = vec4(r,g,b, 1.0);
}

