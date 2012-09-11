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
#define ENABLE_VLD
#if defined(ENABLE_VLD)
#include <vld.h>
#endif

#include <iostream>
#include <string>
#include <v8wrap.h>

#include "Console.h"
#include "Window.h"

using namespace std;

void MainLoop();

int main(int argc, char * argv[])
{
	v8::V8::Initialize();
	v8wrap::InstallGC();

	MainLoop();
	
	v8wrap::ForceGC();
	v8::V8::Dispose();
	return 0;
}

void MainLoop()
{
	Window::Initialize();

	auto ctx(v8::Context::New());
	ctx->Enter();	
	v8::HandleScope scope;

	Console::Register(ctx->Global());
	Window::Register(ctx->Global());

	string input;
	cout<<"Example Javascript environment using v8wrap.  type \"exit\" to quit the application."<<endl;
	cout<<">> ";
	getline(cin, input);
	while (input != "exit")
	{
		auto script = v8::Script::New(v8::String::New(input.c_str()));
		if (script.IsEmpty())
		{
			cout<<"Error compiling: \""<<input<<"\""<<endl;
		}
		else
		{
			script->Run();
		}

		cout<<">> ";
		getline(cin, input);
	}

	ctx->Exit();
	ctx.Dispose();

	Window::Shutdown();
}
