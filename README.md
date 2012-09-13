#v8wrap

v8wrap is a small set of header files meant to make the process of working with V8 in C++ a little easier.  It was written as an exercise in working with V8 - I had been using a few of the other wrapper libraries available (which are great) but found myself wanting to peek behind the scenes and work a little more closely with the V8 API. Now I am throwing this out here in case anyone else might find it useful.  

###Usage
-----
At the core of the library is the Convert<TYPE> template and its static methods ToJS and FromJS.  As you might expect from the names, ToJS will take an argument of type TYPE and return an equivalent V8 value handle while FromJS will take a V8 value handle and return an instance of type TYPE.  To make this a little more clear:

	v8::Handle<v8::Value> js_int = v8wrap::Convert<int>::ToJS(5);
	int cpp_int = v8wrap::Convert<int>::FromJS(js_int);

Aside from converting back and forth between some basic C++ types (int, float, bool, std::string), the library also lets you convert back and forth between JS/C++ functions as well as expose C++ classes to JS code.  For example:

```cpp
	//...
	void foo() { /* Do Something */ }
	...
	v8wrap::AddFunction(v8::Context::GetCurrent()->Global(), "foo", &foo);
	// AddFunction is a convenience function, calling it here is equivalent to doing this:
	//		v8::Context::GetCurrent()->Global()->Set(v8:String::New("foo"), v8wrap::Convert<void(*)()>::ToJS(&foo));
	//...
```

The global for the current v8 context will now have a function called "foo" that can be called from script.  It is also possible to grab a function from JS land and convert it to a std::function - so that e.g. we could have some C++ code call a callback provided to us from JS without it needing to know anything about V8:

```cpp
	//...
	std::function<void()> callback = v8wrap::Convert<std::function<void()> >::FromJS(SomeJSFuncHandle);
	//...
```

To expose a class:

```cpp
	//...
	class Point
	{
	public:
		float x;
		float y;

		float length() { return sqrt(x*x + y*y); }
	};
	//...
	v8wrap::ClassDescription<Point> desc;
	desc.Field<float, &Point::x>("x");
	desc.Field<float, &Point::y>("y");
	desc.Method<float()>().Set<&Point::length>("length");

	v8::Context::GetCurrent()->Global->Set(v8::String::New("Point"), desc.FunctionTemplate()->GetFunction());
```

Within the current context it is now possible to execute the following script:

```cpp	
	//...
	var pt = new Point();
	pt.x = 3;
	pt.y = 4;
	var len = pt.length();
	//...
```

It is important to note that internally, v8wrap keeps an std::shared_ptr to class instances.  So if you create a new Point C++ side and then pass it off to JS you are giving up responsibility for the management of the lifetime of that object.  Similarly, if you receive a pointer to a Point from JS then you shouldn't hold on to that pointer or try to delete it - if you do want to hold on to a Point instance then there are Convert specializations for std::shared_ptr objects.  So:

```cpp
	void CalledFromJS(Point * pt) { SomeSystem::HoldOnToPoint(pt); } // bad stuff could happen!
```

Versus:

```cpp
	void CalledFromJS(std::shared_ptr<Point> & pt) { SomeSystem::HoldOnToSharedPoint(pt); } // everything is cool
```

That about does it for a brief overview of how to use the library.  If you want to actually see the code in action, I have supplied a small example project that registers a few functions for reading from and writing to the console as well as wrapping a Window class for creating windows.
