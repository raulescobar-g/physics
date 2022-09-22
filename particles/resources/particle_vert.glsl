#version 330
#extension GL_ARB_shader_storage_buffer_object : enable

uniform mat4 pP;
uniform mat4 V;

in vec3 vertices;
in vec4 position;
in vec4 color;

out vec4 particleColor;


void main()
{

    
    float particleSize = position.w;
    vec3 center = position.xyz;

    vec4 position_viewspace = V * vec4(position.xyz, 1.0);
    position_viewspace.xy += particleSize * (vertices.xy - vec2(0.5));
	
	gl_Position = pP * position_viewspace;	

	particleColor = color;
}