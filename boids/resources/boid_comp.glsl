#version 460
#extension GL_ARB_shading_language_include : require

uniform float 	dt;
uniform vec3 	gravity;
uniform vec3 	wind;
uniform vec3 	k;
uniform vec4 	attention; 
uniform int 	objects;
uniform float 	time;
uniform float 	steering_speed; 
uniform float 	speed_limit;
uniform float 	acceleration_limit;
uniform float 	vision_distance;
uniform float 	minimum_speed;
uniform int 	boid_count;
uniform float 	predator_speed;
uniform float 	predator_avoidance;
uniform float 	predator_avoidance_internal;
uniform float 	predator_attention_radius;
uniform int		dups; 
uniform int		dims; 
uniform float 	spacing; // h

const float		e = 0.00001;

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
layout(std430, binding=9) buffer Aabb {
	vec4[2] corners[ ];
};
layout(std430, binding=13) buffer Grid {
	int cells[ ];
};

layout( local_size_x = 1, local_size_y = 1, local_size_z = 1 ) in;

// spatial hashing function
int hash(float x, float y, float z) {
    float ds = spacing / float(dims); 
	float offset = spacing / 2.0;
    int xi = int(floor((x + offset) / ds)) * dups;
    int yi = int(floor((y + offset) / ds)) * dims * dups;
    int zi = int(floor((z + offset) / ds)) * dims * dims * dups;
    return xi + yi + zi;
}


// ray traced boid obstacle avoidance
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
   t[9] = (t[8] < 0 || t[7] > t[8]) ? -1.0 : t[7];
   return t[9];
}
vec3 obstacle_avoidance( vec3 o, vec3 ray) { 
	int last = 0;
	float closest_obstacle = 1.0/e; // hacky
	vec3 response = vec3(e);
	for (int j = 0; j < objects; ++j){
		vec3 minimum = corners[j][0].xyz;
		vec3 maximum = corners[j][1].xyz;

		float box_hit = ray_box_intersect(o, ray, minimum, maximum);

		if (box_hit > 0.0 && box_hit < vision_distance) {
		
			for (int i = last; i < last + poly_count[j]; ++i) {

				vec3 vertex_0 = triangles[ i ][0].xyz;
				vec3 vertex_1 = triangles[ i ][1].xyz;
				vec3 vertex_2 = triangles[ i ][2].xyz;

				float t = triangle_hit(o, ray, vertex_0, vertex_1, vertex_2);

				if (t > 0.0  && t < vision_distance && t < closest_obstacle){
					vec3 n = normalize(cross(vertex_1 - vertex_0, vertex_2 - vertex_0));
					closest_obstacle = t;
					float r = (t/vision_distance);
					response = (steering_speed * n) / (r*r);
				}
			}
		}
		last += poly_count[j];
	}
	return response;

}
vec3 path_tracing(vec3 o, vec3 v) {
	vec3 result = vec3(0.0);

	// directions to cast our rays for obstacle avoidance
	vec3 forward = normalize(v);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 side = normalize(cross(forward, up));
	vec3 up_right = normalize(up + side + 2.0 * forward);
	vec3 up_left = normalize(up - side + 2.0 * forward);
	vec3 down_left = normalize(-up - side + 2.0 * forward);
	vec3 down_right = normalize(-up + side + 2.0 * forward);

	result += obstacle_avoidance(o, forward);
	result += obstacle_avoidance(o, up_right);
	result += obstacle_avoidance(o, down_left);
	result += obstacle_avoidance(o, up_left);
	result += obstacle_avoidance(o, down_right);

	return result;
}


