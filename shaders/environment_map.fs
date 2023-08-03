#version 430 core

out vec4 FragColor;

in vec3 v_Norm;
in vec3 v_FragPos;

uniform vec3 u_CameraPos;
uniform samplerCube u_SkyboxTexel;

void main()
{
	vec3 CameraToFrag = normalize(v_FragPos - u_CameraPos);
	vec3 Reflected = reflect(CameraToFrag, normalize(v_Norm));
	FragColor = texture(u_SkyboxTexel, Reflected);
}
