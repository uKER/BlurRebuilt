#include "stdafx.h"
#include "gravity/broadphase/SweepAndPrune.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sharedphysics/AllocationManager.inl"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/broadphase/Object.h"
#include "gravity/broadphase/SweepAndPrune.inl"
//#include "System/Serialise/TextSerialise.h"
#include "gravity/sharedphysics/RadixSort.h"
#include "gravity/sharedphysics/RadixSort.inl"

namespace Gravity
{
	//void DrawList(SharedPhysics::C_DoubleList<C_SweepAndPruneNode>& list, char* name)
	//{
	//	C_SweepAndPruneNode* pNode=list.GetHead();

	//	Sys::I_BinaryStream* pStream = GetDefaultFileSystem()->Open(name, Sys::E_FileAccess_Write);
	//	Sys::TextSerialiser archive(*pStream);

	//	F32 value;
	//	while(pNode->m_pNext)
	//	{
	//		pNode=pNode->m_pNext;
	//		Maths::Store<Maths::HF32>(pNode->m_Value, &value);
	//		if(pNode->m_Type == SP_MIN)
	//			archive << "Object Index: " << pNode->m_pObj->m_pObj->m_Index << ", Value: "<< value<<", SP: MIN\n";
	//		else
	//			archive << "Object Index: " << pNode->m_pObj->m_pObj->m_Index << ", Value: "<< value<<", SP: MAX\n";

	//		
	//	}
	//	delete pStream;
	//}
//
//	void DrawList(SharedPhysics::SortedArray<SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair>>& list, char* name)
//	{
//
//		Sys::I_BinaryStream* pStream = GetDefaultFileSystem()->Open(name, Sys::E_FileAccess_Write);
//		Sys::TextSerialiser archive(*pStream);
//		//archive << "NumOfElements "<<list.m_numElements <<"\n";
//		for(S32 i=0; i<list.m_numElements; ++i)
//		{
//			SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair> wrapper=list.m_pElems[i];
//
//			C_SweepAndPrunePair* pNode=wrapper.m_pElement;
//			
//			archive << "Pair( " << pNode->IndexA()<< " " << pNode->IndexB() <<" )\n";
//		}
////		archive<< "------\n";
//		delete pStream;
//	}



