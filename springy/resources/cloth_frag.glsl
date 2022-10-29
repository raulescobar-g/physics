#version 460

uniform sampler2D cloth_texture;
in vec2 vTex;

out vec4 fragColor;

void main(){
	fragColor = texture(cloth_texture, vTex);
}