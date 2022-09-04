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

	vec3 h = normalize((normalize(lightPos.xyz - position.xyz) - (normalize(position.xyz)) ));
	vec3 cs1 = ks * pow(max(0.0, dot(h,n)), s);
	vec3 cd1 = kd * max(0.0 , dot(n.xyz, normalize(lightPos.xyz - position.xyz)));

	float r = (ka.r + cd1.r + cs1.r );
	float g = (ka.g + cd1.g + cs1.g );
	float b = (ka.b + cd1.b + cs1.b );
	gl_FragColor = vec4(r,g,b, 1.0);
}

