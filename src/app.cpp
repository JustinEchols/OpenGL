
#include "app.h"
#include "app_tile.cpp"
#include "app_random.h"
#include "app_render_group.cpp"

internal u32
EntityAdd(app_state *AppState, entity_type Type)
{
	u32 EntityIndex = AppState->EntityCount++;

	Assert(AppState->EntityCount < ArrayCount(AppState->EntitiesLow));
	Assert(AppState->EntityCount < ArrayCount(AppState->EntitiesHigh));

	AppState->EntityResidence[EntityIndex] = EntityResidence_Low;
	AppState->EntitiesLow[EntityIndex] = {};
	AppState->EntitiesHigh[EntityIndex] = {};
	AppState->EntitiesLow[EntityIndex].Type = Type;

	return(EntityIndex);
}

internal void
EntityResidenceChange(app_state *AppState, entity_residence Residence, u32 EntityIndex)
{
	if(Residence == EntityResidence_High)
	{
		if(AppState->EntityResidence[EntityIndex] != EntityResidence_High)
		{
			high_entity *EntityHigh = AppState->EntitiesHigh + EntityIndex;
			low_entity *EntityLow = AppState->EntitiesLow + EntityIndex;

			// NOTE(Justin): The entities basis is going to be the camera's
			// position. The translation matrix translates the entities position
			// from the camera's world position to a world position relative to
			// the camera's position.

			v3f Translate = WorldPosDifference(&AppState->World, &EntityLow->P, &AppState->CameraP);
			v3f CameraP = {(f32)AppState->CameraP.PackedX, (f32)AppState->CameraP.PackedY, (f32)AppState->CameraP.PackedZ};
			EntityHigh->Basis.O = CameraP;
			EntityHigh->Translate = Mat4Translation(Translate);
		}
	}

	AppState->EntityResidence[EntityIndex] = Residence;
}

internal entity
EntityGet(app_state *AppState, entity_residence Residence, u32 Index)
{
	entity Entity = {};

	if((Index >= 0) && (Index < AppState->EntityCount))
	{
		if(AppState->EntityResidence[Index] < Residence)
		{
			EntityResidenceChange(AppState, Residence, Index);
			Assert(AppState->EntityResidence[Index] >= Residence);
		}

		Entity.Residence = Residence;
		Entity.Low = AppState->EntitiesLow + Index;
		Entity.High = AppState->EntitiesHigh + Index;
	}

	return(Entity);
}

internal basis
BasisStandard(void)
{
	basis Result = {};

	Result.O = V3F(0.0f);
	Result.U = XAxis();
	Result.V = YAxis();
	Result.W = ZAxis();

	return(Result);
}

