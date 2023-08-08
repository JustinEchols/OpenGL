#version 430 core

layout (location = 0) in vec2 v_HousePos;
layout (location = 1) in vec3 v_HouseColor;

out VS_OUT
{
	vec3 Color;
} vs_out;

void main()
{
	gl_Position = vec4(v_HousePos.x, v_HousePos.y, 0.0, 1.0);
	vs_out.Color = v_HouseColor;

}
