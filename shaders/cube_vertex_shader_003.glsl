#version 430 core

layout (location = 0) in vec3 vertex_position;
layout (location = 1) in vec3 vertex_normal;
layout (location = 2) in vec2 texture_coordinates;

uniform mat4 ModelTransform;
uniform mat4 MapToCamera;
uniform mat4 MapToPersp;

out vec3 vertex_n;
out vec3 frag_pos;
out vec2 tex_cood;

void main()
{
	vec4 vertex = vec4(vertex_position, 1.0);

	// The fragment is in world space
	frag_pos = vec3(ModelTransform * vertex);

	// This is now a perp vector not. Almost certainly not a normal vector.
	vertex_n = mat3(transpose(inverse(ModelTransform))) * vertex_normal;

	gl_Position = MapToPersp * MapToCamera * ModelTransform * vertex;

	tex_cood = texture_coordinates;
}
