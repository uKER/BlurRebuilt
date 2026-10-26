#ifndef CONSTRAINTGRAPH_H
#define CONSTRAINTGRAPH_H

#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/Select.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/sharedphysics/SafeStack.h"

#define GENERATE_ISLANDS

/*
 * The main goal of the physics engine is to create and maintain a constraint graph. A constraint is a 
 * base class which will be specialised as contact constraints, spherical constraints, revolute constraints,
 * fixed constraints and prismatic constraints. The constraint graph is used to organise the system of bodies
 * into the set of bodies which must be solved together (islands). The constraint graph records all interactions
 * between bodies. Constraints are by default bi-directional edges in a graph in which bodies are represented by nodes
 * in the graph. However, some types of bodies act as sinks in the constraint graph, such as static or sleeping bodies,
 * while others act as sinks and sources (dynamic objects) or purely sources (fixed, non-dynamic points of constraint)
 *
 */

//Big dirty hack so we can forward declare the virtual function 
#ifndef USE_SOLVER_2
namespace Solver
{
	class C_Jacobian;
	class C_InvertMJT;
};
#else
namespace Solver2
{
	class C_Jacobian;
	class C_InvertMJT;
};
#endif

namespace Gravity
{
	class C_Body;
};

namespace SharedPhysics
{
	class C_ConstraintEdge;
	class C_ConstraintTwin;
	class C_Constraint;
	class C_GraphNode;
	class C_Collider;

	//class C_Deactivator
	//{
	//public:
	//	C_Deactivator(U32 maxCountDown = 15) : m_sSleepCountdown(maxCountDown), m_CountDown(m_sSleepCountdown)
	//	{
	//	}
	//	C_Deactivator(const Maths::HF32 PCREF sleepThresh, const Maths::HF32 PCREF activThresh, C_GraphNode* pNode, U32 maxCountDown = 15) : m_sSleepCountdown(maxCountDown), m_SleepThresholdSq(sleepThresh),
	//		m_ActivateThresholdSq(activThresh), m_pGraphNode(pNode), m_CountDown(m_sSleepCountdown)
	//	{
	//	}

	//	NOALIAS inline void SetThresholds(const F32 sleepThresh, F32 activThresh, C_GraphNode* pNode)
	//	{
	//		m_SleepThresholdSq = sleepThresh;
	//		m_ActivateThresholdSq = m_ActivateThresholdSq;
	//		m_pGraphNode = pNode;
	//		m_CountDown = m_sSleepCountdown;
	//	}
	//	bool CanDeactivate();
	//	bool MustActivate();
	//	bool ActiveAndAboveSleepThresh();
	//	void Activate();

	//	/*Maths::HF32 m_SleepThresholdSq;
	//	Maths::HF32 m_ActivateThresholdSq;*/
	//	S32 m_sSleepCountdown;
	//	Maths::HF32 m_SleepThresholdSq, m_ActivateThresholdSq;
	//	C_GraphNode* m_pGraphNode;
	//	S32 m_CountDown;
	//	void SetSleepCountdown(const U32 countdown)
	//	{
	//		m_sSleepCountdown = countdown;
	//	}
	//	
	//};

	//class ALIGN16 C_GraphNode
	//{
	//public:

	//	enum _TEMPFLAGS
	//	{
	//		E_TYPE_TEMPORARY_NODE
	//	};
	//	enum E_NodeFlags
	//	{
	//		E_Type_None = 0x0,
	//		E_Type_Active = 0x1,
	//		E_Type_Sleeping = 0x2,
	//		E_Type_Waking = 0x4,
	//		E_Type_NoddingOff = 0x8,
	//		E_Type_Sleeping_Or_Waking = (E_Type_Sleeping | E_Type_Waking),
	//		E_Type_Static = 0x10,
	//		E_Type_Sleeping_Or_Static = (E_Type_Sleeping | E_Type_Static),
	//		E_Type_Root_Node = 0x20,
	//		E_Type_Visited_Island_Generation = 0x40,
	//		E_Type_Visited_Island_Processing = 0x80,
	//		E_Type_Visited_Wakeup = 0x100,
	//		E_Euler_Integrate = 0x200,
	//		E_RK4Integrate = 0x400,
	//		E_Type_StaticMesh = 0x800,
	//		E_Type_KinematicObject = 0x1000,
	//		E_Type_ZeroInertia = E_Type_Sleeping_Or_Static | E_Type_KinematicObject,
	//		E_Type_InstancedMesh = 0x2000,
	//		E_Type_Removed = 0x4000,
	//		E_Type_NoIntegrate = 0x8000,
	//		E_Type_Cloth = 0x10000,
	//		E_ApplyClothDamping = 0x20000,
	//		E_OneWayClothCollision = 0x40000
	//		//More to come...
	//	};

