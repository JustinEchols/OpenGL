#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec2 v_TextureCoordinate;

out vec2 v_TexCoord;

void main()
{
	gl_Position = vec4(v_Position.x, v_Position.y, 0.0, 1.0f);
	v_TexCoord = v_TextureCoordinate;
}
