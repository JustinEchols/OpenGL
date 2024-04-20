#if !defined(APP_TILE_H)

struct tile_chunk
{
	u32 *Tiles;

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

	tile_chunk *TileChunks;
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

#define APP_TILE_H
#endif