// boid behavior
vec3 boid_behavior_main(uint gid, int index, vec3 boid_pos, vec3 boid_vel) {
	vec3 total = vec3(0.0);

	for (int i = 0; i < dups && cells[index+i] != -1; ++i) {
		int other_idx = cells[index+i];
		
		if (gid == other_idx || other_idx >= boid_count || other_idx == -2) continue;

		vec3 other_boid_pos = positions[ other_idx ].xyz;
		vec3 other_boid_vel = velocities[ other_idx ].xyz;
		
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
vec3 boid_behavior( uint gid ) {
	vec3 boid_pos = positions[ gid ].xyz;
	vec3 boid_vel = velocities[ gid ].xyz;
	vec3 total = vec3(0.0);
	int index = hash(boid_pos.x, boid_pos.y, boid_pos.z);

	int offset = 1;
	int box_dim = 1 + 2 * offset;
	int start_index = index - offset*dups - offset*dims*dups - offset*dims*dims*dups;

	for (int z = 0; z < box_dim; ++z) {
		for (int y = 0; y < box_dim; ++y) {
			for (int x = 0; x < box_dim; ++x) {
				int curr = start_index + x*dups + y*dims*dups + z*dims*dims*dups;
				total += boid_behavior_main(gid, curr, boid_pos, boid_vel);
			}
		}
	}
	
	return total;
}


// predator hunts boids that are close, and avoids other predators regardless of distance
vec3 predator_behavior_main(uint gid, int index, vec3 predator_pos, vec3 predator_vel) {
	vec3 total = vec3(0.0);
	for (int i = 0; i < dups && cells[index+i] != -1; ++i) {
		int other_idx = cells[index+i];
		if (other_idx == -2) continue;

		vec3 x = positions[ other_idx ].xyz - predator_pos;
		float d = length(x);

		if (d >= predator_attention_radius - e) {
			continue;
		}

		total += (k.x / d) * normalize(x) * predator_speed;
	}

	return total;
}
vec3 predator_behavior( uint gid ) {
	vec3 predator_pos = positions[ gid ].xyz;
	vec3 predator_vel = velocities[ gid ].xyz;
	vec3 total = vec3(0.0);

	int predator_index = hash(predator_pos.x, predator_pos.y, predator_pos.z);

	int offset = 1;
	int box_dim = 1 + 2 * offset;
	int start_index = predator_index - offset*dups - offset*dims*dups - offset*dims*dims*dups;

	for (int z = 0; z < box_dim; ++z) {
		for (int y = 0; y < box_dim; ++y) {
			for (int x = 0; x < box_dim; ++x) {
				int curr = start_index + x*dups + y*dims*dups + z*dims*dims*dups;
				total += predator_behavior_main(gid, curr, predator_pos, predator_vel);
			}
		}
	}

	return total;
}


// avoid predators : OKAY
vec3 predator_influence_on_object( uint gid ) {
	vec3 pos = positions[ gid ].xyz;
	vec3 vel = velocities[ gid ].xyz;
	vec3 total = vec3(0.0);

	for (int i = boid_count; i < gl_NumWorkGroups.x; ++i) {
		if (gid == i) continue;

		vec3 predator_pos = positions[ i ].xyz;
		vec3 predator_vel = velocities[ i ].xyz;
		
		vec3 x = predator_pos - pos;
		float d = length(x);

		if (d >= attention.y - e) {
			continue;
		}

		float c = gid >= boid_count ? predator_avoidance_internal : predator_avoidance;
		total += -(k.x/d) * normalize(x) * c;
	}
	return total;
}


void main(){
	// defines our acceleration priority
	float budget = acceleration_limit;


	// get index of object 
	uint gid = gl_GlobalInvocationID.x;
	bool predator = gl_GlobalInvocationID.x >= boid_count;


	// position and velocity of object
	vec3 o = positions[ gid ].xyz;
	vec3 v = velocities[ gid ].xyz;

	// PRIORITY 1: AVOID OBSTACLES
	vec3 unbounded_obstacle_avoidance = vec3(0.0); //path_tracing(o, v);
	vec3 acceleration = min(budget, length(unbounded_obstacle_avoidance)) * normalize(unbounded_obstacle_avoidance);
	budget = acceleration_limit - length(acceleration);


	// PRIORITY 2: AVOID PREDATOR, OR CHASE PREY
	vec3 predator_influence = predator_influence_on_object( gid );
	acceleration += length(predator_influence) > e ? min(budget, length(predator_influence)) * normalize(predator_influence): vec3(0.0);
	budget = acceleration_limit - length(acceleration);


	// PRIORITY 3: IF BOID, BEHAVE LIKE A BOID
	vec3 unbounded_boid_influence = predator ? predator_behavior(gid) : boid_behavior( gid );
	acceleration += length(unbounded_boid_influence) > e ? min(budget, length(unbounded_boid_influence)) * normalize(unbounded_boid_influence):  vec3(0.0);


	vec3 next_position = o + v * dt;
	vec3 t = v + acceleration * dt;
	vec3 next_velocity = predator ? clamp(length(t), minimum_speed ,speed_limit) * normalize(t) : t;

	int started = hash(o.x, o.y, o.z);
	int ended = hash(next_position.x, next_position.y, next_position.z);

	if (started != ended) {
		int i = started;
		while (cells[i] != int(gid) && i-started <= dups) ++i;
		cells[i] = -2;
		i = ended;
		while (cells[i] > 0 && i-ended <= dups) ++i;
		cells[i] = int(gid);
	}
	
	positions[ gid ].xyz = next_position;
	velocities[ gid ].xyz = next_velocity;

}