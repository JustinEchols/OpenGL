#version 430 core

in VS_OUT
{
	vec2 v_TexCoords;
} fs_in;

uniform sampler2D u_Texel;

out vec4 FragColor;

void main()
{
	FragColor = texture(u_Texel, fs_in.v_TexCoords);
}
