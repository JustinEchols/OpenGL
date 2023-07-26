#version 430 core

// Positions and normals from each mesh
layout (location 0) in vec3 v_Position;
layout (location 1) in vec3 v_Normal;

uniform mat4 ModelTransform;
uniform mat4 MapToPersp;
uniform mat4 MapToCamera;

out vec3 v_CameraPos;
out vec3 v_Ca

void main()
{
	// FragPos is in world space becuase we need to calculate the CameraToFrag incident
	// vector. This is done by subtracting the camera position from the fragment position. The camera
	// position is being passed to the fragment shader as a uniform so it is in world space. So the
	// frag pos vector also needs to be in world space which is why we only multiplied by the model transform
	
	v_FragPos = vec3(ModelTransform * vec4(v_Position, 1.0));
	v_FragNormal = mat3(transpose(inverse(ModelTransform))) * v_Normal;
	gl_Position = MapToPersp * MapToCamera * ModelTransform * vec4(v_Position, 1.0);
}
