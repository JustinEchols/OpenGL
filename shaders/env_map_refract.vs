#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;

uniform mat4 u_ModelTransform;
uniform mat4 u_MapToCamera;
uniform mat4 u_MapToPersp;

out vec3 v_Norm;
out vec3 v_FragPos;

void main()
{
	v_Norm = mat3(transpose(inverse(u_ModelTransform))) * v_Normal;
	v_FragPos = vec3(u_ModelTransform * vec4(v_Position, 1.0));
	gl_Position = u_MapToPersp * u_MapToCamera * u_ModelTransform * vec4(v_Position, 1.0);
}

