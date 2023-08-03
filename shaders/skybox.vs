#version 430 core

layout (location = 0) in vec3 vertex_position;

out vec3 v_TexCoord;

uniform mat4 MapToPersp;
uniform mat4 MapToCamera;

void main()
{
	v_TexCoord = vertex_position;
	//v_TexCoord = vec3(vertex_position.xy, -vertex_position.z);
	gl_Position = MapToPersp * MapToCamera * vec4(vertex_position, 1.0);
}

