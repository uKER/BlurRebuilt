
#include "stdafx.h"

#include "../utility/Vector.h"
#include "../utility/HashOps.h"		//Hashing operations for HashSet and HashMap classes.
#include "../utility/OpenHashTable.h"
#include "../utility/SortedBinaryTree.h"

#include <iostream>
#include <string>
#include <vector>
#include "../tools_debug/DebugManager.h"

Tu::DebugManager* g_pDebugManager;

void LinkedListExample1()
{
	std::cout << "*** Linked List Example ***" << std::endl;

	typedef Ut::UtList<U32> MyList;
	MyList list;

	U32 test = 1;
	list.PushBack(test);
	
	for (MyList::Iterator i = list.Begin(); i != list.End(); ++i)
	{
		std::cout << *i << std::endl;
	}

	std::cout << std::endl;
}

class LinkListTest : public Ut::ListElementSingle<LinkListTest>
{
public:
	int i;

	void Destroy() {}
};

void LinkedListExample2()
{
	std::cout << "*** Linked List Example ***" << std::endl;

	typedef Ut::UtList<LinkListTest> MyList;
	MyList list;

	LinkListTest test1; test1.i = 1;
	LinkListTest test2; test2.i = 2;

	list.PushBack(test1);
	list.PushBack(test2);
	
	for (MyList::Iterator i = list.Begin(); i != list.End(); ++i)
	{
		std::cout << i->i << std::endl;
	}

	std::cout << std::endl;
}

void StaticArrayExample()
{
	std::cout << "*** Static Array Example ***" << std::endl;

	Ut::UtVector<Ut::ArrayVector<U32, 4> > array;
	array[0] = 5;
	array[1] = 6;
	array[2] = 7;
	array[3] = 8;
	for (S32 i = 0; i < array.Size(); i++)
	{
		std::cout << i << std::endl;
	}

	std::cout << std::endl;
}

void DynamicArrayExample()
{
	std::cout << "*** Dynamic Array Example ***" << std::endl;

	Ut::Vector<U32> array;
	Ut::Vector<U32> array2;

	array.PushBack(5);
	array.PushBack(6);
	array.PushBack(7);
	array.PushBack(8);

	array2 = array;
	for (S32 i = 0; i < array2.Size(); i++)
	{
		std::cout << array2[i] << std::endl;
	}

	std::cout << std::endl;
}

	class AftkShaderImageNode;
	class AftkMaterialNode;

	struct UVLink {
		const AftkShaderImageNode* pImageNode;
		unsigned int uvIndex;
	};

	typedef Ut::UtVector<Ut::DynamicVector<UVLink> > UVLinkageList;

void MapExample()
{
	std::cout << "*** Map Example ***" << std::endl;

	Ut::UtMap<const AftkMaterialNode*, UVLinkageList> m_uvLinkInfo;

	Ut::UtMap<U32, std::string> test;

	//One method of insertion.
	test.Insert(Ut::UtMap<U32, std::string>::DataType(4, std::string("Is")));
	//Another method of insertion
	test[2] = "World";
	//We'll stick with the easier method of insertion for this example...
	test[6] = "Test";
	test[5] = "A";
	test[1] = "Hello";
	test[3] = "This";
 
	Ut::UtMap<U32, std::string>::ConstIterator it = test.Begin();
	while(it != test.End())
	{
		std::cout << it->GetKey() << " " << it->GetValue() << std::endl;
		it++;
	}

	std::cout << std::endl;

	//We can also have maps key by types such as std::string.
	Ut::UtMap<std::string, U32> test2;
	test2["Pwn"] = 1337;
	std::cout << test2.Begin()->GetKey() << " " << test2.Begin()->GetValue() << std::endl;

	std::cout << std::endl;
}

