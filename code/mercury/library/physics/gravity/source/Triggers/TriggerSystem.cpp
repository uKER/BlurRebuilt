#include "StdAfx.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/triggers/triggersystem.h"
#include "gravity/triggers/TriggerEvent.h"
#include "gravity/narrowphase/AxisBox.inl"
#include "gravity/narrowphase/CollisionManager.h"
#include <system/perf.h>

namespace Gravity
{  

	Debug::Bool g_dbTriggerDisableSplit("Gravity.TriggerSystem.DisableSplit", false);
	Debug::Bool g_dbTriggerDisableMerge("Gravity.TriggerSystem.DisableMerge", false);

	//U32 C_TriggerLayer::s_LayerIndex = 0;
	U32 C_Trigger::s_TriggerIndex = 0;
	U32 C_TriggerActor::s_TriggerActorIndex = 0;


	C_TriggerLayer::C_TriggerLayer(const Maths::HVec3 PCREF minBox, const Maths::HVec3 PCREF maxBox, 
		const Maths::HVec3 PCREF desiredCellSize, U32 maxDepth, U32 index,
		C_TriggerSystem* pTriggerSystem, const Maths::HVec3 PCREF maxCellSize) : m_pRootNode(NULL), 
		m_pActiveNodes(NULL), m_pSleepingNodes(NULL), m_pPrevActiveNodes(NULL),
		m_pPrevSleepingNodes(NULL), m_CountDownToUpdate(0), m_UpdateFrequency(4),
		m_LayerIndex(index), m_DesiredCellSize(desiredCellSize), m_MaxDepth(maxDepth),
		m_NeedsUpdate(false), m_pTriggerSystem(pTriggerSystem), m_MaxCellSize(maxCellSize),
		m_ActiveNodeCount(0)
	{
		m_LowObjectThreshold = 5;
		m_HighObjectThreshold = 20;
		m_pAllocator = new C_TriggerNodeAllocator;
		m_pRootNode = m_pAllocator->GetNextFreeItem();
		m_pRootNode->m_bbox.m_min = minBox;
		m_pRootNode->m_bbox.m_max = maxBox;
		m_pRootNode->Subdivide(m_DesiredCellSize, m_MaxDepth, *m_pAllocator, m_LeafNodes);
	}

	
	C_TriggerSystem* C_Trigger::GetTriggerSystem()
	{
		if (!m_pLayer)
			return(NULL);
		return(m_pLayer->GetTriggerSystem());
	}

	bool C_TriggerNode::IsActive()
	{
		return m_State == E_TriggerNode_Active;
	}

	bool C_TriggerNode::IsSleeping()
	{
		return m_State == E_TriggerNode_Inactive;
	}

	bool C_TriggerNode::MustActivate()
	{
		return m_pActiveTriggerActors != NULL || m_pActiveTriggers != NULL;
	}

	bool C_TriggerNode::MustAddToSleeping()
	{
		return (m_pActiveTriggerActors == NULL && m_pActiveTriggers == NULL) && 
			(m_pInactiveTriggers != NULL || m_pInactiveTriggerActors != NULL);	
	}

	U32 C_TriggerNode::TriggerAndTriggerActorCount()
	{
		return m_TriggerCount + m_TriggerActorCount;
	}

	void C_TriggerNode::Subdivide(const Maths::HVec3 PCREF desiredSize, const S32 maxDepth, C_TriggerNodeAllocator& memoryAllocator,
			C_TriggerLeafNodeList& leafNodes)
	{
		using namespace Maths;

		m_pActiveTriggers = m_pInactiveTriggers = NULL;
		m_pActiveTriggerActors = m_pInactiveTriggerActors = NULL;
		m_pLeftNode = m_pRightNode = NULL;
		m_pNext = NULL;
		m_State = E_TriggerNode_Inactive;
		//Don't subdivide if we've gone past max-depth or our bounding box is small-enough
		if(maxDepth <= 0 || desiredSize.All_GE(m_bbox.m_max - m_bbox.m_min))
		{
			leafNodes.AddNode(this);
			m_pLeftNode = m_pRightNode = NULL;
			return;
		}

		//No early-out's, so need to split this node...Do fat subdivision


		const Maths::HF32 half(Maths::Constants<HF32>::Half());
		const HVec3 _min(m_bbox.m_min);
		const HVec3 _max(m_bbox.m_max);
		HVec3 doubleExtents(_max - _min);
		const HF32 doubleX(doubleExtents.X());
		const HF32 doubleY(doubleExtents.Y());
		const HF32 doubleZ(doubleExtents.Z());

		const HVec3 ex(doubleExtents * half);
		const HF32 x(ex.X());
		const HF32 y(ex.Y());
		const HF32 z(ex.Z());

		HVec3 rightStart(_min);

		const HVec3 rightEnd(rightStart + ex);

		const Scalar_Bool con1(y.GT(x) & y.GE(z));
		const Scalar_Bool con2(z.GT(x));

		const HVec3 doubleExtentsX(x,		doubleY,	doubleZ);
		const HVec3 doubleExtentsY(doubleX, y,			doubleZ);
		const HVec3 doubleExtentsZ(doubleX,	doubleY,	z);

		const HVec3 rightStartX(rightEnd.X(),	rightStart.Y(), rightStart.Z());
		const HVec3 rightStartY(rightStart.X(), rightEnd.Y(),	rightStart.Z());
		const HVec3 rightStartZ(rightStart.X(), rightStart.Y(), rightEnd.Z());

		doubleExtents = Select(con1, doubleExtentsY, Select(con2, doubleExtentsZ, doubleExtentsX));
		rightStart = Select(con1, rightStartY, Select(con2, rightStartZ, rightStartX));


		//if(ex.Y().GT(ex.X()) &&
		//	ex.Y().GE(ex.Z()))
		//{
		//	doubleExtents.Y(ex.Y());
		//	rightStart.Y(rightStart.Y() + ex.Y());
		//}
		//else if(ex.Z().GT(ex.X())) //Y can't be the largest, so x or z must be!
		//{
		//	doubleExtents.Z(ex.Z());
		//	rightStart.Z(rightStart.Z() + ex.Z());
		//}
		//else
		//{
		//	doubleExtents.X(ex.X());
		//	rightStart.X(rightStart.X() + ex.X());
		//}
		//const HVec3 extents(doubleExtents * half);
		m_pLeftNode = memoryAllocator.GetNextFreeItem();
		m_pRightNode = memoryAllocator.GetNextFreeItem();
		m_pLeftNode->m_bbox.m_min = _min;
		m_pLeftNode->m_bbox.m_max = _min + doubleExtents;
		m_pLeftNode->m_pParentNode = this;
		m_pRightNode->m_bbox.m_min = rightStart;
		m_pRightNode->m_bbox.m_max = _max;
		m_pRightNode->m_pParentNode = this;
		m_pLeftNode->Subdivide(desiredSize, maxDepth-1, memoryAllocator, leafNodes);
		m_pRightNode->Subdivide(desiredSize, maxDepth-1, memoryAllocator, leafNodes);
	}

	C_TriggerLayer::~C_TriggerLayer()
	{
		delete m_pAllocator;
	}

	C_Trigger::~C_Trigger()
	{
		delete m_pTriggerShape;
		m_pTriggerShape = NULL;
		m_pUserData = NULL;
	}


	void C_Trigger::Activate()
	{
		if((m_State & (~(E_DirtyTrigger | E_Processed_Active))) == E_InactiveTrigger_State)
		{
			for(U32 a = 0; a < this->m_NumNodesContained; ++a)
			{
				C_TriggerWrapper* pWrapper(&m_Actors[a]);
				pWrapper->m_pNode->Remove(pWrapper);
				pWrapper->m_pNode->AddToActive(pWrapper);
			}
			m_State = ((m_State | E_ActiveTrigger_State) & (~E_InactiveTrigger_State));
		}
		m_State = m_State & (~E_Processed_Active);
	}

	void C_Trigger::Deactivate()
	{
		if((m_State & (~(E_DirtyTrigger | E_Processed_Active))) == E_ActiveTrigger_State)
		{
			for(U32 a = 0; a < m_NumNodesContained; ++a)
			{
				C_TriggerWrapper* pWrapper(&m_Actors[a]);
				pWrapper->m_pNode->Remove(pWrapper);
				pWrapper->m_pNode->AddToInactive(pWrapper);
			}
			m_State = (m_State | E_InactiveTrigger_State) & (~E_ActiveTrigger_State);
		}
		m_State = m_State & (~E_Processed_Active);
	}

	void C_TriggerActor::Activate()
	{
		if((m_State & (~(E_DirtyTrigger | E_Processed_Active))) == E_InactiveTrigger_State)
		{
			for(U32 a = 0; a < m_NumNodesContained; ++a)
			{
				C_TriggerActorWrapper* pWrapper(&m_Actors[a]);
				pWrapper->m_pNode->Remove(pWrapper);
				pWrapper->m_pNode->AddToActive(pWrapper);
			}
			m_State = ((m_State | E_ActiveTrigger_State) & (~E_InactiveTrigger_State));
		}
	}

	void C_TriggerActor::Deactivate()
	{
		if((m_State & (~(E_DirtyTrigger | E_Processed_Active))) == E_ActiveTrigger_State)
		{
			for(U32 a = 0; a < m_NumNodesContained; ++a)
			{
				C_TriggerActorWrapper* pWrapper(&m_Actors[a]);
				pWrapper->m_pNode->Remove(pWrapper);
				pWrapper->m_pNode->AddToInactive(pWrapper);
			}

			m_State = (m_State | E_InactiveTrigger_State) & (~E_ActiveTrigger_State);
		}
	}

