#version 460
#extension GL_ARB_shading_language_include : require

uniform float dt;
uniform vec3 gravity;

struct particle_data {
	vec4 velocity,force;
};


layout( std430, binding=0 ) buffer Positions {
	float positions[ ]; 
};
layout( std430, binding=1 ) buffer Particles {
	particle_data particles[ ];
};


layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;



void main(){
	uint gid = gl_GlobalInvocationID.x;

    positions[ gid * 3 ] += particles[ gid ].velocity.x * dt;
    positions[ gid * 3 + 1 ] += particles[ gid ].velocity.y * dt;
    positions[ gid * 3 + 2 ] += particles[ gid ].velocity.z * dt;

    particles[ gid ].velocity.xyz += (particles[ gid ].force.xyz / particles[ gid ].force.w) * dt;

    particles[ gid ].force.xyz = gravity * particles[ gid ].force.w;
}