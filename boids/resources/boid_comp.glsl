#version 460
#extension GL_ARB_shading_language_include : require

uniform float dt;
uniform vec3 gravity;
uniform vec3 wind;
uniform vec3 k;
uniform vec4 attention; 
uniform int polygons;
uniform int objects;
uniform float time;

layout (binding = 0, offset=0) uniform atomic_uint counters[3];


layout( std140, binding=4 ) buffer Pos {
	vec4 positions[ ]; 
};

layout( std140, binding=5 ) buffer Vel {
	vec4 velocities[ ];
};

layout( std140, binding=6 ) buffer Col {
	vec4 colors[ ]; 
};

layout(std140, binding=7 ) buffer Tri {
	vec4[3] triangles[ ];
};

layout(std140, binding=8 ) buffer Trans {
	mat4 transforms[ ];
};

layout(std430, binding=9) buffer Data {
	int poly_count[ ];  
};

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;


vec3 sum_accelerations( uint gid ) {
	vec3 boid_pos = positions[ gid ].xyz;
	vec3 boid_vel = velocities[ gid ].xyz;
	vec3 result = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < gl_NumWorkGroups.x; ++i) {
		if (i != gid) {
			vec3 other_boid_pos = positions[ i ].xyz;
			vec3 other_boid_vel = velocities[ i ].xyz;
			
			vec3 x = other_boid_pos - boid_pos;
			float d = length(x);
			float theta = acos(clamp(dot(normalize(boid_vel), normalize(x)), -0.9999, 0.9999));

			float kd = 1.0;
			float kt = 1.0;
			if (d >= attention.y - 0.00001 || theta >= attention.w - 0.00001) {
				continue;
			}
			if (d >= attention.x) {
				kd = (attention.y - d) / (attention.y - attention.x);
			}
			if (theta >= attention.z) {
				kt = (attention.w - theta) / (attention.w - attention.z);
			}

			float budget = 10.0;

			vec3 temp = -(k.x / d) * normalize(other_boid_pos - boid_pos) * kd * kt;
			vec3 collision_avoidance = min(budget, length(temp)) * normalize(temp);
			budget -= length(collision_avoidance);

			temp = k.y * (other_boid_vel - boid_vel) * kd * kt;
			vec3 velocity_matching = min(budget, length(temp)) * normalize(temp);
			budget -= length(collision_avoidance);

			temp = k.z * (x) * kd * kt;
			vec3 centering = min(budget, length(temp)) * normalize(temp);

			result += collision_avoidance + velocity_matching + centering;
		}
	}

	return result;
}

void main(){

	uint gid = gl_GlobalInvocationID.x;

	vec3 acceleration = sum_accelerations( gid );

	vec3 next_position = positions[ gid ].xyz + velocities[ gid ].xyz * dt;
	vec3 temp = velocities[ gid ].xyz + acceleration * dt;
	vec3 next_velocity = min(100.0, length(temp)) * normalize(temp);
	
	positions[ gid ].xyz = next_position;
	velocities[ gid ].xyz = next_velocity;
}



