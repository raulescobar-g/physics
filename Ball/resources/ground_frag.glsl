#version 120

uniform sampler2D texture;

varying vec2 vTex;

void main()
{
	vec4 color = texture2D(texture, vTex);
	gl_FragColor =  color;
}