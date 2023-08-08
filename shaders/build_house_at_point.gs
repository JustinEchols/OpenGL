// The geometry shader generates  5 points from the input point. The output
// primitive type specified is triangle_strip, so a triagnle strip is generated
// using the 5 points we generated from the inupt point. The result ends up
// being a simple 2D house because of the positions of the points we generated
// and how triagnle strips are generatedd in OpenGL

#version 430 core

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

out vec3 HouseColor;

in VS_OUT
{
	vec3 Color;
} gs_in[];

void build_house_at_position(vec4 Pos)
{
	HouseColor = gs_in[0].Color;

	gl_Position = Pos + vec4(-0.2, -0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = Pos + vec4(0.2, -0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = Pos + vec4(-0.2, 0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = Pos + vec4(0.2, 0.2, 0.0, 0.0);
	EmitVertex();

	gl_Position = Pos + vec4(0.0, 0.4, 0.0, 0.0);
	EmitVertex();

	EndPrimitive();
}

void main()
{
	build_house_at_position(gl_in[0].gl_Position);
}
