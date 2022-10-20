#version 460
#extension GL_ARB_shading_language_include : require

uniform float dt;

struct particle_data {
	vec4 velocity,force;
};

layout( std430, binding=0 ) buffer Positions { // start pos
	float start_positions[ ]; 
};
layout( std430, binding=1 ) buffer Particles { // start v
	particle_data start_derivative[ ];
};


layout( std430, binding=2 ) buffer Particles1 { // prev k to be used later dont modify
	particle_data derivative[ ];
};


layout( std430, binding=3 ) buffer Positions2 { // out k
	float final_positions[ ]; 
};
layout( std430, binding=4 ) buffer Particles2 { // out k
	particle_data final_derivative[ ];
};


layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;


void main(){
	uint gid = gl_GlobalInvocationID.x;
    uint i = gid * 3;

    final_positions[ i ]      = start_positions[ i ]        + derivative[ gid ].velocity.x * dt;
    final_positions[ i + 1 ]  = start_positions[ i + 1 ]    + derivative[ gid ].velocity.y * dt;
    final_positions[ i + 2 ]  = start_positions[ i + 2 ]    + derivative[ gid ].velocity.z * dt;

    vec3 acceleration = derivative[ gid ].force.xyz / derivative[ gid ].force.w;

    final_derivative[ gid ].velocity.xyz = start_derivative[ gid ].velocity.xyz + acceleration * dt;
}