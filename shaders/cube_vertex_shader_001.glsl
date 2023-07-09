#version 430 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;

uniform mat4 ModelTransform;
uniform mat4 MapToCamera;
uniform mat4 MapToPersp;

// Normal to each vertex
out vec3 vertex_n;
out vec3 frag_pos;



void main()
{
	vec4 vertex = vec4(vertex_position, 1.0);
	gl_Position = MapToPersp * MapToCamera * ModelTransform * vertex;
	frag_pos = vec3(ModelTransform * vertex);
	vertex_n = mat3(inverse(transpose(ModelTransform))) * vertex_normal;
}
