
#include "app.h"

#define STBI_ASSERT(X) Assert(X)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "app_render_group.cpp"

internal entity *
EntityAdd(app_state *AppState, entity_type Type)
{
	Assert(AppState->EntityCount < ArrayCount(AppState->Entities));

	u32 EntityIndex = AppState->EntityCount++;
	entity *Entity = AppState->Entities + EntityIndex; 

	*Entity = {};
	Entity->Index = EntityIndex;
	Entity->Type = Type;

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

internal entity *
TriangleAdd(app_state *AppState, v4f Vertices[], v3f Colors[])
{
	entity *Entity = EntityAdd(AppState, ENTITY_TRIANGLE);

	Entity->Triangle.Vertices[0] = Vertices[0]; 
	Entity->Triangle.Vertices[1] = Vertices[1]; 
	Entity->Triangle.Vertices[2] = Vertices[2]; 
	Entity->Triangle.Colors[0] = Colors[0]; 
	Entity->Triangle.Colors[1] = Colors[1]; 
	Entity->Triangle.Colors[2] = Colors[2]; 

	return(Entity);
}

internal entity *
RectangleAdd(app_state *AppState, v4f Min, v4f Max)
{
	entity *Entity = EntityAdd(AppState, ENTITY_RECTANGLE);

	Entity->Rectangle.Min = Min;
	Entity->Rectangle.Max = Max;

	return(Entity);
}



internal entity *
ModelAdd(app_state *AppState, loaded_obj Model, mat4 Transform = Mat4Identity(), loaded_bitmap *Texture = 0)
{
	entity *Entity = EntityAdd(AppState, ENTITY_MODEL);

	// TODO(Justin): Init AABB of the model

	Entity->Mesh = Model.Mesh;
	Entity->Basis = BasisStandard();
	Entity->Transform = Transform;

	// TODO(Justin): Texture with loaded model

	return(Entity);
}

internal entity *
QuadAdd(app_state *AppState, mesh *Mesh)
{
	entity *Entity = EntityAdd(AppState, ENTITY_QUAD);

	Entity->Basis = BasisStandard();
	Entity->Mesh = *Mesh;
	
	return(Entity);
}


internal entity *
PlayerAdd(app_state *AppState, loaded_obj Model)
{
	entity *Entity = EntityAdd(AppState, ENTITY_PLAYER);
	Entity->Mesh = Model.Mesh;
	Entity->Basis = BasisStandard();

	return(Entity);
}


internal void
EntityMove(app_state *AppState, entity *Entity, v3f ddP, f32 dt)
{ 

	if(V3FNotZero(ddP))
	{
		ddP = Normalize(ddP);
	}

	f32 PlayerSpeed = 10.0f;
	ddP *= PlayerSpeed;
	ddP += -8.0f * Entity->dP;

	v3f PlayerDelta = 0.5f * ddP * Square(dt) + Entity->dP * dt;

	Entity->Basis.O += PlayerDelta;
	Entity->dP = Entity->dP + ddP * dt;
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
CameraInit(app_state *AppState)
{
	camera *Camera = &AppState->Camera;
	Camera->P = {0.0f, 10.0f, 0.0f};
	Camera->Yaw = -90.0f;
	Camera->Pitch = 0.0f;
	Camera->Direction.x = Cos(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));
	Camera->Direction.y = Sin(DegreeToRad(Camera->Pitch));
	Camera->Direction.z = Sin(DegreeToRad(Camera->Yaw)) * Cos(DegreeToRad(Camera->Pitch));

	AppState->MapToCamera = Mat4CameraMap(Camera->P, Camera->P + Camera->Direction);
}

internal void
CameraUpdate(app_state *AppState, app_offscreen_buffer *BackBuffer, camera *Camera, f32 dMouseX, f32 dMouseY, f32 dt)
{

	f32 Sensitivity = 0.5f;

	f32 dX = dMouseX * dt * Sensitivity;
	f32 dY = dMouseY * dt * Sensitivity;

	f32 NewYaw = Camera->Yaw + dX;
	f32 NewPitch = Camera->Pitch + dY;

	Clamp(-90.0f, NewPitch, 90.0f);

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

	Mesh->VertexCount = VertexCount;
	Mesh->UVCount = UVCount;
	Mesh->NormalCount = NormalCount;
	Mesh->IndicesCount = IndicesCount;
	Mesh->ColorCount = ColorCount;

	return(Mesh);
}

struct loaded_jpeg
{
	void *Memory;
	s32 Width;
	s32 Height;
};

extern "C" APP_UPDATE_AND_RENDER(AppUpdateAndRender)
{
	Platform = Memory->PlatformAPI;

	Assert(sizeof(app_state) <= Memory->PermanentStorageSize);
	app_state *AppState = (app_state *)Memory->PermanentStorage;

	if(!Memory->IsInitialized)
	{
		entity *EntityNull = EntityAdd(AppState, ENTITY_NULL);

		memory_arena *WorldArena = &AppState->WorldArena;
		ArenaInitialize(WorldArena, Memory->PermanentStorageSize - sizeof(app_state),
				(u8 *)Memory->PermanentStorage + sizeof(app_state));

		AppState->Cube = DEBUGObjReadEntireFile(Thread, "models/cube.obj", WorldArena, Platform.DEBUGPlatformReadEntireFile);
		AppState->Dodecahedron = DEBUGObjReadEntireFile(Thread, "models/dodecahedron.obj", WorldArena, Platform.DEBUGPlatformReadEntireFile);
		AppState->Pyramid = DEBUGObjReadEntireFile(Thread, "models/pyramid.obj", WorldArena, Platform.DEBUGPlatformReadEntireFile);
		AppState->Suzanne = DEBUGObjReadEntireFile(Thread, "models/suzanne.obj", WorldArena, Platform.DEBUGPlatformReadEntireFile);
		AppState->Test = DEBUGBitmapReadEntireFile(Thread, "textures/ground.bmp", Platform.DEBUGPlatformReadEntireFile);;

		AppState->MetersToPixels = 5.0f;

		CameraInit(AppState);

		f32 FOV = DegreeToRad(45.0f);
		f32 AspectRatio = (f32)BackBuffer->Width / (f32)BackBuffer->Height;
		f32 ZNear = 0.1f;
		f32 ZFar = 100.0f;

		AppState->MapToPersp = Mat4PerspectiveGL(FOV, AspectRatio, ZNear, ZFar);

		AppState->MapToScreenSpace = 
			Mat4ScreenSpaceMap(BackBuffer->Width, BackBuffer->Height);

		AppState->MapToWorld = Mat4WorldSpaceMap(V3F(0.0f, 0.0f, -1.0f));

		mesh *QuadMesh = MeshAllocate(WorldArena, 4, 4, 4, 0, 4);

		QuadMesh->Vertices[0] = V3F(-100.0f, 0.0, -1.0f);
		QuadMesh->Vertices[1] = V3F(100.0f, 0.0, -1.0f);
		QuadMesh->Vertices[2] = V3F(100.0f, 0.0, -200.0f);
		QuadMesh->Vertices[3] = V3F(-100.0f, 0.0, -200.0f);

		QuadMesh->UV[0] = V2F(0.0f, 0.0);
		QuadMesh->UV[1] = V2F(1.0f, 0.0);
		QuadMesh->UV[2] = V2F(1.0f, 1.0);
		QuadMesh->UV[3] = V2F(0.0f, 1.0);

		QuadMesh->Normals[0] = V3F(0.0f, 0.0, 1.0f);
		QuadMesh->Normals[1] = V3F(0.0f, 0.0, 1.0f);
		QuadMesh->Normals[2] = V3F(0.0f, 0.0, 1.0f);
		QuadMesh->Normals[3] = V3F(0.0f, 0.0, 1.0f);

		QuadMesh->Colors[0] = V4F(0.5f, 0.5f, 0.5f, 1.0f);
		QuadMesh->Colors[1] = V4F(0.5f, 0.5f, 0.5f, 1.0f);
		QuadMesh->Colors[2] = V4F(0.5f, 0.5f, 0.5f, 1.0f);
		QuadMesh->Colors[3] = V4F(0.5f, 0.5f, 0.5f, 1.0f);

		QuadMesh->Texture = &AppState->Test;
		QuadAdd(AppState, QuadMesh);


		//PlayerAdd(AppState, AppState->Cube);
		//ModelAdd(AppState, AppState->Suzanne, Mat4Translation(10.0f, 0.0f, 0.0f));
		//ModelAdd(AppState, AppState->Suzanne, Mat4Translation(-10.0f, 0.0f, 0.0f));

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
	AppState->MetersToPixels = 2.0f;

	f32 dt = Input->dtForFrame;

	camera *Camera = &AppState->Camera;
	app_controller_input *KeyBoardController = &Input->Controllers[0];

	v3f ddP = {};
#if 1
	v3f Shift = {};

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
		// NOTE(Justin): Translating the WORLD POSITION of the camera should
		// is NOT an INVERSE operation. Since the position of the camera is in
		// world space then when the camera moves to the right find the right
		// vector of the camera and add some scalar multiple of it to the
		// position.

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

	//
	// NOTE(Justin): Render
	//

	CameraUpdate(AppState, BackBuffer, Camera, Input->dMouseX, Input->dMouseY, dt);
	AppState->MapToCamera = Mat4CameraMap(Camera->P, Camera->P + Camera->Direction);

#else

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
#endif


	mat4 MapToCamera = AppState->MapToCamera;
	mat4 MapToWorld = AppState->MapToWorld;
	mat4 MapToPersp = AppState->MapToPersp;
	mat4 Mat4MVP = MapToPersp * MapToCamera * MapToWorld;
	mat4 MapToScreen = Mat4ScreenSpaceMap(BackBuffer->Width, BackBuffer->Height);


	temporary_memory RenderMemory = TemporaryMemoryBegin(&TransientState->TransientArena);
	render_group *RenderGroup = RenderGroupAllocate(&TransientState->TransientArena, Megabytes(1),
													AppState->MetersToPixels,
													MapToScreen,
													MapToPersp,
													MapToCamera,
													MapToWorld);

	PushClear(RenderGroup, V4F(0.0f, 0.0f, 0.0f, 0.0f));
	for(u32 EntityIndex = 1; EntityIndex < AppState->EntityCount; EntityIndex++)
	{
		entity *Entity = AppState->Entities + EntityIndex;

		render_basis *Basis = PushStruct(&TransientState->TransientArena, render_basis);
		RenderGroup->DefaultBasis = Basis;


		switch(Entity->Type)
		{
			case ENTITY_PLAYER:
			{
				EntityMove(AppState, Entity, ddP, dt);

				mat4 Translate = Mat4Identity();
				PushModel(RenderGroup, &Entity->Mesh, Entity->Basis, Translate); 
			} break;
			case ENTITY_TRIANGLE:
			{
				v4f *Vertices = (v4f *)Entity->Triangle.Vertices;
				v3f *Colors = (v3f *)Entity->Triangle.Colors;
				PushTriangle(RenderGroup, Vertices, V3F(0.0f, 0.0f, 0.0f), Colors);
			} break;
			case ENTITY_RECTANGLE:
			{
				PushRectangle(RenderGroup, V3F(0.0f), V2F(10.0f));
			}
			case ENTITY_MODEL:
			{
				PushModel(RenderGroup, &Entity->Mesh, Entity->Basis, Entity->Transform); 
			} break;
			case ENTITY_WALL:
			{
			} break;
			case ENTITY_QUAD:
			{
				PushQuad(RenderGroup, Entity->Mesh.Texture, Entity->Basis,
						Entity->Mesh.Vertices, Entity->Mesh.UV, Entity->Mesh.Colors, QUAD_VERTEX_COUNT);
			} break;

			INVALID_DEFAULT_CASE;
		}



	}

	RenderToOutput(RenderGroup, BackBuffer);
	TemporaryMemoryEnd(RenderMemory);

}
