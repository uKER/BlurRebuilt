#ifndef TRIGGER_SYSTEM_H
#define TRIGGER_SYSTEM_H

#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/narrowphase/AxisBox.h"
#include "gravity/narrowphase/Shape.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/Select.h"
#include "gravity/sharedphysics/MemoryAllocator.h"	// [Blur2PC] C_MemoryIterator
#include "Btl/Containers/Vector.h"

namespace Gravity
{

	//class C_Body;
	class C_TriggerShape;
	class C_TriggerLeafNodeIterator;
	class C_TriggerLayer;
	//Trigger system consists of triggers (boxes/spheres), trigger actors (dynamic objects, cameras etc.).
	//Triggers can be attached to dynamic objects, or be static, hence have states (active, sleeping and static).
	//
	//We are going to have multiple trigger layers so, for example, audio can construct their own layer, separate
	//from the gameplay code. This is because the performance may get quite bad with huge numbers of triggers/trigger actors


#define MAX_TRIGGER_WRAPPERS 8

	class C_TriggerNode;
	class C_TriggerLeafNodeList;
	class C_TriggerLeafNodeIterator;
	class C_TriggerActor;
	class C_Trigger;
	class C_TriggerLayer;
	class C_TriggerSystem;

	typedef SharedPhysics::C_AllocationManager<C_TriggerNode, 128, true> C_TriggerNodeAllocator;

	struct C_TriggerPair
	{
		C_Trigger* m_pTrigger;
		C_TriggerActor* m_pTriggerActor;
		C_TriggerLayer* m_pTriggerLayer;
	};

	enum E_Trigger_State
	{
		E_InactiveTrigger_State,
		E_ActiveTrigger_State,
		E_TriggerState_Count,
		E_Processed_Active = 0x100,
		E_DirtyTrigger = 0x1000
	};

	enum E_TriggerNode_State
	{
		E_TriggerNode_None,
		E_TriggerNode_Inactive,
		E_TriggerNode_Active
	};

	struct TriggerLeafState
	{
		C_TriggerNode* m_pActiveRoot, *m_pInactiveRoot;
	};


	class C_TriggerActorWrapper
	{
	public:
		C_TriggerActor* m_pActor;
		C_TriggerNode* m_pNode;

		C_TriggerActorWrapper* m_pNext;
		C_TriggerActorWrapper* m_pPrev;
		bool bDirty;

		C_TriggerActorWrapper() : m_pActor(NULL), m_pNode(NULL), m_pNext(NULL), m_pPrev(NULL), bDirty(false)
		{
		}
	};

	class C_TriggerWrapper
	{
	public:
		C_Trigger* m_pTrigger;
		C_TriggerNode* m_pNode;

		C_TriggerWrapper* m_pNext;
		C_TriggerWrapper* m_pPrev;
		bool bDirty;

		C_TriggerWrapper() : m_pTrigger(NULL), m_pNode(NULL), m_pNext(NULL), m_pPrev(NULL), bDirty(false)
		{
		}
	};

	class C_TriggerActor : public SharedPhysics::C_BlockItem<C_TriggerActor>
	{
		friend class C_TriggerLayer;
		friend class C_TriggerNode;
	public:
		//The body this trigger actor is associated with - this will always be non-null

		C_TriggerShape* m_pShape;
		Collision::C_CRAxisBox m_TransformedBox;
		Collision::C_AxisBoxMinMax m_NodeBox;

		void* m_pUserData;
		C_TriggerActor* m_pPrev;

		U32 m_State;

		U32 m_TriggerIndex;
		static U32 s_TriggerActorIndex;

		C_TriggerActorWrapper m_Actors[MAX_TRIGGER_WRAPPERS];
		U32 m_NumNodesContained;
		C_TriggerLayer* m_pLayer;