	/*void C_SweepAndPrune::AddObject(C_ObjectWrapper* pObjWrapper)
	{
	
		C_Object* pObj=pObjWrapper->m_pObject;
		C_SweepAndPruneObject* obj=&pObjWrapper->m_SAPObject;
		obj->m_pObj =pObj;
		
		//This is used to link the SP_MIN and SP_MAX node int the sweep list to speed up the remove process
		C_SweepAndPruneObjectPtr* ptr=this->m_ActiveObjectListAllocator.GetNextFreeItem();
		ptr->m_pObj = obj;
		obj->m_pMyself = ptr;

		this->m_ObjectList.InsertToEnd(obj);

		
		//Compare pObj's box with the sorted list in the x coordinate
		//obj->m_Nodes[0]=m_XListAllocator.GetNextFreeItem();
		obj->m_Nodes[0].m_pObj=obj;
		obj->m_Nodes[0].m_Type=SP_MIN;
	
		//obj->m_Nodes[1]=m_XListAllocator.GetNextFreeItem();
		obj->m_Nodes[1].m_pObj=obj;
		obj->m_Nodes[1].m_Type=SP_MAX;
		
		obj->UpdateBox();

		bool notFind=true;
		C_SweepAndPruneNode* pNode=m_XList.GetHead();

		while((pNode->m_pNext!=NULL) && notFind)
		{
			notFind=obj->m_Nodes[0].m_Value > pNode->m_pNext->m_Value;
			pNode=SharedPhysics::Fast_Select<C_SweepAndPruneNode>(notFind, pNode->m_pNext, pNode);
			
		}

		m_XList.InsertAfter(pNode, &obj->m_Nodes[0]);

		pNode=pNode->m_pNext;
		notFind=true;

		while((pNode->m_pNext!=NULL) && notFind)
		{
			notFind = obj->m_Nodes[1].m_Value > pNode->m_pNext->m_Value;
			pNode=SharedPhysics::Fast_Select<C_SweepAndPruneNode>(notFind, pNode->m_pNext, pNode);
		}
	
		m_XList.InsertAfter(pNode, &obj->m_Nodes[1]);
		
	}


	void C_SweepAndPrune::RemoveObject(C_SweepAndPruneObject* pObj)
	{
		m_ObjectList.Remove(pObj);
		m_ObjectAllocator.PushListToFree(pObj);
		m_XList.Remove(&pObj->m_Nodes[0]);
		m_XList.Remove(&pObj->m_Nodes[1]);
	}

	void DrawList(SharedPhysics::C_DoubleList<C_SweepAndPruneNode>& list, char* name)
	{
		//C_SweepAndPruneNode* pNode=list.GetHead();

		//Sys::I_BinaryStream* pStream = GetDefaultFileSystem()->Open(name, Sys::E_FileAccess_Write);
		//Sys::TextSerialiser archive(*pStream);

		//F32 value;
		//while(pNode->m_pNext)
		//{
		//	pNode=pNode->m_pNext;
		//	Maths::Store<Maths::HF32>(pNode->m_Value, &value);
		//	if(pNode->m_Type == SP_MIN)
		//		archive << "Object Index: " << pNode->m_pObj->m_pObj->m_Index << ", Value: "<< value<<", SP: MIN\n";
		//	else
		//		archive << "Object Index: " << pNode->m_pObj->m_pObj->m_Index << ", Value: "<< value<<", SP: MAX\n";

		//	
		//}
		//delete pStream;

	}



	//void C_SweepAndPrune::ConstructPair(SharedPhysics::C_DoubleList<C_SweepAndPruneNode> &list)
	//{
	//	
	//	C_SweepAndPruneNode* pNode=list.GetFirstElem();

	//	while(pNode)
	//	{
	//		if(pNode->m_Type == SP_MIN)
	//		{
	//			//Loop through all the active objects and construct active pair before put it into the sweep list
	//			C_SweepAndPruneObjectPtr* ptr=m_ActiveObjectList.GetFirstElem();

	//			while(ptr)
	//			{
	//				C_SweepAndPrunePair* pair=m_ActivePairBufferAllocator.GetNextFreeItem();
	//				pair->SetObjects(pNode->m_pObj, ptr->m_pObj);
	//				SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair> wrapper(pair);
	//				m_ActivePairBuffer.AddToEnd(&wrapper);
	//				ptr=ptr->m_pNext;
	//			
	//			}			
	//			this->m_ActiveObjectList.InsertToEnd(pNode->m_pObj->m_pMyself);
	//			
	//		}
	//		else
	//		{
	//			this->m_ActiveObjectList.Remove(pNode->m_pObj->m_pMyself);
	//		}

	//		pNode=pNode->m_pNext;

	//	}
	//}

	void C_SweepAndPrune::ConstructPair(SharedPhysics::C_DoubleList<C_SweepAndPruneNode> &list)
	{
		
		C_SweepAndPruneNode* pNode=list.GetFirstElem();

		while(pNode)
		{
			if(pNode->m_Type == SP_MIN)
			{
				//Loop through all the active objects and construct active pair before put it into the sweep list
				C_SweepAndPruneObjectPtr* ptr=m_ActiveObjectList.GetFirstElem();
			
				while(ptr)
				{
					C_SweepAndPruneObject* pObj=ptr->m_pObj;
					C_SweepAndPruneObject* pNodeObj=pNode->m_pObj;

					const Maths::Scalar_Bool conYA(pNodeObj->m_Max.Y().LT(pObj->m_Min.Y()));
					const Maths::Scalar_Bool conYB(pNodeObj->m_Min.Y().GT(pObj->m_Max.Y()));

					const Maths::Scalar_Bool conZA(pNodeObj->m_Max.Z().LT(pObj->m_Min.Z()));
					const Maths::Scalar_Bool conZB(pNodeObj->m_Min.Z().GT(pObj->m_Max.Z()));
					
					if((!( conZA | conZB )) & (!(conYA | conYB)))
					{
						C_SweepAndPrunePair* pair=m_ActivePairListAllocator.GetNextFreeItem();
						pair->SetObjects(pNodeObj, pObj);
						SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair> wrapper(pair);
						m_ActivePairList.AddToEnd(&wrapper);
					}
					ptr=ptr->m_pNext;
				
				}			
				this->m_ActiveObjectList.InsertToEnd(pNode->m_pObj->m_pMyself);
				
			}
			else
			{
				this->m_ActiveObjectList.Remove(pNode->m_pObj->m_pMyself);
			}

			pNode=pNode->m_pNext;

		}
	}

	//void C_SweepAndPrune::ConstructPair()
	//{
	//	
	//	for(S32 i=0; i<list.m_numElements; ++i)
	//	{
	//		SharedPhysics::C_RadixPointerWrapper<C_SweepAndPruneNode>& wrapper=list.m_pElems[i];
	//		C_SweepAndPruneNode* pNode=wrapper.m_pElement;

	//		if(pNode->m_Type == SP_MIN)
	//		{

	//			C_SweepAndPruneObjectPtr* ptr=m_ActiveObjectList.GetFirstElem();

	//			while(ptr)
	//			{
	//				C_SweepAndPruneObject* pObj=ptr->m_pObj;
	//				C_SweepAndPruneObject* pNodeObj=pNode->m_pObj;

	//				const Maths::Scalar_Bool conYA(pNodeObj->m_Max.Y().LT(pObj->m_Min.Y()));
	//				const Maths::Scalar_Bool conYB(pNodeObj->m_Min.Y().GT(pObj->m_Max.Y()));

	//				const Maths::Scalar_Bool conZA(pNodeObj->m_Max.Z().LT(pObj->m_Min.Z()));
	//				const Maths::Scalar_Bool conZB(pNodeObj->m_Min.Z().GT(pObj->m_Max.Z()));
	//				
	//				if((!( conZA | conZB )) & (!(conYA | conYB)))
	//				{
	//					C_SweepAndPrunePair* pair=m_ActivePairListAllocator.GetNextFreeItem();
	//					pair->SetObjects(pNodeObj, pObj);
	//					SharedPhysics::C_RadixPointerWrapper<C_SweepAndPrunePair> wrapper(pair);
	//					m_ActivePairList.AddToEnd(&wrapper);
	//				}
	//				ptr=ptr->m_pNext;
	//			
	//			}			
	//			this->m_ActiveObjectList.InsertToEnd(pNode->m_pObj->m_pMyself);
	//			
	//		}
	//		else
	//		{
	//			this->m_ActiveObjectList.Remove(pNode->m_pObj->m_pMyself);
	//		}
	//	}

	//}


	
		//This method use one sigle active pair list to store and sort each pair of active objects, then
		//check for the each pair, if there are three pair in the list, which mean this object pair collide
		//in three coordinates, the these two objects collided.
	
	void C_SweepAndPrune::UpdateObjects()
	{
		//C_SweepAndPruneObject* obj=this->m_ObjectList.GetFirstElem();

		//while(obj)
		//{
		//	if(obj->m_pObj->m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		//	{
		//		//When the object move or rotate, up the the AABB box around it
		//		obj->UpdateBox();
		//	}
		//	obj=obj->m_pNext;
		//}

		Resort(m_XList);

		//DrawList(m_XList, "sortedXList.txt");

		m_ActivePairList.m_numElements = 0;
		m_ActivePairListAllocator.Reset();

		m_ActiveObjectList.Reset();


		//Initialize the active pair list again
		ConstructPair(m_XList);

		//m_Sorter.SortUnsignedInt(m_ActivePairBuffer.m_pElems, m_ActivePairBuffer.m_numElements);


	}


	//
	//	This method use one sigle active pair list to store and sort each pair of active objects, then
	//	check for the each pair, if there are three pair in the list, which mean this object pair collide
	//	in three coordinates, the these two objects collided.
	
	//void C_SweepAndPrune::UpdateObjects()
	//{
	//	C_SweepAndPruneObject* obj=this->m_ObjectList.GetFirstElem();

	//	while(obj)
	//	{
	//		if(obj->m_pObj->m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//		{
	//			//When the object move or rotate, up the the AABB box around it
	//			obj->UpdateBox();
	//		}
	//		obj=obj->m_pNext;
	//	}
	//
	//	m_Sorter.SortFloat(list.m_pElems, list.m_numElements);


	//	m_ActivePairList.m_numElements = 0;
	//	m_ActivePairListAllocator.Reset();

	//	m_ActiveObjectList.Reset();


	//	//Initialize the active pair list
	//	ConstructPair();

	//	//m_Sorter.SortUnsignedInt(m_ActivePairBuffer.m_pElems, m_ActivePairBuffer.m_numElements);


	//}

	*/


};
