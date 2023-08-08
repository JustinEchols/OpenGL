#version 430 core

out vec4 FragColor;

in vec2 v_TexCoord;

uniform sampler2D u_Texel;

void main()
{
	FragColor = texture(u_Texel, v_TexCoord);
};
