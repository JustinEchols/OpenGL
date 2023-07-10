#version 430 core

out vec4 color;

uniform vec3 u_cube_color;
uniform vec3 u_light_color;
uniform vec3 u_light_pos;
uniform vec3 u_camera_pos;

// Normal to each vertex
in vec3 vertex_n;

// The current fragments position in world space.
in vec3 frag_pos;

void main()
{

	float ambient_strength = 0.1f;
	vec3 ambient = ambient_strength * u_light_color;

	vec3 light_direction = normalize(u_light_pos - frag_pos);
	vec3 normal = normalize(vertex_n);

	float diffuse_strength = max(dot(light_direction, normal), 0.0);
	vec3 diffuse = diffuse_strength * u_light_color;

	vec3 frag_to_eye = normalize(u_camera_pos - frag_pos);
	vec3 reflect = reflect(-light_direction, normal);
	
	vec3 specular;
	if(dot(light_direction, normal) > 0.0)
	{
		float specular_intensity_object = 0.5f;
		float specular_strength = pow(max(dot(reflect, frag_to_eye), 0.0), 32);
		specular = specular_intensity_object * specular_strength * u_light_color;
	}
	else
	{
		specular = vec3(0.0, 0.0, 0.0);
	}

	vec3 result = (ambient + diffuse + specular) * u_cube_color;
	color = vec4(result, 1.0f);
}
