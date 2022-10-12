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
uniform float steering_speed; // change these names for the love of god
uniform float speed_limit;
uniform float acceleration_limit;
uniform float vision_distance;
uniform float minimum_speed;
uniform int boid_count;
uniform float predator_speed;
uniform float predator_avoidance;
uniform float predator_avoidance_internal;
uniform float predator_attention_radius;


const float e = 0.00001;

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
layout( std140, binding=10 ) buffer Pred_Pos {
	vec4 predator_positions[ ]; 
};
layout( std140, binding=11 ) buffer Pred_Vel {
	vec4 predator_velocities[ ];
};
layout( std140, binding=12 ) buffer Pred_Col {
	vec4 predator_colors[ ]; 
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
   t[9] = (t[8] < 0.0 || t[7] > t[8]) ? -1.0 : t[7] < 0.0 ? t[8] : t[7];
   return t[9];
}

vec3 obstacle_avoidance( uint gid , vec3 o, vec3 ray) { 
	int last = 0;
	float closest_obstacle = 1.0/e; // hacky
	vec3 response = vec3(e);
	for (int j = 0; j < objects; ++j){
		vec3 minimum = corners[j][0].xyz;
		vec3 maximum = corners[j][1].xyz;

		// float box_hit = ray_box_intersect(o, ray, minimum, maximum);

		// if (box_hit > 0.0 && box_hit < attention.y) {
			for (int i = last; i < last + poly_count[j]; ++i) {

				vec3 vertex_0 = triangles[ i ][0].xyz;
				vec3 vertex_1 = triangles[ i ][1].xyz;
				vec3 vertex_2 = triangles[ i ][2].xyz;

				float t = triangle_hit(o, ray, vertex_0, vertex_1, vertex_2);

				if (t > 0.0  && t < vision_distance && t < closest_obstacle){
					vec3 n = normalize(cross(vertex_1 - vertex_0, vertex_2 - vertex_0));
					closest_obstacle = t;
					float r = (t/vision_distance);
					response = steering_speed * n / (r * r);
				}
			}
		//}
		last += poly_count[j];
	}
	return response;

}

vec3 boid_influences( uint gid ) {
	vec3 boid_pos = positions[ gid ].xyz;
	vec3 boid_vel = velocities[ gid ].xyz;
	vec3 total = vec3(0.0);

	for (int i = 0; i < boid_count; ++i) {
		if (i == gid) continue;

		vec3 other_boid_pos = positions[ i ].xyz;
		vec3 other_boid_vel = velocities[ i ].xyz;
		
		vec3 x = other_boid_pos - boid_pos;
		float d = length(x);
		float theta = acos(clamp(dot(normalize(boid_vel), normalize(x)), -1.0+e, 1.0-e));

		if (d >= attention.y - e || theta >= attention.w) continue;

		float kd = d >= attention.x ? (attention.y - d) / (attention.y - attention.x) : 1.0;
		float kt = theta >= attention.z ? (attention.w - theta) / (attention.w - attention.z) : 1.0;

		// where the magic happens
		vec3 collision_avoidance = -(k.x / d) * normalize(x) * kd * kt;
		vec3 velocity_matching = k.y * (other_boid_vel - boid_vel) * kd * kt;
		vec3 centering = k.z * (x) * kd * kt;

		total += collision_avoidance + velocity_matching + centering;
		
	}

	return total;
}

vec3 predator_hunting( uint gid ) {
	vec3 predator_pos = predator_positions[ gid ].xyz;
	vec3 predator_vel = predator_velocities[ gid ].xyz;
	vec3 total = vec3(0.0);

	for (int i = 0; i < boid_count; ++i) {

		vec3 other_boid_pos = positions[ i ].xyz;
		vec3 other_boid_vel = velocities[ i ].xyz;
		
		vec3 x = other_boid_pos - predator_pos;
		float d = length(x);
		float theta = acos(clamp(dot(normalize(predator_vel), normalize(x)), -1.0+e, 1.0-e));

		float kt = 1.0;
		if (d >= predator_attention_radius - e || theta >= attention.w - e) {
			continue;
		}

		total += (k.x / d) * normalize(x) * kt * predator_speed;
	}

	for (int i = 0; i < gl_NumWorkGroups.x - boid_count; ++i) {
		if (gid == i) continue;

		vec3 other_predator_pos = predator_positions[ i ].xyz;
		vec3 other_predator_vel = predator_velocities[ i ].xyz;
		
		vec3 x = other_predator_pos - predator_pos;
		float d = length(x);

		if (d >= predator_attention_radius - e) {
			continue;
		}

		total += -k.x  * normalize(x) * predator_avoidance_internal;
	}
	return total;
}

