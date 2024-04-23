
inline u32
TileValueUncheckedGet(world *World, tile_chunk *TileChunk, s32 TileX, s32 TileY, s32 TileZ)
{
	Assert(TileChunk);
	Assert(TileX < World->ChunkDim);
	Assert(TileY < World->ChunkDim);
	Assert(TileZ < World->ChunkDim);

	u32 Result = TileChunk->Tiles[TileY * World->ChunkDim * World->ChunkDim +
								  TileZ * World->ChunkDim +
								  TileX];
	return(Result);
}

inline u32
TileValueGet(world *World, tile_chunk *TileChunk, s32 TileX, s32 TileY, s32 TileZ)
{
	u32 Result = 0;
	if(TileChunk && TileChunk->Tiles)
	{
		Result = TileValueUncheckedGet(World, TileChunk, TileX, TileY, TileZ);
	}

	return(Result);
}

internal tile_chunk*
TileChunkGet(world *World, s32 ChunkX, s32 ChunkY, s32 ChunkZ)
{
	tile_chunk *TileChunk= 0;

	if((ChunkX >= 0) && (ChunkX < World->ChunkCountX) &&
	   (ChunkY >= 0) && (ChunkY < World->ChunkCountY) &&
	   (ChunkZ >= 0) && (ChunkZ < World->ChunkCountZ))
	{
		TileChunk = &World->TileChunks[ChunkY * World->ChunkCountZ * World->ChunkCountX + 
									   ChunkZ * World->ChunkCountX +
									   ChunkX];
	}
	
	return(TileChunk);
}

inline chunk_tile_position
ChunkPosGet(world *World, s32 PackedX, s32 PackedY, s32 PackedZ)
{
	chunk_tile_position Result;

	Result.ChunkX = PackedX >> World->ChunkShift;
	Result.ChunkY = PackedY >> World->ChunkShift;
	Result.ChunkZ = PackedZ >> World->ChunkShift;
	Result.TileX = PackedX & World->ChunkMask;
	Result.TileY = PackedY & World->ChunkMask;
	Result.TileZ = PackedZ & World->ChunkMask;

	return(Result);
}

inline void
TileValueSetUnchecked(world *World, tile_chunk *TileChunk, s32 TileX, s32 TileY, s32 TileZ, u32 TileValue)
{
	Assert(TileChunk);
	Assert(TileX < World->ChunkDim);
	Assert(TileY < World->ChunkDim);
	Assert(TileZ < World->ChunkDim);

	TileChunk->Tiles[TileY * World->ChunkDim * World->ChunkDim +
					 TileZ * World->ChunkDim +
					 TileX] = TileValue;
}

inline void
TileValueSet(world *World, tile_chunk *TileChunk, s32 TileX, s32 TileY, s32 TileZ, u32 TileValue)
{
	if(TileChunk)
	{
		TileValueSetUnchecked(World, TileChunk, TileX, TileY, TileZ, TileValue);
	}
}

inline void
TileValueSet(memory_arena *Arena, world *World, s32 PackedX, s32 PackedY, s32 PackedZ, u32 TileValue)
{
	chunk_tile_position ChunkP = ChunkPosGet(World, PackedX, PackedY, PackedZ);
	tile_chunk *TileChunk = TileChunkGet(World, ChunkP.ChunkX, ChunkP.ChunkY, ChunkP.ChunkZ);
	if(!TileChunk->Tiles)
	{
		u32 TileCount = World->ChunkDim * World->ChunkDim * World->ChunkDim;
		TileChunk->Tiles = PushArray(Arena, TileCount, u32);
		for(u32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
		{
			TileChunk->Tiles[TileIndex] = 1;
		}
	}

	Assert(TileChunk);
	TileValueSet(World, TileChunk, ChunkP.TileX, ChunkP.TileY, ChunkP.TileZ, TileValue);
}

inline u32
TileValueGet(world *World, s32 PackedX, s32 PackedY, s32 PackedZ)
{
	u32 Result = 0;

	chunk_tile_position ChunkP = ChunkPosGet(World, PackedX, PackedY, PackedZ);
	tile_chunk *TileChunk = TileChunkGet(World, ChunkP.ChunkX, ChunkP.ChunkY, ChunkP.ChunkZ);
	Result = TileValueGet(World, TileChunk, ChunkP.TileX, ChunkP.TileY, ChunkP.TileZ);

	return(Result);
}

internal b32
WorldTileIsEmpty(world *World, s32 PackedX, s32 PackedY, s32 PackedZ)
{
	b32 Result = false;

	u32 TileValue = TileValueGet(World, PackedX, PackedY, PackedZ);

	Result = (TileValue == 1);

	return(Result);
}


internal void
PlayerPosReCompute(world *World, s32 *PackedX, s32 *PackedY, s32 *PackedZ, f32 *X, f32 *Y, f32 *Z)
{
	// NOTE(Justin): Because the tile side in meters is 1 and the 
	// (X, Z) is an offset from the center of a tile when X >= 0.5f it rounds up
	// to 1 so the tile offset is 1 when X is >= 0.5f. When the player moves
	// directlt up and crosses a tile boundary z < -0.5f and the tile offset is
	// -1 but the player moved up and we therefore need to ADD 1 to the PackedZ
	// value so we do -(-1)

	s32 TileXOffset = F32RoundToS32(*X  /  World->TileSideInMeters);
	s32 TileYOffset = F32RoundToS32(*Y  /  World->TileSideInMeters);
	s32 TileZOffset = F32RoundToS32(*Z / World->TileSideInMeters);

	*PackedX += TileXOffset;
	*PackedY += TileYOffset;
	*PackedZ -= TileZOffset;

	*X -= TileXOffset * World->TileSideInMeters;
	*Y -= TileYOffset * World->TileSideInMeters;
	*Z -= TileZOffset * World->TileSideInMeters;
}

internal v3f
WorldPosDifference(world *World, world_position *A, world_position *B)
{
	v3f Result;

	v3f dTileXYZ = {(f32)A->PackedX - B->PackedX,
				    (f32)A->PackedY - B->PackedY,
					(f32)A->PackedZ - B->PackedZ};

	Result = World->TileSideInMeters * dTileXYZ + (A->OffsetFromTileCenter_ - B->OffsetFromTileCenter_);

	// TODO(Justin): Should we flip the z component here?
	return(Result);
}

