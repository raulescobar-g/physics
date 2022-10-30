#version 430

uniform vec3 lightPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;
uniform float a;

in vec3 position;
in vec3 normal;

out vec4 fragColor;

void main()
{
	vec3 n = normalize(normal);

	vec3 h1 = normalize((normalize(lightPos.xyz-position.xyz) - (normalize(position.xyz)) ));
	vec3 cs1 = ks * pow(max(0.0, dot(h1,n)), s);
	vec3 cd1 = kd * max(0.0 , dot(n.xyz, normalize(lightPos.xyz-position.xyz)));

	float r = ka.r + cd1.r + cs1.r ;
	float g = ka.g + cd1.g + cs1.g ;
	float b = ka.b + cd1.b + cs1.b ;

	fragColor = vec4(r, g, b, a);
}