#include "gravity/broadphase/SweepAndPrune.h"

namespace Gravity
{

NOALIAS inline void C_SweepAndPrunePair::SetObjects(C_SweepAndPruneObject* pObjA, C_SweepAndPruneObject* pObjB)
{
	m_pObjA = pObjA;
	m_pObjB = pObjB;
	
	//U32 indexA = Maths::Min(pObjA->m_pObj->m_Index, pObjB->m_pObj->m_Index);
	//U32 indexB = Maths::Max(pObjA->m_pObj->m_Index, pObjB->m_pObj->m_Index);
	//m_Index = ((indexA << 16) | (indexB & 0xffff));
}

NOALIAS inline void C_SweepAndPruneObject::UpdateBox()
{
	Collision::C_CRAxisBox& box=m_pObj->m_TransformedBox;
	const Maths::HVec3 box_Min(box.centre - box.extents);
	const Maths::HVec3 box_Max(box.centre + box.extents);

	m_Min=box_Min;
	m_Max=box_Max;

	Maths::Store<Maths::HF32, F32>(box_Min.X(), &m_Nodes[0].m_Value);
	Maths::Store<Maths::HF32, F32>(box_Max.X(), &m_Nodes[1].m_Value);
}

NOALIAS inline void C_SweepAndPrune::Resort(C_SweepAndPruneNode* pNode, SharedPhysics::C_DoubleList<C_SweepAndPruneNode>& list)
{
	bool swap=false;

	do
	{
		//Get the next pointer, if the next pointer is null pointer, set itself as the next pointer
		const C_SweepAndPruneNode* pNext=SharedPhysics::Fast_Select<C_SweepAndPruneNode> (pNode->m_pNext == NULL, pNode, pNode->m_pNext);
		//Get the previous pointer, if the previous pointer point to head(dummy), set the previous pointer as itself
		const C_SweepAndPruneNode* pPrev=SharedPhysics::Fast_Select<C_SweepAndPruneNode> (pNode->m_pPrev == list.GetHead(), pNode, pNode->m_pPrev);
		//Test condition
		bool conNext=pNode->m_Value > pNext->m_Value;
		bool conPrev=pNode->m_Value < pPrev->m_Value;

	
		
		//if conNext and conPrev are both false, which means the pNode is in the right place, no need to swap at all,
		//However, we want to use the banchless aproach, therefore, we still need to set the first and second pointer point
		//to itself.
		C_SweepAndPruneNode* pFirst = SharedPhysics::Fast_Select<C_SweepAndPruneNode>(conNext, pNode, SharedPhysics::Fast_Select<C_SweepAndPruneNode>(conPrev, pPrev, pNode));
		C_SweepAndPruneNode* pSecond= SharedPhysics::Fast_Select<C_SweepAndPruneNode>(conNext, pNext, pNode);
		
		list.Swap(pFirst, pSecond);
		swap=(conNext | conPrev);
	}
	while(swap);
}

NOALIAS inline void C_SweepAndPrune::Resort(SharedPhysics::C_DoubleList<C_SweepAndPruneNode>& list)
{
	bool swap=false;


	do
	{
		swap = false;

		
		C_SweepAndPruneNode* pNode=list.GetFirstElem();

		while(pNode->m_pNext)
		{
			//Get the next pointer, if the next pointer is null pointer, set itself as the next pointer
			C_SweepAndPruneNode* pNext=SharedPhysics::Fast_Select<C_SweepAndPruneNode> (pNode->m_pNext == NULL, pNode, pNode->m_pNext);
			
			if(pNode->m_Value > pNext->m_Value)
			{
				list.Swap(pNode, pNext);
				swap=true;
			}
			else
			{
				pNode=pNext;
			}
		}
	}while(swap);

}


NOALIAS inline void C_SweepAndPrune::RemoveObjects()
{
	m_ObjectList.Reset();
	m_ObjectAllocator.Reset();
}




};
