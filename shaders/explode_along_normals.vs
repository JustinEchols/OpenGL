#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;
layout (location = 2) in vec2 v_TextureCoordinate;

uniform mat4 u_MapToPersp;
uniform mat4 u_MapToCamera;
uniform mat4 u_ModelTransform;

out VS_OUT
{
	vec2 v_TexCoords;
} vs_out;

void main()
{
	vs_out.v_TexCoords = v_TextureCoordinate;
	gl_Position = u_MapToPersp * u_MapToCamera * u_ModelTransform * vec4(v_Position, 1.0);
}
