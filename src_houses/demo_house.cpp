#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windows.h>

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>


#include "application.h"
#include "application_cube.h"
#include "application_math.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "application_util.cpp"
#include "application_mesh.cpp"

global_variable input_t AppInput;

internal u32
get_string_length(char *string)
{
	u32 Result = 0;
	for(char *c = string; *c != '\0'; c++)
	{
		Result++;
	}
	return(Result);
}

// TODO(Justin): This is completley unsafe and not to be desired at all. Only
// works for when len a < len b. Meant to be used for copying paths to a buffer
// Will always work if strin_a is a buffer big enough to copy string_b to.

internal void
copy_strings(char *string_a, char *string_b)
{
	char *src = string_b;
	char *dest = string_a;
	while(*src != '\0')
	{
		*dest++ = *src++;
	}
	*dest++ = '\0';
}


#if 1
internal void
concat_strings(char *buff, char *string_a, char *string_b)
{
	// TODO(Justin): Make sure the buffer

	copy_strings(buff, string_a);

	u32 length = get_string_length(string_a);
	char *c = buff + length;

	copy_strings(c, string_b);
}
#endif

#if 0
internal char *
concat_strings(char *string_a, char *string_b)
{
	// TODO(Justin): Make sure the buffer
	char Result[256];

	copy_strings(Result, string_a);
	u32 length = get_string_length(Result);
	char *c = Result + length;
	copy_strings(c, string_b);

	return(Result);
}
#endif

internal b32
strings_are_same(char *string_a, char *string_b)
{
	b32 Result = 0;

	u32 a_length = get_string_length(string_a);
	u32 b_length = get_string_length(string_b);

	if(a_length == b_length)
	{
		char *pa = string_a;
		char *pb = string_b;

		while(*pa != '\0')
		{
			Result = (*pa++ == *pb++);
			if(Result)
			{
				// The chars are the same, do nothing and look at the next pair.
			}
			else
			{
				break;
			}
		}
	}
	return(Result);
}

#if 1
internal char *
single_digit_to_string(u32 digit)
{
	ASSERT((0 <= digit) && (digit <= 9));

	char *Result;

	if(digit == 0)
	{
		Result = "0";
	}
	else if(digit == 1)
	{
		Result = "1";
	}
	else if(digit == 2)
	{
		Result = "2";
	}
	else if(digit == 3)
	{
		Result = "3";
	}
	else if(digit == 4)
	{
		Result = "4";
	}
	else if(digit == 5)
	{
		Result = "5";
	}
	else if(digit == 6)
	{
		Result = "6";
	}
	else if(digit == 7)
	{
		Result = "7";
	}
	else if(digit == 8)
	{
		Result = "8";
	}
	else
	{
		Result = "9";
	}
	return(Result);
}
#endif


internal void
digit_to_string(char *buff, u32 value)
{
	// NOTE(Justin): Only works for numbers 0-99
	ASSERT((0 <= value) && (value <= 99));

	u32 remainder = value % 10;
	u32 ones_digit = remainder;
	u32 tens_digit = (value - remainder) / 10;

	char *c;
	if(tens_digit == 0)
	{
		c = single_digit_to_string(ones_digit);
		copy_strings(buff, c);
	}
	else
	{
		char *b = single_digit_to_string(ones_digit);
		char *a = single_digit_to_string(tens_digit);

		concat_strings(buff, a, b);
	}
}

internal string_t
copy_string_to_buff(char *string)
{
	string_t Result = {};

	char buff[256];
	char *c = string;
	u32 length = get_string_length(string);

	for(u32 char_index = 0; char_index < length; char_index++)
	{
		buff[char_index] = *c++;
	}
	buff[length] = '\0';

	Result.data = buff;
	Result.count = length;
	
	return(Result);
}

internal string_t 
get_path_to_dir(char *full_path_to_file)
{
	string_t Result = {};

	char *one_past_last_slash;
	for(char *c = full_path_to_file; *c != '\0'; c++)
	{
		if(*c == '/')
		{
			one_past_last_slash = c;
		}
	}
	one_past_last_slash++;

	u32 char_count = one_past_last_slash - full_path_to_file;

	char buff[256];
	char *c = full_path_to_file;
	u32 char_index;
	for(char_index = 0; char_index < char_count; char_index++)
	{
		buff[char_index] = *c++;
	}
	buff[char_index] = '\0';

	Result.data = buff;
	Result.count = char_count;

	return(Result);
}



internal vertex_array_t
vertex_array_create()
{
	vertex_array_t Result = {};

	glGenVertexArrays(1, &Result.id);

	return(Result);
}

internal void
vertex_array_add_buffer_layout(u32 index, vertex_array_t *VertexArray, vertex_buffer_layout_t *VertexBufferLayout)
{

	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, VertexBufferLayout->element_count_per_attribute,
			VertexBufferLayout->attribute_type, VertexBufferLayout->normalized,
			VertexBufferLayout->size_for_each_vertex, VertexBufferLayout->attribute_stride);
}

internal void
vertex_array_bind(vertex_array_t VertexArray)
{
	glBindVertexArray(VertexArray.id);
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
glfw_mouse_button_callback(GLFWwindow *Window, int button, int action, int mods)
{
	switch(button)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
		{
			if(action == GLFW_PRESS)
			{
				printf("Mouse Left\n");
			}
		} break;
		case GLFW_MOUSE_BUTTON_RIGHT:
		{
			if(action == GLFW_PRESS)
			{
				printf("Mouse Right\n");
			}
		} break;
	}
}

internal void
glfw_framebuffer_resize_callback(GLFWwindow *Window, int width, int height)
{
	glViewport(0, 0, width, height);
}

