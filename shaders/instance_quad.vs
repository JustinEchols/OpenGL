#version 430 core

layout (location = 0) in vec2 v_PositionNDC;
layout (location = 1) in vec3 v_Color;

out vec3 Color;

uniform vec2 PositionNDCOffsets[100];

void main()
{
	vec2 OffsetNDC = PositionNDCOffsets[gl_InstanceID];
	gl_Position = vec4(v_PositionNDC + OffsetNDC, 0.0, 1.0);
	Color = v_Color;
}


