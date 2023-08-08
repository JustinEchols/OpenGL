#version 430 core

layout (triangles) in;

// We want to draw the normal vector at each vertex of a triangle.
// Triagnles have 3 vertices. For each vertex we need 2 points to draw a line
// segment/strip for a total of 3 * 2 = 6 vertices.

layout (line_strip, max_vertices = 6) out;

in VS_OUT
{
	vec3 v_Normal;
} gs_in[];

const float c = 0.4;

uniform mat4 u_MapToPersp;

void line_generate(int index)
{
	gl_Position = u_MapToPersp * gl_in[index].gl_Position;
	EmitVertex();

	// v0 of line segment
	vec4 VertexPos = gl_in[index].gl_Position;

	// v1 of line segment
	vec4 VertexNormal = vec4(gs_in[index].v_Normal, 0.0);

	gl_Position = u_MapToPersp * (VertexPos + c * VertexNormal);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	line_generate(0);
	line_generate(1);
	line_generate(2);
}
