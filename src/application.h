#if !defined(APPLICATION_H)

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

typedef struct
{
	s32 width, height;
	s32 channel_count;
	u32 mipmap_level;
	u8* memory;
	GLuint id;

} texture_t;

typedef struct
{
	glm::vec3 Pos;
	glm::vec3 Direction;
	glm::vec3 Up;

	f32 speed;

} camera_t;


typedef struct
{
	b32 is_down;
} key_t;

typedef struct
{
	f32 speed;
	glm::vec2 Pos;
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
	key_t Keys[KEY_COUNT];
	mouse_t Mouse;
} input_t;

typedef struct
{
	shader_program_t ShaderProgram;
	camera_t Camera;
} app_state_t;

#define APPLICATION_H
#endif
