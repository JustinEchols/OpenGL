
#include "app_platform.h"
#include "app_intrinsics.h"
#include "app_math.h"

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <gl/gl.h>

#include "win32_app.h"
#include "app_opengl.cpp"


global_variable b32 GlobalRunning;
global_variable win32_offscreen_buffer Win32GlobalBackBuffer;
global_variable s64 GlobalTicksPerSecond;
global_variable WINDOWPLACEMENT GlobalWindowPos = {sizeof(GlobalWindowPos)};

global_variable f32 dMouseX;
global_variable f32 dMouseY;

internal void
Win32CenterCursor(HWND Window)
{
	RECT WindowRect;
	GetWindowRect(Window, &WindowRect);
	u32 CenterX = WindowRect.left + (Win32GlobalBackBuffer.Width / 2);
	u32 CenterY = WindowRect.top + (Win32GlobalBackBuffer.Height / 2);

	RECT SnapPosRect;
	SnapPosRect.left = (s32)CenterX;
	SnapPosRect.right = (s32)CenterX;
	SnapPosRect.top = (s32)CenterY;
	SnapPosRect.bottom = (s32)CenterY;
	ClipCursor(&SnapPosRect);
}

internal void
StringCat(char *SrcA, size_t SrcACount,
		  char *SrcB, size_t SrcBCount,
		  char *Dest, size_t DestCount)
{
	for(u32 Index = 0; Index < SrcACount; ++Index)
	{
		*Dest++ = *SrcA++;
	}

	for(u32 Index = 0; Index < SrcBCount; ++Index)
	{
		*Dest++ = *SrcB++;
	}

	*Dest = '\0';
}

internal u32
StringLen(char *String)
{
	u32 Result = 0;
	while(*String++)
	{
		Result++;
	}

	return(Result);
}

internal void
Win32ExeFilnameSet(win32_state *State)
{
	DWORD PathSize = GetModuleFileNameA(0, State->ExePath, sizeof(State->ExePath));
	for(char *C = State->ExePath; *C; ++C)
	{
		if(*C == '\\')
		{
			State->OnePastLastSlash = C + 1;
		}
	}
}

internal void
Win32AppDLLPathSet(win32_state *State, char *Filename, char *Dest, s32 DestCount)
{
	StringCat(State->ExePath, State->OnePastLastSlash - State->ExePath,
			Filename, StringLen(Filename),
			Dest, DestCount);
}


DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
	if(Memory)
	{
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
	debug_file_read_result Result = {};
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize))
		{
			u32 FileSize32 = TruncateU64ToU32(FileSize.QuadPart);
			Result.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
			if(Result.Content)
			{
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Content, FileSize32, &BytesRead, 0) && (BytesRead == FileSize32))
				{
					Result.Size = FileSize32;
				}
				else
				{
					DEBUGPlatformFreeFileMemory(Thread, Result.Content);
					Result.Content = 0;
				}
			}
			else
			{
			}
		}
		else
		{
		}
		CloseHandle(FileHandle);
	}
	else
	{
	}

	return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
	b32 Result = false;
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
	if(FileHandle != INVALID_HANDLE_VALUE)
	{
		DWORD BytesWritten;
		if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
		{
			Result = (BytesWritten == MemorySize);
		}
		else
		{
		}

		CloseHandle(FileHandle);
	}
	else
	{
	}

	return(Result);
}

internal FILETIME
Win32LastWriteTime(char *Filename)
{
	FILETIME Result = {};

	WIN32_FILE_ATTRIBUTE_DATA Data;

	if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
	{
		Result = Data.ftLastWriteTime;
	}

	return(Result);
}

