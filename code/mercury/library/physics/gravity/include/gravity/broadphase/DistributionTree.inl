#ifndef DISTRIBUTIONTREE_INL
#define DISTRIBUTIONTREE_INL

#include "gravity/broadphase/DistributionTree.h"
#include "gravity/sharedphysics/Select.h"
#include "gravity/broadphase/SweepAndPrune.h"

//using namespace SharedPhysics;
namespace Gravity
{
	NOALIAS inline U32 C_DistributionNode::ObjectCount() __restrict
	{
		return m_ObjectCount;
	}

	NOALIAS inline U32 C_DistributionNode::ChildObjectCount() __restrict
	{
		return m_pLeftNode->m_ObjectCount + m_pRightNode->m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::Remove(C_ObjectWrapper* pWrapper) __restrict
	{
		if(pWrapper->bDirty)
		{
			pWrapper->bDirty = false;
			return;
		}
		C_ObjectWrapper dummyWrapper;
		C_ObjectWrapper* next = SharedPhysics::Fast_Select<C_ObjectWrapper>(pWrapper->m_pNext != NULL, pWrapper->m_pNext, &dummyWrapper);
		next->m_pPrev = pWrapper->m_pPrev;
		C_ObjectWrapper* prev = SharedPhysics::Fast_Select<C_ObjectWrapper>(pWrapper->m_pPrev != NULL, pWrapper->m_pPrev, &dummyWrapper);
		prev->m_pNext = pWrapper->m_pNext;
		m_pActiveObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveObjects == pWrapper, pWrapper->m_pNext, m_pActiveObjects);
		m_pSleepingObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingObjects == pWrapper, pWrapper->m_pNext, m_pSleepingObjects);
		m_pStaticObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pStaticObjects == pWrapper, pWrapper->m_pNext, m_pStaticObjects);
		m_pKinematicObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pKinematicObjects == pWrapper, pWrapper->m_pNext, m_pKinematicObjects);
		m_pActiveClothObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveClothObjects == pWrapper, pWrapper->m_pNext, m_pActiveClothObjects);
		m_pSleepingClothObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingClothObjects == pWrapper, pWrapper->m_pNext, m_pSleepingClothObjects);
		m_pActiveFluidObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveFluidObjects == pWrapper, pWrapper->m_pNext, m_pActiveFluidObjects);
		m_pSleepingFluidObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingFluidObjects == pWrapper, pWrapper->m_pNext, m_pSleepingFluidObjects);
		m_pFluidMeshObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pFluidMeshObjects == pWrapper, pWrapper->m_pNext, m_pFluidMeshObjects);
		m_pRaytraceOnlyObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pRaytraceOnlyObjects == pWrapper, pWrapper->m_pNext, m_pRaytraceOnlyObjects);
		m_pActiveDebrisObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveDebrisObjects == pWrapper, pWrapper->m_pNext, m_pActiveDebrisObjects);
		m_pSleepingDebrisObjects = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingDebrisObjects == pWrapper, pWrapper->m_pNext, m_pSleepingDebrisObjects);
		--m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::AddToActive(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pActiveObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* activeRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveObjects == NULL, &dummyWrapper, m_pActiveObjects);
		activeRoot->m_pPrev = pWrapper;
		m_pActiveObjects = pWrapper;
		++m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::AddToActiveDebris(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pActiveDebrisObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* activeRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveDebrisObjects == NULL, &dummyWrapper, m_pActiveDebrisObjects);
		activeRoot->m_pPrev = pWrapper;
		m_pActiveDebrisObjects = pWrapper;
		++m_ObjectCount;
	}


	NOALIAS inline void C_DistributionNode::AddToActiveCloth(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pActiveClothObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* activeRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveClothObjects == NULL, &dummyWrapper, m_pActiveClothObjects);
		activeRoot->m_pPrev = pWrapper;
		m_pActiveClothObjects = pWrapper;
		++m_ObjectCount;
	}

	
	NOALIAS	inline void C_DistributionNode::AddToActiveFluid(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pActiveFluidObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* activeRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pActiveFluidObjects == NULL, &dummyWrapper, m_pActiveFluidObjects);
		activeRoot->m_pPrev = pWrapper;
		m_pActiveFluidObjects = pWrapper;
		++m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::AddToSleeping(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pSleepingObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* sleepingRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingObjects == NULL, &dummyWrapper, m_pSleepingObjects);
		sleepingRoot->m_pPrev = pWrapper;
		m_pSleepingObjects = pWrapper;
		++m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::AddToSleepingDebris(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pSleepingDebrisObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* sleepingRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingDebrisObjects == NULL, &dummyWrapper, m_pSleepingDebrisObjects);
		sleepingRoot->m_pPrev = pWrapper;
		m_pSleepingDebrisObjects = pWrapper;
		++m_ObjectCount;
	}


	NOALIAS inline void C_DistributionNode::AddToSleepingCloth(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pSleepingClothObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* sleepingRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingClothObjects == NULL, &dummyWrapper, m_pSleepingClothObjects);
		sleepingRoot->m_pPrev = pWrapper;
		m_pSleepingClothObjects = pWrapper;
		++m_ObjectCount;
	}

	NOALIAS	inline void C_DistributionNode::AddToSleepingFluid(C_ObjectWrapper* __restrict pWrapper)__restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pSleepingFluidObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* sleepingRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pSleepingFluidObjects == NULL, & dummyWrapper, m_pSleepingFluidObjects);
		sleepingRoot->m_pPrev = pWrapper;
		m_pSleepingFluidObjects = pWrapper;
		++m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::AddToStatic(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pStaticObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* staticRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pStaticObjects == NULL, &dummyWrapper, m_pStaticObjects);
		staticRoot->m_pPrev = pWrapper;
		m_pStaticObjects = pWrapper;
		++m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::AddToKinematic(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pKinematicObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* kinematicRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pKinematicObjects == NULL, &dummyWrapper, m_pKinematicObjects);
		kinematicRoot->m_pPrev = pWrapper;
		m_pKinematicObjects = pWrapper;
		++m_ObjectCount;
	}

	NOALIAS inline void C_DistributionNode::AddToRaytraceOnly(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		C_ObjectWrapper dummyWrapper;
		pWrapper->m_pNext = m_pRaytraceOnlyObjects;
		pWrapper->m_pPrev = NULL;
		C_ObjectWrapper* kinematicRoot = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_pRaytraceOnlyObjects == NULL, &dummyWrapper, m_pRaytraceOnlyObjects);
		kinematicRoot->m_pPrev = pWrapper;
		m_pRaytraceOnlyObjects = pWrapper;
		++m_ObjectCount;
	}


//#pragma optimize("", off)
	NOALIAS inline void C_DistributionNode::AddObject(C_ObjectWrapper* __restrict pWrapper) __restrict
	{
		const C_Object*  pObj(pWrapper->m_pObject);
		U32 state = pObj->m_ObjectState & (~(E_Dirty | E_Removed));
		//state types are static, sleeping, active...
		C_ObjectWrapper dummyWrapper;
		BIZ_ASSERT(state >= E_ActiveObject && state <= E_SleepingDebrisObject);
		//Bypass badness which comes from the assert above triggering if we continue...
		if(state >= E_ActiveObject && state <= E_SleepingDebrisObject)
		{
			C_ObjectWrapper* pArr = this->m_pObjectArrays[state];
			C_ObjectWrapper* pNext = SharedPhysics::Fast_Select<C_ObjectWrapper>(pArr == NULL, &dummyWrapper, pArr);
			pWrapper->m_pNext = pArr;
			pWrapper->m_pPrev = NULL;
			pNext->m_pPrev = pWrapper;
			m_pObjectArrays[state] = pWrapper;
			++m_ObjectCount;
		}
	}
//#pragma optimize("", on)
};

#endif


