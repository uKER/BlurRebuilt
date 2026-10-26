#ifndef NEWCOLLIDER_H
#define NEWCOLLIDER_H

#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/sharedphysics/ContactStreamIterator.h"
#include "gravity/sharedphysics/ConstraintGraph.h"
#include "gravity/narrowphase/PersistentContactManifold.h"

#define USE_PERSISTENT_MANIFOLD

namespace Collision
{
	class C_Shape;
};

namespace SharedPhysics
{

/*
 * This represents an instance of collision detection involving a convex hull.
 * This is stored as a linked list so that it is also valid for a composite shape constructed from
 * one or more convex hulls
 */
//class C_CollisionInstance : public C_BlockItem<C_CollisionInstance>
//{
//public:
//	C_CollisionInstance() : m_pPrev(NULL)
//	{
//	}
//
//	NOALIAS inline bool MatchingInstance(Collision::C_Shape* pShapeA, Collision::C_Shape* pShapeB)
//	{
//		return (pShapeA == m_pShapeA && pShapeB == m_pShapeB) 
//			|| (pShapeA == m_pShapeB && pShapeB == m_pShapeA);
//	}
//
//	Collision::C_Shape* m_pShapeA;
//	Collision::C_Shape* m_pShapeB;
//	C_CollisionInstance* m_pPrev;
//	Collision::C_PersistentContactManifold m_PersistentManifold;
//	Maths::HMat43 m_PosRotA;
//	Maths::HMat43 m_PosRotB;
//};

class ALIGN16 C_Collider //: public C_BlockItem<C_Collider>
{
public:
	enum E_ColliderState
	{
		E_Normal = 0,
		E_Removed = 1,
		E_Recycled = 2,
		E_ForceDword = 0xffffffff
	};

	enum E_ColliderType
	{
		E_NullCollision,
		E_RigidCollision,
		E_ClothCollision,
		E_RopeCollision,
		E_FluidCollision,
		E_CloseCollision
	};

//#ifdef USE_PERSISTENT_MANIFOLD
//	Collision::C_PersistentContactManifold m_PersistentManifold;
//#endif
	C_Collider* m_pPpuPtr;
	C_GraphNode* m_pNodeA, *m_pNodeB;
	S32 m_IndexA, m_IndexB;
	//Collider is a look-up into the contact stream, which stores the pair of object indices and
	//a C_ConstContactStreamPtr, which allows us to index into the contact stream 
	C_ConstContactStreamPtr m_StreamPtr;
	//Set internally when a contact is removed from the previous buffer and inserted into the current buffer!
	E_ColliderState m_State;
	//Sets the type of collision this is - used for end-of-frame activation/deactivation rules
	E_ColliderType m_Type;
	//The constraint edge which this collider is associated with!
	C_ConstraintEdge* m_pConstraintEdge;
	bool m_PersistentCollider;

public:
	C_Collider(C_GraphNode* pNodeA, C_GraphNode* pNodeB, bool bPersistentCollider = false) : m_pNodeA(pNodeA), m_pNodeB(pNodeB), 
		/*m_IndexA(pNodeA->m_Index), m_IndexB(pNodeB->m_Index),*/ m_State(E_Normal), m_Type(E_NullCollision), m_pConstraintEdge(NULL),
		m_PersistentCollider(bPersistentCollider)
	{
		m_pPpuPtr = this;
		BIZ_ASSERT(pNodeA != pNodeB);
		//if(!pNodeB->TestNodeFlags(C_GraphNode::E_Type_ZeroInertia))
		{
			m_IndexA = Maths::Min(pNodeA->m_Index, pNodeB->m_Index);
			m_IndexB = Maths::Max(pNodeA->m_Index, pNodeB->m_Index);
		}
	}

	C_Collider(bool bPersistentCollider = false) : m_pNodeA(NULL), m_pNodeB(NULL), m_State(E_Normal), m_Type(E_NullCollision), m_pConstraintEdge(NULL),
		m_PersistentCollider(bPersistentCollider)
	{
		m_pPpuPtr = this;
	}

	NOALIAS inline void SetNodes(C_GraphNode* nodeA, C_GraphNode* nodeB)
	{
		BIZ_ASSERT(nodeA != nodeB);
		m_pNodeA = nodeA;
		m_pNodeB = nodeB;

		/*if(nodeB->TestNodeFlags(C_GraphNode::E_Type_ZeroInertia))
		{
			m_IndexA = nodeA->m_Index;
			m_IndexB = nodeB->m_Index;
		}
		else*/
		{
			m_IndexA = Maths::Min(nodeA->m_Index, nodeB->m_Index);
			m_IndexB = Maths::Max(nodeA->m_Index, nodeB->m_Index);
		}
		m_State = E_Normal;
	}

	NOALIAS inline C_GraphNode* NodeA()
	{
		return m_pNodeA;
	}

	NOALIAS inline C_GraphNode* NodeB()
	{
		return m_pNodeB;
	}

	NOALIAS inline C_SingleContactIterator GetIterator()
	{
		return C_SingleContactIterator(&m_StreamPtr);
	}

	NOALIAS inline bool operator < (const C_Collider& other) const
	{
		return (m_IndexA < other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB < other.m_IndexB));
	}

	NOALIAS inline bool operator > (const C_Collider& other) const
	{
		return (m_IndexA > other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB > other.m_IndexB));
	}

	NOALIAS inline bool operator <= (const C_Collider& other) const
	{
		return (m_IndexA < other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB <= other.m_IndexB));
	}

	NOALIAS inline bool operator >= (const C_Collider& other) const
	{
		return (m_IndexA > other.m_IndexA) | ((m_IndexA == other.m_IndexA) & (m_IndexB >= other.m_IndexB));
	}

	NOALIAS inline bool operator == (const C_Collider& other) const
	{
		return (m_IndexA == other.m_IndexA) & (m_IndexB == other.m_IndexB);
	}

	NOALIAS inline bool operator != (const C_Collider& other) const
	{
		return (m_IndexA != other.m_IndexA) | (m_IndexB != other.m_IndexB);
	}
};

class C_PersistentContactCollider : public C_Collider
{
public:
	C_PersistentContactCollider() 
#ifdef USE_PERSISTENT_MANIFOLD
		: C_Collider(true)
#endif
	{
	}

#ifdef USE_PERSISTENT_MANIFOLD
	Collision::C_PersistentContactManifold m_PersistentManifold;
#endif
};
};

#endif
