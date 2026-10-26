#include "stdafx.h"

#include "serialise.h"
#include "../tools_debug/DebugManager.h"

#include <vector>
#include <iostream>

extern Tu::DebugManager* g_pDebugManager;

struct Test
{
	U32 i;
};

template <typename Stream>
Stream& operator Serialise(Stream& stream, const Test& item)
{
	stream & item.i;
	return stream;
}

template <typename Stream>
Stream& operator >> (Stream& stream, Test& item)
{
	stream & item.i;
	return stream;
}

SERIALISATION_VERSION(Test, 1)
SERIALISATION_ID(Test, 'TEST')

void TestSerialise()
{
	U32 id = 'test';

	Ut::OutputStream<std::ostream> out(std::cout);

	out << 1;

	Test t;
	t.i = 2;

	out << t;
	out & t;
	out << Ut::Name("test", t);

	in & t;

	Test* p = &t;
	out << p;
}
