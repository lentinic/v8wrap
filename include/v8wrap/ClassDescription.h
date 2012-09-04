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
#include <map>
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

		void Install(v8::Handle<v8::Object> module, const char * name)
		{
			auto ctx = v8::Context::GetCurrent();
			V8WRAP_ASSERT(!ctx.IsEmpty());

			auto fn = FnTemplate->GetFunction();
			ctx->Global()->SetHiddenValue(Internal::type_id<CLASS>(), fn);
			module->Set(v8::String::New(name), fn);
		}

	private:
		v8::Handle<v8::FunctionTemplate> FnTemplate;
		v8::Handle<v8::ObjectTemplate> Prototype;

		static v8::Handle<v8::Value> Constructor(const v8::Arguments & args)
		{
			static std::map<CLASS *, v8::Persistent<v8::Object> > instance_map;

			if (!args.IsConstructCall())
				return v8::Undefined();

			CLASS * inst;
			if (args[0]->IsExternal())
				inst = Internal::external_cast<CLASS>(args[0]);
			else
				inst = new CLASS;

			if (instance_map.count(inst) > 0)
				return instance_map[inst];

			TypeId id = Internal::type_id<CLASS>();
			
			//std::shared_ptr<CLASS> * ptr = new std::shared_ptr<CLASS>(inst);

			args.Holder()->SetPointerInInternalField(0, id);
			args.Holder()->SetPointerInInternalField(1, inst);

			auto ret(v8::Persistent<v8::Object>::New(args.Holder()));
			ret.MakeWeak(inst, &Internal::WeakCallback<CLASS>);

			instance_map[inst] = ret;

			return ret;
		}

		static v8::Handle<v8::Value> Destructor(v8::Persistent<v8::Value> val, void * param)
		{
			delete static_cast<TYPE*>(param);
			val.Dispose();
			val.Clear();
		}
	};
}
