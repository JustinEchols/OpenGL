#if !defined(WIN32_APP_H)

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	s32 Width;
	s32 Height;
	s32 Stride;
	void *Memory;
};

struct win32_client_dimensions
{
	s32 Width;
	s32 Height;
};


struct win32_app_code
{
	HMODULE DLL;
	FILETIME DLLLastWriteTime;
	app_update_and_render *UpdateAndRender;

	b32 IsValid;
};

#define WIN32_STATE_FILE_NAME_COUNT MAX_PATH
struct win32_state
{
	u64 TotalSize;
	void *AppMemoryBlock;

	char ExePath[WIN32_STATE_FILE_NAME_COUNT];
	char *OnePastLastSlash;
};

#define WIN32_APP_H
#endif
