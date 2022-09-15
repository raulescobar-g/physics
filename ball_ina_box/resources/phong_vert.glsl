#version 150

uniform mat4 P;
uniform mat4 MV;
uniform mat4 iMV;

in vec4 aPos; // in object space
in vec3 aNor; // in object space

out vec3 position;
out vec3 normal;



void main()
{
	position = (MV * aPos).xyz;
	
	gl_Position = P * vec4(position, 1.0);

	normal = normalize((iMV * vec4(aNor, 0.0)).xyz);
	
}
