#version 430 core

out vec4 Color;

in vec3 v_TexCoord;

uniform samplerCube SkyBox;

void main()
{
	Color = texture(SkyBox, v_TexCoord);
}