		C_TriggerActor() : m_pPrev(NULL), m_pUserData(NULL), m_State(E_InactiveTrigger_State),
			m_TriggerIndex(s_TriggerActorIndex++), m_NumNodesContained(0), m_pLayer(NULL)
		{
		}

		~C_TriggerActor();

		void SetUserData(void* data)
		{
			m_pUserData = data;
		}

		void* GetUserData()
		{
			return m_pUserData;
		}

		void SetMatrix(const Maths::HMat43 PCREF matrix, SharedPhysics::C_SafeStack<true>& stack);

		const Maths::HMat43 GetMatrix();

		void MarkDirty()
		{
			m_State =  m_State | E_DirtyTrigger; 
		}

	private:

		void Transform(const Maths::HMat43 PCREF matrix);

		void RemoveFrom(const U32 index);

		void Activate();

		void Deactivate();

		bool IsUpdateRequired();

		bool ContainedInNode(C_TriggerNode* pNode);

	public:
		bool operator < (const C_TriggerActor& other)
		{
			return m_TriggerIndex < other.m_TriggerIndex;
		}

		bool operator <= (const C_TriggerActor& other)
		{
			return m_TriggerIndex <= other.m_TriggerIndex;
		}

		bool operator > (const C_TriggerActor& other)
		{
			return m_TriggerIndex > other.m_TriggerIndex;
		}

		bool operator >= (const C_TriggerActor& other)
		{
			return m_TriggerIndex >= other.m_TriggerIndex;
		}

		bool operator == (const C_TriggerActor& other)
		{
			return m_TriggerIndex == other.m_TriggerIndex;
		}

		bool operator != (const C_TriggerActor& other)
		{
			return m_TriggerIndex != other.m_TriggerIndex;
		}

	};

	class C_Trigger : public SharedPhysics::C_BlockItem<C_Trigger>
	{
		friend class C_TriggerLayer;
		friend class C_TriggerNode;
	public:
		

		Collision::C_Shape* m_pTriggerShape;
		Collision::C_CRAxisBox m_TransformedBox;
		Collision::C_AxisBoxMinMax m_NodeBox;

		void* m_pUserData;
		C_Trigger* m_pPrev;

		U32 m_State;

		U32 m_TriggerIndex;
		static U32 s_TriggerIndex;

		C_TriggerWrapper m_Actors[MAX_TRIGGER_WRAPPERS];
		U32 m_NumNodesContained;
		C_TriggerLayer* m_pLayer;

		C_Trigger() : m_pPrev(NULL), m_pUserData(NULL), m_State(E_InactiveTrigger_State),
			m_TriggerIndex(s_TriggerIndex++), m_NumNodesContained(0), m_pLayer(NULL)
		{
		}

		~C_Trigger();

		void SetUserData(void* data)
		{
			m_pUserData = data;
		}

		void* GetUserData()
		{
			return m_pUserData;
		}

		void MarkDirty()
		{
			m_State |= E_DirtyTrigger; 
		}

		void SetMatrix(const Maths::HMat43 PCREF matrix, SharedPhysics::C_SafeStack<true>& stack);

		const Maths::HMat43 GetMatrix();
		
		C_TriggerSystem* GetTriggerSystem();


	private:
		void Transform(const Maths::HMat43 PCREF matrix);

		void RemoveFrom(const U32 index);

		void Activate();

		void Deactivate();		

		bool IsUpdateRequired();

		bool ContainedInNode(C_TriggerNode* pNode);

	public:
		bool operator < (const C_Trigger& other)
		{
			return m_TriggerIndex < other.m_TriggerIndex;
		}

		bool operator <= (const C_Trigger& other)
		{
			return m_TriggerIndex <= other.m_TriggerIndex;
		}

		bool operator > (const C_Trigger& other)
		{
			return m_TriggerIndex > other.m_TriggerIndex;
		}

		bool operator >= (const C_Trigger& other)
		{
			return m_TriggerIndex >= other.m_TriggerIndex;
		}

