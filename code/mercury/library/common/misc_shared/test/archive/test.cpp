#include "test.h"

#include "misc_shared/archive/archive.h"
#include "misc_shared/archive/btl.h"
#include "btl/utility/string.h"
#include <System/Serialise/I_FileSystem.h>
#include "system/serialise/C_FileBlock.h"

#include <cxxtest/TestSuite.h>

/////////////////////////////////////////////

// The handler instantiates the functions necessary to archive a polymorphic object
// and registers the type with the archive system.
void ActivateHandlers()
{
	SERIALISATION_HANDLER(Base)
	SERIALISATION_HANDLER(Base2)
	SERIALISATION_HANDLER(Derived)
	SERIALISATION_HANDLER(Derived2)
}

////////////////////////////////////////////////////////

template <typename Archive>
void WriteTests(Archive& ar)
{
	ActivateHandlers();

	{
		int i = 10;
		ar << i;
	}

	// basic tests
	{
		// object
		Test test;
		test.m_i = 1;
		ar << test;

		// pointer
		ar << (const Test*) &test;
	}

	// version
	{
		Test2 test2;
		ar << test2;
	}

	// pointer to atomic type
	{
		const int itest = 27;
		ar << &itest;
	}

	// primitive type
	{
		SimpleVector v;
		v.x = 1.01f; v.y = 2.999f; v.z = 3.5f;
		ar & (const SimpleVector&) v;
	}

	// non-polymorphic derived type
	{
		Derived d;
		d.m_i = 2;
		d.m_j = 3;
		ar << d; // not serialised through pointer, so not polymorphic
	}

	// polymorphic type
	{
		Derived* pd = new Derived;
		pd->m_i = 4;
		pd->m_j = 5;
		ar << (Base*) pd;
		delete pd;
	}

	// scoped pointer
	{
		Btl::ScopedPointer<Base> p;
		Btl::Allocate(p, Btl::TypeHolder<Derived>());
		Derived* pd = (Derived*) Pointer(p);
		pd->m_i = 6;
		pd->m_j = 7;
		ar << p;
	}

	// polymorphic type with virtual key function
	{
		Derived2* pd2 = new Derived2;
		pd2->m_i = 8;
		pd2->m_j = 9;
		ar << (Base2*) pd2;
		delete pd2;
	}

	// char* and string
	{
		ar << "Test_String";
		Btl::String s("Test_Btl_String");
		ar << s;
	}

	// vector
	{
		Btl::Vector<U32> v;
		v.PushBack(0);
		v.PushBack(1);
		ar << v;
	}

	// list
	{
		Btl::List<U32> l;
		l.PushBack(0);
		l.PushBack(1);
		ar << l;
	}

	// pair
	{
		Btl::Pair<float, U32> p;
		p.first = 1.0;
		p.second = 2;
		ar << p;
	}

	// set
	{
		Btl::Set<U32> s;
		s.Insert(0);
		s.Insert(1);
		ar << s;
	}

	// multiset
	{
		Btl::MultiSet<U32> s;
		s.Insert(0);
		s.Insert(0);
		ar << s;
	}

	// chained hash set
	{
		Btl::ChainedHashSet<U32> s;
		s.Insert(0);
		s.Insert(1);
		ar << s;
	}

	// chained hash multiset
	{
		Btl::ChainedHashMultiSet<U32> s;
		s.Insert(0);
		s.Insert(0);
		ar << s;
	}

	// open hash set
	{
		Btl::OpenHashSet<U32> s;
		s.Insert(0);
		s.Insert(1);
		ar << s;
	}

	// map
	{
		Btl::Map<U32, U32> s;
		s.Insert(0, 5);
		s.Insert(1, 5);
		ar << s;
	}

	// multimap
	{
		Btl::MultiMap<U32, U32> s;
		s.Insert(0, 6);
		s.Insert(0, 6);
		ar << s;
	}

	// chained hash map
	{
		Btl::ChainedHashMap<U32, U32> s;
		s.Insert(0, 7);
		s.Insert(1, 7);
		ar << s;
	}

	// chained hash multimap
	{
		Btl::ChainedHashMultiMap<U32, U32> s;
		s.Insert(0, 8);
		s.Insert(0, 8);
		ar << s;
	}

	// open hash map
	{
		Btl::OpenHashMap<U32, U32> s;
		s.Insert(0, 9);
		s.Insert(1, 9);
		ar << s;
	}

	// bit vector
	{
		Btl::BitVector t;
		t.PushBack(0);
		t.PushBack(1);
		ar << t;
	}

	Arch::SerialisationIdManager::Destroy();
}

