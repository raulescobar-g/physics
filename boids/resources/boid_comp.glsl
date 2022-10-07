#version 460
#extension GL_ARB_shading_language_include : require

uniform float dt;
uniform vec3 gravity;
uniform vec3 wind;
uniform vec3 k;
uniform vec4 attention; 
uniform int objects;
uniform float time;
//
uniform float steering_speed;
uniform vec4 limits; // vel,acc,dist,sleep

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

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;


// from some paper i forgot where i got this from
float triangle_hit(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2) {

	/* find vectors for two edges sharing vert0 */
	vec3 edge1 = vert1 - vert0;
	vec3 edge2 = vert2 - vert0;

	/* begin calculating determinant - also used to calculate U parameter */
	vec3 pvec = cross(dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	float det = dot(edge1, pvec);

	if (det > -0.0001 && det < 0.0001) {
		return -1.0;
	}
	float inv_det = 1.0 / det;

	/* calculate distance from vert0 to ray origin */
	vec3 tvec = orig - vert0;

	/* calculate U parameter and test bounds */
	float u = dot(tvec, pvec) * inv_det;
	if (u < 0.0 || u > 1.0) {
		return -1.0;
	}

	/* prepare to test V parameter */
	vec3 qvec = cross(tvec, edge1);

	/* calculate V parameter and test bounds */
	float v = dot(dir, qvec) * inv_det;
	if (v < 0.0 || u + v > 1.0){
		return -1.0;
	} 

	return dot(edge2, qvec) * inv_det;
}

// branchless aabb-ray intersection 
// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
float ray_box_intersect ( vec3 rpos, vec3 rdir, vec3 vmin, vec3 vmax ){
   float t[10];
   t[1] = (vmin.x - rpos.x)/rdir.x;
   t[2] = (vmax.x - rpos.x)/rdir.x;
   t[3] = (vmin.y - rpos.y)/rdir.y;
   t[4] = (vmax.y - rpos.y)/rdir.y;
   t[5] = (vmin.z - rpos.z)/rdir.z;
   t[6] = (vmax.z - rpos.z)/rdir.z;
   t[7] = max(max(min(t[1], t[2]), min(t[3], t[4])), min(t[5], t[6]));
   t[8] = min(min(max(t[1], t[2]), max(t[3], t[4])), max(t[5], t[6]));
   t[9] = (t[8] < 0.0 || t[7] > t[8]) ? -1.0 : t[7];
   return t[9];
}

vec3 obstacle_avoidance( uint gid , vec3 ray) { 
	vec3 o = positions[ gid ].xyz;
	

	int last = 0;
	float closest_obstacle = 999999999.0; // hacky
	vec3 response = vec3(0.0);
	for (int j = 0; j < objects; ++j){
		vec3 minimum = corners[j][0].xyz;
		vec3 maximum = corners[j][1].xyz;

		// float box_hit = ray_box_intersect(o, ray, minimum, maximum);

		// if (box_hit < 0.0) {
		// 	continue;
		// }

		for (int i = last; i < last + poly_count[j]; ++i) {

			vec3 vertex_0 = triangles[ i ][0].xyz;
			vec3 vertex_1 = triangles[ i ][1].xyz;
			vec3 vertex_2 = triangles[ i ][2].xyz;

			float t = triangle_hit(o, ray, vertex_0, vertex_1, vertex_2);

			if (t > 0.0  && t < limits.z && t < closest_obstacle){
				vec3 n = normalize(cross(vertex_1 - vertex_0, vertex_2 - vertex_0));
				closest_obstacle = t;
				response = steering_speed * n / t;
			}
		}
		last += poly_count[j];
	}
	return response;

}

vec3 boid_influences( uint gid ) {
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
			if (d >= attention.y - 0.0000001 || theta >= attention.w - 0.0000001) {
				continue;
			}
			if (d >= attention.x) {
				kd = (attention.y - d) / (attention.y - attention.x);
			}
			if (theta >= attention.z) {
				kt = (attention.w - theta) / (attention.w - attention.z);
			}

			vec3 collision_avoidance = -(k.x / d) * normalize(other_boid_pos - boid_pos) * kd * kt;

			vec3 velocity_matching = k.y * (other_boid_vel - boid_vel) * kd * kt;

			vec3 centering = k.z * (x) * kd * kt;

			result += collision_avoidance + velocity_matching + centering;
		}
	}

	return result;
}

void main(){
	float budget = limits.y;

	uint gid = gl_GlobalInvocationID.x;

	vec3 ray = normalize(velocities[ gid ].xyz);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 side = normalize(cross(ray, up));

	vec3 temp = obstacle_avoidance(gid, ray);
	
	vec3 acceleration = length(temp) > 0.001 ? min(budget, length(temp)) * normalize(temp) : vec3(0.0, 0.0, 0.0);
	budget -= length(acceleration);

	temp = boid_influences( gid );
	acceleration += length(temp) > 0.001 ? min(budget, length(temp)) * normalize(temp):  vec3(0.0, 0.0, 0.0);

	vec3 next_position = positions[ gid ].xyz + velocities[ gid ].xyz * dt;


	temp = velocities[ gid ].xyz + acceleration * dt;
	vec3 next_velocity = length(temp) < limits.w ? velocities[ gid ].xyz : min(limits.x, length(temp)) * normalize(temp);
	
	positions[ gid ].xyz = next_position;
	velocities[ gid ].xyz = next_velocity;
}