#version 430 core

out vec4 color;

in vec3 vertex_n;
in vec3 frag_pos;
in vec2 tex_cood;

struct material
{
	// The ambient color is the same as the diffuse color so we can comment the
	// vec3 out.
	//vec3 ambient;
	sampler2D diffuse;
	sampler2D specular;

	float shininess;
};

struct light
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 pos;
};


uniform vec3 u_camera_pos;
uniform material u_material;
uniform light u_light;

void main()
{

	vec4 texel_diffuse = texture(u_material.diffuse, tex_cood);
	vec4 texel_specular = texture(u_material.specular, tex_cood);

	vec3 ambient = vec3(texel_diffuse) * u_light.ambient;

	vec3 light_direction = normalize(u_light.pos - frag_pos);
	vec3 normal = normalize(vertex_n);

	float diffuse_strength = max(dot(light_direction, normal), 0.0);
	vec3 diffuse = diffuse_strength * vec3(texel_diffuse) * u_light.diffuse;

	vec3 frag_to_eye = normalize(u_camera_pos - frag_pos);
	vec3 reflect = reflect(-light_direction, normal);
	
	vec3 specular;
	if(dot(light_direction, normal) > 0.0)
	{
		float specular_strength = pow(max(dot(reflect, frag_to_eye), 0.0), u_material.shininess);
		specular = specular_strength * vec3(texel_specular) * u_light.specular;
	}
	else
	{
		specular = vec3(0.0, 0.0, 0.0);
	}

	vec3 result = ambient + diffuse + specular;
	color = vec4(result, 1.0f);
}
