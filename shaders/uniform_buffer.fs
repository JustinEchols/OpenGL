#version 430 core

out vec4 FragColor;

void main()
{
	FragColor = texture(u_Texel, v_TexCoord);
}
