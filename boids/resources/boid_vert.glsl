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

mat3 calcLookAtMatrix(vec3 origin, vec3 target, float roll) {
  vec3 rr = vec3(sin(roll), cos(roll), 0.0);
  vec3 ww = normalize(target - origin);
  vec3 uu = normalize(cross(ww, rr));
  vec3 vv = normalize(cross(uu, ww));

  return mat3(uu, vv, ww);
}


void main(){
  mat3 look = calcLookAtMatrix(position.xyz, position.xyz + normalize(velocity.xyz), 0.0);
	
  pos = (MV  * vec4(look * color.w *aPos + position.xyz, 1.0)).xyz;
	gl_Position = P *  vec4(pos, 1.0);	

	normal = normalize((iMV * vec4(look* aNor,0.0)).xyz);
}