		bool operator == (const C_Trigger& other)
		{
			return m_TriggerIndex == other.m_TriggerIndex;
		}

		bool operator != (const C_Trigger& other)
		{
			return m_TriggerIndex != other.m_TriggerIndex;
		}
	};

	class C_TriggerNode : public SharedPhysics::C_BlockItem<C_TriggerNode>
	{
	public:

		Collision::C_AxisBoxMinMax m_bbox;
		union
		{
			struct
			{
				
				C_TriggerWrapper* m_pInactiveTriggers;
				C_TriggerWrapper* m_pActiveTriggers;
				
				C_TriggerActorWrapper* m_pInactiveTriggerActors;
				C_TriggerActorWrapper* m_pActiveTriggerActors;
			};
			struct
			{
				C_TriggerWrapper* m_pTriggerArray[E_TriggerState_Count];
				C_TriggerActorWrapper* m_pTriggerActorArray[E_TriggerState_Count];
			};
		};

		E_TriggerNode_State m_State;

		//Left/right nodes, parent and previous node in linked list of nodes
		C_TriggerNode* m_pLeftNode, *m_pRightNode, *m_pParentNode, *m_pPrev;

		//Leaf node list
		C_TriggerNode* m_pNextLeaf, *m_pPrevLeaf;

		U32 m_TriggerCount, m_TriggerActorCount;

		C_TriggerNode() : m_pInactiveTriggers(NULL), m_pActiveTriggers(NULL), m_pInactiveTriggerActors(NULL),
			m_pActiveTriggerActors(NULL), m_State(E_TriggerNode_None),
			m_pLeftNode(NULL), m_pRightNode(NULL), m_pParentNode(NULL), m_pPrev(NULL),
			m_pNextLeaf(NULL), m_pPrevLeaf(NULL), m_TriggerCount(0), m_TriggerActorCount(0)
		{
		}

		void Remove(C_TriggerWrapper* pWrapper);
		void Remove(C_TriggerActorWrapper* pWrapper);
		void AddToActive(C_TriggerWrapper* pWrapper);
		void AddToActive(C_TriggerActorWrapper* pWrapper);
		void AddToInactive(C_TriggerWrapper* pWrapper);
		void AddToInactive(C_TriggerActorWrapper* pWrapper);

		void AddTrigger(C_TriggerWrapper* pWrapper);
		void AddTriggerActor(C_TriggerActorWrapper* pWrapper);

		void DeactivateObjects();

		bool IsActive();
		bool IsSleeping();
		bool MustActivate();
		bool MustAddToSleeping();

		U32 TriggerAndTriggerActorCount();

		void Subdivide(const Maths::HVec3 PCREF desiredSize, const S32 maxDepth, C_TriggerNodeAllocator& memoryAllocator,
			C_TriggerLeafNodeList& list);

		C_TriggerNode* Split(C_TriggerNodeAllocator& memoryAllocator, C_TriggerLeafNodeIterator& leafNodes, Maths::HVec3 PCREF desiredSize, C_TriggerNode* pRoot,
			C_TriggerLayer& tree);
		C_TriggerNode* Merge(C_TriggerNodeAllocator& memoryAllocator, C_TriggerLeafNodeIterator& leafNodes, TriggerLeafState& state,
			const Maths::HVec3 PCREF maxCellSize, C_TriggerLayer& tree);


	};

	class C_TriggerLeafNodeList
	{
	public:
		C_TriggerNode m_Dummy;
		C_TriggerNode* m_pRoot;


		C_TriggerLeafNodeList() : m_pRoot(NULL)
		{
		}

		void AddNode(C_TriggerNode* pNode)
		{
			//Push onto the root...
			C_TriggerNode* pNext(SharedPhysics::Fast_Select<C_TriggerNode>(m_pRoot != NULL, m_pRoot, &m_Dummy));
			pNode->m_pNextLeaf = m_pRoot;
			pNext->m_pPrevLeaf = pNode;
			pNode->m_pPrevLeaf = NULL;
			m_pRoot = pNode;
		}

