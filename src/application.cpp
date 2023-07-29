/*
TODO:
 - Vertex Buffers
	Lock Modify Unlock
	Set data
	streaming
	.
	.
	. 

 - Picking Cache for
	Selecting object
		Mouse point to object
		Click object
		Send ray to see if it interseects
	
 - Testing framework
 - Memory arena

*/

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
#include "application_util.cpp"
#include "application_cube.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

global_variable input_t AppInput;

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
	glVertexAttribPointer(index, VertexBufferLayout->element_count_per_attribute,
			VertexBufferLayout->attribute_type, VertexBufferLayout->normalized,
			VertexBufferLayout->size_for_each_vertex, VertexBufferLayout->attribute_stride);

	glEnableVertexAttribArray(index);
	// TODO(Justin): Should we pass in an index as opposed to storing it in the vertex_array_t struct?
	//glEnableVertexAttribArray(VertexArray->attribute_index);
	//VertexArray->attribute_index++;
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




internal GLuint
shader_program_create_from_strings(char *vertex_shader_str, char *fragment_shader_str)
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
		// TODO(Justin): The filename is probably going to be more useful here
		// instead of just a number...
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
shader_program_create_from_files(char *vertex_shader_filename, char *fragment_shader_filename)
{
	// Check to make sure extensions are correct?
	// Global shader to default too, instead of assert then crash?

	ASSERT(vertex_shader_filename && fragment_shader_filename);

	shader_program_t Result = {};

	Result.vertex_shader_filename = vertex_shader_filename;
	Result.fragment_shader_filename = fragment_shader_filename;

	char vertex_shader_src[1024];
	char fragment_shader_src[MAX_SHADER_SIZE];

	vertex_shader_src[0] = fragment_shader_src[0] = '\0';

	FILE* VertexShaderFileHandle = fopen(vertex_shader_filename, "r");
	if (VertexShaderFileHandle)
	{
		size_t count = fread(vertex_shader_src, 1, sizeof(vertex_shader_src), VertexShaderFileHandle);
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
texture_simple_init(char *filename, texture_type_t texture_type)
{
	//TODO(Justin): Loop through app state struct and check if texture already
	//loaded.
	texture_t Texture = {};

	Texture.path = filename;
	Texture.type = texture_type;

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
uniform_set_b32(GLuint program_id, char *uniform_name, b32 value)
{
	// TODO(Justin): If these function calls!!!
	glUniform1i(glGetUniformLocation(program_id, uniform_name), (int)value);
}

internal void
uniform_set_s32(GLuint program_id, char *uniform_name, s32 value)
{
	glUniform1i(glGetUniformLocation(program_id, uniform_name), value);
}

internal void
uniform_set_f32(GLuint program_id, char *uniform_name, f32 value)
{
	glUniform1f(glGetUniformLocation(program_id, uniform_name), value);
}

internal void
uniform_set_mat4f(GLuint shader_program_id, char *uniform_name, glm::mat4 Transform)
{
	u32 transform_location = glGetUniformLocation(shader_program_id, uniform_name);
	glUniformMatrix4fv(transform_location, 1, GL_FALSE, glm::value_ptr(Transform));
}

internal void
uniform_set_v3f(GLuint shader_program_id, char *uniform_name, f32 x, f32 y, f32 z)
{
	glUniform3f(glGetUniformLocation(shader_program_id, uniform_name), x, y, z);
}

internal void
uniform_set_vec3f(GLuint shader_program_id, char *uniform_name, glm::vec3 V)
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
		//shader_program_reload(&AppState->ShaderProgram);
	}
	else if (glfwGetKey(Window, GLFW_KEY_RIGHT) == GLFW_PRESS)
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
	//u32 index_buffer_memory_size = index_size * indices_count;

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
	//u32 vertex_buffer_memory_size = vertex_size * vertices_count;

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
		mesh_textures_t *MeshTextures, u32 texture_count, aiTextureType texture_type,
		aiString path_to_texture)
{
	texture_t* Texture = &MeshTextures->Textures[MeshTextures->texture_count];
	for(u32 i = 0; i < texture_count; i++)
	{
		aiString texture_filename;
		MeshMaterial->GetTexture(texture_type, i, &texture_filename);
		path_to_texture.Append(texture_filename.C_Str());

		b32 texture_is_loaded = false;
		for(u32 j = 0; j < AppState->loaded_texture_count; j++)
		{
			char *loaded_texture_path = AppState->LoadedTextures[j].path;
			if(loaded_texture_path)
			{
				if(strcmp(loaded_texture_path, path_to_texture.C_Str()) == 0)
				{
					// Texture already previously loaded do not load memory again, only copy data from
					// previously loaded state.
					texture_is_loaded = true;
					*Texture = AppState->LoadedTextures[j];
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
			*Texture = texture_simple_init((char *)path_to_texture.C_Str(), TextureType);
			MeshTextures->texture_count++;
			AppState->LoadedTextures[AppState->loaded_texture_count] = *Texture;
			AppState->loaded_texture_count++;
		}
		Texture++;
	}
}

internal mesh_textures_t
mesh_process_material(app_state_t *AppState, aiMaterial *MeshMaterial)
{
	mesh_textures_t MeshTextures = {};

	u32 texture_diffuse_count = MeshMaterial->GetTextureCount(aiTextureType_DIFFUSE);
	u32 texture_specular_count = MeshMaterial->GetTextureCount(aiTextureType_SPECULAR);

	size_t texture_size = sizeof(texture_t);
	u32 texture_count = texture_diffuse_count + texture_specular_count;
	//u32 texture_memory_size = texture_count * texture_size;

	MeshTextures.Textures = (texture_t*)calloc((size_t)texture_count, texture_size);

	aiString path_to_texture = aiString("models/backpack/");

	mesh_process_texture_map(AppState, MeshMaterial, &MeshTextures, texture_diffuse_count, aiTextureType_DIFFUSE, 
			path_to_texture);

	mesh_process_texture_map(AppState, MeshMaterial, &MeshTextures, texture_specular_count, aiTextureType_SPECULAR,
			path_to_texture);

	return(MeshTextures);
}

// TODO(Justin): Need to remove hardcoded inputs.
internal void
node_process(app_state_t *AppState, const aiScene *Scene, aiNode *Node, model_t *Model,
		char *vertex_shader_filename, char *fragment_shader_filename)
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
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void*)OffsetOfMember(mesh_vertex_t, Normal));

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(mesh_vertex_t), (void*)OffsetOfMember(mesh_vertex_t, TexCoord));

		glBindVertexArray(0);

		//
		// NOTE(Justin): Materials
		//

		mesh_textures_t MeshTextures = {};
		if (Mesh->mMaterialIndex >= 0)
		{
			aiMaterial* MeshMaterial = Scene->mMaterials[Mesh->mMaterialIndex];
			MeshTextures = mesh_process_material(AppState, MeshMaterial);
		}

		ModelMesh->MeshVBO = MeshVBO;
		ModelMesh->MeshEBO = MeshEBO;
		ModelMesh->MeshVAO = MeshVAO;

		ModelMesh->MeshVertices = MeshVertices;
		ModelMesh->MeshIndices = MeshIndices;
		ModelMesh->MeshTextures = MeshTextures;

		shader_program_t MeshShader;
		MeshShader.vertex_shader_filename = vertex_shader_filename;
		MeshShader.fragment_shader_filename = fragment_shader_filename;
		MeshShader = shader_program_create_from_files(MeshShader.vertex_shader_filename,
													  MeshShader.fragment_shader_filename);
		gl_log_shader_info(&MeshShader);

		ModelMesh->MeshShader = MeshShader;
		
		Model->mesh_count++;

		// free memory allocated?
	}
	for(u32 i = 0; i < Node->mNumChildren; i++)
	{
		node_process(AppState, Scene, Node->mChildren[i], Model, vertex_shader_filename, fragment_shader_filename);
	}
}