internal win32_app_code
Win32AppLoad(char *SrcDLLName, char *TempDLLName, char *LockFilename)
{
	win32_app_code Result = {};

	WIN32_FILE_ATTRIBUTE_DATA Ignored;
	if(!GetFileAttributesEx(LockFilename, GetFileExInfoStandard, &Ignored))
	{
		Result.DLLLastWriteTime = Win32LastWriteTime(SrcDLLName);
		CopyFile(SrcDLLName, TempDLLName, FALSE);
		Result.DLL = LoadLibraryA(TempDLLName);
		if(Result.DLL)
		{
			Result.UpdateAndRender = (app_update_and_render *)GetProcAddress(Result.DLL, "AppUpdateAndRender");
			Result.IsValid = (Result.UpdateAndRender != 0);
		}
	}

	if(!Result.IsValid)
	{
		Result.UpdateAndRender = 0;
	}

	return(Result);
}

internal void
Win32AppUnload(win32_app_code *App)
{
	if(App->DLL)
	{
		FreeLibrary(App->DLL);
		App->DLL = 0;
	}

	App->IsValid = false;
	App->UpdateAndRender = 0;
}

internal void
Win32ProcessKeyboardMessage(app_button_state *Button, b32 IsDown)
{
	if(Button->EndedDown != IsDown)
	{
		Button->EndedDown = IsDown;
		++Button->HalfTransitionCount;
	}
}

internal void
Win32FullScreen(HWND Window)
{
	// NOTE(JE): This follows Raymond Chen's solution, for fullscreen toggling, see:
	// http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPos) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPos);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal win32_client_dimensions
Win32ClientDimensions(HWND Window)
{
	win32_client_dimensions Result = {};

	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Width = ClientRect.right - ClientRect.left;
	Result.Height = ClientRect.bottom - ClientRect.top;

	return(Result);
}

internal void
Win32ResizeDIB(win32_offscreen_buffer *Buffer, s32 Width, s32 Height)
{
	if(Buffer->Memory)
	{
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->Stride = Width * 4;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader); 
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	Buffer->Info.bmiHeader.biHeight = Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	s32 TotalSize = Buffer->Width * Buffer->Height * 4;
	Buffer->Memory = VirtualAlloc(0, (size_t)TotalSize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32ProcessPendingMessages(app_controller_input *KeyboardController)
{
	MSG Message;
	while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
	{
		switch(Message.message)
		{
			case WM_QUIT:
				{
					GlobalRunning = false;
				} break;

			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				u32 VKCode = (u32)Message.wParam;

				b32 WasDown = ((Message.lParam & (1 << 30)) != 0);
				b32 IsDown = ((Message.lParam & (1 << 31)) == 0);
				if(WasDown != IsDown)
				{
					if(VKCode =='W')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->W, IsDown);
					}
					else if(VKCode == 'A')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->A, IsDown);
					}
					else if(VKCode == 'S')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->S, IsDown);
					}
					else if(VKCode == 'D')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->D, IsDown);
					}
					else if(VKCode == 'R')
					{
						Win32ProcessKeyboardMessage(&KeyboardController->D, IsDown);
					}
					else if(VKCode == VK_UP)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Up, IsDown);
					}
					else if(VKCode == VK_DOWN)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Down, IsDown);
					}
					else if(VKCode == VK_LEFT)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Left, IsDown);
					}
					else if(VKCode == VK_RIGHT)
					{
						Win32ProcessKeyboardMessage(&KeyboardController->Right, IsDown);
					}
					else if(VKCode == VK_ESCAPE)
					{
						GlobalRunning = false;
					}


					if(IsDown)
					{
						b32 AltKeyWasDown = (Message.lParam & (1 << 29));
						if((VKCode == VK_F4) && AltKeyWasDown)
						{
							GlobalRunning = false;
						}

						if((VKCode == VK_RETURN) && AltKeyWasDown)
						{
							if(Message.hwnd)
							{
								Win32FullScreen(Message.hwnd);
								win32_client_dimensions Dim = Win32ClientDimensions(Message.hwnd);
								Win32ResizeDIB(&Win32GlobalBackBuffer, Dim.Width, Dim.Height);
							}
						}
					}
				}
			} break;

			case WM_INPUT:
			{
				UINT dwSize = sizeof(RAWINPUT);
				static BYTE lpb[sizeof(RAWINPUT)];

				GetRawInputData((HRAWINPUT)Message.lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

				RAWINPUT* RawInput = (RAWINPUT*)lpb;

				if (RawInput->header.dwType == RIM_TYPEMOUSE) 
				{
					// TODO(Justin): Remove global
					dMouseX += RawInput->data.mouse.lLastX;
					dMouseY -= RawInput->data.mouse.lLastY;
				}

			} break;
			default:
			{
				TranslateMessage(&Message);
				DispatchMessageA(&Message);
			} break;
		}
	}
}

