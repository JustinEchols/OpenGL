internal b32
gl_log_restart()
{
	FILE *pfile = fopen(GL_LOG_FILE, "w");
	if(!pfile)
	{
		fprintf(stderr, "ERROR: Could not open GL_LOG_FILE %s for writing\n", GL_LOG_FILE);
		return(false);
	}
	time_t now = time(NULL);
	char *date = ctime(&now);
	fprintf(pfile, "GL_LOG_FILE log. local time %s\n", date);
	fclose(pfile);
	return(true);
}

internal b32 
gl_log_message(const char *message, ...)
{
	va_list arg_ptr;
	FILE* pfile = fopen(GL_LOG_FILE, "a");
	if(!pfile)
	{
		fprintf(stderr, "ERROR: Could not open GL_LOG_FILE %s for appending\n", GL_LOG_FILE);
		return(false);
	}
	va_start(arg_ptr, message);
	vfprintf(pfile, message, arg_ptr);
	va_end(arg_ptr);
	fclose(pfile);
	return(true);
}

internal b32
gl_log_error(const char* message, ...)
{
	va_list arg_ptr;
	FILE* pfile = fopen(GL_LOG_FILE, "a");
	if(!pfile)
	{
		fprintf(stderr, "ERROR: Could not open GL_LOG_FILE %s for appending\n", GL_LOG_FILE);
		return(false);
	}
	va_start(arg_ptr, message);
	vfprintf(pfile, message, arg_ptr);
	va_end(arg_ptr);
	va_start(arg_ptr, message);
	vfprintf(stderr, message, arg_ptr);
	va_end(arg_ptr);
	fclose(pfile);
	return(true);
}

internal void
gl_log_params()
{
	GLenum params[] =
	{
		GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		GL_MAX_CUBE_MAP_TEXTURE_SIZE,
		GL_MAX_DRAW_BUFFERS,
		GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, 
		GL_MAX_TEXTURE_IMAGE_UNITS,
		GL_MAX_TEXTURE_SIZE,
		GL_MAX_VARYING_FLOATS,
		GL_MAX_VERTEX_ATTRIBS,
		GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
		GL_MAX_VERTEX_UNIFORM_COMPONENTS,
		GL_MAX_VIEWPORT_DIMS,
		GL_STEREO
	};
	const char* names[] =
	{
	  "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
	  "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
	  "GL_MAX_DRAW_BUFFERS",
	  "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
	  "GL_MAX_TEXTURE_IMAGE_UNITS",
	  "GL_MAX_TEXTURE_SIZE",
	  "GL_MAX_VARYING_FLOATS",
	  "GL_MAX_VERTEX_ATTRIBS",
	  "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
	  "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
	  "GL_MAX_VIEWPORT_DIMS",
	  "GL_STEREO"
	};
	gl_log_message("GL Context Params\n");
	char msg[256];
	for(int i = 0; i < 10; i++)
	{
		int v = 0;
		glGetIntegerv(params[i], &v);
		gl_log_message("%s %i\n", names[i], v);
	}
	int v[2];
	v[0] = v[1] = 0;
	glGetIntegerv(params[10], v);
	gl_log_message("%s %i %i\n", names[10], v[0], v[1]);
	unsigned char s = 0;
	glGetBooleanv(params[11], &s);
	gl_log_message("%s %u\n", names[11], (u32)s);
	gl_log_message("--------------------------\n");
}

internal const char*
gl_type_to_string(GLenum type)
{
	switch (type)
	{
		case GL_BOOL: return "bool";
		case GL_INT: return "int";
		case GL_FLOAT: return "float";
		case GL_FLOAT_VEC2: return "vec2";
		case GL_FLOAT_VEC3: return "vec3";
		case GL_FLOAT_VEC4: return "vec4";
		case GL_FLOAT_MAT2: return "mat2";
		case GL_FLOAT_MAT3: return "mat3";
		case GL_FLOAT_MAT4: return "mat4";
		case GL_SAMPLER_2D: return "sampler_2d";
		case GL_SAMPLER_3D: return "sampler_3d";
		case GL_SAMPLER_CUBE: return "sampler_cube";
		case GL_SAMPLER_2D_SHADOW: return "sampler_2d_shadow";
		default: break;
	}
	return "other";
}

internal void
gl_log_shader_info(GLuint shader_program)
{
	printf("----------------------\n Shader program %d info:\n", shader_program);
	s32 params = -1;

	glGetProgramiv(shader_program, GL_LINK_STATUS, &params);
	printf("GL_LINK_STATUS = %d\n", params);

	glGetProgramiv(shader_program, GL_ATTACHED_SHADERS, &params);
	printf("GL_ATTACHED_SHADERS = %d\n", params);

	glGetProgramiv(shader_program, GL_ACTIVE_ATTRIBUTES, &params);
	printf("GL_ACTIVE_ATTRIBUTES = %d\n", params);

	// Active Attributes
	for(s32 attr_index = 0; attr_index < params; attr_index++)
	{
		char name[64];
		s32 length_max = 64;
		s32 length_actual = 0;
		s32 size = 0;
		GLenum param_type;
		glGetActiveAttrib(
			shader_program,
			attr_index,
			length_max,
			&length_actual,
			&size,
			&param_type,
			name
		);
		if(size > 1)
		{
			for(s32 j = 0; j < size; j++)
			{
				char name_long[64];
				sprintf(name_long, "%s[%d]", name, j);
				s32 location = glGetAttribLocation(shader_program, name_long);
				printf(" %d) type:%s name:%s location:%d\n\n", attr_index, gl_type_to_string(param_type), name_long, location);
			}
		}
		else 
		{
			s32 location = glGetAttribLocation(shader_program, name);
			printf(" %d) type:%s name: %s location:%d\n\n", attr_index, gl_type_to_string(param_type), name, location);
		}
	}

	// Active Uniforms
	glGetProgramiv(shader_program, GL_ACTIVE_UNIFORMS, &params);
	printf("GL_ACTIVE_UNIFORMS = %d\n", params);
	for(s32 uniform_index = 0; uniform_index < params; uniform_index++)
	{
		char name[64];
		s32 length_max = 64;
		s32 length_actual = 0;
		s32 size = 0;
		GLenum param_type;
		glGetActiveUniform(
			shader_program,
			uniform_index,
			length_max,
			&length_actual,
			&size,
			&param_type,
			name
		);
		if(size > 1)
		{
			for(s32 j = 0; j < size; j++)
			{
				char name_long[64];
				sprintf(name_long, "%s[%d]", name, j);
				s32 location = glGetUniformLocation(shader_program, name_long);
				printf(" %d) type:%s name:%s location:%d\n\n", uniform_index, gl_type_to_string(param_type), name_long, location);
			}
		}
		else
		{
			s32 location = glGetUniformLocation(shader_program, name);
			printf(" %d) type:%s name: %s location:%d\n\n", uniform_index, gl_type_to_string(param_type), name, location);
		}
	}
}
