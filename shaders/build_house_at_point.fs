#version 430 core

out vec4 FragColor;

in vec3 HouseColor;

void main()
{
	FragColor = vec4(HouseColor, 1.0);
}
