#if !defined(APP_TILE_H)

struct tile_chunk
{
	s32 ChunkX;
	s32 ChunkY;
	s32 ChunkZ;

	u32 *Tiles;

	tile_chunk *NextInHash;
};

struct world
{
	u32 ChunkShift;
	u32 ChunkMask;
	s32 ChunkDim;

	s32 ChunkCountX;
	s32 ChunkCountY;
	s32 ChunkCountZ;

	f32 TileSideInMeters;
	f32 ChunkDimInMeters;

	tile_chunk ChunkHash[4096];
};

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


#define APP_TILE_H
#endif
