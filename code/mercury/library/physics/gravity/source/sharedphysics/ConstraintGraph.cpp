#include "StdAfx.h"
#include <Core/Maths/Hvec/New/IncludePhysics.h>
#include <Core/Maths/Hvec/New/IncludePhysics.inl>
#include "gravity/sharedphysics/ConstraintGraph.h"
#include "gravity/sdk/Body.h"
#include <system/perf.h>

namespace SharedPhysics
{

//Maths::HF32 C_Constraint::GetBodyAMassScale()
//{
//	return m_MassScales.X();
//}
//
//Maths::HF32 C_Constraint::GetBodyBMassScale()
//{
//	return m_MassScales.Y();
//}
//
//Maths::HVec3 C_Constraint::GetBodyAInertiaScale()
//{
//	return m_BodyAInertiaScale;
//}
//
//Maths::HVec3 C_Constraint::GetBodyBInertiaScale()
//{
//	return m_BodyBInertiaScale;
//}

bool C_GraphNode::CanDeactivateConstraints() const
{
	//Loop over the graph node's constraints and decide whether any of them are above the sleep threshold!
	C_ConstraintEdge* pEdge = m_pPersistentConstraints;
	while(pEdge)
	{
		if(pEdge->m_pOutboundNode->m_Deactivator.m_CountDown > 0 || pEdge->m_pOutboundNode->m_Deactivator.ActiveAndAboveSleepThresh())
		{
			return false;
		}
		pEdge = pEdge->m_pNext;
	}
	C_ConstraintTwin* pTwin = m_pPersistentTwinConstraints;
	while(pTwin)
	{
		C_ConstraintEdge* fetchedEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
		if(fetchedEdge->m_pOutboundNode->m_Deactivator.m_CountDown > 0 || fetchedEdge->m_pInboundNode->m_Deactivator.ActiveAndAboveSleepThresh())
		{
			return false;
		}
		pTwin = pTwin->m_pNext;
	}
	return true;	
}

bool C_GraphNode::ContainsEdge(C_ConstraintEdge* pEdge)
{
	C_ConstraintEdge* pE = m_pPersistentConstraints;
	while(pE)
	{
		if(pE == pEdge)
			return true;
		pE = pE->m_pNext;
	}
	return false;
}

bool C_Constraint::FlipConstraint(SharedPhysics::C_GraphNode* pFetchedNodeA, SharedPhysics::C_GraphNode* pFetchedNodeB,
		U32& nodeBFlags)
{
	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const bool bMassRatioStaticA(GetBodyAMassScale().EQ(zero));
	const bool bMassRatioStaticB(GetBodyBMassScale().EQ(zero));
	bool bBodyAStatic = pFetchedNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia);
	bool bBodyBStatic = pFetchedNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia);
	bool bFlip = (bBodyAStatic | bMassRatioStaticA) & (!(bBodyBStatic | bMassRatioStaticB));

	/*if(bFlip)
	{
		if(bMassRatioStaticA)
		{
			nodeBFlags = SharedPhysics::C_GraphNode::E_Type_KinematicObject;
		}
		else
		{
			nodeBFlags = pFetchedNodeA->m_NodeFlags;
		}
	}
	else if (bMassRatioStaticB)
	{
		nodeBFlags = SharedPhysics::C_GraphNode::E_Type_KinematicObject;
	}
	else
	{
		nodeBFlags = pFetchedNodeB->m_NodeFlags;
	}*/

	bool bKinematic = ((bFlip & bMassRatioStaticA) | ((!bFlip) & bMassRatioStaticB)) != 0;

	nodeBFlags = Maths::Select(bKinematic, ((U32)SharedPhysics::C_GraphNode::E_Type_KinematicObject),
		Maths::Select(bFlip, pFetchedNodeA->m_NodeFlags, pFetchedNodeB->m_NodeFlags));
	return bFlip;
}

#ifndef PLATFORM_SPU

U32 C_Constraint::m_NumConstraints = 0;

//S32 C_Deactivator::m_sSleepCountdown = 25;