vec3 predator_influence( uint gid ) {
	vec3 boid_pos = positions[ gid ].xyz;
	vec3 boid_vel = velocities[ gid ].xyz;
	vec3 total = vec3(0.0);

	for (int i = 0; i < gl_NumWorkGroups.x - boid_count; ++i) {

		vec3 predator_pos = predator_positions[ i ].xyz;
		vec3 predator_vel = predator_velocities[ i ].xyz;
		
		vec3 x = predator_pos - boid_pos;
		float d = length(x);

		if (d >= attention.y - e) {
			continue;
		}

		total += -(k.x) * normalize(x) * predator_avoidance;
	}
	return total;
}

void main(){
	// defines our acceleration priority
	float budget = acceleration_limit;

	// get index of object 
	uint _gid = gl_GlobalInvocationID.x;
	bool predator = gl_GlobalInvocationID.x >= boid_count;
	uint gid = _gid - (boid_count * int(predator));

	// position and velocity of object
	vec3 o = predator ? predator_positions[ gid ].xyz : positions[ gid ].xyz;
	vec3 v = predator ? predator_velocities[ gid ].xyz : velocities[ gid ].xyz;

	// directions to cast our rays for obstacle avoidance
	vec3 forward = normalize(v);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 side = normalize(cross(forward, up));
	vec3 up_right = normalize(up + side + forward);
	vec3 up_left = normalize(up - side + forward);
	vec3 down_left = normalize(-up - side + forward);
	vec3 down_right = normalize(-up + side + forward);

	// PRIORITY 1: AVOID OBSTACLES
	vec3 unbounded_obstacle_avoidance = obstacle_avoidance(gid, o, forward) + 
										obstacle_avoidance(gid, o, up_right) + 
										obstacle_avoidance(gid, o, up_left) + 
										obstacle_avoidance(gid, o, down_left) + 
										obstacle_avoidance(gid, o, down_right);
	vec3 acceleration = min(budget, length(unbounded_obstacle_avoidance)) * normalize(unbounded_obstacle_avoidance);
	budget = acceleration_limit - length(acceleration);

	// PRIORITY 2: AVOID PREDATOR, OR CHASE PREY
	vec3 predator_influence = predator ? predator_hunting( gid) : predator_influence( gid );
	acceleration += length(predator_influence) > e ? min(budget, length(predator_influence)) * normalize(predator_influence): vec3(0.0);
	budget = acceleration_limit - length(acceleration);

	// PRIORITY 3: IF BOID, BEHAVE LIKE A BOID
	vec3 unbounded_boid_influence = predator ? vec3(0.0) : boid_influences( gid );
	acceleration += length(unbounded_boid_influence) > e ? min(budget, length(unbounded_boid_influence)) * normalize(unbounded_boid_influence):  vec3(0.0);

	if (predator) {
		vec3 next_position = predator_positions[ gid ].xyz + predator_velocities[ gid ].xyz * dt;
		vec3 t = predator_velocities[ gid ].xyz + acceleration * dt;
		vec3 next_velocity = clamp(length(t), minimum_speed ,speed_limit) * normalize(t);
		
		predator_positions[ gid ].xyz = next_position;
		predator_velocities[ gid ].xyz = next_velocity;
	} else {
		vec3 next_position = positions[ gid ].xyz + velocities[ gid ].xyz * dt;
		vec3 t = velocities[ gid ].xyz + acceleration * dt;
		vec3 next_velocity = clamp(length(t), minimum_speed ,speed_limit) * normalize(t);
		
		positions[ gid ].xyz = next_position;
		velocities[ gid ].xyz = next_velocity;
	}
}