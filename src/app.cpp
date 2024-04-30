
#include "app.h"
#include "app_tile.cpp"
#include "app_random.h"
#include "app_render_group.cpp"

internal u32
EntityLowAdd(app_state *AppState, entity_type Type)
{

	Assert(AppState->EntityLowCount < ArrayCount(AppState->EntitiesLow));

	u32 EntityIndex = AppState->EntityLowCount++;
	AppState->EntitiesLow[EntityIndex] = {};
	AppState->EntitiesLow[EntityIndex].Type = Type;

	return(EntityIndex);
}

inline basis
BasisStandard(void)
{
	basis Result = {};

	Result.O = V3F(0.0f);
	Result.U = XAxis();
	Result.V = YAxis();
	Result.W = ZAxis();

	return(Result);
}

internal high_entity *
EntityFrequencySetToHigh(app_state *AppState, u32 LowIndex)
{
	high_entity *EntityHigh = 0;

	low_entity *EntityLow = AppState->EntitiesLow + LowIndex;
	if(EntityLow->HighIndex)
	{
		EntityHigh = AppState->EntitiesHigh_ + EntityLow->HighIndex;
	}
	else
	{
		if(AppState->EntityHighCount < ArrayCount(AppState->EntitiesHigh_))
		{
			u32 HighIndex = AppState->EntityHighCount++;
			EntityHigh = AppState->EntitiesHigh_ + HighIndex;

			v3f Translate = WorldPosDifference(&AppState->World, &EntityLow->P, &AppState->CameraP);
			v3f CameraP = {(f32)AppState->CameraP.PackedX, (f32)AppState->CameraP.PackedY, (f32)AppState->CameraP.PackedZ};
			EntityHigh->Basis = BasisStandard();
			EntityHigh->Basis.O = CameraP;
			EntityHigh->Translate = Mat4Translation(Translate);
			EntityHigh->LowIndex = LowIndex;

			EntityLow->HighIndex = HighIndex;
		}
		else
		{
			INVALID_CODE_PATH;
		}
	}

	return(EntityHigh);
}

internal void
EntityFrequencySetToLow(app_state *AppState, u32 LowIndex)
{
	low_entity *EntityLow = AppState->EntitiesLow + LowIndex;
	u32 HighIndex = EntityLow->HighIndex;
	if(HighIndex)
	{
		u32 LastHighIndex = AppState->EntityHighCount - 1;
		if(HighIndex != LastHighIndex)
		{
			high_entity *Last = AppState->EntitiesHigh_ + LastHighIndex;
			high_entity *Remove = AppState->EntitiesHigh_ + HighIndex;

			*Remove = *Last;
			AppState->EntitiesLow[Last->LowIndex].HighIndex = HighIndex;
		}
	}
	--AppState->EntityHighCount;
	EntityLow->HighIndex = 0;
}

inline low_entity *
EntityLowGet(app_state *AppState, u32 Index)
{
	low_entity *Result = 0;

	if((Index > 0) && (Index < AppState->EntityLowCount))
	{
		Result = AppState->EntitiesLow + Index;
	}

	return(Result);
}

inline entity 
EntityHighGet(app_state *AppState, u32 LowIndex)
{
	entity Result = {};

	if((LowIndex > 0) && (LowIndex < AppState->EntityLowCount))
	{
		Result.LowIndex = LowIndex;
		Result.Low = AppState->EntitiesLow + LowIndex;
		Result.High = EntityFrequencySetToHigh(AppState, LowIndex);
	}

	return(Result);
}



// TODO(Justin): This is a hack.. Do the real version..
internal aabb
AABBMinMax(v3f *Vertices, u32 VerticesCount)
{
	aabb Result = {};

	f32 CurrentMinProject = 0.0f;
	f32 CurrentMaxProject = 0.0f;

	v3f DMax = V3F(1.0f, 1.0f, 1.0f);
	v3f DMin = V3F(-1.0f, -1.0f, -1.0f);

	u32 IndexOfMin = 0;
	u32 IndexOfMax = 0;

	for(u32 Index = 0; Index < VerticesCount; ++Index)
	{
		v3f V = Vertices[Index];

		f32 PMin = Dot(V, DMin);
		f32 PMax = Dot(V, DMax);

		if(PMin > CurrentMinProject)
		{
			CurrentMinProject = PMin;
			IndexOfMin = Index;
		}

		if(PMax > CurrentMaxProject)
		{
			CurrentMaxProject = PMax;
			IndexOfMax = Index;
		}
	}

	Result.Min = Vertices[IndexOfMin];
	Result.Max = Vertices[IndexOfMax];

	return(Result);
}


internal void
PlayerAdd(app_state *AppState, aabb AABB)
{
	u32 EntityIndex = EntityLowAdd(AppState, EntityType_Player);
	low_entity *EntityLow = EntityLowGet(AppState, EntityIndex);

	world *World = &AppState->World;

	EntityLow->Collides = true;
	EntityLow->P.PackedX = 8;
	EntityLow->P.PackedY = 0;
	EntityLow->P.PackedZ = 5;

	// TODO(Jusitn): These are dimensions not w,h and d.
	EntityLow->Width = 0.35f;
	EntityLow->Height = 0.35f;
	EntityLow->Depth = 0.35f;
	EntityLow->BboxDim = 0.7f;
	EntityLow->Scale = Mat4Scale(EntityLow->Height);
	EntityLow->P.OffsetFromTileCenter_.y = EntityLow->Height;

	EntityLow->Mesh = AppState->Cube.Mesh;

	if(AppState->CameraEntityFollowingIndex == 0)
	{
		AppState->CameraEntityFollowingIndex = EntityIndex;
	}
}


