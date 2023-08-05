#version 430 core

layout (location = 0) in vec3 v_Position;

layout (std140) uniform Transforms
{
	mat4 u_MapToPersp;
	mat4 u_MapToCamera;
};

uniform mat4 u_ModelTransform;

void main()
{
	gl_Position = u_MapToPersp * u_MapToCamera * u_ModelTransform * vec4(v_Position, 1.0);
}