		void Remove(C_TriggerNode* pNode)
		{
			
			C_TriggerNode* pNext( SharedPhysics::Fast_Select<C_TriggerNode>(pNode->m_pNextLeaf != NULL, pNode->m_pNextLeaf, &m_Dummy));
			C_TriggerNode* pPrev(SharedPhysics::Fast_Select<C_TriggerNode>(pNode->m_pPrevLeaf != NULL, pNode->m_pPrevLeaf, &m_Dummy));
			pNext->m_pPrevLeaf = pNode->m_pPrevLeaf;
			pPrev->m_pNextLeaf = pNode->m_pNextLeaf;
			m_pRoot = SharedPhysics::Fast_Select<C_TriggerNode>(m_pRoot == pNode, pNode->m_pNextLeaf, m_pRoot); 
			pNode->m_pNextLeaf = pNode->m_pPrevLeaf = NULL;
		}

		void Reset()
		{
			m_pRoot = NULL;
		}
	};

	class C_TriggerLeafNodeIterator
	{
		C_TriggerNode m_Tmp;
		C_TriggerLeafNodeList* m_pList;
		C_TriggerNode* m_pCurrentNode;

	public:

		C_TriggerLeafNodeIterator(C_TriggerLeafNodeList* list) : m_pList(list), m_pCurrentNode(list->m_pRoot)
		{
		}

		bool HasNext()
		{
			return m_pCurrentNode != NULL;
		}

		C_TriggerNode* Next()
		{
			C_TriggerNode* node(m_pCurrentNode);
			m_pCurrentNode = m_pCurrentNode->m_pNextLeaf;			
			return node;
		}

		//Removes the node from list and points currentnode to the next node in the list (after removal)
		void Remove(C_TriggerNode* node)
		{
			if(node == m_pCurrentNode)
			{
				m_pCurrentNode =m_pCurrentNode->m_pNextLeaf;
			}
			m_pList->Remove(node);		
		}

		void AddNode(C_TriggerNode* node)
		{
			m_pList->AddNode(node);
		}
	};

	typedef SharedPhysics::C_PointerWrapper<C_Trigger> C_SortedTriggerWrapper;
	typedef SharedPhysics::C_PointerWrapper<C_TriggerActor> C_SortedTriggerActorWrapper;

	class C_TriggerLayer : public SharedPhysics::C_BlockItem<C_TriggerLayer>
	{
	
		//friend class C_Body;
		friend class C_Scene;
		friend class C_TriggerNode;
		friend class C_TriggerSystem;
		//NOTE - we are using a sorted array here, but are just using this to store objects in a growing
		//array. I doubt I will need these to be sorted for any reason
		//C_DistributionNodeAllocator m_Allocator;

		C_TriggerNode m_DummyNode;
		C_TriggerNodeAllocator* m_pAllocator;
		SharedPhysics::SortedArray<C_SortedTriggerWrapper> m_TriggerArray;
		SharedPhysics::SortedArray<C_SortedTriggerActorWrapper> m_TriggerActorArray;

		Maths::HVec3 m_MaxCellSize;

		//Linked list of active triggers/trigger nodes!
		C_Trigger* m_pActiveTriggers;
		C_TriggerActor* m_pActiveTriggerActors;

		//Arrays of static objects which have been dirtied
		SharedPhysics::SortedArray<C_Trigger*> m_DirtyTriggerArray;
		SharedPhysics::SortedArray<C_TriggerActor*> m_DirtyTriggerActorArray;

		C_TriggerLeafNodeList m_LeafNodes;
		Collision::C_AxisBoxMinMax m_WorldBox;
		Maths::HVec3 m_DesiredCellSize;
		S32 m_MaxDepth;
		U32 m_LowObjectThreshold, m_HighObjectThreshold;

