#version 460
#extension GL_ARB_shading_language_include : require

uniform float h;
uniform float k;
uniform float rho;

layout (binding = 0, offset=0) uniform atomic_uint counters[3];


layout( std140, binding=4 ) buffer Pos {
	vec4 positions[ ]; 
};

layout( std140, binding=5 ) buffer Vel {
	vec4 velocities[ ];
};

layout(std430, binding=10 ) buffer Density {
	float density[ ];  
};
layout( std430, binding=12 ) buffer Pre {
	float pressure[ ];
};

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;


float density_calculation( uint gid ) {
	vec3 pos = positions[ gid ].xyz;
	
	float total = 0.0;

	float h3 = h * h * h;
	float h9 = h3 * h3;
	float pi = 3.14159;
	float mass = velocities[ gid ].w;

	float c = (315.0 * mass) / (64.0 * pi * h9);

	for (int i = 0; i < gl_NumWorkGroups.x; ++i) {
		if (i != gid) {		
			vec3 r = positions[ i ].xyz - pos;
			float r2 = dot(r,r);
			float z = (h*h - r2);
			if (z > 0.0) {
				total += c * z * z * z;
			}
		}
	}

	return total;
}


void main(){
	uint gid = gl_GlobalInvocationID.x;

	density[ gid ] = density_calculation( gid );

	pressure[ gid ] = k * (density[ gid ] - rho);
}