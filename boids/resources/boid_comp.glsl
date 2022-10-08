#version 460
#extension GL_ARB_shading_language_include : require

uniform float dt;
uniform vec3 gravity;
uniform vec3 wind;
uniform int objects;
uniform float time;
uniform float h;
uniform float mu;
uniform float k;
uniform float rho;
uniform float damping;

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

layout(std430, binding=8) buffer Data {
	int poly_count[ ];  
};

layout(std140, binding=9) buffer AABB {
	vec4[2] corners[ ];
};

layout(std430, binding=10) buffer Density {
	float density[ ];  
};

layout( std140, binding=11 ) buffer Acc {
	vec4 prev_accelerations[ ];
};

layout( std430, binding=12 ) buffer Pre {
	float pressure[ ];
};

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

// https://math.stackexchange.com/questions/544946/determine-if-projection-of-3d-point-onto-plane-is-within-a-triangle
bool inside(vec3 collision_position , vec3 vertex_0, vec3 vertex_1, vec3 vertex_2) {
	vec3 u = vertex_1 - vertex_0;
	vec3 v = vertex_2 - vertex_0;
	vec3 n = cross(u, v);
	vec3 w = collision_position - vertex_0;

	float gamma = dot(cross(u, w), n) / dot(n,n);
	float beta = dot(cross(w, v), n) / dot(n,n);
	float alpha = 1.0 - gamma - beta;

	return alpha >= -0.00001 && beta >= -0.00001 && gamma >= -0.00001;
};

vec3 forces(uint gid) {
	vec3 total = vec3(0.0, 0.0, 0.0);
	float pi = 3.14159;
	float h3 = h * h * h;
	float h6 = h3 * h3;
	float mass = velocities[ gid ].w;
	vec3 pos = positions[ gid ].xyz;
	vec3 vel = velocities[ gid ].xyz;

	float c = - (45.0 * mass) / (pi * h6);
	float c2 = (45.0 * mu * mass) / (pi * h6);
	
	for (int i = 0; i < gl_NumWorkGroups.x; ++i) {
		vec3 r = positions[ i ].xyz - pos;
		float z = (h - length(r));
		if ( i == gid || z < 0.0) {
			continue;
		}

		vec3 v = velocities[ i ].xyz - vel;
		vec3 q = r / length(r);
		total += c * -(q) * ((pressure[ gid ] + pressure[ i ]) / (2.0 * density[ i ])) * (z*z);
		total += c2 * (v / density[ i ]) * z;
		
	}
	total += gravity;
	return total;
}

void main(){
	uint gid = gl_GlobalInvocationID.x;

	vec3 acceleration = forces(gid) / (density[ gid ] + 0.0000001);
	
	vec3 next_velocity = velocities[ gid ].xyz 	+ acceleration 	* dt;
	vec3 next_position =  positions[ gid ].xyz  + velocities[ gid ].xyz * dt;
	
	prev_accelerations[ gid ].xyz = acceleration; 

	int last = 0;
	bool collision_found = false;
	for (int j = 0; j < objects; ++j){
		if (collision_found) break;

		// vec3 object_origin = vec3(transforms[j][3][0], transforms[j][3][1], transforms[j][3][2]);
		// float object_radius = length(vec3(transforms[j][0][0], transforms[j][0][1], transforms[j][0][2]));

		// if (length(object_origin - next_position) > object_radius && length(object_origin - positions[ gid ].xyz) > object_radius) {
		// 	continue; // sphere bounding box lets us skip mesh if particle outside sphere, good for high poly meshes
		// }

		for (int i = last; i < last + poly_count[j]; ++i) {

			vec3 vertex_0 = triangles[ i ][0].xyz;
			vec3 vertex_1 = triangles[ i ][1].xyz;
			vec3 vertex_2 = triangles[ i ][2].xyz;

			vec3 normal = normalize(cross(vertex_1 - vertex_0, vertex_2 - vertex_0));

			float d_A = dot(positions[ gid ].xyz - vertex_0, normal);
			float d_B = dot(next_position - vertex_0, normal);

			float f = abs(d_A) / abs(d_B - d_A);
			vec3 collision_position = positions[ gid ].xyz +  next_velocity * dt * f;

			if ( d_B * d_A < 0.0 && inside(collision_position, vertex_0, vertex_1, vertex_2)){
				next_position = collision_position;
				next_velocity = damping * next_velocity;

				collision_found = true;
				break;
			}
		}
		last += poly_count[j];
	}

	positions[ gid ].xyz = next_position;
	velocities[ gid ].xyz = next_velocity;
}