#include <core/core.h>
#include <core/Maths.h>
#include <cxxtest/TestSuite.h>

#include <actor_system/Handle.h>
#include <actor_system/HandleData.h>
#include <actor_system/HandleDeclare.h>
#include <actor_system/HandleItem.h>
#include <actor_system/HandleManager.h>

#include <xt/XtGame.h>


// stub for xt linkage
XT_APPLICATION
	;
XT_END_APPLICATION


class TestHandle : public CxxTest::TestSuite
{
public:
	class C_MockItem : public Util::C_HandleItem<C_MockItem>
	{
	};

	typedef Util::C_Handle<C_MockItem> C_MockHandle;

	typedef Util::C_HandleManager<C_MockItem> C_MockManager;

	void testHandleData()
	{
		const U32 index = 574U;
		const U32 cycle = 134U;
		const U32 fixed = 2U;

		StaticAssert(index < Util::C_HandleData::kIndexSize, INDEX_IS_TOO_LARGE);
		StaticAssert(cycle < Util::C_HandleData::kCycleSize, CYCLE_WILL_BE_TRUNCATED);
		StaticAssert(fixed < Util::C_HandleData::kCycleSize, FIXED_WILL_BE_TRUNCATED);

		// default-initialised handle data object is NULL
		{
			Util::C_HandleData data;
			TS_ASSERT(data.IsNull());
		}

		// index is preserved
		{
			Util::C_HandleData data;
			data.SetIndex(index);
			TS_ASSERT(data.GetIndex() == index);
		}

		// zero cycle isn't preseved, this value is reserved for the NULL handle
		{
			Util::C_HandleData data;
			data.SetCycle(0);
			TS_ASSERT(data.GetCycle() != 0);
		}

		// non-zero cycle is preserved
		{
			Util::C_HandleData data;
			data.SetCycle(cycle);
			TS_ASSERT(data.GetCycle() == cycle);
		}

		// fixed is preserved
		{
			Util::C_HandleData data;
			data.SetFixed(fixed);
			TS_ASSERT(data.GetFixed() == fixed);
		}

		// clear makes NULL
		{
			Util::C_HandleData data;
			data.Init(index, cycle, fixed);
			TS_ASSERT(!data.IsNull());
			data.Clear();
			TS_ASSERT(data.IsNull());
		}

		// copy constructor
		{
			Util::C_HandleData orig;
			orig.Init(index, cycle, fixed);
			Util::C_HandleData copy(orig);
			TS_ASSERT(copy.AsU32() == orig.AsU32());
		}

		// assignment
		{
			Util::C_HandleData orig;
			orig.Init(index, cycle, fixed);
			Util::C_HandleData copy;
			TS_ASSERT(copy.AsU32() != orig.AsU32());
			copy = orig;
			TS_ASSERT(copy.AsU32() == orig.AsU32());
		}

		// cycle is stepped
		{
			Util::C_HandleData orig;
			orig.Init(index, cycle, fixed);
			Util::C_HandleData copy(orig);
			orig.SpinCycle();
			TS_ASSERT(copy.AsU32() != orig.AsU32());
		}
	}

	void testHandleManager()
	{
		const U32 kMaxItems = 2;

		StaticAssert(kMaxItems < Util::C_HandleData::kIndexSize, TOO_MANY_ITEMS);

		C_MockManager manager(kMaxItems);
		C_MockItem items [kMaxItems];

		// manager starts empty
		TS_ASSERT(manager.Count() == 0);

		// allocate max items
		for (U32 i = 0; i < kMaxItems; ++i)
		{
			C_MockItem* pItem = &items[i];
			const C_MockHandle hItem = manager.AllocateHandle(pItem);
			TS_ASSERT(hItem == pItem->GetHandle());
			TS_ASSERT(manager.IsValidHandle(hItem));
			TS_ASSERT(manager.GetItemPtr(hItem) == pItem);
		}
		TS_ASSERT(manager.Count() == kMaxItems);

		// can not add any more
		C_MockItem fail;
		const C_MockHandle hFail = manager.AllocateHandle(&fail);
		TS_ASSERT(!manager.IsValidHandle(hFail));
		TS_ASSERT(manager.GetItemPtr(hFail) == NULL);
		TS_ASSERT(manager.Count() == kMaxItems);

		// remove an item
		C_MockItem* pItem = &items[0];
		manager.FreeHandle(pItem);
		TS_ASSERT(!manager.IsValidHandle(pItem->GetHandle()));
		TS_ASSERT(manager.Count() == (kMaxItems - 1));

		// put it back
		const C_MockHandle hItem = manager.AllocateHandle(pItem);
		TS_ASSERT(hItem == pItem->GetHandle());
		TS_ASSERT(manager.IsValidHandle(hItem));
		TS_ASSERT(manager.GetItemPtr(hItem) == pItem);
		TS_ASSERT(manager.Count() == kMaxItems);
	}

};
