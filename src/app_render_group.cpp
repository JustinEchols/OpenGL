

internal void
RectangleDraw(app_offscreen_buffer *Buffer, v4f Color)
{
	u32 Color32 = (((u32)(255.0f * Color.x + 0.5f) << 16) |
				    ((u32)(255.0f * Color.y + 0.5f) << 8) | 
				    ((u32)(255.0f * Color.z + 0.5f) << 0));

	u32 *Pixel = (u32 *)Buffer->Memory;
	for(s32 Y = 0; Y < Buffer->Height; ++Y)
	{
		for(s32 X = 0; X < Buffer->Width; ++X)
		{
			*Pixel++ = Color32;
		}
	}
}

internal void
PixelSet(app_offscreen_buffer *Buffer, v2f PXY, v3f Color)
{
	s32 X = F32RoundToS32(PXY.x);
	s32 Y = F32RoundToS32(PXY.y);

	if(X < 0)
	{
		X = 0;
	}
	if(X >= Buffer->Width)
	{
		X = Buffer->Width - 1;
	}
	if(Y < 0)
	{
		Y = 0;
	}
	if(Y >= Buffer->Height)
	{
		Y = Buffer->Height - 1;
	}

	u8 *Row = (u8 *)Buffer->Memory + Y * Buffer->Stride + X * 4;
	u32 *Pixel = (u32 *)Row;
	*Pixel = ((F32RoundToU32(255.0f * Color.r + 0.5f)) << 16) |
			  ((F32RoundToU32(255.0f * Color.g + 0.5f)) << 8) |
			  ((F32RoundToU32(255.0f * Color.b + 0.5f)) << 0);
}

internal edge
EdgeFromV3F(v3f VertexMin, v3f VertexMax)
{
	edge Result;

	Result.YStart = F32RoundToS32(VertexMin.y);
	Result.YEnd = F32RoundToS32(VertexMax.y);

	f32 YDist = VertexMax.y - VertexMin.y;
	f32 XDist = VertexMax.x - VertexMin.x;

	Result.XStep = (f32)XDist / (f32)YDist;

	f32 YPreStep = Result.YStart - VertexMin.y;

	Result.X = VertexMin.x + YPreStep * Result.XStep;
	return(Result);
}

internal v3f
Barycentric(v3f X, v3f Y, v3f Z, v3f P)
{
	//
	// NOTE(Justin): The computation of finding the cood. is based on the area
	// interpretation of Barycentric coordinates.
	//
	
	v3f Result = {};
	v3f E1 = Z - X;
	v3f E2 = Y - X;
	v3f F = P - X;

	v3f Beta = {};
	v3f Gamma = {};

	Beta = (Dot(E2, E2) * E1 - Dot(E1, E2) * E2); 
	f32 cBeta = 1.0f / (Dot(E1, E1) * Dot(E2, E2) - SQUARE(Dot(E1, E2)));
	Beta = cBeta * Beta; 

	Gamma = (Dot(E1, E1) * E2 - Dot(E1, E2) * E1); 
	f32 cGamma = 1.0f / (Dot(E1, E1) * Dot(E2, E2) - SQUARE(Dot(E1, E2)));
	Gamma = cGamma * Gamma;

	Result.x = Dot(Beta, F);
	Result.y = Dot(Gamma, F);
	Result.z = 1 - Result.x - Result.y;

	return(Result);
}

internal v3f
BarycentricV2(v2f V0, v2f V1, v2f V2, v2f P)
{
	v3f Result = {};

	f32 SignedDoubleArea = Det(V1 - V0, V2 - V0);

	if(SignedDoubleArea != 0.0f)
	{
		f32 E01 = Det((V1 - V0), (P - V0));
		f32 E12 = Det((V2 - V1), (P - V1));
		f32 E20 = Det((V0 - V2), (P - V2));

		f32 C1 = E01 / SignedDoubleArea;
		f32 C2 = E12 / SignedDoubleArea;
		f32 C3 = E20 / SignedDoubleArea;

		Result = V3F(C1, C2, C3);
	}

	return(Result);
}

