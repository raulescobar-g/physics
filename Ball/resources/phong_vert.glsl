#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 iMV;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 position;
varying vec3 normal;



void main()
{
	position = (MV * aPos).xyz;
	
	gl_Position = P * vec4(position, 1.0);

	normal = normalize((iMV * vec4(aNor,0.0)).xyz);
	
}
