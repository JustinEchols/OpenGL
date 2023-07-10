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
	frag_pos = vec3(ModelTransform * vertex);

	// This is now a perp vector not. Almost certainly not a normal vector.
	vertex_n = mat3(transpose(inverse(ModelTransform))) * vertex_normal;

	gl_Position = MapToPersp * MapToCamera * ModelTransform * vertex;
}