internal void
TriangleDraw(app_offscreen_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, v4f A, v4f B, v4f C)
{
	A = Mat4MVP * A;
	A = (1.0f / A.w) * A;
	A = Mat4ScreenSpace * A;

	B = Mat4MVP * B;
	B = (1.0f / B.w) * B;
	B = Mat4ScreenSpace * B;

	C = Mat4MVP * C;
	C = (1.0f / C.w) * C;
	C = Mat4ScreenSpace * C;

	v2f Tmp = {};
	v2f V0 = A.xy;
	v2f V1 = B.xy;
	v2f V2 = C.xy;

	v2f E1 = V1 - V0;
	v2f E2 = V2 - V0;
	f32 SignedDoubleArea = Det(E1, E2);
	b32 Swapped = false;
	if(SignedDoubleArea < 0)
	{
		Tmp = V1;
		V1 = V2;
		V2 = Tmp;
		Swapped = true;
	}

	s32 XMin = F32RoundToS32(Min3(V0.x, V1.x, V2.x));
	s32 YMin = F32RoundToS32(Min3(V0.y, V1.y, V2.y));
	s32 XMax = F32RoundToS32(Max3(V0.x, V1.x, V2.x));
	s32 YMax = F32RoundToS32(Max3(V0.y, V1.y, V2.y));

	for(s32 Y = YMin; Y < YMax; ++Y)
	{
		for(s32 X = XMin; X < XMax; ++X)
		{
			v2f P = {(f32)X, (f32)Y};

			f32 E01 = Det((V1 - V0), (P - V0));
			f32 E12 = Det((V2 - V1), (P - V1));
			f32 E20 = Det((V0 - V2), (P - V2));

			if((E01 > 0) && (E12 > 0) && (E20 > 0))
			{
				v3f Barycentric = {};
				if(Swapped)
				{
					Barycentric = BarycentricV2(V0, V2, V1, P);
				}
				else
				{
					Barycentric = BarycentricV2(V0, V1, V2, P);
				}

				PixelSet(AppBackBuffer, P, Barycentric);
			}
		}
	}
}


internal void
TriangleDraw(app_offscreen_buffer *AppBackBuffer, mat4 Mat4MVP, mat4 Mat4ScreenSpace, triangle *Triangle)
{
	triangle Fragment = {};
	for(u32 Index = 0; Index < 3; Index++)
	{
		Fragment.Vertices[Index] = Mat4MVP * Triangle->Vertices[Index];
		Fragment.Vertices[Index] = (1.0f / Fragment.Vertices[Index].w) * Fragment.Vertices[Index];
		Fragment.Vertices[Index] = Mat4ScreenSpace * Fragment.Vertices[Index];

	}

	v2f Tmp = {};
	v2f V0 = Fragment.Vertices[0].xy;
	v2f V1 = Fragment.Vertices[1].xy;
	v2f V2 = Fragment.Vertices[2].xy;

	v2f E1 = V1 - V0;
	v2f E2 = V2 - V0;
	f32 SignedDoubleArea = Det(E1, E2);
	b32 Swapped = false;
	if(SignedDoubleArea < 0)
	{
		Tmp = V1;
		V1 = V2;
		V2 = Tmp;
		Swapped = true;
	}

	s32 XMin = F32RoundToS32(Min3(V0.x, V1.x, V2.x));
	s32 YMin = F32RoundToS32(Min3(V0.y, V1.y, V2.y));
	s32 XMax = F32RoundToS32(Max3(V0.x, V1.x, V2.x));
	s32 YMax = F32RoundToS32(Max3(V0.y, V1.y, V2.y));

	for(s32 Y = YMin; Y < YMax; ++Y)
	{
		for(s32 X = XMin; X < XMax; ++X)
		{
			v2f P = {(f32)X, (f32)Y};

			f32 E01 = Det((V1 - V0), (P - V0));
			f32 E12 = Det((V2 - V1), (P - V1));
			f32 E20 = Det((V0 - V2), (P - V2));

			if((E01 > 0) && (E12 > 0) && (E20 > 0))
			{
				v3f Barycentric = {};
				if(Swapped)
				{
					Barycentric = BarycentricV2(V0, V2, V1, P);
				}
				else
				{
					Barycentric = BarycentricV2(V0, V1, V2, P);
				}

				v3f Color = (Barycentric.x * Triangle->Colors[0] +
							Barycentric.y * Triangle->Colors[1] +
							Barycentric.z * Triangle->Colors[2]);
				PixelSet(AppBackBuffer, P, Color);
			}
		}
	}
}

inline mat4
RenderGroupGetViewingTransformation(render_group *RenderGroup)
{
	mat4 Result = RenderGroup->MapToScreen *
				  RenderGroup->MapToPersp *
				  RenderGroup->MapToCamera *
				  RenderGroup->MapToWorld;

	return(Result);
}