internal GLuint
shader_program_create_from_strings(char *vertex_shader_str, char *fragment_shader_str)
{
	ASSERT(vertex_shader_str && fragment_shader_str);

	GLuint shader_program = glCreateProgram();
	GLuint vertex_shader_handle = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragment_shader_handle = glCreateShader(GL_FRAGMENT_SHADER);

	glShaderSource(vertex_shader_handle, 1, &vertex_shader_str, NULL);
	glCompileShader(vertex_shader_handle);

	int lparams = -1;
	glGetShaderiv(vertex_shader_handle, GL_COMPILE_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		// TODO(Justin): The filename is probably going to be more useful here
		// instead of just a number...
		const int length_max = 2048;
		int length_actual = 0;
		char error_string[2048];

		glGetShaderInfoLog(vertex_shader_handle, length_max, &length_actual, error_string);

		fprintf(stderr, "[OpenGL] ERROR: Vertex shader did not compile.\n");
		fprintf(stderr, "[OpenGL] INDEX: %u\n", vertex_shader_handle);
		fprintf(stderr, "[OpenGL] %s%s", error_string, vertex_shader_str);

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
		const int length_max = 2048;
		int length_actual = 0;
		char error_string[2048];

		glGetShaderInfoLog(fragment_shader_handle, length_max, &length_actual, error_string);

		fprintf(stderr, "[OpenGL] ERROR: Fragment shader did not compile\n");
		fprintf(stderr, "[OpenGL] INDEX: %u\n", fragment_shader_handle);
		fprintf(stderr, "[OpenGL] %s%s", error_string, fragment_shader_str);

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


		const int length_max = 2048;
		int length_actual = 0;
		char error_string[2048];
		glGetProgramInfoLog(shader_program, length_max, &length_actual, error_string);

		fprintf(stderr, "[OpenGL] ERROR: could not link shader program\n");
		fprintf(stderr, "[OpenGL] INDEX: %u\n", shader_program);
		fprintf(stderr, "[OpenGL] %s", error_string);

		glDeleteProgram(shader_program);
		return(0);
	}
	return(shader_program);
}

internal void
shader_load_source_from_file(char *buff, size_t buff_size, char *filename)
{
	FILE *FileHandle = fopen(filename, "r");
	if(FileHandle)
	{
		size_t num_chars_read = fread(buff, 1, buff_size, FileHandle);
		buff[num_chars_read] = '\0';
		fclose(FileHandle);
	}
	else
	{
		fprintf(stderr, "[OpenGL] Error: Could not open shader file %s\n", filename);
	}
}

internal void
shader_program_add_shader(shader_program_t *Shader, GLenum SHADER_TYPE, char *shader_filename)
{
	GLuint shader_handle = glCreateShader(SHADER_TYPE);

	char *shader_source_buff = (char *)malloc(MAX_SHADER_SIZE);
	shader_load_source_from_file(shader_source_buff, MAX_SHADER_SIZE, shader_filename);

	glShaderSource(shader_handle, 1, &shader_source_buff, NULL);
	glCompileShader(shader_handle);

	int lparams = -1;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		// TODO(Justin): The filename is probably going to be more useful here
		// instead of just a number...
		const int length_max = 2048;
		int length_actual = 0;
		char error_string[2048];

		glGetShaderInfoLog(shader_handle, length_max, &length_actual, error_string);

		fprintf(stderr, "[OpenGL] ERROR: Shader did not compile.\n");
		fprintf(stderr, "[OpenGL] INDEX: %u\n", shader_handle);
		fprintf(stderr, "[OpenGL] %s%s", error_string, shader_source_buff);

		glDeleteShader(shader_handle);
		// TODO(Justin): Not sure that returning in the middle of a routine is a
		// good idea.
		return;
	}

	glAttachShader(Shader->id, shader_handle);
	glLinkProgram(Shader->id);
	glDeleteShader(shader_handle);

	lparams = -1;
	glGetProgramiv(Shader->id, GL_LINK_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		const int length_max = 2048;
		int length_actual = 0;
		char error_string[2048];

		glGetProgramInfoLog(Shader->id, length_max, &length_actual, error_string);

		fprintf(stderr, "[OpenGL] ERROR: Could not link shader program\n");
		fprintf(stderr, "[OpenGL] INDEX: %u\n", Shader->id);
		fprintf(stderr, "[OpenGL] %s", error_string);

		// TODO(Justin): What happens to the program when linking fails to link
		// a new shader to a another program? Is the nprogram stilluse able just
		// without the new shader that we failed to link. Or is the program now
		// unusable because linking failed? Do we need to delete the program?
		glDeleteProgram(Shader->id);
	}
	gl_log_shader_info(Shader);
}

internal shader_program_t
shader_program_create_from_files(char *vertex_shader_filename, char *fragment_shader_filename)
{
	ASSERT(vertex_shader_filename && fragment_shader_filename);

	shader_program_t Result = {};

	Result.vertex_shader_filename = vertex_shader_filename;
	Result.fragment_shader_filename = fragment_shader_filename;

	char vertex_shader_src[1024];
	shader_load_source_from_file(vertex_shader_src, sizeof(vertex_shader_src), vertex_shader_filename);

	char fragment_shader_src[MAX_SHADER_SIZE];
	shader_load_source_from_file(fragment_shader_src, sizeof(fragment_shader_src), fragment_shader_filename);

	Result.id = shader_program_create_from_strings(vertex_shader_src, fragment_shader_src);
	return(Result);
}

internal GLuint
shader_program_create_from_src(char *source, GLenum SHADER_TYPE)
{
	ASSERT(source);

	GLuint shader_program = glCreateProgram();
	GLuint shader_handle = glCreateShader(SHADER_TYPE);

	glShaderSource(shader_handle, 1, &source, NULL);
	glCompileShader(shader_handle);

	int lparams = -1;
	glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		// TODO(Justin): The filename is probably going to be more useful here
		// instead of just a number...
		const int length_max = 2048;
		int length_actual = 0;
		char error_string[2048];

		glGetShaderInfoLog(shader_handle, length_max, &length_actual, error_string);

		char *shader_type = gl_enum_type_to_string(SHADER_TYPE);
		fprintf(stderr, "[OpenGL] ERROR: %s shader did not compile.\n", shader_type);
		fprintf(stderr, "[OpenGL] INDEX: %u\n", shader_handle);
		fprintf(stderr, "[OpenGL] %s%s", error_string, source);

		glDeleteShader(shader_handle);
		glDeleteProgram(shader_program);
		return 0;
	}

	glAttachShader(shader_program, shader_handle);
	glLinkProgram(shader_program);
	GL_CHECK_ERROR();
	glDeleteShader(shader_handle);

	lparams = -1;
	glGetProgramiv(shader_program, GL_LINK_STATUS, &lparams);
	if(lparams != GL_TRUE)
	{
		const int length_max = 2048;
		int length_actual = 0;
		char error_string[2048];
		glGetProgramInfoLog(shader_program, length_max, &length_actual, error_string);

		fprintf(stderr, "[OpenGL] ERROR: could not link shader program\n");
		fprintf(stderr, "[OpenGL] INDEX: %u\n", shader_program);
		fprintf(stderr, "[OpenGL] %s", error_string);

		glDeleteProgram(shader_program);
		return(0);
	}
	return(shader_program);
}