C_Constraint::C_Constraint(const E_ConstraintType type) : m_ConstraintType(type),
	m_ConstraintIndex(m_NumConstraints++), pNodeA(NULL), pNodeB(NULL), pRoot(NULL), m_Scale(1.f),
	m_MassScales(Maths::Constants<Maths::HF32>::One()),
	m_Baumgarte(0.8f), m_pUserData(NULL)
{
	m_pPPUConstraint = this;
	ZeroWarmStartValues();
}


//void C_Constraint::SetBodyAMassScale(const Maths::HF32 PCREF scale)
//{
//	m_MassScales.X(scale);
//	//m_MassScales.X(scale);
//}
//
//void C_Constraint::SetBodyBMassScale(const Maths::HF32 PCREF scale)
//{
//	m_MassScales.Y(scale);
//}
//
//void C_Constraint::SetBodyAInertiaScale(const Maths::HF32 PCREF scale)
//{
//	m_BodyAInertiaScale = scale;
//	//m_MassScales.Z(scale);
//}
//
//void C_Constraint::SetBodyBInertiaScale(const Maths::HF32 PCREF scale)
//{
//	m_BodyBInertiaScale = scale;
//}

//Links the two graph nodes together. Requires 2 constraints because 
void LinkPersistent(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB)
{
	BIZ_ASSERT(pNodeA != pNodeB);
	C_ConstraintEdge dummy1;
	C_ConstraintTwin dummy2;
	pConstraintA->m_pNext = pNodeA->m_pPersistentConstraints;
	C_ConstraintEdge * constraint = Fast_Select<C_ConstraintEdge>(pNodeA->m_pPersistentConstraints == NULL, &dummy1, pNodeA->m_pPersistentConstraints);
	constraint->m_pPrev = pConstraintA;
	pConstraintA->m_pPrev = NULL;
	pNodeA->m_pPersistentConstraints = pConstraintA;

	pConstraintB->m_pNext = pNodeB->m_pPersistentTwinConstraints;
	C_ConstraintTwin* constraint2 = Fast_Select<C_ConstraintTwin>(pNodeB->m_pPersistentTwinConstraints == NULL, &dummy2, pNodeB->m_pPersistentTwinConstraints);
	constraint2->m_pPrev = pConstraintB;
	pConstraintB->m_pPrev = NULL;
	pNodeB->m_pPersistentTwinConstraints = pConstraintB;

	pConstraintA->m_pInboundNode = pNodeA;
	pConstraintA->m_pOutboundNode = pNodeB;
	//pConstraintA->m_pTwinConstraint = pConstraintB;
	//pConstraintA->m_pTwinConstraint = pConstraintB;
	//pConstraintB->m_pEdge = pConstraintA;
}

