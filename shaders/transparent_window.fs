#version 430 core

in vec2 v_TexCoord;
uniform sampler2D u_WindowTransparentTexel;

out vec4 FragColor;

void main()
{
	vec4 FragColor = texture(u_WindowTransparentTexel, v_TexCoord);

}