internal void
Win32OpenGLInit(HWND Window)
{
	HDC WindowDC = GetDC(Window);

	PIXELFORMATDESCRIPTOR DesiredPF = {};

	DesiredPF.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	DesiredPF.nVersion = 1;
	DesiredPF.iPixelType = PFD_TYPE_RGBA;
	DesiredPF.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	DesiredPF.cColorBits = 32;
	DesiredPF.cAlphaBits = 8;
	DesiredPF.iLayerType = PFD_MAIN_PLANE;

	int PixelFormatResult = ChoosePixelFormat(WindowDC, &DesiredPF);

	PIXELFORMATDESCRIPTOR SuggestedPF;
	DescribePixelFormat(WindowDC, PixelFormatResult, sizeof(SuggestedPF), &SuggestedPF);

	SetPixelFormat(WindowDC, PixelFormatResult, &SuggestedPF);

	HGLRC OpenGLRC = wglCreateContext(WindowDC);
	if(wglMakeCurrent(WindowDC, OpenGLRC))
	{
		// NOTE(Justin): Success
	}
	else
	{
		INVALID_CODE_PATH;
	}

	ReleaseDC(Window, WindowDC);	
}



internal void
Win32DisplayBuffer(win32_offscreen_buffer *Buffer, HDC DeviceContext, s32 Width, s32 Height)
{

#if 0
	StretchDIBits(DeviceContext,
			0, 0, Buffer->Width, Buffer->Height,
			0, 0, Buffer->Width, Buffer->Height,
			Buffer->Memory,
			&Buffer->Info,
			DIB_RGB_COLORS, SRCCOPY);
#else
	SwapBuffers(DeviceContext);
#endif

}



internal LRESULT CALLBACK
Win32MainWindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
	LRESULT Result = 0;
	switch(Message)
	{
		case WM_SIZE:
		{
		} break;
		case WM_CLOSE:
		{
			GlobalRunning = false;
		} break;
		case WM_DESTROY:
		{
			GlobalRunning = false;
		} break;
		case WM_ACTIVATEAPP:
		{
			Win32CenterCursor(Window);
		} break;
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			Assert(!"Keyboard input came in through a non-dispatch message!");
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT Paint;
			HDC DeviceContext = BeginPaint(Window, &Paint);
			win32_client_dimensions Dimensions = Win32ClientDimensions(Window);
			Win32DisplayBuffer(&Win32GlobalBackBuffer, DeviceContext, Dimensions.Width, Dimensions.Height);
			EndPaint(Window, &Paint);
		} break;
		default:
		{
			Result = DefWindowProcA(Window, Message, WParam, LParam);
		} break;
	}
    
    return(Result);
}

inline LARGE_INTEGER
Win32WallClock(void)
{
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return(Result);
}

inline f32 
Win32SecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
	Assert(GlobalTicksPerSecond > 0.0f);
	f32 Result = ((f32)(End.QuadPart - Start.QuadPart) / (f32)GlobalTicksPerSecond);
	return(Result);
}