	//	C_GraphNode() : m_pPersistentConstraints(NULL), m_pPersistentTwinConstraints(NULL), 
	//		m_pVolatileConstraints(NULL), m_pVolatileTwinConstraints(NULL), m_pBody(NULL), 	m_Index(index++), m_IslandIndex(0),
	//		m_NodeFlags(E_Type_Active)
	//	{
	//	}

	//	C_GraphNode(C_Deactivator & deactivator) : m_pPersistentConstraints(NULL), m_pPersistentTwinConstraints(NULL), 
	//		m_pVolatileConstraints(NULL), m_pVolatileTwinConstraints(NULL), m_pBody(NULL), 	m_Index(index++), m_IslandIndex(0),
	//		m_NodeFlags(E_Type_Active), m_Deactivator(deactivator)
	//	{
	//	}

	//	C_GraphNode(_TEMPFLAGS tempFlags)
	//	{
	//		//Use this constructor for a temp storage class, so we don't give it a real index
	//	}

	//	Maths::HVec3 m_TotalKineticEnergy;
	//	//Persistent constraints are constraints which SHOULD exist between samples (e.g. joints). They
	//	//can be updated externally, but will not be re-calculated by the physics engine
	//	C_ConstraintEdge* m_pPersistentConstraints;
	//	C_ConstraintTwin* m_pPersistentTwinConstraints;
	//	//Volatile constraints are constraints which updated dynamically by the physics engine. Such constraints
	//	//may exist over a number of samples, but are expected to change dynamically
	//	C_ConstraintEdge* m_pVolatileConstraints;
	//	C_ConstraintTwin* m_pVolatileTwinConstraints;
	//	//A look-back from the constraint graph to the actual data it represents (rigid body/actor). 
	//	//There is a 1-1 mapping between rigid bodies/actors and graph nodes

	//	Gravity::C_Body* m_pBody; 

	//	C_RigidBody* m_pRigidBody;
	//	U32 m_Index;
	//	S32 m_IslandIndex; //A temporary index value within the island
	//	U32 m_NodeFlags; 
	//	C_Deactivator m_Deactivator;
	//	F32 m_Slop;
	//	F32 m_Baumgarte;

	//	static U32 index;

	//	NOALIAS inline void Slop(const F32 slop)
	//	{
	//		m_Slop = slop;
	//	}

	//	NOALIAS inline void Baumgarte(const F32 baumgarte)
	//	{
	//		m_Baumgarte = baumgarte;
	//	}

	//	NOALIAS inline F32 Slop()
	//	{
	//		return m_Slop;
	//	}

	//	NOALIAS FORCE_INLINE F32 BaumGarte()
	//	{
	//		return m_Baumgarte;
	//	}

	//	NOALIAS inline bool HasConstraints()
	//	{
	//		return m_pPersistentConstraints || m_pPersistentTwinConstraints || m_pVolatileConstraints || 
	//			m_pVolatileTwinConstraints;
	//	}

	//	NOALIAS inline void SetDeactivator(C_Deactivator& deactivator)
	//	{
	//		m_Deactivator = deactivator;
	//		m_Deactivator.m_pGraphNode = this;
	//	}

	//	NOALIAS inline void ClearNodeFlags(U32 flags)
	//	{
	//		m_NodeFlags &= (~flags);
	//	}

