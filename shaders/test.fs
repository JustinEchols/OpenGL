#version 430 core

in vec3 color;
in vec2 tex_coordinate;

uniform sampler2D texture1;
uniform sampler2D texture2;


out vec4 frag_color;


void main()
{
	frag_color = mix(texture(texture1, tex_coordinate), texture(texture2,
	tex_coordinate), 0.2);
} 
