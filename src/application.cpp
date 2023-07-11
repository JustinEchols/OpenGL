/*
TODO:
 -Vertex Buffers
	Lock Modify Unlock
	Set data
	streaming
	.
	.
	. 

 -Picking Cache for
	Selecting object
		Mouse point to object
		Click object
		Send ray to see if it interseects
	
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include "application.h"
#include "application_log.cpp"
#include "application_cube.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

global_variable input_t AppInput;

#define GLCall(gl_func) gl_clear_errors();\
	gl_func;\
	ASSERT(GLLogCall(#gl_func, __FILE__, __LINE__))

internal vertex_array_t
vertex_array_create()
{
	vertex_array_t Result = {};

	glGenVertexArrays(1, &Result.id);

	return(Result);
}

// TODO(Justin): Should we pass in an index as opposed to storing it in the vertex_array_t struct?
internal void
vertex_array_add_buffer_layout(u32 index, vertex_array_t *VertexArray, vertex_buffer_layout_t *VertexBufferLayout)
{
	glVertexAttribPointer(index, VertexBufferLayout->element_count_per_attribute,
			VertexBufferLayout->attribute_type, VertexBufferLayout->normalized,
			VertexBufferLayout->size_for_each_vertex, VertexBufferLayout->attribute_stride);

	glEnableVertexAttribArray(index);
	//glEnableVertexAttribArray(VertexArray->attribute_index);
	//VertexArray->attribute_index++;
}


// NOTE(Justin): Upon creation we will bind the buffer to initiate the copying
// process then unbind it. Is this bind then unbind completley necessary? Imagine
// initializing many different buffers with different data then yes it would be
// necessary

internal vertex_buffer_t
vertex_buffer_create(void *memory, u32 size)
{
	vertex_buffer_t Result = {};

	glGenBuffers(1, &Result.id);
	glBindBuffer(GL_ARRAY_BUFFER, Result.id);
	glBufferData(GL_ARRAY_BUFFER, size, memory, GL_STATIC_DRAW);

	Result.memory = memory;
	Result.size = size;

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	return(Result);
}

internal void
vertex_buffer_bind(vertex_buffer_t *VertexBuffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer->id);
	VertexBuffer->binded = true;
}

internal void
vertex_buffer_unbind(vertex_buffer_t *VertexBuffer)
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	VertexBuffer->binded = false;
}

internal void
vertex_buffer_destroy(vertex_buffer_t *VertexBuffer)
{
	if(VertexBuffer)
	{
		glDeleteBuffers(1, &VertexBuffer->id);
	}
}


internal void
gl_clear_errors()
{
	while (glGetError() != GL_NO_ERROR);
}

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
GLLogCall(const char *gl_function, const char* file, s32 line_number)
{
	b32 Result = false;
	while(GLenum gl_error = glGetError())
	{
		const char* error = gl_enum_type_to_string(gl_error);
		printf("[OpenGL Error] (%s) %s %s %d", error, gl_function, file, line_number);
		return(Result);
	}
	Result = true;
	return(Result);
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
				printf(" %d) TYPE:%s NAME: %s LOCATION:%d\n\n", attr_index, gl_enum_type_to_string	(param_type), name_long, location);
			}
		}
		else 
		{
			s32 location = glGetAttribLocation(shader_program, name);
			printf(" %d) TYPE:%s NAME: %s LOCATION:%d\n\n", attr_index, gl_enum_type_to_string(param_type), name, location);
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

				const char* gl_type = gl_enum_type_to_string(param_type);
				printf(" %d) TYPE:%s NAME:%s LOCATION:%d\n\n", uniform_index, gl_type, name_long, location);
			}
		}
		else
		{
			s32 location = glGetUniformLocation(shader_program, name);
			printf(" %d) TYPE:%s NAME: %s LOCATION:%d\n\n", uniform_index, gl_enum_type_to_string(param_type), name, location);
		}
	}
}



internal void
glfw_error_callback(s32 error, const char* desc)
{
	gl_log_error("GLFW ERROR: Code: %d MSG: %s\n", error, desc);
}

internal void
camera_direction_set(camera_t *Camera, f32 yaw, f32 pitch)
{
	Camera->Direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	Camera->Direction.y = sin(glm::radians(pitch));
	Camera->Direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	Camera->Direction = glm::normalize(Camera->Direction);
}

internal void
glfw_mouse_callback(GLFWwindow *Window, f64 xpos, f64 ypos)
{

	mouse_t *Mouse = &AppInput.Mouse;

	glm::vec2 Delta;
	if(!Mouse->is_initialized)
	{
		Delta.x = Delta.y = 0.0f;
		Mouse->is_initialized = true;
	}
	else
	{
		Delta.x = xpos - Mouse->Pos.x;
		Delta.y = Mouse->Pos.y - ypos;
	}

	Mouse->Pos.x = xpos;
	Mouse->Pos.y = ypos;

	Delta *= Mouse->sensitivity;

	AppInput.yaw += Delta.x;
	AppInput.pitch += Delta.y;

	if(AppInput.pitch > 89.0f)
	{
		AppInput.pitch = 89.0f;
	}
	if(AppInput.pitch < -89.0f)
	{
		AppInput.pitch = -89.0f;
	}
}

internal void
glfw_framebuffer_resize_callback(GLFWwindow *Window, int width, int height)
{
	glViewport(0, 0, width, height);
}

internal GLenum
gl_error_check(const char *filename, s32 line_number)
{
	GLenum error_code;
	// NOTE(Justin): Why would you use a while loop to do the error?

	if ((error_code = glGetError()) != GL_NO_ERROR)
	{
		switch (error_code)
		{
		case GL_INVALID_ENUM:
		{
			const char* error = "INVALID_ENUM";
			printf("%s %s %d", error, filename, line_number);
		}
		break;
		case GL_INVALID_VALUE:
		{
			const char* error = "INVALID_VALUE";
			printf("%s %s %d", error, filename, line_number);
		}
		break;
		case GL_INVALID_OPERATION:
		{
			const char* error = "INVALID_OPERATION";
			printf("%s %s %d", error, filename, line_number);
		}
		break;
		case GL_STACK_OVERFLOW:
		{
			const char* error = "STACK_OVERFLOW";
			printf("%s %s %d", error, filename, line_number);
		}
		break;
		case GL_STACK_UNDERFLOW:
		{
			const char* error = "STACK_UNDERFLOW";
			printf("%s %s %d", error, filename, line_number);
		}
		break;
		case GL_OUT_OF_MEMORY:
		{
			const char* error = "OUT_OF_MEMORY";
			printf("%s %s %d", error, filename, line_number);
		}
		break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
		{
			const char* error = "INVALID_FRAMEBUFFER_OPERATION";
			printf("%s %s %d", error, filename, line_number);
		}
		break;
		}
	}
	return(error_code);
}
#define GL_CHECK_ERROR() gl_error_check(__FILE__, __LINE__)


internal GLuint
shader_program_create_from_strings(const char* vertex_shader_str, const char* fragment_shader_str)
{
	ASSERT(vertex_shader_str && fragment_shader_str);

	GLuint shader_program = glCreateProgram();

	GLuint vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);

	// NOTE(Justin): What do we do if we get invalid handles???
	// 
	// glShaderSource
	// can source multiple shaders. The second arguement is the # of shaders,
	// the third arguement is array of pointers to strings. Each element in the array
	// is a pointer to a different shader source 

	glShaderSource(vertex_shader_handle, 1, &vertex_shader_str, NULL);
	glCompileShader(vertex_shader_handle);

	int lparams = -1;
	glGetShaderiv(vertex_shader_handle, GL_COMPILE_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		fprintf(stderr, "ERROR: vertex shader index %u did not compile\n", vertex_shader_handle);

		const int length_max = 2048;
		int length_actual = 0;
		char slog[2048];
		glGetShaderInfoLog(vertex_shader_handle, length_max, &length_actual, slog);
		fprintf(stderr, "Shader info log for GL index %u: \n%s\n", vertex_shader_handle, slog);

		glDeleteShader(vertex_shader_handle);
		glDeleteShader(fragment_shader_handle);
		glDeleteProgram(shader_program);
		return 0;
	}

	glShaderSource(fragment_shader_handle, 1, &fragment_shader_str, NULL);
	glCompileShader(fragment_shader_handle);

	lparams = -1;
	glGetShaderiv(fragment_shader_handle, GL_COMPILE_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		fprintf(stderr, "ERROR: fragment shader index %u did not compile\n", fragment_shader_handle);

		const int length_max = 2048;
		int length_actual = 0;
		char slog[2048];
		glGetShaderInfoLog(fragment_shader_handle, length_max, &length_actual, slog);
		fprintf(stderr, "Shader info log for GL index %u: \n%s\n", fragment_shader_handle, slog);

		glDeleteShader(vertex_shader_handle);
		glDeleteShader(fragment_shader_handle);
		glDeleteProgram(shader_program);
		return 0;
	}

	glAttachShader(shader_program, vertex_shader_handle);
	glAttachShader(shader_program, fragment_shader_handle);
	
	glLinkProgram(shader_program);
	GL_CHECK_ERROR();

	glDeleteShader(vertex_shader_handle);
	glDeleteShader(fragment_shader_handle);

	lparams = -1;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		fprintf(stderr, "ERROR: could not link shader program GL index %u\n", shader_program);

		const int length_max = 2048;
		int length_actual = 0;
		char slog[2048];
		glGetProgramInfoLog(shader_program, length_max, &length_actual, slog);

		fprintf(stderr, "Program info log for GL index %u: \n%s\n", shader_program, slog);

		glDeleteProgram(shader_program);
		return(0);
	}
	return(shader_program);
}

internal shader_program_t
shader_program_create_from_files(const char* vertex_shader_filename, const char* fragment_shader_filename)
{

	ASSERT(vertex_shader_filename && fragment_shader_filename);

	shader_program_t Result = {};

	Result.vertex_shader_filename = vertex_shader_filename;
	Result.fragment_shader_filename = fragment_shader_filename;

	char vertex_shader_src[1024];
	char fragment_shader_src[2048];

	vertex_shader_src[0] = fragment_shader_src[0] = '\0';

	FILE* VertexShaderFileHandle = fopen(vertex_shader_filename, "r");
	if (VertexShaderFileHandle)
	{
		size_t count = fread(vertex_shader_src, 1, sizeof(vertex_shader_src), VertexShaderFileHandle);
		//ASSERT((count < MAX_SHADER_SIZE - 1) && (count != 0));
		vertex_shader_src[count] = '\0';
		fclose(VertexShaderFileHandle);
	}
	else
	{
		fprintf(stderr, "Error: Could not open vertex shader file %s\n", vertex_shader_filename);
	}

	FILE* FragmentShaderFileHandle = fopen(fragment_shader_filename, "r");
	if(FragmentShaderFileHandle)
	{
		size_t count = fread(fragment_shader_src, 1, sizeof(fragment_shader_src), FragmentShaderFileHandle);
		//ASSERT(count < (MAX_SHADER_SIZE - 1));
		fragment_shader_src[count] = '\0';
		fclose(FragmentShaderFileHandle);

	}
	else
	{
		fprintf(stderr, "Error: Could not open fragment shader file %s\n", fragment_shader_filename);
	}


	Result.id = shader_program_create_from_strings(vertex_shader_src, fragment_shader_src);
	return(Result);
}

internal void
shader_program_reload(shader_program_t *ShaderProgram)
{
	ASSERT(ShaderProgram->vertex_shader_filename &&
		   ShaderProgram->fragment_shader_filename &&
		   ShaderProgram->id);

	shader_program_t TestShaderProgram = {};

	TestShaderProgram = shader_program_create_from_files(ShaderProgram->vertex_shader_filename,
		ShaderProgram->fragment_shader_filename);
	if(TestShaderProgram.id)
	{
		glDeleteProgram(ShaderProgram->id);

		ShaderProgram->id = TestShaderProgram.id;
		printf("Shader reloaded\n");
		gl_log_shader_info(ShaderProgram->id);
		ShaderProgram->reloaded = true;

		glDeleteProgram(TestShaderProgram.id);
	
	}
	else
	{
		printf("Error shader not reloaded");
	}
}

internal void
texture_set_active_and_bind(u32 index, texture_t *Texture)
{
	// TODO(Justin): Need to use an index to figure out which # texture to
	// activeate.
	GLenum TEXTURE_INDEX;
	switch(index)
	{
		case 0:
		{
			TEXTURE_INDEX = GL_TEXTURE0;
		} 
		break;
		case 1:
		{
			TEXTURE_INDEX = GL_TEXTURE1;
		}
		break;
	}
	glActiveTexture(TEXTURE_INDEX);

	// TODO(Justin): Need to have the texture structure contain the texture
	// type.
	glBindTexture(GL_TEXTURE_2D, Texture->id);
}

internal texture_t
texture_simple_init(const char* filename)
{
	texture_t Texture = {};
	glGenTextures(1, &Texture.id);
	glBindTexture(GL_TEXTURE_2D, Texture.id);

	// TODO(Justin): Pass these in as arguements to the function? Or have the
	// texture struct itself contain this info. LAtter seems to make more sence
	// because the parameters can  be thoght of properties of the desired
	// texture.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	Texture.memory = stbi_load(filename, &Texture.width, &Texture.height, &Texture.channel_count, 0);

	if(Texture.memory)
	{
		// TODO(Justin): Figure out why the third if is getting completley nuked
		// even though it should hit for container2.jpg :(

		GLenum PIXEL_FORMAT;
		if(Texture.channel_count == 1)
		{
			PIXEL_FORMAT = GL_RED;
		} 

	    if(Texture.channel_count == 3)
		{
			PIXEL_FORMAT = GL_RGB;
		}

		if(Texture.channel_count == 4)
		{
			PIXEL_FORMAT = GL_RGBA;
		}
		
		Texture.mipmap_level = 0;
		

		//PIXEL_FORMAT = GL_RGBA;
		// TODO(Justin): How are you supposed to know that imgae type in a general manner? One way is to have a naming convention on the names of files that tell use info about the image data.
		glTexImage2D(GL_TEXTURE_2D, Texture.mipmap_level, PIXEL_FORMAT,
					 Texture.width, Texture.height, 0,
					 PIXEL_FORMAT, GL_UNSIGNED_BYTE, Texture.memory);

		glGenerateMipmap(GL_TEXTURE_2D);

	}
	// TODO(Justin): Do we really need to free the memory?
	stbi_image_free(Texture.memory);
	return(Texture);
}

internal void
uniform_set_b32(GLuint program_id, const char* uniform_name, b32 value)
{
	// TODO(Justin): If these function calls!!!
	glUniform1i(glGetUniformLocation(program_id, uniform_name), (int)value);
}

internal void
uniform_set_s32(GLuint program_id, const char* uniform_name, s32 value)
{
	glUniform1i(glGetUniformLocation(program_id, uniform_name), value);
}

internal void
uniform_set_f32(GLuint program_id, const char* uniform_name, f32 value)
{
	glUniform1f(glGetUniformLocation(program_id, uniform_name), value);
}

internal void
uniform_set_mat4f(GLuint shader_program_id, const char* uniform_name, glm::mat4 Transform)
{
	u32 transform_location = glGetUniformLocation(shader_program_id, uniform_name);
	glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(Transform));
}

internal void
uniform_set_v3f(GLuint shader_program_id, const char* uniform_name, f32 x, f32 y, f32 z)
{
	glUniform3f(glGetUniformLocation(shader_program_id, uniform_name), x, y, z);
}

internal void
uniform_set_vec3f(GLuint shader_program_id, const char* uniform_name, glm::vec3 V)
{
	uniform_set_v3f(shader_program_id, uniform_name, V.x, V.y, V.z);
}

internal void
glfw_process_input(GLFWwindow *Window, app_state_t *AppState, f32 time_delta)
{
	camera_t *Camera = &AppState->Camera;

	// To know where to go, first need to know what direction we are headed!
	camera_direction_set(Camera, AppInput.yaw, AppInput.pitch);


	glm::vec3 Right = glm::cross(Camera->Direction, -1.0f * Camera->Up);
	Right = glm::normalize(Right);

	// TODO(Justin): Need to implement multiple key presses? Or situation when
	// we hold down keys
	//glm::vec3 Delta = glm::vec3(0.0f, 0.0f, 0.0f);
	if(glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(Window, true);
	}
	else if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * E2; 
	}
	else if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * Right;
	}
	else if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * -E2;
	}
	else if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Camera->Pos += -1.0f * time_delta * Camera->speed * Right;
	}
	else if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Camera->Pos += 1.0f * time_delta * Camera->speed * Camera->Direction;
	}
	else if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Camera->Pos += -1.0f * time_delta * Camera->speed * Camera->Direction;
	}
	else if (glfwGetKey(Window, GLFW_KEY_1) == GLFW_PRESS)
	{
	}
	else if (glfwGetKey(Window, GLFW_KEY_R) == GLFW_PRESS)
	{
		shader_program_reload(&AppState->ShaderProgram);
	}
	else if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
	}
}

int main(void)
{
	gl_log_restart();
	gl_log_message("Starting GLFW\n%s\n", glfwGetVersionString());
	glfwSetErrorCallback(glfw_error_callback);


	if(!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	//TODO(Justin): Need a build option to for debug build. The debug context is usefule but slow
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

	window_t Window = {};
	Window.width = 960;
	Window.height= 540;
	Window.handle = glfwCreateWindow(Window.width, Window.height, "OpenGL", NULL, NULL);

	if(!Window.handle)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(Window.handle);
	glfwSetInputMode(Window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetFramebufferSizeCallback(Window.handle, glfw_framebuffer_resize_callback);
	glfwSetCursorPosCallback(Window.handle, glfw_mouse_callback);

	gl_log_params();

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		printf("Oops\n");
	}

	//TODO(Justin): OpenGL info struct
	const GLubyte *renderer_name = glGetString(GL_RENDERER);
	const GLubyte* renderer_version = glGetString(GL_VERSION);
	printf("Renderer = %s\n", renderer_name);
	printf("Renderer version = %s\n", renderer_version);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//
	// NOTE(Justin): Buffer initialization
	//

	// Cube
	vertex_array_t CubeVertexArray = vertex_array_create();
	vertex_buffer_t CubeVertexBuffer = vertex_buffer_create(cube_vertices_normals_and_tex_coods, sizeof(cube_vertices_normals_and_tex_coods));

	glBindVertexArray(CubeVertexArray.id);

	vertex_buffer_bind(&CubeVertexBuffer);

	vertex_buffer_layout_t CubeVertexBufferLayout;

	// Positions
	CubeVertexBufferLayout.element_count_per_attribute = 3;
	CubeVertexBufferLayout.attribute_type = GL_FLOAT;
	CubeVertexBufferLayout.normalized = GL_FALSE;
	CubeVertexBufferLayout.size_for_each_vertex = 8 * sizeof(float);
	CubeVertexBufferLayout.attribute_stride = (void*)0;
	vertex_array_add_buffer_layout(0, &CubeVertexArray, &CubeVertexBufferLayout);

	// Normals
	CubeVertexBufferLayout.attribute_stride = (void*)(3 * sizeof(float));
	vertex_array_add_buffer_layout(1, &CubeVertexArray, &CubeVertexBufferLayout);

	// Texture coordinate
	CubeVertexBufferLayout.element_count_per_attribute = 2;
	CubeVertexBufferLayout.attribute_stride = (void*)(6 * sizeof(float));
	vertex_array_add_buffer_layout(2, &CubeVertexArray, &CubeVertexBufferLayout);

	// Lamp
	vertex_array_t LightVertexArray = vertex_array_create();
	glBindVertexArray(LightVertexArray.id);

	vertex_buffer_bind(&CubeVertexBuffer);

	// Position
	CubeVertexBufferLayout.element_count_per_attribute = 3;
	CubeVertexBufferLayout.attribute_stride = (void*)0;
	vertex_array_add_buffer_layout(0, &LightVertexArray, &CubeVertexBufferLayout);

	//
	// NOTE(Justin): Texture initialization
	//

	texture_t container_diffuse = texture_simple_init("textures/container2.png");
	texture_t container_specular = texture_simple_init("textures/container2_specular.png");


	//
	// NOTE(Justin): Shader initialization
	//

	shader_program_t LightShader;
	shader_program_t CubeShader;

	const char* cube_vertex_shader_filename = "shaders/cube_vertex_shader_light_casters.glsl";

	const char* cube_fragment_shader_filename = "shaders/cube_fragment_shader_light_casters.glsl";

	const char* light_vertex_shader_filename = "shaders/light_vertex_shader_001.glsl";
	const char* light_fragment_shader_filename = "shaders/light_fragment_shader_001.glsl";

	// TODO(Justin) is_valid member?
	LightShader = shader_program_create_from_files(light_vertex_shader_filename, light_fragment_shader_filename);
	CubeShader = shader_program_create_from_files(cube_vertex_shader_filename, cube_fragment_shader_filename);

	gl_log_shader_info(LightShader.id);
	gl_log_shader_info(CubeShader.id);

	AppInput.Mouse.Pos.x = Window.width / 2;
	AppInput.Mouse.Pos.y = Window.height / 2;
	AppInput.Mouse.sensitivity = 0.01f;
	AppInput.yaw = -90.0f;
	AppInput.pitch = 0.0f;

	camera_t Camera = {};

	Camera.Pos = glm::vec3(0.0f, 0.0f, 3.0f);
	Camera.Direction.x = cos(glm::radians(AppInput.yaw)) * cos(glm::radians(AppInput.pitch));
	Camera.Direction.y = sin(glm::radians(AppInput.pitch));
	Camera.Direction.z = sin(glm::radians(AppInput.yaw)) * cos(glm::radians(AppInput.pitch));
	Camera.Direction = glm::normalize(Camera.Direction);
	Camera.Up = E2;
	Camera.speed = 5.0f;

	app_state_t AppState = {};
	AppState.LightShader = LightShader;
	AppState.CubeShader = CubeShader;
	AppState.Camera = Camera;


	glm::mat4 MapToCamera = glm::lookAt(AppState.Camera.Pos, AppState.Camera.Pos + AppState.Camera.Direction, AppState.Camera.Up);
	MapToCamera = glm::translate(MapToCamera, glm::vec3(0.0f, 0.0f, -3.0f));

	f32 field_of_view = glm::radians(45.0f);
	f32 aspect_ratio = (f32)Window.width / (f32)Window.height;
	f32 near = 0.1f;
	f32 far = 100.0f;

	glm::mat4 MapToPersp = glm::perspective(field_of_view, aspect_ratio, near, far);

	glm::vec3 light_pos;
	glm::vec3 offset = glm::vec3(1.0f, 1.0f, 1.0f);

	light_pos.x = cube_positions[0][0];
	light_pos.y = cube_positions[0][1];
	light_pos.z = cube_positions[0][2];

	light_pos += offset;

	// /TODO(Justin): Why do we need to do this 
	glUseProgram(AppState.LightShader.id);
	uniform_set_s32(AppState.CubeShader.id, "u_material.diffuse", 0);
	uniform_set_s32(AppState.CubeShader.id, "u_material.specular", 1);

	f32 time_delta = 0.0f;
	f32 time_previous = glfwGetTime();
    while (!glfwWindowShouldClose(Window.handle))
	{
		glfw_process_input(Window.handle, &AppState, time_delta);

		//
		// NOTE(Justin): Render
		//

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//
		// NOTE(Justin): Cube
		//

		glUseProgram(AppState.CubeShader.id);

		light_pos.x = cos(glfwGetTime());
		light_pos.y = cos(glfwGetTime());
		light_pos.z = -sin(glfwGetTime());

		uniform_set_vec3f(AppState.CubeShader.id, "u_camera_pos", Camera.Pos);

		uniform_set_vec3f(AppState.CubeShader.id, "u_material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
		uniform_set_vec3f(AppState.CubeShader.id, "u_material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		uniform_set_f32(AppState.CubeShader.id, "u_material.shininess", 32.0f);

		uniform_set_vec3f(AppState.CubeShader.id, "u_light_point.pos", light_pos);
		uniform_set_vec3f(AppState.CubeShader.id, "u_light_point.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
		uniform_set_vec3f(AppState.CubeShader.id, "u_light_point.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
		uniform_set_vec3f(AppState.CubeShader.id, "u_light_point.specular", glm::vec3(1.0f, 1.0f, 1.0f));

		uniform_set_f32(AppState.CubeShader.id, "u_light_point.atten_constant", 1.0f);
		uniform_set_f32(AppState.CubeShader.id, "u_light_point.atten_linear", 0.09f);
		uniform_set_f32(AppState.CubeShader.id, "u_light_point.atten_quadratic", 0.032f);

		MapToCamera = glm::lookAt(AppState.Camera.Pos, AppState.Camera.Pos + AppState.Camera.Direction, AppState.Camera.Up);

		for (u32 i = 0; i < ARRAY_COUNT(cube_positions); i++)
		{
			glm::mat4 ModelTransform = glm::mat4(1.0f);
			glm::vec3 pos = glm::vec3(cube_positions[i][0], cube_positions[i][1], cube_positions[i][2]);
			ModelTransform = glm::translate(ModelTransform, glm::vec3(pos));

			uniform_set_mat4f(AppState.CubeShader.id, "ModelTransform", ModelTransform);
			uniform_set_mat4f(AppState.CubeShader.id, "MapToCamera", MapToCamera);
			uniform_set_mat4f(AppState.CubeShader.id, "MapToPersp", MapToPersp);

			texture_set_active_and_bind(0, &container_diffuse);
			texture_set_active_and_bind(1, &container_specular);

			glBindVertexArray(CubeVertexArray.id);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}


		//
		// NOTE(Justin): Lamp
		//

		glUseProgram(AppState.LightShader.id);

		glm::mat4 ModelTransform = glm::mat4(1.0f);
		ModelTransform = glm::translate(ModelTransform, light_pos);
		ModelTransform = glm::scale(ModelTransform, glm::vec3(0.2f));

		uniform_set_mat4f(AppState.LightShader.id, "ModelTransform", ModelTransform);
		uniform_set_mat4f(AppState.LightShader.id, "MapToCamera", MapToCamera);
		uniform_set_mat4f(AppState.LightShader.id,"MapToPersp", MapToPersp);

		glBindVertexArray(LightVertexArray.id);
		glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwPollEvents();
        glfwSwapBuffers(Window.handle);

		f32 time_current = glfwGetTime();
		time_delta = time_current - time_previous;
		time_previous = time_current;
	}
	glfwTerminate();
	return 0;
}
