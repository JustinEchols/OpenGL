
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef s32 b32;

typedef float f32;
typedef double f64;

#include "application.h"

#define internal static;
#define global_variable static;
#define local_persist static;

#define GL_LOG_FILE "gl.log"
#include "application_log.cpp"

#define ASSERT(expression) if((!expression)) {*(int *)0 = 0;}
#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0])


#define PI32 3.141592653589f
#define MAX_SHADER_SIZE 100000

// TODO(Justin): Which data structures need to be globals?
global_variable input_t AppInput;




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
	Delta.x = xpos - Mouse->Pos.x;
	Delta.y = Mouse->Pos.y - ypos;

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

internal GLuint
shader_program_create_from_files(const char* vertex_shader_filename, const char* fragment_shader_filename)
{

	ASSERT(vertex_shader_filename && fragment_shader_filename);

	GLuint shader_program;

	char vertex_shader_src[1024];
	char fragment_shader_src[1024];

	vertex_shader_src[0] = fragment_shader_src[0] = '\0';

	FILE* VertexShaderFileHandle = fopen(vertex_shader_filename, "r");
	if(!VertexShaderFileHandle)
	{
		fprintf(stderr, "Error: Could not open vertex shader file %s\n", vertex_shader_filename);
		return(0);
	}
	size_t count = fread(vertex_shader_src, 1, sizeof(vertex_shader_src), VertexShaderFileHandle);
	//ASSERT((count < MAX_SHADER_SIZE - 1) && (count != 0));
	vertex_shader_src[count] = '\0';
	fclose(VertexShaderFileHandle);

	
	FILE* FragmentShaderFileHandle = fopen(fragment_shader_filename, "r");
	if(!FragmentShaderFileHandle)
	{
		fprintf(stderr, "Error: Could not open vertex shader file %s\n", fragment_shader_filename);
		return(0);
	}
	count = fread(fragment_shader_src, 1, sizeof(fragment_shader_src), FragmentShaderFileHandle);
	//ASSERT(count < (MAX_SHADER_SIZE - 1));
	fragment_shader_src[count] = '\0';
	fclose(FragmentShaderFileHandle);

	shader_program = shader_program_create_from_strings(vertex_shader_src, fragment_shader_src);
	return(shader_program);
}

internal void
shader_program_reload(shader_program_t *ShaderProgram)
{
	ASSERT(ShaderProgram->vertex_shader_filename &&
		   ShaderProgram->fragment_shader_filename &&
		   ShaderProgram->id);

	shader_program_t TestShaderProgram = {};

	TestShaderProgram.id = shader_program_create_from_files(ShaderProgram->vertex_shader_filename,
			ShaderProgram->fragment_shader_filename);

	if(TestShaderProgram.id)
	{
		glDeleteProgram(ShaderProgram->id);
		ShaderProgram->id = TestShaderProgram.id;
		printf("Shader reloaded\n");
		gl_log_shader_info(ShaderProgram->id);
		ShaderProgram->reloaded = true;
	}
	else
	{
		printf("Error shader not reloaded");
	}
}

internal texture_t
texture_simple_init(const char* filename, GLenum IMAGE_TYPE)
{
	texture_t Texture = {};
	glGenTextures(1, &Texture.id);
	glBindTexture(GL_TEXTURE_2D, Texture.id);

	// TODO(Justin): Pass these in as arguementsto the function?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	Texture.memory = stbi_load(filename, &Texture.width, &Texture.height, &Texture.channel_count, 0);

	if(Texture.memory)
	{
		Texture.mipmap_level = 0;


		// TODO(Justin): How are you supposed to know that imgae type in a general manner? One way is to have a naming convention on the names of files that tell use info about the image data.
		glTexImage2D(GL_TEXTURE_2D, Texture.mipmap_level, GL_RGB, Texture.width, Texture.height, 0, IMAGE_TYPE, GL_UNSIGNED_BYTE, Texture.memory);
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
	glUniform1i(glGetUniformLocation(program_id, uniform_name), value);
}

internal void
uniform_set_mat4f(GLuint shader_program_id, glm::mat4 Transform, const char* uniform_name)
{
	u32 transform_location = glGetUniformLocation(shader_program_id, uniform_name);
	glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(Transform));
}