	bool C_Trigger::IsUpdateRequired()
	{
		//return (!m_NodeBox.Contains(m_TransformedBox));
		//if((m_State & (~(E_DirtyTrigger | E_Processed_Active))) != 0 || m_NumNodesContained == 0)
		if(((m_State & E_DirtyTrigger) != 0) || m_NumNodesContained == 0)
		{
			m_State &= ~E_DirtyTrigger;
			return true;
		}
		if(m_State ==  E_InactiveTrigger_State)
			return false;

		if(m_NodeBox.Contains(m_TransformedBox))
		{
			const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
			const Maths::HF32 volume(m_TransformedBox.Volume());
			Maths::HF32 intersectVolume(Maths::Constants<Maths::HF32>::Zero());
			Collision::C_AxisBoxMinMax box;
			box.SetInvalidExtents();
			//Loop through all the contained nodes to see if there are any which this aabb is no longer inside...
			for(U32 a = 0; a < m_NumNodesContained; ++a)
			{
				if(!m_Actors[a].m_pNode->m_bbox.Intersects(m_TransformedBox))
				{
					RemoveFrom(a);
					--a;
				}
				else
				{
					intersectVolume += m_TransformedBox.IntersectionVolume(m_Actors[a].m_pNode->m_bbox.m_min, 
						m_Actors[a].m_pNode->m_bbox.m_max);
					box.m_min = Maths::Min(box.m_min, m_Actors[a].m_pNode->m_bbox.m_min);
					box.m_max = Maths::Max(box.m_max, m_Actors[a].m_pNode->m_bbox.m_max);
				}
			}
			m_NodeBox.m_min = box.m_min;
			m_NodeBox.m_max = box.m_max;
			
			return (volume - intersectVolume) >= epsilon;
		}
		return true;
	}

	bool C_TriggerActor::IsUpdateRequired()
	{
		//return (!m_NodeBox.Contains(m_TransformedBox));
		//if((m_State & (~(E_DirtyTrigger | E_Processed_Active))) != 0 || m_NumNodesContained == 0)
		if((m_State & E_DirtyTrigger) != 0 || m_NumNodesContained == 0)
		{
			m_State &= ~E_DirtyTrigger;
			return true;
		}
		if(m_State ==  E_InactiveTrigger_State)
			return false;

		if(m_NodeBox.Contains(m_TransformedBox))
		{
			const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
			const Maths::HF32 volume(m_TransformedBox.Volume());
			Maths::HF32 intersectVolume(Maths::Constants<Maths::HF32>::Zero());
			Collision::C_AxisBoxMinMax box;
			box.SetInvalidExtents();
			//Loop through all the contained nodes to see if there are any which this aabb is no longer inside...
			for(U32 a = 0; a < m_NumNodesContained; ++a)
			{
				if(!m_Actors[a].m_pNode->m_bbox.Intersects(m_TransformedBox))
				{
					RemoveFrom(a);
					--a;
				}
				else
				{
					intersectVolume += m_TransformedBox.IntersectionVolume(m_Actors[a].m_pNode->m_bbox.m_min, 
						m_Actors[a].m_pNode->m_bbox.m_max);
					box.m_min = Maths::Min(box.m_min, m_Actors[a].m_pNode->m_bbox.m_min);
					box.m_max = Maths::Max(box.m_max, m_Actors[a].m_pNode->m_bbox.m_max);
				}
			}
			m_NodeBox.m_min = box.m_min;
			m_NodeBox.m_max = box.m_max;
			
			return (volume - intersectVolume) >= epsilon;
		}
		return true;
	}


	void C_Trigger::Transform(const Maths::HMat43 PCREF matrix)
	{
		Collision::C_CollisionManager::TransformShapeAndBox(m_pTriggerShape, matrix, m_TransformedBox);
	}

	void C_TriggerActor::Transform(const Maths::HMat43 PCREF matrix)
	{
		m_pShape->Transform(matrix);
		m_pShape->GetTransformedBoundingBox(m_TransformedBox);
	}

	void C_TriggerActor::RemoveFrom(const U32 index)
	{
		//Remove from the box with index "index"
		m_Actors[index].m_pNode->Remove(&m_Actors[index]);
		C_TriggerActorWrapper dummy;
		//Now need to shuffle down all the previous ones...

		for(U32 a = index + 1; a < m_NumNodesContained; ++a)
		{
			m_Actors[a-1].m_pNode = m_Actors[a].m_pNode;
			m_Actors[a-1].m_pNext = m_Actors[a].m_pNext;
			m_Actors[a-1].m_pPrev = m_Actors[a].m_pPrev;
			C_TriggerActorWrapper* prev(SharedPhysics::Fast_Select<C_TriggerActorWrapper>(m_Actors[a].m_pPrev == NULL, &dummy, m_Actors[a].m_pPrev));
			prev->m_pNext = &m_Actors[a-1];
			C_TriggerActorWrapper* next(SharedPhysics::Fast_Select<C_TriggerActorWrapper>(m_Actors[a].m_pNext == NULL, &dummy, m_Actors[a].m_pNext));
			next->m_pPrev = &m_Actors[a-1];

			//assert((m_Actors[a].m_pActor->m_State & E_DirtyTrigger) == 0);
			//Test to see if I need to put this wrapper at the head of the linked list/remove it from the previous one...
			m_Actors[a].m_pNode->m_pTriggerActorArray[m_Actors[a].m_pActor->m_State] = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(m_Actors[a].m_pPrev == NULL, &m_Actors[a-1], m_Actors[a].m_pNode->m_pTriggerActorArray[m_Actors[a].m_pActor->m_State]);
		}
		--m_NumNodesContained;
	}

	void C_Trigger::RemoveFrom(const U32 index)
	{
		//Remove from the box with index "index"
		m_Actors[index].m_pNode->Remove(&m_Actors[index]);
		C_TriggerWrapper dummy;
		//Now need to shuffle down all the previous ones...

		for(U32 a = index + 1; a < m_NumNodesContained; ++a)
		{
			m_Actors[a-1].m_pNode = m_Actors[a].m_pNode;
			m_Actors[a-1].m_pNext = m_Actors[a].m_pNext;
			m_Actors[a-1].m_pPrev = m_Actors[a].m_pPrev;
			C_TriggerWrapper* prev(SharedPhysics::Fast_Select<C_TriggerWrapper>(m_Actors[a].m_pPrev == NULL, &dummy, m_Actors[a].m_pPrev));
			prev->m_pNext = &m_Actors[a-1];
			C_TriggerWrapper* next(SharedPhysics::Fast_Select<C_TriggerWrapper>(m_Actors[a].m_pNext == NULL, &dummy, m_Actors[a].m_pNext));
			next->m_pPrev = &m_Actors[a-1];

			//assert((m_Actors[a].m_pTrigger->m_State & E_DirtyTrigger) == 0);
			//Test to see if I need to put this wrapper at the head of the linked list/remove it from the previous one...
			m_Actors[a].m_pNode->m_pTriggerArray[m_Actors[a].m_pTrigger->m_State] = SharedPhysics::Fast_Select<C_TriggerWrapper>(m_Actors[a].m_pPrev == NULL, &m_Actors[a-1], m_Actors[a].m_pNode->m_pTriggerArray[m_Actors[a].m_pTrigger->m_State]);
		}
		--m_NumNodesContained;
	}

	void C_TriggerNode::Remove(C_TriggerWrapper* pWrapper)
	{
		if(pWrapper->bDirty)
		{
			pWrapper->bDirty = false;
			return;
		}
		C_TriggerWrapper dummyWrapper;
		C_TriggerWrapper* next = SharedPhysics::Fast_Select<C_TriggerWrapper>(pWrapper->m_pNext != NULL, pWrapper->m_pNext, &dummyWrapper);
		next->m_pPrev = pWrapper->m_pPrev;
		C_TriggerWrapper* prev = SharedPhysics::Fast_Select<C_TriggerWrapper>(pWrapper->m_pPrev != NULL, pWrapper->m_pPrev, &dummyWrapper);
		prev->m_pNext = pWrapper->m_pNext;

		m_pActiveTriggers = SharedPhysics::Fast_Select<C_TriggerWrapper>(m_pActiveTriggers == pWrapper, pWrapper->m_pNext, m_pActiveTriggers);
		m_pInactiveTriggers = SharedPhysics::Fast_Select<C_TriggerWrapper>(m_pInactiveTriggers == pWrapper, pWrapper->m_pNext, m_pInactiveTriggers);

		pWrapper->m_pNext = pWrapper->m_pPrev = NULL;
		--m_TriggerCount;
	}

	void C_TriggerNode::Remove(C_TriggerActorWrapper* pWrapper)
	{
		if(pWrapper->bDirty)
		{
			pWrapper->bDirty = false;
			return;
		}
		C_TriggerActorWrapper dummyWrapper;
		C_TriggerActorWrapper* next = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(pWrapper->m_pNext != NULL, pWrapper->m_pNext, &dummyWrapper);
		next->m_pPrev = pWrapper->m_pPrev;
		C_TriggerActorWrapper* prev = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(pWrapper->m_pPrev != NULL, pWrapper->m_pPrev, &dummyWrapper);
		prev->m_pNext = pWrapper->m_pNext;

		m_pActiveTriggerActors = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(m_pActiveTriggerActors == pWrapper, pWrapper->m_pNext, m_pActiveTriggerActors);
		m_pInactiveTriggerActors = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(m_pInactiveTriggerActors == pWrapper, pWrapper->m_pNext, m_pInactiveTriggerActors);
		pWrapper->m_pNext = pWrapper->m_pPrev = NULL;
		--m_TriggerActorCount;
	}

