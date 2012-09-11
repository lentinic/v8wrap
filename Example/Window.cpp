/*
Copyright (c) 2012 Chris Lentini
http://divergentcoder.com

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the 
Software, and to permit persons to whom the Software is furnished to do so, 
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Window.h"
#include <v8wrap.h>

using namespace std;
using namespace v8;

ATOM				WindowClass = 0;
HANDLE				WindowThreadHandle = 0;
DWORD				WindowThreadId = 0;
bool				ExitRequested = false;

enum
{
	MSG_CREATE_WINDOW = WM_USER,
};

///////////////////////////////////////////////////////////////////////////////
Window::Window()
	: hwnd(0), name(""), width(0), height(0)
{
}
///////////////////////////////////////////////////////////////////////////////
bool Window::Open(const string & title, int w, int h)
{
	if (WindowClass == 0)
		return false;

	if (hwnd != 0)
		Close();

	int style = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_THICKFRAME;
	RECT wr = { 0, 0, w, h };
	AdjustWindowRect(&wr, style, FALSE);
	width = wr.right - wr.left;
	height = wr.bottom - wr.top;
	name = title;

	PostThreadMessage(WindowThreadId, MSG_CREATE_WINDOW, (WPARAM)this, 0);
	return true;
}
///////////////////////////////////////////////////////////////////////////////
void Window::OpenFinalize()
{
	HINSTANCE instance = GetModuleHandle(0);

	int style = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_THICKFRAME;
	hwnd = CreateWindow("v8wrap::Example::Window", name.c_str(), style, 0, 0, width, height,
		nullptr, nullptr, instance, nullptr);

	V8WRAP_ASSERT_MSG(hwnd != 0, "Window creation failed!");
	if (hwnd == 0)
		return;

	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR) this);
	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);
}
///////////////////////////////////////////////////////////////////////////////
void Window::Close()
{
	PostMessage(hwnd, WM_CLOSE, 0, 0);
	hwnd = 0;
	name = "";
	width = 0;
	height = 0;
}
//////////////////////////////////'/////////////////////////////////////////////
bool Window::Initialize()
{
	const char * classname = "v8wrap::Example::Window";

	HINSTANCE instance = GetModuleHandle(0);

	WNDCLASSEX wclass;
	wclass.cbSize = sizeof(WNDCLASSEX);
	wclass.style = CS_HREDRAW | CS_VREDRAW;
	wclass.lpfnWndProc = &WindowProc;
	wclass.cbClsExtra = 0;
	wclass.cbWndExtra = 0;
	wclass.hInstance = instance;
	wclass.hIcon = LoadIcon(instance, TEXT("BrowserIcon"));
	wclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wclass.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wclass.lpszMenuName = nullptr;
	wclass.hIconSm = nullptr;
	wclass.lpszClassName = classname;

	UnregisterClass(classname, instance);
	WindowClass = RegisterClassEx(&wclass);
	if (WindowClass == 0)
		return false;

	WindowThreadHandle = CreateThread(NULL, 0, &WindowThreadEntry, nullptr, 0, &WindowThreadId);

	if (WindowThreadHandle == 0)
	{
		printf("Window thread creation failed with error code 0x%08x", GetLastError());
		return false;
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////
void Window::Shutdown()
{
	ExitRequested = true;
	WaitForSingleObject(WindowThreadHandle, INFINITE);
	CloseHandle(WindowThreadHandle);
	WindowThreadHandle = 0;
}
///////////////////////////////////////////////////////////////////////////////
void Window::Register(Handle<Object> & container)
{
	v8wrap::ClassDescription<Window> desc;
	desc.Method<bool(const string &, int, int)>().Set<&Window::Open>("Open");
	desc.Method<void()>().Set<&Window::Close>("Close");
	desc.Field<string, &Window::name>("name");
	desc.Field<int, &Window::width>("width");
	desc.Field<int, &Window::height>("height");

	container->Set(String::New("Window"), desc.FunctionTemplate()->GetFunction());
}
///////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
///////////////////////////////////////////////////////////////////////////////
DWORD WINAPI Window::WindowThreadEntry(void * data)
{
	MSG message;
	while (!ExitRequested)
	{
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
		{
			if (message.message == MSG_CREATE_WINDOW)
			{
				Window * window = (Window *)(message.wParam);
				V8WRAP_ASSERT(window != nullptr);
				window->OpenFinalize();
			}

			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////

