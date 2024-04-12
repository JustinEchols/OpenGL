
#include "app_render_group.h"

inline void
OpenGLRectangle(v2f MinP, v2f MaxP, v4f Color)
{
	glBegin(GL_TRIANGLES);

	Color = {1,1,1,1};
	glColor4f(Color.r, Color.g, Color.b, Color.a);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(MinP.x, MinP.y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(MaxP.x, MinP.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(MaxP.x, MaxP.y);

	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(MinP.x, MinP.y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(MaxP.x, MaxP.y);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(MinP.x, MaxP.y);

	glEnd();
}

internal void
OpenGLTriangle(v4f *Vertices, v3f *Colors)
{
	glBegin(GL_TRIANGLES);

	v4f V0 = Vertices[0];
	v4f V1 = Vertices[1];
	v4f V2 = Vertices[2];

	v3f C0 = Colors[0];
	v3f C1 = Colors[1];
	v3f C2 = Colors[2];

	glColor3f(C0.r, C0.g, C0.b);
	glVertex4f(V0.x, V0.y, V0.z, V0.w);

	glColor3f(C1.r, C1.g, C1.b);
	glVertex4f(V1.x, V1.y, V1.z, V1.w);

	glColor3f(C2.r, C2.g, C2.b);
	glVertex4f(V2.x, V2.y, V2.z, V2.w);

	glEnd();
}

internal void
OpenGLPoints(v3f *Points, u32 PointCount)
{
	glBegin(GL_POINTS);
	v3f C = V3F(1.0f);

	for(u32 Index = 0; Index < PointCount; ++Index)
	{
		v3f P = Points[Index];
		glColor3f(C.r, C.g, C.b);
		glVertex3f(P.x, P.y, P.z);
	}

	glEnd();
}

internal void
BasisDraw(basis *B)
{
	v3f O = B->O;
	v3f OX = O + B->U;
	v3f OY = O + B->V;
	v3f OZ = O + B->W;

	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(O.x, O.y, O.z);
	glVertex3f(OX.x, OX.y, OX.z);

	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(O.x, O.y, O.z);
	glVertex3f(OY.x, OY.y, OY.z);

	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(O.x, O.y, O.z);
	glVertex3f(OZ.x, OZ.y, OZ.z);

	glEnd();
}


internal void
OpenGLRenderGroupToOutput(render_group *RenderGroup, app_offscreen_buffer *OutputTarget)
{
	glViewport(0, 0, OutputTarget->Width, OutputTarget->Height);

#if 0
	GLuint TextureHandle = 0;
	static b32 Init = false;
	if(!Init)
	{
		glGenTextures(1, &TextureHandle);
		Init = true;
	}

	glBindTexture(GL_TEXTURE_2D, TextureHandle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,
			OutputTarget->Width, OutputTarget->Height,
			0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, OutputTarget->Memory);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glEnable(GL_TEXTURE_2D);
#endif

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	mat4 Mat4Persp = Mat4Transpose(RenderGroup->MapToPersp);
	glLoadMatrixf((f32 *)&Mat4Persp.e[0][0]);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	mat4 ModelView = Mat4Transpose(RenderGroup->MapToCamera * RenderGroup->MapToWorld);
	glLoadMatrixf((f32 *)&ModelView.e[0][0]);

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

				glClearColor(Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.a);
				glClear(GL_COLOR_BUFFER_BIT);

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

				v3f P = Entry->EntityBasis.Basis->P;

				glDisable(GL_TEXTURE_2D);
				OpenGLRectangle(P.xy, P.xy + Entry->Dim, Entry->Color);
				glEnable(GL_TEXTURE_2D);

				BaseAddress += sizeof(*Entry);

			} break;
			case RENDER_GROUP_ENTRY_TYPE_render_entry_triangle:
			{
				render_entry_triangle *Entry = (render_entry_triangle *)Data;

				OpenGLTriangle(Entry->Vertices, Entry->Colors);

				BaseAddress += sizeof(*Entry);

			} break;
			case RENDER_GROUP_ENTRY_TYPE_render_entry_model:
			{
				render_entry_model *Entry = (render_entry_model *)Data;

				// NOTE(Justin): If the model being rendered is the cube, then
				// must use GL_QUADS, if the model being rendered is the suzanne
				// model then use GL_TRIANGLES. How to we determine this
				// programatically? Why doesnt GL_TRIANGLES just work with the
				// cube?

				// NOTE(Justin):
				// Translation of the basis origin works as expected
				// Scaling the basis axes scales the model as expected
				basis *Basis = &Entry->Basis;
#if 1
				glBegin(GL_QUADS);
				for(u32 Index = 0; Index < Entry->FaceCount; Index += 3)
				{
					v3f V = Entry->Vertices[Entry->Faces[Index]];

					v3f VInB = Basis->O + V.x * Basis->U +
										  V.y * Basis->V +
										  V.z * Basis->W;

					v2f T = Entry->TexCoords[Entry->Faces[Index] + 1];
					v3f N = Entry->Normals[Entry->Faces[Index] + 2];

					glTexCoord2f(T.x, T.y);
					glVertex3f(VInB.x, VInB.y, VInB.z);
				}

				glEnd();
#endif

				//BasisDraw(Basis);

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
			case RENDER_GROUP_ENTRY_TYPE_render_entry_plane:
			{
			} break;

			INVALID_DEFAULT_CASE;
		}
	}
}