	void C_TriggerNode::AddToActive(C_TriggerWrapper* pWrapper)
	{
		C_TriggerWrapper dummyWrapper;
		C_TriggerWrapper* next = SharedPhysics::Fast_Select<C_TriggerWrapper>(m_pActiveTriggers != NULL, m_pActiveTriggers, &dummyWrapper);
		next->m_pPrev = pWrapper;
		pWrapper->m_pNext = m_pActiveTriggers;
		m_pActiveTriggers = pWrapper;
		++m_TriggerCount;
	}

	void C_TriggerNode::AddToInactive(C_TriggerWrapper* pWrapper)
	{
		C_TriggerWrapper dummyWrapper;
		C_TriggerWrapper* next = SharedPhysics::Fast_Select<C_TriggerWrapper>(m_pInactiveTriggers != NULL, m_pInactiveTriggers, &dummyWrapper);
		next->m_pPrev = pWrapper;
		pWrapper->m_pNext = m_pInactiveTriggers;
		m_pInactiveTriggers = pWrapper;
		++m_TriggerCount;
	}

	void C_TriggerNode::AddToActive(C_TriggerActorWrapper* pWrapper)
	{
		C_TriggerActorWrapper dummyWrapper;
		C_TriggerActorWrapper* next = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(m_pActiveTriggerActors != NULL, m_pActiveTriggerActors, &dummyWrapper);
		next->m_pPrev = pWrapper;
		pWrapper->m_pNext = m_pActiveTriggerActors;
		m_pActiveTriggerActors = pWrapper;
		++m_TriggerActorCount;
	}

	void C_TriggerNode::AddToInactive(C_TriggerActorWrapper* pWrapper)
	{
		C_TriggerActorWrapper dummyWrapper;
		C_TriggerActorWrapper* next = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(m_pInactiveTriggerActors != NULL, m_pInactiveTriggerActors, &dummyWrapper);
		next->m_pPrev = pWrapper;
		pWrapper->m_pNext = m_pInactiveTriggerActors;
		m_pInactiveTriggerActors = pWrapper;
		++m_TriggerActorCount;
	}

	void C_TriggerNode::AddTrigger(C_TriggerWrapper* pWrapper)
	{
		const C_Trigger* pTrig(pWrapper->m_pTrigger);
		U32 state = pTrig->m_State & (~(E_DirtyTrigger | E_Processed_Active));
		//state types are static, sleeping, active...
		C_TriggerWrapper dummyWrapper;
		BIZ_ASSERT(state >= E_InactiveTrigger_State && state <= E_ActiveTrigger_State);
		C_TriggerWrapper* pArr = m_pTriggerArray[state];
		C_TriggerWrapper* pNext = SharedPhysics::Fast_Select<C_TriggerWrapper>(pArr == NULL, &dummyWrapper, pArr);
		pWrapper->m_pNext = pArr;
		pWrapper->m_pPrev = NULL;
		pNext->m_pPrev = pWrapper;
		m_pTriggerArray[state] = pWrapper;
		++m_TriggerCount;
	}

	void C_TriggerNode::AddTriggerActor(C_TriggerActorWrapper* pWrapper)
	{
		const C_TriggerActor* pTrig(pWrapper->m_pActor);
		U32 state = pTrig->m_State & (~(E_DirtyTrigger | E_Processed_Active));
		//state types are static, sleeping, active...
		C_TriggerActorWrapper dummyWrapper;
		BIZ_ASSERT(state >= E_InactiveTrigger_State && state <= E_ActiveTrigger_State);
		C_TriggerActorWrapper* pArr = m_pTriggerActorArray[state];
		C_TriggerActorWrapper* pNext = SharedPhysics::Fast_Select<C_TriggerActorWrapper>(pArr == NULL, &dummyWrapper, pArr);
		pWrapper->m_pNext = pArr;
		pWrapper->m_pPrev = NULL;
		pNext->m_pPrev = pWrapper;
		m_pTriggerActorArray[state] = pWrapper;
		++m_TriggerActorCount;
	}

