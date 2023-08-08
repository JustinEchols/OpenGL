#version 430 core

out vec4 FragColor;

uniform vec3 u_CameraPos;
uniform samplerCube u_TexelSkybox;
uniform float u_refractive_index;

in vec3 v_Norm;
in vec3 v_FragPos;

void main()
{
	vec3 CameraToFrag = v_FragPos - u_CameraPos;

	vec3 Refracted = refract(CameraToFrag, normalize(v_Norm), u_refractive_index);

	FragColor = texture(u_TexelSkybox, Refracted);
}
