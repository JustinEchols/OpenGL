#version 430 core

out vec4 FragColor;

in vec2 v_TexCoord;

uniform sampler2D u_AsteroidTexel;

void main()
{
	FragColor = texture(u_AsteroidTexel, v_TexCoord);
}
