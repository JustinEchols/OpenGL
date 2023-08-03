#version 430 core

layout (location = 0) in vec3 v_Position;;
layout (location = 1) in vec2 v_TexCoordinate;

out vec2 v_TexCoord;

uniform mat4 ModelTransform;
uniform mat4 MapToCamera;
uniform mat4 MapToPersp;

void main()
{
	gl_Position = MapToPersp * MapToCamera * ModelTransform * vec4(v_Position, 1.0);
	v_TexCoord = v_TexCoordinate;
}