		//The root of the distribution tree
		C_TriggerNode * m_pRootNode;

		//The linked list of active nodes...
		C_TriggerNode* m_pActiveNodes;
		C_TriggerNode* m_pSleepingNodes;

		C_TriggerNode* m_pPrevActiveNodes;
		C_TriggerNode* m_pPrevSleepingNodes;

		C_TriggerSystem* m_pTriggerSystem;

		U32 m_CountDownToUpdate;
		U32 m_UpdateFrequency;

		U32 m_LayerIndex;
		bool m_NeedsUpdate;
		bool m_Updated;
		U32 m_NumContacts;
		U32 m_ActiveNodeCount;

		//static U32 s_LayerIndex;

public:

		C_TriggerLayer* m_pPrev;

		C_TriggerLayer(const Maths::HVec3 PCREF minBox, const Maths::HVec3 PCREF maxBox, 
			const Maths::HVec3 PCREF desiredCellSize, U32 maxDepth, U32 index, C_TriggerSystem* pTriggerSystem, const Maths::HVec3 PCREF maxCellSize = Maths::HVec3(10.f, 100.f, 10.f));

		~C_TriggerLayer();

		void SetMaxCellSize(const Maths::HVec3 PCREF maxCellSize);
		void AddTrigger(C_Trigger* pTrigger);
		void RemoveTrigger(C_Trigger* pTrigger);

		void AddTriggerActor(C_TriggerActor* pTriggerActor);
		void RemoveTriggerActor(C_TriggerActor* pTriggerActor);

		void UpdateLayer(SharedPhysics::C_SafeStack<true>& stack);

		void SetTriggerMatrix(C_Trigger* pTrigger, const Maths::HMat43 PCREF matrix,
			SharedPhysics::C_SafeStack<true>& stack);
		void SetTriggerActorMatrix(C_TriggerActor* pTriggerActor, const Maths::HMat43 PCREF matrix,
			SharedPhysics::C_SafeStack<true>& stack);

		U32 GenerateIntersectionPairs(SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs,
			SharedPhysics::C_SafeStack<true>& stack);

		void RemoveActiveObjects();

		U32 GetLayerIndex()
		{
			return m_LayerIndex;
		}

		bool WasUpdated()
		{
			return m_Updated;
		}
		
		C_TriggerSystem* GetTriggerSystem()
		{
			return m_pTriggerSystem;
		}

	private:
		void AddToActive(C_TriggerNode* pNode);
		void AddToSleeping(C_TriggerNode* pNode);

		void AddToActive(C_TriggerActor* pNode);
		void AddToActive(C_Trigger* pNode);
		void RemoveFromActive(C_TriggerActor* pNode);
		void RemoveFromActive(C_Trigger* pNode);

		void AddToDirty(C_TriggerActor* pTrigger);
		void AddToDirty(C_Trigger* pTrigger);

		void UpdateTrigger(C_Trigger* pTrigger, SharedPhysics::C_SafeStack<true>& stack);

		void UpdateTriggerActor(C_TriggerActor* pTriggerActor, SharedPhysics::C_SafeStack<true>& stack);

		U32 CompareTriggerToList(SharedPhysics::C_MemoryIterator<C_TriggerPair>& triggerIter, Gravity::C_TriggerActor* pActor, 
		Gravity::C_TriggerWrapper* pList, Collision::C_AxisBoxMinMax& box, const U32& numContacts);	
		U32 ActiveNodeComparison(C_TriggerNode* pActiveNode, SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs);
		U32 ActiveParentComparison(C_TriggerNode* pActiveNode, SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs);
		U32 ActiveChildComparison(C_TriggerNode* pActiveNode, SharedPhysics::C_MemoryIterator<C_TriggerPair>& iter, const U32& numPairs, 
			SharedPhysics::C_SafeStack<true>& stack);
	};

