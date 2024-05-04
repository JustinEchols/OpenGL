#if !defined(WIN32_APP_H)

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Stride;
};

struct win32_client_dimensions
{
	int Width;
	int Height;
};

struct win32_sound_output
{
	int SamplesPerSecond;
	int BytesPerSample;
	u32 RunningSampleIndex;
	DWORD SecondaryBufferSize;
	DWORD SafetyButes;
};


struct win32_app_code
{
	HMODULE DLL;
	FILETIME DLLLastWriteTime;

	app_update_and_render *UpdateAndRender;
	app_get_sound_samples *GetSoundSamples;

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
