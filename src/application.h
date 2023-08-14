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

#define internal static
#define global_variable static
#define local_persist static

#define Kilobytes(count) (1024 * count)
#define Megabytes(count) (1024 * Kilobytes(count))
#define Gigabytes(count) (1024 * Megabytes(count))
#define Terabytes(count) (1024 * Gigabytes(count))

#define GL_LOG_FILE "gl.log"
#define E1 glm::vec3(1.0f, 0.0f, 0.0f)
#define E2 glm::vec3(0.0f, 1.0f, 0.0f)
#define E3 glm::vec3(0.0f, 0.0f, 1.0f)
#define PI32 3.141592653589f
#define MAX_SHADER_SIZE 100000

#define ASSERT(expression) if((!expression)) {*(int *)0 = 0;}
#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))

#define GLCall(gl_func) gl_clear_errors();\
	gl_func;\
	ASSERT(GLLogCall(#gl_func, __FILE__, __LINE__))

#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices)

#define IntFromPtr(ptr) (unsigned long long)((char *)ptr - (char *)0)
#define Member(Type, member) (((Type*)0)->member)
#define OffsetOfMember(Type,member) IntFromPtr(&Member(Type,member))

#define Stringify_(x) #x
#define Stringify(x) Stringify_(x)

#define Glue_(a, b) a##b
#define Glue(a, b) Glue_(a, b)

#define StackStringBufferName(name) Glue(foo, name)
#define StackString(name, count) char StackStringBufferName(name)[count];\
	string2_t name;\
name.size = count;\
name.length = 0;\
name.buff = StackStringBufferName(name);

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
	Transparent,
	ShadowOnly,
	Tinted,
	Vegetation,
	Occluder,
	CollisionOnly,
	Cloud,
	Laser,
} material_type_t;

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
	glm::vec3 Tangent;
	glm::vec3 BiTangent;
	glm::vec3 Normal;
} local_frame_t;

typedef struct
{
	GLFWwindow* handle;
	s32 width, height;
} window_t;

typedef struct
{
	glm::vec3 Min;
	glm::vec3 Max;
} bounding_box_t;


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

//
// NOTE(Justin): OpenGL like primitive data structures
//

typedef struct
{
	GLuint id;
	// TODO(Justin): Remove binded
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
	//vertex_buffer_layout_t VertexBufferLayout;
} vertex_array_t;

typedef struct
{
	u32* indices;
	u32 count;
} index_buffer_t;

typedef struct
{
	s32 size;
	s32 location;
	char name[64];
	//char *type;
	GLenum type;
} uniform_t;

typedef struct
{
	GLuint id;
	s32 link_status;
	s32 attached_shaders_count;
	char *vertex_shader_filename;
	char *geometry_shader_filename;
	char *fragment_shader_filename;

	char *name;

	uniform_t *Uniforms;
	u32 uniforms_count;

	// TODO(Justin): Input attribuites?
} shader_program_t;

enum texture_type_t
{
	TEXTURE_TYPE_DIFFUSE,
	TEXTURE_TYPE_SPECULAR,
	TEXTURE_TYPE_NORMAL,
	TEXTURE_TYPE_HEIGHT,
	TEXTURE_TYPE_SKYBOX,

	TEXTURE_TYPE_COUNT
};

typedef struct
{
	GLuint id;
	s32 width, height;
	s32 channel_count;
	u32 mipmap_level;
	u8 *memory;

	char path[1024];

	// Not sure if enum or string is better.
	texture_type_t type;
} texture_t;

//
// NOTE(Justin): Geometric Primitives
//

typedef struct
{
	GLuint VBO, VAO;

	//glm::vec2 *Positions;
	f32 *positions;
	u32 positions_count;

	//glm::vec3 *Colors;
	f32 *colors;
	u32 colors_count;

	shader_program_t Shader;

} points_ndc_t;
// TODO(Justin): Add normals, tex coords.
typedef struct
{
	GLuint VBO, EBO, VAO;
	texture_t Texture;
	shader_program_t Shader;
} quad_t;

typedef struct
{
	glm::vec3 Min;
	glm::vec3 Max;
} rectangle_t;

typedef struct
{
	glm::vec3 P1;
	glm::vec3 P2;

	GLuint VAO, VBO;

	shader_program_t Shader;

} line_t;


typedef struct
{
	GLuint VBO;

	u32 count;
	void *data;
} instanced_array_t;






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
	u32 *indices;
} mesh_indices_t;

typedef struct
{

	//GLuint VBO, VAO;
	vertex_array_t VertexArray;
	vertex_buffer_t VertexBuffer;
	vertex_buffer_layout_t VertexBufferLayout;

	texture_t Textures[4];

	shader_program_t Shader;

	glm::vec3 Pos;
} cube_t;

typedef struct
{
	u32 texture_count;
	texture_t *Textures;
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


	material_type_t MaterialType;

	// TODO(Justin): Each mesh has a shader reason why is because each mesh has
	// a different set of data attached to it?
} mesh_t;

typedef struct
{
	u32 size;
	u32 length;
	char *buff;
} string2_t;

typedef struct
{
	// TODO(Justin) Data is not a good name to use;
	u32 count;
	char *data;
} string_t;


typedef struct
{
	char *path_to_dir;
	char *name;
	glm::vec3 Pos;

	u32 mesh_count;
	mesh_t *Meshes;
} model_t;

typedef struct
{

	texture_t TextureCubeMap;
	char **texture_files;

	vertex_array_t VertexArray;
	vertex_buffer_t VertexBuffer; 
	vertex_buffer_layout_t VertexBufferLayout;

	shader_program_t Shader;

} skybox_t;


typedef struct
{
	u32 loaded_skybox_count;
	skybox_t Skyboxes[10];

	u32 loaded_texture_count;
	texture_t LoadedTextures[32];


	u32 model_count;
	model_t Models[10];
	shader_program_t LightShader;
	shader_program_t CubeShader;
	camera_t Camera;
} app_state_t;

#define APPLICATION_H
#endif
