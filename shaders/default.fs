#version 430 core

out vec4 FragColor;

uniform sampler2D u_TexelDiffuse1;

in vec2 v_TexCoord;

void main()
{
	vec4 Texel = texture(u_TexelDiffuse1, v_TexCoord);
	FragColor = Texel;
}
