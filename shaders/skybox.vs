#version 430 core

layout (location = 0) in vec3 vertex_position;

out vec3 v_TexCoord;

uniform mat4 MapToPersp;
uniform mat4 MapToCamera;

void main()
{
	// The vertex position is used as the texture coordinates that sample one of the textures
	v_TexCoord = vertex_position;

	gl_Position = MapToPersp * MapToCamera * vec4(vertex_position, 1.0);
}

