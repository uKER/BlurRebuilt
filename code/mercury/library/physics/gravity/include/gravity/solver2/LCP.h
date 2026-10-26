#ifndef SOLVER2_LCP_H
#define SOLVER2_LCP_H

#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/sharedphysics/ConstraintGraph.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sharedphysics/Array.h"
#include "gravity/sharedphysics/SafeStack.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/solver2/ConstraintManager.h"
#include "gravity/sharedphysics/spu_memory.h"


namespace Solver2
{

class LCP
{
public:

	LCP(const F32 beta, const F32 slop, const U32 numIterations, const U32 jointIterationFrequency, SharedPhysics::C_StaticSPUMemoryAllocator* pAllocator,
		SharedPhysics::C_GraphNode* pNullNode, SharedPhysics::C_RigidBody* pNullBody): 
		manager(pAllocator), 
#if 1 //ndef PLATFORM_SPU
		/*bAuxiliary(&manager.m_MemoryPool),*/ aAuxiliary(&manager.m_MemoryPool), 
#endif
		
		m_RigidBodies(&manager.m_MemoryPool), 
		  m_NullBody(pNullNode), m_NullRigid(pNullBody), m_NumContacts(0), m_Beta(beta), 
		  m_NumIteration(numIterations), m_JointIterationFrequency(jointIterationFrequency), m_Slop(slop)
	{
	}

	//void LCPSolver(SharedPhysics::C_GraphNode* root, SharedPhysics::C_SafeStack<true>& stack);
	void LCPSolver(SharedPhysics::C_GraphNode* root);
	void SetBaumgarteParameter(const F32 beta)
	{
		m_Beta=beta;
	}
	void SetNumIteration(const U32 numIteration)
	{
		m_NumIteration=numIteration;
	}
	void SetTimeStep(const F32 time)
	{
		m_Time=time;
	}

	U32 ConstructJacobianAndInvertMJT(SharedPhysics::C_GraphNode* root, SharedPhysics::C_SafeStack<true>& stack);
	//void PostStabilisation(SharedPhysics::C_GraphNode* root, SharedPhysics::C_SafeStack<true>& stack);
	void ConstructAuxiliaryB();
	void Solve();
	bool Validate();
	void Update();

	ConstraintManager manager;
	//SharedPhysics::Array<F32> bAuxiliary;	//v0 + time*Inv(M)*Fext bnX1, per object
	//SharedPhysics::Array<F32> aAuxiliary;	//time*Inv(M)*Tra(J)*lambda, 6nX1, per object
	//SharedPhysics::Array<Maths::Vector6> bAuxiliary; //v0 + time*Inv(M)*Fext bnX1, per object
	SharedPhysics::Array<Maths::Vector6> aAuxiliary; //time*Inv(M)*Tra(J)*lambda, 6nX1, per object
	SharedPhysics::SortedArray<SharedPhysics::C_GraphNode*> m_RigidBodies;

	SharedPhysics::C_GraphNode* m_NullBody;
	SharedPhysics::C_RigidBody* m_NullRigid;

	U32 m_NumContacts;
	U32 m_NumContactsX6;
	U32 m_NumObjectsX6;
	U32 m_NumIteration;
	U32 m_JointIterationFrequency;
	F32 m_Beta; // the baumgarte parameter, which should be in the range of [1, 1/time]
	F32 m_Time;
	F32 m_Slop;
};

};


#endif
