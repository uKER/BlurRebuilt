#ifndef SOLVER2_CONSTRAINTMANAGER_H
#define SOLVER2_CONSTRAINTMANAGER_H

#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sharedphysics/ConstraintGraph.h"
#include "gravity/solver2/Jacobian.h"
#include "gravity/solver2/InvertMJT.h"
#include "gravity/sharedphysics/SortedArray.h"
#include "gravity/sharedphysics/SPU_Memory.h"
#include "gravity/sharedphysics/Array.h"
#include "system/perf.h"


namespace Solver2
{

template <typename Jacobian, typename InvertMJT>
class ALIGN16 C_ConstraintPair
{
public:
	S32 m_RigidBodyAIndex;
	S32 m_RigidBodyBIndex;
	F32 m_Slop;
	Jacobian m_Jacobian;
	InvertMJT m_InvertMJT;
	

	C_ConstraintPair() : m_RigidBodyAIndex(-1), m_RigidBodyBIndex(-1), m_Slop(0.f)
	{
	}

	SharedPhysics::C_Constraint::E_ConstraintType GetConstraintType()
	{
		Spu::MemBaseMarker marker;
		//C_Jacobian* j = (C_Jacobian*)Spu::FetchAlloc(m_pJacobian, sizeof(C_Jacobian));
		SharedPhysics::C_Constraint* pConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_Jacobian.m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		return pConstraint->m_ConstraintType;
	}

	void SetRigidBodyIndice(const U32 rigidBodyAIndex, const U32 rigidBodyBIndex)
	{
		m_RigidBodyAIndex = rigidBodyAIndex;
		m_RigidBodyBIndex = rigidBodyBIndex;
	}

	bool operator <= (const C_ConstraintPair<Jacobian, InvertMJT>& other) const
	{

		return(!m_Jacobian.m_pRigid2->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static) || 
			m_Jacobian.m_pRigid2->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static) && other.m_Jacobian.m_pRigid2->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static));
	}
	
	bool operator > (const C_ConstraintPair<Jacobian, InvertMJT>& other) const
	{
		return(m_Jacobian.m_pRigid2->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static) && !other.m_Jacobian.m_pRigid2->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static));
	}

	bool operator == (const C_ConstraintPair<Jacobian, InvertMJT>& other) const
	{
		return false;
	}

};

//A constraint stream - stores a single pair of jacobian
template <typename Jacobian, typename InvertMJT>
class C_ConstraintStream
{
public:

#ifdef PLATFORM_SPU
	C_ConstraintPair<Jacobian, InvertMJT> m_ConstraintPair;
	U32 m_DmaTag;
#endif
	C_ConstraintPair<Jacobian, InvertMJT>* m_pConstraintPair;

	C_ConstraintStream()
	{
	}

	/*C_ConstraintStream(const U32 tag) : m_DmaTag(tag)
	{
	}*/

	void SetConstraintPair(C_ConstraintPair<Jacobian, InvertMJT>* pPair)
	{
#ifdef PLATFORM_SPU
		Spu::FetchAsync(&m_ConstraintPair, pPair, sizeof(C_ConstraintPair<Jacobian, InvertMJT>), m_DmaTag);
#endif
		m_pConstraintPair = pPair;
	}

	void Wait()
	{
#ifdef PLATFORM_SPU
		Spu::Wait(m_DmaTag);
#endif
	}

	void WriteBackConstraint()
	{
#ifdef PLATFORM_SPU
		Spu::StoreAsync(m_pConstraintPair, &m_ConstraintPair, sizeof(C_ConstraintPair<Jacobian, InvertMJT>), m_DmaTag);
#endif
	}

	C_ConstraintPair<Jacobian, InvertMJT>* GetConstraintPair()
	{
#ifdef PLATFORM_SPU
		return &m_ConstraintPair;
#else
		return m_pConstraintPair;
#endif
	}
};

//const U32 StreamerLength = 2;
//const U32 MultipleFrontWaitMask = ( 2 | 4 );
//const U32 MultipleBackWaitMask = ( 8 | 16);
//const U32 MultipleWaitMask = ( 2 | 4 | 8 | 16 );

const U32 StreamerLength = 4;
const U32 MultipleFrontWaitMask = ( 2 | 4 | 8 | 16 );
const U32 MultipleBackWaitMask = ( 32 | 64 | 128 | 256);
const U32 MultipleWaitMask = ( 2 | 4 | 8 | 16 | 32 | 64 | 128 | 256);

template <typename Jacobian, typename InvertMJT>
class C_ConstraintStreamer
{
public:
	/*C_ConstraintStream<Jacobian, InvertMJT> m_FrontStreams[StreamerLength];
	C_ConstraintStream<Jacobian, InvertMJT> m_BackStreams[StreamerLength];*/
	C_ConstraintStream<Jacobian, InvertMJT> m_Streams[2][StreamerLength];
	U32 m_CurrentBuffer;
	U32 m_CurrentStream;

	C_ConstraintStreamer()
	{
		Initialise();
	}

