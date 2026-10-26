#ifndef DISTRIBUTIONTREE_H
#define DISTRIBUTIONTREE_H

#include "gravity/narrowphase/AxisBox.h"
#include "gravity/broadphase/Object.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/narrowphase/Ray.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/sharedphysics/SafeStack.h"
#include "gravity/sharedphysics/list.h"

namespace DbUtil
{
	class I_DebugRender;
}

namespace Gravity
{
	

	class C_BodyList;
	class C_DistributionNode;

	class C_LeafNodeList;
	class C_LeafNodeIterator;
	class C_DistributionTree;


	typedef SharedPhysics::C_AllocationManager<C_DistributionNode, 256, true> C_DistributionNodeAllocator;

	struct LeafState
	{
		C_DistributionNode* m_pActiveRoot, *m_pSleepingRoot;
	};

	enum E_DistributionNodeState
	{
		E_DNS_Inactive,
		E_DNS_Active,
		E_DNS_Sleeping
	};
	class ALIGN16 C_DistributionNode : public SharedPhysics::C_BlockItem<C_DistributionNode>
	{
	public:
		Collision::C_AxisBoxMinMax m_bbox;
		//Linked-list of active objects in this node
		union
		{
			struct
			{
				C_ObjectWrapper* m_pActiveObjects;
				//Linked list of sleeping objects in this node
				C_ObjectWrapper* m_pSleepingObjects;
				//Linked list of static objects in this node
				C_ObjectWrapper* m_pStaticObjects;
				//Linked list of kinematic objects in this node
				C_ObjectWrapper* m_pKinematicObjects;
				//Linked list of active cloth objects in this node
				C_ObjectWrapper* m_pActiveClothObjects;
				//Linked list of sleeping cloth objects in this node
				C_ObjectWrapper* m_pSleepingClothObjects;
				//Linked list of active fluid objects in this node
				C_ObjectWrapper* m_pActiveFluidObjects;	
				//Linked list of sleeping fluid objects in this node
				C_ObjectWrapper* m_pSleepingFluidObjects;
				//Linked list of fluid mesh objects in this node
				C_ObjectWrapper* m_pFluidMeshObjects;
				//Linked list of raytrace-only things in this node
				C_ObjectWrapper* m_pRaytraceOnlyObjects;
				//Linked list of active debris things in this node
				C_ObjectWrapper* m_pActiveDebrisObjects;
				//Linked list of sleeping debris things in this node
				C_ObjectWrapper* m_pSleepingDebrisObjects;

				
			};
			struct
			{
				C_ObjectWrapper* m_pObjectArrays[12];
			};
		};
		E_DistributionNodeState m_State;

		//Active distribution nodes are stored as part of a linked list. BlockItem already implements
		//a singly-linked list, so we just need to add in the prev pointer to make it double-linked
		//C_DistributionNode *m_pPrev;
		C_DistributionNode* m_pLeftNode, *m_pRightNode, *m_pParentNode, *m_pPrev;
		C_DistributionNode* m_pNextLeaf, *m_pPrevLeaf;
		U32 m_ObjectCount;

		static U32 m_NodeCount;

		C_DistributionNode() : m_pActiveObjects(NULL), m_pSleepingObjects(NULL), m_pStaticObjects(NULL),
			m_pKinematicObjects(NULL), m_pActiveClothObjects(NULL), m_pSleepingClothObjects(NULL), m_pActiveFluidObjects(NULL),
			m_pSleepingFluidObjects(NULL), m_pFluidMeshObjects(NULL), m_pRaytraceOnlyObjects(NULL),
			m_pActiveDebrisObjects(NULL), m_pSleepingDebrisObjects(NULL),
			m_State(E_DNS_Inactive), m_pLeftNode(NULL), m_pRightNode(NULL), m_pParentNode(NULL), m_pPrev(NULL),	
			m_pNextLeaf(NULL), m_pPrevLeaf(NULL), m_ObjectCount(0)
		{
		}

		void Subdivide(const Maths::HVec3 PCREF desiredSize, const S32 maxDepth, C_DistributionNodeAllocator& memoryAllocator,
			C_LeafNodeList& list);

		bool ValidNodeLists();

		//NB - returns the left node of the split - in the case that this node was the first active
		//distribution node, then the left node will now be the first active distribution node!
		C_DistributionNode* Split(C_DistributionNodeAllocator& memoryAllocator, C_LeafNodeIterator& leafNodes, Maths::HVec3 PCREF desiredSize, C_DistributionNode* pRoot,
			C_DistributionTree& tree);
		C_DistributionNode* Merge(C_DistributionNodeAllocator& memoryAllocator, C_LeafNodeIterator& leafNodes, LeafState& state,
			const Maths::HVec3 PCREF maxCellSize, C_DistributionTree& tree);

