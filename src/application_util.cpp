
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
#define GL_CHECK_ERROR() gl_error_check(__FILE__, __LINE__)