	class C_TriggerCollider
	{

	public:
		C_Trigger* m_pTrigger;
		C_TriggerActor* m_pTriggerActor;
		C_TriggerLayer* m_pLayer;
		U32 m_TriggerIndex, m_TriggerActorIndex;
		bool m_Exist;
		bool m_Removed;

		//Work out a hash value for this thing...

	
		C_TriggerCollider() : m_pTrigger(NULL), m_pTriggerActor(NULL), m_pLayer(NULL), m_Exist(true), m_Removed(false),
			m_TriggerIndex(0), m_TriggerActorIndex(0)
		{
		}

		C_TriggerCollider(C_Trigger* pTrigger, C_TriggerActor* pTriggerActor, C_TriggerLayer* pLayer) : m_Exist(true), m_Removed(false)
		{
			SetTriggerData(pTrigger, pTriggerActor, pLayer);
		}

		~C_TriggerCollider()
		{
		}

		void SetTriggerData(C_Trigger* pTrigger, C_TriggerActor* pTriggerActor, C_TriggerLayer* pLayer)
		{
			m_pTrigger = pTrigger;
			m_pTriggerActor = pTriggerActor;
			m_pLayer = pLayer;
			m_TriggerIndex = m_pTrigger->m_TriggerIndex;
			m_TriggerActorIndex = m_pTriggerActor->m_TriggerIndex;
		}

		bool operator < (const C_TriggerCollider& collider)
		{
			return (m_TriggerIndex < collider.m_TriggerIndex) | 
				((m_TriggerIndex == collider.m_TriggerIndex) & 
				(m_TriggerActorIndex < collider.m_TriggerActorIndex));
		}

		bool operator <= (const C_TriggerCollider& collider)
		{
			return (m_TriggerIndex < collider.m_TriggerIndex) | 
				((m_TriggerIndex == collider.m_TriggerIndex) &
				(m_TriggerActorIndex <= collider.m_TriggerActorIndex));
		}

		bool operator > (const C_TriggerCollider& collider)
		{
			return (m_TriggerIndex > collider.m_TriggerIndex) | 
				((m_TriggerIndex == collider.m_TriggerIndex) & 
				(m_TriggerActorIndex > collider.m_TriggerActorIndex));
		}

		bool operator >= (const C_TriggerCollider& collider)
		{
			return (m_TriggerIndex > collider.m_TriggerIndex) | 
				((m_TriggerIndex == collider.m_TriggerIndex) & 
				(m_TriggerActorIndex >= collider.m_TriggerActorIndex));
		}

		bool operator == (const C_TriggerCollider& collider)
		{
			return (m_TriggerIndex == collider.m_TriggerIndex) & 
				(m_TriggerActorIndex == collider.m_TriggerActorIndex);
		}

		bool operator != (const C_TriggerCollider& collider)
		{
			return (m_TriggerIndex != collider.m_TriggerIndex) | 
				(m_TriggerActorIndex != collider.m_TriggerActorIndex);
		}

		//When we find the pair again, mark as found!!!
		void MarkFound()
		{
			m_Exist = true;
		}

		//Test to see if the pair of objects have exited...
		bool HasExited()
		{
			if(m_Removed)
				return true;
			if((m_pTrigger->m_State & (~(E_Processed_Active | E_DirtyTrigger))) == 
				E_InactiveTrigger_State && 
				(m_pTriggerActor->m_State & (~(E_Processed_Active | E_DirtyTrigger))) == 
				E_InactiveTrigger_State)
			{
				//If they're asleep, then the pair are still colliding if they were before...
				return false;
			}

			if(!m_pLayer->WasUpdated())
				return false;
			bool exist = m_Exist;
			m_Exist = false;
			return exist == false;
		}
	};

	class C_TriggerEvent : public SharedPhysics::C_BlockItem<C_TriggerEvent>
	{
	public:
		enum E_TriggerEventType
		{
			E_None_Event, //If you get a message with this...it's probably a bug :-)
			E_Enter_Event,
			E_Exit_Event
		};

