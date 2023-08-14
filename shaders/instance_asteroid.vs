#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 2) in vec2 v_TextureCoord;

// The ModelTransform matrix is declared as a vertex attribute because we
// would like to store an instanced array of these transformation matrices. 
// The maximum size of a vertex attribute is the size of a vec4. A mat4 is
// just 4 vec4s we will reserve 4 vertex attributes for a matrix of dim 4x4.Persp
// The location is 3 so the columns of the matrix will be at locations 3, 4, 5,
// and 6.

layout (location = 3) in mat4 m_InstanceModelTransform;

out vec2 v_TexCoord;

uniform mat4 u_MapToPersp;
uniform mat4 u_MapToCamera;

void main()
{
	gl_Position = u_MapToPersp * u_MapToCamera * m_InstanceModelTransform * vec4(v_Position, 1.0);
	v_TexCoord = v_TextureCoord;
}


