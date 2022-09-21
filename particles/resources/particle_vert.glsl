#version 150

uniform mat4 P;
uniform mat4 V;

in vec3 vertices;
in vec4 position; // in object space
in vec4 color; // in object space

out vec4 particleColor;


void main()
{
    float particleSize = position.w;
    vec3 center = position.xyz;

    vec4 position_viewspace = V * vec4(position.xyz, 1.0);
    position_viewspace.xy += particleSize * (vertices.xy - vec2(0.5));
	
	gl_Position = P * position_viewspace;	

	particleColor = color;
}