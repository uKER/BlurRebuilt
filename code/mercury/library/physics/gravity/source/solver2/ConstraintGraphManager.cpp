#include "StdAfx.h"
#include "gravity/solver2/ConstraintGraphManager.h"
#include "gravity/sharedphysics/Collider.h"
#include "gravity/sharedphysics/AllocationManager.inl"
#include "gravity/sharedphysics/ContactDatabase.inl"
#include "gravity/sharedphysics/ContactStream.inl"
#include "gravity/sharedphysics/ContactStreamIterator.inl"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/sdk/BodyEvents.h"
#include "gravity/sdk/Body.h" 
#include "gravity/narrowphase/shape.h"
#include <system/perf.h>

namespace SharedPhysics
{

//#define ENABLE_WARMSTART 1

	void C_GraphNode::CalculateBVector(const Maths::HF32 PCREF time)
	{
		if(m_NodeFlags & E_Type_Active)
		{
			m_pRigidBody->CalculateBVector(time);
		}
		else
		{
			m_pRigidBody->CopyVelocityToBVector();
		}
	}


	void SharedPhysics::C_Constraint::NormaliseMassRatios(const Maths::HF32 PCREF maxRelativeMass, const Maths::HF32 PCREF assumedInfiniteRelativeMass)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HVec3 vOne(Maths::Constants<Maths::HVec3>::One());
		SharedPhysics::C_RigidBody* pRigidA = pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = pNodeB->m_pRigidBody;
		//const Maths::HF32 recipMaxRelativeMass(maxRelativeMass.Reciprocal());

		if(!pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | 
			SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			const Maths::HF32 massA(pRigidA->Mass());
			const Maths::HF32 massB(pRigidB->Mass());
			//Test to see if we need to do mass scaling!
			const Maths::HF32 bigMass(Maths::Max(massA, massB));
			const Maths::HF32 smallMass(Maths::Min(massA, massB));

			const Maths::HF32 scale(bigMass/smallMass);
			if(scale > assumedInfiniteRelativeMass)
			{
				//Need to set the larger one to zero!
				if(massA > massB)
				{
					SetBodyAMassScale(zero);
					//SetBodyAInertiaScale(zero);
				}
				else
				{
					SetBodyBMassScale(zero);
					//SetBodyBInertiaScale(zero);
				}
			}
			else if(scale > maxRelativeMass)
			{
				const Maths::HF32 newScale(scale/maxRelativeMass);

				if(massA > massB)
				{
					SetBodyAMassScale(newScale);
					//SetBodyAInertiaScale(newScale);
				}
				else
				{
					SetBodyBMassScale(newScale);
					//SetBodyBInertiaScale(newScale);
				}
			}

			/*if(pRigidA->m_Mass > pRigidB->m_Mass)
			{
				Maths::HMat33 bigInertia(pRigidA->m_IBody);
				Maths::HMat33 smallInertia(pRigidB->m_IBody);

				const Maths::HVec3 bigInertiaLength(bigInertia.AX().Length(), bigInertia.AY().Length(), bigInertia.AZ().Length());
				const Maths::HVec3 smallInertiaLength(smallInertia.AX().Length(), smallInertia.AY().Length(), smallInertia.AZ().Length());

				const Maths::HVec3 inertiaScale(bigInertiaLength/smallInertiaLength);

				const Maths::HVec3 _newInertiaScale(inertiaScale/maxRelativeMass);
				const Maths::Vector3_Bool bInRange(_newInertiaScale >= (Maths::HVec3(recipMaxRelativeMass)) & 
					_newInertiaScale <= Maths::HVec3(maxRelativeMass));
				const Maths::HVec3 newInertiaScale(Maths::Select(bInRange, vOne, _newInertiaScale));

				SetBodyAInertiaScale(newInertiaScale);
			}
			else
			{
				Maths::HMat33 bigInertia(pRigidB->m_IBody);
				Maths::HMat33 smallInertia(pRigidA->m_IBody);

				const Maths::HVec3 bigInertiaLength(bigInertia.AX().Length(), bigInertia.AY().Length(), bigInertia.AZ().Length());
				const Maths::HVec3 smallInertiaLength(smallInertia.AX().Length(), smallInertia.AY().Length(), smallInertia.AZ().Length());

				const Maths::HVec3 inertiaScale(bigInertiaLength/smallInertiaLength);

				const Maths::HVec3 _newInertiaScale(inertiaScale/maxRelativeMass);
				const Maths::Vector3_Bool bInRange(_newInertiaScale >= (Maths::HVec3(recipMaxRelativeMass)) & 
					_newInertiaScale <= Maths::HVec3(maxRelativeMass));
				const Maths::HVec3 newInertiaScale(Maths::Select(bInRange, vOne, _newInertiaScale));

				SetBodyBInertiaScale(newInertiaScale);
			}*/
		}
	}
}

namespace Solver2
{

	void C_ConstraintGraphManager::PrepareForNextFrame(Gravity::C_BodyList& activeBodies)
	{
		using namespace SharedPhysics;
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		//Loop through graph nodes and clear volatile constraints...
		//for(S32 a = 0; a < m_GraphNodeArray.m_numElements; ++a)
		//{
		//	SharedPhysics::C_GraphNode* pNode = m_GraphNodeArray.m_pElems[a].m_pElement;
		//	pNode->ClearNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation | C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_Root_Node | C_GraphNode::E_Type_Visited_Wakeup );


		//	//NB!!!! Don't need to blank volatile constraints anymore - we can recycle them instead!
		//	/*pNode->m_pVolatileConstraints = NULL;
		//	pNode->m_pVolatileTwinConstraints = NULL;*/

		//	if(pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		//	{
		//		pNode->m_TotalKineticEnergy = zero;
		//		pNode->m_pRigidBody->m_KineticEnergy = zero;
		//		pNode->m_IslandIndex = 0;
		//	}
		//	else
		//	{
		//		pNode->m_TotalKineticEnergy = pNode->m_pRigidBody->KineticEnergy();
		//		pNode->m_IslandIndex = -1;
		//	}
		//}

		Gravity::C_Body* pBody = activeBodies.m_pFirstBody;
		while(pBody)
		{
			SharedPhysics::C_GraphNode* pNode = pBody->GetGraphNode();
			pNode->m_TotalKineticEnergy = pNode->m_pRigidBody->KineticEnergy();
			pNode->m_IslandIndex = -1;
			pBody = pBody->m_pNext;
		}
	}

	using namespace SharedPhysics;

	template <typename ListItem>
	void CheckList( ListItem* pList )
	{
		if(pList == NULL)
			return;
		ListItem* pItem(pList);
		while(pItem->m_pNext)
		{
			BIZ_ASSERT(pItem->m_pNext->m_pPrev == pItem);
			pItem = pItem->m_pNext;
		}
	}

	SharedPhysics::C_ConstraintEdge* C_ConstraintGraphManager::GetConstraintEdge(SharedPhysics::C_GraphNode* pNodeA, SharedPhysics::C_GraphNode* pNodeB)
	{
		//First see if the constraint exists...
		SharedPhysics::C_ConstraintEdge tempEdge;
		tempEdge.m_pInboundNode = pNodeA;
		tempEdge.m_pOutboundNode = pNodeB;

		BIZ_ASSERT(ValidConstraintArray());
		C_ConstraintEdgeWrapper wrapper(&tempEdge);

		S32 iPos;
		S32 pos(m_ConstraintEdgeArray.FindIndex(&wrapper, iPos));

		if(pos >= 0)
		{
			//Verify that this edge is actually in the body!
			BIZ_ASSERT(pNodeA->ContainsEdge(m_ConstraintEdgeArray[pos].m_pElement));
			return m_ConstraintEdgeArray[pos].m_pElement;
		}

		//Try binning this version because we don't want to accidentally reverse the constraint!
		/*tempEdge.m_pInboundNode = pNodeB;
		tempEdge.m_pOutboundNode = pNodeA;

		C_ConstraintEdgeWrapper* pWrapper(m_ConstraintEdgeArray.Find(&wrapper));
		if(pWrapper)
		{
			BIZ_ASSERT(pNodeB->ContainsEdge(pWrapper->m_pElement));
			return pWrapper->m_pElement;
		}*/

		SharedPhysics::C_ConstraintEdge* pEdge(m_PersistentConstraintEdges.GetNextFreeItem());
		SharedPhysics::C_ConstraintTwin* pTwin(&pEdge->m_TwinConstraint);//m_PersistentTwinConstraints.GetNextFreeItem());
		//pEdge->m_pTwinConstraint = pTwin;
		//pTwin->m_pEdge = pEdge;
		const bool aStatic(pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject));
		pEdge->m_pInboundNode = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(aStatic, pNodeB, pNodeA);
		pEdge->m_pOutboundNode = SharedPhysics::Fast_Select<SharedPhysics::C_GraphNode>(aStatic, pNodeA, pNodeB);

		SharedPhysics::LinkPersistent(pEdge->m_pInboundNode, pEdge->m_pOutboundNode, pEdge, pTwin);

		C_ConstraintEdgeWrapper wr(pEdge);
		
		m_ConstraintEdgeArray.Insert(&wr);
		BIZ_ASSERT(ValidConstraintArray());

