#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat3 T;

attribute vec4 aPos;
attribute vec2 aTex;

varying vec2 vTex;

void main()
{
	gl_Position = P * MV * aPos;
	vTex = (T * vec3(aTex, 1.0)).xy;
}