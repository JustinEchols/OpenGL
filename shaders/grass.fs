#version 430 core

out vec4 FragColor;

uniform sampler2D u_GrassTexel;

in vec2 v_TexCoord;

void main()
{
	vec4 TextureColor = texture(u_GrassTexel, v_TexCoord);
	if(TextureColor.a < 0.1)
	{
		discard;
	}
	FragColor = TextureColor;

}