		NOALIAS inline U32 ObjectCount() __restrict;
		NOALIAS inline U32 ChildObjectCount() __restrict;

		NOALIAS inline void Remove(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToActive(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToActiveDebris(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToActiveCloth(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToActiveFluid(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToSleeping(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToSleepingDebris(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToSleepingCloth(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS	inline void AddToSleepingFluid(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToStatic(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToKinematic(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddToRaytraceOnly(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline void AddObject(C_ObjectWrapper* __restrict pWrapper) __restrict;

		NOALIAS inline bool IsActive() const __restrict
		{
			return (m_State & ~E_Dirty) == E_DNS_Active;
		}

		NOALIAS inline bool MustActivate() const __restrict
		{
			return m_pActiveObjects != NULL || m_pActiveClothObjects != NULL || m_pKinematicObjects != NULL ||
				m_pActiveFluidObjects !=  NULL || m_pActiveDebrisObjects != NULL;
		}

		NOALIAS inline bool IsSleeping() const __restrict
		{
			return (m_State & ~E_Dirty) == E_DNS_Sleeping;
		}

		NOALIAS inline bool MustPutToSleep() const __restrict
		{
			/*return (m_pSleepingObjects != NULL || m_pSleepingClothObjects != NULL 
				|| m_pSleepingFluidObjects != NULL || m_pFluidMeshObjects != NULL || m_pStaticObjects != NULL
				|| m_pSleepingDebrisObjects != NULL) 
				&& (m_pActiveObjects == NULL && m_pKinematicObjects == NULL &&
				m_pActiveClothObjects == NULL);*/
			//KierS - disable this
			return false;
		}
	};

	class C_LeafNodeList
	{
	public:
		C_DistributionNode m_Dummy;
		C_DistributionNode* m_pRoot;


		C_LeafNodeList() : m_pRoot(NULL)
		{
		}

		void AddNode(C_DistributionNode* pNode)
		{
			//Push onto the root...
			C_DistributionNode* pNext(SharedPhysics::Fast_Select<C_DistributionNode>(m_pRoot != NULL, m_pRoot, &m_Dummy));
			pNode->m_pNextLeaf = m_pRoot;
			pNext->m_pPrevLeaf = pNode;
			pNode->m_pPrevLeaf = NULL;
			m_pRoot = pNode;
		}

		void Remove(C_DistributionNode* pNode)
		{
			
			C_DistributionNode* pNext( SharedPhysics::Fast_Select<C_DistributionNode>(pNode->m_pNextLeaf != NULL, pNode->m_pNextLeaf, &m_Dummy));
			C_DistributionNode* pPrev(SharedPhysics::Fast_Select<C_DistributionNode>(pNode->m_pPrevLeaf != NULL, pNode->m_pPrevLeaf, &m_Dummy));
			pNext->m_pPrevLeaf = pNode->m_pPrevLeaf;
			pPrev->m_pNextLeaf = pNode->m_pNextLeaf;
			m_pRoot = SharedPhysics::Fast_Select<C_DistributionNode>(m_pRoot == pNode, pNode->m_pNextLeaf, m_pRoot); 
			pNode->m_pNextLeaf = pNode->m_pPrevLeaf = NULL;
		}

		void Reset()
		{
			m_pRoot = NULL;
		}
	};

	class C_LeafNodeIterator
	{
		C_DistributionNode m_Tmp;
		C_LeafNodeList* m_pList;
		C_DistributionNode* m_pCurrentNode;

	public:

		C_LeafNodeIterator(C_LeafNodeList* list) : m_pList(list), m_pCurrentNode(list->m_pRoot)
		{
		}

		bool HasNext()
		{
			return m_pCurrentNode != NULL;
		}

		C_DistributionNode* Next()
		{
			C_DistributionNode* node(m_pCurrentNode);
			m_pCurrentNode = m_pCurrentNode->m_pNextLeaf;			
			return node;
		}

		//Removes the node from list and points currentnode to the next node in the list (after removal)
		void Remove(C_DistributionNode* node)
		{
			if(node == m_pCurrentNode)
			{
				m_pCurrentNode =m_pCurrentNode->m_pNextLeaf;
			}
			m_pList->Remove(node);		
		}

		void AddNode(C_DistributionNode* node)
		{
			m_pList->AddNode(node);
		}
	};

	typedef SharedPhysics::C_PointerWrapper<C_Object> C_SortedObjectWrapper;

	class C_DistributionTree
	{
		friend class C_Object;
		friend class C_Scene;
		//NOTE - we are using a sorted array here, but are just using this to store objects in a growing
		//array. I doubt I will need these to be sorted for any reason
		//C_DistributionNodeAllocator m_Allocator;
		C_DistributionNode m_DummyNode;
		C_DistributionNodeAllocator* m_pAllocator;
		SharedPhysics::SortedArray<C_SortedObjectWrapper> m_ObjectArray;
		SharedPhysics::SortedArray<C_SortedObjectWrapper> m_StaticObjectArray;
		SharedPhysics::SortedArray<C_SortedObjectWrapper> m_DirtyStaticArray;
		C_LeafNodeList m_LeafNodes;
		Collision::C_AxisBoxMinMax m_WorldBox;
		Maths::HVec3 m_DesiredCellSize;
		Maths::HVec3 m_MaxCellSize;
		S32 m_MaxDepth;
		U32 m_LowObjectThreshold, m_HighObjectThreshold;

		//The root of the distribution tree
		C_DistributionNode * m_pRootNode;

		//The linked list of active nodes...
		C_DistributionNode* m_pActiveNodes;
		C_DistributionNode* m_pSleepingNodes;

		C_DistributionNode* m_pPrevActiveNodes;
		C_DistributionNode* m_pPrevSleepingNodes;

		S32 m_CountDownToUpdate;

		U32 m_UpdateFrequency;

	public:
		C_DistributionTree(Collision::C_AxisBoxMinMax PCREF worldBox, const Maths::HVec3 PCREF desiredSize, 
			const Maths::HVec3 PCREF maxCellSize, S32 maxDepth,	U32 updateFrequency = 4) : 
		m_WorldBox(worldBox), m_DesiredCellSize(desiredSize), m_MaxCellSize(maxCellSize), m_MaxDepth(maxDepth), m_pActiveNodes(NULL), m_pSleepingNodes(NULL),
			m_CountDownToUpdate(updateFrequency), m_UpdateFrequency(updateFrequency)
		{
			m_pAllocator = new C_DistributionNodeAllocator;
			m_pRootNode = m_pAllocator->GetNextFreeItem();
			m_pRootNode->m_bbox = m_WorldBox;
			m_pRootNode->Subdivide(m_DesiredCellSize, m_MaxDepth - 1, *m_pAllocator, m_LeafNodes);
			m_LowObjectThreshold = 5;
			m_HighObjectThreshold = 20;
		}

		C_DistributionTree(U32 updateFrequency = 4) : m_pRootNode(NULL), m_pActiveNodes(NULL), m_pSleepingNodes(NULL),
			m_CountDownToUpdate(updateFrequency), m_UpdateFrequency(updateFrequency)
		{
			m_pAllocator = new C_DistributionNodeAllocator;
			m_LowObjectThreshold = 5;
			m_HighObjectThreshold = 20;
		}

		~C_DistributionTree()
		{
			delete m_pAllocator;
		}

		const Collision::C_AxisBoxMinMax& GetWorldBox()
		{
			return m_WorldBox;
		}

		bool ValidTree(C_DistributionNode* node);

		NOALIAS inline void SetDimensions(Collision::C_AxisBoxMinMax PCREF worldBox, const Maths::HVec3 PCREF desiredSize,
			const Maths::HVec3 PCREF maxCellSize, S32 maxDepth)
		{
			m_WorldBox = worldBox;
			m_DesiredCellSize = desiredSize;
			m_MaxCellSize = maxCellSize;
			m_MaxDepth = maxDepth;
			m_pAllocator->Reset();
			m_LeafNodes.Reset();
			m_pRootNode = m_pAllocator->GetNextFreeItem();
			m_pRootNode->m_bbox = m_WorldBox;
			m_pRootNode->Subdivide(m_DesiredCellSize, m_MaxDepth - 1, *m_pAllocator, m_LeafNodes);
			m_pActiveNodes = m_pSleepingNodes = NULL;

			for(S32 a = 0; a < m_ObjectArray.m_numElements; ++a)
			{
				m_ObjectArray[a].m_pElement->MarkDirty();
				m_DirtyStaticArray.AddToEnd(&m_ObjectArray[a]);
			}

			for(S32 a = 0; a < m_StaticObjectArray.m_numElements; ++a)
			{
				C_SortedObjectWrapper& wrapper(m_StaticObjectArray[a]);
				if((wrapper.m_pElement->m_ObjectState & E_Dirty) == 0)
				{
					wrapper.m_pElement->MarkDirty();
					wrapper.m_pElement->m_NumNodesContained = 0;
					m_DirtyStaticArray.AddToEnd(&m_StaticObjectArray[a]);
				}
			}
		}

		NOALIAS inline void AddDirtyObject(C_Object* pObj)
		{
			C_SortedObjectWrapper wrapper(pObj);
			m_DirtyStaticArray.AddToEnd(&wrapper);
		}

		NOALIAS inline void AddObject(C_Object* pObj)
		{
			C_SortedObjectWrapper wrapper(pObj);
			m_ObjectArray.Insert(&wrapper);
			pObj->m_ObjectState = pObj->m_ObjectState & (~E_Removed);
		}

		NOALIAS inline void AddFluidMeshObject(C_Object* pObj)
		{
			C_SortedObjectWrapper wrapper(pObj);
			m_StaticObjectArray.Insert(&wrapper);
			m_DirtyStaticArray.AddToEnd(&wrapper);
			pObj->m_ObjectState = pObj->m_ObjectState & (~E_Removed);
		}

		NOALIAS inline void AddStaticObject(C_Object* pObj)
		{
			pObj->MarkDirty();
			C_SortedObjectWrapper wrapper(pObj);
			m_StaticObjectArray.Insert(&wrapper);
			m_DirtyStaticArray.AddToEnd(&wrapper);
			pObj->m_ObjectState = pObj->m_ObjectState & (~E_Removed);
		}

		void AddToActive(C_DistributionNode* pNode);

		void AddToSleeping(C_DistributionNode* pNode);

		//Returns the number of objects which this object overlaps with...
		U32 AddSleepingObject(C_Object* pObj, SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack);

		//Returns the number of objects which this object overlaps with..
		//U32 AddStaticObject(C_Object* pObj, SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack);

		U32 CompareObjectToList(SharedPhysics::C_MemoryIterator<C_ObjectPair, 128>& objectIter, Gravity::C_Object* pObject, 
			Gravity::C_ObjectWrapper* pList, Collision::C_AxisBoxMinMax& box, const U32& numContacts);

		U32 CollidedPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack);

		U32 ClothPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack);

		U32 FluidPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, SharedPhysics::C_SafeStack<true>& stack);

		void CombinedPairGeneration(SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pObjPairs, U32& numPairs,
			SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pClothPairs, U32& numClothPairs,
			SharedPhysics::MemoryAllocator<C_ObjectPair, 128>& pFluidPairs, U32& numFluidPairs, SharedPhysics::C_SafeStack<true>& stack);

		bool RayTracing(Collision::C_Ray& ray, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache);

		bool RayTracing(Collision::C_TransparentRay& ray,  Collision::C_CollisionFilter* pFilter, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache);

		bool FatRayTracing(Collision::C_Ray& ray, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache);

		bool FatRayTracing(Collision::C_TransparentRay& ray,  Collision::C_CollisionFilter* pFilter, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache);

		bool RayTraceWater(Collision::C_Ray& ray, Collision::C_CollisionManager& manager, SharedPhysics::C_SafeStack<true> & stack, SharedPhysics::C_SmallSPUCache& spuCache);

		void RemoveObject(C_Object* pObj);

		void RemoveStaticObject(C_Object* pObj);

		void RemoveFluidMeshObject(C_Object* pObj);

		void UpdateTree(SharedPhysics::C_SafeStack<true>& stack, C_BodyList& activeList, C_BodyList& continuousList, C_BodyList& kinematicList);

		void UpdateObject(C_Object* pObj, SharedPhysics::C_SafeStack<true> & stack);

		U32 GetIntersectingBodies(const Collision::C_Sphere& sphere, Collision::C_CollisionFilter* pFilter, SharedPhysics::MemoryAllocator<C_Body*>& allocator, SharedPhysics::C_SafeStack<true>& stack);

		void DrawActiveNodes(DbUtil::I_DebugRender* pRender, bool bShowInactiveParts);

		void DrawSleepingNodes(DbUtil::I_DebugRender* pRender);

		void FillCache(Collision::C_CRAxisBox& box, Collision::C_LineCollisionCache& pCache, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
			const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle, Collision::C_CollisionManager& manager, SharedPhysics::C_SmallSPUCache& spuCache);

	};
};

#include "gravity/broadphase/DistributionTree.inl"

#endif