//Links the two graph nodes together. Requires 2 constraints because 
void LinkPersistent(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA)
{
	BIZ_ASSERT(pNodeA != pNodeB);
	C_ConstraintEdge dummy;
	pConstraintA->m_pNext = pNodeA->m_pPersistentConstraints;
	C_ConstraintEdge * constraint = Fast_Select<C_ConstraintEdge>(pNodeA->m_pPersistentConstraints == NULL, &dummy, pNodeA->m_pPersistentConstraints);
	constraint->m_pPrev = pConstraintA;
	pConstraintA->m_pPrev = NULL;
	pNodeA->m_pPersistentConstraints = pConstraintA;

	pConstraintA->m_pInboundNode = pNodeA;
	pConstraintA->m_pOutboundNode = pNodeB;
	//pConstraintA->m_pTwinConstraint = NULL;
}

	void LinkVolatile(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB)
	{
		BIZ_ASSERT(pNodeA != pNodeB);
		/*C_ConstraintEdge dummy;
		C_ConstraintTwin dummy2;
		pConstraintA->m_pNext = pNodeA->m_pVolatileConstraints;
		C_ConstraintEdge * constraint = Fast_Select<C_ConstraintEdge>(pNodeA->m_pVolatileConstraints == NULL, &dummy, pNodeA->m_pVolatileConstraints);
		constraint->m_pPrev = pConstraintA;
		pConstraintA->m_pPrev = NULL;
		pNodeA->m_pVolatileConstraints = pConstraintA;

		pConstraintB->m_pNext = pNodeB->m_pVolatileTwinConstraints;
		C_ConstraintTwin* constraint2 = Fast_Select<C_ConstraintTwin>(pNodeB->m_pVolatileTwinConstraints == NULL, &dummy2, pNodeB->m_pVolatileTwinConstraints);
		constraint2->m_pPrev = pConstraintB;
		pConstraintB->m_pPrev = NULL;
		pNodeB->m_pVolatileTwinConstraints = pConstraintB;*/

		pConstraintA->m_pNext = pNodeA->m_pVolatileConstraints;
		pConstraintA->m_pPrev = NULL;
		if(pNodeA->m_pVolatileConstraints)
			pNodeA->m_pVolatileConstraints->m_pPrev = pConstraintA;
		pNodeA->m_pVolatileConstraints = pConstraintA;

		pConstraintB->m_pNext = pNodeB->m_pVolatileTwinConstraints;
		pConstraintB->m_pPrev = NULL;
		if(pNodeB->m_pVolatileTwinConstraints)
			pNodeB->m_pVolatileTwinConstraints->m_pPrev = pConstraintB;
		pNodeB->m_pVolatileTwinConstraints = pConstraintB;

		pConstraintA->m_pInboundNode = pNodeA;
		pConstraintA->m_pOutboundNode = pNodeB;
		//pConstraintA->m_pTwinConstraint = pConstraintB;
		//pConstraintB->m_pEdge = pConstraintA;
	}

	void LinkClose(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB)
	{
		BIZ_ASSERT(pNodeA != pNodeB);

		pConstraintA->m_pNext = pNodeA->m_pCloseConstraints;
		pConstraintA->m_pPrev = NULL;
		if(pNodeA->m_pCloseConstraints)
			pNodeA->m_pCloseConstraints->m_pPrev = pConstraintA;
		pNodeA->m_pCloseConstraints = pConstraintA;

		pConstraintB->m_pNext = pNodeB->m_pCloseTwinConstraints;
		pConstraintB->m_pPrev = NULL;
		if(pNodeB->m_pCloseTwinConstraints)
			pNodeB->m_pCloseTwinConstraints->m_pPrev = pConstraintB;
		pNodeB->m_pCloseTwinConstraints = pConstraintB;

		pConstraintA->m_pInboundNode = pNodeA;
		pConstraintA->m_pOutboundNode = pNodeB;
		//pConstraintA->m_pTwinConstraint = pConstraintB;
		//pConstraintB->m_pEdge = pConstraintA;
	}

	void AddTwinConstraint(C_GraphNode* pNodeB, C_ConstraintTwin* pConstraintB)
	{
		C_ConstraintTwin dummy2;
		pConstraintB->m_pNext = pNodeB->m_pVolatileTwinConstraints;
		C_ConstraintTwin* constraint2 = Fast_Select<C_ConstraintTwin>(pNodeB->m_pVolatileTwinConstraints == NULL, &dummy2, pNodeB->m_pVolatileTwinConstraints);
		constraint2->m_pPrev = pConstraintB;
		pConstraintB->m_pPrev = NULL;
		pNodeB->m_pVolatileTwinConstraints = pConstraintB;
	}

	void RemoveTwinConstraint(C_GraphNode* pNodeB, C_ConstraintTwin* pConstraintB)
	{
		C_ConstraintTwin dummy2;
		C_ConstraintTwin* constraint2 = Fast_Select<C_ConstraintTwin>(pConstraintB->m_pPrev == NULL, &dummy2, pConstraintB->m_pPrev);
		constraint2->m_pNext = pConstraintB->m_pNext;
		constraint2 = Fast_Select<C_ConstraintTwin>(pConstraintB->m_pNext == NULL, &dummy2, pConstraintB->m_pNext);
		constraint2->m_pPrev = pConstraintB->m_pPrev;

		pNodeB->m_pPersistentTwinConstraints = Fast_Select<C_ConstraintTwin>(pNodeB->m_pPersistentTwinConstraints == pConstraintB, pConstraintB->m_pNext, pNodeB->m_pPersistentTwinConstraints);
		pNodeB->m_pVolatileTwinConstraints = Fast_Select<C_ConstraintTwin>(pNodeB->m_pVolatileTwinConstraints == pConstraintB, pConstraintB->m_pNext, pNodeB->m_pVolatileTwinConstraints);
	}

	void LinkVolatile(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA)
	{
		BIZ_ASSERT(pNodeA != pNodeB);
		C_ConstraintEdge dummy;
		pConstraintA->m_pNext = pNodeA->m_pVolatileConstraints;
		C_ConstraintEdge * constraint = Fast_Select<C_ConstraintEdge>(pNodeA->m_pVolatileConstraints == NULL, &dummy, pNodeA->m_pVolatileConstraints);
		constraint->m_pPrev = pConstraintA;
		pConstraintA->m_pPrev = NULL;
		pNodeA->m_pVolatileConstraints = pConstraintA;

		pConstraintA->m_pInboundNode = pNodeA;
		pConstraintA->m_pOutboundNode = pNodeB;
		//pConstraintA->m_pTwinConstraint = NULL;
	}

	void Unlink(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB)
	{
		/*C_ConstraintEdge dummy;
		C_ConstraintTwin dummy2;
		C_ConstraintEdge* constraint = Fast_Select<C_ConstraintEdge>(pConstraintA->m_pPrev == NULL, &dummy, pConstraintA->m_pPrev);
		constraint->m_pNext = pConstraintA->m_pNext;
		constraint = Fast_Select<C_ConstraintEdge>(pConstraintA->m_pNext == NULL, &dummy, pConstraintA->m_pNext);
		constraint->m_pPrev = pConstraintA->m_pPrev;

		C_ConstraintTwin* constraint2 = Fast_Select<C_ConstraintTwin>(pConstraintB->m_pPrev == NULL, &dummy2, pConstraintB->m_pPrev);
		constraint2->m_pNext = pConstraintB->m_pNext;
		constraint2 = Fast_Select<C_ConstraintTwin>(pConstraintB->m_pNext == NULL, &dummy2, pConstraintB->m_pNext);
		constraint2->m_pPrev = pConstraintB->m_pPrev;

		pNodeA->m_pPersistentConstraints = Fast_Select<C_ConstraintEdge>(pNodeA->m_pPersistentConstraints == pConstraintA, pConstraintA->m_pNext, pNodeA->m_pPersistentConstraints);
		pNodeA->m_pVolatileConstraints = Fast_Select<C_ConstraintEdge>(pNodeA->m_pVolatileConstraints == pConstraintA, pConstraintA->m_pNext, pNodeA->m_pVolatileConstraints);
		pNodeB->m_pPersistentTwinConstraints = Fast_Select<C_ConstraintTwin>(pNodeB->m_pPersistentTwinConstraints == pConstraintB, pConstraintB->m_pNext, pNodeB->m_pPersistentTwinConstraints);
		pNodeB->m_pVolatileTwinConstraints = Fast_Select<C_ConstraintTwin>(pNodeB->m_pVolatileTwinConstraints == pConstraintB, pConstraintB->m_pNext, pNodeB->m_pVolatileTwinConstraints);*/

		if(pConstraintA->m_pNext)
		{
			pConstraintA->m_pNext->m_pPrev = pConstraintA->m_pPrev;
		}
		if(pConstraintA->m_pPrev)
		{
			pConstraintA->m_pPrev->m_pNext = pConstraintA->m_pNext;
		}

		if(pConstraintB->m_pNext)
		{
			pConstraintB->m_pNext->m_pPrev = pConstraintB->m_pPrev;
		}
		if(pConstraintB->m_pPrev)
		{
			pConstraintB->m_pPrev->m_pNext = pConstraintB->m_pNext;
		}

		if(pNodeA->m_pPersistentConstraints == pConstraintA)
		{
			pNodeA->m_pPersistentConstraints = pConstraintA->m_pNext;
		}

		if(pNodeA->m_pVolatileConstraints == pConstraintA)
		{
			pNodeA->m_pVolatileConstraints = pConstraintA->m_pNext;
		}

		if(pNodeB->m_pPersistentTwinConstraints == pConstraintB)
		{
			pNodeB->m_pPersistentTwinConstraints = pConstraintB->m_pNext;
		}

		if(pNodeB->m_pVolatileTwinConstraints == pConstraintB)
		{
			pNodeB->m_pVolatileTwinConstraints = pConstraintB->m_pNext;
		}

		//pConstraintA->m_pTwinConstraint = NULL;
		//pConstraintB->m_pEdge = NULL;
	}

	void Unlink(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA)
	{
		C_ConstraintEdge dummy;
		C_ConstraintEdge* constraint = Fast_Select<C_ConstraintEdge>(pConstraintA->m_pPrev == NULL, &dummy, pConstraintA->m_pPrev);
		constraint->m_pNext = pConstraintA->m_pNext;
		constraint = Fast_Select<C_ConstraintEdge>(pConstraintA->m_pNext == NULL, &dummy, pConstraintA->m_pNext);
		constraint->m_pPrev = pConstraintA->m_pPrev;

		pNodeA->m_pPersistentConstraints = Fast_Select<C_ConstraintEdge>(pNodeA->m_pPersistentConstraints == pConstraintA, pConstraintA->m_pNext, pNodeA->m_pPersistentConstraints);
		pNodeA->m_pVolatileConstraints = Fast_Select<C_ConstraintEdge>(pNodeA->m_pVolatileConstraints == pConstraintA, pConstraintA->m_pNext, pNodeA->m_pVolatileConstraints);
	}

	bool C_Deactivator::CanDeactivate()
	{
		const Maths::HF32 sleepThreshSq(m_SleepThresholdSq);
		const Maths::HVec3 totalKinEnergy(m_pGraphNode->m_TotalKineticEnergy);
		const Maths::HF32 ke(totalKinEnergy | totalKinEnergy);
		bool result = ((m_pGraphNode->TestNodeFlags(C_GraphNode::E_Type_Active)) & ((ke < sleepThreshSq) != 0));
		m_CountDown = Maths::Select(result, m_CountDown - 1, C_Deactivator::m_sSleepCountdown);
		return m_CountDown <= 0;
	}

	bool C_Deactivator::MustActivate()
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 totalKinEnergy(m_pGraphNode->m_TotalKineticEnergy);
		const Maths::HF32 ke(totalKinEnergy | totalKinEnergy);
		const Maths::Scalar_Bool bActivate(ke.GT(m_ActivateThresholdSq));
		const Maths::Scalar_Bool bStaticActivate(m_StaticActivateThresholdSq.GE(zero) & m_StaticActivateThresholdSq.LE(ke));
		
		bool result = (((m_pGraphNode->TestNodeFlags(C_GraphNode::E_Type_Sleeping)) & (bool)bActivate) | 
			(m_pGraphNode->TestNodeFlags(C_GraphNode::E_Type_Static) & (bool)bStaticActivate)) != 0;
		m_CountDown = Maths::Select(result, C_Deactivator::m_sSleepCountdown, m_CountDown);
		return result;
	}


	bool C_Deactivator::ActiveAndAboveSleepThresh()
	{
		const Maths::HF32 sleepThreshSq(m_SleepThresholdSq);
		const Maths::HVec3 totalKinEnergy(m_pGraphNode->m_TotalKineticEnergy);
		const Maths::HF32 ke(totalKinEnergy | totalKinEnergy);
		return m_pGraphNode->TestNodeFlags(C_GraphNode::E_Type_Active) && ke > sleepThreshSq;
	}

	void C_GraphNode::WakeUpConstrainedObjects(SharedPhysics::C_SafeStack<true>& stack, Gravity::C_BodyList& wakingList)
	{
		//Iterate over all objects this object is constrained to.

		//Is it already processed? Does it have persistent constraints? Is it active and above sleep threshold
		/*if(TestNodeFlags(C_GraphNode::E_Type_Visited_Wakeup) || (m_pPersistentConstraints == NULL && m_pPersistentTwinConstraints == NULL) ||
			!m_Deactivator.ActiveAndAboveSleepThresh())
			return;*/

		if(TestNodeFlags(C_GraphNode::E_Type_Visited_Wakeup) || (m_pPersistentConstraints == NULL && m_pPersistentTwinConstraints == NULL)
			|| !TestNodeFlags(C_GraphNode::E_Type_Active | C_GraphNode::E_Type_KinematicObject | C_GraphNode::E_Type_Waking))
			return;

		bool bFullWakeUp = TestNodeFlags(C_GraphNode::E_Type_KinematicObject) || 
			m_Deactivator.ActiveAndAboveSleepThresh();


		const U32 mergeFlag = Maths::Select(bFullWakeUp,  (U32)0, (U32)C_GraphNode::E_Type_LightWakeUp);


		//OK - we've not visited a node, it has constraints and it isn't below the deactivation threshold, so
		//we need to wake up all nodes constrained to it!!!!

		SetNodeFlags(C_GraphNode::E_Type_Visited_Wakeup);
		C_GraphNode* pStackTop(this);
		stack.Push(this, 0);
		S32 size = 1;
		while(size--)
		{
			C_GraphNode* pNode(pStackTop);
			pStackTop = stack.Pop<C_GraphNode>(Maths::Max(size-1, 0));
			
			C_ConstraintEdge* pEdge(pNode->m_pPersistentConstraints);
			while(pEdge)
			{
				if((pEdge->m_pOutboundNode->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld)) & 
					(!pEdge->m_pOutboundNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Wakeup)))
				{
					stack.Push(pEdge->m_pOutboundNode, size++);
					pEdge->m_pOutboundNode->SetNodeFlags( C_GraphNode::E_Type_Visited_Wakeup );
					if((pEdge->m_pOutboundNode->TestNodeFlags( C_GraphNode::E_Type_Sleeping )) &
						(!pEdge->m_pOutboundNode->TestNodeFlags( C_GraphNode::E_Type_Waking )) )
					{
						if(!pEdge->m_pOutboundNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
						{
							wakingList.AddBody(pEdge->m_pOutboundNode->m_pBody);
						}
						pEdge->m_pOutboundNode->SetNodeFlags( C_GraphNode::E_Type_Waking | mergeFlag | SharedPhysics::C_GraphNode::E_Type_Interfered );
					}
					pStackTop = pEdge->m_pOutboundNode;
				}
				pEdge = pEdge->m_pNext;
			}

			C_ConstraintTwin* pTwin(pNode->m_pPersistentTwinConstraints);
			while(pTwin)
			{
				pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
				//pEdge = pTwin->m_pEdge;
				if((pEdge->m_pInboundNode->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld)) &
					(!pEdge->m_pInboundNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Wakeup)))
				{
					stack.Push(pEdge->m_pInboundNode, size++);
					pEdge->m_pInboundNode->SetNodeFlags( C_GraphNode::E_Type_Visited_Wakeup );
					if((pEdge->m_pInboundNode->TestNodeFlags( C_GraphNode::E_Type_Sleeping )) & 
						(!pEdge->m_pInboundNode->TestNodeFlags( C_GraphNode::E_Type_Waking )) )
					{
						if(!pEdge->m_pInboundNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
						{
							wakingList.AddBody(pEdge->m_pInboundNode->m_pBody);
						}
						pEdge->m_pInboundNode->SetNodeFlags( C_GraphNode::E_Type_Waking | mergeFlag | SharedPhysics::C_GraphNode::E_Type_Interfered );
					}
					pStackTop = pEdge->m_pInboundNode;
				}
				pTwin = pTwin->m_pNext;
			}
		}
	}

	void C_GraphNode::ResistDeactivationConstrainedObjects(SharedPhysics::C_SafeStack<true>& stack)
	{
		//Iterate over all objects this object is constrained to.

		//Is it already processed? Does it have persistent constraints? Is it active and above sleep threshold
		/*if(TestNodeFlags(C_GraphNode::E_Type_Visited_Wakeup) || (m_pPersistentConstraints == NULL && m_pPersistentTwinConstraints == NULL) ||
			!m_Deactivator.ActiveAndAboveSleepThresh())
			return;*/

		if(TestNodeFlags(C_GraphNode::E_Type_Visited_Resistance) || 
			!TestNodeFlags(C_GraphNode::E_Type_Active | C_GraphNode::E_Type_KinematicObject) || 
			(m_Deactivator.m_CountDown <= 1 && !m_Deactivator.ActiveAndAboveSleepThresh()))
			return;

		//OK - we've not visited a node, it has constraints and it isn't below the deactivation threshold, so
		//we need to wake up all nodes constrained to it!!!!

		SetNodeFlags(C_GraphNode::E_Type_Visited_Wakeup | C_GraphNode::E_Type_Visited_Resistance);
		C_GraphNode* pStackTop(this);
		stack.Push(this, 0);
		S32 size = 1;
		while(size--)
		{
			C_GraphNode* pNode(pStackTop);
			pStackTop = stack.Pop<C_GraphNode>(Maths::Max(size-1, 0));
			
			C_ConstraintEdge* pEdge(pNode->m_pPersistentConstraints);
			while(pEdge)
			{
				pEdge->m_pOutboundNode->m_Deactivator.m_CountDown = Maths::Max(2, pEdge->m_pOutboundNode->m_Deactivator.m_CountDown);
				if(pEdge->m_pOutboundNode->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) && 
					!pEdge->m_pOutboundNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Resistance))
				{
					stack.Push(pEdge->m_pOutboundNode, size++);
					pEdge->m_pOutboundNode->SetNodeFlags( C_GraphNode::E_Type_Visited_Resistance );
					//pEdge->m_pOutboundNode->m_Deactivator.Activate();
					pStackTop = pEdge->m_pOutboundNode;
				}
				pEdge = pEdge->m_pNext;
			}

			C_ConstraintTwin* pTwin(pNode->m_pPersistentTwinConstraints);
			while(pTwin)
			{
				//pEdge = pTwin->m_pEdge;
				pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
				pEdge->m_pInboundNode->m_Deactivator.m_CountDown = Maths::Max(2, pEdge->m_pOutboundNode->m_Deactivator.m_CountDown);
				if(pEdge->m_pOutboundNode->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) && 
					!pEdge->m_pOutboundNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Resistance))
				{
					stack.Push(pEdge->m_pInboundNode, size++);
					pEdge->m_pInboundNode->SetNodeFlags( C_GraphNode::E_Type_Visited_Resistance );
					//pEdge->m_pOutboundNode->m_Deactivator.Activate();
					pStackTop = pEdge->m_pInboundNode;
				}
				pTwin = pTwin->m_pNext;
			}
		}
	}

