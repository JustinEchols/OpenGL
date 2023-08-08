#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normals;
layout (location = 2) in vec3 v_TextureCoordinate;

out VS_OUT
{
	vec3 v_Normal;
} vs_out;

uniform mat4 u_ModelTransform;
uniform mat4 u_MapToCamera;

void main()
{
	// The geometry shader calculations are done in view space therefore the normal transform matrix
	// needs to account for this fact. Which is why the normal matrix is calculated here using the
	// product of the view and model transforms. As any vector transformed by these two is in 
	// eye/view/camera space.

	mat3 NormalTransform = mat3(transpose(inverse(u_MapToCamera * u_ModelTransform)));

	vs_out.v_Normal = normalize(vec3(vec4(NormalTransform * v_Normals, 0.0)));

	gl_Position = u_MapToCamera * u_ModelTransform * vec4(v_Position, 1.0);
}
