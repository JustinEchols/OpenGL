#if !defined(APP_RENDER_GROUP_H)

struct loaded_bitmap
{
	void *Memory;
	s32 Width;
	s32 Height;
	u32 Stride;

	v2f Align;
};

struct render_basis
{
	v3f P;
};

struct render_entity_basis
{
	render_basis *Basis;
	v3f Offset;
};

enum render_group_entry_type
{
	RENDER_GROUP_ENTRY_TYPE_render_entry_clear,
	RENDER_GROUP_ENTRY_TYPE_render_entry_rectangle,
	RENDER_GROUP_ENTRY_TYPE_render_entry_bitmap,
	RENDER_GROUP_ENTRY_TYPE_render_entry_coordinate_system,
	RENDER_GROUP_ENTRY_TYPE_render_entry_triangle,
	RENDER_GROUP_ENTRY_TYPE_render_entry_model,
	RENDER_GROUP_ENTRY_TYPE_render_entry_plane,
	RENDER_GROUP_ENTRY_TYPE_render_entry_quad,
};

struct render_group_entry_header
{
	render_group_entry_type Type;
};

struct render_entry_clear
{
	v4f Color;
};

struct render_entry_bitmap
{
	render_entity_basis EntityBasis;
	v4f Color;
	loaded_bitmap *Bitmap;
};

struct render_entry_rectangle
{
	render_entity_basis EntityBasis;

	basis Basis;
	v3f Dim;
	v4f Color;

};

struct render_entry_triangle
{
	render_entity_basis EntityBasis;
	v4f *Vertices;
	v3f *Colors;
};

struct render_entry_model
{
	render_entity_basis EntityBasis;

	basis Basis;
	mat4 Transform;

	u32 *Indices;
	v3f *Vertices;
	v2f *UV;
	v3f *Normals;
	v4f *Colors;
	
	u32 VertexCount;
	u32 UVCount;
	u32 NormalCount;
	u32 IndicesCount;
	u32 ColorCount;

	loaded_bitmap *Texture;
};

struct render_entry_plane
{
	render_entity_basis EntityBasis;
	v3f P;
	v3f N;
	v2f Dim;
};

struct render_entry_quad
{
	render_entity_basis EntityBasis;

	basis Basis;

	v3f *Vertices;
	v2f *UV;
	v4f *Colors;

	u32 VertexCount;

	loaded_bitmap *Texture;

};

struct render_entry_coordinate_system
{
	v2f Origin;
	v2f XAxis;
	v2f YAxis;
	v4f Color;

	loaded_bitmap *Texture;
	loaded_bitmap *NormalMap;
};


struct render_group 
{
	f32 MetersToPixels;
	mat4 MapToScreen;
	mat4 MapToPersp;
	mat4 MapToCamera;
	mat4 MapToWorld;

	render_basis *DefaultBasis;

	u32 PushBufferSize;
	u32 PushBufferMaxSize;
	u8 *PushBufferBase;
};

#define APP_RENDER_GROUP_H
#endif