#endif

	SharedPhysics::C_ConstraintEdge* C_GraphNode::FindPrevConstraint(SharedPhysics::C_GraphNode* pNodeB)
	{
		SharedPhysics::C_ConstraintEdge* pEdge = m_pVolatileConstraints;
		while(pEdge)
		{
			Spu::MemBaseMarker marker;
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pEdge, sizeof(SharedPhysics::C_ConstraintEdge));
			if(pSpuEdge->m_pOutboundNode == pNodeB->m_pPPuPtr)
				return pEdge;
			pEdge = pSpuEdge->m_pNext;
		}

		SharedPhysics::C_ConstraintTwin* pTwin = m_pVolatileTwinConstraints;
		while(pTwin)
		{
			Spu::MemBaseMarker marker;
			pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pEdge, sizeof(SharedPhysics::C_ConstraintEdge));
			SharedPhysics::C_ConstraintTwin* pSpuTwin = &pSpuEdge->m_TwinConstraint;
			/*SharedPhysics::C_ConstraintTwin* pSpuTwin = (SharedPhysics::C_ConstraintTwin*)Spu::FetchAllocUnaligned(pTwin, sizeof(SharedPhysics::C_ConstraintTwin));
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAllocUnaligned(pSpuTwin->m_pEdge, sizeof(SharedPhysics::C_ConstraintEdge));*/
			if(pSpuEdge->m_pInboundNode == pNodeB->m_pPPuPtr)
				return pEdge;
			pTwin = pSpuTwin->m_pNext;
		}

		pEdge = m_pCloseConstraints;
		while(pEdge)
		{
			Spu::MemBaseMarker marker;
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pEdge, sizeof(SharedPhysics::C_ConstraintEdge));
			if(pSpuEdge->m_pOutboundNode == pNodeB->m_pPPuPtr)
				return pEdge;
			pEdge = pSpuEdge->m_pNext;
		}

		pTwin = m_pCloseTwinConstraints;
		while(pTwin)
		{
			Spu::MemBaseMarker marker;
			pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
			SharedPhysics::C_ConstraintEdge* pSpuEdge = (SharedPhysics::C_ConstraintEdge*)Spu::FetchAlloc(pEdge, sizeof(SharedPhysics::C_ConstraintEdge));
			SharedPhysics::C_ConstraintTwin* pSpuTwin = &pSpuEdge->m_TwinConstraint;
			if(pSpuEdge->m_pInboundNode == pNodeB->m_pPPuPtr)
				return pEdge;
			pTwin = pSpuTwin->m_pNext;
		}

		return NULL;
	}

};