	//	NOALIAS inline void SetNodeFlags(U32 flags)
	//	{
	//		m_NodeFlags |= flags;
	//	}

	//	NOALIAS inline bool TestNodeFlags(U32 flags) const
	//	{
	//		return (m_NodeFlags & flags) != 0;
	//	}

	//	void WakeUpConstrainedObjects(SharedPhysics::C_SafeStack<true>& stack);

	//	NOALIAS inline bool operator < (const C_GraphNode& other) const __restrict
	//	{
	//		return m_Index < other.m_Index;
	//	}

	//	NOALIAS inline bool operator <= (const C_GraphNode& other) const __restrict
	//	{
	//		return m_Index <= other.m_Index;
	//	}

	//	NOALIAS inline bool operator > (const C_GraphNode& other) const __restrict
	//	{
	//		return m_Index > other.m_Index;
	//	}

	//	NOALIAS inline bool operator >= (const C_GraphNode& other) const __restrict
	//	{
	//		return m_Index >= other.m_Index;
	//	}

	//	NOALIAS inline bool operator == (const C_GraphNode& other) const __restrict
	//	{
	//		return m_Index == other.m_Index;
	//	}

	//	NOALIAS inline bool operator != (const C_GraphNode& other) const __restrict
	//	{
	//		return m_Index != other.m_Index;
	//	}
	//};

//	class ALIGN16 C_Constraint
//	{
//	public:
//		enum E_ConstraintType
//		{
//			E_Type_Fixed,
//			E_Type_Spherical,
//			E_Type_Revolute,
//			E_Type_RevoluteLimit,
//			E_Type_Prismatic,
//			E_Type_PrismaticLimit,
//			E_Type_CarWheel,
//			E_Type_Angular,
//			E_Type_Contact
//		};
//
//		C_Constraint(const E_ConstraintType type) : m_ConstraintType(type),
//			m_ConstraintIndex(m_NumConstraints++), pNodeA(NULL), pNodeB(NULL), m_Scale(1.f)
//		{
//			ZeroWarmStartValues();
//		}
//
//		virtual ~C_Constraint()
//		{
//		}
//
//		NOALIAS inline E_ConstraintType ConstraintType()
//		{
//			return m_ConstraintType;
//		}
//
//		NOALIAS inline bool operator < (const C_Constraint& other)
//		{
//			return m_ConstraintIndex < other.m_ConstraintIndex;
//		}
//
//		NOALIAS inline bool operator <= (const C_Constraint& other)
//		{
//			return m_ConstraintIndex <= other.m_ConstraintIndex;
//		}
//
//		NOALIAS inline bool operator > (const C_Constraint& other)
//		{
//			return m_ConstraintIndex > other.m_ConstraintIndex;
//		}
//
//		NOALIAS inline bool operator >= (const C_Constraint& other)
//		{
//			return m_ConstraintIndex >= other.m_ConstraintIndex;
//		}
//
//		NOALIAS inline bool operator == (const C_Constraint& other)
//		{
//			return m_ConstraintIndex == other.m_ConstraintIndex;
//		}
//
//		NOALIAS inline bool operator != (const C_Constraint& other)
//		{
//			return m_ConstraintIndex != other.m_ConstraintIndex;
//		}
//
//		void SetScale(const F32 scale)
//		{
//			m_Scale = scale;
//		}
//
//#ifndef USE_SOLVER_2
//
//		//No longer required to be virtual!
//		virtual void CreateJacobian(Solver::C_Jacobian* jaco)
//		{
//		}
//		//virtual void CreateLambdaLimit3(Solver::C_Jacobian3* jaco) = 0;
//		virtual void CreateInvertMJT(Solver::C_InvertMJT* imjt, Solver::C_Jacobian* jaco)
//		{
//		}
//#else
//		/*virtual void CreateJacobian(Solver2::C_Jacobian* jaco)
//		{
//		}
//		//virtual void CreateLambdaLimit3(Solver::C_Jacobian3* jaco) = 0;
//		virtual void CreateInvertMJT(Solver2::C_InvertMJT* imjt, Solver2::C_Jacobian* jaco)
//		{
//		}*/
//#endif
//		
//	
//		virtual void Release() = 0;
//		NOALIAS inline void StoreWarmstartValues(F32 *warmStart)
//		{
//			m_WarmStartValues[0] = warmStart[0];
//			m_WarmStartValues[1] = warmStart[1];
//			m_WarmStartValues[2] = warmStart[2];
//			m_WarmStartValues[3] = warmStart[3];
//			m_WarmStartValues[4] = warmStart[4];
//			m_WarmStartValues[5] = warmStart[5];
//			m_WarmStartValues[6] = warmStart[6];
//		}
//
//		NOALIAS inline void ZeroWarmStartValues()
//		{
//			m_WarmStartValues[0] = m_WarmStartValues[1] = m_WarmStartValues[2] = m_WarmStartValues[3] = m_WarmStartValues[4] = m_WarmStartValues[5] = m_WarmStartValues[6] = 0.f;
//		}
//		F32 m_WarmStartValues[7];
//		E_ConstraintType m_ConstraintType;
//		U32 m_ConstraintIndex;
//		C_Constraint* m_pPPUConstraint;
//		F32 m_Scale;
//		static U32 m_NumConstraints;
//
//		C_GraphNode* pNodeA;
//		C_GraphNode* pNodeB;
//
//	};


