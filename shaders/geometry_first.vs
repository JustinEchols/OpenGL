#version 430 core

layout (location = 0) in vec3 v_Position;

void main()
{
	gl_Position = vec4(v_Position.x, v_Position.y, 0.0, 1.0);
}