		return pEdge;
	}


	void C_ConstraintGraphManager::ReleaseConstraint(SharedPhysics::C_Constraint* pConstraint)
	{
		//Switch based on which type of constraint this is!!!

		switch(pConstraint->m_ConstraintType)
		{
		case SharedPhysics::C_Constraint::E_Type_FixedRotational:
			m_FixedConstraints.PushToFree((C_FixedRotationalConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_Distance:
			m_DistanceConstraints.PushToFree((C_DistanceConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_Spherical:
			m_SphericalConstraints.PushToFree((C_SphericalConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_Revolute:
			m_RevoluteConstraints.PushToFree((C_RevoluteConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_RevoluteLimit:
			m_RevoluteLimitConstraint.PushToFree((C_RevoluteLimitConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_Prismatic:
			m_PrismaticConstraint.PushToFree((C_PrismaticConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_PrismaticLimit:
			m_PrismaticLimitConstraint.PushToFree((C_PrismaticLimitConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_CarWheel:
			m_CarWheelConstraints.PushToFree((C_CarWheelConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_TwoRotational:
			m_TwoRotationalConstraints.PushToFree((C_TwoRotationalConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_SphericalLimit:
			m_SphericalLimitConstraints.PushToFree((C_SphericalLimitConstraint*)pConstraint);
			break;
		case SharedPhysics::C_Constraint::E_Type_Proxy:
			m_ProxyConstraints.PushToFree((C_ProxyConstraint*)pConstraint);
			break;
		default:
			{
				BIZ_ASSERT(0);
			}
		}

	}


	void C_ConstraintGraphManager::RemoveDebugConstraints(SharedPhysics::C_Constraint* pConstraint)
	{
		if(pConstraint == NULL || m_DebugConstraints.m_numElements == 0)
			return;

		U32 size = m_DebugConstraints.m_numElements;
		C_ConstraintPtr wrapper(pConstraint);
		m_DebugConstraints.Remove(&wrapper);
		
	}

	void C_ConstraintGraphManager::AddDebugConstraints(SharedPhysics::C_Constraint* pConstraint)
	{
		C_ConstraintPtr wrapper(pConstraint);
		m_DebugConstraints.AddToEnd(&wrapper);
	}

	void C_ConstraintGraphManager::RemoveConstraints(SharedPhysics::C_Constraint* pConstraint)
	{
		if(pConstraint == NULL)
			return;

		if(pConstraint->m_ConstraintType == SharedPhysics::C_Constraint::E_Type_Proxy)
		{
			C_ProxyConstraint* pRev = (C_ProxyConstraint*)pConstraint;
			for(S32 i=0; i<pRev->m_Constraints.m_numElements; ++i)
			{
				RemoveConstraints(pRev->m_Constraints[i]);
			}
			ReleaseConstraint(pConstraint);
			return;
		}

		SharedPhysics::C_GraphNode* pNodeA = pConstraint->pNodeA;
		SharedPhysics::C_GraphNode* pNodeB = pConstraint->pNodeB;


		//Loop through all the persistent constraints in nodeA to find a constraint!

		C_ConstraintEdge* pEdge = pNodeA->m_pPersistentConstraints;

		while(pEdge)
		{
			if(pEdge->m_pOutboundNode == pNodeB)
			{
				//Loop through all the constraint points in this edge!
				SharedPhysics::C_ConstraintPoint* pPoint = pEdge->m_pFirstConstraint;
				while(pPoint)
				{
					if(pPoint->m_pConstraint == pConstraint)
					{
						//Remove this constraint!
						if(pPoint == pEdge->m_pFirstConstraint)
							pEdge->m_pFirstConstraint = pPoint->m_pNext;
						if(pPoint->m_pPrev)
							pPoint->m_pPrev->m_pNext = pPoint->m_pNext;
						if(pPoint->m_pNext)
							pPoint->m_pNext->m_pPrev = pPoint->m_pPrev;
						this->m_PersistentConstraintPoints.PushToFree(pPoint);						
						break;
					}
					pPoint = pPoint->m_pNext;
				}
				

				SharedPhysics::C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;

				if(pEdge->m_pFirstConstraint == NULL)
				{
					Unlink(pNodeA, pNodeB, pEdge, pTwin);
					C_ConstraintEdgeWrapper wrapper(pEdge);
					BIZ_ASSERT(ValidConstraintArray());
					bool bRemoved = this->m_ConstraintEdgeArray.Remove(&wrapper);
					BIZ_ASSERT(bRemoved);
					BIZ_ASSERT(ValidConstraintArray());
					m_PersistentConstraintEdges.PushToFree(pEdge);
					//m_PersistentTwinConstraints.PushToFree(pTwin);
				}

				//Release constraint!
				ReleaseConstraint(pConstraint);

				//return;
				break;
			}
			pEdge = pEdge->m_pNext;
		}

		pEdge = pNodeB->m_pPersistentConstraints;

		while(pEdge)
		{
			if(pEdge->m_pOutboundNode == pNodeA)
			{
				//Loop through all the constraint points in this edge!
				SharedPhysics::C_ConstraintPoint* pPoint = pEdge->m_pFirstConstraint;
				while(pPoint)
				{
					if(pPoint->m_pConstraint == pConstraint)
					{
						//Remove this constraint!
						if(pPoint == pEdge->m_pFirstConstraint)
							pEdge->m_pFirstConstraint = pPoint->m_pNext;
						if(pPoint->m_pPrev)
							pPoint->m_pPrev->m_pNext = pPoint->m_pNext;
						if(pPoint->m_pNext)
							pPoint->m_pNext->m_pPrev = pPoint->m_pPrev;

						this->m_PersistentConstraintPoints.PushToFree(pPoint);						
						break;
					}
					pPoint = pPoint->m_pNext;
				}

				SharedPhysics::C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;

				if(pEdge->m_pFirstConstraint == NULL)
				{
					Unlink(pNodeB, pNodeA, pEdge, pTwin);
					C_ConstraintEdgeWrapper wrapper(pEdge);
					BIZ_ASSERT(ValidConstraintArray());
					bool bRemoved = m_ConstraintEdgeArray.Remove(&wrapper);
					BIZ_ASSERT(bRemoved);
					BIZ_ASSERT(ValidConstraintArray());
					m_PersistentConstraintEdges.PushToFree(pEdge);
					//m_PersistentTwinConstraints.PushToFree(pTwin);
				}

				//Release constraint!
				ReleaseConstraint(pConstraint);
				//return;
				break;
			}
			pEdge = pEdge->m_pNext;
		}
	}

	bool C_ConstraintGraphManager::ValidConstraintArray()
	{
		return true;
		if(m_ConstraintEdgeArray.m_numElements == 0)
			return true;

		SharedPhysics::C_ConstraintEdge* pEdge = m_ConstraintEdgeArray[0].m_pElement;
		if(pEdge->m_pInboundNode == NULL || pEdge->m_pOutboundNode == NULL)
				return false;
		for(S32 a = 1; a < m_ConstraintEdgeArray.m_numElements; ++a)
		{
			SharedPhysics::C_ConstraintEdge* pNextEdge = m_ConstraintEdgeArray[a].m_pElement;
			if(pNextEdge->m_pInboundNode == NULL || pNextEdge->m_pOutboundNode == NULL)
				return false;

			if(*pEdge > *pNextEdge)
			{
				DebugBreak();
				return false;
			}
			pEdge = pNextEdge;
		}
		return true;
	}

	void C_ConstraintGraphManager::RemoveConstraints( SharedPhysics::C_GraphNode* __restrict pNode, Gravity::C_EventStream& eventStream)
	{
		SharedPhysics::SortedArray<C_Constraint*> constraints;

		SharedPhysics::C_ConstraintTwin* pTwin = pNode->m_pPersistentTwinConstraints;
		while(pTwin)
		{
			SharedPhysics::C_ConstraintTwin* pNextTwin = pTwin->m_pNext;
			SharedPhysics::C_ConstraintEdge* pConstraintEdge = SharedPhysics::C_ConstraintEdge::GetConstraintEdge(pTwin);
			BIZ_ASSERT(ValidConstraintArray());
			C_ConstraintEdgeWrapper constraintEdgeWrapper(pConstraintEdge);
			m_ConstraintEdgeArray.Remove(&constraintEdgeWrapper);
			BIZ_ASSERT(ValidConstraintArray());

			Unlink(pConstraintEdge->m_pInboundNode, pConstraintEdge->m_pOutboundNode, pConstraintEdge, pTwin);

			SharedPhysics::C_ConstraintPoint* pConstraint = pConstraintEdge->m_pFirstConstraint;
			while(pConstraint)
			{
				SharedPhysics::C_ConstraintPoint* pNextConstraint = pConstraint->m_pNext;

				SharedPhysics::C_Constraint* pCon = pConstraint->m_pConstraint;

				Gravity::C_BodyEvent* pEvent = eventStream.CreateEvent();
				pEvent->SetConstraintUserData(pCon->m_pUserData);
				pEvent->SetEventType(Gravity::C_BodyEvent::E_RemoveConstraint);

				m_PersistentConstraintPoints.PushToFree(pConstraint);
				if(pCon->pRoot)
				{
					/*C_ConstraintPtr wrapper(pCon->pRoot);
					constraints.InsertOverwrite(&wrapper);	*/
					constraints.InsertOverwrite(&pCon->pRoot);
				}
				ReleaseConstraint(pCon);
				//debugConstraints.Remove(&SharedPhysics::C_PointerWrapper<SharedPhysics::C_Constraint>(pCon));

				pConstraint = pNextConstraint;
			}

			m_PersistentConstraintEdges.PushToFree(pConstraintEdge);
			//m_PersistentTwinConstraints.PushToFree(pTwin);
			BIZ_ASSERT(ValidConstraintArray());

			pTwin = pNextTwin;
		}

		SharedPhysics::C_ConstraintEdge* pConstraintEdge = pNode->m_pPersistentConstraints;

		while(pConstraintEdge)
		{
			SharedPhysics::C_ConstraintEdge* pNextEdge = pConstraintEdge->m_pNext;
			SharedPhysics::C_ConstraintTwin* pTwin = &pConstraintEdge->m_TwinConstraint;

			BIZ_ASSERT(ValidConstraintArray());
			C_ConstraintEdgeWrapper constraintEdgeWrapper(pConstraintEdge);
			bool bRemoved = m_ConstraintEdgeArray.Remove(&constraintEdgeWrapper);
			BIZ_ASSERT(bRemoved);
			BIZ_ASSERT(ValidConstraintArray());

			Unlink(pConstraintEdge->m_pInboundNode, pConstraintEdge->m_pOutboundNode, pConstraintEdge, pTwin);

			SharedPhysics::C_ConstraintPoint* pConstraint = pConstraintEdge->m_pFirstConstraint;
			while(pConstraint)
			{
				SharedPhysics::C_ConstraintPoint* pNextConstraint = pConstraint->m_pNext;

				SharedPhysics::C_Constraint* pCon = pConstraint->m_pConstraint;

				m_PersistentConstraintPoints.PushToFree(pConstraint);

				Gravity::C_BodyEvent* pEvent = eventStream.CreateEvent();
				pEvent->SetConstraintUserData(pCon->m_pUserData);
				pEvent->SetEventType(Gravity::C_BodyEvent::E_RemoveConstraint);
				if(pCon->pRoot)
				{
					/*C_ConstraintPtr wrapper(pCon->pRoot);
					constraints.InsertOverwrite(&wrapper);	*/
					constraints.InsertOverwrite(&pCon->pRoot);
				}

				ReleaseConstraint(pCon);
				//debugConstraints.Remove(&SharedPhysics::C_PointerWrapper<SharedPhysics::C_Constraint>(pCon));

				pConstraint = pNextConstraint;
			}

			m_PersistentConstraintEdges.PushToFree(pConstraintEdge);
			//m_PersistentTwinConstraints.PushToFree(pTwin);

			//In case calling destructors broke the array (in which case we're in trouble with the sorted array - or we've got
			//the same element in multiple times...)
			BIZ_ASSERT(ValidConstraintArray());

			pConstraintEdge = pNextEdge;
		}

		//remove all the constraints in the debugconstraint list
		for(S32 i=0; i<constraints.m_numElements; ++i)
		{
			BIZ_ASSERT(constraints[i]->m_ConstraintType == SharedPhysics::C_Constraint::E_Type_Proxy);
			
			C_ProxyConstraint* pConstraint = (C_ProxyConstraint*)constraints[i];
			pConstraint->m_Constraints.m_numElements = 0;
			RemoveDebugConstraints(constraints[i]);
			//ReleaseConstraint(constraints[i]);
		}
	}


	void C_ConstraintGraphManager::RemoveGraphNode( SharedPhysics::C_GraphNode* __restrict pNode, Gravity::C_EventStream& eventStream, bool bRemoveConstraints)
	{
		C_GraphNodeWrapper wrapper(pNode);
		m_GraphNodeArray.Remove(&wrapper);
		if(bRemoveConstraints)
			RemoveConstraints( pNode, eventStream);
	}
	
	void C_ConstraintGraphManager::RemoveStaticGraphNode(SharedPhysics::C_GraphNode* __restrict pNode)
	{
		C_GraphNodeWrapper wrapper(pNode);
		m_StaticGraphNodeArray.Remove(&wrapper);
		//RemoveConstraints(pNode);
	}


	void C_ConstraintGraphManager::MergeSmallConstraintGroups(Gravity::C_BodyList& activeBodies, Gravity::C_BodyList& continuousBodies, Gravity::C_BodyList& kinematicBodies, const S32 islandSizeThreshold)
	{
		using namespace Gravity;
		PERF_EVENT("Gravity.MergeSmallConstraintGroups");
		C_Body* pCurrentBody = activeBodies.m_pFirstBody;
		C_Body* pNextBody = (pCurrentBody == NULL) ? NULL : pCurrentBody->m_pNext;

		while(pCurrentBody)
		{
			SharedPhysics::C_GraphNode* pGraphNode = pCurrentBody->GetGraphNode();

			if(pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node))
			{
				while(pNextBody && pGraphNode->m_IslandSize < islandSizeThreshold)
				{
					while(pNextBody)
					{
						SharedPhysics::C_GraphNode* pNextNode = pNextBody->GetGraphNode();
						if(pNextNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node) && 
							(pNextNode->m_IslandSize < islandSizeThreshold))
						{
							//Connect this node to next body...

							C_ConstraintEdge* pEdge = m_Edges.Create();
							pEdge->m_pNext = pEdge->m_pPrev = NULL;
							//C_ConstraintTwin* pTwin = m_Twins.Create();
							C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
							SharedPhysics::LinkVolatile(pGraphNode, pNextNode, pEdge, pTwin);
							pNextNode->ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node);
							pGraphNode->m_IslandSize += pNextNode->m_IslandSize;

							pNextBody = pNextBody->m_pNext;
							break;
						}
						pNextBody = pNextBody->m_pNext;
					}
				}
			}
			pCurrentBody = pNextBody;
			pNextBody = (pNextBody == NULL) ? NULL : pNextBody->m_pNext;
		}


		//pCurrentBody = kinematicBodies.m_pFirstBody;
		//pNextBody = (pCurrentBody == NULL) ? NULL : pCurrentBody->m_pNext;

		//while(pCurrentBody)
		//{
		//	SharedPhysics::C_GraphNode* pGraphNode = pCurrentBody->GetGraphNode();

		//	if(pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node))
		//	{
		//		while(pNextBody && pGraphNode->m_IslandSize < islandSizeThreshold)
		//		{
		//			while(pNextBody)
		//			{
		//				SharedPhysics::C_GraphNode* pNextNode = pNextBody->GetGraphNode();
		//				if(pNextNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node) && 
		//					(pNextNode->m_IslandSize < islandSizeThreshold))
		//				{
		//					//Connect this node to next body...

		//					C_ConstraintEdge* pEdge = m_Edges.Create();
		//					pEdge->m_pNext = pEdge->m_pPrev = NULL;
		//					//C_ConstraintTwin* pTwin = m_Twins.Create();
		//					C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
		//					SharedPhysics::LinkVolatile(pGraphNode, pNextNode, pEdge, pTwin);
		//					pNextNode->ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node);
		//					pGraphNode->m_IslandSize += pNextNode->m_IslandSize;

		//					pNextBody = pNextBody->m_pNext;
		//					break;
		//				}
		//				pNextBody = pNextBody->m_pNext;
		//			}
		//		}
		//	}
		//	pCurrentBody = pNextBody;
		//	pNextBody = (pNextBody == NULL) ? NULL : pNextBody->m_pNext;
		//}


		pCurrentBody = continuousBodies.m_pFirstBody;
		pNextBody = (pCurrentBody == NULL) ? NULL : pCurrentBody->m_pNext;

		while(pCurrentBody)
		{
			SharedPhysics::C_GraphNode* pGraphNode = pCurrentBody->GetGraphNode();

			if(pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node))
			{
				while(pNextBody && pGraphNode->m_IslandSize < islandSizeThreshold)
				{
					while(pNextBody)
					{
						SharedPhysics::C_GraphNode* pNextNode = pNextBody->GetGraphNode();
						if(pNextNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node) && 
							(pNextNode->m_IslandSize < islandSizeThreshold))
						{
							//Connect this node to next body...

							C_ConstraintEdge* pEdge = m_Edges.Create();
							pEdge->m_pNext = pEdge->m_pPrev = NULL;
							//C_ConstraintTwin* pTwin = m_Twins.Create();
							C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
							SharedPhysics::LinkVolatile(pGraphNode, pNextNode, pEdge, pTwin);
							pNextNode->ClearNodeFlags(SharedPhysics::C_GraphNode::E_Type_Root_Node);
							pGraphNode->m_IslandSize += pNextNode->m_IslandSize;

							pNextBody = pNextBody->m_pNext;
							break;
						}
						pNextBody = pNextBody->m_pNext;
					}
				}
			}
			pCurrentBody = pNextBody;
			pNextBody = (pNextBody == NULL) ? NULL : pNextBody->m_pNext;
		}


	}

	S32 C_ConstraintGraphManager::SplitConstraintGroups(C_SafeStack<true>& stack, Gravity::C_BodyList& activeBodies, Gravity::C_BodyList& continuousBodies, Gravity::C_BodyList& kinematicBodies,
		const Maths::HF32 PCREF time)
	{
		PERF_EVENT("Gravity.SplitConstraintGroups");
		//Clear visited flags, island index and root node  
		{
			PERF_EVENT("Gravity.SplitConstraintGroups1");
			//for(S32 a =0; a < m_GraphNodeArray.m_numElements; ++a)
			Gravity::C_Body* pBody = activeBodies.m_pFirstBody;
			while(pBody)
			{
				//C_GraphNodeWrapper& wrapper(m_GraphNodeArray.m_pElems[a]);
				//C_GraphNode* pNode = wrapper.m_pElement;
				C_GraphNode* pNode = pBody->GetGraphNode();//wrapper.m_pElement;
				pNode->ClearNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation | C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_Root_Node | C_GraphNode::E_Type_Visited_Wakeup | 
					C_GraphNode::E_Type_Root_Node);
				pNode->m_IslandIndex = -1;
				pNode->ClearNodeFlags(C_GraphNode::E_State_Changed);
				pNode->m_IslandSize = 0;
				pBody = pBody->m_pNext;
			}

			pBody = continuousBodies.m_pFirstBody;
			while(pBody)
			{
				//C_GraphNodeWrapper& wrapper(m_GraphNodeArray.m_pElems[a]);
				//C_GraphNode* pNode = wrapper.m_pElement;
				C_GraphNode* pNode = pBody->GetGraphNode();//wrapper.m_pElement;
				pNode->ClearNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation | C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_Root_Node | C_GraphNode::E_Type_Visited_Wakeup | 
					C_GraphNode::E_Type_Root_Node);
				pNode->m_IslandIndex = -1;
				pNode->ClearNodeFlags(C_GraphNode::E_State_Changed);
				pNode->m_IslandSize = 0;
				pBody = pBody->m_pNext;
			}

			pBody = kinematicBodies.m_pFirstBody;
			while(pBody)
			{
				//C_GraphNodeWrapper& wrapper(m_GraphNodeArray.m_pElems[a]);
				//C_GraphNode* pNode = wrapper.m_pElement;
				C_GraphNode* pNode = pBody->GetGraphNode();//wrapper.m_pElement;
				pNode->ClearNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation | C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_Root_Node | C_GraphNode::E_Type_Visited_Wakeup | 
					C_GraphNode::E_Type_Root_Node);
				pNode->m_IslandIndex = 0;
				pNode->ClearNodeFlags(C_GraphNode::E_State_Changed);
				pNode->m_IslandSize = 0;
				pBody = pBody->m_pNext;
			}
		}

		//Now that everything is cleared, we need to traverse through the islands and see which node is 
		//part of which island

#if 1 //def GENERATE_ISLANDS
		const U32 skipMask = (C_GraphNode::E_Type_Sleeping_Or_Static | C_GraphNode::E_Type_Visited_Island_Generation);
		S32 islandIndex = 0;
		//for(S32 a = 0; a < m_GraphNodeArray.m_numElements; ++a)
		Gravity::C_Body* pBody = activeBodies.m_pFirstBody;
		while(pBody)
		{
			//C_GraphNodeWrapper wrapper(m_GraphNodeArray[a]);
			//C_GraphNode* pCurrentNode = wrapper.m_pElement;
			C_GraphNode* pCurrentNode = pBody->GetGraphNode();//wrapper.m_pElement;
			//assert(pCurrentNode->m_IslandIndex == -1);
			
			if(!pCurrentNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation))
			{
				//Walk from this node to all other nodes connected to it via the constraint graph
				++islandIndex;
				const C_GraphNode* const pRootNode = pCurrentNode;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
				if(!pCurrentNode->HasConstraints())
					continue;
				++pCurrentNode->m_IslandSize;
				C_GraphNode* __restrict pStackTop = pCurrentNode;
				S32 size = 1;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Root_Node);
				stack.Push(pCurrentNode, 0);

				//pNode->m_IslandIndex = islandIndex;

				while(size--)
				{
					C_GraphNode* pNode = pStackTop;//pStack[size];
					pNode->CalculateBVector(time);
					//if(size > 0)
					pStackTop = stack.Pop<C_GraphNode>(Maths::Max(size - 1, 0));
					
					//Loop through all this node's constraint edges to id the other 
					//bodies in this island...
					if(pNode->TestNodeFlags(C_GraphNode::E_Type_Active /*| C_GraphNode::E_Type_KinematicObject*/))
					{
						//const U32 origSize = size;
						C_ConstraintEdge* __restrict pEdge = pNode->m_pPersistentConstraints;
						while(pEdge)
						{
							C_GraphNode* __restrict pOtherNode = pEdge->m_pOutboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;

							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							//pNode->m_IslandIndex = islandIndex;
							pEdge = pEdge->m_pNext;
						}

						C_ConstraintTwin* __restrict pTwin = pNode->m_pPersistentTwinConstraints;
						while(pTwin)
						{
							pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
							C_GraphNode* __restrict pOtherNode = pEdge->m_pInboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							pTwin = pTwin->m_pNext;
						}

						pEdge = pNode->m_pVolatileConstraints;
						while(pEdge)
						{
							C_GraphNode* __restrict pOtherNode = pEdge->m_pOutboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							//pNode->m_IslandIndex = islandIndex;
							pEdge = pEdge->m_pNext;
						}
						
						pTwin = pNode->m_pVolatileTwinConstraints;
						while(pTwin)
						{
							pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
							C_GraphNode* __restrict pOtherNode = pEdge->m_pInboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							pTwin = pTwin->m_pNext;
						}
					}
				}

				
			}
			pBody = pBody->m_pNext;
		}

#if 0 //Kinematic will have woken up all bodies it touches, so the kinematic shouldn't need to be the root. This will avoid making islands with just a kinematic body in it!
		pBody = kinematicBodies.m_pFirstBody;
		while(pBody)
		{
			//C_GraphNodeWrapper wrapper(m_GraphNodeArray[a]);
			//C_GraphNode* pCurrentNode = wrapper.m_pElement;
			C_GraphNode* pCurrentNode = pBody->GetGraphNode();//wrapper.m_pElement;
			//assert(pCurrentNode->m_IslandIndex == -1);
			
			if(!pCurrentNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation))
			{
				//Walk from this node to all other nodes connected to it via the constraint graph
				++islandIndex;
				const C_GraphNode* const pRootNode = pCurrentNode;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
				if(!pCurrentNode->HasConstraints())
					continue;
				++pCurrentNode->m_IslandSize;
				C_GraphNode* __restrict pStackTop = pCurrentNode;
				S32 size = 1;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Root_Node);
				stack.Push(pCurrentNode, 0);

				//pNode->m_IslandIndex = islandIndex;

				while(size--)
				{
					C_GraphNode* pNode = pStackTop;//pStack[size];
					//if(size > 0)
					pStackTop = stack.Pop<C_GraphNode>(Maths::Max(size - 1, 0));
					
					//Loop through all this node's constraint edges to id the other 
					//bodies in this island...
					
					//const U32 origSize = size;
					if(pNode->TestNodeFlags(C_GraphNode::E_Type_Active | C_GraphNode::E_Type_KinematicObject))
					{
						C_ConstraintEdge* __restrict pEdge = pNode->m_pPersistentConstraints;
						while(pEdge)
						{
							C_GraphNode* __restrict pOtherNode = pEdge->m_pOutboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;

							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							//pNode->m_IslandIndex = islandIndex;
							pEdge = pEdge->m_pNext;
						}

						C_ConstraintTwin* __restrict pTwin = pNode->m_pPersistentTwinConstraints;
						while(pTwin)
						{
							C_GraphNode* __restrict pOtherNode = pTwin->m_pEdge->m_pInboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							pTwin = pTwin->m_pNext;
						}

						pEdge = pNode->m_pVolatileConstraints;
						while(pEdge)
						{
							C_GraphNode* __restrict pOtherNode = pEdge->m_pOutboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							//pNode->m_IslandIndex = islandIndex;
							pEdge = pEdge->m_pNext;
						}
						
						pTwin = pNode->m_pVolatileTwinConstraints;
						while(pTwin)
						{
							C_GraphNode* __restrict pOtherNode = pTwin->m_pEdge->m_pInboundNode;
							stack.Push(pOtherNode, size);
							const bool b = !pOtherNode->TestNodeFlags(skipMask);
							const U32 uB = (U32)b;
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += uB;
							pCurrentNode->m_IslandSize += uB;
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							pTwin = pTwin->m_pNext;
						}
					}
				}
			}
			pBody = pBody->m_pNext;
		}
#endif

		pBody = continuousBodies.m_pFirstBody;
		while(pBody)
		{
			//C_GraphNodeWrapper wrapper(m_GraphNodeArray[a]);
			//C_GraphNode* pCurrentNode = wrapper.m_pElement;
			C_GraphNode* pCurrentNode = pBody->GetGraphNode();//wrapper.m_pElement;
			//assert(pCurrentNode->m_IslandIndex == -1);
			
			if(!pCurrentNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation))
			{
				//Walk from this node to all other nodes connected to it via the constraint graph
				++islandIndex;
				const C_GraphNode* const pRootNode = pCurrentNode;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
				if(!pCurrentNode->HasConstraints())
					continue;
				C_GraphNode* __restrict pStackTop = pCurrentNode;
				S32 size = 1;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Root_Node);
				stack.Push(pCurrentNode, 0);

				//pNode->m_IslandIndex = islandIndex;

				while(size--)
				{
					C_GraphNode* pNode = pStackTop;//pStack[size];
					pNode->CalculateBVector(time);
					//if(size > 0)
					pStackTop = stack.Pop<C_GraphNode>(Maths::Max(size - 1, 0));
					
					//Loop through all this node's constraint edges to id the other 
					//bodies in this island...
					if(pNode->TestNodeFlags(C_GraphNode::E_Type_Active /*| C_GraphNode::E_Type_KinematicObject*/))
					{
						//const U32 origSize = size;
						C_ConstraintEdge* __restrict pEdge = pNode->m_pPersistentConstraints;
						while(pEdge)
						{
							C_GraphNode* __restrict pOtherNode = pEdge->m_pOutboundNode;
							stack.Push(pOtherNode, size);
							bool b = !pOtherNode->TestNodeFlags(skipMask);
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += U32(b);
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							//pNode->m_IslandIndex = islandIndex;
							pEdge = pEdge->m_pNext;
						}

						C_ConstraintTwin* __restrict pTwin = pNode->m_pPersistentTwinConstraints;
						while(pTwin)
						{
							pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
							C_GraphNode* __restrict pOtherNode = pEdge->m_pInboundNode;
							stack.Push(pOtherNode, size);
							bool b = !pOtherNode->TestNodeFlags(skipMask);
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += U32(b);
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							pTwin = pTwin->m_pNext;
						}

						pEdge = pNode->m_pVolatileConstraints;
						while(pEdge)
						{
							C_GraphNode* __restrict pOtherNode = pEdge->m_pOutboundNode;
							stack.Push(pOtherNode, size);
							bool b = !pOtherNode->TestNodeFlags(skipMask);
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += U32(b);
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							//pNode->m_IslandIndex = islandIndex;
							pEdge = pEdge->m_pNext;
						}
						
						pTwin = pNode->m_pVolatileTwinConstraints;
						while(pTwin)
						{
							pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
							C_GraphNode* __restrict pOtherNode = pEdge->m_pInboundNode;
							stack.Push(pOtherNode, size);
							bool b = !pOtherNode->TestNodeFlags(skipMask);
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += U32(b);
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							pTwin = pTwin->m_pNext;
						}
					}
				}
			}
			pBody = pBody->m_pNext;
		}

		//OK - loop through system and ensure we've tagged up all active bodies!!!!

	/*	for(S32 a = 0; a < m_GraphNodeArray.m_numElements; ++a)
		{
			C_GraphNodeWrapper wrapper(m_GraphNodeArray[a]);
			C_GraphNode* pCurrentNode = wrapper.m_pElement;
			assert((!pCurrentNode->TestNodeFlags(C_GraphNode::E_Type_Active)) || pCurrentNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation));
		}*/

		return islandIndex;
#else
		return 0;
#endif
	}

	S32 C_ConstraintGraphManager::SplitConstraintGroupsPostStabilise(C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.SplitConstraintGroupPostStabilise");
		//Clear visited flags, island index and root node  
		for(S32 a =0; a < m_GraphNodeArray.m_numElements; ++a)
		{
			C_GraphNodeWrapper wrapper(m_GraphNodeArray.m_pElems[a]);
			C_GraphNode* pNode = wrapper.m_pElement;
			pNode->ClearNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation | C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_Root_Node | C_GraphNode::E_Type_Visited_Wakeup | 
				C_GraphNode::E_Type_Root_Node);
			pNode->m_IslandIndex = Maths::Select(pNode->TestNodeFlags(C_GraphNode::E_Type_Sleeping_Or_Static), 0, -1);
		}

		//Now that everything is cleared, we need to traverse through the islands and see which node is 
		//part of which island

#if 1 //def GENERATE_ISLANDS
		const U32 skipMask = (C_GraphNode::E_Type_ZeroInertia | C_GraphNode::E_Type_Visited_Island_Generation);
		S32 islandIndex = 0;
		for(S32 a = 0; a < m_GraphNodeArray.m_numElements; ++a)
		{
			C_GraphNodeWrapper wrapper(m_GraphNodeArray[a]);
			C_GraphNode* __restrict pCurrentNode = wrapper.m_pElement;
			
			if(pCurrentNode->TestNodeFlags(C_GraphNode::E_Type_Active) && !pCurrentNode->TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation))
			{
				//Walk from this node to all other nodes connected to it via the constraint graph
				++islandIndex;
				const C_GraphNode* const pRootNode = pCurrentNode;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
				if(pCurrentNode->m_pPersistentConstraints == NULL && pCurrentNode->m_pPersistentTwinConstraints == NULL)
					continue;
				C_GraphNode* __restrict pStackTop = pCurrentNode;
				S32 size = 1;
				pCurrentNode->SetNodeFlags(C_GraphNode::E_Type_Root_Node);
				stack.Push(pCurrentNode, 0);

				//pNode->m_IslandIndex = islandIndex;

				while(size--)
				{
					C_GraphNode* pNode = pStackTop;//pStack[size];
					//if(size > 0)
					pStackTop = stack.Pop<C_GraphNode>(Maths::Max(size - 1, 0));
					
					//Loop through all this node's constraint edges to id the other 
					//bodies in this island...
					if(pNode->TestNodeFlags(C_GraphNode::E_Type_Active))
					{
						//const U32 origSize = size;
						C_ConstraintEdge* __restrict pEdge = pNode->m_pPersistentConstraints;
						while(pEdge)
						{
							C_GraphNode* __restrict pOtherNode = pEdge->m_pOutboundNode;
							stack.Push(pOtherNode, size);
							bool b = !pOtherNode->TestNodeFlags(skipMask);
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += U32(b);
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							//pNode->m_IslandIndex = islandIndex;
							pEdge = pEdge->m_pNext;
						}

						C_ConstraintTwin* __restrict pTwin = pNode->m_pPersistentTwinConstraints;
						while(pTwin)
						{
							pEdge = C_ConstraintEdge::GetConstraintEdge(pTwin);
							C_GraphNode* __restrict pOtherNode = pEdge->m_pInboundNode;
							stack.Push(pOtherNode, size);
							bool b = !pOtherNode->TestNodeFlags(skipMask);
							pStackTop = Fast_Select<C_GraphNode>(b, pOtherNode, pStackTop);
							size += U32(b);
							pOtherNode->SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Generation);
							pTwin = pTwin->m_pNext;
						}
					}
				}

				
			}
		}
		return islandIndex;
#else
		return 0;
#endif
	}



#ifdef USE_PERSISTENT_MANIFOLD

void C_ConstraintGraphManager::BuildConstraintGraph(SharedPhysics::C_PersistentContactCollider* pColliderArray, const U32 numContacts,
		SharedPhysics::C_MemoryIterator<SharedPhysics::C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false>& meshContacts, const U32 numMeshContacts)
	{
		PERF_EVENT("Gravity.BuildConstraintGraph");
	
		FlipBuffers();

		m_Edges.SetAllocator(m_pVolatileConstraintEdges);
		//m_Twins.SetAllocator(m_pVolatileTwinConstraints);
		m_Points.SetAllocator(m_pVolatileConstraintPoints);
		m_Constraints.SetAllocator(m_pContactConstraints);


		for(U32 a = 0; a < numContacts; ++a)
		{
			//C_Collider* pCollider(&pColliderArray[a]);
			C_PersistentContactCollider* pCollider(&pColliderArray[a]);
			pCollider->m_PersistentManifold.m_pResult = NULL;
			C_GraphNode* pNodeA = pCollider->NodeA();
			C_GraphNode* pNodeB = pCollider->NodeB();
			//if(pCollider->m_State != C_Collider::E_Removed)
			{
				C_SingleContactIterator iter(pCollider->GetIterator());
				if(iter.validResult())
				{
					C_ContactStreamResult* result = iter.result();

					pCollider->m_PersistentManifold.m_pResult = result;
					
					

				//	const bool bWarmStart = false;//!(pNodeA->TestNodeFlags(C_GraphNode::E_State_Changed) || pNodeB->TestNodeFlags(C_GraphNode::E_State_Changed));
			
					if(result->m_TotalContacts < 4)
					{
						const Maths::HMat33 invRotA(pNodeA->m_pRigidBody->m_OrientationMatrix.InverseRotation());
						const Maths::HMat33 invRotB(pNodeB->m_pRigidBody->m_OrientationMatrix.InverseRotation());
						while(iter.hasNextPatch())
						{
							C_ContactStreamPatch* pPatch = iter.nextPatch();
							const Maths::C_Normal norm(Maths::Load<Maths::HVec3>(&pPatch->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

							while(iter.hasNextPoint())
							{
								C_ContactStreamPoint* pPoint = iter.nextPoint();
								U32 index2 = pPoint->m_PhysicsMaterialIndices & 0xffff;
								U32 index1 = pPoint->m_PhysicsMaterialIndices >> 16;
								C_MaterialPair& pair(GetMaterialPair(index1, index2));
								Maths::HVec3 contactPoint = Maths::Load<Maths::HVec3>(&pPoint->m_ContactPoint);
								//Maths::HVec3 contactPointB = contactPointA - norm * pPoint->m_PenetrationDepth;
								Collision::C_ManifoldContact contact;
								
								contact.m_LocalContactA =  (contactPoint - pNodeA->m_pRigidBody->m_Position) * invRotA;
								contact.m_LocalContactB = (contactPoint - pNodeB->m_pRigidBody->m_Position) * invRotB;
								contact.m_Normal = norm;
								contact.m_HashValue = pPoint->m_HashValue;
								contact.m_PhysicsMaterialIndices = pPoint->m_PhysicsMaterialIndices;
								contact.m_PenetrationDepth = pPoint->m_PenetrationDepth;
								contact.m_Baumgarte = Maths::Max(Maths::Min(pNodeA->m_Baumgarte, pNodeB->m_Baumgarte), result->m_BaumgarteOverride);
								pCollider->m_PersistentManifold.AddManifoldContact(contact);				
							}
						}
					}
					else
					{
						pCollider->m_PersistentManifold.m_NumContacts = 0;
						C_ConstraintPoint* pConstraintPoints = pCollider->m_pConstraintEdge == NULL ?  NULL : pCollider->m_pConstraintEdge->m_pFirstConstraint;
						C_ConstraintEdge* pEdge = m_Edges.Create();
						pEdge->m_pNext = pEdge->m_pPrev = NULL;
						
						{
							C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
							SharedPhysics::LinkVolatile(pNodeA, pNodeB, pEdge, pTwin);
						}
						pCollider->m_pConstraintEdge = pEdge;
						pEdge->m_pCollider = pCollider;

						//const Maths::HVec3 a_to_b(pNodeB->m_pRigidBody->m_Position - pNodeA->m_pRigidBody->m_Position);
						pCollider->m_pConstraintEdge->ClearEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);

						
						pEdge->m_pFirstConstraint = NULL;
						pEdge->m_EdgeFlags = C_ConstraintEdge::E_Clear_Flags;
#if ENABLE_WARMSTART
						const bool bWarmStart = !(pNodeA->TestNodeFlags(C_GraphNode::E_State_Changed) || pNodeB->TestNodeFlags(C_GraphNode::E_State_Changed));
#endif
						while(iter.hasNextPatch())
						{
							C_ContactStreamPatch* pPatch = iter.nextPatch();
							const Maths::C_Normal norm(Maths::Load<Maths::HVec3>(&pPatch->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

							while(iter.hasNextPoint())
							{
								C_ContactStreamPoint* pPoint = iter.nextPoint();
								C_ConstraintPoint* pConstraintPoint = m_Points.Create();
								pConstraintPoint->m_HashValue = pPoint->m_HashValue;
								pPoint->m_pConstraint = pConstraintPoint;
								C_ContactConstraint* pConstraint(m_Constraints.Create());
								new (pConstraint) C_ContactConstraint();
								pConstraintPoint->m_pConstraint = pConstraint;
								pConstraint->m_ContactPoint = Maths::Load<Maths::HVec3>(&pPoint->m_ContactPoint);
								pConstraint->m_Normal = norm;
								//pConstraint->m_PenetrationDepth = Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth);
								U32 index2 = pPoint->m_PhysicsMaterialIndices & 0xffff;
								U32 index1 = pPoint->m_PhysicsMaterialIndices >> 16;
								C_MaterialPair& pair(GetMaterialPair(index1, index2));
								pConstraint->SetCoefficients(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth), 
									Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pair.m_Friction), Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
								pConstraint->SetBodyAMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectAMassScale));
								pConstraint->SetBodyBMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectBMassScale));
								pConstraint->SetBodyAInertiaScale(Maths::Load<Maths::HF32>(&result->m_ObjectAInertiaScale));//, result->m_ObjectAInertiaScaleY, result->m_ObjectAInertiaScaleZ));
								pConstraint->SetBodyBInertiaScale(Maths::Load<Maths::HF32>(&result->m_ObjectBInertiaScale));//, result->m_ObjectBInertiaScaleY, result->m_ObjectBInertiaScaleZ));
								pConstraint->ZeroWarmStartValues();
								pConstraint->SetBaumgarte(Maths::Max(Maths::Min(pNodeA->m_Baumgarte, pNodeB->m_Baumgarte), result->m_BaumgarteOverride));
#if ENABLE_WARMSTART
								if(bWarmStart)
								{
									C_ConstraintPoint* pOldPoint(pConstraintPoints);
									while(pOldPoint)
									{
										if(pOldPoint->m_HashValue == pConstraintPoint->m_HashValue)
										{
											pConstraint->StoreWarmstartValues(pOldPoint->m_pConstraint->m_WarmStartValues);
											break;
										}
										pOldPoint = pOldPoint->m_pNext;
									}
								}
#endif
								pEdge->AddConstraintPoint(pConstraintPoint);
							}
						}
					}
				}
				else
				{
					pCollider->m_PersistentManifold.m_NumContacts = 0;
					C_ConstraintEdge* pEdge = m_Edges.Create();
					pEdge->m_pNext = pEdge->m_pPrev = NULL;
					{
						//C_ConstraintTwin* pTwin = m_Twins.Create();
						C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
						SharedPhysics::LinkClose(pNodeA, pNodeB, pEdge, pTwin);
					}
					pCollider->m_pConstraintEdge = pEdge;
					pEdge->m_pCollider = pCollider;
					pCollider->m_Type = SharedPhysics::C_Collider::E_CloseCollision;
				}
			}
		}

		for(U32 a = 0; a < numContacts; ++a)
		{
			//C_Collider* pCollider(&pColliderArray[a]);

			C_PersistentContactCollider* pCollider(&pColliderArray[a]);

			if(pCollider->m_PersistentManifold.m_NumContacts > 0)
			{
				SharedPhysics::C_GraphNode* pNodeA = pCollider->m_pNodeA;
				SharedPhysics::C_GraphNode* pNodeB = pCollider->m_pNodeB;

				C_ConstraintPoint* pConstraintPoints = pCollider->m_pConstraintEdge == NULL ?  NULL : pCollider->m_pConstraintEdge->m_pFirstConstraint;
				C_ConstraintEdge* pEdge = m_Edges.Create();
				pEdge->m_pNext = pEdge->m_pPrev = NULL;
				
				{
					C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
					SharedPhysics::LinkVolatile(pNodeA, pNodeB, pEdge, pTwin);
				}
				pCollider->m_pConstraintEdge = pEdge;
				pEdge->m_pCollider = pCollider;

				//const Maths::HVec3 a_to_b(pNodeB->m_pRigidBody->m_Position - pNodeA->m_pRigidBody->m_Position);
				pCollider->m_pConstraintEdge->ClearEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);

				
				pEdge->m_pFirstConstraint = NULL;
				pEdge->m_EdgeFlags = C_ConstraintEdge::E_Clear_Flags;


				Collision::C_PersistentContactManifold& manifold = pCollider->m_PersistentManifold;

#if ENABLE_WARMSTART
				const bool bWarmStart = !(pNodeA->TestNodeFlags(C_GraphNode::E_State_Changed) || pNodeB->TestNodeFlags(C_GraphNode::E_State_Changed));
#endif

				for(U32 b=0; b< manifold.m_NumContacts; ++b)
				{
					Collision::C_ManifoldContact& pContact = manifold.m_Manifold[b];
					C_ConstraintPoint* pConstraintPoint = m_Points.Create();
					pConstraintPoint->m_HashValue = pContact.m_HashValue;
					pContact.m_pConstraint = pConstraintPoint;
					C_ContactConstraint* pConstraint(m_Constraints.Create());
					new (pConstraint) C_ContactConstraint();
					pConstraintPoint->m_pConstraint = pConstraint;
					//pConstraint->m_ContactPoint =pContact.m_WorldContactA;
					pConstraint->m_ContactPoint = pContact.m_LocalContactA * pCollider->m_pNodeA->m_pBody->GetShape()->m_PosRot;
					pConstraint->m_Normal = Maths::C_Normal(pContact.m_Normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);
					U32 index2 = pContact.m_PhysicsMaterialIndices & 0xffff;
					U32 index1 = pContact.m_PhysicsMaterialIndices >> 16;
					C_MaterialPair& pair(GetMaterialPair(index1, index2));
				
					pConstraint->SetCoefficients(Maths::Load<Maths::HF32>(&pContact.m_PenetrationDepth), 
						Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pair.m_Friction), Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
					pConstraint->SetBodyAMassScale(Maths::Load<Maths::HF32>(&manifold.m_pResult->m_ObjectAMassScale));
					pConstraint->SetBodyBMassScale(Maths::Load<Maths::HF32>(&manifold.m_pResult->m_ObjectBMassScale));
					pConstraint->SetBodyAInertiaScale(Maths::Load<Maths::HF32>(&manifold.m_pResult->m_ObjectAInertiaScale));//, result->m_ObjectAInertiaScaleY, result->m_ObjectAInertiaScaleZ));
					pConstraint->SetBodyBInertiaScale(Maths::Load<Maths::HF32>(&manifold.m_pResult->m_ObjectBInertiaScale));//, result->m_ObjectBInertiaScaleY, result->m_ObjectBInertiaScaleZ));
					pConstraint->ZeroWarmStartValues();
					pConstraint->SetBaumgarte(pContact.m_Baumgarte);

					/*F32 baumgarte = pContact.m_Baumgarte > 1.f ? pContact.m_Baumgarte: 1.f;
					pConstraint->SetBaumgarte(baumgarte);*/
					
#if ENABLE_WARMSTART
					if(bWarmStart)
					{
						C_ConstraintPoint* pOldPoint(pConstraintPoints);
						while(pOldPoint)
						{
							if(pOldPoint->m_HashValue == pConstraintPoint->m_HashValue)
							{
								pConstraint->StoreWarmstartValues(pOldPoint->m_pConstraint->m_WarmStartValues);
								break;
							}
							pOldPoint = pOldPoint->m_pNext;
						}
					}
#endif
					pCollider->m_pConstraintEdge->AddConstraintPoint(pConstraintPoint);	
				}
			}
		}
					
			

		for(U32 a = 0; a < numMeshContacts; ++a)
		{
			C_Collider* pCollider(meshContacts.Get());
			//if(pCollider->m_State != C_Collider::E_Removed)
			{
				C_SingleContactIterator iter(pCollider->GetIterator());
				if(iter.validResult())
				{
					C_ContactStreamResult* result = iter.result();
					
					C_GraphNode* pNodeA = pCollider->NodeA();
					C_GraphNode* pNodeB = pCollider->NodeB();

#if ENABLE_WARMSTART
					const bool bWarmStart = !(pNodeA->TestNodeFlags(C_GraphNode::E_State_Changed) || pNodeB->TestNodeFlags(C_GraphNode::E_State_Changed));
#endif
				
					C_ConstraintPoint* pConstraintPoints = pCollider->m_pConstraintEdge == NULL ?  NULL : pCollider->m_pConstraintEdge->m_pFirstConstraint;
					C_ConstraintEdge* pEdge = m_Edges.Create();
					pEdge->m_pNext = pEdge->m_pPrev = NULL;
					
					{
						//C_ConstraintTwin* pTwin = m_Twins.Create();
						C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
						SharedPhysics::LinkVolatile(pNodeA, pNodeB, pEdge, pTwin);
					}
					pCollider->m_pConstraintEdge = pEdge;
					pEdge->m_pCollider = pCollider;

					//const Maths::HVec3 a_to_b(pNodeB->m_pRigidBody->m_Position - pNodeA->m_pRigidBody->m_Position);
					pCollider->m_pConstraintEdge->ClearEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);

					
					pEdge->m_pFirstConstraint = NULL;
					pEdge->m_EdgeFlags = C_ConstraintEdge::E_Clear_Flags;
					while(iter.hasNextPatch())
					{
						C_ContactStreamPatch* pPatch = iter.nextPatch();
						const Maths::C_Normal norm(Maths::Load<Maths::HVec3>(&pPatch->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

						while(iter.hasNextPoint())
						{
							C_ContactStreamPoint* pPoint = iter.nextPoint();
							C_ConstraintPoint* pConstraintPoint = m_Points.Create();
							pConstraintPoint->m_HashValue = pPoint->m_HashValue;
							pPoint->m_pConstraint = pConstraintPoint;
							C_ContactConstraint* pConstraint(m_Constraints.Create());
							//Don't construct/destruct above, but do call constructor here (saves a destructor)
							new (pConstraint) C_ContactConstraint();
							pConstraintPoint->m_pConstraint = pConstraint;
							pConstraint->m_ContactPoint = Maths::Load<Maths::HVec3>(&pPoint->m_ContactPoint);
							pConstraint->m_Normal = norm;
							//pConstraint->m_PenetrationDepth = Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth);
							U32 index2 = pPoint->m_PhysicsMaterialIndices & 0xffff;
							U32 index1 = pPoint->m_PhysicsMaterialIndices >> 16;
							C_MaterialPair& pair(GetMaterialPair(index1, index2));
							/*pConstraint->m_FrictionCofficient = (Maths::Load<Maths::HF32>(&pair.m_Friction));
							pConstraint->m_Restitution = Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient);
							pConstraint->m_pContactPoint = pPoint;*/
							/*pConstraint->SetFriction(Maths::Load<Maths::HF32>(&pair.m_Friction));
							pConstraint->SetRestitution(Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
							pConstraint->SetPenetrationDepth(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth));*/
							pConstraint->SetCoefficients(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth), 
								Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pair.m_Friction), Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
							pConstraint->SetBodyAMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectAMassScale));
							pConstraint->SetBodyBMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectBMassScale));
							pConstraint->SetBodyAInertiaScale(Maths::HF32(Maths::Load<Maths::HF32>(&result->m_ObjectAInertiaScale)));//, result->m_ObjectAInertiaScaleY, result->m_ObjectAInertiaScaleZ));
							pConstraint->SetBodyBInertiaScale(Maths::HF32(Maths::Load<Maths::HF32>(&result->m_ObjectBInertiaScale)));//, result->m_ObjectBInertiaScaleY, result->m_ObjectBInertiaScaleZ));
							//Not needed!
							//pConstraint->ZeroWarmStartValues();
							pConstraint->SetBaumgarte(Maths::Max(Maths::Min(pNodeA->m_Baumgarte, pNodeB->m_Baumgarte), result->m_BaumgarteOverride));
							
#if ENABLE_WARMSTART
							if(bWarmStart)
							{
								C_ConstraintPoint* pOldPoint(pConstraintPoints);
								while(pOldPoint)
								{
									if(pOldPoint->m_HashValue == pConstraintPoint->m_HashValue)
									{
										pConstraint->StoreWarmstartValues(pOldPoint->m_pConstraint->m_WarmStartValues);
										break;
									}
									pOldPoint = pOldPoint->m_pNext;
								}
							}
#endif
							pEdge->AddConstraintPoint(pConstraintPoint);							
						}
					}
				}
			}
		}
	}

