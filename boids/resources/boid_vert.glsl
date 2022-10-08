#version 460
#extension GL_ARB_shader_storage_buffer_object : enable

uniform mat4 P;
uniform mat4 MV;
uniform mat4 iMV;

in vec3 aPos;
in vec3 aNor;

in vec4 position;
in vec4 color;
in vec4 velocity;

out vec3 normal;
out vec3 pos;


void main(){
	
  pos = (MV  * vec4(aPos + position.xyz, 1.0)).xyz;
	gl_Position = P *  vec4(pos, 1.0);	

	normal = normalize((iMV * vec4( aNor ,0.0)).xyz);
}