template <typename Archive>
void ReadTests(Archive& ar)
{
	ActivateHandlers();

	{
		int i;
		ar >> i;
		assert(i == 10);
	}

	// basic tests
	{
		// object
		Test2 test; // m_j initialised to -1 when loading previous version
		ar >> test;
		assert(test.m_i == 1);
		assert(test.m_j == -1);

		// pointer
		Test* p;
		ar >> p;
		assert(p->m_i == 1);
		delete p;
	}

	// version
	{
		Test2 test2;
		ar >> test2;
	}

	// pointer to atomic type
	{
		int* itest;
		ar >> itest;
		assert(*itest == 27);
		delete itest;
	}

	// primitive type
	{
		SimpleVector v;
		ar >> v;
		assert(v.x == 1.01f);
		assert(v.y == 2.999f);
		assert(v.z == 3.5f);
	}

	// non-polymorphic derived type
	{
		Derived d;
		ar >> d;
		assert(d.m_i == 2);
		assert(d.m_j == 3);
	}

	// polymorphic type
	{
		Base* pb;
		ar >> pb;
		Derived* pd = (Derived*) pb; // loaded through base but created as derived since polymorphic
		assert(pd->m_i == 4);
		assert(pd->m_j == 5);
		delete pb;
	}

	// scoped pointer
	{
		Btl::ScopedPointer<Base, Arch::FactoryAllocator<Base> > p;
		ar >> p;
		Derived* pd = (Derived*) Pointer(p);
		assert(pd->m_i == 6);
		assert(pd->m_j == 7);
	}

	// polymorphic type with virtual key function
	{
		Base2* pb2;
		ar >> pb2;
		Derived2* pd2 = (Derived2*) pb2;
		assert(pd2->m_i == 8);
		assert(pd2->m_j == 9);
		delete pb2;
	}

	// char* and string
	{
		char* pString;
		ar >> pString;
		delete pString;
		Btl::String string;
		ar >> string;
	}

	// vector
	{
		Btl::Vector<U32> v;
		ar >> v;
		assert(v.Size() == 2);
		assert(v[0] == 0);
		assert(v[1] == 1);
	}

	// list
	{
		Btl::List<U32> l;
		ar >> l;
		assert(l.Size() == 2);
		assert(l.Front() == 0);
	}

	// pair
	{
		Btl::Pair<float, U32> p;
		ar >> p;
		assert(p.first == 1.0);
		assert(p.second == 2);
	}

	// set
	{
		Btl::Set<U32> s;
		ar >> s;
		assert(s.Size() == 2);
		assert(*s.Begin() == 0);
	}

	// multiset
	{
		Btl::MultiSet<U32> s;
		ar >> s;
		assert(s.Size() == 2);
		assert(*s.Begin() == 0);
	}

	// chained hash set
	{
		Btl::ChainedHashSet<U32> s;
		ar >> s;
		assert(s.Size() == 2);
		assert(*s.Begin() == 0);
	}

	// chained hash multiset
	{
		Btl::ChainedHashMultiSet<U32> s;
		ar >> s;
		assert(s.Size() == 2);
		assert(*s.Begin() == 0);
	}

	// open hash set
	{
		Btl::OpenHashSet<U32> s;
		ar >> s;
		assert(s.Size() == 2);
		assert(*s.Begin() == 0);
	}

	// map
	{
		Btl::Map<U32, U32> s;
		ar >> s;
		assert(s.Size() == 2);

	}

	// multimap
	{
		Btl::MultiMap<U32, U32> s;
		ar >> s;
		assert(s.Size() == 2);
	}

	// chained hash map
	{
		Btl::ChainedHashMap<U32, U32> s;
		ar >> s;
		assert(s.Size() == 2);
	}

	// chained hash multimap
	{
		Btl::ChainedHashMultiMap<U32, U32> s;
		ar >> s;
		assert(s.Size() == 2);
	}

	// open hash set
	{
		Btl::OpenHashMap<U32, U32> s;
		ar >> s;
		assert(s.Size() == 2);
	}

	// bit vector
	{
		Btl::BitVector t;
		ar >> t;
		assert(t.Size() == 2);
		assert(t[0] == 0);
		assert(t[1] == 1);
	}
}

class Archive_Tests : public CxxTest::TestSuite
{
public:
	void test_Archive()
	{
		{
			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("test.dat", Sys::E_FileAccess_Write);

			if (!pFile)
			{
				assert(!"Could not open file.");
				return;
			}
			
			{
				Sys::C_FileBlock rootBlock( pFile, "MySys" );
				Sys::C_FileBlock dataBlock( &rootBlock, "Data" );
				Arch::BinaryOutputArchive ar(dataBlock, Ut::DoEndianSwap::swap );
				WriteTests(ar);
			}

			delete pFile;
		}

		{
			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("test.xml", Sys::E_FileAccess_Write);

			if (!pFile)
			{
				assert(!"Could not open file.");
				return;
			}

			{
				Arch::XmlOutputArchive ar( *pFile );
				WriteTests(ar);
			}

			delete pFile;
		}

		{
			Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open("test.dat", Sys::E_FileAccess_Read);

			if (!pFile)
			{
				assert(!"Could not open file.");
				return;
			}

			{
				Sys::C_FileBlock rootBlock( pFile, "MySys" );
				Sys::C_FileBlock dataBlock( &rootBlock, "Data" );
				Arch::BinaryInputArchive ar( dataBlock );
				ReadTests(ar);
			}

			delete pFile;
		}
	}
};

