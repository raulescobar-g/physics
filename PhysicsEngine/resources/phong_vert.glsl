#version 460

uniform mat4 P;
uniform mat4 MV;
uniform mat4 iMV;

in vec3 aPos; 
in vec3 aNor; 

out vec3 position;
out vec3 normal;


void main()
{
	position = (MV * vec4(aPos, 1.0)).xyz;
	normal = normalize((iMV * vec4(aNor,0.0)).xyz);
	
	gl_Position = P * vec4(position, 1.0);
}