	private:
		E_TriggerEventType m_EventType;
		C_Trigger* m_pTrigger;
		C_TriggerActor* m_pTriggerActor;
		C_TriggerLayer* m_pTriggerLayer;

	public:
		C_TriggerEvent() : m_EventType(E_None_Event), m_pTrigger(NULL), m_pTriggerActor(NULL),
			m_pTriggerLayer(NULL)
		{
		}

		C_TriggerEvent(E_TriggerEventType type, C_Trigger* pTrigger, C_TriggerActor* pTriggerActor, 
			C_TriggerLayer* pTriggerLayer)
		{
			InitialiseEvent(type, pTrigger, pTriggerActor, pTriggerLayer);
		}

		void InitialiseEvent(E_TriggerEventType type, C_Trigger* pTrigger, C_TriggerActor* pTriggerActor, 
			C_TriggerLayer* pTriggerLayer)
		{
			m_EventType = type;
			m_pTrigger = pTrigger;
			m_pTriggerActor = pTriggerActor;
			m_pTriggerLayer = pTriggerLayer;
		}

		E_TriggerEventType GetEventType()
		{
			return m_EventType;
		}

		C_Trigger* GetTrigger()
		{
			return m_pTrigger;
		}

		C_TriggerActor* GetTriggerActor()
		{
			return m_pTriggerActor;
		}

		C_TriggerLayer* GetTriggerLayer()
		{
			return m_pTriggerLayer;
		}

		bool ValidTrigger();
	};

	//Pure virtual callback class
	class C_TriggerEventCallback
	{
	public:
		virtual void ProcessTriggerEvents(SharedPhysics::C_MemoryIterator<C_TriggerEvent>& iter, const U32 numEvents) = 0;
	};

	class C_TriggerSystem
	{
		friend class C_Scene;
		U32 m_NumExistingColliders;
		SharedPhysics::MemoryAllocator<C_TriggerPair>* m_pPairAllocator;
		SharedPhysics::MemoryAllocator<C_TriggerEvent>* m_pTriggerEvents;
		SharedPhysics::SortedArray<C_TriggerCollider> m_CollidingPairs;

		C_TriggerEventCallback* m_pCallback;
		C_TriggerLayer* m_pLayers;
	public:
		C_TriggerSystem();
		~C_TriggerSystem();

		C_TriggerLayer* CreateLayer(const Maths::HVec3 PCREF minBox, const Maths::HVec3 PCREF maxBox, 
			const Maths::HVec3 PCREF desiredCellSize, U32 maxDepth, U32 index = 0);

		void SetCallback(C_TriggerEventCallback* pCallback)
		{
			m_pCallback = pCallback;
		}

		U32 GetTriggerCount();

		U32 GetTriggerActorCount();

		U32 GetActiveNodeCount();

		C_TriggerLayer* GetLayer(U32 index);

		void DestroyLayer(C_TriggerLayer* pLayer);

		void DestroyLayers();

		void UpdateLayers(SharedPhysics::C_SafeStack<true>& stack);

		void GenerateEvents(SharedPhysics::C_S32SafeStack& stack);

		void RemoveColliders(C_Trigger* pTrigger);
		void RemoveColliders(C_TriggerActor* pTriggerActor);

		C_TriggerCollider* GetActorsInTrigger(C_Trigger* pTrigger, U32& numActorsInTrigger);

		void GetTriggersActorIsIn(C_TriggerActor* pTriggerActor, Btl::Vector<C_Trigger*>& triggerList);

		bool AddCollider(C_TriggerCollider& collider);

	private:
		void EmitEvents(SharedPhysics::C_S32SafeStack& stack);
		bool CollideShape(C_Trigger* pTrigger, C_TriggerActor* pTriggerActor);

	};
}

#endif