void SetExample()
{
	std::cout << "*** Set Example ***" << std::endl;

	Ut::UtSet<std::string> test;
	test.Insert(std::string("ThisIsAString"));
	test.Insert(std::string("AnotherString"));
	test.Insert(std::string("ILikeStrings"));
 
	Ut::UtSet<std::string>::Iterator it = test.Begin();
	while(it != test.End())
	{
		//*it = std::string("won't work");

		std::cout << (std::string)(*it) << std::endl;
		it++;
	}

	std::cout << std::endl;
}

void HashSetExample()
{
	std::cout << "*** HashSet Example ***" << std::endl;

	//Some variables to grab the pointers of.
	S32 obj1 = 4;
	S32 obj2 = -2;
	S32 obj3 = 2;

	Ut::OpenHashSet<void*, Ut::HashKeyOpPtr > hashTable;
	
	hashTable.Insert(&obj1);
	hashTable.Insert(&obj2);
	//Attempt to add the same object twice - not possible as it's already a 
	//member of the the set.
	hashTable.Insert(&obj2);
	hashTable.Insert(&obj3);
//	std::cout << "# active elements " << hashTable.Size() << std::endl;

	Ut::OpenHashSet<void*, Ut::HashKeyOpPtr >::ConstIterator it;
	for (it = hashTable.Begin(); it != hashTable.End(); it++)
	{
		void* p = (*it);
		std::cout << *((S32*)(p)) << std::endl;
	}

	it = hashTable.FindByIndex(0);

	std::cout << std::endl;
}

void HashMapExample()
{
	std::cout << "*** HashMap Example ***" << std::endl;	

	//Some variables to grab the pointers of.
	std::string obj1("Hello");
	std::string obj2("World");
	std::string obj3("This is");
	std::string obj4("a Test");

	Ut::OpenHashMap<void*, S32, Ut::HashKeyOpPtr > hashTable;
	Ut::OpenHashMap<void*, S32, Ut::HashKeyOpPtr >::InsertionReturnType result;
	
	result = hashTable.Insert(&obj1);
	result.first->GetValue() = 2;

	result = hashTable.Insert(&obj2);
	result.first->GetValue() = 4;
	//Attempt to add the same object twice - not possible as it's already a 
	//member of the the set. This is true even though the value we're trying to add is 
	//different to that stored under the key &obj2.
	result = hashTable.Insert(&obj2);
	//The 'second' value of the result should be false, indicating no record was added.
	assert(result.second == false);

	result = hashTable.Insert(&obj3);
	result.first->GetValue() = 6;

	hashTable[&obj4] = 8;

	std::cout << "# active elements " << hashTable.Size() << std::endl;

	Ut::OpenHashMap<void*, S32, Ut::HashKeyOpPtr >::ConstIterator it;
	for (it = hashTable.Begin(); it != hashTable.End(); it++)
	{
		std::cout	<< *((std::string*)it->GetKey()) << " " 
					<< it->GetValue() << std::endl;
	}

	Ut::OpenHashMap<void*, S32, Ut::HashKeyOpPtr > hashTable2 = hashTable;
	std::cout << std::endl;
}

int main(int argc, char* argv[])
{
	g_pDebugManager = new Tu::DebugManager();
	#ifdef _DEBUG
		g_pDebugManager->SetDebugEnabledFlag(true);
		g_pDebugManager->SetConsoleEnabledFlag(true);
		g_pDebugManager->SetLogFileEnabledFlag(true);
		g_pDebugManager->Create("./UtilityExamples.log");
	#else 
		g_pDebugManager->SetDebugEnabledFlag(true);
		g_pDebugManager->SetConsoleEnabledFlag(true);
		g_pDebugManager->SetLogFileEnabledFlag(true);
		g_pDebugManager->Create("./UtilityExamples.log");
	#endif

	*g_pDebugManager << "Utility Examples:\n";

	//extern void TestSerialise();
	//TestSerialise();

	LinkedListExample1();
	LinkedListExample2();

	StaticArrayExample();

	DynamicArrayExample();

	MapExample();

	SetExample();

	HashSetExample();

	HashMapExample();

	extern void TreeTest();
	TreeTest();

	g_pDebugManager->Destroy();
	delete g_pDebugManager;
	return 0;
}


