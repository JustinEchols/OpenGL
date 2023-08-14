#version 430 core

out vec4 FragColor;

in vec2 v_TexCoord;

uniform sampler2D u_TexelDiffuse1;

void main()
{
	FragColor = texture(u_TexelDiffuse1, v_TexCoord);
}
