#include "stdafx.h"

#include "../utility/SortedBinaryTree.h"

#include "../tools_debug/DebugManager.h"

extern Tu::DebugManager* g_pDebugManager;

// global variable to stop the compiler optimising it out
volatile int g_test;

// convenient function for printing containers
template <typename Iterator>
void DebugPrintSequence(Iterator begin, Iterator end)
{
	for (Iterator it = begin; it != end; it++)
	{
		if (it != begin) *g_pDebugManager << ", ";
		*g_pDebugManager << *it;
	}

	*g_pDebugManager << "\n";
}

/////////////////////////////////////////////////////

void InsertEraseUt()
{
	Ut::UtSet<U32> set;

	// insert and erase some random values
	for (int i = 0; i < 100; i++)
	{
		set.Insert(rand() % 50);
		set.Erase(rand() % 50);
	}

	// check the set to make sure it is in a valid state
	set.PrintDebug();

	// print the sequence of numbers
	DebugPrintSequence(set.Begin(), set.End());
}

// same for stl
void InsertEraseStl()
{
	std::set<U32> set;

	for (int i = 0; i < 100; i++)
	{
		set.insert(rand() % 50);
		set.erase(rand() % 50);
	}

	DebugPrintSequence(set.begin(), set.end());
}

void FindUt()
{
	typedef Ut::UtSet<U32> Set;
	Set set;
	Set::Iterator it;

	// insert values
	for (int i = 0; i < 50; i++)
		set.Insert(i);

	*g_pDebugManager << set.Size() << "\n";

	// find an item
	it = set.Find(20);
	assert(it != set.End());
	g_test = *it;
	//*it = 200; // not allowed

	// search for random values
	for (int i = 0; i < 100; i++)
	{
		it = set.Find(rand() % 100);
		if (it != set.End())
			g_test = *it;
	}
}

void FindStl()
{
	typedef std::set<U32> Set;
	Set set;
	Set::iterator it;

	for (int i = 0; i < 50; i++)
		set.insert(i);

	*g_pDebugManager << (int) set.size() << "\n";

	it = set.find(20);
	assert(it != set.end());
	g_test = *it;
	//*it = 200; // not allowed

	for (int i = 0; i < 100; i++)
	{
		it = set.find(rand() % 100);
		if (it != set.end())
			g_test = *it;
	}
}

// create a set and output it forwards and backwards
void IterateUt()
{
	Ut::UtSet<U32> set;

	for (int i = 0; i < 50; i++)
		set.Insert(i);

	DebugPrintSequence(set.Begin(), set.End());
	DebugPrintSequence(set.ReverseBegin(), set.ReverseEnd());
}

void IterateStl()
{
	std::set<U32> set;

	for (int i = 0; i < 50; i++)
		set.insert(i);

	DebugPrintSequence(set.begin(), set.end());
	DebugPrintSequence(set.rbegin(), set.rend());
}

void MapUt()
{
	typedef Ut::UtMap<U32, std::string> Map;
	Map map;
	Map::Iterator it;

	// insert some items
	map.Insert(Map::DataType(1, "one"));
	map.Insert(Map::DataType(2, "two"));

	// copy the whole map
	Map copy(map);

	// insert another item
	map[3] = "three";

	it = map.Find(3); // look in the original
	assert(it != map.End());
	it = copy.Find(3); // look in the copy
	assert(it == copy.End());

	// output both maps
	DebugPrintSequence(map.Begin(), map.End());
	DebugPrintSequence(copy.Begin(), copy.End());
}

void InsertSequential()
{
	typedef Ut::UtSet<U32> Set;
	Set set;
	Set::Iterator it = set.Begin();

	// insert items in sequence
	for (int i = 0; i < 50; i++)
	{
		it = set.Insert(it, i).first; // this is faster than random insertion
	}

	// check the set for errors
	set.PrintDebug();

	// output the set
	*g_pDebugManager << set.Size() << "\n";
	DebugPrintSequence(set.Begin(), set.End());
}

void EraseIterator()
{
	typedef Ut::UtSet<U32> Set;
	Set set;

	// add some items
	set.Insert(0);
	set.Insert(1); 
	Set::Iterator i2 = set.Insert(2).first;

	// changing the set doesn't invalidate the iterator
	set.Erase(0);
	set.Insert(3); 

	// erase using the iterator
	DebugPrintSequence(set.Begin(), set.End());
	set.Erase(i2);
	DebugPrintSequence(set.Begin(), set.End());
}

void MultiSetUt()
{
	Ut::MultiSet<U32> set;

	// add some items
	for (int i = 0; i < 50; i++)
		set.Insert(rand() % 10);

	DebugPrintSequence(set.Begin(), set.End());

	int count = set.Erase(5); // erase all the fives
	*g_pDebugManager << "Erased " << count << " items.\n";

	count = set.Erase(11); // no elevens
	assert(count == 0);

	DebugPrintSequence(set.Begin(), set.End());

	count = set.Erase(0); // erase from the beginning

	DebugPrintSequence(set.Begin(), set.End());

	count = set.Erase(9); // erase from the end

	DebugPrintSequence(set.Begin(), set.End());
}

void MultiMapUt()
{
	typedef Ut::MultiMap<U32, std::string> Map;
	Map map;
	Map::Iterator it;

	// add items, some with identical keys
	map.Insert(Map::DataType(1, "one"));
	map.Insert(Map::DataType(1, "copy"));
	map.Insert(Map::DataType(2, "two"));
	map.Insert(Map::DataType(2, "two"));

	for (it = map.Begin(); it != map.End(); it++)
		*g_pDebugManager << it->GetKey() << ", " << it->GetValue() << "\n";

	// find the first item with this key
	it = map.Find(1);

	// erase all items with this key
	int count = map.Erase(1);

	for (it = map.Begin(); it != map.End(); it++)
		*g_pDebugManager << it->GetKey() << ", " << it->GetValue() << "\n";
}

// swap some sets
void TestSwap()
{
	typedef Ut::UtSet<U32> Set;

	Set set1;

	for (int i = 0; i < 10; i++)
		set1.Insert(i);

	Set set2;

	set2.Insert(20);

	DebugPrintSequence(set1.Begin(), set1.End());
	DebugPrintSequence(set2.Begin(), set2.End());

	Swap(set1, set2);

	Set::Iterator i = set1.Find(20);

	DebugPrintSequence(set1.Begin(), set1.End());
	DebugPrintSequence(set2.Begin(), set2.End());

	int n = set2.Erase(5);

	// this clears the set and releases all the memory
	// (Clear() by itself may not release memory)
	{
		Set empty;
		Swap(set1, empty);
	}

	// also destroys the set
	set2.Destroy();

	DebugPrintSequence(set1.Begin(), set1.End());
	DebugPrintSequence(set2.Begin(), set2.End());
}

//////////////////////////////////////////////////////////

void TreeTest()
{
	*g_pDebugManager << "*** More set and map examples ***" << "\n";	

	InsertEraseUt();
	//InsertEraseStl();
	FindUt();
	//FindStl();
	IterateUt();
	MapUt();
	InsertSequential();
	EraseIterator();
	MultiSetUt();
	MultiMapUt();
	TestSwap();
}