	//class C_ConstraintPoint : public C_BlockItem<C_ConstraintPoint>
	//{
	//public:
	//	C_ConstraintPoint(C_Constraint* pConstraint = NULL) : m_pConstraint(pConstraint), m_pPrev(NULL)
	//	{
	//	}
	//
	//	C_Constraint* m_pConstraint;	//The thing which stores the constraint itself...
	//	U32 m_HashValue;				//Used to index contacts!!!!
	//	C_ConstraintPoint* m_pPrev;
	//};

	class C_ConstraintTwin : public C_BlockItem<C_ConstraintTwin>
	{
	public:

		C_ConstraintTwin() : m_pPrev(NULL)
		{
		}

		~C_ConstraintTwin()
		{
			//m_pEdge = NULL;
		}

		NOALIAS inline bool operator < (const C_ConstraintTwin& other) const;

		NOALIAS inline bool operator <= (const C_ConstraintTwin& other) const;

		NOALIAS inline bool operator > (const C_ConstraintTwin& other) const;

		NOALIAS inline bool operator >= (const C_ConstraintTwin& other) const;

		NOALIAS inline bool operator == (const C_ConstraintTwin& other) const;


		NOALIAS inline bool operator != (const C_ConstraintTwin& other) const;
		//C_ConstraintEdge* m_pEdge;
		C_ConstraintTwin* m_pPrev;
	};


	//A constraintEdge is a representation of an arbitrary constraint in the constraint graph. It is generalised
	//in that it does not provide specific a implementation of a given constraint type, such as contact or revolute.
	class ALIGN16 C_ConstraintEdge : public C_BlockItem<C_ConstraintEdge>
	{
	public:

		enum E_EdgeFlags
		{
			E_Clear_Flags = 0,
			E_Visited_Island_Processing = 1,
			E_Visited_Island_Processing2 = 2,
			E_ReversedContacts = 3 //Used when dealing with a twin constraint
		};
		C_ConstraintEdge() : m_pInboundNode(NULL), m_pOutboundNode(NULL), 
			m_pFirstConstraint(NULL), m_pPrev(NULL), m_EdgeFlags(0), m_pCollider(NULL)
		{
		}

		~C_ConstraintEdge()
		{
			m_pInboundNode = NULL;
			m_pOutboundNode = NULL;
			m_pFirstConstraint = NULL;
			m_pCollider = NULL;
		}

		C_Collider* m_pCollider;
		C_GraphNode* m_pInboundNode, * m_pOutboundNode;
		//C_Constraint* m_pConstraint; //The thing which actually stores the formulae for this constraint
		C_ConstraintPoint* m_pFirstConstraint;
		C_ConstraintEdge* m_pPrev;
		//A twinned constraint. So we can make our way back from in the reverse direction to the constraint
		U32 m_EdgeFlags;
		C_ConstraintTwin m_TwinConstraint;


