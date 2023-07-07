#version 430 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec2 texture_coordinate;

out vec2 tex_coordinate;

uniform mat4 ModelTransform;
uniform mat4 MapToCamera;
uniform mat4 MapToPersp;

void main()
{
	gl_Position = MapToPersp * MapToCamera * ModelTransform * vec4(vertex_position, 1.0);
	tex_coordinate = texture_coordinate;
}
