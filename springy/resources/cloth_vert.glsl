#version 460

uniform mat4 P;
uniform mat4 MV;

in vec3 aPos; // In object space
in vec2 aTex;

out vec2 vTex;


void main() {
	gl_Position = P * MV * vec4(aPos, 1.0);
	vTex = aTex;
}