#else

	void C_ConstraintGraphManager::BuildConstraintGraph(SharedPhysics::C_PersistentContactCollider* pColliderArray, const U32 numContacts,
		SharedPhysics::C_MemoryIterator<SharedPhysics::C_Collider, ACTIVE_RESULT_BLOCK_SIZE, false>& meshContacts, const U32 numMeshContacts)
	{
		PERF_EVENT("Gravity.BuildConstraintGraph");
		//const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		//const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		//const Maths::HF32 fzero(Maths::Constants<Maths::HF32>::Zero());

		//TODO - need to push these back into the memory allocators as and when we don't need them anymore
		//m_pPrevVolatileConstraintPoints->Reset();
		//m_pPrevVolatileConstraintEdges->Reset();
		//m_pPrevVolatileTwinConstraints->Reset();
		//m_pPrevContactConstraints->Reset();
		FlipBuffers();

		/*SharedPhysics::C_MemoryIterator<SharedPhysics::C_ConstraintEdge, 128, true> edges(m_pVolatileConstraintEdges);
		SharedPhysics::C_MemoryIterator<SharedPhysics::C_ConstraintTwin, 128, true> twins(m_pVolatileTwinConstraints);
		SharedPhysics::C_MemoryIterator<SharedPhysics::C_ConstraintPoint, 256, true> points(m_pVolatileConstraintPoints);
		SharedPhysics::C_MemoryIterator<C_ContactConstraint, 256, true> constraints(m_pContactConstraints);*/

		m_Edges.SetAllocator(m_pVolatileConstraintEdges);
		//m_Twins.SetAllocator(m_pVolatileTwinConstraints);
		m_Points.SetAllocator(m_pVolatileConstraintPoints);
		m_Constraints.SetAllocator(m_pContactConstraints);


		////Do I still need to do this???...NO!
		//for(S32 a = 0; a < contacts.m_numElements; ++a)
		//{
		//	C_Collider* pCollider(contacts.m_pElems[a].m_pCollider);
		//	pCollider->m_pNodeA->m_pVolatileConstraints = NULL;
		//	pCollider->m_pNodeA->m_pVolatileTwinConstraints = NULL;

		//	pCollider->m_pNodeB->m_pVolatileConstraints = NULL;
		//	pCollider->m_pNodeB->m_pVolatileTwinConstraints = NULL;
		//}

		/*{
			PERF_EVENT("Gravity.BuildConstraintGraph1");
			for(S32 a = 0; a < m_GraphNodeArray.m_numElements; ++a)
			{
				C_GraphNode* pNode(m_GraphNodeArray[a].m_pElement);
				pNode->m_pVolatileConstraints = NULL;
				pNode->m_pVolatileTwinConstraints = NULL;
			}
		}*/
		//Loop through all contacts and build the relevant constraint graphs from them...
		

		/*for(S32 a = 0; a < contacts.m_numElements; ++a)
		{
			C_Collider* pCollider(contacts.m_pElems[a].m_pCollider);
			BIZ_ASSERT(pCollider->m_pNodeA->m_pVolatileConstraints == NULL);
			BIZ_ASSERT(pCollider->m_pNodeA->m_pVolatileTwinConstraints == NULL);
			BIZ_ASSERT(pCollider->m_pNodeB->m_pVolatileConstraints == NULL);
			BIZ_ASSERT(pCollider->m_pNodeB->m_pVolatileTwinConstraints == NULL);
			pCollider->m_pNodeA->m_pVolatileConstraints = NULL;
			pCollider->m_pNodeA->m_pVolatileTwinConstraints = NULL;
			pCollider->m_pNodeB->m_pVolatileConstraints = NULL;
			pCollider->m_pNodeB->m_pVolatileTwinConstraints = NULL;
		}*/

		for(U32 a = 0; a < numContacts; ++a)
		{
			C_PersistentContactCollider* pCollider(&pColliderArray[a]);

			
			//if(pCollider->m_State != C_Collider::E_Removed)
			{

				C_GraphNode* pNodeA = pCollider->NodeA();
				C_GraphNode* pNodeB = pCollider->NodeB(); 

				C_SingleContactIterator iter(pCollider->GetIterator());
				if(iter.validResult())
				{
					C_ContactStreamResult* result = iter.result();
					
#if ENABLE_WARMSTART
					const bool bWarmStart = !(pNodeA->TestNodeFlags(C_GraphNode::E_State_Changed) || pNodeB->TestNodeFlags(C_GraphNode::E_State_Changed));
#endif
				
					C_ConstraintPoint* pConstraintPoints = pCollider->m_pConstraintEdge == NULL ?  NULL : pCollider->m_pConstraintEdge->m_pFirstConstraint;
					C_ConstraintEdge* pEdge = m_Edges.Create();
					pEdge->m_pNext = pEdge->m_pPrev = NULL;
					
					{
						C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;//m_Twins.Create();
						SharedPhysics::LinkVolatile(pNodeA, pNodeB, pEdge, pTwin);
					}
					pCollider->m_pConstraintEdge = pEdge;
					pEdge->m_pCollider = pCollider;

					//const Maths::HVec3 a_to_b(pNodeB->m_pRigidBody->m_Position - pNodeA->m_pRigidBody->m_Position);
					pCollider->m_pConstraintEdge->ClearEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);

					
					pEdge->m_pFirstConstraint = NULL;
					pEdge->m_EdgeFlags = C_ConstraintEdge::E_Clear_Flags;
					while(iter.hasNextPatch())
					{
						C_ContactStreamPatch* pPatch = iter.nextPatch();
						const Maths::C_Normal norm(Maths::Load<Maths::HVec3>(&pPatch->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

						while(iter.hasNextPoint())
						{
							C_ContactStreamPoint* pPoint = iter.nextPoint();
							C_ConstraintPoint* pConstraintPoint = m_Points.Create();
							pConstraintPoint->m_HashValue = pPoint->m_HashValue;
							pPoint->m_pConstraint = pConstraintPoint;
							C_ContactConstraint* pConstraint(m_Constraints.Create());
							new (pConstraint) C_ContactConstraint();
							pConstraintPoint->m_pConstraint = pConstraint;
							pConstraint->m_ContactPoint = Maths::Load<Maths::HVec3>(&pPoint->m_ContactPoint);
							pConstraint->m_Normal = norm;
							//pConstraint->m_PenetrationDepth = Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth);
							U32 index2 = pPoint->m_PhysicsMaterialIndices & 0xffff;
							U32 index1 = pPoint->m_PhysicsMaterialIndices >> 16;
							C_MaterialPair& pair(GetMaterialPair(index1, index2));
							/*pConstraint->m_FrictionCofficient = (Maths::Load<Maths::HF32>(&pair.m_Friction));
							pConstraint->m_Restitution = Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient);
							pConstraint->m_pContactPoint = pPoint;*/
							/*pConstraint->SetFriction(Maths::Load<Maths::HF32>(&pair.m_Friction));
							pConstraint->SetRestitution(Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
							pConstraint->SetPenetrationDepth(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth));*/
							pConstraint->SetCoefficients(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth), 
								Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pair.m_Friction), Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
							pConstraint->SetBodyAMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectAMassScale));
							pConstraint->SetBodyBMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectBMassScale));
							pConstraint->SetBodyAInertiaScale(Maths::Load<Maths::HF32>(&result->m_ObjectAInertiaScale));//, result->m_ObjectAInertiaScaleY, result->m_ObjectAInertiaScaleZ));
							pConstraint->SetBodyBInertiaScale(Maths::Load<Maths::HF32>(&result->m_ObjectBInertiaScale));//, result->m_ObjectBInertiaScaleY, result->m_ObjectBInertiaScaleZ));
							pConstraint->ZeroWarmStartValues();
							pConstraint->SetBaumgarte(Maths::Max(Maths::Min(pNodeA->m_Baumgarte, pNodeB->m_Baumgarte), result->m_BaumgarteOverride));
							
#if ENABLE_WARMSTART
							if(bWarmStart)
							{
								C_ConstraintPoint* pOldPoint(pConstraintPoints);
								while(pOldPoint)
								{
									if(pOldPoint->m_HashValue == pConstraintPoint->m_HashValue)
									{
										pConstraint->StoreWarmstartValues(pOldPoint->m_pConstraint->m_WarmStartValues);
										break;
									}
									pOldPoint = pOldPoint->m_pNext;
								}
							}
#endif
							pEdge->AddConstraintPoint(pConstraintPoint);							
						}
					}
				}
				else
				{
					C_ConstraintEdge* pEdge = m_Edges.Create();
					pEdge->m_pNext = pEdge->m_pPrev = NULL;
					{
						//C_ConstraintTwin* pTwin = m_Twins.Create();
						C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
						SharedPhysics::LinkClose(pNodeA, pNodeB, pEdge, pTwin);
					}
					pCollider->m_pConstraintEdge = pEdge;
					pEdge->m_pCollider = pCollider;
					pCollider->m_Type = SharedPhysics::C_Collider::E_CloseCollision;
				}
			}
		}

		for(U32 a = 0; a < numMeshContacts; ++a)
		{
			C_Collider* pCollider(meshContacts.Get());
			//if(pCollider->m_State != C_Collider::E_Removed)
			{
				

				C_SingleContactIterator iter(pCollider->GetIterator());
				if(iter.validResult())
				{
					C_ContactStreamResult* result = iter.result();
					C_GraphNode* pNodeA = pCollider->NodeA();
					C_GraphNode* pNodeB = pCollider->NodeB();
					
#if ENABLE_WARMSTART
					const bool bWarmStart = !(pNodeA->TestNodeFlags(C_GraphNode::E_State_Changed) || pNodeB->TestNodeFlags(C_GraphNode::E_State_Changed));
#endif
				
					C_ConstraintPoint* pConstraintPoints = pCollider->m_pConstraintEdge == NULL ?  NULL : pCollider->m_pConstraintEdge->m_pFirstConstraint;
					C_ConstraintEdge* pEdge = m_Edges.Create();
					pEdge->m_pNext = pEdge->m_pPrev = NULL;
					
					{
						//C_ConstraintTwin* pTwin = m_Twins.Create();
						C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
						SharedPhysics::LinkVolatile(pNodeA, pNodeB, pEdge, pTwin);
					}
					pCollider->m_pConstraintEdge = pEdge;
					pEdge->m_pCollider = pCollider;

					//const Maths::HVec3 a_to_b(pNodeB->m_pRigidBody->m_Position - pNodeA->m_pRigidBody->m_Position);
					pCollider->m_pConstraintEdge->ClearEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);

					
					pEdge->m_pFirstConstraint = NULL;
					pEdge->m_EdgeFlags = C_ConstraintEdge::E_Clear_Flags;
					while(iter.hasNextPatch())
					{
						C_ContactStreamPatch* pPatch = iter.nextPatch();
						const Maths::C_Normal norm(Maths::Load<Maths::HVec3>(&pPatch->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

						while(iter.hasNextPoint())
						{
							C_ContactStreamPoint* pPoint = iter.nextPoint();
							C_ConstraintPoint* pConstraintPoint = m_Points.Create();
							pConstraintPoint->m_HashValue = pPoint->m_HashValue;
							pPoint->m_pConstraint = pConstraintPoint;
							C_ContactConstraint* pConstraint(m_Constraints.Create());
							new (pConstraint) C_ContactConstraint();
							pConstraintPoint->m_pConstraint = pConstraint;
							pConstraint->m_ContactPoint = Maths::Load<Maths::HVec3>(&pPoint->m_ContactPoint);
							pConstraint->m_Normal = norm;
							//pConstraint->m_PenetrationDepth = Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth);
							U32 index2 = pPoint->m_PhysicsMaterialIndices & 0xffff;
							U32 index1 = pPoint->m_PhysicsMaterialIndices >> 16;
							C_MaterialPair& pair(GetMaterialPair(index1, index2));
							/*pConstraint->m_FrictionCofficient = (Maths::Load<Maths::HF32>(&pair.m_Friction));
							pConstraint->m_Restitution = Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient);
							pConstraint->m_pContactPoint = pPoint;*/
							/*pConstraint->SetFriction(Maths::Load<Maths::HF32>(&pair.m_Friction));
							pConstraint->SetRestitution(Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
							pConstraint->SetPenetrationDepth(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth));*/
							pConstraint->SetCoefficients(Maths::Load<Maths::HF32>(&pPoint->m_PenetrationDepth), 
								Maths::Load<Maths::HVec3>((MathsStorage::Float3*)&pair.m_Friction), Maths::Load<Maths::HF32>(&pair.m_RestitutionCoefficient));
							pConstraint->SetBodyAMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectAMassScale));
							pConstraint->SetBodyBMassScale(Maths::Load<Maths::HF32>(&result->m_ObjectBMassScale));
							pConstraint->SetBodyAInertiaScale(Maths::Load<Maths::HF32>(&result->m_ObjectAInertiaScale));//, result->m_ObjectAInertiaScaleY, result->m_ObjectAInertiaScaleZ));
							pConstraint->SetBodyBInertiaScale(Maths::Load<Maths::HF32>(&result->m_ObjectBInertiaScale));//, result->m_ObjectBInertiaScaleY, result->m_ObjectBInertiaScaleZ));
							pConstraint->ZeroWarmStartValues();
							pConstraint->SetBaumgarte(Maths::Max(Maths::Min(pNodeA->m_Baumgarte, pNodeB->m_Baumgarte), result->m_BaumgarteOverride));
							
#if ENABLE_WARMSTART
							if(bWarmStart)
							{
								C_ConstraintPoint* pOldPoint(pConstraintPoints);
								while(pOldPoint)
								{
									if(pOldPoint->m_HashValue == pConstraintPoint->m_HashValue)
									{
										pConstraint->StoreWarmstartValues(pOldPoint->m_pConstraint->m_WarmStartValues);
										break;
									}
									pOldPoint = pOldPoint->m_pNext;
								}
							}
#endif
							pEdge->AddConstraintPoint(pConstraintPoint);							
						}
					}
				}
			}
		}

	}
	
