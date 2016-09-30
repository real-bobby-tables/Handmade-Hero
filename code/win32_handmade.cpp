#include <windows.h>

#define internal static
#define local_persist static 
#define global_variable static 

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void* BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height)
{

  if(BitmapHandle)
    {
      DeleteObject(BitmapHandle);
    }

  else
    {
      BitmapDeviceContext = CreateCompatibleDC(0);
    }

  BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
  BitmapInfo.bmiHeader.biWidth = Width;
  BitmapInfo.bmiHeader.biHeight = Height;
  BitmapInfo.bmiHeader.biPlanes = 1;
  BitmapInfo.bmiHeader.biBitCount = 32;
  BitmapInfo.bmiHeader.biCompression = BI_RGB;
  //maybe dont free first, free after, then free first if that fails
  
  BitmapHandle = CreateDIBSection(BitmapDeviceContext,
                           &BitmapInfo,
                           DIB_RGB_COLORS,
                           &BitmapMemory,
                           0,
                           0);

  
}

internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
  StretchDIBits( DeviceContext,
                    X, Y, Width, Height,
                    X, Y, Width, Height,
                    BitmapMemory,
                    &BitmapInfo,
                    DIB_RGB_COLORS,
                    SRCCOPY);
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
		case WM_SIZE:
		{
      RECT ClientRect;
      GetClientRect(Window, &ClientRect);
      int Height = ClientRect.bottom - ClientRect.top;
      int Width = ClientRect.right - ClientRect.left;
      Win32ResizeDIBSection(Width, Height);
  
		} break;


		case WM_DESTROY:
		{
		  Running = false;
		} break;


		case WM_CLOSE:
		{  
      Running = false;
		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

    case WM_PAINT:
    {
      PAINTSTRUCT Paint;
      HDC DeviceContext = BeginPaint(Window,&Paint);
      int X = Paint.rcPaint.left;
      int Y = Paint.rcPaint.top;
      int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
      int Width = Paint.rcPaint.right - Paint.rcPaint.left;
      Win32UpdateWindow(DeviceContext, X, Y, Width, Height);
      local_persist DWORD Operation = WHITENESS;
      PatBlt(DeviceContext, X, Y, Width, Height, Operation);
      if(Operation == WHITENESS)
        {
          Operation = BLACKNESS;
        }
      else
        {
          Operation = WHITENESS;
        }
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
		LPSTR CommanddLine,
		int ShowCode)
{
	WNDCLASS WindowClass = {};

	//check if hredraw and vredraw still matter in the modern time
	WindowClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = Instance;
    //WindowClass.hIcon = ;
    WindowClass.lpszClassName = "Handmade Hero Window Class";

    if(RegisterClass(&WindowClass))
      {
        HWND WindowHandle = CreateWindowExA( 0,
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

        if (WindowHandle)
          {
            MSG Message;
            while(Running)
              {
                BOOL MessageResult = GetMessage(&Message,0,0,0);
                if (MessageResult > 0)
                  {
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                  }
                else
                  {
                    break;
                  }
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
