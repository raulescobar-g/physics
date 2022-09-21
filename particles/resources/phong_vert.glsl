#version 150

uniform mat4 P;
uniform mat4 MV;
uniform mat4 iMV;

in vec4 aPos; // in object space
in vec3 aNor; // in object space

void main()
{
	vec3 position = (MV * aPos).xyz;
	
	gl_Position = P * vec4(position, 1.0);

}