	void Initialise()
	{
#ifdef PLATFORM_SPU
		for(U32 a = 0; a < StreamerLength; ++a)
		{
			m_Streams[0][a].m_DmaTag = a + 1;
			m_Streams[1][a].m_DmaTag = StreamerLength + a + 1;
		}
#endif
		m_CurrentStream = m_CurrentBuffer = 0;
	}

	C_ConstraintStream<Jacobian, InvertMJT>* CurrentStream()
	{
		return &m_Streams[m_CurrentBuffer][m_CurrentStream];
	}

	void FlipStream()
	{
		m_CurrentStream = (m_CurrentStream + 1) % StreamerLength;
	}

	C_ConstraintStream<Jacobian, InvertMJT>* CurrentStreamBuffer()
	{
		return m_Streams[m_CurrentBuffer];
	}

	C_ConstraintStream<Jacobian, InvertMJT>* BackStreamBuffer()
	{
		return m_Streams[1 - m_CurrentBuffer];
	}

	void FlipStreamBuffer()
	{
		/*C_ConstraintStream<Jacobian, InvertMJT>* temp = &m_FrontStreams;
		&m_FrontStreams = &m_BackStreams;
		&m_BackStreams = temp;*/
		m_CurrentBuffer = 1 - m_CurrentBuffer;
	}

	void WaitFrontAll()
	{
#ifdef PLATFORM_SPU
		const U32 selector = ~(((U32)m_CurrentBuffer)-1);
		const U32 waitMask( (selector & MultipleBackWaitMask) | ((~selector) * MultipleFrontWaitMask));
		Spu::WaitMultiple(waitMask);
		//Spu::WaitMultiple(MultipleWaitMask);
#endif
	}

	void WaitBackAll()
	{
#ifdef PLATFORM_SPU
		const U32 selector = ~(((U32)m_CurrentBuffer)-1);
		const U32 waitMask( (selector & MultipleFrontWaitMask) | ((~selector) * MultipleBackWaitMask));
		Spu::WaitMultiple(waitMask);
		//Spu::WaitMultiple(MultipleWaitMask);
#endif
	}

	void WaitAll()
	{
#ifdef PLATFORM_SPU
		Spu::WaitMultiple(MultipleWaitMask);
#endif
	}
};

#define LARGE_BUFFER 256
#define MEDIUM_BUFFER 64
#define SMALL_BUFFER 16


template <typename Jacobian, typename InvertMJT>
inline void AddToEnd(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT> * >& list, C_ConstraintPair<Jacobian, InvertMJT>* pPair)
{
#ifdef PLATFORM_SPU
	if(list.m_numElements == list.m_size)
	{
		list.Resize(list.m_numElements * 2 + 16);
	}
	Spu::Set((U32*)list.m_pElems + list.m_numElements, (U32)pPair);
	list.m_numElements++;
#else
	list.AddToEnd(&pPair);
#endif
}


class ConstraintManager
{
public:
	ConstraintManager(SharedPhysics::C_StaticSPUMemoryAllocator* m_pAllocator) : 
	  m_MemoryPool(m_pAllocator), m_ContactPairs(&m_MemoryPool, LARGE_BUFFER), 
		  m_StaticContactPairs(&m_MemoryPool, LARGE_BUFFER), m_SphericalPairs(&m_MemoryPool, MEDIUM_BUFFER), 
		  m_SphericalLimitPairs(&m_MemoryPool, SMALL_BUFFER), m_RevolutePairs(&m_MemoryPool, SMALL_BUFFER), 
		  m_RevoluteLimitPairs(&m_MemoryPool, SMALL_BUFFER), m_PrismaticPairs(&m_MemoryPool, SMALL_BUFFER),
			m_PrismaticLimitPairs(&m_MemoryPool, SMALL_BUFFER), m_CarWheelPairs(&m_MemoryPool, SMALL_BUFFER), 
			m_FixedJointPairs(&m_MemoryPool, SMALL_BUFFER),m_DistancePairs(&m_MemoryPool, SMALL_BUFFER), 
			m_TwoRotationalPairs(&m_MemoryPool, SMALL_BUFFER)//,m_AngularPairs(&m_MemoryPool, SMALL_BUFFER) 
	{
	}

	~ConstraintManager()
	{
	}

	/*template <typename Jacobian, typename InvertMJT>
	void ProcessIslandIndices(C_ConstraintStream<Jacobian, InvertMJT>* buffer);*/

	template <typename Jacobian, typename InvertMJT>
	void SetIslandIndices(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>*>& constraints);

	////Construct Jacobian for joints
	//template <typename Constraint, typename Jacobian, typename InvertMJT>
	//void ConstructJacobian(SharedPhysics::C_Constraint* ppuConstraint,
	//	SharedPhysics::C_GraphNode* node1, SharedPhysics::C_GraphNode* node2, /*C_Jacobian** jacobian, C_InvertMJT** invMJT,*/
	//	SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints);