internal void
PlayerAdd(app_state *AppState, aabb AABB)
{
	u32 EntityIndex = EntityAdd(AppState, EntityType_Player);
	entity Entity = EntityGet(AppState, EntityResidence_Low, EntityIndex);

	world *World = &AppState->World;

	Entity.Low->Collides = true;
	Entity.Low->P.PackedX = 8;
	Entity.Low->P.PackedY = 0;
	Entity.Low->P.PackedZ = 5;

	// NOTE(Justin): Curently not used
	Entity.Low->Width = 0;
	Entity.Low->Height = 0.35f;
	Entity.Low->Depth = 0;

	Entity.Low->P.OffsetFromTileCenter_.y = Entity.Low->Height;

	Entity.High->Basis = BasisStandard();
	Entity.High->Mesh[0] = AppState->Cube.Mesh;
	Entity.High->Mesh[0].Colors[0] = {1.0f, 1.0f, 0.0f, 1.0f};
	Entity.High->AABB = AABB;
	Entity.High->Scale = Mat4Scale(Entity.Low->Height);

	EntityResidenceChange(AppState, EntityResidence_High, EntityIndex);
	if(EntityGet(AppState, EntityResidence_Low, AppState->CameraEntityFollowingIndex).Residence == EntityResidence_Low)
	{
		AppState->CameraEntityFollowingIndex = EntityIndex;
	}
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
ModelAdd(app_state *AppState, mesh *Mesh, s32 PackedX, s32 PackedY, s32 PackedZ,
		mat4 Scale, aabb AABB, loaded_bitmap *Texture = 0)
{
	u32 EntityIndex = EntityAdd(AppState, EntityType_Wall);
	entity Entity = EntityGet(AppState, EntityResidence_Low, EntityIndex);

	Entity.Low->Collides = true;
	Entity.Low->P.PackedX = PackedX;
	Entity.Low->P.PackedY = PackedY;
	Entity.Low->P.PackedZ = PackedZ;
	Entity.Low->Height = 0.5f;
	Entity.Low->P.OffsetFromTileCenter_.y = Entity.Low->Height;

	Entity.High->Mesh[0] = *Mesh;
	Entity.High->Basis = BasisStandard();
	Entity.High->AABB = AABB;
	Entity.High->Scale = Scale;

	// TODO(Justin): Parametrize this value or compute the AABB.
	// If the vertices of the model are already on the convex hull then we can
	// loop through the vertices and project the vertex onto 6 coordinate axes.
	// The max proj (coordinate) is the vertex furhters away the min proj
	// (coordinate) is the fursthers away in the opposite directon.

	// TODO(Justin): Texture with loaded model
}

internal void
QuadAdd(app_state *AppState, mesh *Mesh, s32 PackedX, s32 PackedY, s32 PackedZ, mat4 Scale)
{
	u32 EntityIndex = EntityAdd(AppState, EntityType_Quad);
	entity Entity = EntityGet(AppState, EntityResidence_Low, EntityIndex);
	
	Entity.Low->P.PackedX = PackedX;
	Entity.Low->P.PackedY = PackedY;
	Entity.Low->P.PackedZ = PackedZ;
	Entity.Low->Collides = false;

	Entity.High->Basis = BasisStandard();
	Entity.High->Mesh[0] = *Mesh;
	Entity.High->Scale = Scale;
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

// TODO(Justin): Try MK diff for AABB vs AABB collision detection

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

	b32 Collided = false;

	for(u32 Iteration = 0; Iteration < 4; ++Iteration)
	{
		f32 tMin = 1.0f;
		v3f WallNormal = V3F(0.0f);
		v3f DesiredPosition = OldPlayerP + PlayerDelta;
		for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; ++EntityIndex)
		{
			if(AppState->EntityResidence[EntityIndex] == EntityResidence_High)
			{
				low_entity *Low = AppState->EntitiesLow + EntityIndex;
				high_entity *High = AppState->EntitiesHigh + EntityIndex;
				if(High != Entity.High)
				{
					if(Low->Collides)
					{
						v3f TestP = Mat4Column(High->Translate, 3).xyz;
						v3f CurrentP = Mat4Column(Entity.High->Translate, 3).xyz;
						v3f RelP = CurrentP - TestP;

						// TODO(Justin): Paramterize the dimensions of the AABB.
						aabb MKSumAABB = AABBCenterDim(V3F(0.0f), V3F(1.7f));

						// NOTE(Justin): This is a point vs plane test that uses the Minkowski sum.
						// and works as follows. Assume
						// that a collision happens on the left face (the other
						// cases are similar). First the position of the
						// player in the wall cubes relative space is computed
						// by A - B. This means that in this relative space the
						// wall cube is the origin and the vector A - B is a
						// position vector in the relative space and is the player's
						// position. Then a bounding box is constructed at the
						// origin of the space. Moreoever this bounding box is
						// formed by doing the MK sum of the player bounding box and
						// enity bounding box. This is done so that we can turn the
						// problem of AABB vs AABB collision detection into point vs.
						// plane collision detction. The point being the
						// player's relative position and the plane being a face
						// of the expanded AABB.
						//
						// The minimum point of the AABB
						// is on the left face of the wall cube and is also a
						// point on the infinite plane of the left face. Now the
						// plane convention that is used is the convention that
						// the signed distance D is positive whenver the origin is
						// "below" the plane and D is negative when the origin is
						// "above" the plane. The normal of the left face is (-1,
						// 0, 0) therefore the signed distance of the left face from
						// the center is 0.85f because the dimensions of the
						// expanded AABB are (1.7, 1.7, 1.7) and 1.7/2 = 0.85, and by the convention we use for the plane
						// equation. Taking the dot product of (-1,0,0) and
						// the minimum point of the AABB correctly computes the
						// signed distance of the plane of the left face of the
						// expanded AABB. Once we have the relative
						// position and the distance D we compute the
						// intersection time t by solving the parametrized plane
						// equation. if a valid t is computed in the range [0,1]
						// we compute the new relative position of the player using t and the player's delta and then
						// check whether or not it is in the bounding box.
						// This check must be done because the plane test uses the mathematical
						// definition of a plane, which is inifinite in extent.
						// If we do not check whether or not the new relative position is
						// in the bounding box the player will collide with the left
						// face all the time. Therefore if the new relative position is
						// in the AABB we know for certain the player collided
						// with the wall and can update the player's position
						// and velocity accordingly.

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

	// NOTE(Justin): Map into tile space.
	Entity.Low->P = PlayerPosReCompute(World, AppState->CameraP, Entity.High);
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


inline v3f
V3FFromWorldPos(world *World, world_position P)
{
	v3f Result;

	Result.x = World->ChunkDimInMeters * P.PackedX + P.OffsetFromTileCenter_.x;
	Result.y = World->ChunkDimInMeters * P.PackedX + P.OffsetFromTileCenter_.y;
	Result.z = -1.0f * World->ChunkDimInMeters * P.PackedX - P.OffsetFromTileCenter_.z;

	return(Result);
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
	s32 TileXSpan = 17*2;
	s32 TileYSpan = 10;
	s32 TileZSpan = 17*2;

	aabb CameraBounds = AABBCenterDim(V3F(0.0f, -12.0f, 0.0f), World->TileSideInMeters * V3F((f32)TileXSpan,
																							 (f32)TileYSpan,
																							 (f32)TileZSpan));

	for(u32 EntityIndex = 0; EntityIndex < ArrayCount(AppState->EntitiesHigh); ++EntityIndex)
	{
		if(AppState->EntityResidence[EntityIndex] == EntityResidence_High)
		{
			high_entity *EntityHigh = AppState->EntitiesHigh + EntityIndex;
			EntityHigh->Basis.O -= EntityOffsetForFrame;
			EntityHigh->Translate = Mat4Translation(EntityOffsetForFrame) * EntityHigh->Translate;
			v3f Offset = Mat4Column(EntityHigh->Translate, 3).xyz;
			if(!IsInAABB(CameraBounds, Offset))
			{
				EntityResidenceChange(AppState, EntityResidence_Low, EntityIndex);
			}
		}
	}

	s32 TileXMin = NewCameraP.PackedX - TileXSpan / 2;
	s32 TileYMin = -TileYSpan / 2;
	s32 TileZMin = NewCameraP.PackedZ - TileZSpan / 2;

	s32 TileXMax = NewCameraP.PackedX + TileXSpan / 2;
	s32 TileYMax = TileYSpan / 2;
	s32 TileZMax = NewCameraP.PackedZ + TileZSpan / 2;
	for(u32 EntityIndex = 0; EntityIndex < ArrayCount(AppState->EntitiesLow); ++EntityIndex)
	{
		low_entity *EntityLow = AppState->EntitiesLow + EntityIndex;
		if(AppState->EntityResidence[EntityIndex] == EntityResidence_Low)
		{
			if((EntityLow->P.PackedX >= TileXMin) &&
			   (EntityLow->P.PackedY >= TileYMin) &&
			   (EntityLow->P.PackedZ >= TileZMin) &&
			   (EntityLow->P.PackedX <= TileXMax) &&
			   (EntityLow->P.PackedY <= TileYMax) &&
			   (EntityLow->P.PackedZ <= TileZMax))
			{
				EntityResidenceChange(AppState, EntityResidence_High, EntityIndex);
			}
		}
	}
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
		//entity *EntityNull = EntityAdd(AppState, ENTITY_NULL);

		memory_arena *WorldArena = &AppState->WorldArena;
		ArenaInitialize(WorldArena, Memory->PermanentStorageSize - sizeof(app_state),
				(u8 *)Memory->PermanentStorage + sizeof(app_state));

		AppState->Cube = DEBUGObjReadEntireFile(Thread, "models/cube.obj", WorldArena, Platform.DEBUGPlatformReadEntireFile);
		AppState->Ground = DEBUGBitmapReadEntireFile(Thread, "textures/ground.bmp", Platform.DEBUGPlatformReadEntireFile);
		AppState->Gray = DEBUGBitmapReadEntireFile(Thread, "gray_with_boundary.bmp", Platform.DEBUGPlatformReadEntireFile);
		AppState->White = DEBUGBitmapReadEntireFile(Thread, "white_with_border.bmp", Platform.DEBUGPlatformReadEntireFile);
		AppState->Black = DEBUGBitmapReadEntireFile(Thread, "black.bmp", Platform.DEBUGPlatformReadEntireFile);

		mesh SourceCube = AppState->Cube.Mesh;
		mesh *WallCube = MeshAllocate(WorldArena, SourceCube.VertexCount,
												  SourceCube.UVCount,
												  SourceCube.NormalCount,
												  SourceCube.IndicesCount,
												  SourceCube.ColorCount);

		MeshCopy(&SourceCube, WallCube);
		WallCube->Colors[0] = V4F(1.0f);

		aabb BBox = AABBMinMax(WallCube->Vertices, WallCube->VertexCount);

		mesh *QuadGround = MeshAllocate(WorldArena, 4, 4, 4, 0, 4);

		// TODO(Justin): Obj file for quad
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
		World->ChunkCountX = 8;
		World->ChunkCountY = 1;
		World->ChunkCountZ = 8;

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
		for(s32 ScreenIndex = 0; ScreenIndex < 5; ++ScreenIndex)
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


		PlayerAdd(AppState, BBox);
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

		CameraInit(AppState, V3F(0.0, 12.0f, 0.0f), -90.0f, -70.0f);
		world_position NewCameraP;
		NewCameraP.PackedX = AppState->CameraP.PackedX;
		NewCameraP.PackedY = AppState->CameraP.PackedY;
		NewCameraP.PackedZ = AppState->CameraP.PackedZ;


		CameraSet(AppState, NewCameraP);

		f32 FOV = DegreeToRad(45.0f);
		f32 AspectRatio = (f32)BackBuffer->Width / (f32)BackBuffer->Height;
		f32 ZNear = 0.1f;
		f32 ZFar = 100.0f;

		AppState->MapToPersp = Mat4PerspectiveGL(FOV, AspectRatio, ZNear, ZFar);
		AppState->MapToScreenSpace = Mat4ScreenSpaceMap(BackBuffer->Width, BackBuffer->Height);
		AppState->MapToWorld = Mat4WorldSpaceMap(V3F(0.0f, 0.0f, 0.0f));


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


		entity CameraFollowingEntity = EntityGet(AppState, EntityResidence_High, AppState->CameraEntityFollowingIndex);
		if(CameraFollowingEntity.Residence != EntityResidence_NonExistant)
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
	for(u32 EntityIndex = 0; EntityIndex < AppState->EntityCount; EntityIndex++)
	{
		if(AppState->EntityResidence[EntityIndex] == EntityResidence_High)
		{
			entity Entity = EntityGet(AppState, EntityResidence_High, EntityIndex);

			render_basis *Basis = PushStruct(&TransientState->TransientArena, render_basis);
			RenderGroup->DefaultBasis = Basis;

			switch(Entity.Low->Type)
			{
				case EntityType_Player:
				{
					EntityMove(AppState, Entity, ddP, dt);

					PushModel(RenderGroup, &Entity.High->Mesh[0], Entity.High->Basis,
																  Entity.High->Translate,
																  Entity.High->Scale); 

					chunk_tile_position ChunkP = ChunkPosGet(World, Entity.Low->P.PackedX,
																	Entity.Low->P.PackedY,
																	Entity.Low->P.PackedZ);

					basis B = Entity.High->Basis;
					B.O = {(f32)Entity.Low->P.PackedX, 0.0f, -(f32)Entity.Low->P.PackedZ};
					v3f Dim = {1.0f, 0.0f, 1.0f};
					v4f Color = {1.0f, 0.0f, 1.0f, 1.0f};
					PushRectangle(RenderGroup, B, Dim, Color);

				} break;
				case EntityType_Wall:
				{
					PushModel(RenderGroup, &Entity.High->Mesh[0], Entity.High->Basis,
															      Entity.High->Translate,
																  Entity.High->Scale); 
				} break;
				case EntityType_Quad:
				{
					mesh *Mesh = &Entity.High->Mesh[0];
					PushQuad(RenderGroup, Mesh->Texture,
							Entity.High->Translate, Entity.High->Scale, Entity.High->Basis,
							Mesh->Vertices, Mesh->UV, Mesh->Colors, QUAD_VERTEX_COUNT);

					basis B = Entity.High->Basis;
					B.O = {(f32)Entity.Low->P.PackedX, 0.0f, -(f32)Entity.Low->P.PackedZ};
					v3f Dim = {0.1f, 0.0f, 0.1f};
					v4f Color = {1.0f, 1.0f, 0.0f, 1.0f};
					PushRectangle(RenderGroup, B, Dim, Color);
				} break;

				INVALID_DEFAULT_CASE;
			}
		}
	}

	RenderToOutput(RenderGroup, BackBuffer);
	TemporaryMemoryEnd(RenderMemory);

}
