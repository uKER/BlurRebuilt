#ifndef SWEEPANDPRUNE_H
#define SWEEPANDPRUNE_H

//#include "Object.h"
#include "gravity/sharedphysics/List.h"
#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/RadixSort.h"


namespace Gravity
{

	class C_Object;
	class C_ObjectWrapper;
	class C_SweepAndPruneObject;


class C_SweepAndPrunePair : public SharedPhysics::C_DoubleListElem<C_SweepAndPrunePair>
{

public:
	C_SweepAndPrunePair(): m_pObjA(NULL), m_pObjB(NULL)
	{
	}

	NOALIAS inline void SetObjects(C_SweepAndPruneObject* pObjA, C_SweepAndPruneObject* pObjB);

	//NOALIAS __forceinline bool operator < (const C_SweepAndPrunePair& other)
	//{
	//	return m_Index < other.m_Index;
	//	//return m_IndexA < other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB < other.m_IndexB);
	//}

	//NOALIAS __forceinline bool operator > (const C_SweepAndPrunePair& other)
	//{
	//	return m_Index > other.m_Index;
	//	//return m_IndexA > other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB > other.m_IndexB);
	//}

	//NOALIAS __forceinline bool operator <= (const C_SweepAndPrunePair& other)
	//{
	//	return m_Index <= other.m_Index;
	//	//return m_IndexA < other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB <= other.m_IndexB);
	//}

	//NOALIAS __forceinline bool operator >= (const C_SweepAndPrunePair& other)
	//{
	//	return m_Index >= other.m_Index;
	//	//return m_IndexA > other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB >= other.m_IndexB);
	//}

	//NOALIAS __forceinline bool operator == (const C_SweepAndPrunePair& other)
	//{
	//	return m_Index == other.m_Index;
	//	//return (m_IndexA == other.m_IndexA && m_IndexB == other.m_IndexB);
	//}

	//NOALIAS __forceinline bool operator != (const C_SweepAndPrunePair& other)
	//{
	//	return m_Index != other.m_Index;
	//	//return (m_IndexA != other.m_IndexA && m_IndexB != other.m_IndexB);
	//}

	//NOALIAS __forceinline U32& GetRadix()
	//{
	//	return m_Index;
	//}

	//NOALIAS __forceinline U32 IndexA()
	//{
	//	return (m_Index >> 16);
	//}

	//NOALIAS __forceinline U32 IndexB()
	//{
	//	return m_Index & 0xffff;
	//}

	//U32 m_Index;
	C_SweepAndPruneObject* m_pObjA;
	C_SweepAndPruneObject* m_pObjB;
};

class C_SweepAndPruneObjectPtr : public SharedPhysics::C_DoubleListElem<C_SweepAndPruneObjectPtr>
{
public:
	C_SweepAndPruneObject* m_pObj;
};

enum C_SweepAndPruneType
{
	SP_MIN,
	SP_MAX
};

class C_SweepAndPruneNode : public SharedPhysics::C_DoubleListElem<C_SweepAndPruneNode>
{
public:
	C_SweepAndPruneNode(): m_pObj(NULL)
	{
	}

