#include "stdafx.h"
#include "vis_core/vis.h"
#include "vis_core/Network/Network.h"
#include "vis_core/Network/Client.h"
#include <crtdbg.h>

#include "cc++/Persist.h"
#include "Vis_Core/Network/memory_stream.hpp"


using namespace ost;

class TestObject : public ost::BaseObject
{
	DECLARE_PERSISTENCE(TestObject);

public:
	int m_iDummy;

	TestObject()
	{
		m_iDummy = 0;        
	};

	bool write( Engine &ar ) const	{ ar << m_iDummy; return __super::write(ar); }
	bool read ( Engine &ar )		{ ar >> m_iDummy; return __super::read(ar); }
};

class ChildObject : public TestObject
{
	DECLARE_PERSISTENCE(ChildObject);

public:
	int m_iDummy2;

	ChildObject()
	{
		m_iDummy2 = 0;        
	};

	bool write( Engine &ar ) const { ar << m_iDummy2; return __super::write(ar); }
	bool read ( Engine &ar )	   { ar >> m_iDummy2; return __super::read(ar); }
};


IMPLEMENT_PERSISTENCE(TestObject, "TestObj");
IMPLEMENT_PERSISTENCE(ChildObject, "Child");

void Test()
{
	ChildObject test, *pObject;
	TestObject test2;
	ost::BaseObject *pObject2;

	test.m_iDummy = 20;
	test.m_iDummy2 = 20;

	int testint = 10;

	pObject = &test;
	pObject2 = NULL;

	// create memory stream
//	std::memorystream ms;
	std::stringstream ms;
	ost::Engine oa(ms, ost::Engine::modeWrite);

	// serialize the object into the stream
	oa << pObject;

	// deserialize
	ost::Engine ia(ms, ost::Engine::modeRead);

	ia >> pObject2;

}
