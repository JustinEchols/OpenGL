#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;
layout (location = 2) in vec2 v_TexCoordinate;

uniform mat4 ModelTransform;
uniform mat4 MapToCamera;
uniform mat4 MapToPersp;

out vec3 v_Norm;
out vec3 v_FragPos;
out vec2 v_TexCoord;

void main()
{
	v_Norm		= mat3(transpose(inverse(ModelTransform))) * v_Normal;
	v_FragPos	= vec3(ModelTransform * vec4(v_Position, 1.0f));
	v_TexCoord	= v_TexCoordinate;
	gl_Position = MapToPersp * MapToCamera * ModelTransform * vec4(v_Position, 1.0f);
}

