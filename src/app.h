#if !defined(APP_H)

#include "app_platform.h"

struct memory_arena
{
	u8 *Base;
	memory_index Size;
	memory_index Used;

	s32 TempCount;
};

struct temporary_memory
{
	memory_arena *MemoryArena;
	memory_index Used;
};

internal void
ArenaInitialize(memory_arena *MemoryArena, memory_index Size, u8 *Base)
{
	MemoryArena->Size = Size;
	MemoryArena->Base = Base;
	MemoryArena->Used = 0;
	MemoryArena->TempCount = 0;
}

#define PushStruct(MemoryArena, type) (type *)PushSize_(MemoryArena, sizeof(type))
#define PushArray(MemoryArena, Count, type) (type *)PushSize_(MemoryArena, (Count) * sizeof(type))
#define PushSize(MemoryArena, Size) PushSize_(MemoryArena, Size)
void *
PushSize_(memory_arena *MemoryArena, memory_index Size)
{
	Assert((MemoryArena->Used + Size) <= MemoryArena->Size);
	void *Result = MemoryArena->Base + MemoryArena->Used;
	MemoryArena->Used += Size;
	return(Result);
}

inline temporary_memory
TemporaryMemoryBegin(memory_arena *MemoryArena)
{
	temporary_memory Result;

	Result.MemoryArena = MemoryArena;
	Result.Used = MemoryArena->Used;

	MemoryArena->TempCount++;

	return(Result);
}

inline void
TemporaryMemoryEnd(temporary_memory TempMemory)
{
	memory_arena *MemoryArena = TempMemory.MemoryArena;
	Assert(MemoryArena->Used >= TempMemory.Used);
	MemoryArena->Used = TempMemory.Used;
	Assert(MemoryArena->TempCount > 0);
	MemoryArena->TempCount--;
}

inline void
MemoryArenaCheck(memory_arena *MemoryArena)
{
	Assert(MemoryArena->TempCount == 0);
}

#define ZeroStruct(Instance) ZeroSize(&(Instance), sizeof(Instance))
inline void
ZeroSize(void *Ptr, memory_index Size)
{
	u8 *Byte = (u8*)Ptr;
	while(Size--)
	{
		*Byte++ = 0;
	}
}




#include "app_intrinsics.h"
#include "app_math.h"


#include "app_render_group.h"

struct mesh
{
	u32 *Indices;
	v3f *Vertices;
	v2f *TexCoords;
	v3f *Normals;

	// NOTE(Justin): Obj format for face is v/vt/vn where v is the vertex
	// position vt is the texture coordinate and vn is the normal. These are
	// indices into the respective attribute arrays. The obj file format starts
	// the indices from 1. When loading the data we subtract 1 so that the
	// indices start from 0.

	u32 *Faces;
	
	u32 VertexCount;
	u32 TexCoordCount;
	u32 NormalCount;
	u32 FaceCount;
	u32 IndicesCount;

	loaded_bitmap *Texture;
};

struct loaded_obj
{
	char *FileName;

	u8 *Memory;
	u32 Size;

	mesh Mesh;
};




struct camera
{
	v3f P;
	v3f Direction;
	v3f Up;

	f32 Yaw;
	f32 Pitch;
};

struct world
{
};

struct triangle
{
	v4f Vertices[3];
	v3f Colors[3];
};

struct rectangle
{
	v4f Min;
	v4f Max;
};

struct plane
{
	v3f P;
	v3f N;
	v2f Dim;
};

enum entity_type
{
	ENTITY_NULL,
	ENTITY_PLAYER,
	ENTITY_CIRCLE,
	ENTITY_RECTANGLE,
	ENTITY_TRIANGLE,
	ENTITY_MODEL,
	ENTITY_WALL,
};



// NOTE(Justin): Should we consider being able to store basis vectors as an
// array of floats and 3 v3fs? That way we can just use matrix multiplication to
// calculate the new basis after a transformation is applied.

struct entity
{
	u32 Index;

	entity_type Type;
	u32 Flags;

	v3f P;
	v3f dP;

	triangle Triangle;
	rectangle Rectangle;

	plane Wall;
	//aabb AABB;
	loaded_obj Model;

	basis Basis;
	
	mat4 Transform;

};

struct edge
{
	f32 X;
	f32 XStep;
	s32 YStart;
	s32 YEnd;
};


struct app_state
{
	memory_arena WorldArena;
	world *World;

	f32 PixelsToMeters;

	loaded_bitmap Test;

	loaded_obj Cube;
	loaded_obj Suzanne;
	loaded_obj Dodecahedron;
	loaded_obj Pyramid;
	loaded_obj CameraModel;

	camera Camera;

	mat4 MapToWorld;
	mat4 MapToCamera;
	mat4 MapToPersp;
	mat4 MapToScreenSpace;

	triangle Triangle;

	entity Entities[256];
	u32 EntityCount;

	u32 PlayerIndex;

	f32 Time;
};

struct transient_state
{
	b32 IsInitialized;
	memory_arena TransientArena;
};

global_variable platform_api Platform;
#define APP_H
#endif
