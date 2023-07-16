#version 430 core

out vec4 Color;

in vec3 vertex_n;
in vec3 frag_pos;
in vec2 tex_cood;

struct material
{
	sampler2D Diffuse;
	sampler2D Specular;

	float shininess;
};

struct light_spot
{
	vec3 Pos;
	vec3 Direction;
	float cos_of_inner_angle;
	float cos_of_outer_angle;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	float atten_constant;
	float atten_linear;
	float atten_quadratic;
};

struct light_point
{
	vec3 Pos;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;

	float atten_constant;
	float atten_linear;
	float atten_quadratic;
};


struct light_directional
{
	vec3 Direction;

	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

uniform vec3				u_CameraPos;
uniform material			u_Material;
uniform light_point			u_LightPoint;
//uniform light_directional	u_LightDirectional;
uniform light_spot			u_LightSpot;

vec3 light_directional_calc(light_directional LightDir, vec3 Normal, vec3 FragToCameraDir, vec3 TexelDiffuse, vec3 TexelSpecular)
{
	vec3 NormalLightDir = normalize(-LightDir.Direction);
	float diffuse = max(dot(NormalLightDir, Normal), 0.0);
	vec3 Reflect = reflect(-NormalLightDir, Normal);


	vec3 Ambient = LightDir.Ambient * TexelDiffuse;
	vec3 Diffuse = diffuse * LightDir.Diffuse * TexelDiffuse;

	vec3 Specular;
	if(dot(NormalLightDir, Normal) > 0.0)
	{
		float spec = pow(max(dot(FragToCameraDir, Reflect), 0.0), u_Material.shininess);
		vec3 Specular = spec * LightDir.Specular * TexelSpecular;
	}
	else
	{
		Specular = vec3(0.0, 0.0, 0.0);
	}
	return(Ambient + Diffuse + Specular);
}

vec3 light_point_calc(light_point LightPoint, vec3 Normal, vec3 FragToCameraDir, vec3 TexelDiffuse, vec3 TexelSpecular)
{
	vec3 FragToLightDir = normalize(LightPoint.Pos - frag_pos);
	float diffuse = max(dot(FragToLightDir, Normal), 0.0);
	vec3 Reflect = reflect(-FragToLightDir, Normal);

	vec3 Ambient = LightPoint.Ambient * TexelDiffuse;
	vec3 Diffuse = diffuse * LightPoint.Diffuse * TexelDiffuse;

	vec3 Specular;
	if(dot(FragToLightDir, Normal) > 0.0)
	{
		float spec = pow(max(dot(FragToCameraDir, Reflect), 0.0), u_Material.shininess);
		Specular = spec * LightPoint.Specular * TexelSpecular;
	}
	else
	{
		Specular = vec3(0.0, 0.0, 0.0);
	}

	return(Ambient + Diffuse + Specular);
}

void main()
{
	vec3 Result;

	vec3 TexelDiffuse = vec3(texture(u_Material.Diffuse, tex_cood));
	vec3 TexelSpecular = vec3(texture(u_Material.Specular, tex_cood));

	vec3 Normal = normalize(vertex_n);
	vec3 FragToCameraDir = normalize(u_CameraPos - frag_pos);

	Result = light_point_calc(u_LightPoint, Normal, FragToCameraDir, TexelDiffuse, TexelSpecular);

	Color = vec4(Result, 1.0f);
}