		static C_ConstraintEdge* GetConstraintEdge(const C_ConstraintTwin* const pTwin)
		{
			return (C_ConstraintEdge*)(((U8*)pTwin) - offsetof(C_ConstraintEdge, m_TwinConstraint));
		}

		NOALIAS inline bool TestEdgeFlags(const U32 edgeFlag) const __restrict
		{
			return (m_EdgeFlags & edgeFlag) != 0;
		}

		NOALIAS inline void SetEdgeFlags(const U32 edgeFlag)
		{
			m_EdgeFlags |= edgeFlag;
		}

		NOALIAS inline void ClearEdgeFlags(const U32 edgeFlag)
		{
			m_EdgeFlags &= (~edgeFlag);
		}

		NOALIAS inline void AddConstraintPoint(C_ConstraintPoint* pPoint)
		{
			C_ConstraintPoint dummy;
			C_ConstraintPoint* next = Fast_Select<C_ConstraintPoint>(m_pFirstConstraint == NULL, &dummy, m_pFirstConstraint);
			next->m_pPrev = pPoint;
			pPoint->m_pPrev = NULL;
			pPoint->m_pNext = m_pFirstConstraint;
			m_pFirstConstraint = pPoint;
		}

		/*NOALIAS inline void SetTwin(C_ConstraintTwin* pTwin)
		{
			C_ConstraintTwin dummy;
			C_ConstraintTwin* pNext = Fast_Select<C_ConstraintTwin>(m_pTwinConstraint == NULL, &dummy, m_pTwinConstraint);
			pNext->m_pPrev = pTwin;
			pTwin->m_pPrev = NULL;
			pTwin->m_pNext = m_pTwinConstraint;
			m_pTwinConstraint = pTwin;
		}*/

		NOALIAS inline bool operator < (const C_ConstraintEdge& other) const
		{
			return m_pInboundNode->m_Index < other.m_pInboundNode->m_Index ||
				(m_pInboundNode->m_Index == other.m_pInboundNode->m_Index && m_pOutboundNode->m_Index < other.m_pOutboundNode->m_Index);

		}

		NOALIAS inline bool operator <= (const C_ConstraintEdge& other) const
		{
			return m_pInboundNode->m_Index < other.m_pInboundNode->m_Index ||
				(m_pInboundNode->m_Index == other.m_pInboundNode->m_Index && m_pOutboundNode->m_Index <= other.m_pOutboundNode->m_Index);
		}

		NOALIAS inline bool operator > (const C_ConstraintEdge& other) const
		{
			return m_pInboundNode->m_Index > other.m_pInboundNode->m_Index ||
				(m_pInboundNode->m_Index == other.m_pInboundNode->m_Index && m_pOutboundNode->m_Index > other.m_pOutboundNode->m_Index);
		}

		NOALIAS inline bool operator >= (const C_ConstraintEdge& other) const
		{
			return m_pInboundNode->m_Index > other.m_pInboundNode->m_Index ||
				(m_pInboundNode->m_Index == other.m_pInboundNode->m_Index && m_pOutboundNode->m_Index >= other.m_pOutboundNode->m_Index);
		}

		NOALIAS inline bool operator == (const C_ConstraintEdge& other) const
		{
			return (m_pInboundNode->m_Index == other.m_pInboundNode->m_Index && m_pOutboundNode->m_Index == other.m_pOutboundNode->m_Index);
		}

		NOALIAS inline bool operator != (const C_ConstraintEdge& other) const
		{
			return (m_pInboundNode->m_Index != other.m_pInboundNode->m_Index || m_pOutboundNode->m_Index != other.m_pOutboundNode->m_Index);
		}
	};