	////Construct Jacobian for the contacts
	//template <typename Constraint, typename Jacobian, typename InvertMJT>
	//void ConstructJacobian(SharedPhysics::C_Constraint* ppuConstraint,
	//	SharedPhysics::C_GraphNode* node1, SharedPhysics::C_GraphNode* node2,/* C_Jacobian** jacobian, C_InvertMJT** invMJT,*/
	//	SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& staticConstraints);

	template <typename Jacobian, typename InvertMJT>
	void ProcessBVecWarmDiagonalValue(C_ConstraintStream<Jacobian, InvertMJT>* buffer, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, SharedPhysics::Array<Maths::Vector6>& bAuxiliary);


	template <typename Jacobian, typename InvertMJT>
	void ProcessConstraint(C_ConstraintStream<Jacobian, InvertMJT>* buffer, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, Maths::VF32& totalDif, Maths::VF32& newTotDif);

	//Construct Jacobian for joints
	template <typename Constraint, typename Jacobian, typename InvertMJT>
	void ConstructJacobian(SharedPhysics::C_Constraint* ppuConstraint,
		Gravity::C_ConstraintBody* supConstraintBody1, Gravity::C_ConstraintBody* spuConstraintBody2, 
		SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints);

	//Construct Jacobian for the contacts
	template <typename Constraint, typename Jacobian, typename InvertMJT>
	void ConstructJacobian(SharedPhysics::C_Constraint* ppuConstraint,
		Gravity::C_ConstraintBody* spuConstraintBody1, Gravity::C_ConstraintBody* spuConstraintBody2,
		SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& staticConstraints);

	template <typename Jacobian, typename InvertMJT>
	void StoreWarmStartValues(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& arr);

	template <typename Jacobian, typename InvertMJT, typename Vector>
	Maths::VF32 UpdateConstraint(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, const Maths::VF32 PCREF totalDif);

	template <typename Jacobian, typename InvertMJT, typename Vector>
	Maths::VF32 UpdateStaticConstraint(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, const Maths::VF32 PCREF totalDif);

	template <typename Jacobian, typename InvertMJT>
	void BVecWarmDiagonal(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, 
		SharedPhysics::Array<Maths::Vector6>& aAuxiliary/*, SharedPhysics::Array<Maths::Vector6>& bAuxiliary*/);


	void BVecWarmDiagonal(SharedPhysics::Array<Maths::Vector6>& aAuxiliary/*, SharedPhysics::Array<Maths::Vector6>& bAuxiliary*/);


	void StoreWarmStartValues();


	Maths::VF32 UpdateConstraints(SharedPhysics::Array<Maths::Vector6>& aAuxiliary);
	Maths::VF32 UpdateConstraintsNoJoints(SharedPhysics::Array<Maths::Vector6>& aAuxiliary);

	void Reset();

//#ifndef PLATFORM_SPU
//	template <typename Serialiser>
//	static void SaveState(SharedPhysics::C_Constraint* pConstraint, Serialiser& serial);
//
//	template <typename Deserialiser>
//	static void LoadState(SharedPhysics::C_Constraint* pConstraint, Deserialiser& serial);
//#endif


	void CreateJacobianInvMJT(SharedPhysics::C_Constraint* constraint, Gravity::C_ConstraintBody* spuConstraintBody1, Gravity::C_ConstraintBody* spuConstraintBody2);


	//There are different types of constraint, check for the constraint
	//type and return the correct value for them

	//SharedPhysics::C_GenericStackAllocator<65536>* m_pStackAllocator;

	SharedPhysics::C_SPUMemoryPool m_MemoryPool;

	SharedPhysics::C_SPUCache m_Cache;

	SharedPhysics::SortedArray<C_ConstraintPair<C_ContactJacobian,				C_ContactInvertMJT>* > m_ContactPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_ContactJacobian,				C_ContactInvertMJT>* > m_StaticContactPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_SphericalJointJacobian,		C_SphericalJointInvertMJT>* > m_SphericalPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_SphericalJointLimitJacobian,	C_SphericalJointLimitInvertMJT>* > m_SphericalLimitPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_RevoluteJointJacobian,		C_RevoluteJointInvertMJT>* > m_RevolutePairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_RevoluteJointLimitJacobian,	C_RevoluteJointLimitInvertMJT>* > m_RevoluteLimitPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_PrismaticJointJacobian,		C_PrismaticJointInvertMJT>* > m_PrismaticPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_PrismaticJointLimitJacobian,	C_PrismaticJointLimitInvertMJT>* > m_PrismaticLimitPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_CarWheelJointJacobian,		C_CarWheelJointInvertMJT>* > m_CarWheelPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_FixedRotationalJacobian,		C_FixedRotationalInvertMJT>* > m_FixedJointPairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_DistanceJacobian,				C_DistanceInvertMJT>* > m_DistancePairs;
	SharedPhysics::SortedArray<C_ConstraintPair<C_TwoRotationalJacobian,		C_TwoRotationalInvertMJT>* > m_TwoRotationalPairs;

};
	

};


#endif