internal void
ModelAdd(app_state *AppState, mesh *Mesh, s32 PackedX, s32 PackedY, s32 PackedZ,
		mat4 Scale, aabb AABB, loaded_bitmap *Texture = 0)
{
	u32 EntityIndex = EntityLowAdd(AppState, EntityType_Wall);
	low_entity *EntityLow = EntityLowGet(AppState, EntityIndex);

	EntityLow->Collides = true;
	EntityLow->P.PackedX = PackedX;
	EntityLow->P.PackedY = PackedY;
	EntityLow->P.PackedZ = PackedZ;
	EntityLow->Width = 0.5f;
	EntityLow->Height = 0.5f;
	EntityLow->Depth = 0.5f;

	EntityLow->BboxDim = 1.0f;
	EntityLow->P.OffsetFromTileCenter_.y = EntityLow->Height;
	EntityLow->Scale = Scale;

	EntityLow->Mesh = *Mesh;
}

internal void
QuadAdd(app_state *AppState, mesh *Mesh, s32 PackedX, s32 PackedY, s32 PackedZ, mat4 Scale)
{
	u32 EntityIndex = EntityLowAdd(AppState, EntityType_Quad);
	low_entity *EntityLow = EntityLowGet(AppState, EntityIndex);
	
	EntityLow->Collides = false;
	EntityLow->P.PackedX = PackedX;
	EntityLow->P.PackedY = PackedY;
	EntityLow->P.PackedZ = PackedZ;
	EntityLow->Scale = Scale;
	EntityLow->Mesh = *Mesh;
}

inline b32
InSameTile(world_position *A, world_position *B)
{
	b32 Result = ((A->PackedX == B->PackedX) &&
				 (A->PackedY == B->PackedY) &&
				 (A->PackedZ == B->PackedZ));

	return(Result);
}

inline b32
AABBIntersectsAABB(aabb A, aabb B)
{

	b32 Result = ((A.Max.x >= B.Min.x) &&
				  (A.Max.y >= B.Min.y) &&
				  (A.Max.z >= B.Min.z) &&
				  (A.Min.x < B.Max.x) &&
				  (A.Min.y < B.Max.y) &&
				  (A.Min.z < B.Max.z));
	
	return(Result);
}

internal b32
WallCollisionTest(v3f RelP, v3f PlayerDelta, v3f PointOnPlane, v3f PlaneNormal, aabb MKSumAABB, f32 *tMin)
{
	b32 Collided = false;

	f32 tEpsilon = 0.001f;
	f32 D = Dot(PlaneNormal, PointOnPlane);
	if(V3FNotZero(PlayerDelta))
	{
		f32 tResult = (D - Dot(PlaneNormal, RelP)) / Dot(PlaneNormal, PlayerDelta);
		if((tResult >= 0.0f) && tResult < *tMin)
		{
			v3f PointOfIntersection = RelP + tResult * PlayerDelta;
			if(IsInAABB(MKSumAABB, PointOfIntersection))
			{
				*tMin = Max(0.0f, tResult - tEpsilon);
				Collided = true;
			}
		}
	}

	return(Collided);
}