	C_SweepAndPruneObject* m_pObj;
	C_SweepAndPruneType m_Type;
	F32 m_Value;
};


//class C_SweepAndPruneNode : public SharedPhysics::C_DoubleListElem<C_SweepAndPruneNode>
//{
//public:
//	C_SweepAndPruneNode(): m_pObj(NULL)
//	{
//	}
//
//	NOALIAS __forceinline bool operator < (const C_SweepAndPruneNode& other)
//	{
//		return m_Value< other.m_Value;
//		//return m_IndexA < other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB < other.m_IndexB);
//	}
//
//	NOALIAS __forceinline bool operator > (const C_SweepAndPruneNode& other)
//	{
//		return m_Value > other.m_Value;
//		//return m_IndexA > other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB > other.m_IndexB);
//	}
//
//	NOALIAS __forceinline bool operator <= (const C_SweepAndPruneNode& other)
//	{
//		return m_Value <= other.m_Value;
//		//return m_IndexA < other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB <= other.m_IndexB);
//	}
//
//	NOALIAS __forceinline bool operator >= (const C_SweepAndPruneNode& other)
//	{
//		return m_Value >= other.m_Value;
//		//return m_IndexA > other.m_IndexA || (m_IndexA == other.m_IndexA && m_IndexB >= other.m_IndexB);
//	}
//
//	NOALIAS __forceinline bool operator == (const C_SweepAndPruneNode& other)
//	{
//		return m_Value == other.m_Value;
//		//return (m_IndexA == other.m_IndexA && m_IndexB == other.m_IndexB);
//	}
//
//	NOALIAS __forceinline bool operator != (const C_SweepAndPruneNode& other)
//	{
//		return m_Value != other.m_Value;
//		//return (m_IndexA != other.m_IndexA && m_IndexB != other.m_IndexB);
//	}
//
//	NOALIAS __forceinline U32& GetRadix()
//	{
//		return *((U32*)&m_Value);
//	}
//
//	C_SweepAndPruneObject* m_pObj;
//	C_SweepAndPruneType m_Type;
//	F32 m_Value;
//};







class C_SweepAndPruneObject : public SharedPhysics::C_DoubleListElem<C_SweepAndPruneObject>
{
public:
	C_SweepAndPruneObject() : m_pObj(NULL)
	{
	}

	NOALIAS inline void UpdateBox();
	
	C_Object* m_pObj;
	Maths::HVec3 m_Min;
	Maths::HVec3 m_Max;
	C_SweepAndPruneNode m_Nodes[2]; // store the min and max of x coordinate
	C_SweepAndPruneObjectPtr* m_pMyself;
};



class C_SweepAndPrune
{
public:
	
	SharedPhysics::C_RadixSorter<SharedPhysics::C_RadixPointerWrapper<C_SweepAndPruneNode> > m_Sorter;

	SharedPhysics::C_DoubleList<C_SweepAndPruneObject> m_ObjectList; //store all the objects
	SharedPhysics::C_AllocationManager<C_SweepAndPruneObject, 100, true> m_ObjectAllocator;

	SharedPhysics::SortedArray<SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair> > m_ActivePairList;
	SharedPhysics::C_AllocationManager<C_SweepAndPrunePair, 300, true> m_ActivePairListAllocator;

	/*SharedPhysics::SortedArray<SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair>> m_ActivePairBuffer;
	SharedPhysics::C_AllocationManager<C_SweepAndPrunePair, 300, true> m_ActivePairBufferAllocator;*/

	
	void AddObject(C_ObjectWrapper* pObj);
	void RemoveObject(C_SweepAndPruneObject* pObj);
	NOALIAS inline void RemoveObjects();


	void UpdateObjects();


	SharedPhysics::C_DoubleList<C_SweepAndPruneObjectPtr> m_ActiveObjectList;
	SharedPhysics::C_AllocationManager<C_SweepAndPruneObjectPtr, 100, true> m_ActiveObjectListAllocator;


	SharedPhysics::C_DoubleList<C_SweepAndPruneNode> m_XList; //sort the object in the x coordinate

	//SharedPhysics::SortedArray<SharedPhysics::C_RadixPointerWrapper<C_SweepAndPruneNode>> list;

private:
	NOALIAS inline void Resort(C_SweepAndPruneNode* pNode,  SharedPhysics::C_DoubleList<C_SweepAndPruneNode> &list);
	NOALIAS inline void Resort(SharedPhysics::C_DoubleList<C_SweepAndPruneNode>& list);
	void ConstructPair(SharedPhysics::C_DoubleList<C_SweepAndPruneNode> &list);
	void ConstructPair();
};
}

#endif

