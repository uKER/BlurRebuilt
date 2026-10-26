#include "StdAfx.h"
#include "gravity/broadphase/DistributionTree.h"
//#include "gravity/broadphase/DistributionTree.inl"
#include "gravity/broadphase/Object.h"
#include "gravity/broadphase/Object.inl"
//#include "gravity/narrowphase/PrimitiveTest.h"
//#include "gravity/narrowphase/PrimitiveTest.inl"
#include "gravity/sharedphysics/AllocationManager.inl"
#include "gravity/sharedphysics/MemoryAllocator.inl"
#include "gravity/sharedphysics/SafeStack.h"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/AxisBox.inl"
#include "gravity/sdk/Body.h"
#include <system/perf.h>
#include "system/debug/I_DebugRender.h"


namespace Gravity
{

//#pragma optimize("", off)
//#define CHECK_BPHASE
//#define SHOW_UPDATE_OBJECTS

	Debug::Bool g_dbDrawPotentialClosestPoints("Gravity.Debug.DrawPotentialClosestPoints", false);
	Debug::Bool g_dbDrawClosestPoints("Gravity.Debug.DrawClosestPoints", false);
	Debug::Bool g_dbDrawComparedBodies("Gravity.Debug.DrawComparedBodies", false);
	Debug::Bool g_dbDrawEnclosingBox("Gravity.Debug.DrawEnclosingBox", false);
	Debug::Bool g_dbDisableSplit("Gravity.DisableSplit", false);
	Debug::Bool g_dbDisableMerge("Gravity.DisableMerge", false);
	Debug::Bool g_dbForceUpdate("Gravity.ForceBroadPhaseUpdate", false);

	//using namespace Collision;
	U32 C_DistributionNode::m_NodeCount = 0;

	bool C_Object::IsUpdateRequired() __restrict
	{
		//return (!m_NodeBox.Contains(m_TransformedBox));
		if(((m_ObjectState & E_Dirty) != 0) | (m_NumNodesContained == 0))
		{
			m_ObjectState &= ~E_Dirty;
			return true;
		}
		if((m_ObjectState ==  E_SleepingObject) | 
			(m_ObjectState == E_StaticObject) |
			(m_ObjectState == E_SleepingClothObject) |
			(m_ObjectState == E_SleepingFluidObject) |
			 (m_ObjectState == E_FluidMeshObject))
			return false;

		//assert(m_ObjectState != E_KinematicObject);

		
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
				if(!m_Nodes[a].m_pNode->m_bbox.Intersects(m_TransformedBox))
				{
					RemoveFrom(a);
					--a;
				}
				else
				{
					intersectVolume += m_TransformedBox.IntersectionVolume(m_Nodes[a].m_pNode->m_bbox.m_min, 
						m_Nodes[a].m_pNode->m_bbox.m_max);
					box.m_min = Maths::Min(box.m_min, m_Nodes[a].m_pNode->m_bbox.m_min);
					box.m_max = Maths::Max(box.m_max, m_Nodes[a].m_pNode->m_bbox.m_max);
				}
			}
			m_NodeBox.m_min = box.m_min;
			m_NodeBox.m_max = box.m_max;
			
