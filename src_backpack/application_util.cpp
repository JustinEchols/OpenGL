

internal void
gl_clear_errors()
{
	while (glGetError() != GL_NO_ERROR);
}

internal b32 
gl_log_restart()
{
	FILE *FileHandle = fopen(GL_LOG_FILE, "w");
	if(!FileHandle)
	{
		fprintf(stderr, "ERROR: Could not open GL_LOG_FILE %s for writing\n", GL_LOG_FILE);
		return(false);
	}
	time_t now = time(NULL);
	char *date = ctime(&now);
	fprintf(FileHandle, "GL_LOG_FILE log. local time %s\n", date);
	fclose(FileHandle);
	return(true);
}

internal b32 
gl_log_message(char *message, ...)
{
	va_list arg_ptr;
	FILE *FileHandle = fopen(GL_LOG_FILE, "a");
	if(!FileHandle)
	{
		fprintf(stderr, "ERROR: Could not open GL_LOG_FILE %s for appending\n", GL_LOG_FILE);
		return(false);
	}
	va_start(arg_ptr, message);
	vfprintf(FileHandle, message, arg_ptr);
	va_end(arg_ptr);
	fclose(FileHandle);
	return(true);
}

internal b32
gl_log_error(char  *message, ...)
{
	va_list arg_ptr;
	FILE *FileHandle = fopen(GL_LOG_FILE, "a");
	if(!FileHandle)
	{
		fprintf(stderr, "ERROR: Could not open GL_LOG_FILE %s for appending\n", GL_LOG_FILE);
		return(false);
	}
	va_start(arg_ptr, message);
	vfprintf(FileHandle, message, arg_ptr);
	va_end(arg_ptr);
	va_start(arg_ptr, message);
	vfprintf(stderr, message, arg_ptr);
	va_end(arg_ptr);
	fclose(FileHandle);
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
	char *names[] =
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
	for(int i = 0; i < 10; i++)
	{
		s32 integer_value = 0;
		glGetIntegerv(params[i], &integer_value);
		gl_log_message("%s %i\n", names[i], integer_value);
	}

	s32 integer_values[2];
	integer_values[0] = integer_values[1] = 0;
	glGetIntegerv(params[10], integer_values);
	gl_log_message("%s %i %i\n", names[10], integer_values[0], integer_values[1]);

	u8 bool_value = 0;
	glGetBooleanv(params[11], &bool_value);
	gl_log_message("%s %u\n", names[11], (u32)bool_value);
	gl_log_message("--------------------------\n");
}

internal char *
gl_enum_type_to_string(GLenum type)
{
	switch (type)
	{
		case GL_BOOL:
			{
				return "bool";
			} break;
		case GL_INT:
			{
				return "int";
			} break;
		case GL_FLOAT:
			{
				return "float";
			} break;
		case GL_FLOAT_VEC2:
			{
				return "vec2";
			} break;
		case GL_FLOAT_VEC3:
			{
				return "vec3";
			} break;
		case GL_FLOAT_VEC4:
			{
				return "vec4";
			} break;
		case GL_FLOAT_MAT2:
			{
				return "mat2";
			} break;
		case GL_FLOAT_MAT3:
			{
				return "mat3";
			} break;
		case GL_FLOAT_MAT4:
			{
				return "mat4";
			} break;
		case GL_SAMPLER_2D:
			{
				return "sampler_2d";
			} break;
		case GL_SAMPLER_3D:
			{
				return "sampler_3d";
			} break;
		case GL_SAMPLER_CUBE:
			{
				return "sampler_cube";
			} break;
		case GL_SAMPLER_2D_SHADOW:
			{
				return "sampler_2d_shadow";
			} break;
		case GL_VERTEX_SHADER:
			{
				return "Vertex";
			} break;
		case GL_FRAGMENT_SHADER:
			{
				return "Fragment";
			} break;
		case GL_GEOMETRY_SHADER:
			{
				return "Geometry";
			} break;
		default:
			{
			}
	}
	return "other";
}

internal b32
GLLogCall(char *gl_function, char  *file, s32 line_number)
{
	b32 Result = false;
	while(GLenum gl_error = glGetError())
	{
		char  *error = gl_enum_type_to_string(gl_error);
		printf("[OpenGL Error] (%s) %s %s %d", error, gl_function, file, line_number);
		return(Result);
	}
	Result = true;
	return(Result);
}

internal void
glfw_error_callback(s32 error, const char  *desc)
{
	gl_log_error("GLFW ERROR: Code: %d MSG: %s\n", error, desc);
}

internal GLenum
gl_error_check(char *filename, s32 line_number)
{
	GLenum error_code;
	// NOTE(Justin): Why would you use a while loop to do the error?

	if ((error_code = glGetError()) != GL_NO_ERROR)
	{
		switch (error_code)
		{
			case GL_INVALID_ENUM:
			{
				char *error = "INVALID_ENUM";
				printf("%s %s %d", error, filename, line_number);
			}
			break;
			case GL_INVALID_VALUE:
			{
				char *error = "INVALID_VALUE";
				printf("%s %s %d", error, filename, line_number);
			}
			break;
			case GL_INVALID_OPERATION:
			{
				char *error = "INVALID_OPERATION";
				printf("%s %s %d", error, filename, line_number);
			}
			break;
			case GL_STACK_OVERFLOW:
			{
				char *error = "STACK_OVERFLOW";
				printf("%s %s %d", error, filename, line_number);
			}
			break;
			case GL_STACK_UNDERFLOW:
			{
				char *error = "STACK_UNDERFLOW";
				printf("%s %s %d", error, filename, line_number);
			}
			break;
			case GL_OUT_OF_MEMORY:
			{
				char *error = "OUT_OF_MEMORY";
				printf("%s %s %d", error, filename, line_number);
			}
			break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
			{
				char *error = "INVALID_FRAMEBUFFER_OPERATION";
				printf("%s %s %d", error, filename, line_number);
			}
			break;
		}
	}
	return(error_code);
}

