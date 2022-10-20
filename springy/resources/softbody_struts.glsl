#version 460
#extension GL_ARB_shading_language_include : require
#extension GL_NV_shader_atomic_float : require

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
layout(std430, binding=3 ) buffer Struts {
	strut_data struts[ ];
};

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;
	

void main(){

    // for every strut with gid
	uint gid = gl_GlobalInvocationID.x;

    strut_data strut = struts[ gid ];

    vec3 force_buffer_0 = vec3(0.0);
	vec3 force_buffer_1 = vec3(0.0);
    vec3 force_buffer_2 = vec3(0.0);
    vec3 force_buffer_3 = vec3(0.0);

    // torsional forces
    face_data face_left = faces[ strut.f1 ];
    face_data face_right = faces[ strut.f2 ];

    int p0 = strut.v1;
    int p1 = strut.v2;
    strut_data _s = face_left.s1 == gid ? struts[face_left.s2] : struts[face_left.s1];
    int p2 = _s.v1 == p0 || _s.v1 == p1 ? _s.v2 : _s.v1;
    _s = face_right.s1 == gid ? struts[face_right.s2] : struts[face_right.s1];
    int p3 = _s.v1 == p0 || _s.v1 == p2 ? _s.v2 : _s.v1;

    vec3 x0 = vec3(positions[ p0*3 ], positions[ p0*3 + 1 ], positions[ p0*3 + 2 ]);
	vec3 x1 = vec3(positions[ p1*3 ], positions[ p1*3 + 1 ], positions[ p1*3 + 2 ]);
	vec3 x2 = vec3(positions[ p2*3 ], positions[ p2*3 + 1 ], positions[ p2*3 + 2 ]);
    vec3 x3 = vec3(positions[ p3*3 ], positions[ p3*3 + 1 ], positions[ p3*3 + 2 ]);

    // if edge strut, dont compute torsional forces
    // if (strut.f1 != -1 && strut.f2 != -1) {

    //     vec3 nl = normalize(cross(x1-x0, x2-x0));
    //     vec3 nr = normalize(cross(x3-x0, x1- x0));

    //     vec3 h = normalize(x1-x0);
    //     float d02 = dot(x2-x0, h);
    //     float d03 = dot(x3-x0, h);
    //     vec3 rl = (x2-x0) - d02*h;
    //     vec3 rr = (x3-x0) - d03*h;

    //     float sl = dot(particles[p2].velocity.xyz, nl);
    //     float sr = dot(particles[p3].velocity.xyz, nr);

    //     float theta = atan(dot(cross(nl, nr), h), dot(nl, nr));

    //     float dtheta_left =  sl/ length(rl);
    //     float dtheta_right =  sr/ length(rr);

    //     float torsion = strut.tk * (theta - strut.to);
    //     float damping = -strut.td * (dtheta_left + dtheta_right);
    //     vec3 torque = (torsion + damping) * h;

    //     // if edge dont add them
    
    //     force_buffer_2 = (dot(torque, h) / length(rl)) * nl;
    //     force_buffer_3 = (dot(torque, h) / length(rr)) * nr;
    //     force_buffer_1 = - ( (d02*force_buffer_2 + d03*force_buffer_3) / length(x1-x0));
    //     force_buffer_0 = -(force_buffer_1 + force_buffer_2 + force_buffer_3);

    // }
	
    // spring and damping force on strut
    vec3 dx = x1-x0;
    vec3 v0 = particles[ p0 ].velocity.xyz;
    vec3 v1 = particles[ p1 ].velocity.xyz;
    vec3 Fs = strut.k * (length(dx) - strut.lo) * normalize(dx);
	vec3 Fd = strut.d * dot(v1 - v0, normalize(dx)) * normalize(dx);

    force_buffer_1 += Fs + Fd;
	force_buffer_2 -= Fs + Fd;
	
	//atomic add - because each struct adds force to shared vertices
	atomicAdd(particles[p0].force.x, force_buffer_0.x);
	atomicAdd(particles[p0].force.y, force_buffer_0.y);
	atomicAdd(particles[p0].force.z, force_buffer_0.z);

	atomicAdd(particles[p1].force.x, force_buffer_1.x);
	atomicAdd(particles[p1].force.y, force_buffer_1.y);
	atomicAdd(particles[p1].force.z, force_buffer_1.z);

    atomicAdd(particles[p2].force.x, force_buffer_2.x);
	atomicAdd(particles[p2].force.y, force_buffer_2.y);
	atomicAdd(particles[p2].force.z, force_buffer_2.z);

    atomicAdd(particles[p3].force.x, force_buffer_3.x);
	atomicAdd(particles[p3].force.y, force_buffer_3.y);
	atomicAdd(particles[p3].force.z, force_buffer_3.z);
}