internal shader_program_t
shader_program_create_from_file(GLenum SHADER_TYPE, char *filename)
{
	ASSERT(filename);
	shader_program_t Result = {};

	if(SHADER_TYPE == GL_VERTEX_SHADER) 
	{
		Result.vertex_shader_filename = filename;
	}
	else if(SHADER_TYPE == GL_FRAGMENT_SHADER)
	{
		Result.fragment_shader_filename = filename;
	}
	else if(SHADER_TYPE == GL_GEOMETRY_SHADER)
	{
		Result.geometry_shader_filename = filename;
	}
	char shader_src[MAX_SHADER_SIZE];
	shader_load_source_from_file(shader_src, sizeof(shader_src), filename);

	Result.id = shader_program_create_from_src(shader_src, SHADER_TYPE);
	return(Result);
}

#if 0
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
		gl_log_shader_info(&ShaderProgram);
		ShaderProgram->reloaded = true;

		glDeleteProgram(TestShaderProgram.id);
	
	}
	else
	{
		printf("Error shader not reloaded");
	}
}
#endif

internal void
texture_set_active_and_bind(u32 index, texture_t *Texture)
{
	glActiveTexture(GL_TEXTURE0 + index);

	// TODO(Justin): Make the struct actually store the GL type
	
	glBindTexture(GL_TEXTURE_2D, Texture->id);
}

