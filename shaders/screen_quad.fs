#version 430 core

in vec2 v_TexCoord;
uniform sampler2D u_ScreenTexel;

out vec4 FragColor;

void main()
{
	FragColor = texture(u_ScreenTexel, v_TexCoord);
}
