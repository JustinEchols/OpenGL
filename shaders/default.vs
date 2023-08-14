#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 2) in vec2 v_TextureCoord;

out vec2 v_TexCoord;

uniform mat4 u_MapToPersp;
uniform mat4 u_MapToCamera;
uniform mat4 u_ModelTransform;

void main()
{
	v_TexCoord = v_TextureCoord;
	gl_Position = u_MapToPersp * u_MapToCamera * u_ModelTransform * vec4(v_Position, 1.0);
}

