#include <windows.h>
#include <stdint.h>


#define internal static
#define local_persist static 
#define global_variable static 

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

struct win32_offscreen_buffer
{
  BITMAPINFO Info;
  void* Memory;
  int Width;
  int Height;
  int Pitch;
};

struct win32_window_dimension
{
  int Width;
  int Height;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;

win32_window_dimension Win32GetWindowDimension(HWND Window)
{
  win32_window_dimension res;

  RECT ClientRect;
  GetClientRect(Window, &ClientRect);
  res.Width = ClientRect.right - ClientRect.left;
  res.Height = ClientRect.bottom - ClientRect.top;

  return (res);
}

internal void RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{
  //lets check what the optimizer does kids

  uint8* Row = (uint8*)Buffer.Memory;

  for(int Y = 0; Y < Buffer.Height; ++Y)
    {
      uint32* Pixel = (uint32*)Row;
      for(int X = 0; X < Buffer.Width; ++X)
        {
          //remeber, this is little endian, so the pixel order is reversed!
          //BB GG RR
          uint8 Blue = (X + XOffset);
          uint8 Green = (Y + YOffset);

          *Pixel++ = ((Green << 8) | Blue);
        }
      Row += Buffer.Pitch;
    }

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
                                  int WindowHeight, win32_offscreen_buffer Buffer)
{
  StretchDIBits(DeviceContext,
                0,0, WindowWidth, WindowHeight,
                0,0, Buffer.Width, Buffer.Height,
                Buffer.Memory, &Buffer.Info,
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

    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window,&Paint);
      //this way, we dont clutter up our paint function
      //simply get the dimensionn and put it in the dimensions struct
      win32_window_dimension Dimension = Win32GetWindowDimension(Window);

      //then, to render to the window, pass the dimensions and the offscreen buffer
      Win32RenderToWindow(DeviceContext, Dimension.Width, Dimension.Height, GlobalBackBuffer);
      EndPaint(Window, &Paint);
    } break;

		default:
		{
      Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}

	return (Result);
}



int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, 
		LPSTR CommandLine,
		int ShowCode)
{
	WNDCLASS WindowClass = {};

  Win32ResizeDIBSection(&GlobalBackBuffer, 1200, 780);

  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
  WindowClass.hInstance = Instance;
    //WindowClass.hIcon = ;
    WindowClass.lpszClassName = "Handmade Hero Window Class";

    if(RegisterClassA(&WindowClass))
      {
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
            HDC DeviceContext = GetDC(Window);

            int XOffset = 0, YOffset = 0;
            GlobalRunning = true;
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
                RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

                win32_window_dimension Dims = Win32GetWindowDimension(Window);
                Win32RenderToWindow(DeviceContext, Dims.Width, Dims.Height, GlobalBackBuffer);


                ++XOffset;
                YOffset += 2;
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