			return (volume - intersectVolume) >= epsilon;
		}
		return true;
	}


	void C_DistributionTree::AddToActive(C_DistributionNode* pNode)
	{
		BIZ_ASSERT(this);
		BIZ_ASSERT(pNode->m_State != E_DNS_Active);

		if(pNode == m_pPrevActiveNodes)
		{
			m_pPrevActiveNodes = pNode->m_pNext;
		}
		else if(pNode == m_pPrevSleepingNodes)
		{
			m_pPrevSleepingNodes = pNode->m_pNext;
		}

		//Unlink from existing list!!!
		C_DistributionNode* next(SharedPhysics::Fast_Select<C_DistributionNode>(pNode->m_pNext != NULL, pNode->m_pNext, &m_DummyNode));
		C_DistributionNode* prev(SharedPhysics::Fast_Select<C_DistributionNode>(pNode->m_pPrev != NULL, pNode->m_pPrev, &m_DummyNode));
		next->m_pPrev = pNode->m_pPrev;
		prev->m_pNext = pNode->m_pNext;

		BIZ_ASSERT(next->m_pPrev != next);
		BIZ_ASSERT(prev->m_pNext != prev);

		m_pSleepingNodes = SharedPhysics::Fast_Select<C_DistributionNode>(pNode == m_pSleepingNodes, pNode->m_pNext, m_pSleepingNodes);

		//Now add to the list!
		pNode->m_State = E_DNS_Active;
		pNode->m_pNext = m_pActiveNodes;
		C_DistributionNode* actives = SharedPhysics::Fast_Select<C_DistributionNode>(m_pActiveNodes != NULL, m_pActiveNodes, &m_DummyNode);
		actives->m_pPrev = pNode;
		pNode->m_pPrev = NULL;
		m_pActiveNodes = pNode;
	}

	void C_DistributionTree::AddToSleeping(C_DistributionNode* pNode)
	{
		BIZ_ASSERT(pNode->m_State != E_DNS_Sleeping);

		if(pNode == m_pPrevActiveNodes)
		{
			m_pPrevActiveNodes = pNode->m_pNext;
		}
		else if(pNode == m_pPrevSleepingNodes)
		{
			m_pPrevSleepingNodes = pNode->m_pNext;
		}


		//Unlink from existing list!!!
		C_DistributionNode* next(SharedPhysics::Fast_Select<C_DistributionNode>(pNode->m_pNext != NULL, pNode->m_pNext, &m_DummyNode));
		C_DistributionNode* prev(SharedPhysics::Fast_Select<C_DistributionNode>(pNode->m_pPrev != NULL, pNode->m_pPrev, &m_DummyNode));
		next->m_pPrev = pNode->m_pPrev;
		prev->m_pNext = pNode->m_pNext;

		BIZ_ASSERT(next->m_pPrev != next);
		BIZ_ASSERT(prev->m_pNext != prev);

		m_pActiveNodes = SharedPhysics::Fast_Select<C_DistributionNode>(pNode == m_pActiveNodes, pNode->m_pNext, m_pActiveNodes);

		pNode->m_State = E_DNS_Sleeping;
		pNode->m_pNext = m_pSleepingNodes;
		C_DistributionNode* sleepies = SharedPhysics::Fast_Select<C_DistributionNode>(m_pSleepingNodes != NULL, m_pSleepingNodes, &m_DummyNode);
		sleepies->m_pPrev = pNode;
		pNode->m_pPrev = NULL;
		m_pSleepingNodes = pNode;
	}

	void C_DistributionNode::Subdivide(const Maths::HVec3 PCREF desiredSize, const S32 maxDepth, 
									   C_DistributionNodeAllocator& memoryAllocator,
									   C_LeafNodeList& leafNodes)
	{
		using namespace Maths;

		++m_NodeCount;
		m_pActiveObjects = m_pSleepingObjects = m_pStaticObjects = m_pKinematicObjects = 
			m_pActiveDebrisObjects = m_pSleepingDebrisObjects = m_pFluidMeshObjects = 
			m_pActiveFluidObjects = m_pSleepingFluidObjects = m_pActiveClothObjects =
			m_pSleepingClothObjects = NULL;
		m_pLeftNode = m_pRightNode = NULL;
		m_pNext = NULL;
		m_State = E_DNS_Inactive;
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


	C_DistributionNode* C_DistributionNode::Split(C_DistributionNodeAllocator& memoryAllocator, C_LeafNodeIterator& leafNodes, Maths::HVec3 PCREF cellSize, C_DistributionNode* pRoot, 
		C_DistributionTree& tree)
	{
		using namespace Maths;

		const Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HVec3 twoCellSize(cellSize * two);
		Maths::HVec3 doubleExtents(m_bbox.m_max - m_bbox.m_min);
		if(g_dbDisableSplit || (twoCellSize).All_GE(doubleExtents))
		{
			return pRoot;
		}

		m_NodeCount += 2;

		const Maths::HVec3 ratio(doubleExtents/cellSize);

		//Need to look at the bounding boxes of the objects in the cell and split accordingly!
		//First pick fattest axis, and then sort objects by their extents and find the mid-point to split them...

		const HF32 half(Maths::Constants<HF32>::Half());
		const HVec3 _min(m_bbox.m_min);
		const HVec3 _max(m_bbox.m_max);
		const HF32 doubleX(doubleExtents.X());
		const HF32 doubleY(doubleExtents.Y());
		const HF32 doubleZ(doubleExtents.Z());

		const HVec3 ex(doubleExtents * half);
		const HF32 x(ex.X());
		const HF32 y(ex.Y());
		const HF32 z(ex.Z());

		HVec3 rightStart(_min);

		const HVec3 rightEnd(rightStart + ex);

		const Scalar_Bool con1(ratio.Y().GT(ratio.X()) & ratio.Y().GT(ratio.Z()));
		const Scalar_Bool con2(ratio.Z().GT(ratio.X()));

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

		//OK - now I need to remove push all this node's objects into it's children nodes
		//I know this node has objects, otherwise we wouldn't split it!

		C_ObjectWrapper* pWrapper(m_pActiveObjects);
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pSleepingObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pStaticObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		//assert(m_pKinematicObjects == NULL);

		pWrapper = m_pKinematicObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pActiveClothObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pSleepingClothObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pActiveFluidObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pSleepingFluidObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pFluidMeshObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRaytraceOnlyObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}
		
		pWrapper = m_pActiveDebrisObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pSleepingDebrisObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}


		leafNodes.AddNode(m_pLeftNode);
		leafNodes.AddNode(m_pRightNode);
		leafNodes.Remove(this);

		m_pLeftNode->m_State = E_DNS_Inactive;
		m_pRightNode->m_State = E_DNS_Inactive;

		m_pActiveObjects = m_pSleepingObjects = m_pStaticObjects = m_pKinematicObjects = 
			m_pActiveClothObjects = m_pSleepingClothObjects = m_pActiveFluidObjects = m_pSleepingFluidObjects = m_pFluidMeshObjects =
			m_pRaytraceOnlyObjects = m_pActiveDebrisObjects = m_pSleepingDebrisObjects = NULL;
		m_pLeftNode->m_pActiveObjects = m_pLeftNode->m_pSleepingObjects = m_pLeftNode->m_pStaticObjects = m_pLeftNode->m_pKinematicObjects =
			m_pLeftNode->m_pActiveClothObjects = m_pLeftNode->m_pSleepingClothObjects = 
			m_pLeftNode->m_pSleepingFluidObjects = m_pLeftNode->m_pActiveFluidObjects = m_pLeftNode->m_pFluidMeshObjects = 
			m_pLeftNode->m_pRaytraceOnlyObjects = m_pLeftNode->m_pActiveDebrisObjects = m_pLeftNode->m_pSleepingDebrisObjects = NULL;
		m_pRightNode->m_pActiveObjects = m_pRightNode->m_pSleepingObjects = m_pRightNode->m_pStaticObjects = m_pRightNode->m_pKinematicObjects =
			m_pRightNode->m_pActiveClothObjects = m_pRightNode->m_pSleepingClothObjects = 
			m_pRightNode->m_pSleepingFluidObjects = m_pRightNode->m_pActiveFluidObjects = 
			m_pRightNode->m_pFluidMeshObjects = m_pRightNode->m_pRaytraceOnlyObjects = 
			m_pRightNode->m_pActiveDebrisObjects = m_pRightNode->m_pSleepingDebrisObjects = NULL;

		m_ObjectCount = 0;

	/*	if(pRoot!=NULL && pRoot == pRoot->m_pNext)
		{
			assert(0);
		}*/

		//if(this == pRoot)
		//	return pRoot->m_pNext;

		/*if(pRoot!=NULL && pRoot == pRoot->m_pNext)
		{
			assert(0);
		}*/
		return pRoot;
	}

	C_DistributionNode* C_DistributionNode::Merge(C_DistributionNodeAllocator& memoryAllocator, C_LeafNodeIterator& leafNodes, LeafState& state,
		const Maths::HVec3 PCREF maxCellSize, C_DistributionTree& tree)
	{
		if(g_dbDisableMerge || maxCellSize.All_LE(m_bbox.m_max - m_bbox.m_min))
			return state.m_pActiveRoot;
		//return state.m_pActiveRoot;
		m_NodeCount -= 2;
		
		//This node's children need to be zipped together...
		//Problem - need to make sure that we only add a given object once...
		//First, add all the objects in the left child to this node - easy....
		C_ObjectWrapper* pWrapper(m_pLeftNode->m_pActiveObjects);
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pSleepingObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pStaticObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pKinematicObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pActiveClothObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pSleepingClothObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pActiveFluidObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pSleepingFluidObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pFluidMeshObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pRaytraceOnlyObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pActiveDebrisObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pLeftNode->m_pSleepingDebrisObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pActiveObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pSleepingObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pStaticObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pKinematicObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pActiveClothObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pSleepingClothObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pActiveFluidObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper-&pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pSleepingFluidObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pFluidMeshObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pRaytraceOnlyObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pActiveDebrisObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		pWrapper = m_pRightNode->m_pSleepingDebrisObjects;
		while(pWrapper)
		{
			C_Object* pObj = pWrapper->m_pObject;
			//pObj->RemoveFrom(pWrapper - &pObj->m_Nodes[0]);
			pObj->MarkDirty();
			pWrapper->bDirty = true;
			tree.AddDirtyObject(pObj);
			pWrapper = pWrapper->m_pNext;
		}

		C_DistributionNode* pLeftNode(m_pLeftNode), *pRightNode(m_pRightNode);
		C_DistributionNode* pNextNode(SharedPhysics::Fast_Select<C_DistributionNode>(m_pLeftNode == state.m_pActiveRoot, m_pLeftNode->m_pNext, m_pRightNode->m_pNext));
		//In case it was left node, then right node or right node, then left node...
		pNextNode = SharedPhysics::Fast_Select<C_DistributionNode>(pNextNode == m_pLeftNode, m_pLeftNode->m_pNext, SharedPhysics::Fast_Select<C_DistributionNode>(pNextNode == m_pRightNode, m_pRightNode->m_pNext, pNextNode));
		C_DistributionNode* pNextSleepingNode(SharedPhysics::Fast_Select<C_DistributionNode>(m_pLeftNode == state.m_pSleepingRoot, m_pLeftNode->m_pNext, m_pRightNode->m_pNext));
		pNextSleepingNode = SharedPhysics::Fast_Select<C_DistributionNode>(m_pLeftNode == state.m_pSleepingRoot, m_pLeftNode->m_pNext, m_pRightNode->m_pNext);

		if(state.m_pSleepingRoot == m_pLeftNode || state.m_pSleepingRoot == m_pRightNode)
		{
			state.m_pSleepingRoot = state.m_pSleepingRoot->m_pNext;
			if(state.m_pSleepingRoot == m_pLeftNode || state.m_pSleepingRoot == m_pRightNode)
			{
				state.m_pSleepingRoot = state.m_pSleepingRoot->m_pNext;
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
			C_DistributionNode* nextNode(SharedPhysics::Fast_Select<C_DistributionNode>(m_pLeftNode->m_pNext != NULL, m_pLeftNode->m_pNext, m_pLeftNode));
			C_DistributionNode* prevNode(SharedPhysics::Fast_Select<C_DistributionNode>(m_pLeftNode->m_pPrev != NULL, m_pLeftNode->m_pPrev, m_pLeftNode));
			nextNode->m_pPrev = m_pLeftNode->m_pPrev;
			prevNode->m_pNext = m_pLeftNode->m_pNext;
			m_pLeftNode->m_pNext = m_pLeftNode->m_pPrev = NULL;
		}

		//if(m_pRightNode->IsActive())
		{
			//OK - it was active, so let's remove it from the active list
			C_DistributionNode* nextNode(SharedPhysics::Fast_Select<C_DistributionNode>(m_pRightNode->m_pNext != NULL, m_pRightNode->m_pNext, m_pRightNode));
			C_DistributionNode* prevNode(SharedPhysics::Fast_Select<C_DistributionNode>(m_pRightNode->m_pPrev != NULL, m_pRightNode->m_pPrev, m_pRightNode));
			nextNode->m_pPrev = m_pRightNode->m_pPrev;
			prevNode->m_pNext = m_pRightNode->m_pNext;
			m_pRightNode->m_pNext = m_pRightNode->m_pPrev = NULL;
		}

		m_pLeftNode->m_State = E_DNS_Inactive;
		m_pRightNode->m_State = E_DNS_Inactive;

		m_pLeftNode->m_pActiveObjects = m_pLeftNode->m_pSleepingObjects = m_pLeftNode->m_pStaticObjects = 
			m_pLeftNode->m_pKinematicObjects = m_pLeftNode->m_pActiveClothObjects = 
			m_pLeftNode->m_pSleepingClothObjects = m_pLeftNode->m_pSleepingFluidObjects = 
			m_pLeftNode->m_pActiveFluidObjects = m_pLeftNode->m_pFluidMeshObjects = m_pLeftNode->m_pRaytraceOnlyObjects =	
			m_pLeftNode->m_pActiveDebrisObjects = m_pLeftNode->m_pSleepingDebrisObjects = 
			m_pRightNode->m_pActiveObjects = m_pRightNode->m_pSleepingObjects = m_pRightNode->m_pStaticObjects = 
			m_pRightNode->m_pKinematicObjects = m_pRightNode->m_pActiveClothObjects = 
			m_pRightNode->m_pSleepingClothObjects = m_pRightNode->m_pSleepingFluidObjects = 
			m_pRightNode->m_pActiveFluidObjects = m_pRightNode->m_pFluidMeshObjects = 
			m_pRightNode->m_pRaytraceOnlyObjects = m_pRightNode->m_pActiveDebrisObjects = 
			m_pRightNode->m_pSleepingDebrisObjects = NULL;

		m_pLeftNode->m_ObjectCount = 0;
		m_pRightNode->m_ObjectCount = 0;
		
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


	
	U32 C_DistributionTree::CompareObjectToList(SharedPhysics::C_MemoryIterator<C_ObjectPair, 128>& objectIter, Gravity::C_Object* pObject, 
		Gravity::C_ObjectWrapper* pList, Collision::C_AxisBoxMinMax& box, const U32& numContacts)
	{
#if 0
		Maths::HMat43 identity;
		identity.SetIdentity();
#endif
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half()); 
		U32 count = numContacts;
		C_ObjectWrapper* pWrapper=pList;
		while(pWrapper)
		{
			//PERF_EVENT("Gravity.BroadPhase_CompareObject");
			C_Object* pOtherObj=pWrapper->m_pObject;

#if 0
			if(g_dbDrawComparedBodies)
			{
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(pObject->m_TransformedBox.centre - pObject->m_TransformedBox.extents, 
					pObject->m_TransformedBox.centre + pObject->m_TransformedBox.extents, identity, Maths::RGBA(255, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(pOtherObj->m_TransformedBox.centre - pOtherObj->m_TransformedBox.extents, 
					pOtherObj->m_TransformedBox.centre + pOtherObj->m_TransformedBox.extents, identity, Maths::RGBA(255, 0, 0));
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(pObject->m_TransformedBox.centre, 
					pOtherObj->m_TransformedBox.centre, Maths::RGBA(255, 0, 0));


			}
#endif

			//if(pObject->m_TransformedBox.Intersects(pOtherObj->m_TransformedBox) && 
			if(pObject->m_TransformedBox.IntersectsInAxisBox(pOtherObj->m_TransformedBox, box) &&
				pObject->m_pShape->CanCollide(pOtherObj->m_pShape))
			{
				/*const Maths::HVec3 closestA(pObject->m_TransformedBox.ClosestPoint(pOtherObj->m_TransformedBox.centre));
				const Maths::HVec3 closestB(pOtherObj->m_TransformedBox.ClosestPoint(pObject->m_TransformedBox.centre));
				
				const Maths::HVec3 p((closestA + closestB)*half);*/
#if 0
				if(g_dbDrawPotentialClosestPoints)
				{
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(box.m_min, box.m_max, identity, Maths::RGBA(0, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(p, 0.1f, Maths::RGBA(0, 0, 255));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.1f, Maths::RGBA(255, 0, 0));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.1f, Maths::RGBA(0, 255, 0));
					DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 0, 0));
				}
#endif
				//if(box.Contains(p))
				{
#if 0
					if(g_dbDrawClosestPoints)
					{
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestA, 0.1f, Maths::RGBA(255, 0, 0));
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(closestB, 0.1f, Maths::RGBA(0, 255, 0));
						DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(closestA, closestB, Maths::RGBA(255, 0, 0));
					}
#endif
						//C_ObjectPair* pair=pObjPair.Create(index++);
					C_ObjectPair* pair(objectIter.Create());
					++count;
					pair->m_pObjectA=pObject;
					pair->m_pObjectB=pOtherObj;
				}
			}
			pWrapper=pWrapper->m_pNext;
		}
		return count;
	}


	Debug::Bool g_dbDrawOverlappingBoxes("Gravity.DrawOverlappingBoxes", false);
	U32 C_DistributionTree::CollidedPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPair, SharedPhysics::C_SafeStack<true>& stack)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.BroadPhase");
		using namespace Maths;
		

		U32 count = 0;
		SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> iter(&pObjPair);
		const HF32 half(Constants<HF32>::Half());
		C_DistributionNode* pNode=m_pActiveNodes;
		
		while(pNode)
		{
			C_AxisBoxMinMax& box=pNode->m_bbox;
			C_ObjectWrapper* pActiveWrapper=pNode->m_pActiveObjects;
			while(pActiveWrapper)
			{
				C_Object* pActiveObj=pActiveWrapper->m_pObject;
				count = CompareObjectToList(iter, pActiveObj, pNode->m_pSleepingObjects, box, count);
				count = CompareObjectToList(iter, pActiveObj, pNode->m_pKinematicObjects, box, count);
				count = CompareObjectToList(iter, pActiveObj, pNode->m_pStaticObjects, box, count);
				count = CompareObjectToList(iter, pActiveObj, pActiveWrapper->m_pNext, box, count);
				pActiveWrapper=pActiveWrapper->m_pNext;
			}

			
			if(pNode->m_pKinematicObjects)
			{
				C_ObjectWrapper* pWrapper(pNode->m_pSleepingObjects);
				while(pWrapper)
				{
					C_Object* pSleepyObj(pWrapper->m_pObject);
					count = CompareObjectToList(iter, pSleepyObj, pNode->m_pKinematicObjects, box, count);
					pWrapper = pWrapper->m_pNext;
				}
			}

			C_DistributionNode* pParent(pNode->m_pParentNode);
			while(pParent)
			{
				C_ObjectWrapper* pParentStatics(pParent->m_pStaticObjects);
				//while(pParentStatics)
				if(pParent->m_pStaticObjects || pParent->m_pKinematicObjects || pParent->m_pSleepingObjects || 
					pParent->m_pActiveObjects)
				{
					C_ObjectWrapper* pActives(pNode->m_pActiveObjects);
					while(pActives)
					{
						C_Object* pActiveObject(pActives->m_pObject);
						count = CompareObjectToList(iter, pActiveObject, pParent->m_pStaticObjects, box, count);
						count = CompareObjectToList(iter, pActiveObject, pParent->m_pKinematicObjects, box, count);
						count = CompareObjectToList(iter, pActiveObject, pParent->m_pActiveObjects, box, count);
						count = CompareObjectToList(iter, pActiveObject, pParent->m_pSleepingObjects, box, count);
						pActives = pActives->m_pNext;
					}

					if(pParent->m_pKinematicObjects)
					{
						C_ObjectWrapper* pSleepy(pNode->m_pSleepingObjects);
						while(pSleepy)
						{
							C_Object* pSleepyObject(pSleepy->m_pObject);
							count = CompareObjectToList(iter, pSleepyObject, pParent->m_pKinematicObjects, box, count);
							pSleepy = pSleepy->m_pNext;
						}
					}


					if(pNode->m_pStaticObjects || pNode->m_pKinematicObjects || pNode->m_pSleepingObjects)
					{
						C_ObjectWrapper* pParentActive(pParent->m_pActiveObjects);
						while(pParentActive)
						{
							C_Object* pActiveObj(pParentActive->m_pObject);
							count = CompareObjectToList(iter, pActiveObj, pNode->m_pSleepingObjects, box, count);
							count = CompareObjectToList(iter, pActiveObj, pNode->m_pKinematicObjects, box, count);
							count = CompareObjectToList(iter, pActiveObj, pNode->m_pStaticObjects, box, count);
							pParentActive = pParentActive->m_pNext;
						}

						if(pNode->m_pKinematicObjects)
						{
							C_ObjectWrapper* pParentSleeping(pParent->m_pSleepingObjects);
							while(pParentSleeping)
							{
								C_Object* pSleepyObject(pParentSleeping->m_pObject);
								count = CompareObjectToList(iter, pSleepyObject, pNode->m_pKinematicObjects, box, count);
								pParentSleeping = pParentSleeping->m_pNext;
							}
						}
					}
				}
				pParent = pParent->m_pParentNode;
			}

			pNode=pNode->m_pNext;
		}

		C_DistributionNode* pSleepingNodes(m_pSleepingNodes);
		while(pSleepingNodes)
		{
			C_DistributionNode* pParent(pSleepingNodes->m_pParentNode);
			while(pParent)
			{
				if(pParent->m_pKinematicObjects || pParent->m_pActiveObjects)
				{
					C_ObjectWrapper* pSleepy(pSleepingNodes->m_pSleepingObjects);
					while(pSleepy)
					{
						C_Object* pSleepyObject(pSleepy->m_pObject);
						count = CompareObjectToList(iter, pSleepyObject, pParent->m_pKinematicObjects, pSleepingNodes->m_bbox, count);
						count = CompareObjectToList(iter, pSleepyObject, pParent->m_pActiveObjects, pSleepingNodes->m_bbox, count);
						pSleepy = pSleepy->m_pNext;
					}
				}

				if(pSleepingNodes->m_pStaticObjects)
				{
					C_ObjectWrapper* pActive(pParent->m_pActiveObjects);
					while(pActive)
					{
						C_Object* pActiveObject(pActive->m_pObject);
						count = CompareObjectToList(iter, pActiveObject, pSleepingNodes->m_pStaticObjects, pSleepingNodes->m_bbox, count);
						pActive = pActive->m_pNext;
					}
				}

				pParent = pParent->m_pParentNode;
			}
			pSleepingNodes = pSleepingNodes->m_pNext;
		}

		return count;
	}

	U32 C_DistributionTree::ClothPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack)
	{
		using namespace Collision;
		using namespace Maths;
		U32 count = 0;
		SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> iter(&pObjPairs);
		const HF32 half(Constants<HF32>::Half());
		C_DistributionNode* pNode=m_pActiveNodes;
		
		while(pNode)
		{
			C_AxisBoxMinMax& box=pNode->m_bbox;
			if(pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects)
			{
				C_ObjectWrapper* pWrapper(pNode->m_pActiveObjects);
				while(pWrapper)
				{
					C_Object* pObj = pWrapper->m_pObject;
					count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
					count = CompareObjectToList(iter, pObj, pNode->m_pSleepingClothObjects, box, count);
					pWrapper = pWrapper->m_pNext;
				}

				pWrapper = pNode->m_pKinematicObjects;
				while(pWrapper)
				{
					C_Object* pObj = pWrapper->m_pObject;
					count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
					count = CompareObjectToList(iter, pObj, pNode->m_pSleepingClothObjects, box, count);
					pWrapper = pWrapper->m_pNext;
				}

				if(pNode->m_pActiveClothObjects)
				{
					pWrapper = pNode->m_pSleepingObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}

					pWrapper = pNode->m_pStaticObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}
				}
			}
			//Then walk up the tree!
			//Go from here to parent nodes!

			C_DistributionNode* pParent = pNode->m_pParentNode;
			while(pParent)
			{
				//Compare pNode's cloth with parents dynamics!
				if(pNode->m_pActiveClothObjects)
				{
					C_ObjectWrapper* pWrapper = pParent->m_pStaticObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}
					pWrapper = pParent->m_pSleepingObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}
				}

				if(pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects)
				{
					C_ObjectWrapper* pWrapper = pParent->m_pKinematicObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
						count = CompareObjectToList(iter, pObj, pNode->m_pSleepingClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}

					pWrapper = pParent->m_pActiveObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pActiveClothObjects, box, count);
						count = CompareObjectToList(iter, pObj, pNode->m_pSleepingClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}
				}

				//Compare pParent's cloth with pNode's dynamics
				if(pParent->m_pActiveClothObjects || pParent->m_pSleepingClothObjects)
				{
					C_ObjectWrapper* pWrapper = pNode->m_pActiveObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pParent->m_pActiveClothObjects, box, count);
						count = CompareObjectToList(iter, pObj, pParent->m_pSleepingClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}

					pWrapper = pNode->m_pKinematicObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pParent->m_pActiveClothObjects, box, count);
						count = CompareObjectToList(iter, pObj, pParent->m_pSleepingClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}
				}
				if(pParent->m_pActiveClothObjects)
				{
					C_ObjectWrapper* pWrapper = pNode->m_pStaticObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pParent->m_pActiveClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}

					pWrapper = pNode->m_pSleepingObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pParent->m_pActiveClothObjects, box, count);
						pWrapper = pWrapper->m_pNext;
					}
				}

				pParent = pParent->m_pParentNode;
			}
			pNode = pNode->m_pNext;
		}

		pNode = m_pSleepingNodes;
		while(pNode)
		{
			//Compare sleeping node with kinematics and actives!
			C_AxisBoxMinMax& box=pNode->m_bbox;
			C_DistributionNode* pParent = pNode->m_pParentNode;
			while(pParent)
			{
				if(pNode->m_pSleepingClothObjects)
				{
					C_ObjectWrapper* pWrapper = pParent->m_pKinematicObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pSleepingClothObjects, pNode->m_bbox, count);
						pWrapper = pWrapper->m_pNext;
					}

					pWrapper = pParent->m_pActiveObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pNode->m_pSleepingClothObjects, pNode->m_bbox, count);
						pWrapper = pWrapper->m_pNext;
					}
				}
				if(pParent->m_pActiveClothObjects)
				{
					C_ObjectWrapper* pWrapper = pNode->m_pSleepingObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pParent->m_pActiveClothObjects, pNode->m_bbox, count);
						pWrapper = pWrapper->m_pNext;
					}

					pWrapper = pNode->m_pStaticObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pParent->m_pActiveClothObjects, pNode->m_bbox, count);
						pWrapper = pWrapper->m_pNext;
					}
				}
				pParent = pParent->m_pParentNode;
			}
			pNode = pNode->m_pNext;
		}
		return count;
	}

	//Spit out collision pairs with fluid mesh!!!
	U32 C_DistributionTree::FluidPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack)
	{
		U32 count = 0;
		SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> iter(&pObjPairs);

		C_DistributionNode* pNode=m_pActiveNodes;

		while(pNode)
		{

			if(pNode->m_pFluidMeshObjects)
			{
				//Loop through all the fluid mesh objects and compare them 
				C_ObjectWrapper* pWrapper = pNode->m_pActiveObjects;
				while(pWrapper)
				{
					C_Object* pObj = pWrapper->m_pObject;
					count = CompareObjectToList(iter, pObj, pNode->m_pFluidMeshObjects, pNode->m_bbox, count);
					pWrapper = pWrapper->m_pNext;
				}
			}

			C_DistributionNode* pParent = pNode->m_pParentNode;
			while(pParent)
			{
				if(pParent->m_pFluidMeshObjects)
				{
					C_ObjectWrapper* pWrapper = pNode->m_pActiveObjects;
					while(pWrapper)
					{
						C_Object* pObj = pWrapper->m_pObject;
						count = CompareObjectToList(iter, pObj, pParent->m_pFluidMeshObjects, pNode->m_bbox, count);
						pWrapper = pWrapper->m_pNext;
					}
				}
				pParent = pParent->m_pParentNode;
			}

			pNode = pNode->m_pNext;
		}

		return count;
	}

	void C_DistributionTree::CombinedPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, 
		U32& numPairs, SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pClothPairs, U32& numClothPairs,
		SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pFluidPairs, U32& numFluidPairs, SharedPhysics::C_SafeStack<true>& stack)
	{
		using namespace Collision;
		PERF_EVENT("Gravity.BroadPhase_Combined");
		using namespace Maths;
		

		U32 count = 0, clothCount = 0, fluidCount = 0;
		SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> iter(&pObjPairs);
		SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> fluidIter(&pFluidPairs);
		SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> clothIter(&pClothPairs);
		C_DistributionNode* pNode=m_pActiveNodes;
		
		while(pNode)
		{
			C_AxisBoxMinMax& box=pNode->m_bbox;
			C_ObjectWrapper* pActiveWrapper=pNode->m_pActiveObjects;
			{
				//PERF_EVENT("Gravity.BroadPhase_ActiveNodeComparison");
				while(pActiveWrapper)
				{
					C_Object* pActiveObj=pActiveWrapper->m_pObject;
					//Dynamic object comparisons
					count = CompareObjectToList(iter, pActiveObj, pNode->m_pSleepingObjects, box, count);
					count = CompareObjectToList(iter, pActiveObj, pNode->m_pKinematicObjects, box, count);
					count = CompareObjectToList(iter, pActiveObj, pNode->m_pStaticObjects, box, count);
					count = CompareObjectToList(iter, pActiveObj, pActiveWrapper->m_pNext, box, count);

					//Cloth comparisons
					clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pActiveClothObjects, box, clothCount);
					clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pSleepingClothObjects, box, clothCount);

					//Fluid comparisons
					fluidCount = CompareObjectToList(fluidIter, pActiveObj, pNode->m_pFluidMeshObjects, box, fluidCount);
					pActiveWrapper=pActiveWrapper->m_pNext;
				}

				
				if(pNode->m_pKinematicObjects || pNode->m_pActiveClothObjects)
				{
					C_ObjectWrapper* pWrapper(pNode->m_pSleepingObjects);
					while(pWrapper)
					{
						C_Object* pSleepyObj(pWrapper->m_pObject);
						count = CompareObjectToList(iter, pSleepyObj, pNode->m_pKinematicObjects, box, count);

						//Cloth comparisons
						clothCount = CompareObjectToList(clothIter, pSleepyObj, pNode->m_pActiveClothObjects, box, clothCount);
						
						pWrapper = pWrapper->m_pNext;
					}
				}

				if(pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects)
				{
					C_ObjectWrapper* pWrapper(pNode->m_pKinematicObjects);
					while(pWrapper)
					{
						C_Object* pKinematicObj(pWrapper->m_pObject);
						clothCount = CompareObjectToList(clothIter, pKinematicObj, pNode->m_pActiveClothObjects, box, clothCount);
						clothCount = CompareObjectToList(clothIter, pKinematicObj, pNode->m_pSleepingClothObjects, box, clothCount);
						pWrapper = pWrapper->m_pNext;
					}
				}

				if(pNode->m_pStaticObjects)
				{
					C_ObjectWrapper* pWrapper(pNode->m_pStaticObjects);
					while(pWrapper)
					{
						C_Object* pStaticObj(pWrapper->m_pObject);
						clothCount = CompareObjectToList(clothIter, pStaticObj, pNode->m_pActiveClothObjects, box, clothCount);
						pWrapper = pWrapper->m_pNext;
					}
				}
			}

			{
				//PERF_EVENT("Gravity.BroadPhase_ActiveParentComparison");
				C_DistributionNode* pParent(pNode->m_pParentNode);
				while(pParent)
				{
					C_ObjectWrapper* pParentStatics(pParent->m_pStaticObjects);
					//while(pParentStatics)
					if(pParent->m_pStaticObjects || pParent->m_pKinematicObjects || pParent->m_pSleepingObjects || 
						pParent->m_pActiveObjects || pParent->m_pActiveClothObjects || pParent->m_pSleepingClothObjects || 
						pParent->m_pFluidMeshObjects)
					{
						C_ObjectWrapper* pActives(pNode->m_pActiveObjects);
						while(pActives)
						{
							C_Object* pActiveObject(pActives->m_pObject);
							//Dynamic comparisons
							count = CompareObjectToList(iter, pActiveObject, pParent->m_pStaticObjects, box, count);
							count = CompareObjectToList(iter, pActiveObject, pParent->m_pKinematicObjects, box, count);
							count = CompareObjectToList(iter, pActiveObject, pParent->m_pActiveObjects, box, count);
							count = CompareObjectToList(iter, pActiveObject, pParent->m_pSleepingObjects, box, count);

							//Cloth comparisons
							clothCount = CompareObjectToList(clothIter, pActiveObject, pParent->m_pActiveClothObjects, box, clothCount);
							clothCount = CompareObjectToList(clothIter, pActiveObject, pParent->m_pSleepingClothObjects, box, clothCount);

							//Fluid comparisons
							fluidCount = CompareObjectToList(fluidIter, pActiveObject, pParent->m_pFluidMeshObjects, box, fluidCount);

							pActives = pActives->m_pNext;
						}

						if(pParent->m_pKinematicObjects || pParent->m_pActiveClothObjects)
						{
							C_ObjectWrapper* pSleepy(pNode->m_pSleepingObjects);
							while(pSleepy)
							{
								C_Object* pSleepyObject(pSleepy->m_pObject);
								count = CompareObjectToList(iter, pSleepyObject, pParent->m_pKinematicObjects, box, count);

								//Cloth comparisons
								clothCount = CompareObjectToList(clothIter, pSleepyObject, pParent->m_pActiveClothObjects, box, clothCount);

								pSleepy = pSleepy->m_pNext;
							}
						}

						if(pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects)
						{
							C_ObjectWrapper* pKinematic(pParent->m_pKinematicObjects);
							while(pKinematic)
							{
								C_Object* pKinematicObject(pKinematic->m_pObject);
								//Cloth comparisons
								clothCount = CompareObjectToList(clothIter, pKinematicObject, pNode->m_pActiveClothObjects, box, clothCount);
								clothCount = CompareObjectToList(clothIter, pKinematicObject, pNode->m_pSleepingClothObjects, box, clothCount);
								pKinematic = pKinematic->m_pNext;
							}
						}


						if(pNode->m_pStaticObjects || pNode->m_pKinematicObjects || pNode->m_pSleepingObjects || 
							pNode->m_pActiveClothObjects || pNode->m_pSleepingClothObjects || pNode->m_pFluidMeshObjects)
						{
							C_ObjectWrapper* pParentActive(pParent->m_pActiveObjects);
							while(pParentActive)
							{
								C_Object* pActiveObj(pParentActive->m_pObject);
								count = CompareObjectToList(iter, pActiveObj, pNode->m_pSleepingObjects, box, count);
								count = CompareObjectToList(iter, pActiveObj, pNode->m_pKinematicObjects, box, count);
								count = CompareObjectToList(iter, pActiveObj, pNode->m_pStaticObjects, box, count);

								//Cloth comparisons
								clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pActiveClothObjects, box, clothCount);
								clothCount = CompareObjectToList(clothIter, pActiveObj, pNode->m_pSleepingClothObjects, box, clothCount);
								
								//Fluid comparisons
								fluidCount = CompareObjectToList(fluidIter, pActiveObj, pNode->m_pFluidMeshObjects, box, fluidCount);
								pParentActive = pParentActive->m_pNext;
							}

							if(pNode->m_pKinematicObjects || pNode->m_pActiveClothObjects)
							{
								C_ObjectWrapper* pParentSleeping(pParent->m_pSleepingObjects);
								while(pParentSleeping)
								{
									C_Object* pSleepyObject(pParentSleeping->m_pObject);
									count = CompareObjectToList(iter, pSleepyObject, pNode->m_pKinematicObjects, box, count);
									//Cloth comparisons
									clothCount = CompareObjectToList(clothIter, pSleepyObject, pNode->m_pActiveClothObjects, box, clothCount);

									pParentSleeping = pParentSleeping->m_pNext;
								}
							}
							if(pParent->m_pActiveClothObjects || pParent->m_pSleepingClothObjects)
							{
								C_ObjectWrapper* pKinematic(pNode->m_pKinematicObjects);
								while(pKinematic)
								{
									C_Object* pKinematicObject(pKinematic->m_pObject);
									//Cloth comparisons
									clothCount = CompareObjectToList(clothIter, pKinematicObject, pParent->m_pActiveClothObjects, box, clothCount);
									clothCount = CompareObjectToList(clothIter, pKinematicObject, pParent->m_pSleepingClothObjects, box, clothCount);
									pKinematic = pKinematic->m_pNext;
								}
							}
							if(pParent->m_pActiveClothObjects)
							{
								C_ObjectWrapper* pStatic(pNode->m_pStaticObjects);
								while(pStatic)
								{
									C_Object* pStaticObject(pStatic->m_pObject);
									//Cloth comparisons
									clothCount = CompareObjectToList(clothIter, pStaticObject, pParent->m_pActiveClothObjects, box, clothCount);
									pStatic = pStatic->m_pNext;
								}
							}

							if(pNode->m_pActiveClothObjects)
							{
								C_ObjectWrapper* pWrapper(pParent->m_pStaticObjects);
								while(pWrapper)
								{
									C_Object* pStaticObject(pWrapper->m_pObject);
									clothCount = CompareObjectToList(clothIter, pStaticObject, pParent->m_pActiveClothObjects, box, clothCount);																
									pWrapper = pWrapper->m_pNext;
								}

								pWrapper = pParent->m_pSleepingObjects;
								while(pWrapper)
								{
									C_Object* pSleepingObject(pWrapper->m_pObject);
									clothCount = CompareObjectToList(clothIter, pSleepingObject, pParent->m_pActiveClothObjects, box, clothCount);																
									pWrapper = pWrapper->m_pNext;
								}
							}
						}
					}
					pParent = pParent->m_pParentNode;
				}
			}

			pNode=pNode->m_pNext;
		}

		C_DistributionNode* pSleepingNodes(m_pSleepingNodes);
		while(pSleepingNodes)
		{
			{
				//PERF_EVENT("Gravity.BroadPhase_SleepingParentComparison");
				C_DistributionNode* pParent(pSleepingNodes->m_pParentNode);
				while(pParent)
				{
					if(pParent->m_pKinematicObjects || pParent->m_pActiveObjects)
					{
						C_ObjectWrapper* pSleepy(pSleepingNodes->m_pSleepingObjects);
						while(pSleepy)
						{
							C_Object* pSleepyObject(pSleepy->m_pObject);
							count = CompareObjectToList(iter, pSleepyObject, pParent->m_pKinematicObjects, pSleepingNodes->m_bbox, count);
							count = CompareObjectToList(iter, pSleepyObject, pParent->m_pActiveObjects, pSleepingNodes->m_bbox, count);
							pSleepy = pSleepy->m_pNext;
						}
					}

					if(pSleepingNodes->m_pStaticObjects)
					{
						C_ObjectWrapper* pActive(pParent->m_pActiveObjects);
						while(pActive)
						{
							C_Object* pActiveObject(pActive->m_pObject);
							count = CompareObjectToList(iter, pActiveObject, pSleepingNodes->m_pStaticObjects, pSleepingNodes->m_bbox, count);
							pActive = pActive->m_pNext;
						}
					}

					pParent = pParent->m_pParentNode;
				}
			}
			pSleepingNodes = pSleepingNodes->m_pNext;
		}
		numPairs = count;
		numClothPairs = clothCount;
		numFluidPairs = fluidCount;
	}



	void C_DistributionTree::RemoveObject(C_Object* pObj)
	{
		//Loop through all the nodes this objects in and remove them from this 
		C_SortedObjectWrapper wrapper(pObj);
		pObj->m_ObjectState |= E_Removed;
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&pObj->m_Nodes[a]);
			pWrapper->m_pNode->Remove(pWrapper);
		}
		pObj->m_NumNodesContained = 0;
		m_ObjectArray.Remove(&wrapper);		
	}

	void C_DistributionTree::RemoveStaticObject(C_Object* pObj)
	{
		//Loop through all the nodes this objects in and remove them from this 
		C_SortedObjectWrapper wrapper(pObj);
		pObj->m_ObjectState |= E_Removed;
		for(U32 a = 0; a < pObj->m_NumNodesContained; ++a)
		{
			C_DistributionNodeWrapper* pWrapper(&pObj->m_Nodes[a]);
			pWrapper->m_pNode->Remove(pWrapper);
		}
		pObj->m_NumNodesContained = 0;
		m_StaticObjectArray.Remove(&wrapper);
		
	}

	void C_DistributionTree::RemoveFluidMeshObject(C_Object* pObj)
	{
		RemoveStaticObject(pObj);		
	}

	U32 C_DistributionTree::AddSleepingObject(C_Object* pObj, SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack)
	{
		using namespace Collision;
		using namespace Maths;

		SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> iter(&pObjPairs);
		pObj->m_NumNodesContained=0;
		C_CRAxisBox& pObjBox=pObj->m_TransformedBox;
		HVec3 _min(Maths::Constants<HVec3>::Max());
		HVec3 _max(-Maths::Constants<HVec3>::Max());

		U32 size = 1;
		C_DistributionNode* __restrict pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, 0);
		while(size--)
		{
			C_DistributionNode* node = pStackTop;
			if(size > 0)
				pStackTop = stack.Pop<C_DistributionNode>(size-1);
			if(node->m_bbox.Intersects(pObjBox))
			{
				if(node->m_pLeftNode)
				{
					stack.Push(node->m_pRightNode, size++);
					stack.Push(node->m_pLeftNode, size++);
					pStackTop = node->m_pLeftNode;
					continue;
				}
				//If down here, we are at a leaf node, so need to push the object in there
				C_ObjectWrapper* pWrapper2=node->m_pSleepingObjects;
				
				//Compare the pObj with the sleeping objects in this leaf node. If their box intersect, put them into the pObjPairs
				while(pWrapper2)
				{
					C_Object* pObj2=pWrapper2->m_pObject;
					C_CRAxisBox& pObjBox2=pObj2->m_TransformedBox;
					if(pObjBox.Intersects(pObjBox2))
					{
						C_ObjectPair* pPair=iter.Create();
						pPair->m_pObjectA=pObj;
						pPair->m_pObjectB=pObj2;
						
					}
					pWrapper2=pWrapper2->m_pNext;
				}

				//Compare the pObj with the static objects in this leaf node. If their box intersect, put them into the pObjPairs
				pWrapper2=node->m_pStaticObjects;
				while(pWrapper2)
				{
					C_Object* pObj2=pWrapper2->m_pObject;
					C_CRAxisBox& pObjBox2=pObj2->m_TransformedBox;
					if(pObjBox.Intersects(pObjBox2))
					{
						C_ObjectPair* pPair=iter.Create();
						pPair->m_pObjectA=pObj;
						pPair->m_pObjectB=pObj2;
					}
					pWrapper2=pWrapper2->m_pNext;
				}

				C_ObjectWrapper* pWrapper = &pObj->m_Nodes[pObj->m_NumNodesContained++];
				pWrapper->m_pObject = pObj;
				pWrapper->m_pNode = node;
				node->AddToSleeping(pWrapper);
				if(!node->IsActive() && node->MustActivate())
				{
					node->m_State = E_DNS_Active;
					node->m_pNext = m_pActiveNodes;
				//	node->m_pPrev = NULL;
					//C_DistributionNode* pNext = Fast_Select<C_DistributionNode>(m_pActiveNodes == NULL, &dummyNode, m_pActiveNodes);
					//pNext->m_pPrev = node;
					m_pActiveNodes = node;
				}
				_min = Min(node->m_bbox.m_min, _min);
				_max = Max(node->m_bbox.m_max, _max);
			}
		}
		pObj->m_NodeBox.m_max = _max;
		pObj->m_NodeBox.m_min = _min;
		return iter.NumElements();
	}

	//U32 C_DistributionTree::AddStaticObject(C_Object* pObj, SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack)
	//{
	//	using namespace Maths;

	//	SharedPhysics::C_MemoryIterator<C_ObjectPair, 128> iter(&pObjPairs);
	//	pObj->m_NumNodesContained=0;
	//	C_CRAxisBox& pObjBox=pObj->m_TransformedBox;
	//	HVec3 _min(Constants<HVec3>::Max());
	//	HVec3 _max(-Constants<HVec3>::Max());

	//	U32 size = 1;
	//	C_DistributionNode* __restrict pStackTop(m_pRootNode);
	//	stack.Push(m_pRootNode, 0);;
	//	while(size--)
	//	{
	//		C_DistributionNode* node = pStackTop;
	//		if(size > 0)
	//			pStackTop = stack.Pop<C_DistributionNode>(size-1);
	//		if(node->m_bbox.Intersects(pObjBox))
	//		{
	//			if(node->m_pLeftNode)
	//			{
	//				stack.Push(node->m_pRightNode, size++);
	//				stack.Push(node->m_pLeftNode, size++);
	//				pStackTop = node->m_pLeftNode;
	//				continue;
	//			}
	//			//If down here, we are at a leaf node, so need to push the object in there
	//			C_ObjectWrapper* pWrapper2=node->m_pSleepingObjects;
	//			
	//			//Compare the pObj with the sleeping objects in this leaf node. If their box intersect, put them into the pObjPairs
	//			while(pWrapper2)
	//			{
	//				C_Object* pObj2=pWrapper2->m_pObject;
	//				C_CRAxisBox& pObjBox2=pObj2->m_TransformedBox;
	//				if(pObjBox.Intersects(pObjBox2))
	//				{
	//					C_ObjectPair* pPair=iter.Create();
	//					pPair->m_pObjectA=pObj;
	//					pPair->m_pObjectB=pObj2;
	//					
	//				}
	//				pWrapper2=pWrapper2->m_pNext;
	//			}

	//			C_ObjectWrapper* pWrapper = &pObj->m_Nodes[pObj->m_NumNodesContained++];
	//			pWrapper->m_pObject = pObj;
	//			pWrapper->m_pNode = node;
	//			node->AddToStatic(pWrapper);
	//			if(!node->IsActive() && node->MustActivate())
	//			{
	//				node->m_State = E_DNS_Active;
	//				node->m_pNext = m_pActiveNodes;
	//			//	node->m_pPrev = NULL;
	//				//C_DistributionNode* pNext = Fast_Select<C_DistributionNode>(m_pActiveNodes == NULL, &dummyNode, m_pActiveNodes);
	//				//pNext->m_pPrev = node;
	//				m_pActiveNodes = node;
	//			}
	//			_min = Min(node->m_bbox.m_min, _min);
	//			_max = Max(node->m_bbox.m_max, _max);
	//		}
	//	}
	//	pObj->m_NodeBox.m_max = _max;
	//	pObj->m_NodeBox.m_min = _min;
	//	return iter.NumElements();
	//}

	bool C_DistributionTree::ValidTree(C_DistributionNode* node)
	{
		//Loop through the tree and ensure (a) all nodes are contained in their parent's bound box and that
		//the union of both sub-boxes equals the parent's box!

	
		if(node->m_pLeftNode)
		{
			if(!node->m_pLeftNode->m_bbox.Inside(node->m_bbox))
				return false;
			if(!node->m_pRightNode->m_bbox.Inside(node->m_bbox))
				return false;
			//Now check to make sure that the boxes cover the expected space
			if(!node->m_pLeftNode->m_bbox.m_min.All_EQ(node->m_bbox.m_min))
				return false;
			if(!node->m_pRightNode->m_bbox.m_max.All_EQ(node->m_bbox.m_max))
				return false;
			const Maths::HF32 parentVolume(node->m_bbox.Volume());
			const Maths::HF32 childrenVolume(node->m_pLeftNode->m_bbox.Volume() + node->m_pRightNode->m_bbox.Volume());
			if( parentVolume != childrenVolume )
				return false;

			bool validA = ValidTree(node->m_pLeftNode);
			bool validB = ValidTree(node->m_pRightNode);
			return validA && validB;
		}
		return true;
	}

	void C_DistributionTree::UpdateObject(C_Object* pObj, SharedPhysics::C_SafeStack<true> & stack)
	{
		//PERF_EVENT("DistributionTree.UpdateObject");
		using namespace Maths;

		if(((pObj->m_ObjectState & E_NoCollision) == 0) && (/*g_dbForceUpdate ||*/ pObj->IsUpdateRequired()))
		{

#ifdef SHOW_UPDATE_OBJECTS
			Maths::HMat33 identity;
			identity.SetIdentity();
			Maths::HMat43 mat(identity, pObj->m_TransformedBox.centre);
			DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(-pObj->m_TransformedBox.extents, pObj->m_TransformedBox.extents,
				mat, Maths::RGBA(255, 0, 0));
#endif

			for(U32 b = 0; b < pObj->m_NumNodesContained; ++b)
			{
				C_DistributionNodeWrapper* pWrapper(&pObj->m_Nodes[b]);
				BIZ_ASSERT(pObj->m_Nodes[b].m_pNode->ValidNodeLists());
				pWrapper->m_pNode->Remove(pWrapper);
				BIZ_ASSERT(pObj->m_Nodes[b].m_pNode->ValidNodeLists());
			}
			pObj->m_NumNodesContained = 0;
			HVec3 _min(Maths::Constants<HVec3>::Max());
			HVec3 _max(-Maths::Constants<HVec3>::Max());

			//Update is required, so need to go ahead and insert the object into the distribution tree
			S32 size = 1;
			C_DistributionNode*  __restrict pStackTop(m_pRootNode);
			stack.Push(m_pRootNode, 0);
			while(size--)
			{
				C_DistributionNode* node = pStackTop;
				//if(size > 0)
				pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(size-1, 0));
				if(node->m_bbox.Intersects(pObj->m_TransformedBox))
				{
					if(node->m_pLeftNode && pObj->ContainedInNode(node))
					{
						stack.Push(node->m_pRightNode, size++);
						stack.Push(node->m_pLeftNode, size++);
						pStackTop = node->m_pLeftNode;
						continue;
					}
					/*if(((pObj->m_ObjectState == E_ActiveObject) || 
						(pObj->m_ObjectState == E_SleepingObject)) && 
						node->m_pLeftNode != NULL)
					{
						DebugBreak();
					}*/
					//If down here, we are at a leaf node, so need to push the object in there
					assert(pObj->m_NumNodesContained < NUM_DISTRIBUTION_NODES);
					if(pObj->m_NumNodesContained >= NUM_DISTRIBUTION_NODES)
					{
						continue;
					}
					C_ObjectWrapper* pWrapper = &pObj->m_Nodes[pObj->m_NumNodesContained++];
					pWrapper->m_pObject = pObj;
					pWrapper->m_pNode = node;
					BIZ_ASSERT(node->ValidNodeLists());
					node->AddObject(pWrapper);
					BIZ_ASSERT(node->ValidNodeLists());
					_min = Min(node->m_bbox.m_min, _min);
					_max = Max(node->m_bbox.m_max, _max);
					if(!node->IsActive())
					{
						if(node->MustActivate())
						{
							AddToActive(node);
						}
						else if (!node->IsSleeping() && node->MustPutToSleep())
						{
							AddToSleeping(node);
						}
					}
				}
			}
			pObj->m_NodeBox.m_max = _max;
			pObj->m_NodeBox.m_min = _min;
		}
	}

	bool C_DistributionNode::ValidNodeLists()
	{
		//Loop through all the elements in the list and verify they are all correct!

#ifdef CHECK_BPHASE
		C_ObjectWrapper* pNode = m_pActiveObjects;
		while(pNode)
		{
			BIZ_ASSERT((pNode->m_pObject->m_ObjectState & ~E_Dirty) == E_ActiveObject);
			pNode = pNode->m_pNext;
		}

		pNode = m_pSleepingObjects;
		while(pNode)
		{
			BIZ_ASSERT((pNode->m_pObject->m_ObjectState & ~E_Dirty) == E_SleepingObject);
			pNode = pNode->m_pNext;
		}

		pNode = m_pKinematicObjects;
		while(pNode)
		{
			BIZ_ASSERT((pNode->m_pObject->m_ObjectState & ~E_Dirty) == E_KinematicObject);
			pNode = pNode->m_pNext;
		}

		pNode = m_pStaticObjects;
		while(pNode)
		{
			BIZ_ASSERT((pNode->m_pObject->m_ObjectState & ~E_Dirty) == E_StaticObject);
			pNode = pNode->m_pNext;
		}

		pNode = m_pActiveClothObjects;
		while(pNode)
		{
			BIZ_ASSERT((pNode->m_pObject->m_ObjectState & ~E_Dirty) == E_ActiveClothObject );
			pNode = pNode->m_pNext;
		}

		pNode = m_pSleepingClothObjects;
		while(pNode)
		{
			BIZ_ASSERT((pNode->m_pObject->m_ObjectState & ~E_Dirty) == E_SleepingClothObject);
			pNode = pNode->m_pNext;
		}
#endif

		return true;
	}

	void C_DistributionTree::UpdateTree(SharedPhysics::C_SafeStack<true>& stack,
		Gravity::C_BodyList& activeList, Gravity::C_BodyList& continuousList, Gravity::C_BodyList& kinematicList)
	{
		PERF_EVENT("Gravity.UpdateTree");
		//First bit - let's work out if we need to restructure the tree before we do anything else...

		using namespace Maths;
		if(!m_pRootNode)
			return;

		if((!(g_dbDisableSplit && g_dbDisableMerge)))// && (--m_CountDownToUpdate) <= 0)
		{
			m_CountDownToUpdate = m_UpdateFrequency;
			LeafState state;
			C_LeafNodeIterator iter(&m_LeafNodes);
			{
				PERF_EVENT("Gravity.RemapTree");
				C_DistributionNode* pNode = this->m_pActiveNodes;
				//while(iter.HasNext())
				while(pNode)
				{
					//C_DistributionNode* pNode(iter.Next());
					if(pNode->m_pLeftNode)
					{
						pNode = pNode->m_pNext;
						continue;
					}
					C_DistributionNode* pPrevNode(pNode->m_pPrev);
					//Check extents of box
					if(pNode->ObjectCount() > m_HighObjectThreshold )
					{
						m_pActiveNodes = pNode->Split(*m_pAllocator, iter, m_DesiredCellSize, m_pActiveNodes, *this);
					/*	if(m_pActiveNodes!=NULL && m_pActiveNodes == m_pActiveNodes->m_pNext)
						{
								assert(0);
						}*/
					}
					else
					{
						C_DistributionNode* parent(pNode->m_pParentNode);
						if(parent && parent->m_pLeftNode->m_pLeftNode == NULL && parent->m_pRightNode->m_pLeftNode == NULL &&
							((parent->ObjectCount() + parent->m_pLeftNode->ObjectCount() + parent->m_pRightNode->ObjectCount()) < m_LowObjectThreshold /*|| !(parent->m_pLeftNode->IsActive() || parent->m_pRightNode->IsActive())*/))
						{
							state.m_pActiveRoot = m_pActiveNodes;
							state.m_pSleepingRoot = m_pSleepingNodes;
							m_pActiveNodes = parent->Merge(*m_pAllocator, iter, state, m_MaxCellSize, *this);
							
							/*if(m_pActiveNodes!=NULL && m_pActiveNodes == m_pActiveNodes->m_pNext)
							{
								assert(0);
							}*/
							m_pSleepingNodes = state.m_pSleepingRoot;
						}
					}
					pPrevNode = SharedPhysics::Fast_Select<C_DistributionNode>(pPrevNode == NULL, m_pActiveNodes, pPrevNode);
					if(pPrevNode == NULL)
						break;		

					pNode = SharedPhysics::Fast_Select<C_DistributionNode>(pPrevNode->m_pNext == pNode, pNode->m_pNext, pPrevNode->m_pNext);

					/*if(pPrevNode->m_pNext == pNode)
						pNode = pNode->m_pNext;
					else
						pNode = pPrevNode->m_pNext;*/
				}
			}
		}

		//assert(ValidTree(m_pRootNode));
		//Updating the tree structure. First thing - delete the list of active nodes...
		//Loop through active nodes and mark as inactive...
		C_DistributionNode dummyNode;
		/*C_DistributionNode* pOldActiveNodes(m_pActiveNodes);
		C_DistributionNode* pOldSleepingNodes(m_pSleepingNodes);*/
		m_pPrevActiveNodes = m_pActiveNodes;
		m_pPrevSleepingNodes = m_pSleepingNodes;
		/*while(pOldActiveNodes)
		{
			assert(pOldActiveNodes->IsActive());
			pOldActiveNodes = pOldActiveNodes->m_pNext;
		}

		while(pOldSleepingNodes)
		{
			assert(pOldSleepingNodes->IsSleeping());
			pOldSleepingNodes = pOldSleepingNodes->m_pNext;
		}

		pOldActiveNodes = m_pActiveNodes;
		pOldSleepingNodes = m_pSleepingNodes;*/
		m_pActiveNodes = NULL;
		m_pSleepingNodes = NULL;
		//Loop through the array of objects...
		{
			PERF_EVENT("Gravity.UpdateObjects1");
			C_Body* pBody = activeList.m_pFirstBody;
			while(pBody)
			{
				C_Object* pObj = pBody->GetObject();
				//C_Object* pObj = m_ObjectArray[a].m_pElement;
				BIZ_ASSERT(!pBody->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_StaticMesh));
				UpdateObject(pObj, stack);
				pBody = pBody->m_pNext;
			}

			pBody = kinematicList.m_pFirstBody;
			while(pBody)
			{
				C_Object* pObj = pBody->GetObject();
				//C_Object* pObj = m_ObjectArray[a].m_pElement;
				UpdateObject(pObj, stack);
				pBody = pBody->m_pNext;
			}

			pBody = continuousList.m_pFirstBody;
			while(pBody)
			{
				C_Object* pObj = pBody->GetObject();
				//C_Object* pObj = m_ObjectArray[a].m_pElement;
				UpdateObject(pObj, stack);
				pBody = pBody->m_pNext;
			}

			/*for(S32 a = 0; a < m_ObjectArray.m_numElements; ++a)
			{
				C_Object* pObj = m_ObjectArray[a].m_pElement;
				UpdateObject(pObj, stack);
			}*/
		}
		{
			PERF_EVENT("Gravity.UpdateObjects2");
			for(S32 a = 0; a < m_DirtyStaticArray.m_numElements; ++a)
			{
				C_Object* pObj = m_DirtyStaticArray.m_pElems[a].m_pElement;
				//if(!(pObj->m_ObjectState & E_Removed))
				/*if(pObj->m_pGraphNode && pObj->m_pGraphNode->m_pBody && pObj->m_pGraphNode->m_pBody->TestBodyFlags(C_Body::E_InsertedIntoWorld) && 
					(pObj->m_ObjectState & E_Dirty))*/
				//if(C_Body::GetBody(pObj)->TestBodyFlags(C_Body::E_InsertedIntoWorld))
				if(pObj->m_pGraphNode && pObj->m_pGraphNode->m_pBody && pObj->m_pGraphNode->m_pBody->TestBodyFlags(C_Body::E_InsertedIntoWorld))
				{
					UpdateObject(pObj, stack);
				}
				pObj->m_ObjectState = pObj->m_ObjectState & ~(E_Removed);
			}
			m_DirtyStaticArray.m_numElements = 0;
		}

		{
			PERF_EVENT("Gravity.UpdatePrevNodes");
			C_DistributionNode* pOldNode(m_pPrevActiveNodes);
			C_DistributionNode* pLastNode(NULL);
			while(pOldNode)
			{
				C_DistributionNode* pNextNode(pOldNode->m_pNext);
				if(!pOldNode->MustActivate())
				{
					//AddToActive(pOldNode);
					if (pOldNode->MustPutToSleep())
					{
						AddToSleeping(pOldNode);
					}
					else
					{
						if(pOldNode == m_pPrevActiveNodes)
							m_pPrevActiveNodes = pOldNode->m_pNext;
						if(pOldNode->m_pNext)
							pOldNode->m_pNext->m_pPrev = pOldNode->m_pPrev;
						if(pOldNode->m_pPrev)
							pOldNode->m_pPrev->m_pNext = pOldNode->m_pNext;
						pOldNode->m_State = E_DNS_Inactive;
						pOldNode->m_pNext = pOldNode->m_pPrev = NULL;
					}
				}
				else
				{
					pLastNode = pOldNode;
				}
				pOldNode = pNextNode;
			}

			if(pLastNode)
			{
				pLastNode->m_pNext = m_pActiveNodes;
				if(m_pActiveNodes)
					m_pActiveNodes->m_pPrev = pLastNode;
				m_pActiveNodes = m_pPrevActiveNodes;
			}

			pLastNode = NULL;

			C_DistributionNode* pOldSleepingNode(m_pPrevSleepingNodes);
			while(pOldSleepingNode)
			{
				C_DistributionNode* pNextNode(pOldSleepingNode->m_pNext);
				
				if(pOldSleepingNode->MustActivate())
				{
					AddToActive(pOldSleepingNode);
				}
				else if (!pOldSleepingNode->MustPutToSleep())
				{
					
				//AddToSleeping(pOldSleepingNode);
					if(pOldSleepingNode == m_pPrevSleepingNodes)
						m_pPrevSleepingNodes = pOldSleepingNode->m_pNext;
					if(pOldSleepingNode->m_pNext)
						pOldSleepingNode->m_pNext->m_pPrev = pOldSleepingNode->m_pPrev;
					if(pOldSleepingNode->m_pPrev)
						pOldSleepingNode->m_pPrev->m_pNext = pOldSleepingNode->m_pNext;
					pOldSleepingNode->m_State = E_DNS_Inactive;
					pOldSleepingNode->m_pNext = pOldSleepingNode->m_pPrev = NULL;
				}
				else
				{
					pLastNode = pOldSleepingNode;
				}
				pOldSleepingNode = pNextNode;
			}

			if(pLastNode)
			{
				pLastNode->m_pNext = m_pSleepingNodes;
				if(m_pSleepingNodes)
					m_pSleepingNodes->m_pPrev = pLastNode;
				m_pSleepingNodes = m_pPrevSleepingNodes;
			}
			m_pPrevActiveNodes = m_pPrevSleepingNodes = NULL;
		}
	}

	U32 C_DistributionTree::GetIntersectingBodies(const Collision::C_Sphere& sphere, Collision::C_CollisionFilter* pFilter, SharedPhysics::MemoryAllocator<C_Body*>& allocator, SharedPhysics::C_SafeStack<true>& stack)
	{
		if(m_pRootNode == NULL)
			return 0;
		S32 size = 1;
		C_DistributionNode*  __restrict pStackTop(m_pRootNode);
		stack.Push(m_pRootNode, 0);

		Maths::HVec3 closest;
		U32 numHits = 0;
		SharedPhysics::C_MemoryIterator<C_Body*> iter(&allocator);

		while(size--)
		{
			C_DistributionNode* node = pStackTop;
			//if(size > 0)
			pStackTop = stack.Pop<C_DistributionNode>(Maths::Max(size-1, 0));

			//Test against static/kinematic objects!
			C_ObjectWrapper* pWrapper = node->m_pStaticObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pKinematicObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pActiveObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pSleepingObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pActiveClothObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pSleepingClothObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pRaytraceOnlyObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pActiveDebrisObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = node->m_pSleepingDebrisObjects;
			while(pWrapper)
			{
				if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphereInBox(sphere.Centre(), sphere.Radius(),
					node->m_bbox))
				//if(pWrapper->m_pObject->m_TransformedBox.IntersectsSphere(sphere.Centre(), sphere.Radius()))
				{
					if(pFilter->CanCollide(&pWrapper->m_pObject->m_pShape->m_Filter))
					{
						C_Body** pBody = iter.Create();
						*pBody = pWrapper->m_pObject->m_pGraphNode->m_pBody;
						++numHits;
					}
				}
				pWrapper = pWrapper->m_pNext;
			}

			if(node->m_pLeftNode != NULL)
			{
				if(node->m_pRightNode->m_bbox.IntersectSphere(sphere.Centre(), sphere.Radius()))
				{
					pStackTop = node->m_pRightNode;
					stack.Push(node->m_pRightNode, size++);
				}

				if(node->m_pLeftNode->m_bbox.IntersectSphere(sphere.Centre(), sphere.Radius()))
				{
					pStackTop = node->m_pLeftNode;
					stack.Push(node->m_pLeftNode, size++);
				}
			}

		}
		return numHits;
	}

	
	void C_DistributionTree::DrawActiveNodes(DbUtil::I_DebugRender* pRender, bool bShowInactiveParts)
	{
		Maths::HMat43 identity;
		identity.SetIdentity();
		C_DistributionNode* pNode = m_pActiveNodes;
		while(pNode)
		{
			pRender->Draw3dBox(pNode->m_bbox.m_min, pNode->m_bbox.m_max, identity, Maths::RGBA(255, 0, 0));

			C_ObjectWrapper* pWrapper(pNode->m_pActiveClothObjects);
			while(pWrapper)
			{
				C_Object* pObject = pWrapper->m_pObject;
				pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
					pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
					pObject->m_TransformedBox.centre), Maths::RGBA(0, 0, 255));
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = pNode->m_pActiveObjects;
			while(pWrapper)
			{
				C_Object* pObject = pWrapper->m_pObject;
				pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
					pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
					pObject->m_TransformedBox.centre), Maths::RGBA(255, 255, 0));
				pWrapper = pWrapper->m_pNext;
			}

			pWrapper = pNode->m_pKinematicObjects;
			while(pWrapper)
			{
				C_Object* pObject = pWrapper->m_pObject;
				pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
					pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
					pObject->m_TransformedBox.centre), Maths::RGBA(255, 0, 255));
				pWrapper = pWrapper->m_pNext;
			}

			if(bShowInactiveParts)
			{

				pWrapper = pNode->m_pSleepingClothObjects;
				while(pWrapper)
				{
					C_Object* pObject = pWrapper->m_pObject;
					pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
						pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
						pObject->m_TransformedBox.centre), Maths::RGBA(0, 255, 0));
					pWrapper = pWrapper->m_pNext;
				}

				pWrapper = pNode->m_pStaticObjects;
				while(pWrapper)
				{
					C_Object* pObject = pWrapper->m_pObject;
					pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
						pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
						pObject->m_TransformedBox.centre), Maths::RGBA(0, 0, 255));
					pWrapper = pWrapper->m_pNext;
				}

				

				pWrapper = pNode->m_pSleepingObjects;
				while(pWrapper)
				{
					C_Object* pObject = pWrapper->m_pObject;
					pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
						pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
						pObject->m_TransformedBox.centre), Maths::RGBA(0, 255, 0));
					pWrapper = pWrapper->m_pNext;
				}

				pWrapper = pNode->m_pRaytraceOnlyObjects;
				while(pWrapper)
				{
					C_Object* pObject = pWrapper->m_pObject;
					pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
						pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
						pObject->m_TransformedBox.centre), Maths::RGBA(0, 255, 255));
					pWrapper = pWrapper->m_pNext;
				}
			}

			pNode = pNode->m_pNext;
		}
	}

	void C_DistributionTree::DrawSleepingNodes(DbUtil::I_DebugRender* pRender)
	{
		Maths::HMat43 identity;
		identity.SetIdentity();
		C_DistributionNode* pNode = m_pSleepingNodes;
		while(pNode)
		{
			pRender->Draw3dBox(pNode->m_bbox.m_min, pNode->m_bbox.m_max, identity, Maths::RGBA(0, 255, 0));
			C_ObjectWrapper* pWrapper(pNode->m_pSleepingClothObjects);
			while(pWrapper)
			{
				C_Object* pObject = pWrapper->m_pObject;
				pRender->Draw3dBox(-pObject->m_TransformedBox.extents, 
					pObject->m_TransformedBox.extents, Maths::HMat43(Maths::Constants<Maths::HMat33>::Identity(), 
					pObject->m_TransformedBox.centre), Maths::RGBA(0, 255, 0));
				pWrapper = pWrapper->m_pNext;
			}
			pNode = pNode->m_pNext;
		}
	}

//#pragma optimize("", on)

};


