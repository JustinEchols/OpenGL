#if !defined(APP_WORLD_H)

struct chunk_tile_position
{
	s32 ChunkX;
	s32 ChunkY;
	s32 ChunkZ;

	s32 TileX;
	s32 TileY;
	s32 TileZ;
};

struct world_position
{
	s32 PackedX;
	s32 PackedY;
	s32 PackedZ;

	// NOTE(Justin): This a position relative to the center of a tile. The _
	// at the end is used to denote the fact that this value should rarely be
	// changed.
	v3f OffsetFromTileCenter_;
};

struct world_entity_block
{
	u32 EntityCount;
	u32 EntityLowIndex[16];
	world_entity_block *Next;
};

struct world_chunk 
{
	s32 ChunkX;
	s32 ChunkY;
	s32 ChunkZ;

	world_entity_block FirstBlock;

	world_chunk *NextInHash;
};

struct world
{
	u32 ChunkShift;
	u32 ChunkMask;
	s32 ChunkDim;

	f32 TileSideInMeters;
	f32 ChunkDimInMeters;

	world_chunk ChunkHash[4096];
};

#define APP_WORLD_H
#endif