// NOTE(Justin): Should this return a model?
// NOTE(Justin): The meshes of a model may require different shaders?
internal void
model_process(app_state_t *AppState, char *full_path_to_model, char *vertex_shader_filename,
															   char *fragment_shader_filename)
{
	Assimp::Importer Importer;
	const aiScene* Scene = Importer.ReadFile(full_path_to_model, ASSIMP_LOAD_FLAGS);

	model_t Result;

	u32 mesh_count = Scene->mNumMeshes;
	size_t mesh_size = sizeof(mesh_t);

	Result.Meshes = (mesh_t *)calloc((size_t)mesh_count, mesh_size);
	Result.mesh_count = 0;
	
	aiNode *Node = Scene->mRootNode;

	node_process(AppState, Scene, Node, &Result, vertex_shader_filename, fragment_shader_filename);

	if(mesh_count == Result.mesh_count)
	{
		AppState->Models[AppState->model_count] = Result;
		AppState->model_count++;
	}
	else
	{
	}
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
	if(Mesh->MeshTextures.texture_count)
	{
		for(u32 texture_index = 0; texture_index < Mesh->MeshTextures.texture_count; texture_index++)
		{
			texture_t* MeshTexture = Mesh->MeshTextures.Textures + texture_index;
			// TODO(Justin): What type of texture are we setting? Is it implicit
			// in MeshTexture. It is being set as GL_TEXTURE_2D for each one.
			// The type of texture should probably be store in MeshTexturee.

			texture_set_active_and_bind(texture_index, MeshTexture);
		}
	}
	glBindVertexArray(Mesh->MeshVAO);
	glDrawElements(GL_TRIANGLES, Mesh->MeshIndices.indices_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
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


	glm::vec3 *Pos = cube_positions;
	for(u32 position_index = 0; position_index < cube_count; position_index++)
	{
		glm::mat4 ModelTransform = glm::mat4(1.0f);

		ModelTransform = glm::translate(ModelTransform, *Pos);
		uniform_set_mat4f(Cube->Shader.id, "ModelTransform", ModelTransform);

		texture_set_active_and_bind(0, &Cube->Textures[0]);
		texture_set_active_and_bind(1, &Cube->Textures[1]);
		glBindVertexArray(Cube->VertexArray.id);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);

		Pos++;
	}
}

