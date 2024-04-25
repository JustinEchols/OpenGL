/*
 TODO(Justin):

 Texture array
 Texture indices

*/
#if !defined(APP_H)

#include "app_platform.h"



#define QUAD_VERTEX_COUNT 4

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

#define ARRAY_COPY(Count, Src, Dest) MemoryCopy((Count)*sizeof(*(Src)), (Src), (Dest))
internal void *
MemoryCopy(memory_index Size, void *SrcInit, void *DestInit)
{
	u8 *Src = (u8 *)SrcInit;
	u8 *Dest = (u8 *)DestInit;
	while(Size--) {*Dest++ = *Src++;}

	return(DestInit);
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
#include "app_tile.h"
#include "app_render_group.h"

struct mesh
{
	char *Name;
	char *FullPath;

	v3f *Vertices;
	v2f *UV;
	v3f *Normals;
	v4f *Colors;

	// NOTE(Justin): Obj format for face is v/vt/vn where v is the vertex
	// position vt is the texture coordinate and vn is the normal. These are
	// indices into the respective attribute arrays. The obj file format starts
	// the indices from 1. When loading the data we subtract 1 so that the
	// indices start from 0.

	u32 *Indices;
	
	u32 VertexCount;
	u32 UVCount;
	u32 NormalCount;
	u32 IndicesCount;
	u32 ColorCount;

	loaded_bitmap *Texture;
};

// TODO(Justin): GL flag for rendering specific geometry?
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
	//world_position P;
	v3f Direction;
	v3f Up;

	f32 Yaw;
	f32 Pitch;
};

struct triangle
{
	v4f Vertices[3];
	v3f Colors[3];
};

struct quad
{
	v3f Vertices[4];
	v3f Colors[4];
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
	EntityType_Null,
	EntityType_Player,
	EntityType_Wall,
	EntityType_Quad, // Ground
};

struct interval
{
	f32 Min, Max;
};

struct edge
{
	f32 X;
	f32 XStep;
	s32 YStart;
	s32 YEnd;
};

struct aabb_min_max
{
	v3f Min;
	v3f Max;
};

enum entity_residence
{
	EntityResidence_NonExistant,
	EntityResidence_Low,
	EntityResidence_High,
};

struct low_entity 
{
	entity_type Type;
	b32 Collides;

	world_position P;
	f32 Width, Height, Depth;
};

// NOTE(Justin): Maybe pull this out to entity.
struct high_entity 
{
	basis Basis;
	v3f dP;

	mesh Mesh[2];
	aabb_min_max AABBMinMax;

	mat4 Translate;
	mat4 Scale;
};

struct entity
{
	u32 Index;
	u32 Flags;

	u32 Residence;
	low_entity *Low;
	high_entity *High;
};


struct app_state
{
	memory_arena WorldArena;

	f32 MetersToPixels;

	world World;

	loaded_bitmap Ground;
	loaded_bitmap Gray;
	loaded_bitmap White;
	loaded_bitmap Black;

	loaded_obj Cube;

	camera Camera;
	b32 CameraIsFree;
	world_position CameraP;


	mat4 MapToWorld;
	mat4 MapToCamera;
	mat4 MapToPersp;
	mat4 MapToScreenSpace;

	u32 EntityCount;
	entity_residence EntityResidence[4096];
	low_entity EntitiesLow[4096];
	high_entity EntitiesHigh[4096];

	u32 CameraEntityFollowingIndex;
};

struct transient_state
{
	b32 IsInitialized;
	memory_arena TransientArena;
};

global_variable platform_api Platform;
#define APP_H
#endif
