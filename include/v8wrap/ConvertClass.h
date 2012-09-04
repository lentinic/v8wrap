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

#pragma once

#include <v8.h>

namespace v8wrap
{
	template<class CLASS>
	struct Convert<CLASS *>
	{
		static CLASS * FromJS(const v8::Local<v8::Value> & js)
		{
			V8WRAP_ASSERT(js->IsObject());

			if (!js->IsObject())
				return NULL;

			v8::Local<v8::Object> obj(v8::Local<v8::Object>::Cast(js));
			if (obj->InternalFieldCount() != 2)
				return NULL;

			TypeId id = obj->GetPointerFromInternalField(0);
			TypeId target = Internal::type_id<CLASS>();
			V8WRAP_ASSERT(id == target);
			if (id != target)
				return NULL;

			return static_cast<CLASS*>(obj->GetPointerFromInternalField(1));
		}
		
		static v8::Handle<v8::Value> ToJS(CLASS * inst)
		{
			TypeId id = Internal::type_id<CLASS>();
			auto ctx = v8::Context::GetCurrent();
			
			V8WRAP_ASSERT(!ctx.IsEmpty());

			auto fnval = ctx->Global()->GetHiddenValue(Internal::type_id<CLASS>());
			if (fnval.IsEmpty() || !fnval->IsFunction())
				return v8::Undefined();

			auto fn = v8::Function::Cast(*fnval);
			v8::Handle<v8::Value> ext = v8::External::New(inst);
			v8::Local<v8::Object> jsref = fn->NewInstance(1, &ext);

			return jsref;
		}
	};
}
