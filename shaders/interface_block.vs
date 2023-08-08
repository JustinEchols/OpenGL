#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 2) in vec2 v_TexCoords;

uniform mat4 u_ModelTransform;
uniform mat4 u_MapToCamera;
uniform mat4 u_MapToPersp;


out VS_OUT
{
	vec2 v_TexCoords;
} vs_out;

void main()
{
	gl_Position = u_MapToPersp * u_MapToCamera * u_ModelTransform * vec4(v_Position, 1.0);
	vs_out.v_TexCoords = v_TexCoords;
}