internal cube_t
cube_init(f32 *vertices, u32 vertices_count,
		char *vertex_shader_filename, char *fragment_shader_filename,
		char *texture_diffuse_filename, char *texture_specular_filename)
{
	cube_t Result = {};

	if(texture_diffuse_filename)
	{
		texture_t TextureDiffuse = texture_simple_init(texture_diffuse_filename, TEXTURE_TYPE_DIFFUSE);
		Result.Textures[0] = TextureDiffuse;
	}
	if(texture_specular_filename)
	{
		texture_t TextureSpecular = texture_simple_init(texture_specular_filename, TEXTURE_TYPE_SPECULAR);
		Result.Textures[1] = TextureSpecular;
	}

	vertex_array_t VertexArray = vertex_array_create();
	vertex_buffer_t VertexBuffer = vertex_buffer_create(vertices, vertices_count * sizeof(f32));

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
	for (u32 texture_index = 0; texture_index < 6; texture_index++)
	{

		Result.TextureCubeMap.memory = stbi_load(*(texture_files + texture_index),
												 &Result.TextureCubeMap.width,
												 &Result.TextureCubeMap.height,
												 &Result.TextureCubeMap.channel_count, 0);
		if (Result.TextureCubeMap.memory)
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void *)0);

	glEnableVertexAttribArray(0);
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

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);


	app_state_t AppState = {};

	//
	// NOTE(Justin): Model Initalization
	//

	// TODO(Justin): Still need to remove hardcoded values in the model loading
	// sub routines.
	//model_process(&AppState, "models/backpack/backpack.obj", "shaders/backpack.vs", "shaders/backpack.fs");
	//model_process(&AppState, "models/suzanne.obj", "shaders/suzanne.vs", "suzanne.fs");
	

	line_t Line = line_create_from_two_points(E1, E2);
	line_t Line1 = line_create_from_two_points(E1, -E2);



	cube_t Cube = cube_init(&cube_vertices_normals_and_tex_coods[0],
							ArrayCount(cube_vertices_normals_and_tex_coods),
							"shaders/cube_light_casters.vs", "shaders/cube_light_casters.fs",
							"textures/marble.jpg", "");