#define PushRenderElement(RenderGroup, type) (type *)PushRenderElement_(RenderGroup, sizeof(type), RENDER_GROUP_ENTRY_TYPE_##type)
inline void *
PushRenderElement_(render_group *RenderGroup, u32 Size, render_group_entry_type Type)
{
	void *Result = 0;

	Size += sizeof(render_group_entry_header);

	if((RenderGroup->PushBufferSize + Size) < RenderGroup->PushBufferMaxSize)
	{
		render_group_entry_header *Header = (render_group_entry_header *)(RenderGroup->PushBufferBase + RenderGroup->PushBufferSize);
		Header->Type = Type;
		Result = (u8 *)Header + sizeof(*Header);
		RenderGroup->PushBufferSize += Size;
	}
	else
	{
		INVALID_CODE_PATH;
	}

	return(Result);
}

inline void
PushRectangle(render_group *RenderGroup, basis B, v3f Dim, v4f Color = V4F(1.0f))
{
	render_entry_rectangle *Entry = PushRenderElement(RenderGroup, render_entry_rectangle);
	if(Entry)
	{
		Entry->Basis = B;
		Entry->Dim = Dim;
		Entry->Color = Color;
	}
}

internal void
PushTriangle(render_group *RenderGroup, v4f *Vertices, v3f Offset, v3f *Colors)
{
	render_entry_triangle *Entry = PushRenderElement(RenderGroup, render_entry_triangle);
	if(Entry)
	{
		mat4 M = RenderGroupGetViewingTransformation(RenderGroup);
		f32 MetersToPixels = RenderGroup->MetersToPixels;

		Entry->EntityBasis.Basis = RenderGroup->DefaultBasis;
		Entry->EntityBasis.Offset = MetersToPixels * Offset;
		Entry->Vertices = Vertices;
		Entry->Colors = Colors;
	}
}

internal void
PushModel(render_group *RenderGroup, mesh *Mesh, basis B, mat4 Translate, mat4 Scale)
{
	render_entry_model *Entry = PushRenderElement(RenderGroup, render_entry_model);
	if(Entry)
	{
		B.O  = Translate * B.O;
		B.U  = Scale * B.U;
		B.V  = Scale * B.V;
		B.W  = Scale * B.W;

		mat4 MetersToPixels = Mat4Scale(RenderGroup->MetersToPixels);

		B.O = MetersToPixels * B.O;
		B.U = MetersToPixels * B.U;
		B.V = MetersToPixels * B.V;
		B.W = MetersToPixels * B.W;

		Entry->Basis = B;
		Entry->Indices = Mesh->Indices;
		Entry->Vertices = Mesh->Vertices;
		Entry->UV = Mesh->UV;
		Entry->Normals = Mesh->Normals;
		Entry->Colors = Mesh->Colors;

		Entry->VertexCount = Mesh->VertexCount;
		Entry->UVCount = Mesh->UVCount;
		Entry->NormalCount = Mesh->NormalCount;
		Entry->IndicesCount = Mesh->IndicesCount;
		Entry->ColorCount = Mesh->ColorCount;

		Entry->Texture = Mesh->Texture;
	}
}

internal void
PushQuad(render_group *RenderGroup, loaded_bitmap *Texture, mat4 Translate, mat4 Scale,
		basis B, v3f *Vertices, v2f *UV, v4f *Colors, u32 VertexCount)
{
	render_entry_quad *Entry = PushRenderElement(RenderGroup, render_entry_quad);
	if(Entry)
	{
		B.O  = Translate * B.O;
		B.U  = Scale * B.U;
		B.V  = Scale * B.V;
		B.W  = Scale * B.W;

		mat4 MetersToPixels = Mat4Scale(RenderGroup->MetersToPixels);
		B.O = MetersToPixels * B.O;
		B.U = MetersToPixels * B.U;
		B.V = MetersToPixels * B.V;
		B.W = MetersToPixels * B.W;

		Entry->Basis = B;
		Entry->Vertices = Vertices;
		Entry->UV = UV;
		Entry->Colors = Colors;
		Entry->VertexCount = VertexCount;

		Entry->Texture = Texture;
	}
}

inline void
PushClear(render_group *RenderGroup, v4f Color)
{
	render_entry_clear *Entry = PushRenderElement(RenderGroup, render_entry_clear);
	if(Entry)
	{
		Entry->Color = Color;
	}
}

inline render_entry_coordinate_system * 
PushCoordinateSystem(render_group *RenderGroup, v2f Origin, v2f XAxis, v2f YAxis, v4f Color,
				 loaded_bitmap *Texture, loaded_bitmap *NormalMap)
{
	render_entry_coordinate_system *Entry = PushRenderElement(RenderGroup, render_entry_coordinate_system);
	if(Entry)
	{
		mat4 M = RenderGroupGetViewingTransformation(RenderGroup);
		f32 MetersToPixels = RenderGroup->MetersToPixels;

		Entry->Origin = Origin;
		Entry->XAxis = XAxis;
		Entry->YAxis = YAxis;
		Entry->Color = Color;
		Entry->Texture = Texture;
		Entry->NormalMap = NormalMap;
	}

	return(Entry);
}


