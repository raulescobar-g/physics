#version 150

uniform vec3 lightPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

in vec3 normal;
in vec3 position;

out vec4 fragColor;

void main()
{
	vec3 n = normalize(normal);

	vec3 l = normalize(lightPos.xyz - position.xyz);

	vec3 h = normalize((l - (normalize(position.xyz)) ));

	vec3 diffuse = kd * max(0.0, dot(n,l));
	vec3 specular = ks * max(0.0, pow( dot(n,h),s ) );
	vec3 color = ka + (diffuse + specular);

	fragColor = vec4(color, 1.0);
}