#version 460
#extension GL_ARB_shading_language_include : require

uniform float dt;

struct particle_data {
	vec4 velocity,force;
};

layout( std430, binding=0 ) buffer Positions { 
	float positions[ ]; 
};
layout( std430, binding=1 ) buffer Particles { 
	particle_data derivative[ ];
};

layout( std430, binding=2 ) buffer Particles1 { 
	particle_data derivative_k1[ ];
};
layout( std430, binding=3 ) buffer Particles2 { 
	particle_data derivative_k2[ ];
};
layout( std430, binding=4 ) buffer Particles3 { 
	particle_data derivative_k3[ ];
};
layout( std430, binding=5 ) buffer Particles4 { 
	particle_data derivative_k4[ ];
};

layout( std430, binding=6 ) buffer PastPositions { 
	float past_positions[ ]; 
};
layout( std430, binding=7 ) buffer PastVelocities { 
	float past_velocities[ ]; 
};

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;


void main(){
	uint gid = gl_GlobalInvocationID.x;
    uint i = gid * 3;

    vec3 p = derivative_k1[ gid ].velocity.xyz + (2.0 * derivative_k2[ gid ].velocity.xyz) + (2.0 * derivative_k3[ gid ].velocity.xyz) + derivative_k4[ gid ].velocity.xyz;
    
    past_positions[i] = positions[i];
    past_positions[i+1] = positions[i+1];
    past_positions[i+2] = positions[i+2];

    past_velocities[i] = derivative[ gid ].velocity.x;
    past_velocities[i+1] = derivative[ gid ].velocity.y;
    past_velocities[i+2] = derivative[ gid ].velocity.z;

    positions[ i ]      += p.x * dt;
    positions[ i + 1 ]  += p.y * dt;
    positions[ i + 2 ]  += p.z * dt;

    vec3 a_k1 = derivative_k1[ gid ].force.xyz / derivative_k1[ gid ].force.w;
    vec3 a_k2 = derivative_k2[ gid ].force.xyz / derivative_k2[ gid ].force.w;
    vec3 a_k3 = derivative_k3[ gid ].force.xyz / derivative_k3[ gid ].force.w;
    vec3 a_k4 = derivative_k4[ gid ].force.xyz / derivative_k4[ gid ].force.w;

    derivative[ gid ].velocity.xyz += (a_k1 + (2.0*a_k2) + (2.0*a_k3) + a_k4) * dt;
}