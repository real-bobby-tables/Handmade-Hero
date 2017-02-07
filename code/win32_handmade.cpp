#include <windows.h>
#include <malloc.h>
#include <stdint.h>
#include <xinput.h>
#include <dsound.h>
#include <math.h>

/*

  Handmade Hero episode 13, 00:00
  
  4coder test
  
  
 */
#define internal static
#define local_persist static 
#define global_variable static 
#define Pi32 3.14159265359f

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef float real32;
typedef double real64;


#include "handmade.cpp"


struct win32_window_dimension
{
    int Width;
    int Height;
};

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;

//NOTE(brandon) this is the support for xinputgetstate
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE* pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    //OutputDebugStringA("Using get stub\n");
    return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state* XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_


//NOTE(brandon) this is the support for xinputsetstate
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    //OutputDebugStringA("Using set stub\n");
    return (ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state* XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

//man, this is way too long 
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, \
LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)

typedef DIRECT_SOUND_CREATE(direct_sound_create);

internal void Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
    
    if(DSoundLibrary)
    {
        //beware, this should be one line, but needed to be broken up
        direct_sound_create* DirectSoundCreate = (direct_sound_create*)
            GetProcAddress(DSoundLibrary, "DirectSoundCreate");
        
        LPDIRECTSOUND DirectSound;
        
        if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {
            WAVEFORMATEX WaveFormat = {};
            WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
            WaveFormat.nChannels = 2;
            WaveFormat.nSamplesPerSec = SamplesPerSecond;
            WaveFormat.wBitsPerSample = 16;
            WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
            WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
            WaveFormat.cbSize = 0;
            
            if(SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                
                LPDIRECTSOUNDBUFFER PrimaryBuffer;
                if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {
                        //NOTE(brandon): finally set the format
                        OutputDebugStringA("Primary buffer format was set! \n");
                    }
                    
                    else
                    {
                        OutputDebugStringA("Failed to set primary buffer!\n");
                    }
                }
                
                else
                {
                    
                }
            }
            else
            {
                
            }
            
            //NOTE(brandon): THIS IS WHERE THE SECONDARY BUFFER IS CREATED
            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;
            HRESULT Error =DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0);
            if(SUCCEEDED(Error))
            {
                OutputDebugStringA("Secondary Buffer created successfully \n");
            }
            
            else
            {
                OutputDebugStringA("Secondary buffer could not be created!\n");
            }
        }
        
        else
        {
            //TODO(brandon): log diagnostic here
        }
        
    }
    
    else
    {
        //TODO(brandon): log diagnostic here
    }
}

internal void Win32LoadXInput()
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    
    if(!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }
    
    if(XInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
        if(!XInputGetState) {XInputGetState = XInputGetStateStub;}
        
        XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
        if(!XInputSetState) {XInputSetState = XInputSetStateStub;}
        
        //TODO(brandon): log diagnostic stuff here
    }
    
    else
    {
        //TODO(brandon): Log diagnostic stuff here
    }
}

internal win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension ret;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    ret.Width = ClientRect.right - ClientRect.left;
    ret.Height = ClientRect.bottom - ClientRect.top;
    
    return (ret);
}


internal void Win32ResizeDIBSection(win32_offscreen_buffer* Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    
    int BytesPerPixel = 4;
    //the documentation on to what the BitmapInfo
    //stuff is can be found on msdn
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    //maybe dont free first, free after, then free first if that fails
    
    int BitmapMemorySize = (Buffer->Width * Buffer->Height) * BytesPerPixel; //gotta keep our stuff 4 byte aligned y
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width * BytesPerPixel;
}