internal void
RenderGroupToOutput(render_group *RenderGroup, app_offscreen_buffer *OutputTarget)
{
	v2f ScreenCenter = 0.5f * V2F((f32)OutputTarget->Width, (f32)OutputTarget->Height);

	f32 MetersToPixels = 1.0f / RenderGroup->MetersToPixels;
	mat4 M = RenderGroupGetViewingTransformation(RenderGroup);
	for(u32 BaseAddress = 0; BaseAddress < RenderGroup->PushBufferSize; )
	{
		render_group_entry_header *Header = (render_group_entry_header *)(RenderGroup->PushBufferBase + BaseAddress);
		BaseAddress += sizeof(*Header);

		void *Data = (u8 *)Header + sizeof(*Header);
		switch(Header->Type)
		{
			case RENDER_GROUP_ENTRY_TYPE_render_entry_clear:
			{
				render_entry_clear *Entry = (render_entry_clear *)Data;

				RectangleDraw(OutputTarget, Entry->Color);

				BaseAddress += sizeof(*Entry);
			} break;
			case RENDER_GROUP_ENTRY_TYPE_render_entry_bitmap:
			{
				render_entry_bitmap *Entry = (render_entry_bitmap *)Data;
				Assert(Entry->Bitmap);  
				BaseAddress += sizeof(*Entry);
			} break;
			case RENDER_GROUP_ENTRY_TYPE_render_entry_rectangle:
			{
				render_entry_rectangle *Entry = (render_entry_rectangle *)Data;
				BaseAddress += sizeof(*Entry);

			} break;
			case RENDER_GROUP_ENTRY_TYPE_render_entry_triangle:
			{

				render_entry_triangle *Entry = (render_entry_triangle *)Data;

				mat4 MVP = RenderGroup->MapToPersp *
						   RenderGroup->MapToCamera *
						   RenderGroup->MapToWorld;

				mat4 ScreenSpace = RenderGroup->MapToScreen;

				triangle T = {};
				T.Vertices[0] = Entry->Vertices[0];
				T.Vertices[1] = Entry->Vertices[1];
				T.Vertices[2] = Entry->Vertices[2];
				T.Colors[0] = Entry->Colors[0];
				T.Colors[1] = Entry->Colors[1];
				T.Colors[2] = Entry->Colors[2];

				TriangleDraw(OutputTarget, MVP, ScreenSpace, &T);
				//TriangleDraw(OutputTarget, MVP, ScreenSpace, Vertices[0], Vertices[1], Vertices[2]);
				BaseAddress += sizeof(*Entry);

			} break;
			case RENDER_GROUP_ENTRY_TYPE_render_entry_model:
			{
				render_entry_model *Entry = (render_entry_model *)Data;

				BaseAddress += sizeof(*Entry);

			} break;
			case RENDER_GROUP_ENTRY_TYPE_render_entry_coordinate_system:
			{
				render_entry_coordinate_system *Entry = (render_entry_coordinate_system *)Data;

				v4f Color = {1, 1, 0, 1};
				v2f Dim = {2, 2};

				v2f P = Entry->Origin;

				BaseAddress += sizeof(*Entry);

			} break;

			INVALID_DEFAULT_CASE;
		}
	}
}

internal render_group *
RenderGroupAllocate(memory_arena *Arena, u32 PushBufferMaxSize, f32 MetersToPixels,
		mat4 MapToScreen, mat4 MapToPersp, mat4 MapToCamera, mat4 MapToWorld)
{
	render_group *Result = PushStruct(Arena, render_group);

	Result->MetersToPixels = MetersToPixels;

	Result->MapToPersp = MapToPersp;
	Result->MapToScreen = MapToScreen;
	Result->MapToCamera = MapToCamera;
	Result->MapToWorld= MapToWorld;

	Result->DefaultBasis = PushStruct(Arena, render_basis);
	Result->DefaultBasis->P = V3F(0.0f);

	Result->PushBufferSize = 0;
	Result->PushBufferMaxSize = PushBufferMaxSize;
	Result->PushBufferBase = (u8 *)PushSize(Arena, PushBufferMaxSize);

	return(Result);
}

inline void
RenderToOutput(render_group *RenderGroup, app_offscreen_buffer *OutputTarget)
{
	b32 UsingHardware = true;
	if(UsingHardware)
	{
		Platform.RenderToOpenGL(RenderGroup, OutputTarget);
	}
	else
	{
		RenderGroupToOutput(RenderGroup, OutputTarget);
	}
}
