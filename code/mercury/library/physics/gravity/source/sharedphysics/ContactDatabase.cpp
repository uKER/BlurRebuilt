#include "StdAfx.h"
#include "gravity/sharedphysics/ContactDatabase.h"
#include "gravity/sharedphysics/ContactDatabase.inl"
#include "gravity/sharedphysics/AllocationManager.inl"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/sharedphysics/ContactStream.inl"
#include "system/perf.h"

namespace SharedPhysics
{
	C_Collider* C_ActiveContactManager::FindCollider(C_Collider* pCollider, S32& index) const
	{
		Spu::MemBaseMarker marker;
		C_ActiveResultPtr* pResults = (C_ActiveResultPtr*)Spu::FetchAllocUnaligned(m_ActiveArray.m_pElems, sizeof(C_ActiveResultPtr) * m_ActiveArray.m_numElements);

		int l = 0, r = m_ActiveArray.m_numElements, mid;
		while(l < r)
		{
			mid = (l + r)/2;
			//cmp = CompFunc(elem, m_pElems + mid);
			C_ActiveResultPtr& res = pResults[mid];
			SharedPhysics::C_Collider* pLocalCollider((SharedPhysics::C_Collider*)Spu::FetchAlloc(res.m_pCollider, sizeof(SharedPhysics::C_Collider)));
			
			if(*pCollider < *pLocalCollider)
			{
				r = mid;
			}
			else if(*pCollider > *pLocalCollider)
			{
				l = mid + 1;
			}
			else
			{
				return pResults[mid].m_pCollider;
			}
		}
		return NULL;
	}

	int CompareFuncA(const void* a, const void*b)
	{
		C_ActiveResultPtr* ptr = (C_ActiveResultPtr*)a;
		C_ActiveResultPtr* ptrB = (C_ActiveResultPtr*)b;
		return (*ptr <= *ptrB) ? -1 : 1;
	}

	int CompareFuncB(const void* a, const void*b)
	{
		C_ActiveResultRvPtr* ptr = (C_ActiveResultRvPtr*)a;
		C_ActiveResultRvPtr* ptrB = (C_ActiveResultRvPtr*)b;
		return (*ptr <= *ptrB) ? -1 : 1;
	}

