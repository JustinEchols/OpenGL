#if !defined(APPLICATION_H)

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdarg.h>
#include <time.h>

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

#define internal static;
#define global_variable static;
#define local_persist static;

#define ASSERT(expression) if((!expression)) {*(int *)0 = 0;}
//#define ASSERT(expression) if((!expression)) __debugbreak()
#define ARRAY_COUNT(a) (sizeof(a) / sizeof(a[0]))

#define GL_LOG_FILE "gl.log"
#define E1 glm::vec3(1.0f, 0.0f, 0.0f)
#define E2 glm::vec3(0.0f, 1.0f, 0.0f)
#define E3 glm::vec3(0.0f, 0.0f, 1.0f)
#define PI32 3.141592653589f
#define MAX_SHADER_SIZE 100000

typedef struct
{
	glm::vec3 Pos;
	glm::vec3 Direction;
	glm::vec3 Up;

	f32 speed;

} camera_t;

typedef struct
{
	glm::vec3 Pos;
	glm::vec3 Color;
} light_t;


typedef struct
{
	b32 is_down;
} key_t;

typedef struct
{
	glm::vec2 Pos;
	f32 sensitivity;
	b32 is_initialized;
} mouse_t;

enum
{
	KEY_W,
	KEY_A,
	KEY_S,
	KEY_D,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,

	KEY_COUNT,
};

typedef struct 
{
	f32 pitch, yaw;
	key_t Keys[KEY_COUNT];
	mouse_t Mouse;
} input_t;

typedef struct
{
	GLuint id;
	b32 binded;
	void *memory;
	u32 size;
} vertex_buffer_t;

typedef struct
{
	// TODO(Justin): attribute_count?
	u32 element_count_per_attribute;
	GLenum attribute_type;
	GLenum normalized;
	u32 size_for_each_vertex;
	void* attribute_stride;
} vertex_buffer_layout_t;

typedef struct
{
	GLuint id;
	vertex_buffer_layout_t VertexBufferLayout;

} vertex_array_t;

// TODO(Justin): index buffer/ element array buffer...j
typedef struct
{
	u32* indices;
	u32 count;
} index_buffer_t;



// TODO(Justin): Not sure where the attribute_count should be. In a the vertex
// buffer layout or in the mesh vertex definition. 
typedef struct
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoord;
} mesh_vertex_t;



typedef struct
{
	GLuint id;
	const char *vertex_shader_filename;
	const char *fragment_shader_filename;
	b32 reloaded;
} shader_program_t;

typedef struct
{
	GLFWwindow* handle;
	s32 width, height;
} window_t;

enum texture_type_t
{
	TEXTURE_TYPE_DIFFUSE,
	TEXTURE_TYPE_SPECULAR,

	TEXTURE_TYPE_COUNT
};

typedef struct
{
	GLuint id;
	s32 width, height;
	s32 channel_count;
	u32 mipmap_level;
	u8* memory;
	const char* path;
	texture_type_t type;
} texture_t;

typedef struct
{
	u32 diffuse_maps_count;
	u32 specular_maps_count;

	texture_t *DiffuseMaps;
	texture_t *SpecularMaps;
} material_t;

typedef struct
{
	u32 vertices_count;
	u32 indices_count;
	u32 texture_count;
	//u32 texture_diffuse_count;
	//u32 texture_specular_count;

	u32 *Indices;
	mesh_vertex_t *Vertices;
	texture_t *Textures;
	//texture_t *TexturesDiffuse;
	//texture_t *TexturesSpecular; 
	
	// TODO(Justin): Each mesh has a shader reason why is because each mesh has
	// a different set of data attached to it?
} mesh_t;

typedef struct
{
	// TODO(Justin): Implement this.
	u32 mesh_count;
	mesh_t *Meshes;
} model_t;


typedef struct
{
	// Count or index??
	u32 loaded_texture_count;
	texture_t LoadedTextures[16];
	shader_program_t BackPackShader;
	shader_program_t LightShader;
	shader_program_t CubeShader;
	camera_t Camera;
} app_state_t;

#define APPLICATION_H
#endif