internal texture_t
texture_simple_init(char *filename, texture_type_t texture_type, b32 is_using_transparency)
{
	//TODO(Justin): Loop through app state struct and check if texture already
	//loaded.
	texture_t Texture = {};


	copy_strings(Texture.path, filename);
	//Texture.path = filename;
	Texture.type = texture_type;

	glGenTextures(1, &Texture.id);
	glBindTexture(GL_TEXTURE_2D, Texture.id);

	// TODO(Justin): Pass these in as arguements to the function? Or have the
	// texture struct itself contain this info. LAtter seems to make more sence
	// because the parameters can  be thoght of properties of the desired
	// texture.
	if(is_using_transparency)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_set_flip_vertically_on_load(true);
	Texture.memory = stbi_load(filename, &Texture.width, &Texture.height, &Texture.channel_count, 0);

	if(Texture.memory)
	{
		GLenum PIXEL_FORMAT = GL_RED;
	    if(Texture.channel_count == 3)
		{
			PIXEL_FORMAT = GL_RGB;
		}

		if(Texture.channel_count == 4)
		{
			PIXEL_FORMAT = GL_RGBA;
		}
		
		Texture.mipmap_level = 0;

		glTexImage2D(GL_TEXTURE_2D, Texture.mipmap_level, PIXEL_FORMAT,
					 Texture.width, Texture.height, 0,
					 PIXEL_FORMAT, GL_UNSIGNED_BYTE, Texture.memory);

		glGenerateMipmap(GL_TEXTURE_2D);
	}
	return(Texture);
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
	else if(glfwGetKey(Window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * E2; 
	}
	else if(glfwGetKey(Window, GLFW_KEY_A) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * Right;
	}
	else if(glfwGetKey(Window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		Camera->Pos += time_delta * Camera->speed * -E2;
	}
	else if(glfwGetKey(Window, GLFW_KEY_D) == GLFW_PRESS)
	{
		Camera->Pos += -1.0f * time_delta * Camera->speed * Right;
	}
	else if(glfwGetKey(Window, GLFW_KEY_W) == GLFW_PRESS)
	{
		Camera->Pos += 1.0f * time_delta * Camera->speed * Camera->Direction;
	}
	else if(glfwGetKey(Window, GLFW_KEY_S) == GLFW_PRESS)
	{
		Camera->Pos += -1.0f * time_delta * Camera->speed * Camera->Direction;
	}
	else if(glfwGetKey(Window, GLFW_KEY_1) == GLFW_PRESS)
	{
	}
	else if(glfwGetKey(Window, GLFW_KEY_R) == GLFW_PRESS)
	{
		//shader_program_reload(&AppState->ShaderProgram);
	}
	else if(glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
	}
	else if(glfwGetKey(Window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
	}
}


internal texture_type_t
assimp_texture_type_convert(aiTextureType texture_type)
{
	texture_type_t Result;

	// TODO(Justin): More cases
	if(texture_type == aiTextureType_DIFFUSE)
	{
		Result = TEXTURE_TYPE_DIFFUSE;
	}
	else
	{
		Result = TEXTURE_TYPE_SPECULAR;
	}
	return(Result);
}

// For each face, loop through the indices and copy them. When using GL_TRIANGULATE
// the number of indices is always 3.
internal mesh_indices_t
mesh_process_indices(aiMesh *Mesh)
{
	mesh_indices_t MeshIndices = {};

	size_t index_size = sizeof(u32);


	// NOTE(Justin): This assignmenet of indices count only works for meshes
	// composed of triangles only.
	u32 indices_count = Mesh->mNumFaces * 3;

	MeshIndices.indices = (u32 *)calloc((size_t)indices_count, index_size);
	MeshIndices.indices_count = indices_count;

	u32* DestMeshIndex = MeshIndices.indices;
	for (u32 i = 0; i < Mesh->mNumFaces; i++)
	{
		aiFace *MeshFace = &Mesh->mFaces[i];
		u32* SrcMeshIndex = MeshFace->mIndices;
		for (u32 j = 0; j < MeshFace->mNumIndices; j++)
		{
			*DestMeshIndex++ = *SrcMeshIndex++;
		}
	}
	return(MeshIndices);
}

internal mesh_vertices_t 
mesh_process_vertices(aiMesh *Mesh)
{
	mesh_vertices_t MeshVertices = {};

	u32 vertices_count = Mesh->mNumVertices;
	size_t vertex_size = sizeof(mesh_vertex_t);

	// Allocate an array of mesh_vertex_t vertices (postion, normal, and texture coordinates)
	MeshVertices.Vertices = (mesh_vertex_t*)calloc((size_t)vertices_count, vertex_size);
	MeshVertices.vertices_count = vertices_count;

	mesh_vertex_t *MeshVertex = MeshVertices.Vertices;
	for (u32 i = 0; i < vertices_count; i++)
	{
		MeshVertex->Position.x = Mesh->mVertices[i].x;
		MeshVertex->Position.y = Mesh->mVertices[i].y;
		MeshVertex->Position.z = Mesh->mVertices[i].z;

		MeshVertex->Normal.x = Mesh->mNormals[i].x;
		MeshVertex->Normal.y = Mesh->mNormals[i].y;
		MeshVertex->Normal.z = Mesh->mNormals[i].z;

		if (Mesh->mTextureCoords[0])
		{
			// TODO(Justin): This check every loop is completley redundant?
			MeshVertex->TexCoord.x = Mesh->mTextureCoords[0][i].x;
			MeshVertex->TexCoord.y = Mesh->mTextureCoords[0][i].y;
		}
		else
		{
			MeshVertex->TexCoord = glm::vec2(0.0f, 0.0f);
		}
		MeshVertex++;
	}
	return(MeshVertices);
}


internal void
mesh_process_texture_map(app_state_t *AppState, aiMaterial *MeshMaterial,
		mesh_textures_t *MeshTextures, u32 texture_count, aiTextureType texture_type, char *path_to_dir)
		//aiString path_to_texture)
{
	// NOTE(Justin): This I believe was not the problem with the model loading
	// code. The path to teh texture keeps getting overwritten.
	//texture_t* Texture = &MeshTextures->Textures[MeshTextures->texture_count];
	for(u32 i = 0; i < texture_count; i++)
	{
		texture_t Texture = {};

		aiString texture_filename;
		MeshMaterial->GetTexture(texture_type, i, &texture_filename);
		aiString path_to_texture = aiString(path_to_dir);
		path_to_texture.Append(texture_filename.C_Str());

		b32 texture_is_loaded = false;
		for(u32 j = 0; j < AppState->loaded_texture_count; j++)
		{
			const char * loaded_texture_path = AppState->LoadedTextures[j].path;
			if(loaded_texture_path)
			{
				if(strcmp(loaded_texture_path, path_to_texture.C_Str()) == 0)
				{
					// Texture already previously loaded do not load memory again, only copy data from
					// previously loaded state.
					texture_is_loaded = true;
					Texture = AppState->LoadedTextures[j];
					MeshTextures->Textures[MeshTextures->texture_count] = Texture;
					MeshTextures->texture_count++;
					break;
				}
			}
		}
		if(!texture_is_loaded)
		{
			// Texture does not exist, load the texture into the mesh array and also into the loaded 
			// textures of the app state.
			texture_type_t TextureType = assimp_texture_type_convert(texture_type);
			Texture = texture_simple_init((char *)path_to_texture.C_Str(), TextureType, 0);
			MeshTextures->Textures[MeshTextures->texture_count] = Texture;
			MeshTextures->texture_count++;
			AppState->LoadedTextures[AppState->loaded_texture_count] = Texture;
			AppState->loaded_texture_count++;
		}
		//Texture++;
	}
}

internal mesh_textures_t
mesh_process_material(app_state_t *AppState, aiMaterial *MeshMaterial, char *path_to_dir)
{
	mesh_textures_t MeshTextures = {};

	u32 texture_diffuse_count = MeshMaterial->GetTextureCount(aiTextureType_DIFFUSE);
	u32 texture_specular_count = MeshMaterial->GetTextureCount(aiTextureType_SPECULAR);


	size_t texture_size = sizeof(texture_t);
	u32 texture_count = texture_diffuse_count + texture_specular_count;

	MeshTextures.Textures = (texture_t *)calloc((size_t)texture_count, texture_size);
	//MeshTextures.texture_count = texture_count;
	//aiString path_to_texture1 = aiString("models/backpack/");
	//aiString path_to_texture = aiString();
	//path_to_texture = path_to_dir;

	mesh_process_texture_map(AppState, MeshMaterial, &MeshTextures, texture_diffuse_count, aiTextureType_DIFFUSE, 
			path_to_dir);

	mesh_process_texture_map(AppState, MeshMaterial, &MeshTextures, texture_specular_count, aiTextureType_SPECULAR,
			path_to_dir);

	return(MeshTextures);
}

internal void
node_process(app_state_t *AppState, const aiScene *Scene, aiNode *Node, model_t *Model,
		char *path_to_dir, char *vertex_shader_filename, char *geometry_shader_filename, char *fragment_shader_filename)
{
	mesh_t* ModelMesh = &Model->Meshes[Model->mesh_count];
	for (u32 i = 0; i < Node->mNumMeshes; i++)
	{
		aiMesh* Mesh = Scene->mMeshes[Node->mMeshes[i]];

		mesh_vertices_t MeshVertices = mesh_process_vertices(Mesh);
		mesh_indices_t MeshIndices = mesh_process_indices(Mesh);

		GLuint MeshVAO, MeshVBO, MeshEBO;
		glGenVertexArrays(1, &MeshVAO);
		glGenBuffers(1, &MeshVBO);
		glGenBuffers(1, &MeshEBO);

		glBindVertexArray(MeshVAO);
		glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);

		glBufferData(GL_ARRAY_BUFFER, MeshVertices.vertices_count * sizeof(mesh_vertex_t), MeshVertices.Vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MeshEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, MeshIndices.indices_count * sizeof(u32), MeshIndices.indices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void *)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void *)OffsetOfMember(mesh_vertex_t, Normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void *)OffsetOfMember(mesh_vertex_t, TexCoord));

		glBindVertexArray(0);

		//
		// NOTE(Justin): Materials
		//

		mesh_textures_t MeshTextures = {};
		if (Mesh->mMaterialIndex >= 0)
		{
			aiMaterial* MeshMaterial = Scene->mMaterials[Mesh->mMaterialIndex];
			MeshTextures = mesh_process_material(AppState, MeshMaterial, path_to_dir);
		}

		ModelMesh->MeshVBO = MeshVBO;
		ModelMesh->MeshEBO = MeshEBO;
		ModelMesh->MeshVAO = MeshVAO;

		ModelMesh->MeshVertices = MeshVertices;
		ModelMesh->MeshIndices = MeshIndices;
		ModelMesh->MeshTextures = MeshTextures;


		// TODO(Justin): System for handling shaders.

		shader_program_t MeshShader;
		MeshShader = shader_program_create_from_file(GL_VERTEX_SHADER, vertex_shader_filename);
		if(geometry_shader_filename)
		{
			MeshShader.geometry_shader_filename = geometry_shader_filename;
			shader_program_add_shader(&MeshShader, GL_GEOMETRY_SHADER, geometry_shader_filename);
		}
		shader_program_add_shader(&MeshShader, GL_FRAGMENT_SHADER, fragment_shader_filename);

		gl_log_shader_info(&MeshShader);
		ModelMesh->MeshShader = MeshShader;
		Model->mesh_count++;
	}
	for(u32 i = 0; i < Node->mNumChildren; i++)
	{
		node_process(AppState, Scene, Node->mChildren[i], Model, path_to_dir,
				vertex_shader_filename, geometry_shader_filename, fragment_shader_filename);
	}
}

// NOTE(Justin): The meshes of a model may require different shaders since
// models are usually composed of many different materials. For example the
// exterior of a car will be reflective and have specular highlights, the
// interior will be diffuse/albedo.



