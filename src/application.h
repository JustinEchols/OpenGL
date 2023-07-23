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

#define GL_LOG_FILE "gl.log"
#define E1 glm::vec3(1.0f, 0.0f, 0.0f)
#define E2 glm::vec3(0.0f, 1.0f, 0.0f)
#define E3 glm::vec3(0.0f, 0.0f, 1.0f)
#define PI32 3.141592653589f
#define MAX_SHADER_SIZE 100000

#define ASSERT(expression) if((!expression)) {*(int *)0 = 0;}
//#define ASSERT(expression) if((!expression)) __debugbreak()
#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))

#define GLCall(gl_func) gl_clear_errors();\
	gl_func;\
	ASSERT(GLLogCall(#gl_func, __FILE__, __LINE__))

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

#define IntFromPtr(ptr) (unsigned long long)((char *)ptr - (char *)0)
#define Member(Type, member) (((Type*)0)->member)
#define OffsetOfMember(Type,member) IntFromPtr(&Member(Type,member))

typedef enum
{
	Standard,
	Foliage,
	Lake,
	Reflective,
	Blended,
	Distant,
	Refract,
	Translucent,
	ShadowOnly,
	Tinted,
	Vegetation,
	Occluder,
	CollisionOnly,
	Cloud,
	Laser,
} material_type_t;

typedef struct
{
	glm::vec3 Tangent;
	glm::vec3 BiTangent;
	glm::vec3 Normal;
} local_frame_t;

typedef struct
{
	glm::vec3 Min;
	glm::vec3 Max;
} bounding_box_t;

typedef enum
{
	CastsShadow,
	LightMapped,
	UnderWater,
	VertexLightMap,
	Ground,
	Solid,
	Walkable,
	WindAnimation,
	TranslucentUseEnvironmentMap,
	TranslucentEnvironmentMapIsFiltered,
	TranslucentSortMeshByCentroid,
	TranslucentHasVertexColors,

} material_flags_t;


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
	u32 vertices_count;
	mesh_vertex_t *Vertices;
} mesh_vertices_t;

typedef struct
{
	u32 indices_count;
	u32* indices;
} mesh_indices_t;


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
	vertex_array_t VertexArray;
	vertex_buffer_t VertexBuffer;
	vertex_buffer_layout_t VertexBufferLayout;

	glm::vec3 Pos;
} cube_t;

typedef struct
{
	u32 texture_count;
	texture_t* Textures;
} mesh_textures_t;


// TODO(Justin): Eventually we may want to individually classify each
// individual mesh for a model to do animations and so on. Therefore 
// Do we need to generate like an enum for each model we load that refers
// to a particular mesh of the model Backpack[TORCH].MeshVertices
typedef struct
{
	GLuint MeshVBO, MeshVAO, MeshEBO;

	mesh_vertices_t MeshVertices;
	mesh_indices_t  MeshIndices;
	mesh_textures_t MeshTextures;

	shader_program_t MeshShader;

	// TODO(Justin): Each mesh has a shader reason why is because each mesh has
	// a different set of data attached to it?
} mesh_t;

typedef struct
{
	glm::vec3 Pos;
	u32 mesh_count;
	mesh_t *Meshes;
} model_t;

typedef struct
{
	texture_t TextureCubeMap;
	const char** texture_files;

	vertex_array_t VertexArray;
	vertex_buffer_t VertexBuffer; 
	vertex_buffer_layout_t VertexBufferLayout;

	shader_program_t Shader;

} skybox_t;


typedef struct
{
	u32 loaded_texture_count;
	texture_t LoadedTextures[16];

	u32 model_count;
	model_t Models[10];
	shader_program_t LightShader;
	shader_program_t CubeShader;
	camera_t Camera;
} app_state_t;

#define APPLICATION_H
#endif
