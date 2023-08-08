#version 430 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT
{
	vec2 v_TexCoords;
} gs_in[];

out vec2 v_TexCoord;

uniform float u_time;

vec4 explode(vec4 Pos, vec3 Normal)
{
	vec4 Result;

	float magnitude = 2.0;
	float c = magnitude * (0.5 * (sin(u_time) + 1.0));
	vec3 Dir = c * Normal;

	Result = Pos + Dir;
	return(Result);
};

vec3 get_normal()
{
	vec3 Result;

	vec3 U = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
	vec3 V = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);

	Result = normalize(cross(U, V));

	return(Result);
}

void main()
{
	vec3 TriangleNormal = get_normal();

	gl_Position = explode(gl_in[0].gl_Position, TriangleNormal);
	v_TexCoord = gs_in[0].v_TexCoords;
	EmitVertex();

	gl_Position = explode(gl_in[1].gl_Position, TriangleNormal);
	v_TexCoord = gs_in[1].v_TexCoords;
	EmitVertex();

	gl_Position = explode(gl_in[2].gl_Position, TriangleNormal);
	v_TexCoord = gs_in[2].v_TexCoords;
	EmitVertex();
	EndPrimitive();
}

