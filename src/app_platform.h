#if !defined(APP_PLATFORM_H)

#ifdef __cplusplus
extern "C" {
#endif

//
// NOTE(Justin): Compilers
//

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

//
// NOTE(Justin): Types
//

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <limits.h>
#include <float.h>

typedef int8_t		s8;
typedef int16_t 	s16;
typedef int32_t 	s32;
typedef int64_t 	s64;
typedef s32			b32;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef size_t		memory_index;

typedef float		f32;
typedef double		f64;

#define internal		static
#define local_persist	static
#define global_variable static

#define PI32 3.1415926535897f
#define ONE_OVER_ROOT_TWO 0.707106781188f

#define DegreeToRad(Degrees) ((Degrees) * (PI32 / 180.0f))

#if APP_SLOW
#define Assert(Expression) if (!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define INVALID_CODE_PATH Assert(!"Invaid code path")
#define INVALID_DEFAULT_CASE default: {INVALID_CODE_PATH;} break

#define ArrayCount(A) (sizeof(A) / sizeof((A)[0]))

#define Kilobytes(Count) (1024LL * (Count))
#define Megabytes(Count) (1024LL * Kilobytes(Count))
#define Gigabytes(Count) (1024LL * Megabytes(Count))
#define Terabytes(Count) (1024LL * Gigabytes(Count))

#define ABS(x) (((x) > 0) ? (x) : -(x))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) < (b)) ? (b) : (a))
#define SQUARE(x) ((x) * (x))
#define CUBE(x) ((x) * (x) * (x))

inline u32
TruncateU64ToU32(u64 Value)
{
	Assert(Value <= 0xFFFFFFFF);
	u32 Result = (u32)Value;
	return(Result);
}

typedef struct thread_context
{
	int PlaceHolder;
} thread_context;


typedef struct debug_file_read_result
{
	void *Content;
	u32 Size;
} debug_file_read_result;

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_file_read_result name(thread_context *Thread, char *Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32 name(thread_context *Thread, char *Filename, u32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(thread_context *Thread, void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

typedef void platform_opengl_render(struct render_group *RenderGroup, struct app_offscreen_buffer *Buffer);


typedef struct
{
	platform_opengl_render *RenderToOpenGL;

#if APP_INTERNAL
	debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
	debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
	debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
#endif
} platform_api;

#define BITMAP_BYTES_PER_PIXEL 4
typedef struct app_offscreen_buffer
{
	s32 Width;
	s32 Height;
	s32 Stride;
	void *Memory;
} app_offscreen_buffer;

typedef struct app_button_state
{
	b32 EndedDown;
	u32 HalfTransitionCount;
} app_button_state;

typedef struct app_controller_input
{
	union
	{
		app_button_state Buttons[8];
		struct
		{

			app_button_state W;
			app_button_state A;
			app_button_state S;
			app_button_state D;

			app_button_state R;

			app_button_state Up;
			app_button_state Down;
			app_button_state Left;
			app_button_state Right;
		};
	};
} app_controller_input;

typedef struct app_input
{
	app_button_state MouseButtons[5];
	s32 MouseX, MouseY, MouseZ;
	f32 dMouseX, dMouseY;

	f32 dtForFrame;
	b32 ExecutableReloaded;

	app_controller_input Controllers[1];
} app_input;

struct app_memory
{
	b32 IsInitialized;
	u64 PermanentStorageSize;
	void *PermanentStorage;

	u64 TransientStorageSize;
	void *TransientStorage;

	platform_api PlatformAPI;
};

#define APP_UPDATE_AND_RENDER(name) void name(thread_context *Thread, app_memory *Memory, app_input *Input, app_offscreen_buffer *BackBuffer)
typedef APP_UPDATE_AND_RENDER(app_update_and_render);

#ifdef __cplusplus
}
#endif

#define APP_PLATFORM_H
#endif