// TODO(Justin): Maybe it is a good idea to allocate all the memory upfront and
// then pass the model to multiple sub routines to fill out the memory? The
// allocation at this top level allocates only enough memory for the members of
// the struct. We still have to allocate memory for the other resources such as
// mesh veritces, mesh indices, mesh tectures, and so on.

internal char *
get_model_name(char *full_path_to_file)
{
	char *Result;

	u32 length = get_string_length(full_path_to_file);
	char *c = full_path_to_file + length;

	while(*c != '/')
	{
		c--;
	}
	// c gets decremented so that it points to '/' THEN the loop breaks.
	// Therefore we need to increment the pointer one time to point to the start
	// of the model name.
	
	// TODO(Justin): Return the string without the extension.
	// IDEAD twopointer one points to the beginig of the name
	// one points to the '.'. While the first pointer is not equal to the
	// second,
	// copy into a buff?
	char *model_name_with_extension = ++c;

	Result = model_name_with_extension;

	return(Result);

}

internal void
model_process(app_state_t *AppState, const char *model_filename,
		char *vertex_shader_filename, char *geometry_shader_filename, char *fragment_shader_filename)
{
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(model_filename, ASSIMP_LOAD_FLAGS);

	model_t Result;

	u32 mesh_count = Scene->mNumMeshes;
	size_t mesh_size = sizeof(mesh_t);

	Result.Meshes = (mesh_t *)calloc((size_t)mesh_count, mesh_size);
	Result.mesh_count = 0;
	Result.name = get_model_name((char *)model_filename);
	
	aiNode *Node = Scene->mRootNode;

	string_t PathToDir = get_path_to_dir((char *)model_filename);

	node_process(AppState, Scene, Node, &Result, PathToDir.data, vertex_shader_filename, geometry_shader_filename, fragment_shader_filename);
	if(mesh_count == Result.mesh_count)
	{
		AppState->Models[AppState->model_count] = Result;
		AppState->model_count++;
	}
	else
	{
	}
}

// The mesh draw funciton will probably end up being a very long function
// possibly with many parameters..