int CALLBACK
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCode)
{
	LARGE_INTEGER PerfCountFrequency;
	QueryPerformanceFrequency(&PerfCountFrequency);
	GlobalTicksPerSecond = PerfCountFrequency.QuadPart;

	win32_state Win32State = {};

	Win32ExeFilnameSet(&Win32State);

	char AppDLLPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32AppDLLPathSet(&Win32State, "app.dll", AppDLLPath, sizeof(AppDLLPath));

	char TempAppDLLPath[WIN32_STATE_FILE_NAME_COUNT];
	Win32AppDLLPathSet(&Win32State, "app_temp.dll", TempAppDLLPath, sizeof(TempAppDLLPath));

	char LockFilePath[WIN32_STATE_FILE_NAME_COUNT];
	Win32AppDLLPathSet(&Win32State, "lock.tmp", LockFilePath, sizeof(LockFilePath));

	UINT DesiredSchedulerMS = 1;
	b32 SchedulerGranularitySet = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

	WNDCLASSA WindowClass = {};

	WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
	WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	WindowClass.lpszClassName = "OpenGL Window Class";



	//Win32ResizeDIB(&Win32GlobalBackBuffer, 960, 540);
	//Win32ResizeDIB(&Win32GlobalBackBuffer, 1920, 1080);

	if(RegisterClassA(&WindowClass))
	{
		HWND Window = CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"OpenGL",
				WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if(Window)
		{
			win32_client_dimensions ClientDim = Win32ClientDimensions(Window);
			Win32ResizeDIB(&Win32GlobalBackBuffer, ClientDim.Width, ClientDim.Height);
			Win32OpenGLInit(Window);

			RAWINPUTDEVICE Rid[1];
			Rid[0].usUsagePage = ((USHORT) 0x01); 
			Rid[0].usUsage = ((USHORT) 0x02); 
			Rid[0].dwFlags = RIDEV_INPUTSINK;   
			Rid[0].hwndTarget = Window;
			RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]));

			ShowCursor(false);
			Win32CenterCursor(Window);
				
			s32 MonitorRefreshRate = 60;
			HDC RefreshDC = GetDC(Window);
			s32 Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
			ReleaseDC(Window, RefreshDC);
			if(Win32RefreshRate > 1)
			{
				MonitorRefreshRate = Win32RefreshRate;
			}

			f32 AppRefreshRate = MonitorRefreshRate / 2.0f;
			f32 TargetSecondsPerFrame = 1.0f / (f32)AppRefreshRate;

#if APP_INTERNAL
			LPVOID BaseAddress = (LPVOID)Terabytes(2);
#else

			LPVOID BaseAddress = 0;
#endif
			app_memory AppMemory = {};
			AppMemory.PermanentStorageSize = Megabytes(64);
			AppMemory.TransientStorageSize = Gigabytes(4);

			AppMemory.PlatformAPI.RenderToOpenGL = OpenGLRenderGroupToOutput;

#if APP_INTERNAL
			AppMemory.PlatformAPI.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
			AppMemory.PlatformAPI.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
			AppMemory.PlatformAPI.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;
