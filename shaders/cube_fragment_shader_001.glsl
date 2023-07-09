#version 430 core

out vec4 color;

uniform vec3 u_cube_color;
uniform vec3 u_light_color;
uniform vec3 u_light_pos;
uniform vec3 u_eye_pos;

// Normal to each vertex
in vec3 vertex_n;

// The current fragments position in world space.
in vec3 frag_pos;

void main()
{
	vec3 diff = u_light_pos - frag_pos;
	vec3 direction = normalize(diff);
	vec3 normal = normalize(vertex_n);


	vec3 frag_to_eye = normalize(u_eye_pos - frag_pos);
	vec3 reflect = reflect(-direction, normal);
	
	//vec3 reflect = (diff) - 2 * dot(normal, (diff)) * diff;
	//reflect = normalize(reflect);

	//vec3 frag_to_eye = u_eye_pos - diff;
	//frag_to_eye = normalize(frag_to_eye);

	float specular_intensity_object = 0.5f;
	float specular_strength = pow(max(dot(reflect, frag_to_eye), 0.0), 128);
	vec3 specular = specular_intensity_object * specular_strength * u_light_color;

	// If the vectors point in the opposite directions, the
	// dot product < 0 and the amount of directional diffusion
	// of the light does not make sense because the light ray
	// and surface do not come in contact.

	float diffuse_strength = max(dot(direction, normal), 0.0);
	vec3 diffuse = diffuse_strength * u_light_color;

	float ambient_strength = 0.1;
	vec3 ambient = ambient_strength * u_light_color;

	vec3 result = (ambient + diffuse + specular) * u_cube_color;
	color = vec4(result, 1.0f);
}