#endif

	void C_ConstraintGraphManager::BuildClothConstraintGraph(SharedPhysics::C_ContactStream* pClothStream)
	{
		SharedPhysics::C_ParticleContactStreamIterator iter(pClothStream);
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		while(iter.HasNextResult())
		{
			SharedPhysics::C_ParticleContactPair* pPair = iter.NextResult();

			if(!pPair->m_pNodePtr->TestNodeFlags(SharedPhysics::C_GraphNode::E_OneWayClothCollision | SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Static))
			{
				C_ConstraintEdge* pEdge = m_Edges.Create();
				pEdge->m_pNext = pEdge->m_pPrev = NULL;
				
				{
					C_ConstraintTwin* pTwin = &pEdge->m_TwinConstraint;
					SharedPhysics::LinkVolatile(pPair->m_pParticlePtr, pPair->m_pNodePtr, pEdge, pTwin);
				}

				while(iter.HasNextContact())
				{
					SharedPhysics::C_ParticleContactStorage* pContact = iter.NextContact();
					const Maths::HVec3 contact(Maths::Load<Maths::HVec3>(&pContact->m_Contact));
					const Maths::HF32 pen(Maths::Load<Maths::HF32>(&pContact->m_Penetration));
					const Maths::C_Normal norm(-Maths::Load<Maths::HVec3>(&pContact->m_EscapeVector), Maths::C_Normal::UNIT_LENGTH_VECTOR);
					const Maths::C_Normal triangleNormal(Maths::Load<Maths::HVec3>(&pContact->m_TriangleNormal), Maths::C_Normal::UNIT_LENGTH_VECTOR);
					const Maths::HF32 stiffness(Maths::Load<Maths::HF32>(&pContact->m_Stiffness));

		
					C_ConstraintPoint* pConstraintPoint = m_Points.Create();
					pConstraintPoint->m_HashValue = 0;
					//pContact.m_pConstraint = pConstraintPoint;
					C_ContactConstraint* pConstraint(m_Constraints.Create());
					new (pConstraint) C_ContactConstraint();
					pConstraintPoint->m_pConstraint = pConstraint;
					pConstraint->m_ContactPoint = contact;
					pConstraint->m_Normal = norm;
					//U32 index2 = 0;
					//U32 index1 = 0;
					//C_MaterialPair& pair(GetMaterialPair(index1, index2));
				
					pConstraint->SetCoefficients(pen, Maths::HVec3(0.15f), Maths::HF32(0.f));
					pConstraint->SetBodyAMassScale(one);
					pConstraint->SetBodyBMassScale(one);
					pConstraint->SetBodyAInertiaScale(one);//, result->m_ObjectAInertiaScaleY, result->m_ObjectAInertiaScaleZ));
					pConstraint->SetBodyBInertiaScale(one);//, result->m_ObjectBInertiaScaleY, result->m_ObjectBInertiaScaleZ));
					pConstraint->ZeroWarmStartValues();
					pConstraint->SetBaumgarte(0.8f);	
					pEdge->AddConstraintPoint(pConstraintPoint);
					//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contact, 0.05f, Maths::RGBA(0, 255, 0));
				}
			}
		}
	}

	void C_ConstraintGraphManager::ClearLeftoverContacts(SharedPhysics::C_ContactDatabase & contactDatabase,
		Gravity::C_BodyList& wakingList)
	{
		PERF_EVENT("Gravity.ClearLeftOverContacts");
		using namespace SharedPhysics;
		const Maths::HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
		//Loop through and find all contacts which are left-over in the contact database
		//These contacts have ended, so must be removed (and all the constraints/graph edges/constraint points
		//must be freed. Additionally, if either of these objects are asleep, they must be woken up because they
		//may be floating in mid-air...

		C_ActiveContactManager* prevContacts = contactDatabase.m_pPrevBuffer;
		U32 numColliders = prevContacts->NumColliders();

		for(U32 a = 0; a < numColliders; ++a)
		{
			SharedPhysics::C_Collider* pCollider = prevContacts->GetCollider(a);
			//Wake up any sleeping objects pointed to by this collider!

			//Flag it as waking. This informs the next stage to grab its contacts from the sleepy part of the 
			//database!
			//NB - only nodeB will be sleeping, because nodeA will be active! Contacts between 2 sleepy objects
			//or sleepy and static will not be present in the active part of the contact database
			if(pCollider->m_State == C_Collider::E_Normal) 
			{
				if(pCollider->m_pNodeA->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) && 
					pCollider->m_pNodeA->TestNodeFlags(C_GraphNode::E_Type_Sleeping) && 
					(!pCollider->m_pNodeB->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) || 
					!pCollider->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static)))
				{
					if(!pCollider->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
					{
						wakingList.AddBody(pCollider->m_pNodeA->m_pBody);
						pCollider->m_pNodeA->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
						pCollider->m_pNodeA->m_TotalKineticEnergy = zero;
						pCollider->m_pNodeA->m_pRigidBody->m_KineticEnergy = zero;
					}
					pCollider->m_pNodeA->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Waking);
				}
				//pCollider->m_pNodeA->m_Deactivator.Activate();
				else if(pCollider->m_pNodeB->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) && 
					pCollider->m_pNodeB->TestNodeFlags(C_GraphNode::E_Type_Sleeping) && 
					(!pCollider->m_pNodeA->m_pBody->TestBodyFlags(Gravity::C_Body::E_InsertedIntoWorld) || 
					!pCollider->m_pNodeA->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static)))
				{
					if(!pCollider->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered))
					{
						wakingList.AddBody(pCollider->m_pNodeB->m_pBody);
						pCollider->m_pNodeB->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Interfered);
						pCollider->m_pNodeB->m_TotalKineticEnergy = zero;
						pCollider->m_pNodeB->m_pRigidBody->m_KineticEnergy = zero;
						pCollider->m_pNodeB->SetNodeFlags(C_GraphNode::E_Type_Waking);
					}
					pCollider->m_pNodeB->SetNodeFlags(SharedPhysics::C_GraphNode::E_Type_Waking);
				}
			}

			/*C_ConstraintEdge * pEdge = pCollider->m_pConstraintEdge;
			if(!pEdge)
				continue;
			C_ConstraintTwin* pTwin = pCollider->m_pConstraintEdge->m_pTwinConstraint;
			//Remove the edge from the linked list...
			Unlink(pEdge->m_pInboundNode, pEdge->m_pOutboundNode, pEdge, pTwin);
			//Now that the nodes are unlinked, we need to clean up the constraint, free all its members and 
			//free the collider. All these components are allocated in blocks and handled by allocation managers
			//so we just need to pass them to be freed in the right place

			
			C_ConstraintPoint* pConstraintPoint = pEdge->m_pFirstConstraint;
			C_ConstraintPoint* pCurrentConstraintPoint = pEdge->m_pFirstConstraint;
			while(pConstraintPoint)
			{
				pConstraintPoint = pConstraintPoint->m_pNext;
				m_ContactConstraints.PushToFree((C_ContactConstraint*)pCurrentConstraintPoint->m_pConstraint);
				m_VolatileConstraintPoints.PushToFree(pCurrentConstraintPoint);
				pCurrentConstraintPoint = pConstraintPoint;
			}
			m_VolatileConstraintEdges.PushToFree(pEdge);
			m_VolatileTwinConstraints.PushToFree(pTwin);
			pCollider->m_pConstraintEdge = NULL;*/
			
			//Don't need to free the collider, the colliders will be reset when we flip buffers!
			//prevContacts->FreeCollider(pCollider);
		}
	} 

	void C_ConstraintGraphManager::PutToSleep(SharedPhysics::C_GraphNode* pNode, SharedPhysics::C_ContactDatabase& contactDatabase)
	{
		PERF_EVENT("Gravity.PutToSleep");
		using namespace SharedPhysics;
		//C_Collider tempCollider;
		//tempCollider.m_IndexA = pNode->m_Index;
		//tempCollider.m_IndexB = 0;
		//S32 index = contactDatabase.m_pPrevBuffer->FindFirstIndex(pNode);
		//const U32 numContacts = contactDatabase.m_pPrevBuffer->NumColliders();
		//for(U32 a = index; a < numContacts; ++a)
		//{
		//	C_Collider* pCollider = contactDatabase.m_pPrevBuffer->Collider(a);
		//	if( pCollider->m_IndexA != pNode->m_Index )
		//		break;
		//	
		//	/*SharedPhysics::C_GraphNode* pOtherNode(Fast_Select<SharedPhysics::C_GraphNode>(pCollider->NodeA() == pNode, pCollider->NodeA(), pCollider->NodeB()));
		//	if(pOtherNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) && 
		//		pCollider->m_State != C_Collider::E_Removed)
		//	{
		//		//contactDatabase.MakeSleepyContact(pCollider);
		//		//pCollider->m_Removed = true;
		//		//Free the constraint + collider
		//	}*/
		//	pCollider->m_State = C_Collider::E_Removed;
		//}

		//index = contactDatabase.m_pPrevBuffer->FindFirstIndexRv(pNode);
		//for(U32 a = index; a < numContacts; ++a)
		//{
		//	C_Collider* pCollider = contactDatabase.m_pPrevBuffer->RvCollider(a);
		//	if( pCollider->m_IndexB != pNode->m_Index )
		//		break;

		//	pCollider->m_State = C_Collider::E_Removed;
		//}

		//Go through and remove all constraints from this

		SharedPhysics::C_ConstraintEdge* pEdge = pNode->m_pVolatileConstraints;
		while(pEdge)
		{
			if(pEdge->m_pCollider)/* && pEdge->m_pOutboundNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) && 
				pEdge->m_pCollider->m_State != C_Collider::E_Removed)*/
			{
				pEdge->m_pCollider->m_State = C_Collider::E_Removed;
			}
			pEdge = pEdge->m_pNext;
		}

		SharedPhysics::C_ConstraintTwin* pTwin = pNode->m_pVolatileTwinConstraints;
		while(pTwin)
		{
			//pEdge = pTwin->m_pEdge;
			pEdge = SharedPhysics::C_ConstraintEdge::GetConstraintEdge(pTwin);
			if(pEdge->m_pCollider)/* && pEdge->m_pInboundNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static) && 
				pEdge->m_pCollider->m_State != C_Collider::E_Removed)*/
			{
				pEdge->m_pCollider->m_State = C_Collider::E_Removed;
			}
			pTwin = pTwin->m_pNext;
		}
	}


};
