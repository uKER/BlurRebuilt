#ifndef __GRAVITY_OBJECT_H__
#define __GRAVITY_OBJECT_H__

#include "gravity/broadphase/Object.h"
#include "gravity/broadPhase/distributionTree.h"
#include "gravity/narrowphase/CollisionManager.h"

namespace Gravity
{

NOALIAS inline void C_Object::PutToSleep() __restrict
{
	if(m_ObjectState == E_FluidMeshObject)
		return;
	if(this->m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth | SharedPhysics::C_GraphNode::E_Type_Rope))
	{
		m_ObjectState = E_SleepingClothObject;
		for(U32 a =0 ; a < m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
			pWrapper->m_pNode->Remove(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			pWrapper->m_pNode->AddToSleepingCloth(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
		}
		
	}
	else if (m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris))
	{
		m_ObjectState = E_SleepingDebrisObject;
		for(U32 a =0 ; a < m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
			pWrapper->m_pNode->Remove(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			pWrapper->m_pNode->AddToSleepingDebris(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
		}
	}
	else
	{
		m_ObjectState = E_SleepingObject;
		for(U32 a =0 ; a < m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
			pWrapper->m_pNode->Remove(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			pWrapper->m_pNode->AddToSleeping(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
		}
		
	}

}

NOALIAS inline Collision::C_CollisionObject C_Object::CollisionObject(const Maths::HF32 PCREF time)
{
	return Collision::C_CollisionObject(m_pShape, m_Index, m_TransformedBox, m_pGraphNode, time);
}

NOALIAS inline Collision::C_CollisionObject C_Object::CollisionObject()
{
	return Collision::C_CollisionObject(m_pShape, m_Index, m_TransformedBox, m_pGraphNode);
}

NOALIAS inline void C_Object::Transform(const Maths::HMat43 PCREF matrix) __restrict
{
	Collision::C_CollisionManager::TransformShapeAndBox(m_pShape, matrix, m_TransformedBox);
#if 0
	for(U32 a = 0; a < this->m_NumNodesContained; ++a)
	{
		m_Nodes[a].m_SAPObject.UpdateBox();
	}
#endif
}

NOALIAS inline void C_Object::WakeUp(C_DistributionTree* pTree) __restrict
{
	if(m_ObjectState == E_FluidMeshObject)
		return;
	if(m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Cloth | SharedPhysics::C_GraphNode::E_Type_Rope))
	{
		m_ObjectState = E_ActiveClothObject;
		for(U32 a =0 ; a < m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
			pWrapper->m_pNode->Remove(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			pWrapper->m_pNode->AddToActiveCloth(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			if(!pWrapper->m_pNode->IsActive() && pWrapper->m_pNode->MustActivate())
			{
				pTree->AddToActive(pWrapper->m_pNode);
			}
		}
		
	}
	else if (m_pGraphNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Debris))
	{
		m_ObjectState = E_ActiveDebrisObject;
		for(U32 a =0 ; a < m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
		
			pWrapper->m_pNode->Remove(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			pWrapper->m_pNode->AddToActiveDebris(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			if(!pWrapper->m_pNode->IsActive() && pWrapper->m_pNode->MustActivate())
			{
				pTree->AddToActive(pWrapper->m_pNode);
			}
		}
	}
	else
	{
		m_ObjectState = E_ActiveObject;
		for(U32 a =0 ; a < m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
		
			pWrapper->m_pNode->Remove(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			pWrapper->m_pNode->AddToActive(pWrapper);
			BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
			if(!pWrapper->m_pNode->IsActive() && pWrapper->m_pNode->MustActivate())
			{
				pTree->AddToActive(pWrapper->m_pNode);
			}
		}
	}
}

NOALIAS inline bool C_Object::ContainedInNode(C_DistributionNode* pNode)
{
	//return ((m_ObjectState != E_KinematicObject && m_ObjectState != E_StaticObject && 
	//	m_ObjectState != E_ActiveFluidObject && m_ObjectState != E_SleepingFluidObject 
	//	&& m_ObjectState != E_FluidMeshObject)
	//	|| m_TransformedBox.IsSmaller(pNode->m_bbox.m_min, pNode->m_bbox.m_max));
	return m_TransformedBox.IsSmaller(pNode->m_bbox.m_min, pNode->m_bbox.m_max);
	/*const Maths::HVec3 min(m_TransformedBox.centre - m_TransformedBox.extents);
	const Maths::HVec3 max(m_TransformedBox.centre + m_TransformedBox.extents);
	return pNode->m_bbox.m_min.All_LE(min) && pNode->m_bbox.m_max.All_GT(max);*/
}

NOALIAS inline void C_Object::MakeStatic() __restrict
{
	if(m_ObjectState == E_FluidMeshObject)
		return;
	m_ObjectState = E_StaticObject;
	for(U32 a =0 ; a < m_NumNodesContained; ++a)
	{
		C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
		pWrapper->m_pNode->Remove(pWrapper);
		BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
		pWrapper->m_pNode->AddToStatic(pWrapper);
		BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
	}	
}

NOALIAS inline void C_Object::MakeRaytraceOnly() __restrict
{
	if(m_ObjectState == E_RaytraceOnlyObject)
		return;
	m_ObjectState = E_RaytraceOnlyObject;
	for(U32 a =0 ; a < m_NumNodesContained; ++a)
	{
		C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
		pWrapper->m_pNode->Remove(pWrapper);
		BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
		pWrapper->m_pNode->AddToRaytraceOnly(pWrapper);
		BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
	}	
}

NOALIAS inline void C_Object::MakeKinematic()
{
	if(m_ObjectState == E_FluidMeshObject)
		return;
	m_ObjectState = E_KinematicObject;
	for(U32 a =0 ; a < m_NumNodesContained; ++a)
	{
		C_DistributionNodeWrapper* pWrapper(&m_Nodes[a]);
		pWrapper->m_pNode->Remove(pWrapper);
		BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
		pWrapper->m_pNode->AddToKinematic(pWrapper);
		BIZ_ASSERT(pWrapper->m_pNode->ValidNodeLists());
	}
	
	m_pGraphNode->m_pRigidBody->m_KineticEnergy = Maths::Constants<Maths::HVec3>::Four() * Maths::Constants<Maths::HVec3>::Four();
}

NOALIAS inline void C_Object::RemoveFrom(const U32 index) __restrict
{
	//Remove from the box with index "index"

	m_Nodes[index].m_pNode->Remove(&m_Nodes[index]);
	C_ObjectWrapper dummy;
	//Now need to shuffle down all the previous ones...

	for(U32 a = index + 1; a < m_NumNodesContained; ++a)
	{
		m_Nodes[a-1].m_pNode = m_Nodes[a].m_pNode;
		m_Nodes[a-1].m_pNext = m_Nodes[a].m_pNext;
		m_Nodes[a-1].m_pPrev = m_Nodes[a].m_pPrev;
		C_ObjectWrapper* prev(SharedPhysics::Fast_Select<C_ObjectWrapper>(m_Nodes[a].m_pPrev == NULL, &dummy, m_Nodes[a].m_pPrev));
		prev->m_pNext = &m_Nodes[a-1];
		C_ObjectWrapper* next(SharedPhysics::Fast_Select<C_ObjectWrapper>(m_Nodes[a].m_pNext == NULL, &dummy, m_Nodes[a].m_pNext));
		next->m_pPrev = &m_Nodes[a-1];

		BIZ_ASSERT((m_Nodes[a].m_pObject->m_ObjectState & E_Dirty) == 0);
		//Test to see if I need to put this wrapper at the head of the linked list/remove it from the previous one...
		m_Nodes[a].m_pNode->m_pObjectArrays[m_Nodes[a].m_pObject->m_ObjectState] = SharedPhysics::Fast_Select<C_ObjectWrapper>(m_Nodes[a].m_pPrev == NULL, &m_Nodes[a-1], m_Nodes[a].m_pNode->m_pObjectArrays[m_Nodes[a].m_pObject->m_ObjectState]);
	}
	--m_NumNodesContained;
	//printf("Removed from a node!!!\n");
}

//NOALIAS inline bool C_Object::IsUpdateRequired() __restrict
//{
//	//return (!m_NodeBox.Contains(m_TransformedBox));
//	if((m_ObjectState & E_Dirty) != 0)
//	{
//		m_ObjectState &= ~E_Dirty;
//		return true;
//	}
//	if(((m_ObjectState ==  E_SleepingObject) || 
//		(m_ObjectState == E_StaticObject) ||
//		(m_ObjectState == E_SleepingClothObject) || 
//		(m_ObjectState == E_SleepingFluidObject)))
//		return false;
//
//	//assert(m_ObjectState != E_KinematicObject);
//
//	
//	if(m_NodeBox.Contains(m_TransformedBox))
//	{
//		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon2());
//		const Maths::HF32 volume(m_TransformedBox.Volume());
//		Maths::HF32 intersectVolume(Maths::Constants<Maths::HF32>::Zero());
//		Collision::C_AxisBoxMinMax box;
//		box.SetInvalidExtents();
//		//Loop through all the contained nodes to see if there are any which this aabb is no longer inside...
//		for(U32 a = 0; a < m_NumNodesContained; ++a)
//		{
//			if(!m_Nodes[a].m_pNode->m_bbox.Intersects(m_TransformedBox))
//			{
//				RemoveFrom(a);
//				--a;
//			}
//			else
//			{
//				intersectVolume += m_TransformedBox.IntersectionVolume(m_Nodes[a].m_pNode->m_bbox.m_min, 
//					m_Nodes[a].m_pNode->m_bbox.m_max);
//				box.m_min = Maths::Min(box.m_min, m_Nodes[a].m_pNode->m_bbox.m_min);
//				box.m_max = Maths::Max(box.m_max, m_Nodes[a].m_pNode->m_bbox.m_max);
//			}
//		}
//		m_NodeBox.m_min = box.m_min;
//		m_NodeBox.m_max = box.m_max;
//		
//		return (volume - intersectVolume) <= epsilon;
//	}
//	return true;
//}

};

#endif