#if 0
	cube_t Cube = cube_init(&cube_vertices_normals_and_tex_coods[0],
							ArrayCount(cube_vertices_normals_and_tex_coods),
							"shaders/cube_light_casters.vs", "shaders/cube_light_casters.fs",
							"textures/container2.png", "textures/container2_specular.png");
#endif

	// Lamp
	vertex_array_t LightVertexArray = vertex_array_create();

	glBindVertexArray(LightVertexArray.id);
	vertex_buffer_bind(&Cube.VertexBuffer);

	// Position
	Cube.VertexBufferLayout.element_count_per_attribute = 3;
	Cube.VertexBufferLayout.attribute_stride = (void*)0;
	vertex_array_add_buffer_layout(0, &LightVertexArray, &Cube.VertexBufferLayout);

	//
	// NOTE(Justin): Texture initialization
	//

	texture_t TextureContainerMarble = texture_simple_init("textures/marble.png", TEXTURE_TYPE_DIFFUSE);
	texture_t TexturePlaneMetal = texture_simple_init("textures/metal.png", TEXTURE_TYPE_SPECULAR);
	texture_t TextureContainerDiffuse = texture_simple_init("textures/container2.png", TEXTURE_TYPE_DIFFUSE);
	texture_t TextureContainerSpecular = texture_simple_init("textures/container2_specular.png", TEXTURE_TYPE_SPECULAR);
	texture_t TextureGrass = texture_simple_init("textures/grass.png", TEXTURE_TYPE_DIFFUSE);

	char *skybox_texture_files[] =  
	{
		"textures/skybox/right.jpg",
		"textures/skybox/left.jpg",
		"textures/skybox/top.jpg",
		"textures/skybox/bottom.jpg",
		"textures/skybox/front.jpg",
		"textures/skybox/back.jpg",

	};

	skybox_t SkyBox = skybox_init(skybox_texture_files, &skybox_vertices[0], ArrayCount(skybox_vertices),
		"shaders/skybox.vs", "shaders/skybox.fs");

	shader_program_t LightShader;

	char *light_vertex_shader_filename = "shaders/light_001.vs";
	char *light_fragment_shader_filename = "shaders/light_001.fs";

	LightShader = shader_program_create_from_files(light_vertex_shader_filename, light_fragment_shader_filename);

	gl_log_shader_info(&LightShader);

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

	AppState.Camera = Camera;
	AppState.LightShader = LightShader;
	
	//
	// NOTE(Justin): Transforms and Positions.
	//

	glm::mat4 MapToCamera = glm::lookAt(AppState.Camera.Pos, AppState.Camera.Pos + AppState.Camera.Direction, AppState.Camera.Up);
	MapToCamera = glm::translate(MapToCamera, glm::vec3(0.0f, 0.0f, -3.0f));

	f32 field_of_view = glm::radians(45.0f);
	f32 aspect_ratio = (f32)Window.width / (f32)Window.height;
	f32 n = 0.1f;
	f32 f = 100.0f;

	glm::mat4 MapToPersp = glm::perspective(field_of_view, aspect_ratio, n, f);

	glm::vec3 LightPositions[4];
    glm::vec3 offset = glm::vec3(1.0f, 1.0f, 1.0f);

	LightPositions[0] = cube_positions[0];

	glUseProgram(Cube.Shader.id);
	uniform_set_s32(Cube.Shader.id, "u_material.diffuse", 0);
	uniform_set_s32(Cube.Shader.id, "u_material.specular", 1);


	for(u32 MeshIndex = 0; MeshIndex < AppState.Models[0].mesh_count; MeshIndex++)
	{
		mesh_t *Mesh = AppState.Models[0].Meshes + MeshIndex;
		glUseProgram(Mesh->MeshShader.id);
		uniform_set_s32(Mesh->MeshShader.id, "u_Material.Diffuse1", 0);
		uniform_set_s32(Mesh->MeshShader.id, "u_Material.Specular1", 1);
	}

	glUseProgram(SkyBox.Shader.id);
	uniform_set_s32(SkyBox.Shader.id, "SkyBox", 0);
	

	f32 time_delta = 0.0f;
	f32 time_previous = (f32)glfwGetTime();
	while (!glfwWindowShouldClose(Window.handle))
	{
		glfw_process_input(Window.handle, &AppState, time_delta);

		//
		// NOTE(Justin): Render
		//

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//
		// NOTE(Justin): SkyBox
		//

		glDepthMask(GL_FALSE);
		glUseProgram(SkyBox.Shader.id);
		glActiveTexture(GL_TEXTURE0);

		MapToCamera = glm::mat4(glm::mat3(MapToCamera));
		uniform_set_mat4f(SkyBox.Shader.id, "MapToCamera", MapToCamera);
		uniform_set_mat4f(SkyBox.Shader.id,"MapToPersp", MapToPersp);

		GLCall(glBindVertexArray(SkyBox.VertexArray.id));
		GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, SkyBox.TextureCubeMap.id));

		GLCall(glDrawArrays(GL_TRIANGLES, 0, 36));
		glDepthMask(GL_TRUE);


		MapToCamera = glm::lookAt(AppState.Camera.Pos, AppState.Camera.Pos + AppState.Camera.Direction, AppState.Camera.Up);
		
		for (u32 model_index = 0; model_index < AppState.model_count; model_index++)
		{
			glm::mat4 ModelTransform = glm::mat4(1.0f);
			ModelTransform = glm::translate(ModelTransform, glm::vec3(cube_positions[model_index]));
			model_t Model = AppState.Models[model_index];
			for(u32 mesh_index = 0; mesh_index < Model.mesh_count; mesh_index++)
			{
				mesh_t ModelMesh = Model.Meshes[mesh_index];
				mesh_draw(&AppState, &ModelMesh, ModelTransform, MapToCamera, MapToPersp, LightPositions[0]);
			}
		}

		LightPositions[0].x = 5.0f * cos(glfwGetTime());
		LightPositions[0].y = 0.0f;
		LightPositions[0].z = -5.0f * sin(glfwGetTime());
		
		cubes_draw(&AppState, &Cube, MapToCamera, MapToPersp, LightPositions[0], ArrayCount(cube_positions),
																								cube_positions);

		//
		// NOTE(Justin): Lamp
		//


		glUseProgram(AppState.LightShader.id);

		// NOTE that we are updating and setting the global unifrom light position above which is why the cube is transformed accordingly
		// it is because we have alrady calculated the lamps new position above and are just using it here
		glm::mat4 ModelTransform = glm::mat4(1.0f);

		LightPositions[0].x = 5.0f * cos(glfwGetTime());
		LightPositions[0].y = 0.0f;
		LightPositions[0].z = -5.0f * sin(glfwGetTime());

		ModelTransform = glm::translate(ModelTransform, LightPositions[0]);
		glm::vec3 RotateXZ = glm::vec3(5.0f * cos(glfwGetTime()), 0.0f, -5.0f * sin(glfwGetTime()));
		ModelTransform = glm::translate(ModelTransform, RotateXZ);

		ModelTransform = glm::scale(ModelTransform, glm::vec3(0.2f));

		uniform_set_mat4f(AppState.LightShader.id, "ModelTransform", ModelTransform);
		uniform_set_mat4f(AppState.LightShader.id, "MapToCamera", MapToCamera);
		uniform_set_mat4f(AppState.LightShader.id,"MapToPersp", MapToPersp);

		glBindVertexArray(LightVertexArray.id);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//
		// NOTE(Jusint): Line
		//

		line_draw(&Line, ModelTransform, MapToCamera, MapToPersp, E2);
		line_draw(&Line1, ModelTransform, MapToCamera, MapToPersp, E2);

        glfwPollEvents();
        glfwSwapBuffers(Window.handle);

		f32 time_current = (f32)glfwGetTime();
		time_delta = time_current - time_previous;
		time_previous = time_current;
	}
	glfwTerminate();
	return 0;
}