	C_TriggerNode* C_TriggerNode::Split(C_TriggerNodeAllocator& memoryAllocator, C_TriggerLeafNodeIterator& leafNodes, Maths::HVec3 PCREF cellSize, C_TriggerNode* pRoot, C_TriggerLayer& tree)
	{
		using namespace Maths;

		const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HVec3 twoCellSize(cellSize * two);
		if(g_dbTriggerDisableSplit || (twoCellSize).All_GE(m_bbox.m_max - m_bbox.m_min))
		{
			return pRoot;
		}

		//m_NodeCount += 2;
		//m_TriggerCount +=2;
		//m_TriggerActorCount +=2;

		//Need to look at the bounding boxes of the objects in the cell and split accordingly!
		//First pick fattest axis, and then sort objects by their extents and find the mid-point to split them...

		const HF32 half(Maths::Constants<HF32>::Half());
		const HVec3 _min(m_bbox.m_min);
		const HVec3 _max(m_bbox.m_max);
		HVec3 doubleExtents(_max - _min);
		const HF32 doubleX(doubleExtents.X());
		const HF32 doubleY(doubleExtents.Y());
		const HF32 doubleZ(doubleExtents.Z());

		const HVec3 ex(doubleExtents * half);
		const HF32 x(ex.X());
		const HF32 y(ex.Y());
		const HF32 z(ex.Z());

		HVec3 rightStart(_min);

		const HVec3 rightEnd(rightStart + ex);

		const Scalar_Bool con1(y.GT(x) & y.GE(z));
		const Scalar_Bool con2(z.GT(x));

		const HVec3 doubleExtentsX(x,		doubleY,	doubleZ);
		const HVec3 doubleExtentsY(doubleX, y,			doubleZ);
		const HVec3 doubleExtentsZ(doubleX,	doubleY,	z);

		const HVec3 rightStartX(rightEnd.X(),	rightStart.Y(), rightStart.Z());
		const HVec3 rightStartY(rightStart.X(), rightEnd.Y(),	rightStart.Z());
		const HVec3 rightStartZ(rightStart.X(), rightStart.Y(), rightEnd.Z());

		doubleExtents = Select(con1, doubleExtentsY, Select(con2, doubleExtentsZ, doubleExtentsX));
		rightStart = Select(con1, rightStartY, Select(con2, rightStartZ, rightStartX));


		//const HVec3 extents(doubleExtents * half);
		m_pLeftNode = memoryAllocator.GetNextFreeItem();
		m_pRightNode = memoryAllocator.GetNextFreeItem();
		m_pLeftNode->m_bbox.m_min = _min;
		m_pLeftNode->m_bbox.m_max = _min + doubleExtents;
		m_pLeftNode->m_pParentNode = this;
		m_pRightNode->m_bbox.m_min = rightStart;
		m_pRightNode->m_bbox.m_max = _max;
		m_pRightNode->m_pParentNode = this;

	
		C_TriggerWrapper* pWrapper(m_pActiveTriggers);
		while(pWrapper)
		{
			C_Trigger* pObj = pWrapper->m_pTrigger;
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pInactiveTriggers;
		while(pWrapper)
		{
			C_Trigger* pObj = pWrapper->m_pTrigger;
		//	pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		C_TriggerActorWrapper* pActorWrapper(m_pActiveTriggerActors);
		while(pActorWrapper)
		{
			C_TriggerActor* pObj = pActorWrapper->m_pActor;
			pObj->MarkDirty();
			pActorWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pActorWrapper = pActorWrapper->m_pNext;
		}

		pActorWrapper = m_pInactiveTriggerActors;
		while(pActorWrapper)
		{
			C_TriggerActor* pObj = pActorWrapper->m_pActor;
			pObj->MarkDirty();
			pActorWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pActorWrapper = pActorWrapper->m_pNext;
		}

		leafNodes.AddNode(m_pLeftNode);
		leafNodes.AddNode(m_pRightNode);
		leafNodes.Remove(this);

		m_pLeftNode->m_State = E_TriggerNode_Inactive;
		m_pRightNode->m_State = E_TriggerNode_Inactive;

		m_pActiveTriggers = m_pInactiveTriggers= NULL;
		m_pActiveTriggerActors = m_pInactiveTriggerActors = NULL;
		m_pLeftNode->m_pActiveTriggers= m_pLeftNode->m_pInactiveTriggers = NULL;
		m_pRightNode->m_pActiveTriggers = m_pRightNode->m_pInactiveTriggers = NULL;
		m_pLeftNode->m_pActiveTriggerActors = m_pLeftNode->m_pInactiveTriggerActors = NULL;
		m_pRightNode->m_pActiveTriggerActors = m_pRightNode->m_pInactiveTriggerActors = NULL;

		//m_ObjectCount = 0;
		m_TriggerCount = 0;
		m_TriggerActorCount = 0;

		return pRoot;
	}

	C_TriggerNode* C_TriggerNode::Merge(C_TriggerNodeAllocator& memoryAllocator, C_TriggerLeafNodeIterator& leafNodes, TriggerLeafState& state,
		const Maths::HVec3 PCREF maxCellSize, C_TriggerLayer& tree)
	{
		if(g_dbTriggerDisableMerge || maxCellSize.All_LE(m_bbox.m_max - m_bbox.m_min))
			return state.m_pActiveRoot;
		//return state.m_pActiveRoot;
		//m_NodeCount -= 2;
		//m_TriggerCount -= 2;
		//m_TriggerActorCount -= 2;
		
	
		C_TriggerWrapper* pWrapper(m_pLeftNode->m_pActiveTriggers);
		while(pWrapper)
		{
			C_Trigger* pObj = pWrapper->m_pTrigger;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pInactiveTriggers;
		while(pWrapper)
		{
			C_Trigger* pObj = pWrapper->m_pTrigger;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pWrapper = pWrapper->m_pNext;
		}

	
		pWrapper = m_pRightNode->m_pActiveTriggers;
		while(pWrapper)
		{
			C_Trigger* pObj = pWrapper->m_pTrigger;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pInactiveTriggers;
		while(pWrapper)
		{
			C_Trigger* pObj = pWrapper->m_pTrigger;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		C_TriggerActorWrapper* pActorWrapper(m_pLeftNode->m_pActiveTriggerActors);
		while(pActorWrapper)
		{
			C_TriggerActor* pObj = pActorWrapper->m_pActor;
			pObj->MarkDirty();
			pActorWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pActorWrapper = pActorWrapper->m_pNext;
		}

		pActorWrapper = m_pLeftNode->m_pInactiveTriggerActors;
		while(pActorWrapper)
		{
			C_TriggerActor* pObj = pActorWrapper->m_pActor;
			pObj->MarkDirty();
			pActorWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pActorWrapper = pActorWrapper->m_pNext;
		}

		pActorWrapper = m_pRightNode->m_pActiveTriggerActors;
		while(pActorWrapper)
		{
			C_TriggerActor* pObj = pActorWrapper->m_pActor;
			pObj->MarkDirty();
			pActorWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pActorWrapper = pActorWrapper->m_pNext;
		}

		pActorWrapper = m_pRightNode->m_pInactiveTriggerActors;
		while(pActorWrapper)
		{
			C_TriggerActor* pObj = pActorWrapper->m_pActor;
			pObj->MarkDirty();
			pActorWrapper->bDirty = true;
			tree.AddToDirty(pObj);
			pActorWrapper = pActorWrapper->m_pNext;
		}

		
		C_TriggerNode* pLeftNode(m_pLeftNode), *pRightNode(m_pRightNode);
		//C_TriggerNode* pNextNode(SharedPhysics::Fast_Select<C_TriggerNode>(m_pLeftNode == state.m_pActiveRoot, m_pLeftNode->m_pNext, m_pRightNode->m_pNext));
		//In case it was left node, then right node or right node, then left node...
		//pNextNode = SharedPhysics::Fast_Select<C_TriggerNode>(pNextNode == m_pLeftNode, m_pLeftNode->m_pNext, SharedPhysics::Fast_Select<C_TriggerNode>(pNextNode == m_pRightNode, m_pRightNode->m_pNext, pNextNode));
		
		C_TriggerNode* pNextNode(SharedPhysics::Fast_Select<C_TriggerNode>(m_pLeftNode == state.m_pActiveRoot, SharedPhysics::Fast_Select<C_TriggerNode>(m_pLeftNode->m_pNext == m_pRightNode, m_pRightNode->m_pNext, m_pLeftNode->m_pNext), 
			SharedPhysics::Fast_Select<C_TriggerNode>(m_pRightNode->m_pNext == m_pLeftNode, m_pLeftNode->m_pNext, m_pRightNode->m_pNext)));
		
		C_TriggerNode* pNextSleepingNode(SharedPhysics::Fast_Select<C_TriggerNode>(m_pLeftNode == state.m_pInactiveRoot, m_pLeftNode->m_pNext, m_pRightNode->m_pNext));
		pNextSleepingNode = SharedPhysics::Fast_Select<C_TriggerNode>(m_pLeftNode == state.m_pInactiveRoot, m_pLeftNode->m_pNext, m_pRightNode->m_pNext);

		if(state.m_pInactiveRoot == m_pLeftNode || state.m_pInactiveRoot == m_pRightNode)
		{
			state.m_pInactiveRoot = state.m_pInactiveRoot->m_pNext;
			if(state.m_pInactiveRoot == m_pLeftNode || state.m_pInactiveRoot== m_pRightNode)
			{
				state.m_pInactiveRoot = state.m_pInactiveRoot->m_pNext;
			}
		}
		if(state.m_pActiveRoot == m_pLeftNode || state.m_pActiveRoot == m_pRightNode)
		{
			state.m_pActiveRoot = state.m_pActiveRoot->m_pNext;
			if(state.m_pActiveRoot == m_pLeftNode || state.m_pActiveRoot == m_pRightNode)
			{
				state.m_pActiveRoot = state.m_pActiveRoot->m_pNext;
			}
		}

		//if(m_pLeftNode->IsActive())
		{
			//OK - it was active, so let's remove it from the active list
			C_TriggerNode* nextNode(SharedPhysics::Fast_Select<C_TriggerNode>(m_pLeftNode->m_pNext != NULL, m_pLeftNode->m_pNext, m_pLeftNode));
			C_TriggerNode* prevNode(SharedPhysics::Fast_Select<C_TriggerNode>(m_pLeftNode->m_pPrev != NULL, m_pLeftNode->m_pPrev, m_pLeftNode));
			nextNode->m_pPrev = m_pLeftNode->m_pPrev;
			prevNode->m_pNext = m_pLeftNode->m_pNext;
			m_pLeftNode->m_pNext = m_pLeftNode->m_pPrev = NULL;
		}

		//if(m_pRightNode->IsActive())
		{
			//OK - it was active, so let's remove it from the active list
			C_TriggerNode* nextNode(SharedPhysics::Fast_Select<C_TriggerNode>(m_pRightNode->m_pNext != NULL, m_pRightNode->m_pNext, m_pRightNode));
			C_TriggerNode* prevNode(SharedPhysics::Fast_Select<C_TriggerNode>(m_pRightNode->m_pPrev != NULL, m_pRightNode->m_pPrev, m_pRightNode));
			nextNode->m_pPrev = m_pRightNode->m_pPrev;
			prevNode->m_pNext = m_pRightNode->m_pNext;
			m_pRightNode->m_pNext = m_pRightNode->m_pPrev = NULL;
		}

		m_pLeftNode->m_State = E_TriggerNode_Inactive;
		m_pRightNode->m_State = E_TriggerNode_Inactive;

		m_pLeftNode->m_pActiveTriggers= m_pLeftNode->m_pInactiveTriggers =		
		m_pRightNode->m_pActiveTriggers = m_pRightNode->m_pInactiveTriggers= NULL;
		m_pLeftNode->m_pActiveTriggerActors= m_pLeftNode->m_pInactiveTriggerActors =		
		m_pRightNode->m_pActiveTriggerActors = m_pRightNode->m_pInactiveTriggerActors= NULL;

	/*	m_pLeftNode->m_ObjectCount = 0;
		m_pRightNode->m_ObjectCount = 0;*/
		m_pLeftNode->m_TriggerCount = 0;
		m_pRightNode->m_TriggerCount = 0;

		m_pLeftNode->m_TriggerActorCount = 0;
		m_pRightNode->m_TriggerActorCount = 0;
		
		leafNodes.AddNode(this);
		leafNodes.Remove(m_pLeftNode);
		leafNodes.Remove(m_pRightNode);

		memoryAllocator.PushToFree(m_pLeftNode);
		memoryAllocator.PushToFree(m_pRightNode);
//		m_pActiveObjects = m_pSleepingObjects = m_pActiveClothObjects = m_pSleepingClothObjects = NULL;
		m_pLeftNode->m_pParentNode = m_pRightNode->m_pParentNode = NULL;
		m_pLeftNode = m_pRightNode = NULL;

		return state.m_pActiveRoot;//SharedPhysics::Fast_Select<C_DistributionNode>(pLeftNode == pRootNode || pRightNode == pRootNode, pNextNode, pRootNode);
	}



	void C_TriggerLayer::SetMaxCellSize(const Maths::HVec3 PCREF maxCellSize)
	{
		m_MaxCellSize = maxCellSize;
	}

	void C_TriggerLayer::AddTrigger(C_Trigger* pTrigger)
	{
		C_SortedTriggerWrapper wrapper(pTrigger);
		m_TriggerArray.Insert(&wrapper);
		pTrigger->m_pLayer = this;
		//m_DirtyTriggerArray.AddToEnd(&pTrigger);

	}
	void C_TriggerLayer::RemoveTrigger(C_Trigger* pTrigger)
	{
		//Remove from all the nodes!!!
		for(U32 a = 0; a < pTrigger->m_NumNodesContained; ++a)
		{
			pTrigger->m_Actors[a].m_pNode->Remove(&pTrigger->m_Actors[a]);
		}
		C_SortedTriggerWrapper wrapper(pTrigger);
		m_TriggerArray.Remove(&wrapper);
		pTrigger->m_NumNodesContained = 0;
		pTrigger->m_pLayer = NULL;

		this->m_pTriggerSystem->RemoveColliders(pTrigger);
	}

	void C_TriggerLayer::AddTriggerActor(C_TriggerActor* pTriggerActor)
	{
		C_SortedTriggerActorWrapper wrapper(pTriggerActor);
		m_TriggerActorArray.Insert(&wrapper);
		//m_DirtyTriggerActorArray.AddToEnd(&pTriggerActor);
		pTriggerActor->m_pLayer = this;
	}

	void C_TriggerLayer::RemoveTriggerActor(C_TriggerActor* pTriggerActor)
	{
		//Remove from all the nodes!!!
		for(U32 a = 0; a < pTriggerActor->m_NumNodesContained; ++a)
		{
			pTriggerActor->m_Actors[a].m_pNode->Remove(&pTriggerActor->m_Actors[a]);
		}
		C_SortedTriggerActorWrapper wrapper(pTriggerActor);
		m_TriggerActorArray.Remove(&wrapper);
		m_pTriggerSystem->RemoveColliders(pTriggerActor);
		pTriggerActor->m_NumNodesContained = 0;
		pTriggerActor->m_pLayer = NULL;
	}

	void C_TriggerLayer::AddToActive(C_TriggerNode* pNode)
	{

		//Remove from existing list structure...
		{
			C_TriggerNode* pPrev = pNode->m_pPrev;
			C_TriggerNode* pNext = pNode->m_pNext;

			if(pPrev)
				pPrev->m_pNext = pNode->m_pNext;
			if(pNext)
				pNext->m_pPrev = pNode->m_pPrev;

			if(pNode == m_pSleepingNodes)
			{
				m_pSleepingNodes = pNode->m_pNext;
			}
		}

		//Add this node to the active node list...
		C_TriggerNode* pNextNode = SharedPhysics::Fast_Select<C_TriggerNode>(m_pActiveNodes == NULL, &m_DummyNode, m_pActiveNodes);
		pNextNode->m_pPrev = pNode;
		pNode->m_pNext = m_pActiveNodes;
		pNode->m_pPrev = NULL;
		m_pActiveNodes = pNode;
		pNode->m_State = E_TriggerNode_Active;
	}

	void C_TriggerLayer::AddToSleeping(C_TriggerNode* pNode)
	{
		{
			C_TriggerNode* pPrev = pNode->m_pPrev;
			C_TriggerNode* pNext = pNode->m_pNext;

			if(pPrev)
				pPrev->m_pNext = pNode->m_pNext;
			if(pNext)
				pNext->m_pPrev = pNode->m_pPrev;

			if(pNode == m_pActiveNodes)
			{
				m_pActiveNodes = pNode->m_pNext;
			}
		}
		C_TriggerNode dummy;
		C_TriggerNode* pNextNode = SharedPhysics::Fast_Select<C_TriggerNode>(m_pSleepingNodes == NULL, &m_DummyNode, m_pSleepingNodes);
		pNextNode->m_pPrev = pNode;
		pNode->m_pNext = m_pSleepingNodes;
		pNode->m_pPrev = NULL;
		m_pSleepingNodes = pNode;

		pNode->m_State = E_TriggerNode_Inactive;
	}

	void C_TriggerLayer::AddToActive(C_Trigger* pTrigger)
	{
		//Add this node to the active node list...
		if(m_pActiveTriggers)
		{
			m_pActiveTriggers->m_pPrev = pTrigger;
		}
		pTrigger->m_pNext = m_pActiveTriggers;
		pTrigger->m_pPrev = NULL;
		m_pActiveTriggers = pTrigger;
	}

	void C_TriggerLayer::AddToActive(C_TriggerActor* pTrigger)
	{
		//Add this node to the active node list...
		if(m_pActiveTriggerActors)
		{
			m_pActiveTriggerActors->m_pPrev = pTrigger;
		}
		pTrigger->m_pNext = m_pActiveTriggerActors;
		pTrigger->m_pPrev = NULL;
		m_pActiveTriggerActors = pTrigger;
	}

	void C_TriggerLayer::AddToDirty(C_TriggerActor* pTrigger)
	{
		m_DirtyTriggerActorArray.AddToEnd(&pTrigger);
	}

	void C_TriggerLayer::AddToDirty(C_Trigger* pTrigger)
	{
		m_DirtyTriggerArray.AddToEnd(&pTrigger);
	}

	void C_TriggerLayer::RemoveFromActive(C_Trigger* pTrigger)
	{
		//Remove this trigger to the active node list...
		if(pTrigger->m_pNext)
		{
			pTrigger->m_pNext->m_pPrev = pTrigger->m_pPrev;
		}
		if(pTrigger->m_pPrev)
		{
			pTrigger->m_pPrev->m_pNext = pTrigger->m_pNext;
		}
		if(pTrigger == m_pActiveTriggers)
		{
			m_pActiveTriggers = pTrigger->m_pNext;
		}
		pTrigger->m_pNext = pTrigger->m_pPrev = NULL;
	}

	void C_TriggerLayer::RemoveFromActive(C_TriggerActor* pTrigger)
	{
		//Remove this trigger to the active node list...
		if(pTrigger->m_pNext)
		{
			pTrigger->m_pNext->m_pPrev = pTrigger->m_pPrev;
		}
		if(pTrigger->m_pPrev)
		{
			pTrigger->m_pPrev->m_pNext = pTrigger->m_pNext;
		}
		if(pTrigger == m_pActiveTriggerActors)
		{
			m_pActiveTriggerActors = pTrigger->m_pNext;
		}
		pTrigger->m_pNext = pTrigger->m_pPrev = NULL;
	}

	bool C_Trigger::ContainedInNode(C_TriggerNode* pNode)
	{
		return m_TransformedBox.IsSmaller(pNode->m_bbox.m_min, pNode->m_bbox.m_max);
	}

	bool C_TriggerActor::ContainedInNode(C_TriggerNode* pNode)
	{
		return m_TransformedBox.IsSmaller(pNode->m_bbox.m_min, pNode->m_bbox.m_max);
	}


	void C_TriggerLayer::UpdateTrigger(C_Trigger* pTrigger, SharedPhysics::C_SafeStack<true>& stack)
	{
		using namespace Maths;
		//Go through and place it in the trigger layer...

		if(pTrigger->IsUpdateRequired())
		{
			for(U32 b = 0; b < pTrigger->m_NumNodesContained; ++b)
			{
				C_TriggerWrapper* pWrapper(&pTrigger->m_Actors[b]);
				pWrapper->m_pNode->Remove(pWrapper);
			}
			pTrigger->m_NumNodesContained = 0;
			HVec3 _min(Maths::Constants<HVec3>::Max());
			HVec3 _max(-Maths::Constants<HVec3>::Max());

			S32 size = 1;
			C_TriggerNode* pStackTop = m_pRootNode;
			stack.Push(m_pRootNode, 0);

			while(size--)
			{
				C_TriggerNode* pCurrentNode = pStackTop;
				pStackTop = stack.Pop<C_TriggerNode>(Maths::Max(size - 1, 0));
				
				if(pCurrentNode->m_bbox.Intersects(pTrigger->m_TransformedBox))
				{
					if(pCurrentNode->m_pLeftNode && pTrigger->ContainedInNode(pCurrentNode))
					{
						stack.Push(pCurrentNode->m_pRightNode, size++);
						stack.Push(pCurrentNode->m_pLeftNode, size++);
						pStackTop = pCurrentNode->m_pLeftNode;
						continue;
					}
					//If down here, we are at a leaf node, so need to push the object in there
					if(pTrigger->m_NumNodesContained >= MAX_TRIGGER_WRAPPERS)
					{
						continue;
					}

					C_TriggerWrapper* pWrapper = &pTrigger->m_Actors[pTrigger->m_NumNodesContained++];
					pWrapper->m_pTrigger = pTrigger;
					pWrapper->m_pNode = pCurrentNode;
					pCurrentNode->AddTrigger(pWrapper);
					_min = Min(pCurrentNode->m_bbox.m_min, _min);
					_max = Max(pCurrentNode->m_bbox.m_max, _max);
					if(!pCurrentNode->IsActive())
					{
						if(pCurrentNode->MustActivate())
						{
							AddToActive(pCurrentNode);
						}
						/*else if (!pCurrentNode->IsSleeping() && pCurrentNode->MustAddToSleeping())
						{
							AddToSleeping(pCurrentNode);
						}*/
					}
				}
			}
			pTrigger->m_NodeBox.m_max = _max;
			pTrigger->m_NodeBox.m_min = _min;
		}
		else
		{
			for(U32 b = 0; b < pTrigger->m_NumNodesContained; ++b)
			{
				C_TriggerWrapper* pWrapper(&pTrigger->m_Actors[b]);
				if(!pWrapper->m_pNode->IsActive())
				{
					if(pWrapper->m_pNode->MustActivate())
					{
						AddToActive(pWrapper->m_pNode);
					}
				}
			}
		}
	}

	void C_TriggerLayer::UpdateTriggerActor(C_TriggerActor* pTrigger, SharedPhysics::C_SafeStack<true>& stack)
	{

		using namespace Maths;
		//Go through and place it in the trigger layer...

		if(pTrigger->IsUpdateRequired())
		{
			for(U32 b = 0; b < pTrigger->m_NumNodesContained; ++b)
			{
				C_TriggerActorWrapper* pWrapper(&pTrigger->m_Actors[b]);
				pWrapper->m_pNode->Remove(pWrapper);
			}
			pTrigger->m_NumNodesContained = 0;
			HVec3 _min(Maths::Constants<HVec3>::Max());
			HVec3 _max(-Maths::Constants<HVec3>::Max());

			S32 size = 1;
			C_TriggerNode* pStackTop = m_pRootNode;
			stack.Push(m_pRootNode, 0);

			while(size--)
			{
				C_TriggerNode* pCurrentNode = pStackTop;
				pStackTop = stack.Pop<C_TriggerNode>(Maths::Max(size - 1, 0));
				
				if(pCurrentNode->m_bbox.Intersects(pTrigger->m_TransformedBox))
				{
					if(pCurrentNode->m_pLeftNode && pTrigger->ContainedInNode(pCurrentNode))
					{
						stack.Push(pCurrentNode->m_pRightNode, size++);
						stack.Push(pCurrentNode->m_pLeftNode, size++);
						pStackTop = pCurrentNode->m_pLeftNode;
						continue;
					}
					//If down here, we are at a leaf node, so need to push the object in there
					if(pTrigger->m_NumNodesContained >= MAX_TRIGGER_WRAPPERS)
					{
						continue;
					}

					C_TriggerActorWrapper* pWrapper = &pTrigger->m_Actors[pTrigger->m_NumNodesContained++];
					pWrapper->m_pActor = pTrigger;
					pWrapper->m_pNode = pCurrentNode;
					pCurrentNode->AddTriggerActor(pWrapper);
					_min = Min(pCurrentNode->m_bbox.m_min, _min);
					_max = Max(pCurrentNode->m_bbox.m_max, _max);
					if(!pCurrentNode->IsActive())
					{
						if(pCurrentNode->MustActivate())
						{
							AddToActive(pCurrentNode);
						}
						/*else if (!pCurrentNode->IsSleeping() && pCurrentNode->MustAddToSleeping())
						{
							AddToSleeping(pCurrentNode);
						}*/
					}
				}
			}
			pTrigger->m_NodeBox.m_max = _max;
			pTrigger->m_NodeBox.m_min = _min;
		}
		else
		{
			for(U32 b = 0; b < pTrigger->m_NumNodesContained; ++b)
			{
				C_TriggerActorWrapper* pWrapper(&pTrigger->m_Actors[b]);
				if(!pWrapper->m_pNode->IsActive())
				{
					if(pWrapper->m_pNode->MustActivate())
					{
						AddToActive(pWrapper->m_pNode);
					}
				}
			}
		}
	}


	void C_TriggerLayer::UpdateLayer(SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.RemapTriggerSystem");
		using namespace Maths;
		if(!m_pRootNode)
			return;
		if((!(g_dbTriggerDisableSplit && g_dbTriggerDisableMerge)))
		{
			m_CountDownToUpdate = m_UpdateFrequency;
			TriggerLeafState state;
			C_TriggerLeafNodeIterator iter(&m_LeafNodes);
			{
				
				C_TriggerNode* pNode = this->m_pActiveNodes;
				//while(iter.HasNext())
				while(pNode)
				{
					//C_DistributionNode* pNode(iter.Next());
					if(pNode->m_pLeftNode)
					{
						pNode = pNode->m_pNext;
						continue;
					}
					C_TriggerNode* pPrevNode(pNode->m_pPrev);
					//Check extents of box
					if(pNode->TriggerAndTriggerActorCount() > m_HighObjectThreshold )
					{
						m_pActiveNodes = pNode->Split(*m_pAllocator, iter, m_DesiredCellSize, m_pActiveNodes, *this);
					}
					else
					{
						C_TriggerNode* parent(pNode->m_pParentNode);
						if(parent && parent->m_pLeftNode->m_pLeftNode == NULL && parent->m_pRightNode->m_pLeftNode == NULL &&
							((parent->TriggerAndTriggerActorCount() + parent->m_pLeftNode->TriggerAndTriggerActorCount() + parent->m_pRightNode->TriggerAndTriggerActorCount()) < m_LowObjectThreshold /*|| !(parent->m_pLeftNode->IsActive() || parent->m_pRightNode->IsActive())*/))
						{
							state.m_pActiveRoot = m_pActiveNodes;
							//state.m_pSleepingRoot = m_pSleepingNodes;
							state.m_pInactiveRoot = m_pSleepingNodes;
							m_pActiveNodes = parent->Merge(*m_pAllocator, iter, state, m_MaxCellSize, *this);
							
							//m_pSleepingNodes = state.m_pSleepingRoot;
							m_pSleepingNodes = state.m_pInactiveRoot;
						}
					}
					pPrevNode = SharedPhysics::Fast_Select<C_TriggerNode>(pPrevNode == NULL, m_pActiveNodes, pPrevNode);
					if(pPrevNode == NULL)
						break;		

					pNode = SharedPhysics::Fast_Select<C_TriggerNode>(pPrevNode->m_pNext == pNode, pNode->m_pNext, pPrevNode->m_pNext);
				}
			}
		}

		//Now update the dirty objects...these will always be updated!!!
		for(S32 a = 0; a < m_DirtyTriggerArray.m_numElements; ++a)
		{
			C_Trigger* pTrigger = m_DirtyTriggerArray[a];
			UpdateTrigger(pTrigger, stack);
		}

		for(S32 a = 0; a < m_DirtyTriggerActorArray.m_numElements; ++a)
		{
			C_TriggerActor* pTrigger = m_DirtyTriggerActorArray[a];
			UpdateTriggerActor(pTrigger, stack);
		}
		m_DirtyTriggerArray.m_numElements = 0;
		m_DirtyTriggerActorArray.m_numElements = 0;
	}

	bool C_TriggerEvent::ValidTrigger()
	{
		return m_pTriggerActor->m_pShape != NULL && 
			m_pTrigger->m_pTriggerShape != NULL;
	}


	C_TriggerActor::~C_TriggerActor()
	{
		delete m_pShape;
		m_pShape = NULL;
		m_pUserData = NULL;
	}

	void C_Trigger::SetMatrix(const Maths::HMat43 PCREF matrix, SharedPhysics::C_SafeStack<true>& stack)
	{
		Transform(matrix);
		if(m_pLayer)
		{
			m_pLayer->SetTriggerMatrix(this, matrix, stack);
		}
	}

	void C_TriggerActor::SetMatrix(const Maths::HMat43 PCREF matrix, SharedPhysics::C_SafeStack<true>& stack)
	{
		Transform(matrix);
		if(m_pLayer)
		{
			m_pLayer->SetTriggerActorMatrix(this, matrix, stack);
		}
	}

	const Maths::HMat43 C_Trigger::GetMatrix()
	{
		return m_pTriggerShape->m_PosRot;
	}

	const Maths::HMat43 C_TriggerActor::GetMatrix()
	{
		return m_pShape->m_PosRot;
	}


	void C_TriggerLayer::SetTriggerMatrix(C_Trigger* pTrigger, const Maths::HMat43 PCREF matrix,
		SharedPhysics::C_SafeStack<true>& stack)
	{
		//Set the transform matrix on the trigger!!!
		m_NeedsUpdate = true;
		pTrigger->Activate();
		//pTrigger->Transform(matrix);
		UpdateTrigger(pTrigger, stack);
	}

	void C_TriggerLayer::SetTriggerActorMatrix(C_TriggerActor* pTriggerActor, const Maths::HMat43 PCREF matrix,
		SharedPhysics::C_SafeStack<true>& stack)
	{
		m_NeedsUpdate = true;
		pTriggerActor->Activate();
		//pTriggerActor->Transform(matrix);
		UpdateTriggerActor(pTriggerActor, stack);
	}

	U32 C_TriggerLayer::CompareTriggerToList(SharedPhysics::C_MemoryIterator<C_TriggerPair>& triggerIter, Gravity::C_TriggerActor* pActor, 
		Gravity::C_TriggerWrapper* pList, Collision::C_AxisBoxMinMax& box, const U32& numPairs)
	{
		U32 count = numPairs;
		C_TriggerWrapper* pWrapper=pList;
		while(pWrapper)
		{
			C_Trigger* pTrig=pWrapper->m_pTrigger;

			if(pActor->m_TransformedBox.IntersectsInAxisBox(pTrig->m_TransformedBox, box) & pActor->m_pShape->CanCollide(pTrig->m_pTriggerShape))
			{
				C_TriggerPair* pair(triggerIter.Create());
				++count;
				pair->m_pTriggerActor=pActor;
				pair->m_pTrigger=pTrig;
				pair->m_pTriggerLayer = this;
			}
			pWrapper=pWrapper->m_pNext;
		}
		return count;
	}

	U32 C_TriggerLayer::ActiveNodeComparison(C_TriggerNode* pActiveNode, SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs)
	{
		U32 numContacts = numPairs;
		if(pActiveNode->m_pActiveTriggers || pActiveNode->m_pInactiveTriggers)
		{
			C_TriggerActorWrapper* pWrapper = pActiveNode->m_pActiveTriggerActors;
			while(pWrapper)
			{
				C_TriggerActor* pActor = pWrapper->m_pActor;
				numContacts = CompareTriggerToList(iter, pActor, pActiveNode->m_pActiveTriggers, pActiveNode->m_bbox, numContacts);
				numContacts = CompareTriggerToList(iter, pActor, pActiveNode->m_pInactiveTriggers, pActiveNode->m_bbox, numContacts);
				pWrapper = pWrapper->m_pNext;
			}

			if(pActiveNode->m_pActiveTriggers)
			{
				pWrapper = pActiveNode->m_pInactiveTriggerActors;
				while(pWrapper)
				{
					C_TriggerActor* pActor = pWrapper->m_pActor;
					numContacts = CompareTriggerToList(iter, pActor, pActiveNode->m_pActiveTriggers, pActiveNode->m_bbox, numContacts);
					pWrapper = pWrapper->m_pNext;
				}
			}
		}
		return numContacts;
	}

	U32 C_TriggerLayer::ActiveParentComparison(C_TriggerNode* pActiveNode, SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs)
	{
		U32 numContacts = numPairs;
		C_TriggerNode* pParent = pActiveNode->m_pParentNode;
		while(pParent)
		{
			if(pParent->m_pActiveTriggers || pParent->m_pInactiveTriggers)
			{
				C_TriggerActorWrapper* pWrapper = pActiveNode->m_pActiveTriggerActors;
				while(pWrapper)
				{
					C_TriggerActor* pActor = pWrapper->m_pActor;
					numContacts = CompareTriggerToList(iter, pActor, pParent->m_pActiveTriggers, pActiveNode->m_bbox, numContacts);
					numContacts = CompareTriggerToList(iter, pActor, pParent->m_pInactiveTriggers, pActiveNode->m_bbox, numContacts);
					pWrapper = pWrapper->m_pNext;
				}
			}
			if(pActiveNode->m_pActiveTriggers)
			{
				/*C_TriggerActorWrapper* pWrapper = pParent->m_pActiveTriggerActors;
				while(pWrapper)
				{
					C_TriggerActor* pActor = pWrapper->m_pActor;
					numContacts = CompareTriggerToList(iter, pActor, pActiveNode->m_pActiveTriggers, pActiveNode->m_bbox, numContacts);
					numContacts = CompareTriggerToList(iter, pActor, pActiveNode->m_pInactiveTriggers, pActiveNode->m_bbox, numContacts);
					pWrapper = pWrapper->m_pNext;
				}*/

				C_TriggerActorWrapper* pWrapper = pParent->m_pInactiveTriggerActors;
				while(pWrapper)
				{
					C_TriggerActor* pActor = pWrapper->m_pActor;
					numContacts = CompareTriggerToList(iter, pActor, pActiveNode->m_pActiveTriggers, pActiveNode->m_bbox, numContacts);
					pWrapper = pWrapper->m_pNext;
				}
			}
			pParent = pParent->m_pParentNode;				
		}
		
		return numContacts;
		
	}

	U32 C_TriggerLayer::ActiveChildComparison(C_TriggerNode* pActiveNode, SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs,
		SharedPhysics::C_SafeStack<true>& stack)
	{
		U32 numContacts = numPairs;
		if(pActiveNode->m_pLeftNode)
		{
			C_TriggerNode* pStackTop(pActiveNode->m_pLeftNode);
			S32 size = 2;
			stack.Push(pActiveNode->m_pRightNode, 0);
			stack.Push(pActiveNode->m_pLeftNode, 1);

			while(size--)
			{
				C_TriggerNode* pChild = pStackTop;
				pStackTop = stack.Pop<C_TriggerNode>(Maths::Max(size - 1, 0));

				if(pChild->m_pLeftNode)
				{
					stack.Push(pChild->m_pRightNode, size++);
					stack.Push(pChild->m_pLeftNode, size++);
					pStackTop = pChild->m_pLeftNode;
				}

				//if(pChild->IsSleeping())
				{
					if(pChild->m_pInactiveTriggers || pChild->m_pActiveTriggers)
					{
						C_TriggerActorWrapper* pWrapper = pActiveNode->m_pActiveTriggerActors;
						while(pWrapper)
						{
							C_TriggerActor* pActor = pWrapper->m_pActor;
							if(pChild->m_bbox.Intersects(pActor->m_TransformedBox))
							{
								numContacts = CompareTriggerToList(iter, pActor, pChild->m_pInactiveTriggers, pChild->m_bbox, numContacts);
								numContacts = CompareTriggerToList(iter, pActor, pChild->m_pActiveTriggers, pChild->m_bbox, numContacts);
							}
							pWrapper = pWrapper->m_pNext;
						}
					}
					if(pActiveNode->m_pActiveTriggers)
					{
						C_TriggerActorWrapper* pWrapper = pChild->m_pInactiveTriggerActors;
						while(pWrapper)
						{
							C_TriggerActor* pActor = pWrapper->m_pActor;
							if(pChild->m_bbox.Intersects(pActor->m_TransformedBox))
							{
								numContacts = CompareTriggerToList(iter, pActor, pActiveNode->m_pActiveTriggers, pChild->m_bbox, numContacts);
							}
							pWrapper = pWrapper->m_pNext;
						}
					}
				}
			}
		}
		return numContacts;
	}

	void C_TriggerNode::DeactivateObjects()
	{
		{
			C_TriggerWrapper* pWrapper = m_pActiveTriggers;
			C_TriggerWrapper* pLastWrapper = NULL;
			while(pWrapper)
			{
				pLastWrapper = pWrapper;
				pWrapper->m_pTrigger->m_State = E_InactiveTrigger_State;
				pWrapper = pWrapper->m_pNext;
			}
			if(pLastWrapper)
			{
				pLastWrapper->m_pNext = m_pInactiveTriggers;
				if(m_pInactiveTriggers)
				{
					m_pInactiveTriggers->m_pPrev = pLastWrapper;
				}
				m_pInactiveTriggers = m_pActiveTriggers;
			}
			m_pActiveTriggers = NULL;
		}
		{
			C_TriggerActorWrapper* pWrapper = m_pActiveTriggerActors;
			C_TriggerActorWrapper* pLastWrapper = NULL;
			while(pWrapper)
			{
				pLastWrapper = pWrapper;
				pWrapper->m_pActor->m_State = E_InactiveTrigger_State;
				pWrapper = pWrapper->m_pNext;
			}
			if(pLastWrapper)
			{
				pLastWrapper->m_pNext = m_pInactiveTriggerActors;
				if(m_pInactiveTriggerActors)
				{
					m_pInactiveTriggerActors->m_pPrev = pLastWrapper;
				}
				m_pInactiveTriggerActors = m_pActiveTriggerActors;
			}
			m_pActiveTriggerActors = NULL;
		}
	}



	U32 C_TriggerLayer::GenerateIntersectionPairs(SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs,
		SharedPhysics::C_SafeStack<true>& stack)
	{
		//Loop through all the active nodes and compare...
		//Loop through all the sleeping nodes and compare with any parents with triggers...
		if(!m_NeedsUpdate)
		{
			m_Updated = false;
			return 0;
		}
		m_NeedsUpdate = false;
		m_Updated = true;

		PERF_EVENT("Gravity.TriggerPairs");
		U32 numContacts = numPairs;
		C_TriggerNode* pActiveNode = m_pActiveNodes;
		m_ActiveNodeCount = 0;
		while(pActiveNode)
		{
			C_TriggerNode* pNextNode = pActiveNode->m_pNext;
			numContacts = ActiveNodeComparison(pActiveNode, iter, numContacts);
			//Go up through parent...
			numContacts = ActiveParentComparison(pActiveNode, iter, numContacts);
			numContacts = ActiveChildComparison(pActiveNode, iter, numContacts, stack);
			++m_ActiveNodeCount;
			pActiveNode = pNextNode;
		}
		return numContacts;
	}

	C_TriggerSystem::C_TriggerSystem() : m_pLayers(NULL), m_pCallback(NULL)
	{
		m_pPairAllocator = new SharedPhysics::MemoryAllocator<C_TriggerPair>;
		m_pTriggerEvents = new SharedPhysics::MemoryAllocator<C_TriggerEvent>;
	}

	C_TriggerSystem::~C_TriggerSystem()
	{
		DestroyLayers();
		delete m_pPairAllocator;
		delete m_pTriggerEvents;
	}

	void C_TriggerSystem::DestroyLayers()
	{
		C_TriggerLayer* pLayer = m_pLayers;
		while(pLayer)
		{
			C_TriggerLayer* pNextLayer = pLayer->m_pNext;
			delete pLayer;
			pLayer = pNextLayer;
		}
		m_pLayers = NULL;
	}

	C_TriggerLayer* C_TriggerSystem::CreateLayer(const Maths::HVec3 PCREF minBox, const Maths::HVec3 PCREF maxBox, 
			const Maths::HVec3 PCREF desiredCellSize, U32 maxDepth, U32 index)
	{
		C_TriggerLayer* pNewLayer = new C_TriggerLayer(minBox, maxBox, desiredCellSize, maxDepth, index, this);
		pNewLayer->m_pNext = m_pLayers;
		if(m_pLayers)
		{
			m_pLayers->m_pPrev = pNewLayer;
		}
		pNewLayer->m_pPrev = NULL;
		m_pLayers = pNewLayer;
		return pNewLayer;
	}

	C_TriggerLayer* C_TriggerSystem::GetLayer(U32 index)
	{
		C_TriggerLayer* pLayer = m_pLayers;
		while(pLayer)
		{
			if(pLayer->GetLayerIndex() == index)
				return pLayer;
			pLayer = pLayer->m_pNext;
		}
		return NULL;
	}

	void C_TriggerSystem::DestroyLayer(C_TriggerLayer* pLayer)
	{
		if(pLayer->m_pNext)
			pLayer->m_pNext->m_pPrev = pLayer->m_pPrev;
		if(pLayer->m_pPrev)
			pLayer->m_pPrev->m_pNext = pLayer->m_pNext;
		if(pLayer == m_pLayers)
			m_pLayers = pLayer->m_pNext;
		delete pLayer;
	}

	void C_TriggerSystem::UpdateLayers(SharedPhysics::C_SafeStack<true>& stack)
	{
		C_TriggerLayer* pLayer = m_pLayers;
		while(pLayer)
		{
			if(pLayer->m_NeedsUpdate)
			{
				pLayer->UpdateLayer(stack);
			}
			pLayer = pLayer->m_pNext;	
		}
	}

	bool C_TriggerSystem::AddCollider(C_TriggerCollider& collider)
	{
		const S32 numPairs = m_CollidingPairs.m_numElements;
		m_CollidingPairs.m_numElements = m_NumExistingColliders;
		C_TriggerCollider* existingCollider = m_CollidingPairs.Find(&collider);
		m_CollidingPairs.m_numElements = numPairs;
		if(existingCollider)
		{
			existingCollider->MarkFound();
			return false;
		}
		m_CollidingPairs.AddToEnd(&collider);
		return true;
	}

	void C_TriggerSystem::EmitEvents(SharedPhysics::C_S32SafeStack& stack)
	{
		PERF_EVENT("TriggerSystem.EmitEvents");
		SharedPhysics::C_MemoryIterator<C_TriggerEvent> iter(m_pTriggerEvents);
		U32 numEvents = 0;

		S32 startIndex = this->m_NumExistingColliders;
		for(S32 a = startIndex; a < m_CollidingPairs.m_numElements; ++a)
		{
			//New pair...
			C_TriggerCollider& collider = m_CollidingPairs[a];
			//Clear its up-to-date flag - we use this to find out if we've exited the trigger
			collider.m_Exist = false;
			C_TriggerEvent* pEvent = iter.Create();
			pEvent->InitialiseEvent(C_TriggerEvent::E_Enter_Event, collider.m_pTrigger, collider.m_pTriggerActor,
				collider.m_pLayer);
			++numEvents;
		}

		//OK. Now go through the existing ones and emit all exit events...
		S32 endIndex = startIndex;
		for(S32 a = 0; a < endIndex; ++a)
		{
			C_TriggerCollider& collider = m_CollidingPairs[a];
			if(collider.HasExited())
			{
				if(!collider.m_Removed)
				{
					C_TriggerEvent* pEvent = iter.Create();
					pEvent->InitialiseEvent(C_TriggerEvent::E_Exit_Event, collider.m_pTrigger, collider.m_pTriggerActor,
						collider.m_pLayer);

					++numEvents;
				}
				
				m_CollidingPairs[a] = m_CollidingPairs[m_CollidingPairs.m_numElements - 1];
				
				if(m_CollidingPairs.m_numElements <= startIndex)
				{
					--a;
				}
				m_CollidingPairs.m_numElements--;
				endIndex--;	
			}
		}
		m_CollidingPairs.QuickSort(stack, 5);

		{
			PERF_EVENT("TriggerSystem.ProcessTriggerEvents");
			SharedPhysics::C_MemoryIterator<C_TriggerEvent> resultIter(m_pTriggerEvents);
			m_pCallback->ProcessTriggerEvents(resultIter, numEvents);
		}

	}

	bool C_TriggerSystem::CollideShape(C_Trigger* pTrigger, C_TriggerActor* pTriggerActor)
	{
		switch(pTriggerActor->m_pShape->m_TriggerType)
		{
		case C_TriggerShape::E_TriggerBox:
			{
				C_TriggerBox* pBox = (C_TriggerBox*)pTriggerActor->m_pShape;
				return pBox->isInsideTriggerShape(pTrigger->m_pTriggerShape);
			}

		case C_TriggerShape::E_TriggerSphere:
			{
				C_TriggerSphere* pSphere = (C_TriggerSphere*)pTriggerActor->m_pShape;
				return pSphere->isInsideTriggerShape(pTrigger->m_pTriggerShape);
			}
		}
		return false;
	}

	void C_TriggerSystem::RemoveColliders(C_Trigger* pTrigger)
	{
		for(S32 a = 0; a < m_CollidingPairs.m_numElements; ++a)
		{
			C_TriggerCollider& collider = m_CollidingPairs[a];
			if(collider.m_pTrigger == pTrigger)
				collider.m_Removed = true;
		}
	}

	void C_TriggerSystem::RemoveColliders(C_TriggerActor* pTriggerActor)
	{
		for(S32 a = 0; a < m_CollidingPairs.m_numElements; ++a)
		{
			C_TriggerCollider& collider = m_CollidingPairs[a];
			if(collider.m_pTriggerActor == pTriggerActor)
				collider.m_Removed = true;
		}
	}

	void C_TriggerSystem::GetTriggersActorIsIn(C_TriggerActor* pTriggerActor, Btl::Vector<C_Trigger*>& triggerList)
	{
		triggerList.Clear();
		for(S32 a = 0; a < m_CollidingPairs.m_numElements; ++a)
		{
			if(m_CollidingPairs[a].m_pTriggerActor == pTriggerActor)
			{
				triggerList.PushBack(m_CollidingPairs[a].m_pTrigger);
			}
		}
	}

	C_TriggerCollider* C_TriggerSystem::GetActorsInTrigger(C_Trigger* pTrigger, U32& numActorsInTrigger)
	{
		
		C_TriggerCollider collider;
		collider.m_TriggerIndex = pTrigger->m_TriggerIndex;
		collider.m_TriggerActorIndex = 0;


		S32 startPos = -1;
		S32 findPos = m_CollidingPairs.FindIndex(&collider, startPos);
		if(findPos >= 0)
		{
			startPos = findPos;
		}
		S32 endIndex = startPos;
		for(endIndex = startPos; endIndex < m_CollidingPairs.m_numElements; ++endIndex)
		{
			if(m_CollidingPairs[endIndex].m_pTrigger != pTrigger)
			{
				break;
			}
		}
		if(endIndex > startPos)
		{
			numActorsInTrigger = endIndex - startPos;
			return &m_CollidingPairs[startPos];
		}
		numActorsInTrigger = 0;
		return NULL;
	}

	void C_TriggerLayer::RemoveActiveObjects()
	{
		C_TriggerNode* pActiveNode = m_pActiveNodes;
		while(pActiveNode)
		{
			C_TriggerNode* pNextNode = pActiveNode->m_pNext;

			pActiveNode->DeactivateObjects();
			pActiveNode->m_pNext = pActiveNode->m_pPrev = NULL;
			pActiveNode->m_State = Gravity::E_TriggerNode_Inactive;
			pActiveNode = pNextNode;
		}
		m_pActiveNodes = NULL;
	}


	void C_TriggerSystem::GenerateEvents(SharedPhysics::C_S32SafeStack& stack)
	{
		PERF_EVENT("TriggerSystem.GenerateEvents");
		//No point in creating events if no one wants to hear them...
		if(m_pCallback == NULL)
			return; 

		SharedPhysics::C_SafeStack<true> genericStack(stack.m_pData, stack.m_Size);

		this->UpdateLayers(genericStack);

		m_NumExistingColliders = m_CollidingPairs.m_numElements;
		SharedPhysics::C_MemoryIterator<C_TriggerPair> iter(m_pPairAllocator);
		C_TriggerLayer* pLayer = m_pLayers;
		U32 numPairs = 0;
		while(pLayer)
		{
			numPairs = pLayer->GenerateIntersectionPairs(iter, numPairs, genericStack);
			pLayer = pLayer->m_pNext;
		}
		
		//Go through and do narrow phase...
		{
			PERF_EVENT("TriggerSystem.NarrowPhase");
			SharedPhysics::C_MemoryIterator<C_TriggerPair> resultIter(m_pPairAllocator);

			for(U32 a = 0; a < numPairs; ++a)
			{
				C_TriggerPair* pPair = resultIter.Get();
				//Do actual collision detection, update pairs, send out messages for new colliders
				if(CollideShape(pPair->m_pTrigger, pPair->m_pTriggerActor))
				{
					C_TriggerCollider collider(pPair->m_pTrigger, pPair->m_pTriggerActor, pPair->m_pTriggerLayer);
					AddCollider(collider);
				}
				
			}
		}

		EmitEvents(stack);
		
		{
			PERF_EVENT("RemoveActiveObjects");
			pLayer = m_pLayers;
			while(pLayer)
			{
				pLayer->RemoveActiveObjects();
				pLayer = pLayer->m_pNext;
			}
		}
	}

	U32 C_TriggerSystem::GetTriggerCount()
	{
		U32 count = 0;
		C_TriggerLayer* pLayer = m_pLayers;
		while(pLayer)
		{
			count += pLayer->m_TriggerArray.m_numElements;
			pLayer = pLayer->m_pNext;
		}
		
		return count;
	}

	U32 C_TriggerSystem::GetTriggerActorCount()
	{
		U32 count = 0;
		C_TriggerLayer* pLayer = m_pLayers;
		while(pLayer)
		{
			count += pLayer->m_TriggerActorArray.m_numElements;
			pLayer = pLayer->m_pNext;
		}
		
		return count;
	}

	U32 C_TriggerSystem::GetActiveNodeCount()
	{
		U32 count = 0;
		C_TriggerLayer* pLayer = m_pLayers;
		while(pLayer)
		{
			count += pLayer->m_ActiveNodeCount;
			pLayer = pLayer->m_pNext;
		}
		
		return count;
	}

}