	NOALIAS inline bool C_ConstraintTwin::operator < (const C_ConstraintTwin& other) const
	{
		C_ConstraintEdge* pEdge = C_ConstraintEdge::GetConstraintEdge(this);
		C_ConstraintEdge* pOtherEdge = C_ConstraintEdge::GetConstraintEdge(&other);
		return pEdge->m_pOutboundNode->m_Index < pOtherEdge->m_pOutboundNode->m_Index ||
			(pEdge->m_pOutboundNode->m_Index == pOtherEdge->m_pOutboundNode->m_Index && pEdge->m_pInboundNode->m_Index < pOtherEdge->m_pInboundNode->m_Index);
	}
	NOALIAS inline bool C_ConstraintTwin::operator <= (const C_ConstraintTwin& other) const
	{
		C_ConstraintEdge* pEdge = C_ConstraintEdge::GetConstraintEdge(this);
		C_ConstraintEdge* pOtherEdge = C_ConstraintEdge::GetConstraintEdge(&other);
		return pEdge->m_pOutboundNode->m_Index < pOtherEdge->m_pOutboundNode->m_Index ||
			(pEdge->m_pOutboundNode->m_Index == pOtherEdge->m_pOutboundNode->m_Index && pEdge->m_pInboundNode->m_Index <= pOtherEdge->m_pInboundNode->m_Index);
	}

	NOALIAS inline bool C_ConstraintTwin::operator > (const C_ConstraintTwin& other) const
	{
		C_ConstraintEdge* pEdge = C_ConstraintEdge::GetConstraintEdge(this);
		C_ConstraintEdge* pOtherEdge = C_ConstraintEdge::GetConstraintEdge(&other);
		return pEdge->m_pOutboundNode->m_Index > pOtherEdge->m_pOutboundNode->m_Index ||
			(pEdge->m_pOutboundNode->m_Index == pOtherEdge->m_pOutboundNode->m_Index && pEdge->m_pInboundNode->m_Index > pOtherEdge->m_pInboundNode->m_Index);
	}

	NOALIAS inline bool C_ConstraintTwin::operator >= (const C_ConstraintTwin& other) const
	{
		C_ConstraintEdge* pEdge = C_ConstraintEdge::GetConstraintEdge(this);
		C_ConstraintEdge* pOtherEdge = C_ConstraintEdge::GetConstraintEdge(&other);
		return pEdge->m_pOutboundNode->m_Index > pOtherEdge->m_pOutboundNode->m_Index ||
			(pEdge->m_pOutboundNode->m_Index == pOtherEdge->m_pOutboundNode->m_Index && pEdge->m_pInboundNode->m_Index >= pOtherEdge->m_pInboundNode->m_Index);
	}

	NOALIAS inline bool C_ConstraintTwin::operator == (const C_ConstraintTwin& other) const
	{
		C_ConstraintEdge* pEdge = C_ConstraintEdge::GetConstraintEdge(this);
		C_ConstraintEdge* pOtherEdge = C_ConstraintEdge::GetConstraintEdge(&other);
		return *pOtherEdge == *pEdge;
	}

	NOALIAS inline bool C_ConstraintTwin::operator != (const C_ConstraintTwin& other) const
	{
		C_ConstraintEdge* pEdge = C_ConstraintEdge::GetConstraintEdge(this);
		C_ConstraintEdge* pOtherEdge = C_ConstraintEdge::GetConstraintEdge(&other);
		return *pOtherEdge != *pEdge;
	}

	//Links the two graph nodes together. Requires 2 constraints because 
	void LinkPersistent(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB);

	//Links the two graph nodes together. Requires 2 constraints because 
	void LinkPersistent(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA);

	void LinkVolatile(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB);

	void LinkClose(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB);

	void AddTwinConstraint(C_GraphNode* pNodeB, C_ConstraintTwin* pConstraintB);

	void RemoveTwinConstraint(C_GraphNode* pNodeB, C_ConstraintTwin* pConstraintB);

	void LinkVolatile(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA);

	void Unlink(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA, C_ConstraintTwin* pConstraintB);

	void Unlink(C_GraphNode* pNodeA, C_GraphNode* pNodeB, C_ConstraintEdge* pConstraintA);

};



#endif

