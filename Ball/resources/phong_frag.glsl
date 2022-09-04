#version 120

uniform vec3 lightPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 normal;
varying vec3 position;



void main()
{
	vec3 n = normalize(normal);

	vec3 l = normalize(lightPos.xyz - position.xyz);

	vec3 h = normalize((l - (normalize(position.xyz)) ));

	vec3 diffuse = kd * max(0.0, dot(n,l));
	vec3 specular = ks * max(0.0, pow( dot(n,h),s ) );
	vec3 color = ka + (diffuse + specular);

	gl_FragColor = vec4(color, 1.0);
}