internal void
EntityMove(app_state *AppState, entity Entity, v3f ddP, f32 dt)
{ 
	world *World = &AppState->World;

	if(V3FNotZero(ddP))
	{
		ddP = Normalize(ddP);
	}

	f32 PlayerSpeed = 10.0f;
	ddP *= PlayerSpeed;
	ddP += -3.0f * Entity.High->dP;

	v3f OldPlayerP = Mat4Column(Entity.High->Translate, 3).xyz;
	v3f PlayerDelta = 0.5f * ddP * Square(dt) + Entity.High->dP * dt;
	Entity.High->dP = ddP * dt + Entity.High->dP;

	for(u32 Iteration = 0; Iteration < 4; ++Iteration)
	{
		f32 tMin = 1.0f;
		v3f WallNormal = V3F(0.0f);
		v3f DesiredPosition = OldPlayerP + PlayerDelta;
		b32 Collided = false;
		for(u32 TestEntityHighIndex = 1; TestEntityHighIndex < AppState->EntityHighCount; ++TestEntityHighIndex)
		{
			if(TestEntityHighIndex != Entity.Low->HighIndex)
			{
				entity EntityToTest = {};
				EntityToTest.High = AppState->EntitiesHigh_ + TestEntityHighIndex;
				EntityToTest.LowIndex = EntityToTest.LowIndex;
				EntityToTest.Low = AppState->EntitiesLow + EntityToTest.LowIndex;

				if(EntityToTest.Low->Collides)
				{
					v3f TestP = Mat4Column(EntityToTest.High->Translate, 3).xyz;
					v3f CurrentP = Mat4Column(Entity.High->Translate, 3).xyz;
					v3f RelP = CurrentP - TestP;

					// TODO(Justin): Paramterize the dimensions of the AABB.
					f32 BboxDim = Entity.Low->BboxDim + EntityToTest.Low->BboxDim;
					aabb MKSumAABB = AABBCenterDim(V3F(0.0f), V3F(BboxDim));

					// NOTE(Jusitn): Left face
					v3f PlaneNormal = {-1.0f, 0.0f, 0.0f};
					v3f PointOnPlane = MKSumAABB.Min;
					if(WallCollisionTest(RelP, PlayerDelta, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
					{
						WallNormal = PlaneNormal;
						Collided = true;
					}

					// NOTE(Jusitn): Right face
					PlaneNormal = {1.0f, 0.0f, 0.0f};
					PointOnPlane = MKSumAABB.Max;
					if(WallCollisionTest(RelP, PlayerDelta, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
					{
						WallNormal = PlaneNormal;
						Collided = true;
					}

					// NOTE(Jusitn): Front face
					PlaneNormal = {0.0f, 0.0f, 1.0f};
					PointOnPlane = MKSumAABB.Max;
					if(WallCollisionTest(RelP, PlayerDelta, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
					{
						WallNormal = PlaneNormal;
						Collided = true;
					}

					// NOTE(Jusitn): Back face
					PlaneNormal = {0.0f, 0.0f, -1.0f};
					PointOnPlane = MKSumAABB.Min;
					if(WallCollisionTest(RelP, PlayerDelta, PointOnPlane, PlaneNormal, MKSumAABB, &tMin))
					{
						WallNormal = PlaneNormal;
						Collided = true;
					}
				}
			}
		}

		v3f OffsetFromCamera = Mat4Column(Entity.High->Translate, 3).xyz;
		OffsetFromCamera += tMin * PlayerDelta;
		Entity.High->Translate = Mat4Translation(OffsetFromCamera);
		if(Collided)
		{
			Entity.High->dP = Entity.High->dP - Dot(WallNormal, Entity.High->dP) * WallNormal;
			PlayerDelta = DesiredPosition - Mat4Column(Entity.High->Translate, 3).xyz;
			PlayerDelta = PlayerDelta - Dot(WallNormal, PlayerDelta) * WallNormal;
		}
		else
		{
			break;
		}
	}

	Entity.Low->P = WorldPosMapIntoTileSpace(World, AppState->CameraP, Entity.High);
}


inline f32
StringToFloat(char *String)
{
	f32 Result = (f32)atof(String);
	return(Result);
}

inline u32
CharTOU32(char C)
{
	u32 Result = 0;
	Result = Result * 10 + C - '0';
	return(Result);
}

inline b32
CharIsNum(char C)
{
	b32 Result = false;

	if((C >= '0') && (C <= '9'))
	{
		Result = true;
	}

	return(Result);
}

internal void
ParseV3VertexAttribute(u8 *Data, v3f *AttributeArray, u32 Count, u32 FloatCount)
{
	for(u32 Index = 0; Index < Count; ++Index)
	{
		f32 Floats[3] = {};
		for(u32 FloatIndex = 0; FloatIndex < FloatCount; ++FloatIndex)
		{
			f32 Value = StringToFloat((char *)Data);
			Floats[FloatIndex] = Value;
			while(*Data != ' ')
			{
				Data++;
			}
			Data++;
		}

		v3f Attribute = {Floats[0], Floats[1], Floats[2]};
		AttributeArray[Index] = Attribute;
	}
}

// TODO(Jusitn): Do the parsing in one routine
internal void
ParseV2VertexAttribute(u8 *Data, v2f *AttributeArray, u32 Count, u32 FloatCount)
{
	for(u32 Index = 0; Index < Count; ++Index)
	{
		f32 Floats[2] = {};
		for(u32 FloatIndex = 0; FloatIndex < FloatCount; ++FloatIndex)
		{
			f32 Value = StringToFloat((char *)Data);
			Floats[FloatIndex] = Value;
			while(*Data != ' ')
			{
				Data++;
			}
			Data++;
		}

		v2f Attribute = {Floats[0], Floats[1]};
		AttributeArray[Index] = Attribute;
	}
}

internal loaded_obj
DEBUGObjReadEntireFile(thread_context *Thread, char *FileName, memory_arena *Arena,
		debug_platform_read_entire_file *DEBUGPlatformReadEntireFile)
{
	loaded_obj Result = {};

	debug_file_read_result ObjFile = DEBUGPlatformReadEntireFile(Thread, FileName);
	if(ObjFile.Size != 0)
	{
		Result.FileName = FileName;
		Result.Memory = (u8 *)ObjFile.Content;
		Result.Size = ObjFile.Size;

		u32 VertexCount = 0;
		u32 TextureCoordCount = 0;
		u32 NormalCount = 0;
		u32 IndicesCount = 0;

		u32 IndexCount = 0;

		char v = 'v';
		char Space = ' ';
		char t = 't';
		char n = 'n';
		char f = 'f';

		u32 FirstVertexOffset = 0;
		u32 FirstTextureOffset = 0;
		u32 FirstNormalOffset = 0;
		u32 FirstFaceOffset = 0;

		u32 FaceRows = 0;
		u32 FaceCols = 0;

		u32 FilePosition = 0;

		u8 *Content = Result.Memory;
		mesh *Mesh = &Result.Mesh;
		while(*Content++)
		{
			char Current = *Content;
			char Next = *(Content + 1);
			if(Next == '\0')
			{
				Assert((Current > 0) && (Current < 127) && (Current == '\n'));
				break;
			}

			if((Current == v) && (Next == Space))
			{
				if(FirstVertexOffset == 0)
				{
					FirstVertexOffset = FilePosition + 3;
				}
				Mesh->VertexCount++;
			}

			if((Current == t) && (Next == Space))
			{
				if(FirstTextureOffset == 0)
				{
					FirstTextureOffset = FilePosition + 4;
				}
				Mesh->UVCount++;
			}

			if((Current == n) && (Next == Space))
			{
				if(FirstNormalOffset == 0)
				{
					FirstNormalOffset = FilePosition + 4;
				}
				Mesh->NormalCount++;
			}

			if((Current == f) && (Next == Space))
			{
				if(FirstFaceOffset == 0)
				{
					FirstFaceOffset = FilePosition + 3;
				}

				if(FaceRows == 0)
				{
					// NOTE(Justin): To find the number of FaceCols count the number of
					// spaces in the first face row.
					for(char *C = (char *)Content; *C != '\n'; C++)
					{
						if(*C == Space)
						{
							FaceCols++; 
						}
					}
				}

				FaceRows++;
			}

			FilePosition++;
		}

		IndicesCount = 3 * FaceRows * FaceCols;
		Mesh->IndicesCount = IndicesCount;

		Mesh->Vertices = PushArray(Arena, Mesh->VertexCount, v3f);
		Mesh->UV = PushArray(Arena, Mesh->UVCount, v2f);
		Mesh->Normals = PushArray(Arena, Mesh->NormalCount, v3f);
		Mesh->Indices = PushArray(Arena, Mesh->IndicesCount, u32);

		// TODO(Justin): Find a better way to do this? Typically a color is
		// associated to each vertex so this seems reasonable...
		Mesh->Colors = PushArray(Arena, Mesh->VertexCount, v4f);

		Content = Result.Memory;
		Content += FirstVertexOffset;

		u8 *VertexData =  Content;
		ParseV3VertexAttribute(VertexData, Mesh->Vertices, Mesh->VertexCount, 3);

		Content = Result.Memory;
		Content += FirstTextureOffset;

		u8 *TextureData = Content;
		ParseV2VertexAttribute(TextureData, Mesh->UV, Mesh->UVCount, 2);

		Content = Result.Memory;
		Content += FirstNormalOffset;

		u8 *NormalData = Content;
		ParseV3VertexAttribute(NormalData, Mesh->Normals, Mesh->NormalCount, 3);

		Content = Result.Memory;
		Content += FirstFaceOffset;

		u8 *FaceData = Content;
		u32 Index = 0;
		char *C = (char *)FaceData;
		for(u32 Iteration = 0; Iteration < IndicesCount; ++Iteration)
		{
			b32 Updated = false;
			u32 Num = 0;
			while(CharIsNum(*C))
			{
				Num = Num * 10 + (*C) - '0';
				C++;
				Updated = true;
			}

			if(Updated)
			{
				Mesh->Indices[Index++] = Num - 1;
			}

			while(*C && !CharIsNum(*C))
			{
				C++;
			}
		}
	}

	return(Result);
}

#pragma pack(push, 1)
struct bitmap_header
{
	u16 FileType;
	u32 FileSize;
	u16 Reserved1;
	u16 Reserved2;
	u32 Offset;
	u32 Size;
	s32 Width;
	s32 Height;
	u16 Planes;
	u16 BitsPerPixel; 
	u32 Compression;
	u32 BitmapSize;
	s32 hResolution;
	s32 vResolution;  
	u32 ColorsUsed;
	u32 ColorsImportant;

	u32 RedMask;
	u32 GreenMask;
	u32 BlueMask;
};
#pragma pack(pop)

internal loaded_bitmap
DEBUGBitmapReadEntireFile(thread_context *Thread, char *Filename, debug_platform_read_entire_file *DEBUGPlatformReadEntireFile)
{
	loaded_bitmap Result = {};
	debug_file_read_result BitmapFile = DEBUGPlatformReadEntireFile(Thread, Filename);
	if(BitmapFile.Size != 0)
	{
		bitmap_header *BitmapHeader = (bitmap_header *)BitmapFile.Content;
		u32 *Pixels = (u32 *)((u8 *)BitmapFile.Content + BitmapHeader->Offset);
		Result.Memory = (void *)Pixels;
		Result.Width = BitmapHeader->Width;
		Result.Height = BitmapHeader->Height;
		Result.Stride = Result.Width * BITMAP_BYTES_PER_PIXEL;
		Result.Align = V2F((0.0f));

		Assert(BitmapHeader->Compression == 3);

		u32 RedMask = BitmapHeader->RedMask;
		u32 GreenMask = BitmapHeader->GreenMask;
		u32 BlueMask = BitmapHeader->BlueMask;
		u32 AlphaMask = ~(RedMask | GreenMask | BlueMask);

		bit_scan_result RedScan = U32FindFirstLeastSigBit(RedMask);
		bit_scan_result GreenScan = U32FindFirstLeastSigBit(GreenMask);
		bit_scan_result BlueScan = U32FindFirstLeastSigBit(BlueMask);
		bit_scan_result AlphaScan = U32FindFirstLeastSigBit(AlphaMask);

		Assert(RedScan.Found);
		Assert(GreenScan.Found);
		Assert(BlueScan.Found);
		Assert(AlphaScan.Found);

		s32 RedShiftDown = (s32)RedScan.Index;
		s32 GreenShiftDown = (s32)GreenScan.Index;
		s32 BlueShiftDown = (s32)BlueScan.Index;
		s32 AlphaShiftDown = (s32)AlphaScan.Index;

		u32 *Src = Pixels;
		for(s32 Y = 0; Y < BitmapHeader->Height; ++Y)
		{
			for(s32 X = 0; X < BitmapHeader->Width; ++X)
			{
				u32 R = ((*Src & RedMask) >> RedShiftDown);
				u32 G = ((*Src & GreenMask) >> GreenShiftDown);
				u32 B = ((*Src & BlueMask) >> BlueShiftDown);
				u32 A = ((*Src & AlphaMask) >> AlphaShiftDown);

				u32 Color = ((A << 24) | (R << 16) | (G << 8) | (B << 0));

				*Src++ = Color;
			}
		}
	}

	return(Result);
}

internal void
EntityOffsetAndCheckFrequency(app_state *AppState, v3f OffsetForFrame, aabb CameraBounds)
{
	for(u32 EntityIndex = 1; EntityIndex < AppState->EntityHighCount; )
	{
		high_entity *EntityHigh = AppState->EntitiesHigh_ + EntityIndex;
		EntityHigh->Basis.O -= OffsetForFrame;
		EntityHigh->Translate = Mat4Translation(OffsetForFrame) * EntityHigh->Translate;
		v3f Offset = Mat4Column(EntityHigh->Translate, 3).xyz;
		if(IsInAABB(CameraBounds, Offset))
		{
			EntityIndex++;
		}
		else
		{
			EntityFrequencySetToLow(AppState, EntityIndex);
		}
	}
}

internal void
CameraSet(app_state *AppState, world_position NewCameraP)
{
	world *World = &AppState->World;

	v3f CameraDelta = WorldPosDifference(World, &NewCameraP, &AppState->CameraP);
	v3f EntityOffsetForFrame = -1.0f * CameraDelta;

	AppState->CameraP = NewCameraP;
	AppState->Camera.P.x = (f32)NewCameraP.PackedX;
	AppState->Camera.P.y = (f32)NewCameraP.PackedY;
	AppState->Camera.P.z = -(f32)NewCameraP.PackedZ;

	// TODO(Justin): Formalize these. They are chosen arbitrarily.
	s32 TileXSpan = 17;
	s32 TileYSpan = 5;
	s32 TileZSpan = 17;

	aabb CameraBounds = AABBCenterDim(V3F(0.0f, -12.0f, 0.0f), World->TileSideInMeters * V3F((f32)TileXSpan,
																							 (f32)TileYSpan,
																							 (f32)TileZSpan));


	EntityOffsetAndCheckFrequency(AppState, EntityOffsetForFrame, CameraBounds);

	s32 TileXMin = NewCameraP.PackedX - TileXSpan / 2;
	s32 TileYMin = -TileYSpan / 2;
	s32 TileZMin = NewCameraP.PackedZ - TileZSpan / 2;

	s32 TileXMax = NewCameraP.PackedX + TileXSpan / 2;
	s32 TileYMax = TileYSpan / 2;
	s32 TileZMax = NewCameraP.PackedZ + TileZSpan / 2;
	for(u32 EntityIndex = 1; EntityIndex < AppState->EntityLowCount; ++EntityIndex)
	{
		low_entity *EntityLow = AppState->EntitiesLow + EntityIndex;
		if(EntityLow->HighIndex == 0)
		{
			if((EntityLow->P.PackedX >= TileXMin) &&
			   (EntityLow->P.PackedY >= TileYMin) &&
			   (EntityLow->P.PackedZ >= TileZMin) &&
			   (EntityLow->P.PackedX <= TileXMax) &&
			   (EntityLow->P.PackedY <= TileYMax) &&
			   (EntityLow->P.PackedZ <= TileZMax))
			{
				EntityFrequencySetToHigh(AppState, EntityIndex);
			}
		}
	}
}

internal void
CameraInit(app_state *AppState, v3f P, f32 Yaw, f32 Pitch)
{
	camera *Camera = &AppState->Camera;

	// TODO(Justin): Should the camera's position be a WORLD position?
	Camera->P = P;
	AppState->CameraP.PackedX = 17/2;
	AppState->CameraP.PackedY = 12;
	AppState->CameraP.PackedZ = 0;
	AppState->CameraIsFree = false;

	Camera->Yaw = Yaw;
	Camera->Pitch = Pitch;
	Camera->Direction.x = Cos(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));
	Camera->Direction.y = Sin(DegreeToRad(Camera->Pitch));
	Camera->Direction.z = Sin(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));

	// NOTE(Justin): These packed values are the center of a "Screen", NOT a
	// chunk.

	AppState->MapToCamera = Mat4CameraMap(Camera->P, Camera->P + Camera->Direction);
}

internal void
CameraUpdate(app_state *AppState, camera *Camera, f32 dMouseX, f32 dMouseY, f32 dt)
{
	f32 Sensitivity = 0.5f;

	f32 dX = dMouseX * dt * Sensitivity;
	f32 dY = dMouseY * dt * Sensitivity;

	// TODO(Justin): If camera is free then update direction else dont.
	f32 NewYaw = Camera->Yaw + dX;
	f32 NewPitch = Camera->Pitch + dY;

	NewPitch = Clamp(-89.0f, NewPitch, 89.0f);

	Camera->Yaw = NewYaw;
	Camera->Pitch = NewPitch;

	Camera->Direction.x = Cos(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));
	Camera->Direction.y = Sin(DegreeToRad(Camera->Pitch));
	Camera->Direction.z = Sin(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));

	Camera->Direction = Normalize(Camera->Direction);

	AppState->MapToCamera = Mat4CameraMap(Camera->P, Camera->P + Camera->Direction);
}

internal mesh *
MeshAllocate(memory_arena *Arena, u32 VertexCount, u32 UVCount,
								  u32 NormalCount, u32 IndicesCount, u32 ColorCount)
{
	mesh *Mesh = PushStruct(Arena, mesh);
	Mesh->Vertices = PushArray(Arena, VertexCount, v3f);
	Mesh->UV = PushArray(Arena, UVCount, v2f);
	Mesh->Normals = PushArray(Arena, NormalCount, v3f);
	Mesh->Indices = PushArray(Arena, IndicesCount, u32);
	Mesh->Colors = PushArray(Arena, ColorCount, v4f);
	Mesh->Texture = PushStruct(Arena, loaded_bitmap);

	Mesh->VertexCount = VertexCount;
	Mesh->UVCount = UVCount;
	Mesh->NormalCount = NormalCount;
	Mesh->IndicesCount = IndicesCount;
	Mesh->ColorCount = ColorCount;

	return(Mesh);
}

internal void
MeshCopy(mesh *Src, mesh *Dest)
{
	Assert(Src);
	Assert(Dest);

	Dest->Vertices = (v3f *)ARRAY_COPY(Src->VertexCount, Src->Vertices, Dest->Vertices);
	Dest->UV = (v2f *)ARRAY_COPY(Src->UVCount, Src->UV, Dest->UV);
	Dest->Normals = (v3f *)ARRAY_COPY(Src->NormalCount, Src->Normals, Dest->Normals);
	Dest->Colors = (v4f *)ARRAY_COPY(Src->ColorCount, Src->Colors, Dest->Colors);
	Dest->Indices = (u32 *)ARRAY_COPY(Src->IndicesCount, Src->Indices, Dest->Indices);
}

extern "C" APP_UPDATE_AND_RENDER(AppUpdateAndRender)
{
	Platform = Memory->PlatformAPI;

	Assert(sizeof(app_state) <= Memory->PermanentStorageSize);
	app_state *AppState = (app_state *)Memory->PermanentStorage;

	if(!Memory->IsInitialized)
	{
		EntityLowAdd(AppState, EntityType_Null);

		memory_arena *WorldArena = &AppState->WorldArena;
		ArenaInitialize(WorldArena, Memory->PermanentStorageSize - sizeof(app_state),
				(u8 *)Memory->PermanentStorage + sizeof(app_state));

		CameraInit(AppState, V3F(0.0, 12.0f, 0.0f), -90.0f, -70.0f);
		f32 FOV = DegreeToRad(45.0f);
		f32 AspectRatio = (f32)BackBuffer->Width / (f32)BackBuffer->Height;
		f32 ZNear = 0.1f;
		f32 ZFar = 100.0f;

		AppState->MapToPersp = Mat4PerspectiveGL(FOV, AspectRatio, ZNear, ZFar);
		AppState->MapToScreenSpace = Mat4ScreenSpaceMap(BackBuffer->Width, BackBuffer->Height);
		AppState->MapToWorld = Mat4WorldSpaceMap(V3F(0.0f, 0.0f, 0.0f));

		AppState->Cube = DEBUGObjReadEntireFile(Thread, "models/cube.obj", WorldArena, Platform.DEBUGPlatformReadEntireFile);
		AppState->Ground = DEBUGBitmapReadEntireFile(Thread, "textures/ground.bmp", Platform.DEBUGPlatformReadEntireFile);
		AppState->Gray = DEBUGBitmapReadEntireFile(Thread, "gray_with_boundary.bmp", Platform.DEBUGPlatformReadEntireFile);
		AppState->White = DEBUGBitmapReadEntireFile(Thread, "white_with_border.bmp", Platform.DEBUGPlatformReadEntireFile);
		AppState->Black = DEBUGBitmapReadEntireFile(Thread, "black.bmp", Platform.DEBUGPlatformReadEntireFile);

		mesh *SourceCube = &AppState->Cube.Mesh;
		SourceCube->Colors[0] = V4F(1.0f);

		mesh *WallCube = MeshAllocate(WorldArena, SourceCube->VertexCount,
												 SourceCube->UVCount,
												 SourceCube->NormalCount,
												 SourceCube->IndicesCount,
												 SourceCube->ColorCount);

		WallCube->Colors[0] = {1.0f, 1.0f, 0.0f, 1.0f};
		MeshCopy(SourceCube, WallCube);

		aabb BBox = AABBMinMax(SourceCube->Vertices, SourceCube->VertexCount);

		//mesh *QuadGround = &AppState->QuadGround;
		mesh *QuadGround = MeshAllocate(WorldArena, 4, 4, 4, 0, 4);

		QuadGround->Vertices[0] = V3F(-0.5f, 0.0, 0.5f);
		QuadGround->Vertices[1] = V3F(0.5f, 0.0, 0.5f);
		QuadGround->Vertices[2] = V3F(0.5f, 0.0, -0.5f);
		QuadGround->Vertices[3] = V3F(-0.5f, 0.0, -0.5f);

		QuadGround->UV[0] = V2F(0.0f, 0.0);
		QuadGround->UV[1] = V2F(1.0f, 0.0);
		QuadGround->UV[2] = V2F(1.0f, 1.0);
		QuadGround->UV[3] = V2F(0.0f, 1.0);

		QuadGround->Normals[0] = V3F(0.0f, 0.0, 1.0f);
		QuadGround->Normals[1] = V3F(0.0f, 0.0, 1.0f);
		QuadGround->Normals[2] = V3F(0.0f, 0.0, 1.0f);
		QuadGround->Normals[3] = V3F(0.0f, 0.0, 1.0f);

		v4f White = V4F(1.0f);
		QuadGround->Colors[0] = White; 
		QuadGround->Colors[1] = White;
		QuadGround->Colors[2] = White;
		QuadGround->Colors[3] = White;

		// TODO(Justin): Texture array and ids.
		QuadGround->Texture = &AppState->Gray;

		mat4 ScaleQuad = Mat4Identity();
		mat4 ScaleCube = Mat4Scale(0.5f);

		world *World = &AppState->World;
		
		World->ChunkShift = 4;
		World->ChunkMask = (1 << World->ChunkShift) - 1;
		World->ChunkDim = (1 << World->ChunkShift);
		World->ChunkCountX = 2;
		World->ChunkCountY = 1;
		World->ChunkCountZ = 2;

		World->TileSideInMeters = 1.0f;
		World->ChunkDimInMeters = World->ChunkDim * World->TileSideInMeters;

		AppState->MetersToPixels = 1.0f / World->TileSideInMeters;

		World->TileChunks = PushArray(WorldArena, World->ChunkCountX *
												  World->ChunkCountY *
												  World->ChunkCountZ, tile_chunk);

		s32 TilesInXPerScreen = 17;
		s32 TilesInZPerScreen = 9;
		s32 ScreenX = 0;
		s32 ScreenZ = 0;
		u32 RandomNumberIndex = 0;
		for(s32 ScreenIndex = 0; ScreenIndex < 1; ++ScreenIndex)
		{
			for(s32 TileY = 0; TileY < 1; ++TileY)
			{
				for(s32 TileZ = 0; TileZ < TilesInZPerScreen; ++TileZ)
				{
					for(s32 TileX = 0; TileX < TilesInXPerScreen; ++TileX)
					{
						s32 PackedX = ScreenX * TilesInXPerScreen + TileX;
						s32 PackedY = TileY;
						s32 PackedZ = ScreenZ * TilesInZPerScreen + TileZ;

						u32 TileValue = 1;
						if(TileY == 0)
						{
							if((TileX == 0) || (TileX == (TilesInXPerScreen - 1)))
							{
								if(TileZ == (TilesInZPerScreen / 2))
								{
									TileValue = 1;
								}
								else
								{
									TileValue = 2;
								}

							}
							if((TileZ == 0) || (TileZ == (TilesInZPerScreen - 1)))
							{
								if(TileX == (TilesInXPerScreen / 2))
								{
									TileValue = 1;
								}
								else
								{
									TileValue = 2;
								}
							}
						}
						else
						{
							TileValue = 0;
							if((TileX == 0) || (TileX == (TilesInXPerScreen - 1)))
							{
								if(TileZ != (TilesInZPerScreen / 2))
								{
									TileValue = 2;
								}

							}
							if((TileZ == 0) || (TileZ == (TilesInZPerScreen - 1)))
							{
								if(TileX != (TilesInXPerScreen / 2))
								{
									TileValue = 2;
								}
							}
						}

						TileValueSet(WorldArena, World, PackedX, PackedY, PackedZ, TileValue);
					}
				}
			}

			u32 RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
			if(RandomChoice == 0)
			{
				ScreenX++;
			}
			else
			{
				ScreenZ++;
			}
		}

		// NOTE(Justin): Add ground and wall entities for allocated tiles only



		for(s32 ChunkY = 0; ChunkY < World->ChunkCountY; ++ChunkY)
		{
			for(s32 ChunkZ = 0; ChunkZ < World->ChunkCountZ; ++ChunkZ)
			{
				for(s32 ChunkX = 0; ChunkX < World->ChunkCountX; ++ChunkX)
				{
					tile_chunk *TileChunk = TileChunkGet(World, ChunkX, ChunkY, ChunkZ);
					if(TileChunk && TileChunk->Tiles)
					{
						for(s32 TileY = 0; TileY < 1; ++TileY)
						{
							for(s32 TileZ = 0; TileZ < World->ChunkDim ; ++TileZ)
							{
								for(s32 TileX = 0; TileX < World->ChunkDim; ++TileX)
								{
									u32 TileValue = TileValueGet(World, TileChunk, TileX, TileY, TileZ);

									s32 PackedX = ChunkX * World->ChunkDim + TileX;
									s32 PackedY = ChunkY * World->ChunkDim + TileY;
									s32 PackedZ = ChunkZ * World->ChunkDim + TileZ;
									if(TileValue == 1)
									{
										QuadAdd(AppState, QuadGround, PackedX, PackedY, PackedZ, ScaleQuad);
									}
									else if(TileValue == 2)
									{
										ModelAdd(AppState, WallCube, PackedX, PackedY, PackedZ, ScaleCube, BBox);
									}
								}
							}
						}
					}
				}
			}
		}

		world_position NewCameraP = {};
		NewCameraP.PackedX = AppState->CameraP.PackedX;
		NewCameraP.PackedY = AppState->CameraP.PackedY;
		NewCameraP.PackedZ = AppState->CameraP.PackedZ;
		CameraSet(AppState, NewCameraP);

		PlayerAdd(AppState, BBox);

		Memory->IsInitialized = true;
	}

	Assert(sizeof(transient_state) <= Memory->TransientStorageSize);
	transient_state *TransientState = (transient_state *)Memory->TransientStorage;
	if(!TransientState->IsInitialized)
	{
		ArenaInitialize(&TransientState->TransientArena,
				Memory->TransientStorageSize - sizeof(transient_state),
				(u8 *)Memory->TransientStorage + sizeof(transient_state));

		TransientState->IsInitialized = true;
	}

	f32 dt = Input->dtForFrame;

	camera *Camera = &AppState->Camera;
	app_controller_input *KeyBoardController = &Input->Controllers[0];

	world *World = &AppState->World;

	// NOTE(Justin): C toggles fixed/free camera
	// TODO(Justin): Make the toggling between camera modes more robust.
	if(KeyBoardController->C.EndedDown)
	{
		AppState->CameraIsFree = !AppState->CameraIsFree;
	}

	v3f ddP = {};
	v3f Shift = {};
	v3f EntityOffsetForFrame = {};
	if(AppState->CameraIsFree)
	{
		if(KeyBoardController->W.EndedDown)
		{
			Shift += Camera->Direction;
		}
		if(KeyBoardController->S.EndedDown)
		{
			Shift += -1.0f * Camera->Direction;
		}
		if(KeyBoardController->A.EndedDown)
		{
			Shift += -1.0f * Cross(YAxis(), -1.0f *Camera->Direction);
		}
		if(KeyBoardController->D.EndedDown) 
		{
			Shift += Cross(YAxis(), -1.0f * Camera->Direction);
		}

		if(V3FNotZero(Shift))
		{
			Shift = Normalize(Shift);
		}

		f32 CameraSpeed = 8.0f;
		Shift = dt * CameraSpeed * Shift;

		Camera->P += Shift;
		CameraUpdate(AppState, Camera, Input->dMouseX, Input->dMouseY, dt);
	}
	else
	{
		if(KeyBoardController->W.EndedDown)
		{
			ddP += -1.0f * ZAxis();
		}
		if(KeyBoardController->S.EndedDown)
		{
			ddP += 1.0f * ZAxis();
		}
		if(KeyBoardController->A.EndedDown)
		{
			ddP += -1.0f * XAxis();
		}
		if(KeyBoardController->D.EndedDown) 
		{
			ddP += 1.0f * XAxis();
		}

		// TODO(Justin): Better way to clear the camera input.
		Input->dMouseX = 0;
		Input->dMouseY = 0;


		entity CameraFollowingEntity = EntityHighGet(AppState, AppState->CameraEntityFollowingIndex);
		if(CameraFollowingEntity.High)
		{
			v3f EntityOffsetFromCamera = Mat4Column(CameraFollowingEntity.High->Translate, 3).xyz;

			world_position NewCameraP = AppState->CameraP;

			if(EntityOffsetFromCamera.x > (9.0f * World->TileSideInMeters))
			{
				NewCameraP.PackedX += 17;
			}
			if(EntityOffsetFromCamera.x < (-9.0f * World->TileSideInMeters))
			{
				NewCameraP.PackedX -= 17;
			}

			if(EntityOffsetFromCamera.z < (-9.0f * World->TileSideInMeters))
			{
				NewCameraP.PackedZ += 9;
			}
			if(EntityOffsetFromCamera.z > (0.0f * World->TileSideInMeters))
			{
				NewCameraP.PackedZ -= 9;
			}

			// TODO(Justin): Map new entities in and old entities out

			CameraSet(AppState, NewCameraP);
			CameraUpdate(AppState, Camera, Input->dMouseX, Input->dMouseY, dt);
		}
	}

	//
	// NOTE(Justin): Render
	//

	mat4 MapToCamera = AppState->MapToCamera;
	mat4 MapToWorld = AppState->MapToWorld;
	mat4 MapToPersp = AppState->MapToPersp;
	mat4 Mat4MVP = MapToPersp * MapToCamera * MapToWorld;
	mat4 MapToScreen = Mat4ScreenSpaceMap(BackBuffer->Width, BackBuffer->Height);

	temporary_memory RenderMemory = TemporaryMemoryBegin(&TransientState->TransientArena);
	render_group *RenderGroup = RenderGroupAllocate(&TransientState->TransientArena, Gigabytes(2),
													AppState->MetersToPixels,
													MapToScreen,
													MapToPersp,
													MapToCamera,
													MapToWorld);

	PushClear(RenderGroup, V4F(0.0f, 0.0f, 0.0f, 0.0f));
	for(u32 EntityHighIndex = 1; EntityHighIndex < AppState->EntityHighCount; EntityHighIndex++)
	{
		entity Entity;
		Entity.High = AppState->EntitiesHigh_ + EntityHighIndex;
		Entity.LowIndex = Entity.High->LowIndex;
		Entity.Low = AppState->EntitiesLow + Entity.LowIndex;

		render_basis *Basis = PushStruct(&TransientState->TransientArena, render_basis);
		RenderGroup->DefaultBasis = Basis;
		switch(Entity.Low->Type)
		{
			case EntityType_Player:
			{
				EntityMove(AppState, Entity, ddP, dt);

				PushModel(RenderGroup, &Entity.Low->Mesh, Entity.High->Basis,
														  Entity.High->Translate,
														  Entity.Low->Scale); 
				basis B = Entity.High->Basis;
				B.O = {(f32)Entity.Low->P.PackedX, 0.0f, -(f32)Entity.Low->P.PackedZ};
				v3f Dim = {1.0f, 0.0f, 1.0f};
				v4f Color = {1.0f, 0.0f, 1.0f, 1.0f};
				PushRectangle(RenderGroup, B, Dim, Color);

			} break;
			case EntityType_Wall:
			{
				PushModel(RenderGroup, &Entity.Low->Mesh, Entity.High->Basis,
														  Entity.High->Translate,
														  Entity.Low->Scale); 
			} break;
			case EntityType_Quad:
			{
				PushQuad(RenderGroup, &Entity.Low->Mesh, Entity.High->Basis,
														 Entity.High->Translate,
														 Entity.Low->Scale);

				basis B = Entity.High->Basis;
				B.O = {(f32)Entity.Low->P.PackedX, 0.0f, -(f32)Entity.Low->P.PackedZ};
				v3f Dim = {0.1f, 0.0f, 0.1f};
				v4f Color = {1.0f, 1.0f, 0.0f, 1.0f};
				PushRectangle(RenderGroup, B, Dim, Color);
			} break;

			INVALID_DEFAULT_CASE;
		}
	}

	RenderToOutput(RenderGroup, BackBuffer);
	TemporaryMemoryEnd(RenderMemory);

}