internal void
mesh_draw2(mesh_t *Mesh, shader_program_t *Shader)
{
	if (Mesh->MeshTextures.texture_count)
	{
		u32 diffuse_count = 1;
		u32 specular_count = 1;
		u32 normal_count = 1;
		u32 height_count = 1;

		char uniform_name[64];
		char digit_buff[16];
		for(u32 texture_index = 0; texture_index < Mesh->MeshTextures.texture_count; texture_index++)
		{
			texture_t* MeshTexture = Mesh->MeshTextures.Textures + texture_index;

			glActiveTexture(GL_TEXTURE0 + texture_index);

			if(MeshTexture->type == TEXTURE_TYPE_DIFFUSE)
			{
				copy_strings(uniform_name, "u_TexelDiffuse");
				digit_to_string(digit_buff, diffuse_count);
				concat_strings(uniform_name, uniform_name, digit_buff);
				diffuse_count++;
			}
			else if(MeshTexture->type == TEXTURE_TYPE_SPECULAR)
			{
				copy_strings(uniform_name, "u_TexelSpecular");
				digit_to_string(digit_buff, specular_count);
				concat_strings(uniform_name, uniform_name, digit_buff);
				specular_count++;
			}
			else if(MeshTexture->type == TEXTURE_TYPE_NORMAL)
			{
				copy_strings(uniform_name, "u_TexelNormal");
				digit_to_string(digit_buff, normal_count);
				concat_strings(uniform_name, uniform_name, digit_buff);
				normal_count++;
			}
			else if(MeshTexture->type == TEXTURE_TYPE_HEIGHT)
			{
				copy_strings(uniform_name, "u_TexelHeight");
				digit_to_string(digit_buff, height_count);
				concat_strings(uniform_name, uniform_name, digit_buff);
				height_count++;
			}
			glUniform1i(glGetUniformLocation(Shader->id, uniform_name), texture_index);
			glBindTexture(GL_TEXTURE_2D, MeshTexture->id);
		}
	}
	glBindVertexArray(Mesh->MeshVAO);
	glDrawElements(GL_TRIANGLES, Mesh->MeshIndices.indices_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}

internal void
mesh_draw(app_state_t *AppState, mesh_t *Mesh, glm::mat4 ModelTransform, glm::mat4 MapToCamera, glm::mat4 MapToPersp, glm::vec3 LightPosition)
{
	shader_program_t Shader = Mesh->MeshShader;
	glUseProgram(Shader.id);

	// TODO(Justin): Can we get the program information for the mesh shader and
	// then set the uniforms we need to set? Otherwise will need a different
	// draw call for each mesh that uses a different shader with differen inputs
	// and outputs... :(
	uniform_set_mat4f(Shader.id, "ModelTransform", ModelTransform);
	uniform_set_mat4f(Shader.id, "MapToCamera", MapToCamera);
	uniform_set_mat4f(Shader.id,"MapToPersp", MapToPersp);

	uniform_set_vec3f(Shader.id, "u_CameraPos", AppState->Camera.Pos);
	uniform_set_f32(Shader.id, "u_Material.shininess", 32.0f);

	LightPosition.x = 5.0f * cos(glfwGetTime());
	LightPosition.y = 0.0f;
	LightPosition.z = -5.0f * sin(glfwGetTime());

	uniform_set_vec3f(Shader.id, "u_LightPoint.Pos", LightPosition);
	uniform_set_vec3f(Shader.id, "u_LightPoint.Ambient", glm::vec3(0.2f, 0.2f, 0.2f));
	uniform_set_vec3f(Shader.id, "u_LightPoint.Diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
	uniform_set_vec3f(Shader.id, "u_LightPoint.Specular", glm::vec3(1.0f, 1.0f, 1.0f));

	uniform_set_f32(Shader.id, "u_LightPoint.atten_constant", 1.0f);
	uniform_set_f32(Shader.id, "u_LightPoint.atten_linear", 0.09f);
	uniform_set_f32(Shader.id, "u_LightPoint.atten_quadratic", 0.032f);

	// TODO(Justin): Loop through all the textures for each mesh and set them.
	if (Mesh->MeshTextures.texture_count)
	{
		for(u32 texture_index = 0; texture_index < Mesh->MeshTextures.texture_count; texture_index++)
		{
			texture_t* MeshTexture = Mesh->MeshTextures.Textures + texture_index;
			// TODO(Justin): What type of texture are we setting? Is it implicit
			// in MeshTexture. It is being set as GL_TEXTURE_2D for each one.
			// The type of texture should probably be store in MeshTexturee.
			texture_set_active_and_bind(texture_index, MeshTexture);
			//MeshTexture++;
			//texture_set_active_and_bind(1, MeshTexture);
		}
	}
	glBindVertexArray(Mesh->MeshVAO);
	glDrawElements(GL_TRIANGLES, Mesh->MeshIndices.indices_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0);
}



internal void
cubes_draw(app_state_t *AppState, cube_t *Cube, glm::mat4 MapToCamera, glm::mat4 MapToPersp,
												glm::vec3 LightPos, u32 cube_count, glm::vec3 *cube_positions)
{
	glUseProgram(Cube->Shader.id);

	uniform_set_mat4f(Cube->Shader.id, "MapToCamera", MapToCamera);
	uniform_set_mat4f(Cube->Shader.id,"MapToPersp", MapToPersp);

	uniform_set_vec3f(Cube->Shader.id, "u_camera_pos", AppState->Camera.Pos);
	uniform_set_vec3f(Cube->Shader.id, "u_light_point.pos", LightPos);
	uniform_set_vec3f(Cube->Shader.id, "u_light_point.ambient", glm::vec3(0.2, 0.2, 0.2));
	uniform_set_vec3f(Cube->Shader.id, "u_light_point.diffuse", glm::vec3(0.2, 0.2, 0.2));
	uniform_set_vec3f(Cube->Shader.id, "u_light_point.specular", glm::vec3(1.0, 1.0, 1.0));

	uniform_set_f32(Cube->Shader.id, "u_light_point.atten_constant", 0.2f);
	uniform_set_f32(Cube->Shader.id, "u_light_point.atten_linear", 0.01f);
	uniform_set_f32(Cube->Shader.id, "u_light_point.atten_quadratic", 0.002f);

	uniform_set_f32(Cube->Shader.id, "u_material.shininess", 32.0f);

	// NOTE(Justin): Two ways of getting at a position to render a cube at. One
	// way is to get a pointer to the array of positions. The pointer will be
	// the current position for each loop. After the end of each loop we just
	// increment this pointer to point to the next element in the array. This 
	// implementation is commented out.
	//
	// The other way is to get a pointer to a position from the array each loop
	// using the base address of the array and an offset which is the index
	// variable used in the loop. This is the current implementation.
	//
	// glm::vec3 *Pos = cube_positions;
	for(u32 position_index = 0; position_index < cube_count; position_index++)
	{
		glm::vec3 *Pos = cube_positions + position_index;

		glm::mat4 ModelTransform = glm::mat4(1.0f);

		ModelTransform = glm::translate(ModelTransform, *Pos);
		uniform_set_mat4f(Cube->Shader.id, "ModelTransform", ModelTransform);

		texture_set_active_and_bind(0, &Cube->Textures[0]);
		texture_set_active_and_bind(1, &Cube->Textures[1]);
		glBindVertexArray(Cube->VertexArray.id);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		//Pos++;
	}
}

internal cube_t
cube_create(f32 *vertices, u32 vertices_count,
		char *vertex_shader_filename, char *geometry_shader_filename, char *fragment_shader_filename,
		char *texture_diffuse_filename, char *texture_specular_filename)
{
	cube_t Result = {};

	// TODO(Justin): Get rid of these small helper functions?
	vertex_array_t VertexArray = vertex_array_create();
	vertex_buffer_t VertexBuffer = vertex_buffer_create(vertices, vertices_count * 8 * sizeof(f32));


	glBindVertexArray(VertexArray.id);
	vertex_buffer_bind(&VertexBuffer);
	vertex_buffer_layout_t VertexBufferLayout;

	// Positions
	VertexBufferLayout.element_count_per_attribute = 3;
	VertexBufferLayout.attribute_type = GL_FLOAT;
	VertexBufferLayout.normalized = GL_FALSE;
	VertexBufferLayout.size_for_each_vertex = 8 * sizeof(float);
	VertexBufferLayout.attribute_stride = (void*)0;
	vertex_array_add_buffer_layout(0, &VertexArray, &VertexBufferLayout);

	// Normals
	VertexBufferLayout.attribute_stride = (void*)(3 * sizeof(float));
	vertex_array_add_buffer_layout(1, &VertexArray, &VertexBufferLayout);

	// Texture coordinates
	VertexBufferLayout.element_count_per_attribute = 2;
	VertexBufferLayout.attribute_stride = (void*)(6 * sizeof(float));
	vertex_array_add_buffer_layout(2, &VertexArray, &VertexBufferLayout);

	texture_t TextureDiffuse = texture_simple_init(texture_diffuse_filename, TEXTURE_TYPE_DIFFUSE, 0);
	texture_t TextureSpecular = texture_simple_init(texture_specular_filename, TEXTURE_TYPE_SPECULAR, 0);

	Result.Textures[0] = TextureDiffuse;
	Result.Textures[1] = TextureSpecular;
	Result.VertexArray = VertexArray;
	Result.VertexBuffer = VertexBuffer;
	Result.VertexBufferLayout = VertexBufferLayout;

	//
	// NOTE(Justin): Shader creation.
	//

	Result.Shader = shader_program_create_from_file(GL_VERTEX_SHADER, vertex_shader_filename);
	if(geometry_shader_filename)
	{
		shader_program_add_shader(&Result.Shader, GL_GEOMETRY_SHADER, geometry_shader_filename);
	}
	shader_program_add_shader(&Result.Shader, GL_FRAGMENT_SHADER, fragment_shader_filename);

	gl_log_shader_info(&Result.Shader);

	return(Result);
}


// TODO(Justin): Rename to cube_textured_init()
internal cube_t
cube_init(f32 *vertices, u32 vertices_count,
		char *vertex_shader_filename, char *fragment_shader_filename,
		char *texture_diffuse_filename, char *texture_specular_filename)
{
	cube_t Result = {};

	// TODO(Justin): Get rid of these small helper functions?
	vertex_array_t VertexArray = vertex_array_create();
	vertex_buffer_t VertexBuffer = vertex_buffer_create(vertices, vertices_count * 8 * sizeof(f32));


	glBindVertexArray(VertexArray.id);
	vertex_buffer_bind(&VertexBuffer);
	vertex_buffer_layout_t VertexBufferLayout;

	// Positions
	VertexBufferLayout.element_count_per_attribute = 3;
	VertexBufferLayout.attribute_type = GL_FLOAT;
	VertexBufferLayout.normalized = GL_FALSE;
	VertexBufferLayout.size_for_each_vertex = 8 * sizeof(float);
	VertexBufferLayout.attribute_stride = (void*)0;
	vertex_array_add_buffer_layout(0, &VertexArray, &VertexBufferLayout);

	// Normals
	VertexBufferLayout.attribute_stride = (void*)(3 * sizeof(float));
	vertex_array_add_buffer_layout(1, &VertexArray, &VertexBufferLayout);

	// Texture coordinates
	VertexBufferLayout.element_count_per_attribute = 2;
	VertexBufferLayout.attribute_stride = (void*)(6 * sizeof(float));
	vertex_array_add_buffer_layout(2, &VertexArray, &VertexBufferLayout);

	texture_t TextureDiffuse = texture_simple_init(texture_diffuse_filename, TEXTURE_TYPE_DIFFUSE, 0);
	texture_t TextureSpecular = texture_simple_init(texture_specular_filename, TEXTURE_TYPE_SPECULAR, 0);

	Result.Textures[0] = TextureDiffuse;
	Result.Textures[1] = TextureSpecular;
	Result.VertexArray = VertexArray;
	Result.VertexBuffer = VertexBuffer;
	Result.VertexBufferLayout = VertexBufferLayout;
	Result.Shader = shader_program_create_from_files(vertex_shader_filename, fragment_shader_filename);
	gl_log_shader_info(&Result.Shader);

	return(Result);
}

internal cube_t
cube_no_textures_init(f32 *vertices, u32 vertices_count,
		char *vertex_shader_filename, char *fragment_shader_filename)
{
	cube_t Result = {};

	// TODO(Justin): Get rid of these small helper functions?
	vertex_array_t VertexArray = vertex_array_create();
	vertex_buffer_t VertexBuffer = vertex_buffer_create(vertices, vertices_count * sizeof(f32));

	glBindVertexArray(VertexArray.id);
	vertex_buffer_bind(&VertexBuffer);
	vertex_buffer_layout_t VertexBufferLayout;

	// Positions
	VertexBufferLayout.element_count_per_attribute = 3;
	VertexBufferLayout.attribute_type = GL_FLOAT;
	VertexBufferLayout.normalized = GL_FALSE;
	VertexBufferLayout.size_for_each_vertex = 6 * sizeof(float);
	VertexBufferLayout.attribute_stride = (void*)0;
	vertex_array_add_buffer_layout(0, &VertexArray, &VertexBufferLayout);

	// Normals
	VertexBufferLayout.attribute_stride = (void*)(3 * sizeof(float));
	vertex_array_add_buffer_layout(1, &VertexArray, &VertexBufferLayout);

	Result.VertexArray = VertexArray;
	Result.VertexBuffer = VertexBuffer;
	Result.VertexBufferLayout = VertexBufferLayout;
	Result.Shader = shader_program_create_from_files(vertex_shader_filename, fragment_shader_filename);
	gl_log_shader_info(&Result.Shader);

	return(Result);
}

internal skybox_t
skybox_init(char **texture_files, f32 *skybox_vertices, u32 vertices_count,
		char *vertex_shader_filename, char *fragment_shader_filename)
{
	skybox_t Result = {};

	Result.texture_files = texture_files;

	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &Result.TextureCubeMap.id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Result.TextureCubeMap.id);

	// A cubemap is assumed to always have 6 textures, one for each face.
	stbi_set_flip_vertically_on_load(false);
	for(u32 texture_index = 0; texture_index < 6; texture_index++)
	{

		Result.TextureCubeMap.memory = stbi_load(*(texture_files + texture_index),
												 &Result.TextureCubeMap.width,
												 &Result.TextureCubeMap.height,
												 &Result.TextureCubeMap.channel_count, 0);
		if(Result.TextureCubeMap.memory)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + texture_index,
						0, GL_RGB, 
						Result.TextureCubeMap.width, 
						Result.TextureCubeMap.height,
						0, GL_RGB, 
						GL_UNSIGNED_BYTE, 
						Result.TextureCubeMap.memory);

			stbi_image_free(Result.TextureCubeMap.memory);
		}
		else
		{
		}
	}
	stbi_set_flip_vertically_on_load(true);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	Result.VertexArray = vertex_array_create();

	// WARNING(Justin): NASTY bug here I was passing in &skybox_vertices which is actually **float NOT *float and
	// *float is the correct arguement type. Result was black textures for
	// skybox
	Result.VertexBuffer = vertex_buffer_create(skybox_vertices, vertices_count * sizeof(f32));

	glBindVertexArray(Result.VertexArray.id);
	vertex_buffer_bind(&Result.VertexBuffer);

	Result.VertexBufferLayout.element_count_per_attribute = 3;
	Result.VertexBufferLayout.attribute_type = GL_FLOAT;
	Result.VertexBufferLayout.normalized = GL_FALSE;
	Result.VertexBufferLayout.size_for_each_vertex = 3 * sizeof(f32);
	Result.VertexBufferLayout.attribute_stride = (void *)0;

	vertex_array_add_buffer_layout(0, &Result.VertexArray, &Result.VertexBufferLayout);

	Result.Shader.vertex_shader_filename = vertex_shader_filename;
	Result.Shader.fragment_shader_filename = fragment_shader_filename;
	Result.Shader = shader_program_create_from_files(Result.Shader.vertex_shader_filename, 
													 Result.Shader.fragment_shader_filename);
	gl_log_shader_info(&Result.Shader);

	return(Result);
}


