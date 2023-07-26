#version 430 core

out vec4 Color;

in vec3 v_FragPos;
in vec3 v_FragNormal

uniform vec3 u_CameraPos;
uniform samplerCube SkyBox;

void main()
{
	vec3 CameraToFrag = normalize(v_FragPos - u_CameraPos);
	vec3 Reflect = reflect(CameraToFrag, normal(v_FragNormal));
	Color = vec4(texture(SkyBox, R).rgb, 1.0);
}
