#version 330



in vec4 particleColor;
out vec4 fragColor;

void main()
{
	if (particleColor.w < 0.0) {
		discard;
	}	else {
		fragColor = vec4(particleColor.xyz, 1.0);
	}
}