internal line_t
line_create_from_two_points(glm::vec3 P1, glm::vec3 P2)
{
	line_t Result = {};

	Result.P1 = P1;
	Result.P2 = P2;

	glGenVertexArrays(1, &Result.VAO);
	glGenBuffers(1, &Result.VBO);

	glBindVertexArray(Result.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, Result.VBO);

	// NOTE(Justin): The buffering of the data works as follows. The first two
	// members of the line_t struct are the glm::vec3 P1 and P2 vertex
	// positions. The opengl buffer just needs to be big enough to store these
	// two vertices. Since these are the first two members we can give the
	// address of the first vertex and an offset to the next to correctly
	// describe the data that is needing to be copied to the buffer.

	glBufferData(GL_ARRAY_BUFFER, 2 * sizeof(Result.P1), &Result.P1, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	Result.Shader = shader_program_create_from_files("shaders/line.vs", "shaders/line.fs");
	gl_log_shader_info(&Result.Shader);

	return(Result);
}

internal void
line_draw(line_t *Line, glm::mat4 ModelTransform, glm::mat4 MapToCamera, glm::mat4 MapToPersp, glm::vec3 Color)
{
	glUseProgram(Line->Shader.id);

	ModelTransform = glm::mat4(1.0f);
	uniform_set_mat4f(Line->Shader.id, "ModelTransform", ModelTransform);
	uniform_set_mat4f(Line->Shader.id, "MapToCamera", MapToCamera);
	uniform_set_mat4f(Line->Shader.id,"MapToPersp", MapToPersp);
	uniform_set_vec3f(Line->Shader.id, "LineColor", Color);

	glBindVertexArray(Line->VAO);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

internal quad_t
quad_create(f32 *vertices, u32 *indices, char *texture_filename, texture_type_t TEXTURE_TYPE,
		char *vertex_shader_filename, char *fragment_shader_filename, b32 is_using_transparency)
{
	quad_t Result = {};

	glGenVertexArrays(1, &Result.VAO);
	glGenBuffers(1, &Result.EBO);
	glGenBuffers(1, &Result.VBO);

	glBindVertexArray(Result.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, Result.VBO);

	// NOTE(Justin): Only attributes: positions and tex coords.
	// Also using EBO to do indexed rendering.

	glBufferData(GL_ARRAY_BUFFER, 4 * 5 * sizeof(f32), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Result.EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(u32), indices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(f32), (void *)(3 * sizeof(f32)));

	Result.Texture = texture_simple_init(texture_filename, TEXTURE_TYPE, is_using_transparency);
	//Result.Shader = shader_program_create_from_files(vertex_shader_filename, fragment_shader_filename);
	Result.Shader = shader_program_create_from_file(GL_VERTEX_SHADER, vertex_shader_filename);
	shader_program_add_shader(&Result.Shader, GL_FRAGMENT_SHADER, fragment_shader_filename);

	gl_log_shader_info(&Result.Shader);

	return(Result);
}

internal void GLAPIENTRY
opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, 
																	const GLchar *message, const void *userParam)
{
	//char *gl_source = gl_enum_type_to_string(source);
	//char *gl_type = gl_enum_type_to_string(type);
	//char *gl_severity = gl_enum_type_to_string(severity);

	printf("[OpenGL Debug Callback] %s\n", message);
}

internal f32
get_asteroid_displacement(f32 offset)
{
	f32 Result = 0.0f;
	Result = ((rand() % (s32)(2 * offset * 100)) / 100.0f) - offset;
	return(Result);

}

internal model_t
model_get(app_state_t *AppState, char *model_name)
{
	model_t Result = {};

	for(u32 model_index = 0; model_index < AppState->model_count; model_index++)
	{
		model_t Model = AppState->Models[model_index];
		if(strings_are_same(Model.name, model_name))
		{
			Result = Model;
		}
	}
	return(Result);
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

	// TODO(Justin): Need a build option to for debug build. The debug context is usefule but slow
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
	glfwSetMouseButtonCallback(Window.handle, glfw_mouse_button_callback);

	gl_log_params();

	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		printf("Oops\n");
	}

	// TODO(Justin): OpenGL info struct
	const GLubyte *renderer_name = glGetString(GL_RENDERER);
	const GLubyte* renderer_version = glGetString(GL_VERSION);
	printf("Renderer = %s\n", renderer_name);
	printf("Renderer version = %s\n", renderer_version);

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(&opengl_debug_callback, 0);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	app_state_t AppState = {};

	//
	// NOTE(Justin): glBufferSubData and geometry shader example
	//

	shader_program_t HousesShader = shader_program_create_from_file(GL_VERTEX_SHADER, "shaders/build_house_at_point.vs");
	shader_program_add_shader(&HousesShader, GL_GEOMETRY_SHADER, "shaders/build_house_at_point.gs");
	shader_program_add_shader(&HousesShader, GL_FRAGMENT_SHADER, "shaders/build_house_at_point.fs");

	points_ndc_t PointsNDC;

	// Careful with the semantics of these variables they are not technically
	// position counts and color counts they are counts of how many f32s are in
	// the array. An actual position count would be the number of f32s in the array 
	// divided by 2. Since an ndc pos consists of 2 floats.

	PointsNDC.positions = &ndc_space_positions[0];
	PointsNDC.positions_count = ArrayCount(ndc_space_positions);
	PointsNDC.colors = &ndc_space_colors[0];
	PointsNDC.colors_count = ArrayCount(ndc_space_colors);

	glGenVertexArrays(1, &PointsNDC.VAO);
	glGenBuffers(1, &PointsNDC.VBO);
	glBindVertexArray(PointsNDC.VAO);
	glBindBuffer(GL_ARRAY_BUFFER, PointsNDC.VBO);

	// To write to part of a buffer using glBufferSubData, we first need a buffer to store the data.
	// Call glBufferData, give it the correct size, and pass NULL as the pointer
	// to the data. This allocates  GPU memory for the currently bound buffer.
	// We can write to it now or bind later and write to it.

	glBufferData(GL_ARRAY_BUFFER, (PointsNDC.positions_count + PointsNDC.colors_count) * sizeof(f32), 
			NULL, GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER, 0, PointsNDC.positions_count * sizeof(f32), PointsNDC.positions);
	glBufferSubData(GL_ARRAY_BUFFER, PointsNDC.positions_count * sizeof(f32),
									 PointsNDC.colors_count * sizeof(f32), PointsNDC.colors);

	// AS long as we correctly specify the format of the data in the buffer (how
	// the data looks) and give it a ! index then we can draw properly. The
	// stride is now the size of the attribute since attribuites of the same
	// type are tightly packed. they are laid out in memory consecutively. When
	// we need to specify a pointer to the next attribute the stride is the size
	// of the next attribute but the starting offset is now an offset that is
	// the size of the entire data associated with the first attribute.
	//
	// interleaved attributes before: xyrgb1 xyrgb2 xyrgb3 ... xyrgbn
	//
	// consecutive attributes after:  xy1 xy2 xy3 ... xyn rgb1 rgb2 rgb3 ... rgbn

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void *)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)(PointsNDC.positions_count * sizeof(f32)));

	f32 time_delta = 0.0f;
	f32 time_previous = (f32)glfwGetTime();
	while (!glfwWindowShouldClose(Window.handle))
	{
		glfw_process_input(Window.handle, &AppState, time_delta);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//
		// NOTE(Justin): Render
		//

		glUseProgram(HousesShader.id);
		glBindVertexArray(PointsNDC.VAO);
		glDrawArrays(GL_POINTS, 0, 4);

        glfwPollEvents();
        glfwSwapBuffers(Window.handle);

		f32 time_current = (f32)glfwGetTime();
		time_delta = time_current - time_previous;
		time_previous = time_current;
	}
	glfwTerminate();
	return 0;
}
