#version 330
#extension GL_ARB_compute_shader: enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout( std140, binding=4 ) buffer Pos
{
	vec4 Positions[ ]; // array of structures
};

layout( std140, binding=5 ) buffer Vel
{
	vec4 Velocities[ ]; // array of structures
};

layout( std140, binding=6 ) buffer Col
{
	vec4 Colors[ ]; // array of structures
};

layout( local_size_x = 1024, local_size_y = 1, local_size_z = 1 ) in;


void main()
{
	const vec3 G = vec3( 0., -9.8, 0. );
	const float DT = 1.0/24.0;

	uint gid = gl_GlobalInvocationID.x;

	vec3 p = Positions[ gid ].xyz;
	float size = Positions[ gid ].w;
	vec3 v = Velocities[ gid ].xyz;
	float m = Velocities[ gid ].w;
	vec3 color = Colors[ gid ].xyz;
	float life = Colors[ gid ].w;
	

	vec3 pp = p + v*DT;
	vec3 vp = v + G*DT;

	Positions[ gid ].xyz = pp;
	Velocities[ gid ].xyz = vp;	
}