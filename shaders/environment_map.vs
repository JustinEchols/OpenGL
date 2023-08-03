#version 430 core

layout (location = 0) in vec3 v_Position;
layout (location = 1) in vec3 v_Normal;

uniform mat4 ModelTransform;
uniform mat4 MapToPersp;
uniform mat4 MapToCamera;

out vec3 v_Norm;
out vec3 v_FragPos;

void main()
{
	v_Norm = mat3(transpose(inverse(ModelTransform))) * v_Normal;
	v_FragPos = vec3(ModelTransform * vec4(v_Position, 1.0));
	gl_Position = MapToPersp * MapToCamera * ModelTransform * vec4(v_Position, 1.0);
}