	void C_ActiveContactManager::SortDatabase(C_S32SafeStack& stack, SharedPhysics::C_RadixSorter64<C_ActiveResultPtr>* pSorter)
	{
		PERF_EVENT("Gravity.ContactDatabase.Sort");
		/*pSorter->SortUnsigned64(m_ActiveArray.m_pElems, m_ActiveArray.m_numElements, 30);

		SharedPhysics::C_RadixSorter64<C_ActiveResultRvPtr>* pSorter2 = (SharedPhysics::C_RadixSorter64<C_ActiveResultRvPtr>*) pSorter;

		pSorter2->SortUnsigned64(m_ActiveRvArray.m_pElems, m_ActiveArray.m_numElements, 30);*/

		/*pSorter->SortUnsigned32(m_ActiveArray.m_pElems, m_ActiveArray.m_numElements, 30);

		SharedPhysics::C_RadixSorter64<C_ActiveResultRvPtr>* pSorter2 = (SharedPhysics::C_RadixSorter64<C_ActiveResultRvPtr>*) pSorter;

		pSorter2->SortUnsigned32(m_ActiveRvArray.m_pElems, m_ActiveArray.m_numElements, 30);*/


		m_ActiveArray.QuickSort(stack, 7);
		m_ActiveRvArray.QuickSort(stack, 7);

		/*qsort(m_ActiveArray.m_pElems, m_ActiveArray.m_numElements, sizeof(C_ActiveResultPtr), CompareFuncA);
		qsort(m_ActiveRvArray.m_pElems, m_ActiveRvArray.m_numElements, sizeof(C_ActiveResultRvPtr), CompareFuncB);*/
	}
	/*template <class DataType>
	bool FindRange(const SortedArray<DataType> & arr, U32 index, U32& low, U32& high)
	{
		S32 l = 0, r = arr.m_numElements;

		while(l < r)
		{
			const U32 mid((l + r) >> 1);
			const U32 comp(arr.m_pElems[mid].m_pElement->CompareFirstElement(index));
			if(comp < 0)
				r = mid;
			else if(comp > 0)
				l = mid + 1;
			else
			{
				//Found it, so find the low and high values for the range
				for(l = mid-1; (l >= 0) && (arr.m_pElems[l].m_pElement->CompareFirstElement(index) == 0); --l);
				for(r = mid; (r < arr.m_numElements) && (arr.m_pElems[r].m_pElement->CompareFirstElement(index) == 0); ++r);

				low = l + 1;
				high = r;
				return true;
			}
		}
		return false;
	}

	void C_ContactDatabase::WriteAllSleepyContactsToStream(C_GraphNode* pGraphNode, C_ContactStream& stream)
	{
		//Search through the 2 arrays and 
		U32 low1, high1, low2, high2;
		if(FindRange<C_ResultPtr>(m_SleepyContactArray, pGraphNode->m_Index, low1, high1))
		{
			//Write all contacts from low to high into contact stream and remove them from arrays...
			for(U32 a = low1; a < high1; ++a)
			{
				//Remove corresponding contact from the reverse buffer
				C_Collider* pCollider = m_pCurrentBuffer->GetCollider();
				C_CollisionResultDB* pDb(m_SleepyContactArray[a].m_pElement);
				pCollider->SetNodes(pDb->m_pNodeA, pDb->m_pNodeB);
				pCollider->m_WitnessPointA = pDb->m_WitnessPointLocalA;
				pCollider->m_WitnessPointB = pDb->m_WitnessPointLocalB;
				pCollider->m_State = C_Collider::E_Normal;
				pCollider->m_StreamPtr = stream;
				pCollider->m_pConstraintEdge = NULL;
				m_pCurrentBuffer->AddToDatabase(pCollider);
				//TODO add the corresponding collider to the current buffer of the database...
				EncodeContacts(*pDb, stream);
				ReleaseSleepyContact(pDb);
			}
			m_SleepyContactArray.Remove(low1, (high1 - low1));
		}

		if(FindRange<C_ResultRvPtr>(m_SleepyContactRvArray, pGraphNode->m_Index, low2, high2))
		{
			//Write all contacts from low to high into contact stream and remove them from arrays...
			for(U32 a = low2; a < high2; ++a)
			{
				C_Collider* pCollider = m_pCurrentBuffer->GetCollider();
				pCollider->m_pConstraintEdge = NULL;
				C_CollisionResultDBReverse* pRev(m_SleepyContactRvArray[a].m_pElement);
				C_CollisionResultDB* pDb(pRev->m_pResult);
				pCollider->SetNodes(pDb->m_pNodeB, pDb->m_pNodeA);
				pCollider->m_WitnessPointA = pDb->m_WitnessPointLocalA;
				pCollider->m_WitnessPointB = pDb->m_WitnessPointLocalB;
				pCollider->m_State = C_Collider::E_Normal;
				pCollider->m_StreamPtr = stream;
				m_pCurrentBuffer->AddToDatabase(pCollider);
				//TODO add the corresponding collider to the current buffer of the database...
				EncodeContacts(*pDb, stream);
				ReleaseSleepyContact(pRev);
			}
			m_SleepyContactRvArray.Remove(low2, (high2 - low2));
		}

		assert((((high1 - low1) + (high2 - low2)) != 0) && "Empty stream!");
	}

	void C_ContactDatabase::PurgeAllContacts(C_GraphNode* pGraphNode)
	{
		//Search through the 2 arrays and 
		U32 low, high;
		if(FindRange<C_ResultPtr>(m_SleepyContactArray, pGraphNode->m_Index, low, high))
		{
			//Write all contacts from low to high into contact stream and remove them from arrays...
			for(U32 a = low; a < high; ++a)
			{
				//Remove corresponding contact from the reverse buffer
				C_CollisionResultDB* pDb(m_SleepyContactArray[a].m_pElement);
				ReleaseSleepyContact(pDb);
			}
			m_SleepyContactArray.Remove(low, (high - low));
		}

		if(FindRange<C_ResultRvPtr>(m_SleepyContactRvArray, pGraphNode->m_Index, low, high))
		{
			//Write all contacts from low to high into contact stream and remove them from arrays...
			for(U32 a = low; a < high; ++a)
			{
				C_CollisionResultDBReverse* pRev(m_SleepyContactRvArray[a].m_pElement);
				ReleaseSleepyContact(pRev);
			}
			m_SleepyContactRvArray.Remove(low, (high - low));
		}
	}*/
};
