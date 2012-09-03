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

#include <memory>
#include <v8.h>
#include "internal/ClassField.h"
#include "internal/Assert.h"

namespace v8wrap
{
	template<class CLASS>
	class ClassDescription
	{
	public:
		ClassDescription()
			:	FnTemplate(v8::FunctionTemplate::New(&Constructor)),
				Prototype(FnTemplate->PrototypeTemplate())
		{
			FnTemplate->InstanceTemplate()->SetInternalFieldCount(2);
		}

		template<class TYPE, TYPE CLASS:: * PTR>
		void Field(const char * name)
		{
			Prototype->SetAccessor(v8::String::New(name), &Internal::Field<CLASS,TYPE>::Get<PTR>,
				&Internal::Field<CLASS,TYPE>::Set<PTR>);
		}

	public:
		v8::Handle<v8::FunctionTemplate> FnTemplate;
		v8::Handle<v8::ObjectTemplate> Prototype;

		static v8::Handle<v8::Value> Constructor(const v8::Arguments & args)
		{
			if (!args.IsConstructCall())
				return v8::Undefined();

			CLASS * inst = new CLASS;

			TypeId id = Internal::type_id<CLASS>();
			args.Holder()->SetPointerInInternalField(0, id);
			args.Holder()->SetPointerInInternalField(1, inst);

			auto ret(v8::Persistent<v8::Object>::New(args.Holder()));
			ret.MakeWeak(inst, &Internal::WeakCallback<CLASS>);

			return ret;
		}
	};

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
	};
}