internal void
gl_log_shader_info(shader_program_t *Shader)
{
	printf("----------------------\n Shader program %d info:\n", Shader->id);
	s32 params = -1;

	glGetProgramiv(Shader->id, GL_LINK_STATUS, &params);
	printf("GL_LINK_STATUS = %d\n", params);

	glGetProgramiv(Shader->id, GL_ATTACHED_SHADERS, &params);
	printf("GL_ATTACHED_SHADERS = %d\n", params);

	glGetProgramiv(Shader->id, GL_ACTIVE_ATTRIBUTES, &params);
	printf("GL_ACTIVE_ATTRIBUTES = %d\n", params);

	// TODO(Justin): Store active attributes info?
	for(s32 attr_index = 0; attr_index < params; attr_index++)
	{
		char name[64];
		s32 length_max = 64;
		s32 length_actual = 0;
		s32 size = 0;
		GLenum param_type;
		glGetActiveAttrib(Shader->id, attr_index, length_max, &length_actual, &size, &param_type, name);
		if(size > 1)
		{
			for(s32 j = 0; j < size; j++)
			{
				char name_long[64];
				sprintf(name_long, "%s[%d]", name, j);
				s32 location = glGetAttribLocation(Shader->id, name_long);
				printf(" %d) TYPE:%s NAME: %s LOCATION:%d\n\n", attr_index, gl_enum_type_to_string	(param_type), name_long, location);
			}
		}
		else 
		{
			s32 location = glGetAttribLocation(Shader->id, name);
			printf(" %d) TYPE:%s NAME: %s LOCATION:%d\n\n", attr_index, gl_enum_type_to_string(param_type), name, location);
		}
	}

	// Active Uniforms
	glGetProgramiv(Shader->id, GL_ACTIVE_UNIFORMS, &params);
	printf("GL_ACTIVE_UNIFORMS = %d\n", params);

	Shader->uniforms_count = params;
	Shader->Uniforms = (uniform_t *)calloc((size_t)Shader->uniforms_count, sizeof(uniform_t));

	for(s32 uniform_index = 0; uniform_index < params; uniform_index++)
	{
		uniform_t *Uniform = Shader->Uniforms + uniform_index;

		s32 length_max = 64;
		s32 length_actual = 0;
		s32 size = 0;

		glGetActiveUniform(Shader->id, uniform_index, length_max, &length_actual, &Uniform->size, &Uniform->type, Uniform->name);
	
		b32 is_array_uniform = (Uniform->size > 1);
		if(is_array_uniform)
		{
			// TODO(Justin): Need to store multiple data here
			for(s32 uniform_array_index = 0; uniform_array_index < size; uniform_array_index++)
			{
				char name_long[64];
				sprintf(name_long, "%s[%d]", Uniform->name, uniform_array_index);
				Uniform->location = glGetUniformLocation(Shader->id, name_long);

				char *gl_type = gl_enum_type_to_string(Uniform->type);
				printf(" %d) TYPE:%s NAME:%s LOCATION:%d\n\n", uniform_index, gl_type, name_long, Uniform->location);
			}
		}
		else
		{
			Uniform->location = glGetUniformLocation(Shader->id, Uniform->name);
			printf(" %d) TYPE:%s NAME: %s LOCATION:%d\n\n", uniform_index, gl_enum_type_to_string(Uniform->type), Uniform->name, Uniform->location);
		}
	}
}

internal void
uniform_set_b32(GLuint shader_program_id, char *uniform_name, b32 value)
{
	// TODO(Justin): If these function calls!!!

	u32 uniform_location = glGetUniformLocation(shader_program_id, uniform_name);
	glUniform1i(uniform_location, (int)value);
}

internal void
uniform_set_s32(GLuint shader_program_id, char *uniform_name, s32 value)
{
	u32 uniform_location = glGetUniformLocation(shader_program_id, uniform_name);
	glUniform1i(uniform_location, value);
}

internal void
uniform_set_f32(GLuint shader_program_id, char *uniform_name, f32 value)
{
	u32 uniform_location = glGetUniformLocation(shader_program_id, uniform_name);
	glUniform1f(uniform_location, value);
}

internal void
uniform_set_mat4f(GLuint shader_program_id, char *uniform_name, glm::mat4 Transform)
{
	u32 uniform_location = glGetUniformLocation(shader_program_id, uniform_name);
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, glm::value_ptr(Transform));
}

internal void
uniform_set_3f32(GLuint shader_program_id, char *uniform_name, f32 x, f32 y, f32 z)
{
	glUniform3f(glGetUniformLocation(shader_program_id, uniform_name), x, y, z);
}

internal void
uniform_set_vec3f(GLuint shader_program_id, char *uniform_name, glm::vec3 V)
{
	uniform_set_3f32(shader_program_id, uniform_name, V.x, V.y, V.z);
}

internal void
uniform_set_2f32(GLuint shader_program_id, char *uniform_name, f32 x, f32 y)
{
	glUniform2f(glGetUniformLocation(shader_program_id, uniform_name), x, y);
}
internal void
uniform_set_vec2f(GLuint shader_program_id, char *uniform_name, glm::vec2 V)
{
	uniform_set_2f32(shader_program_id, uniform_name, V.x, V.y);
}

#define GL_CHECK_ERROR() gl_error_check(__FILE__, __LINE__)