internal void
process_input(GLFWwindow *Window, app_state_t *AppState, f32 time_delta)
{
	camera_t *Camera = &AppState->Camera;
	if(glfwGetKey(Window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(Window, true);
	}
	else if (glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * glm::vec3(0.0f, 1.0f, 0.0f);
	}
	else if (glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * glm::vec3(-1.0f, 0.0f, 0.0f);
	}
	else if (glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * glm::vec3(0.0f, -1.0f, 0.0f);
	}
	else if (glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * glm::vec3(1.0f, 0.0f, 0.0f);
	}
	else if (glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * glm::vec3(0.0f, 0.0f, -1.0f);
	}
	else if (glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * glm::vec3(0.0f, 0.0f, 1.0f);
	}
	else if (glfwGetKey(Window, GLFW_KEY_R) == GLFW_PRESS)
	{
		shader_program_reload(&AppState->ShaderProgram);
	}
	else if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
	}

	camera_direction_set(Camera, AppInput.yaw, AppInput.pitch);
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
	Window.handle = glfwCreateWindow(Window.width, Window.height, "Hello World", NULL, NULL);

	if(!Window.handle)
	{
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(Window.handle);
	glfwSetInputMode(Window.handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

	shader_program_t ShaderProgram;

	ShaderProgram.vertex_shader_filename = "shaders/test_vertex_shader.glsl";
	ShaderProgram.fragment_shader_filename = "shaders/test_fragment_shader.glsl";
	ShaderProgram.id = shader_program_create_from_files(ShaderProgram.vertex_shader_filename, ShaderProgram.fragment_shader_filename);
	gl_log_shader_info(ShaderProgram.id);


	// NOTE(Justin): Can use an array of GLuints for each texture we want
	texture_t Texture, Texture2;

	Texture = texture_simple_init("textures/container.jpg", GL_RGB);
	Texture2 = texture_simple_init("textures/awesomeface.png", GL_RGBA);

	f32 vertices[] =
	{
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	glm::vec3 cube_positions[] =
	{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};


	GLuint VBO, VAO, EBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);


	// Positions
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

	// Texture coords
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	app_state_t AppState = {};
	AppState.ShaderProgram = ShaderProgram;

	glUseProgram(AppState.ShaderProgram.id);

	// TODO(Justin): How are you supposed to keep track of a bunch of unioform names?
	uniform_set_s32(AppState.ShaderProgram.id, "texture1", 0);
	uniform_set_s32(AppState.ShaderProgram.id, "texture2", 1);


	glm::vec3 WorldOrigin = glm::vec3(0.0f, 0.0f, 0.0f);

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

	Camera.Up = glm::vec3(0.0f, 1.0f, 0.0f);
	Camera.speed = 10.0f;

	AppState.Camera = Camera;



	// mouse_yaw, mouse_pitch? Should the yaw and pitch be members of the mouse
	// struct?




	glm::mat4 MapToCamera = glm::lookAt(AppState.Camera.Pos, AppState.Camera.Pos + AppState.Camera.Direction, AppState.Camera.Up);
	MapToCamera = glm::translate(MapToCamera, glm::vec3(0.0f, 0.0f, -3.0f));

	f32 field_of_view = glm::radians(45.0f);
	f32 aspect_ratio = (f32)Window.width / (f32)Window.height;
	f32 near = 0.1f;
	f32 far = 100.0f;

	glm::mat4 MapToPersp = glm::perspective(field_of_view, aspect_ratio, near, far);

	uniform_set_mat4f(AppState.ShaderProgram.id, MapToPersp, "MapToPersp");
	uniform_set_mat4f(AppState.ShaderProgram.id, MapToCamera, "MapToCamera");

	u32 frame_count = 0;
	f32 time_delta = 0.0f;
	f32 time_previous = glfwGetTime();
    while (!glfwWindowShouldClose(Window.handle))
	{
		process_input(Window.handle, &AppState, time_delta);

		if(AppState.ShaderProgram.reloaded)
		{
			uniform_set_mat4f(AppState.ShaderProgram.id, MapToPersp, "MapToPersp");
			uniform_set_mat4f(AppState.ShaderProgram.id, MapToCamera, "MapToCamera");
			AppState.ShaderProgram.reloaded = false;
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture.id);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2.id);

		glUseProgram(AppState.ShaderProgram.id);
		GL_CHECK_ERROR();

		MapToCamera = glm::lookAt(AppState.Camera.Pos, AppState.Camera.Pos + AppState.Camera.Direction, AppState.Camera.Up);
		uniform_set_mat4f(AppState.ShaderProgram.id, MapToCamera, "MapToCamera");

		glBindVertexArray(VAO);
		for (u32 i = 0; i < 9; i++)
		{
			glm::mat4 ModelTransform = glm::mat4(1.0f);
			ModelTransform = glm::translate(ModelTransform, cube_positions[i]);
			ModelTransform = glm::rotate(ModelTransform,
										 ((f32)time_delta * glm::radians(50.0f)),
										 glm::vec3(1.0f, 0.3f, 0.5f));


			uniform_set_mat4f(AppState.ShaderProgram.id, ModelTransform, "ModelTransform");
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

        glfwPollEvents();
        glfwSwapBuffers(Window.handle);

		f32 time_current = glfwGetTime();

		time_delta = time_current - time_previous;

		time_previous = time_current;

	}
	glfwTerminate();
	return 0;
}
