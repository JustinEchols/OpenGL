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

struct light_point
{
	vec3 pos;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;


	float atten_constant;
	float atten_linear;
	float atten_quadratic;
};

struct light_directional
{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform vec3 u_camera_pos;
uniform material u_material;
uniform light_point u_light_point;

void main()
{

	vec4 texel_diffuse = texture(u_material.diffuse, tex_cood);
	vec4 texel_specular = texture(u_material.specular, tex_cood);

	vec3 ambient = vec3(texel_diffuse) * u_light_point.ambient;

	vec3 light_direction = normalize(u_light_point.pos - frag_pos);
	vec3 normal = normalize(vertex_n);

	float diffuse_strength = max(dot(light_direction, normal), 0.0);
	vec3 diffuse = diffuse_strength * vec3(texel_diffuse) * u_light_point.diffuse;

	vec3 frag_to_eye = normalize(u_camera_pos - frag_pos);
	vec3 reflect = reflect(-light_direction, normal);
	
	vec3 specular;
	if(dot(light_direction, normal) > 0.0)
	{
		float specular_strength = pow(max(dot(reflect, frag_to_eye), 0.0), u_material.shininess);
		specular = specular_strength * vec3(texel_specular) * u_light_point.specular;
	}
	else
	{
		specular = vec3(0.0, 0.0, 0.0);
	}

	float d = length(u_light_point.pos - frag_pos);

	float C = u_light_point.atten_constant;
	float L = u_light_point.atten_linear;
	float Q = u_light_point.atten_quadratic;

	float attenuation = 1.0f / (C + L * d + Q * (d * d));

	ambient *= attenuation;
	diffuse *= attenuation;
	specular *= attenuation;

	vec3 result = ambient + diffuse + specular;
	color = vec4(result, 1.0f);
}
