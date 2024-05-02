
#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX / 64)

internal tile_chunk *
TileChunkGet(world *World, s32 ChunkX, s32 ChunkY, s32 ChunkZ, memory_arena *Arena = 0)
{
	Assert(ChunkX > -TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkY > -TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkZ > -TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkX < TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkY < TILE_CHUNK_SAFE_MARGIN);
	Assert(ChunkZ < TILE_CHUNK_SAFE_MARGIN);

	// TODO(Justin): Better hash function.
	u32 HashValue = 17 * ChunkX + 9 * ChunkZ + 3 * ChunkY;
	u32 HashIndex = HashValue & (ArrayCount(World->ChunkHash) - 1);

	Assert(HashIndex < ArrayCount(World->ChunkHash));

	tile_chunk *WorldChunk = World->ChunkHash + HashIndex;
	do
	{
		if((WorldChunk->ChunkX == ChunkX) &&
		   (WorldChunk->ChunkY == ChunkY) &&
		   (WorldChunk->ChunkZ == ChunkZ))
		{
			break;
		}

		if(Arena && (WorldChunk->ChunkX != 0) && (!WorldChunk->NextInHash))
		{
			WorldChunk->NextInHash = PushStruct(Arena, tile_chunk);
			WorldChunk = WorldChunk->NextInHash;
			WorldChunk->ChunkX = 0;
		}

		if(Arena && (WorldChunk->ChunkX == 0))
		{

			WorldChunk->ChunkX = ChunkX;
			WorldChunk->ChunkY = ChunkY;
			WorldChunk->ChunkZ = ChunkZ;

			u32 TileCount = World->ChunkDim * World->ChunkDim * World->ChunkDim;
			WorldChunk->Tiles = PushArray(Arena, TileCount, u32);
			for(u32 Index = 0; Index < TileCount; ++Index)
			{
				WorldChunk->Tiles[Index] = 1;
			}

			WorldChunk->NextInHash = 0;

			break;
		}

		WorldChunk = WorldChunk->NextInHash;

	} while(WorldChunk);

	return(WorldChunk);
}

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

#if 0
internal tile_chunk *
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
#endif



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
	tile_chunk *TileChunk = TileChunkGet(World, ChunkP.ChunkX, ChunkP.ChunkY, ChunkP.ChunkZ, Arena);
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

internal v3f
WorldPosDifference(world *World, world_position *A, world_position *B)
{
	v3f Result;

	v3f dTileXYZ = {(f32)A->PackedX - B->PackedX,
				    (f32)A->PackedY - B->PackedY,
					(f32)A->PackedZ - B->PackedZ};

	Result = World->TileSideInMeters * dTileXYZ + (A->OffsetFromTileCenter_ - B->OffsetFromTileCenter_);

	// TODO(Justin): Why do we need to multiply z by -1? Does any offset
	// vector have to flip the z component? I think so...
	Result.z *= -1;

	return(Result);
}


internal world_position 
WorldPosMapIntoTileSpace(world *World, world_position P,  high_entity *EntityHigh)
{
	world_position Result = P;

	v3f OffsetFromCamera = Mat4Column(EntityHigh->Translate, 3).xyz;
	Result.OffsetFromTileCenter_ += OffsetFromCamera;

	// NOTE(Justin): Because the tile side in meters is 1 and the 
	// (X, Z) is an offset from the center of a tile when X >= 0.5f it rounds up
	// to 1 so the tile offset is 1 when X is >= 0.5f. When the player moves
	// directlt up and crosses a tile boundary z < -0.5f and the tile offset is
	// -1 but the player moved up and we therefore need to ADD 1 to the PackedZ
	// value so we do -(-1)

	// Get the tile offset
	s32 TileXOffset = F32RoundToS32(Result.OffsetFromTileCenter_.x / World->TileSideInMeters);
	s32 TileYOffset = F32RoundToS32(Result.OffsetFromTileCenter_.y / World->TileSideInMeters);
	s32 TileZOffset = F32RoundToS32(Result.OffsetFromTileCenter_.z / World->TileSideInMeters);

	// Add the tile offset to the packed coordinates
	Result.PackedX += TileXOffset;
	Result.PackedY += TileYOffset;
	Result.PackedZ -= TileZOffset;

	// Recompute the offset from tile center
	Result.OffsetFromTileCenter_.x -= TileXOffset * World->TileSideInMeters;
	Result.OffsetFromTileCenter_.y -= TileYOffset * World->TileSideInMeters;
	Result.OffsetFromTileCenter_.z -= TileZOffset * World->TileSideInMeters;

	return(Result);
}

internal void
WorldInitialize(world *World, f32 TileSideInMeters)
{
	World->ChunkShift = 4;
	World->ChunkMask = (1 << World->ChunkShift) - 1;
	World->ChunkDim = (1 << World->ChunkShift);
	World->TileSideInMeters = TileSideInMeters;
	World->ChunkDimInMeters = World->ChunkDim * World->TileSideInMeters;

	for(u32 ChunkIndex = 0; ChunkIndex < ArrayCount(World->ChunkHash); ++ChunkIndex)
	{
		World->ChunkHash[ChunkIndex].ChunkX = 0;
	}


}


