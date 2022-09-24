#version 330
#extension GL_ARB_compute_shader: enable
#extension GL_ARB_shader_storage_buffer_object: enable
#extension GL_ARB_arrays_of_arrays: enable

uniform float dt;
uniform vec3 gravity;
uniform vec3 wind;
uniform int polygons;
uniform int objects;


layout( std140, binding=4 ) buffer Pos {
	vec4 Positions[ ]; 
};

layout( std140, binding=5 ) buffer Vel {
	vec4 Velocities[ ];
};

layout( std140, binding=6 ) buffer Col {
	vec4 Colors[ ]; 
};

layout(std140, binding=7 ) buffer Tri {
	vec4[3] Triangles[ ];
};

layout(std140, binding=8 ) buffer Trans {
	mat4 Transforms[ ];
};

layout(std140, binding=9) buffer Data {
	int Amount[ ];  
};

layout( local_size_x = 1024, local_size_y = 1, local_size_z = 1 ) in;


// https://math.stackexchange.com/questions/544946/determine-if-projection-of-3d-point-onto-plane-is-within-a-triangle
bool inside(vec3 x , vec3 v0, vec3 v1, vec3 v2, vec3 _n) {
	vec3 u = v1 - v0;
	vec3 v = v2 - v0;
	vec3 n = cross(u, v);
	vec3 w = x - v0;

	float gamma = dot(cross(u, w), n) / dot(n,n);
	float beta = dot(cross(w, v), n) / dot(n,n);
	float alpha = 1.0 - gamma - beta;

	return alpha > 0.0 && beta > 0.0 && gamma > 0.0;
};


void main(){

	uint gid = gl_GlobalInvocationID.x;

	if (Colors[ gid ].w > 0.0) {
		float cr = 0.3;
		float cf = 0.05;

		Colors[ gid ].w -= dt;

		// delete these
		vec3 p = Positions[ gid ].xyz;
		float size = Positions[ gid ].w;
		vec3 v = Velocities[ gid ].xyz;
		float m = Velocities[ gid ].w;
		vec3 color = Colors[ gid ].xyz;

		vec3 acceleration = gravity;
		
		vec3 pp = Positions[ gid ].xyz + Velocities[ gid ].xyz * dt;
		vec3 vp = Velocities[ gid ].xyz + acceleration * dt;


		int last = 0;
		bool found = false;
		for (int j = 0; j < objects; ++j){
			if (found) break;

			for (int i = last; i < last + Amount[j]; ++i) {

				vec3 v0 = (Transforms[j] * Triangles[ i ][0]).xyz;
				vec3 v1 = (Transforms[j] * Triangles[ i ][1]).xyz;
				vec3 v2 = (Transforms[j] * Triangles[ i ][2]).xyz;

				vec3 perp = normalize(cross(v1 - v0, v2 - v0));

				float disA = dot(Positions[ gid ].xyz - v0, perp);
				float disB = dot(pp - v0, perp);

				float f = abs(disA) / abs(disB - disA);
				vec3 x = Positions[ gid ].xyz + Velocities[ gid ].xyz * dt * f;

				if ( disB * disA < 0.0 && inside(x, v0, v1, v2, perp)){
					
					Colors[ gid ].xyz = vec3(1.0,1.0,1.0); // delete here
					pp = pp - (1 + cr) * disA * perp;
					vec3 vn = dot(vp , perp) * perp;
					vec3 vt = vp - vn;
					vp = -cr * vn + (1 - cf) * vt;
					found = true;
					break;
				}
			}
			last += Amount[j];
		}

		Positions[ gid ].xyz = pp;
		Velocities[ gid ].xyz = vp;
	}
}