#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_NV_shader_atomic_float : require

uniform vec3 wind;

struct particle_data {
	vec4 velocity,force;
};
struct face_data {
	int s1,s2,s3;
	float a12,a23,a31;
};
struct strut_data {
	float k,d,lo;
    float tk,td,to;
	int v1,v2;
	int f1,f2;
};

layout( std430, binding=0 ) buffer Positions {
	float positions[ ]; 
};
layout( std430, binding=1 ) buffer Particles {
	particle_data particles[ ];
};
layout(std430, binding=2 ) buffer Faces {
	face_data faces[ ];
};
layout( std430, binding=3 ) buffer Struts {
	strut_data struts[ ];
};

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

void main(){
	// init memory and constants
	face_data face;
	strut_data strut_1,strut_2,strut_3;
	particle_data particle_1,particle_2,particle_3;

	float cd = -0.3;
	float cl = -0.2;
	
	uint gid = gl_GlobalInvocationID.x;

	// extract relevant data
	face = faces[ gid ];
	strut_1 = struts[ face.s1 ];
	strut_2 = struts[ face.s2 ];
	strut_3 = struts[ face.s3 ];

	int p1_idx = strut_1.v1;
	int p2_idx = strut_1.v2;
	int p3_idx = strut_2.v1 == p1_idx || strut_2.v1 == p2_idx ? strut_2.v2: strut_2.v1;

	particle_1 = particles[p1_idx];
	particle_2 = particles[p2_idx];
	particle_3 = particles[p3_idx];

	vec3 force_buffer_1 = vec3(0.0); // move to its own shader
	vec3 force_buffer_2 = vec3(0.0);
	vec3 force_buffer_3 = vec3(0.0);

	vec3 position_1 = vec3(positions[p1_idx * 3], positions[p1_idx * 3 + 1], positions[p1_idx * 3 + 2]);
	vec3 position_2 = vec3(positions[p2_idx * 3], positions[p2_idx * 3 + 1], positions[p2_idx * 3 + 2]);
	vec3 position_3 = vec3(positions[p3_idx * 3], positions[p3_idx * 3 + 1], positions[p3_idx * 3 + 2]);

	//calculate force on face
	vec3 v = (particle_1.velocity.xyz + particle_2.velocity.xyz + particle_3.velocity.xyz) / 3.0; // fix this part HERE
	vec3 norm = cross(position_2 - position_1, position_3 - position_1);

	vec3 n = normalize(norm);
	vec3 vr = v - wind;
	float A = length(norm) / 2.0;
	vec3 q = cross(n, vr);
	float Ae = A * dot(n,vr);
	vec3 lift_dir = cross(vr, normalize(q));

	vec3 Fda = cd * Ae * vr;
	vec3 Fl = cl * Ae * lift_dir;
	vec3 f = Fda + Fl;

	// distribute force on face to vertices proportional to angle size
	force_buffer_2 += f * (face.a12/3.14159);
	force_buffer_3 += f * (face.a23/3.14159);
	force_buffer_1 += f * (face.a31/3.14159);

	
	//atomic add - because each face has shared struts and vertices
	atomicAdd(particles[p1_idx].force.x, force_buffer_1.x);
	atomicAdd(particles[p1_idx].force.y, force_buffer_1.y);
	atomicAdd(particles[p1_idx].force.z, force_buffer_1.z);

	atomicAdd(particles[p2_idx].force.x, force_buffer_2.x);
	atomicAdd(particles[p2_idx].force.y, force_buffer_2.y);
	atomicAdd(particles[p2_idx].force.z, force_buffer_2.z);

	atomicAdd(particles[p3_idx].force.x, force_buffer_3.x);
	atomicAdd(particles[p3_idx].force.y, force_buffer_3.y);
	atomicAdd(particles[p3_idx].force.z, force_buffer_3.z);
}