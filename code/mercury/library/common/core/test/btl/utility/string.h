#include "test/btl/test.h"
#include "btl/utility/string.h"
#include <string>
#include <sstream>
#include <cxxtest/TestSuite.h>

using namespace Btl;

class Btl_String_Tests : public CxxTest::TestSuite
{
public:
	void testString()
	{
		String6 s1("test");
		assert(s1.Length() == 4);

		s1 += "1";
		assert(s1 == "test1");

		String s2 = s1.Upper();
		assert(s2 == "TEST1");

		s1.Clear();
		assert(s1 == ""); 
		assert(s1.Empty());
	}

	void testReplace()
	{
		String s;
		s.Reserve(20);
		s.Resize(15);
		s.Replace(0, 15, "replaced string");
		assert(s.Length() == 15 && s == "replaced string");
	}

	void testMisc()
	{
		String s1 = "one two three";

		// note: returns new string but looks like it should replace in the string
		String s2 = s1.Replace("three", "four");
		//s2.Insert(9, "three "); // missing function

		// this one does replace in the string
		s1.Replace(0, 3, "ONE");

		//s1.Erase(0, 3); // missing function
		//s1.Insert(0, "one");

		S32 p = s1.Search("two");
		assert(p == 4);

		p = s1.Search("five");
		assert(p == -1);

		assert(s1[4] == 't');
		s1[4] = 'T';

		// expected (pos, count) like stl substr not (lower, upper)
		assert(s1(4, 5) == 'T'); 
	}

	void testSwap()
	{
		String s1 = "first";
		String s2 = "second";

		// note: should use more efficient swap method
		Swap(s1, s2);

		assert(s1 == "second" && s2 == "first");
	}

	void testDeserialise()
	{
		String s;
		s << -1;
		s << " zero ";
		s << "one";
		s << String(" two ");
		s << 3;
		s << ' ';
		s << 4.f;
		s << " ";
		s << 5.0;

		class Test {};

		//s << Test(); // should not compile, and indeed it doesn't now.

		assert(s == "-1 zero one two 3 4. 5."); 
	}

	void testSerialise()
	{
		// As a future extension you should be be able to stream out if you can stream in.
		// Would look something like this:
		std::stringstream s;
		s << 1;
		s << 2;
		s << 3;
		s << ' ' << 4 << ' ';
		s << "five";

		int i1; s >> i1;
		int i2; s >> i2;
		std::string s1; s >> s1; 

		assert(i1 == 123);
		assert(i2 == 4);
		assert(s1 == "five");
	}

	void testCopy()
	{
		String s;
		s.CopyFrom("source string", 10);
		assert(s == "source str");

		char buffer[10];
		s.CopyTo(buffer, 9);

		s.CopyFrom(buffer, 10);
		assert(s == "source st");
	}
};