internal void Win32RenderToWindow(HDC DeviceContext, int WindowWidth,
                                  int WindowHeight, win32_offscreen_buffer* Buffer)
{
    StretchDIBits(DeviceContext,
                  0,0, WindowWidth, WindowHeight,
                  0,0, Buffer->Width, Buffer->Height,
                  Buffer->Memory, &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(
HWND   Window,
UINT   Message,
WPARAM WParam,
LPARAM LParam)
{
    LRESULT Result = 0;
    
    switch(Message)
    {
        
        case WM_DESTROY:
        {
            GlobalRunning = false;
        } break;
        
        
        case WM_CLOSE:
        {
            GlobalRunning = false;
        } break;
        
        case WM_ACTIVATEAPP:
        {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYDOWN:
        case WM_KEYUP:
        {
            uint32 VKCode = WParam;
            bool32 WasDown = ((LParam & (1 << 30)));
            bool32 IsDown = ((LParam & (1 << 31)));
            
            
            if(WasDown != IsDown)
            {
                if(VKCode == 'W')
                {
                    
                }
                else if (VKCode == 'A')
                {
                    
                }
                else if (VKCode == 'S')
                {
                    
                }
                else if (VKCode == 'D')
                {
                    
                }
                else if (VKCode == 'Q')
                {
                    
                }
                else if (VKCode == 'E')
                {
                    
                }
                else if (VKCode == VK_UP)
                {
                    
                }
                else if (VKCode == VK_DOWN)
                {
                    
                }
                else if (VKCode == VK_LEFT)
                {
                    
                }
                else if (VKCode == VK_RIGHT)
                {
                    
                }
                else if (VKCode == VK_ESCAPE)
                {
                    
                }
                else if (VKCode == VK_SPACE)
                {
                    
                }
            }
            
            bool32 AltKeyWasDown = LParam & (1 << 29);
            
            if((VKCode == VK_F4) && AltKeyWasDown)
            {
                GlobalRunning = false;
            }
            
        } break;
        
        
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window,&Paint);
            //this way, we dont clutter up our paint function
            //simply get the dimensionn and put it in the dimensions struct
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            
            //then, to render to the window, pass the dimensions and the offscreen buffer
            Win32RenderToWindow(DeviceContext, Dimension.Width, Dimension.Height, &GlobalBackBuffer);
            EndPaint(Window, &Paint);
        } break;
        
        default:
        {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    
    return (Result);
}

struct win32_sound_output
{
    int SamplesPerSecond;
    int ToneHz;
    int WavePeriod;
    int HalfWavePeriod;
    int BytesPerSample;
    uint32 RunningSampleIndex;
    int SecondaryBufferSize;
    int16 ToneVolume;
    int LatencySampleCount;
};

internal void Win32ClearBuffer(win32_sound_output* SoundOutput)
{
    VOID* Region1;
    DWORD Region1Size;
    VOID* Region2;
    DWORD Region2Size;
    
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize, &Region1, &Region1Size,
                                             &Region2, &Region2Size, 
                                             0)))
    {
        uint8* DestSample = (uint8*)Region1;
        for(DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        
        DestSample = (uint8*)Region2;
        for(DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
        {
            *DestSample++ = 0;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void Win32FillSoundBuffer(win32_sound_output* SoundOutput, DWORD ByteToLock, DWORD BytesToWrite, game_sound_output_buffer* SourceBuffer)
{
    VOID* Region1;
    DWORD Region1Size;
    VOID* Region2;
    DWORD Region2Size;
    
    if(SUCCEEDED(GlobalSecondaryBuffer->Lock(ByteToLock, BytesToWrite, 
                                             &Region1, &Region1Size,
                                             &Region2, &Region2Size, 
                                             0)))
    {
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16* DestSample = (int16*)Region1;
        int16* SourceSample = SourceBuffer->Samples;
        for(DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        
        
        DWORD Region2SampleCount = Region2Size/SoundOutput->BytesPerSample;
        DestSample = (int16*)Region2;
        for(DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;
            ++SoundOutput->RunningSampleIndex;
        }
        
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}



int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, 
                     LPSTR CommandLine,
                     int ShowCode)
{
    
    OutputDebugStringA("Beginning win main\n");
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    int64 PerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    Win32LoadXInput();
    
    WNDCLASSA WindowClass = {};
    
    Win32ResizeDIBSection(&GlobalBackBuffer, 1200, 780);
    
    
    WindowClass.style = CS_HREDRAW | CS_VREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon = ;
    WindowClass.lpszClassName = "Handmade Hero Window Class";
    
    if(RegisterClassA(&WindowClass))
    {
        OutputDebugStringA("Creating window...\n");
        
        HWND Window = CreateWindowExA( 0,
                                      WindowClass.lpszClassName,
                                      "Handmade Hero",
                                      WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      CW_USEDEFAULT,
                                      0,
                                      0,
                                      Instance,
                                      0);
        
        if (Window)
        {
            OutputDebugStringA("Window successfully created\n");
            HDC DeviceContext = GetDC(Window);
            
            
            int XOffset = 0, YOffset = 0;
            win32_sound_output SoundOutput = {};
            SoundOutput.SamplesPerSecond = 48000;
            SoundOutput.ToneHz = 256;
            SoundOutput.ToneVolume = 3000;
            SoundOutput.RunningSampleIndex = 0;
            SoundOutput.WavePeriod = SoundOutput.SamplesPerSecond / SoundOutput.ToneHz;
            SoundOutput.BytesPerSample = sizeof(int16) * 2;
            SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
            SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond / 15;
            
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);
            GlobalRunning = true;
            
            LARGE_INTEGER LastCounter;
            QueryPerformanceCounter(&LastCounter);
            
            int64 LastCycleCount = __rdtsc();
            
            while(GlobalRunning)
            {
                MSG Message;
                
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                
                
                //poll this more frequently
                for(DWORD ControllerIndex = 0; ControllerIndex < XUSER_MAX_COUNT; ++ControllerIndex)
                {
                    XINPUT_STATE ControllerState;
                    
                    if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                    {
                        XINPUT_GAMEPAD* Pad = &ControllerState.Gamepad;
                        
                        bool ButtonA = (Pad->wButtons & XINPUT_GAMEPAD_A);
                        bool ButtonB = (Pad->wButtons & XINPUT_GAMEPAD_B);
                        bool ButtonX = (Pad->wButtons & XINPUT_GAMEPAD_X);
                        bool ButtonY = (Pad->wButtons & XINPUT_GAMEPAD_Y);
                        
                        if(ButtonA)
                        {
                            YOffset += 6;
                        }
                        
                        if(ButtonB)
                        {
                            XOffset += 6;
                        }
                        
                        if(ButtonX)
                        {
                            XOffset -= 6;
                        }
                        
                        if(ButtonY)
                        {
                            YOffset -= 6;
                        }
                        
                    }
                    else
                    {
                        //NOTE(brandon) the controller is not plugged in
                        //OutputDebugStringA("The controller is not plugged in!\n");
                    }
                    
                }
                
                DWORD ByteToLock;
                DWORD BytesToWrite;
                DWORD TargetCursor;
                DWORD PlayCursor;
                DWORD WriteCursor;
                bool32 SoundIsValid = false;
                if((SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor,&WriteCursor))))
                {
                    
                    ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
                    TargetCursor = ((PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize);
                    
                    if(ByteToLock > TargetCursor)
                    {
                        BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
                        BytesToWrite += TargetCursor;
                    }
                    
                    else
                    {
                        BytesToWrite = TargetCursor - ByteToLock;
                    }
                    
                    SoundIsValid = true;
                }
                
                int16* Samples = (int16*)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                game_sound_output_buffer SoundBuffer = {};
                SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
                SoundBuffer.Samples = Samples;
                
                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackBuffer.Memory;
                Buffer.Width = GlobalBackBuffer.Width;
                Buffer.Height = GlobalBackBuffer.Height;
                Buffer.Pitch  = GlobalBackBuffer.Pitch;
                GameUpdateAndRender(&Buffer, XOffset, YOffset, &SoundBuffer, SoundOutput.ToneHz);
                
                if(SoundIsValid)
                {
                    //direct sound output test
                    Win32FillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite, &SoundBuffer);
                }
                
                win32_window_dimension Dims = Win32GetWindowDimension(Window);
                Win32RenderToWindow(DeviceContext, Dims.Width, Dims.Height, &GlobalBackBuffer);
                
                int64 EndCycleCount = __rdtsc();
                
                LARGE_INTEGER EndCounter;
                QueryPerformanceCounter(&EndCounter);
                
                int64 CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
                
                int64 CyclesElapsed = EndCycleCount - LastCycleCount;
                int32 MegaCycles =(int32)(CyclesElapsed /( 1000 * 1000));
                int32 MSPerFrame = (int32)((1000 * CounterElapsed) / PerfCountFrequency);
                int32 FPS = (int32)(PerfCountFrequency/CounterElapsed);
                char InfoBuffer[256];
                wsprintf(InfoBuffer, "Milliseconds/frame: %dms, FPS: %d, MegaCycles: %d\n", MSPerFrame, FPS, MegaCycles);
                OutputDebugStringA(InfoBuffer);
                
                LastCounter = EndCounter;
                LastCycleCount = EndCycleCount;
                
                ++XOffset;
                ++YOffset;
                
            }
        }
        
        else
        {
            //log stuff
        }
        
    }
    
    
    else
    {
        //log crap
    }
    return (0);
}
