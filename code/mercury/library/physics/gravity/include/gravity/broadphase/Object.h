#ifndef OBJECT_H
#define OBJECT_H

#include "gravity/narrowphase/AxisBox.h"
//#include "gravity/sharedphysics/ConstraintGraph.h"
//#include "gravity/narrowphase/CollisionResult.h"
//#include "gravity/broadphase/SweepAndPrune.h"
//Distribution node = KD-cell which object is in...

namespace Collision
{
	class C_Shape;
	class C_CollisionObject;
};

namespace SharedPhysics
{
	class C_GraphNode;
}

namespace Gravity
{
class C_DistributionNode;
class C_DistributionTree;
class C_Object;
//using namespace Collision;

class ALIGN16 C_ObjectPair
{
public:
	C_Object* m_pObjectA;
	C_Object* m_pObjectB;
};

class C_ObjectWrapper
{
public:

	C_ObjectWrapper() : m_pObject(NULL), m_pNode(NULL), m_pNext(NULL), m_pPrev(NULL), bDirty(false)
	{
	}
	C_Object* m_pObject;
	//C_SweepAndPruneObject m_SAPObject;
	C_DistributionNode* m_pNode;
	C_ObjectWrapper* m_pNext;
	C_ObjectWrapper* m_pPrev;
	bool bDirty;
};

typedef C_ObjectWrapper C_DistributionNodeWrapper;

enum E_ObjectState
{
	E_ActiveObject = 0,
	E_SleepingObject = 1,
	E_StaticObject= 2,
	E_KinematicObject = 3,
	E_ActiveClothObject = 4,
	E_SleepingClothObject = 5,
	E_ActiveFluidObject=6,
	E_SleepingFluidObject=7,
	E_FluidMeshObject = 8,
	E_RaytraceOnlyObject = 9,
	E_ActiveDebrisObject = 10,
	E_SleepingDebrisObject = 11,
	E_Dirty = 0x10,
	E_Removed = 0x100,
	E_NoCollision = 0x1000
};

#define NUM_DISTRIBUTION_NODES 8

class ALIGN16 C_Object
{
public:
	//The distribution nodes this object is contained within. Fixed to a maximum of 8 (which is 
	//correct provided the object is NOT LARGER than the volume of the distribution node)
	C_DistributionNodeWrapper m_Nodes[NUM_DISTRIBUTION_NODES];
	//Transformed, world-space bounding box
	Collision::C_CRAxisBox m_TransformedBox;
	//World-space axis box around the distribution nodes this object is contained in.
	Collision::C_AxisBoxMinMax m_NodeBox;


	U32 m_NumNodesContained;
	U32 m_ObjectState;

	Collision::C_Shape* m_pShape;

	//User data - in fact, this will be used for pointer back to the C_GraphNode in the physics data.
	SharedPhysics::C_GraphNode* m_pGraphNode;
	U32 m_Index;

	C_Object(const U32 index, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode) : 
	m_NumNodesContained(0), m_ObjectState(E_ActiveObject), m_pShape(pShape), m_pGraphNode(pGraphNode), m_Index(index)
	{
	}

	void Transform(const Maths::HMat43 PCREF matrix);

	NOALIAS FORCE_INLINE void MarkDirty()
	{
		m_ObjectState |= E_Dirty;
	}

	NOALIAS FORCE_INLINE void MakeKinematic();

	NOALIAS FORCE_INLINE bool IsKinematic()
	{
		return m_ObjectState == E_KinematicObject;
	}

	NOALIAS FORCE_INLINE bool ContainedInNode(C_DistributionNode* pNode);

	NOALIAS FORCE_INLINE void ClearDirtyFlag()
	{
		m_ObjectState &= ~E_Dirty;
	}

	NOALIAS inline void RemoveFrom(const U32 index) __restrict;

	bool IsUpdateRequired();

	NOALIAS inline void PutToSleep() __restrict;

	NOALIAS inline void WakeUp(C_DistributionTree* pTree) __restrict;

	NOALIAS inline void MakeStatic()__restrict;

	NOALIAS inline void MakeRaytraceOnly() __restrict;

	NOALIAS inline Collision::C_CollisionObject CollisionObject(const Maths::HF32 PCREF time);

	NOALIAS inline Collision::C_CollisionObject CollisionObject();

	NOALIAS inline bool operator < (const C_Object& other) const __restrict
	{
		return m_Index < other.m_Index;
	}

	NOALIAS inline bool operator <= (const C_Object& other) const __restrict
	{
		return m_Index <= other.m_Index;
	}

	NOALIAS inline bool operator > (const C_Object& other) const __restrict
	{
		return m_Index > other.m_Index;
	}

	NOALIAS inline bool operator >= (const C_Object& other) const __restrict
	{
		return m_Index >= other.m_Index;
	}

	NOALIAS inline bool operator == (const C_Object& other) const __restrict
	{
		return m_Index == other.m_Index;
	}

	NOALIAS inline bool operator != (const C_Object& other) const __restrict
	{
		return m_Index != other.m_Index;
	}
};
};

#endif