#endif


			u64 TotalSize = AppMemory.PermanentStorageSize + AppMemory.TransientStorageSize;

			AppMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize,
					MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

			AppMemory.TransientStorage = (u8 *)AppMemory.PermanentStorage + AppMemory.PermanentStorageSize;

			if(AppMemory.PermanentStorage && AppMemory.TransientStorage)
			{
				app_input Input[2] = {};
				app_input *NewInput = &Input[0];
				app_input *OldInput = &Input[1];

				LARGE_INTEGER LastTickCount = Win32WallClock();

				win32_app_code App = Win32AppLoad(AppDLLPath, TempAppDLLPath, LockFilePath);

				u64 LastCycleCount = __rdtsc();
				GlobalRunning = true;
				while(GlobalRunning)
				{
					NewInput->dtForFrame = TargetSecondsPerFrame;

					NewInput->ExecutableReloaded = false;
					FILETIME NewDLLWriteTime = Win32LastWriteTime(AppDLLPath);
					if(CompareFileTime(&NewDLLWriteTime, &App.DLLLastWriteTime) != 0)
					{
						Win32AppUnload(&App);
						App = Win32AppLoad(AppDLLPath, TempAppDLLPath, LockFilePath);
						NewInput->ExecutableReloaded = true;
					}

					app_controller_input *OldKeyboardController = OldInput->Controllers;
					app_controller_input *NewKeyboardController = NewInput->Controllers;
					*NewKeyboardController = {};
					for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
					{
						NewKeyboardController->Buttons[ButtonIndex].EndedDown =
							OldKeyboardController->Buttons[ButtonIndex].EndedDown;

					}

					dMouseX = 0.0f;
					dMouseY = 0.0f;

					Win32ProcessPendingMessages(NewKeyboardController);

					NewInput->dMouseX = dMouseX;
					NewInput->dMouseY = dMouseY;

					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0],
							GetKeyState(VK_LBUTTON) & (1 << 15));

					Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0],
							GetKeyState(VK_RBUTTON) & (1 << 15));

					thread_context Thread = {};

					app_offscreen_buffer Buffer = {};
					Buffer.Width = Win32GlobalBackBuffer.Width;
					Buffer.Height = Win32GlobalBackBuffer.Height;
					Buffer.Stride = Win32GlobalBackBuffer.Stride;
					Buffer.Memory = Win32GlobalBackBuffer.Memory;

					if(App.UpdateAndRender)
					{
						App.UpdateAndRender(&Thread, &AppMemory, NewInput, &Buffer);
					}

					LARGE_INTEGER WorkTickCount = Win32WallClock();
					f32 SecondsElapsedForWork = Win32SecondsElapsed(LastTickCount, WorkTickCount);

					f32 SecondsElapsedForFrame = SecondsElapsedForWork;
					if(SecondsElapsedForFrame < TargetSecondsPerFrame)
					{
						if(SchedulerGranularitySet)
						{
							DWORD SleepTime = (DWORD)(1000.0f * (TargetSecondsPerFrame - SecondsElapsedForFrame));
							if(SleepTime > 0)
							{
								Sleep(SleepTime);
							}
						}
						f32 TestSecondsElapsedForFrame = Win32SecondsElapsed(LastTickCount, Win32WallClock());

						if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
						{
							//TODO(JE): Missed sleep
						}

						while(SecondsElapsedForFrame < TargetSecondsPerFrame)
						{
							SecondsElapsedForFrame = Win32SecondsElapsed(LastTickCount, Win32WallClock());
						}
					}
					else
					{
						// NOTE(JE): Missed Frame.
					}

					LARGE_INTEGER EndTickCount = Win32WallClock();
					LastTickCount = EndTickCount;

					HDC DeviceContext = GetDC(Window);
					win32_client_dimensions Dimensions = Win32ClientDimensions(Window);
					Win32DisplayBuffer(&Win32GlobalBackBuffer, DeviceContext, Dimensions.Width, Dimensions.Height);
					ReleaseDC(Window, DeviceContext);

					app_input *Temp = NewInput;
					NewInput= OldInput;
					OldInput = Temp;

#if 0
					f32 MsPerFrame = (f32)(((1000.0f * (f32)ElapsedTickCount) / (f32)GlobalTicksPerSecond));
					f32 FPS = (f32)GlobalTicksPerSecond / (f32)ElapsedTickCount;
					f32 MegaCyclesPerFram = (f32)(ElapsedCycleCount / (1000.0f * 1000.0f));


					char TextBuffer[256];
					sprintf_s(TextBuffer, "%.02f ms/f %.02f f/s %.02f mc/f\n", MsPerFrame, FPS, MegaCyclesPerFram);
					OutputDebugStringA(TextBuffer);

					u64 EndCycleCount = __rdtsc();
					u64 ElapsedCycleCount = EndCycleCount - LastCycleCount;
					LastCycleCount = EndCycleCount;
#endif
				}
			}
		}
	}

	return(0);
}
