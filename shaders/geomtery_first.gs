#version 430 core

layout (points) in;
layout (points, max_vertices = 1) out;


// This geometry shader input that is one point and output that is a line strip.
// In the main routine, we generate two new points from the input point. The two
// new points are shifted horizontally from the point point. One to the left and
// one to the right. Since the output is a line_strip the result is a line strip
// between the two generated points and that passes through the input point.

void main()
{
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	EndPrimitive();
}
