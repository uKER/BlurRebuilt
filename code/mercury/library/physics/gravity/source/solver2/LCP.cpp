#include <stdafx.h>
#include "gravity/solver2/LCP.h"
#include "gravity/sharedphysics/MemoryAllocator.h"
#include "gravity/sharedphysics/ConstraintGraph.h"
#include "gravity/sharedphysics/MemoryAllocator.inl"
#include "gravity/solver2/ConstraintManager.inl"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/solver2/constraint.h"
#include "gravity/solver2/jacobian.h"
#include "system/perf.h"

//#ifdef USER_KSTOREY
//#define TRAP_HUGE_VALUES
//#endif


using namespace Maths;
using namespace SharedPhysics;

//#define ENABLE_RE_RUNS

#ifndef PLATFORM_SPU
Debug::Float g_dbExitThreshold("Gravity.Solver.ExitThreshold", 1e-10f, Debug::Range(0.f, 0.001f));
#else
static float g_dbExitThreshold = 1e-10f;
#endif

#ifndef PLATFORM_SPU
Debug::Logger &g_PhysLog( Debug::GetLogger("mercury.gravity.validation") );
#endif

namespace Solver2
{
	
void AddToArray(SharedPhysics::SortedArray<SharedPhysics::C_GraphNode*>& bodies, SharedPhysics::C_GraphNode* pBody)
{
#ifdef PLATFORM_SPU
	if(bodies.m_numElements == bodies.m_size)
	{
		bodies.Resize(bodies.m_numElements * 2 + 16);
	}
	Spu::Set((U32*)bodies.m_pElems + bodies.m_numElements, (U32)pBody);
	bodies.m_numElements++;
#else
	bodies.AddToEnd(&pBody);
#endif
}

//void LCP::PostStabilisation(C_GraphNode* root, SharedPhysics::C_SafeStack<true>& stack)
//{
//	S32 size = 1;
//	stack.Push(root, 0);
//	C_GraphNode* stackTop(root);
//	
//
//	while(size--)
//	{
//		Spu::MemBaseMarker outerMarker;
//		C_GraphNode* node1 = stackTop;
//		C_GraphNode* fetchedNode = (C_GraphNode*)Spu::FetchAlloc(node1, sizeof(C_GraphNode));
//		//if(size > 0)
//		stackTop = stack.Pop<C_GraphNode>(Maths::Max(size-1, 0));
//
//		C_ConstraintEdge* edge=fetchedNode->m_pPersistentConstraints;
//		bool swap = false;
//
//		while(edge)
//		{
//			Spu::MemBaseMarker innerMarker;
//			C_ConstraintEdge* fetchedEdge = (C_ConstraintEdge*)Spu::FetchAlloc(edge, sizeof(C_ConstraintEdge));
//			if(!fetchedEdge->TestEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing2))
//			{
//				fetchedEdge->SetEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing2);
//				C_GraphNode* node2=fetchedEdge->m_pOutboundNode;
//				C_GraphNode* fetchedNode2 = (C_GraphNode*)Spu::FetchAlloc(node2, sizeof(C_GraphNode));
//				//C_RigidBody* rigid1=node1->m_pRigidBody;
//				//C_RigidBody* rigid2=node2->m_pRigidBody;
//				C_ConstraintPoint* pConstraint = fetchedEdge->m_pFirstConstraint;
//				while(pConstraint)
//				{
//					Spu::MemBaseMarker innerInnerMarker;
//					C_ConstraintPoint* spuConstraintPoint = (C_ConstraintPoint*)Spu::FetchAllocUnaligned(pConstraint, sizeof(C_ConstraintPoint));
//					C_Constraint* constraint=spuConstraintPoint->m_pConstraint;
//					if(node2->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
//					{
//						C_GraphNode* node = node1;
//						node1 = node2;
//						node2 = node;
//						swap = true;
//					}
//					manager.PostStabilisation(constraint, node1, node2, swap);
//					swap = false;
//					
//					pConstraint = spuConstraintPoint->m_pNext;
//				}
//				bool b(!fetchedNode2->TestNodeFlags( C_GraphNode::E_Type_Visited_Island_Processing));
//				U32 incrementor((U32)b);
//				fetchedNode2->SetNodeFlags( C_GraphNode::E_Type_Visited_Island_Processing );
//				stack.Push(node2, size);
//				stackTop = Fast_Select<C_GraphNode>(b, node2, stackTop);
//	#ifdef PLATFORM_SPU
//				Spu::Store(node2, fetchedNode2, sizeof(C_GraphNode));
//				Spu::Store(edge, fetchedEdge, sizeof(C_ConstraintEdge));
//	#endif
//				size += incrementor;
//			}
//			edge = fetchedEdge->m_pNext;
//		}
//		SharedPhysics::C_ConstraintTwin* pTwins = fetchedNode->m_pPersistentTwinConstraints;
//		while(pTwins)
//		{
//			Spu::MemBaseMarker innerMarker;
//			C_ConstraintTwin* spuTwin = (C_ConstraintTwin*)Spu::FetchAllocUnaligned(pTwins, sizeof(C_ConstraintTwin));
//			edge = spuTwin->m_pEdge;
//			C_ConstraintEdge* fetchedEdge = (C_ConstraintEdge*)Spu::FetchAlloc(edge, sizeof(C_ConstraintEdge));
//			if(!fetchedEdge->TestEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing2))
//			{
//				fetchedEdge->SetEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing2);
//				C_GraphNode* node2=fetchedEdge->m_pInboundNode;
//				C_GraphNode* fetchedNode2 = (C_GraphNode*)Spu::FetchAlloc(node2, sizeof(C_GraphNode));
//				//C_RigidBody* rigid1=node1->m_pRigidBody;
//				//C_RigidBody* rigid2=node2->m_pRigidBody;
//				C_ConstraintPoint* pConstraint = fetchedEdge->m_pFirstConstraint;
//				while(pConstraint)
//				{
//					Spu::MemBaseMarker innerInnerMarker;
//					C_ConstraintPoint* spuConstraintPoint = (C_ConstraintPoint*)Spu::FetchAllocUnaligned(pConstraint, sizeof(C_ConstraintPoint));
//					C_Constraint* constraint=spuConstraintPoint->m_pConstraint;
//					
//					manager.PostStabilisation(constraint, node2, node1, false);
//					pConstraint = spuConstraintPoint->m_pNext;
//				}
//				bool b(!fetchedNode2->TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Processing));
//				U32 incrementor((U32)b);
//				fetchedNode2->SetNodeFlags( C_GraphNode::E_Type_Visited_Island_Processing );
//				stack.Push(node2, size);
//				stackTop = Fast_Select<C_GraphNode>(b, node2, stackTop);
//	#ifdef PLATFORM_SPU
//				Spu::Store(node2, fetchedNode2, sizeof(C_GraphNode));
//				Spu::Store(edge, fetchedEdge, sizeof(C_ConstraintEdge));
//	#endif
//				size += incrementor;
//			}
//			pTwins = spuTwin->m_pNext;
//		}
//	}
//
//}

U32 LCP::ConstructJacobianAndInvertMJT(C_GraphNode* root, SharedPhysics::C_SafeStack<true> & stack)
{
	PERF_EVENT("gravity.solver.ConstructJacobian");
	S32 size = 1;
	stack.Push(root, 0);
	C_GraphNode* stackTop(root);
	m_RigidBodies.RestoreDataBlocks();
	S32 localIndex = 1;
	AddToArray(m_RigidBodies, m_NullBody);
	S32 numContacts = 0;

	{
		AddToArray(m_RigidBodies, root);
		U32 nodeFlags = Spu::Get(&root->m_NodeFlags) | SharedPhysics::C_GraphNode::E_Type_Visited_Island_Processing;
		Spu::Set(&root->m_NodeFlags, nodeFlags);
		Spu::Set((U32*)&root->m_IslandIndex, (U32)localIndex++);
	}

	
	/*SharedPhysics::C_MemoryIterator<Solver::C_ContactJacobian, 32> jacoIter(&jaco);
	SharedPhysics::C_MemoryIterator<Solver::C_ContactInvertMJT, 32> imjtIter(&imjt);*/
	while(size--)
	{
		Spu::MemBaseMarker outerMarker;
		C_GraphNode* node1 = stackTop; 
		Gravity::C_ConstraintBody* spuLcpBody1 = (Gravity::C_ConstraintBody*)Spu::FetchAlloc(node1, sizeof(Gravity::C_ConstraintBody));
		stackTop = stack.Pop<C_GraphNode>(Maths::Max(size-1, 0));
		BIZ_ASSERT(spuLcpBody1->m_GraphNode.m_IslandIndex < localIndex);

		//Won't travel from a sleeping/static/kinematic node to it's neighbours - only from an active node!

		if(spuLcpBody1->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			C_ConstraintEdge* edge=spuLcpBody1->m_GraphNode.m_pVolatileConstraints;
			while(edge)
			{
				Spu::MemBaseMarker innerMarker;
				C_ConstraintEdge* fetchedEdge = (C_ConstraintEdge*)Spu::FetchAlloc(edge, sizeof(C_ConstraintEdge));
				if(!fetchedEdge->TestEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing))
				{
					fetchedEdge->SetEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);
					C_GraphNode* node2=fetchedEdge->m_pOutboundNode;
					Gravity::C_ConstraintBody* spuLcpBody2 = (Gravity::C_ConstraintBody*)Spu::FetchAlloc(node2, sizeof(Gravity::C_ConstraintBody));
					C_ConstraintPoint* pConstraint = fetchedEdge->m_pFirstConstraint;

					if(spuLcpBody2->m_GraphNode.m_IslandIndex < 0)
					{
						AddToArray(m_RigidBodies, node2);
						spuLcpBody2->m_GraphNode.m_IslandIndex = localIndex++;
					}

					while(pConstraint)
					{
						Spu::MemBaseMarker innerInnerMarker;
						C_ConstraintPoint* spuConstraintPoint = (C_ConstraintPoint*)Spu::FetchAllocUnaligned(pConstraint, sizeof(C_ConstraintPoint));
						C_Constraint* constraint=spuConstraintPoint->m_pConstraint;
						manager.ConstructJacobian<C_ContactConstraint, C_ContactJacobian, C_ContactInvertMJT>(constraint, spuLcpBody1, spuLcpBody2, manager.m_ContactPairs, manager.m_StaticContactPairs);
						++numContacts;
						pConstraint = spuConstraintPoint->m_pNext;
					}
					bool b(!spuLcpBody2->m_GraphNode.TestNodeFlags( C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_ZeroInertia ));
					U32 incrementor((U32)b);
					spuLcpBody2->m_GraphNode.SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Processing);
					stack.Push(node2, size);
					stackTop = Fast_Select<C_GraphNode>(b, node2, stackTop);
#ifdef PLATFORM_SPU
					Spu::StoreAsync(node2, spuLcpBody2, sizeof(C_GraphNode), 10);
					Spu::StoreAsync(edge, fetchedEdge, sizeof(C_ConstraintEdge), 10);
					Spu::Wait(10);
#endif
					size += incrementor;
				}
				edge = fetchedEdge->m_pNext;
			}
			C_ConstraintTwin* pTwins = spuLcpBody1->m_GraphNode.m_pVolatileTwinConstraints;
			while(pTwins)
			{
				Spu::MemBaseMarker innerMarker;
				edge = C_ConstraintEdge::GetConstraintEdge(pTwins);
				C_ConstraintEdge* fetchedEdge = (C_ConstraintEdge*)Spu::FetchAlloc(edge, sizeof(C_ConstraintEdge));
				C_ConstraintTwin* spuTwin = &fetchedEdge->m_TwinConstraint;
				if(!fetchedEdge->TestEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing))
				{
					fetchedEdge->SetEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);
					C_GraphNode* node2=fetchedEdge->m_pInboundNode;
					Gravity::C_ConstraintBody* spuLcpBody2 = (Gravity::C_ConstraintBody*) Spu::FetchAlloc(node2, sizeof(Gravity::C_ConstraintBody));
					if(spuLcpBody2->m_GraphNode.m_IslandIndex < 0)
					{
						AddToArray(m_RigidBodies, node2);
						spuLcpBody2->m_GraphNode.m_IslandIndex = localIndex++;
					}
				
					C_ConstraintPoint* pConstraint = fetchedEdge->m_pFirstConstraint;
					while(pConstraint)
					{
						Spu::MemBaseMarker innerInnerMarker;
						C_ConstraintPoint* spuConstraintPoint = (C_ConstraintPoint*)Spu::FetchAllocUnaligned(pConstraint, sizeof(C_ConstraintPoint));
						C_Constraint* constraint=spuConstraintPoint->m_pConstraint;
						manager.ConstructJacobian<C_ContactConstraint, C_ContactJacobian, C_ContactInvertMJT>(constraint, spuLcpBody2, spuLcpBody1, manager.m_ContactPairs, manager.m_StaticContactPairs);
						++numContacts;
						pConstraint = spuConstraintPoint->m_pNext;
					}
					bool b(!spuLcpBody2->m_GraphNode.TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_ZeroInertia));
					U32 incrementor((U32)b);
					spuLcpBody2->m_GraphNode.SetNodeFlags( C_GraphNode::E_Type_Visited_Island_Processing);
					stack.Push(node2, size);
					stackTop = Fast_Select<C_GraphNode>(b, node2, stackTop);
#ifdef PLATFORM_SPU
					Spu::StoreAsync(node2, spuLcpBody2, sizeof(SharedPhysics::C_GraphNode), 10);
					Spu::StoreAsync(edge, fetchedEdge, sizeof(C_ConstraintEdge), 10);
					Spu::Wait(10);
#endif
					size += incrementor;
				}
				pTwins = spuTwin->m_pNext;
			}

			edge = spuLcpBody1->m_GraphNode.m_pPersistentConstraints;
			while(edge)
			{
				Spu::MemBaseMarker innerMarker;
				C_ConstraintEdge* fetchedEdge = (C_ConstraintEdge*)Spu::FetchAlloc(edge, sizeof(C_ConstraintEdge));
				if(!fetchedEdge->TestEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing))
				{
					fetchedEdge->SetEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);
					C_GraphNode* node2=fetchedEdge->m_pOutboundNode;
					Gravity::C_ConstraintBody* spuLcpBody2 = (Gravity::C_ConstraintBody*)Spu::FetchAlloc(node2, sizeof(Gravity::C_ConstraintBody));
					if(spuLcpBody2->m_GraphNode.m_IslandIndex < 0)
					{
						AddToArray(m_RigidBodies, node2);
						spuLcpBody2->m_GraphNode.m_IslandIndex = localIndex++;
					}

					if(!(spuLcpBody1->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia) && 
						spuLcpBody2->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia)))
					{
						C_ConstraintPoint* pConstraint = fetchedEdge->m_pFirstConstraint;
						while(pConstraint)
						{
							Spu::MemBaseMarker innerInnerMarker;
							C_ConstraintPoint* spuConstraintPoint = (C_ConstraintPoint*)Spu::FetchAllocUnaligned(pConstraint, sizeof(C_ConstraintPoint));
							C_Constraint* constraint=spuConstraintPoint->m_pConstraint;
							manager.CreateJacobianInvMJT(constraint, spuLcpBody1, spuLcpBody2);
							++numContacts;
							pConstraint = spuConstraintPoint->m_pNext;
						}
						bool b(!spuLcpBody2->m_GraphNode.TestNodeFlags( C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_ZeroInertia));
						U32 incrementor((U32)b);
						spuLcpBody2->m_GraphNode.SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Processing);
						stack.Push(node2, size);
						stackTop = Fast_Select<C_GraphNode>(b, node2, stackTop);
	#ifdef PLATFORM_SPU
						Spu::StoreAsync(node2, spuLcpBody2, sizeof(SharedPhysics::C_GraphNode), 10);
						Spu::StoreAsync(edge, fetchedEdge, sizeof(C_ConstraintEdge), 10);
						Spu::Wait(10);
	#endif
						size += incrementor;
					}
				}
				edge = fetchedEdge->m_pNext;
			}
			pTwins = spuLcpBody1->m_GraphNode.m_pPersistentTwinConstraints;

			while(pTwins)
			{
				Spu::MemBaseMarker innerMarker;

				edge = C_ConstraintEdge::GetConstraintEdge(pTwins);
				C_ConstraintEdge* fetchedEdge = (C_ConstraintEdge*)Spu::FetchAlloc(edge, sizeof(C_ConstraintEdge));
				C_ConstraintTwin* spuTwin = &fetchedEdge->m_TwinConstraint;

				if(!fetchedEdge->TestEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing))
				{
					fetchedEdge->SetEdgeFlags(C_ConstraintEdge::E_Visited_Island_Processing);
					C_GraphNode* node2=fetchedEdge->m_pInboundNode;
					Gravity::C_ConstraintBody* spuLcpBody2 = (Gravity::C_ConstraintBody*)Spu::FetchAlloc(node2, sizeof(Gravity::C_ConstraintBody));
					if(spuLcpBody2->m_GraphNode.m_IslandIndex < 0)
					{
						AddToArray(m_RigidBodies, node2);
						spuLcpBody2->m_GraphNode.m_IslandIndex = localIndex++;
					}

					if(!(spuLcpBody1->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia) && 
						spuLcpBody2->m_GraphNode.TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia)))
					{
						C_ConstraintPoint* pConstraint = fetchedEdge->m_pFirstConstraint;
						while(pConstraint)
						{
							Spu::MemBaseMarker innerInnerMarker;
							C_ConstraintPoint* spuConstraintPoint = (C_ConstraintPoint*)Spu::FetchAllocUnaligned(pConstraint, sizeof(C_ConstraintPoint));
							C_Constraint* constraint=spuConstraintPoint->m_pConstraint;
							manager.CreateJacobianInvMJT(constraint, spuLcpBody2, spuLcpBody1);
							++numContacts;
							pConstraint = spuConstraintPoint->m_pNext;
						}
						bool b(!spuLcpBody2->m_GraphNode.TestNodeFlags(C_GraphNode::E_Type_Visited_Island_Processing | C_GraphNode::E_Type_ZeroInertia));
						U32 incrementor((U32)b);
						spuLcpBody2->m_GraphNode.SetNodeFlags(C_GraphNode::E_Type_Visited_Island_Processing);
						stack.Push(node2, size);
						stackTop = Fast_Select<C_GraphNode>(b, node2, stackTop);
	#ifdef PLATFORM_SPU
						Spu::StoreAsync(node2, spuLcpBody2, sizeof(C_GraphNode), 10);
						Spu::StoreAsync(edge, fetchedEdge, sizeof(C_ConstraintEdge), 10);
						Spu::Wait(10);
	#endif
						size += incrementor;
					}
				}
				pTwins = spuTwin->m_pNext;
			}
		}
	}
	return numContacts;
}


void LCP::ConstructAuxiliaryB()
{
	//PERF_EVENT("gravity.solver.AuxiliaryB");

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Maths::Vector6 vZero(Maths::Const<Maths::Vector6>::Zero());
	const Maths::HMat33 zeroMatrix(Maths::Constants<Maths::HMat33>::Zero());

//#ifndef PLATFORM_SPU
//	bAuxiliary[0] = Maths::Const<Maths::Vector6>::Zero();
//#else
//	tempData = Maths::Const<Maths::Vector6>::Zero();
//	Spu::Store(bAuxiliary.data, &tempData, sizeof(Maths::Vector6));
//#endif

	{
		//Maths::Vector6* bAux = (Maths::Vector6*)manager.m_Cache.CreateAlloc(&bAuxiliary[0], sizeof(Maths::Vector6));
		Maths::Vector6* aAux = (Maths::Vector6*)manager.m_Cache.CreateAlloc(&aAuxiliary[0], sizeof(Maths::Vector6));
		//*bAux = vZero;
		*aAux = vZero;
		manager.m_Cache.Store(&aAuxiliary[0], aAux, sizeof(Maths::Vector6));
		//manager.m_Cache.Store(&bAuxiliary[0], bAux, sizeof(Maths::Vector6));
	}


	//bAuxiliary[0] = Maths::Const<Maths::Vector6>::Zero();

	Spu::MemBaseMarker outerMarker;
	Gravity::C_ConstraintBody** nodeList = (Gravity::C_ConstraintBody**)Spu::FetchAllocUnaligned(m_RigidBodies.m_pElems, sizeof(Gravity::C_ConstraintBody*) * m_RigidBodies.m_numElements);


	SharedPhysics::C_SpuArrayStreamer <Gravity::C_ConstraintBody, 8, 4> streamer(&nodeList[1], m_RigidBodies.m_numElements - 1);
	for(S32 i=1; i<this->m_RigidBodies.m_numElements; ++i)
	{
		Spu::MemBaseMarker marker;
		//C_GraphNode* ppuNode = (C_GraphNode*)Spu::Get((U32*)m_RigidBodies.m_pElems+i);
		//C_GraphNode* ppuNode = nodeList[i];
		//C_GraphNode* pNode = (C_GraphNode*)Spu::FetchAlloc(ppuNode, sizeof(C_GraphNode) + sizeof(C_RigidBody));
		//C_RigidBody* rigid= (C_RigidBody*)(pNode + 1); //*rigidBody.Get(i);

		Gravity::C_ConstraintBody* pBody = streamer.NextElement();
		C_GraphNode* pNode = &pBody->m_GraphNode;
		C_RigidBody* rigid = &pBody->m_RigidBody;
		//const U32 index(i * bufferSize);
		
		//if(pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
		
			/*Maths::HF32 recipMass(rigid->ReciprocalMass());
			Maths::HMat33 invInertia(rigid->m_InverseInertiaTensor);
			if(!pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
			{
				recipMass = zero;
				invInertia = zeroMatrix;
			}*/
				

			/*const Maths::HVec3 temp(rigid->m_InertiaTensor * rigid->m_AngularVelocity);
			const Maths::HVec3 temp1(temp ^ rigid->m_AngularVelocity);
			const Maths::HVec3 temp2(rigid->m_Torque - temp1);
			const Maths::HVec3 ang((rigid->m_AngularVelocity) + (temp2 * rigid->m_InverseInertiaTensor) * m_Time);*/

			/*Maths::HVec3 vel(rigid->m_LinearVelocity);
			Maths::HVec3 ang(rigid->m_AngularVelocity);
			if(!pNode->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_No_Solver_Force))
			{
				vel += rigid->m_Force * m_Time * rigid->m_ReciprocalMass;
				ang += (rigid->m_Torque * rigid->m_InverseInertiaTensor) * m_Time;
			}*/

			//const Maths::HVec3 vel(rigid->m_LinearVelocity + rigid->m_Force * m_Time * recipMass);
			//const Maths::HVec3 ang(rigid->m_AngularVelocity + (rigid->m_Torque * invInertia) * m_Time);


			//const Maths::Vector3 nvel(vel);
			//const Maths::Vector3 nang(ang);

			//Maths::Vector6* bAux = (Maths::Vector6*)manager.m_Cache.CreateAlloc(&bAuxiliary[i], sizeof(Maths::Vector6));
			//*bAux = Maths::MakeVector6(nvel, nang);
			Maths::Vector6* aAux = (Maths::Vector6*)manager.m_Cache.CreateAlloc(&aAuxiliary[i], sizeof(Maths::Vector6));
			*aAux = vZero;
			manager.m_Cache.Store(&aAuxiliary[i], aAux, sizeof(Maths::Vector6));
			//manager.m_Cache.Store(&bAuxiliary[i], bAux, sizeof(Maths::Vector6));
		}
	}
}


void LCP::Solve()//const U32 numObjects)
{
	{
		PERF_EVENT("gravity.solver.BVecWarmDiagonal");
		manager.BVecWarmDiagonal(aAuxiliary/*, bAuxiliary*/);
	}

	{
		PERF_EVENT("gravity.solver.Solve");
		const VF32 threshold = MakeVF32(g_dbExitThreshold);
		const U32 jointIterationFrequency(Maths::Min<U32>(Maths::Max<U32>(1, m_JointIterationFrequency), m_NumIteration));
		//bool solveJoint = false;
		const U32 jointCount = m_NumIteration/jointIterationFrequency;

		const U32 numShortIterations = m_NumIteration - jointCount;

		for(U32 k=0; k<numShortIterations; ++k)
		{
			//const U32 module = k%m_JointIterationFrequency;
			const Maths::VF32 totalDif = manager.UpdateConstraintsNoJoints(aAuxiliary);
			//Early out
			if(totalDif < threshold)
				break;
		}

		for(U32 k=0; k<jointCount; ++k)
		{
			//const U32 module = k%m_JointIterationFrequency;
			const Maths::VF32 totalDif = manager.UpdateConstraints(aAuxiliary);
			//Early out
			if(totalDif < threshold)
				break;
		}

	}

}

#ifndef CFG_SPEED
#pragma optimize("", off)
#endif
bool LCP::Validate()
{
#ifndef PLATFORM_SPU
	/*const Maths::HVec3 bigValue(4000.f);
	const Maths::HVec3 bigAng(100.f);*/

	bool success = true;
	Spu::MemBaseMarker outerMarker;
	Gravity::C_ConstraintBody** nodeList = (Gravity::C_ConstraintBody**)Spu::FetchAllocUnaligned(m_RigidBodies.m_pElems, sizeof(Gravity::C_ConstraintBody*) * m_RigidBodies.m_numElements);

	C_SpuArrayStreamer<Gravity::C_ConstraintBody, 8, 4> streamer(&nodeList[1], m_RigidBodies.m_numElements - 1);

	for(S32 i=1; i< this->m_RigidBodies.m_numElements; ++i)
	{
		Spu::MemBaseMarker marker;

		Gravity::C_ConstraintBody* pLcpBody = streamer.NextElement();
		const Maths::Vector6* a = (Maths::Vector6*)manager.m_Cache.FetchAlloc(&aAuxiliary[i ], sizeof(Maths::Vector6));
		/*const Maths::Vector6* b = (Maths::Vector6*)manager.m_Cache.FetchAlloc(&bAuxiliary[i ], sizeof(Maths::Vector6));*/

		//Don't assert on invalid debris - Nighwing debris is huge and weighs less than a feather so 
		//can easily get smacked into infinity
		if(pLcpBody->m_GraphNode.TestNodeFlags(C_GraphNode::E_Type_Debris))
			continue;

		const Maths::HVec3 a0(Maths::ExtractLoVector3(*a));
		const Maths::HVec3 a1(Maths::ExtractHiVector3(*a));

		const Maths::HVec3 b0(Maths::ExtractLoVector3(pLcpBody->m_RigidBody.m_BVector));
		const Maths::HVec3 b1(Maths::ExtractHiVector3(pLcpBody->m_RigidBody.m_BVector));

		BIZ_ASSERT(b0.Finite());
		if (!b0.Finite())
		{
			LOGFATAL(g_PhysLog, "!b0.Finite()");
			success = false;
		}
		
		BIZ_ASSERT(b1.Finite());
		if (!b1.Finite())
		{
			LOGFATAL(g_PhysLog, "!b1.Finite()");
			success = false;
		}
		
	/*	BIZ_ASSERT(Abs(b0).All_LT(bigValue));
		if (!Abs(b0).All_LT(bigValue))
		{
			LOGFATAL(g_PhysLog, "!Abs(b0).All_LT(bigValue)");		
			success = false;
		}

		BIZ_ASSERT(Abs(b1).All_LT(bigAng));
		if (!Abs(b1).All_LT(bigAng))
		{
			LOGFATAL(g_PhysLog, "!Abs(b1).All_LT(bigAng)");
			success = false;
		}*/

		BIZ_ASSERT(a0.Finite());
		if (!a0.Finite())
		{
			LOGFATAL(g_PhysLog, "!a0.Finite()");
			success = false;
		}
		
		BIZ_ASSERT(a1.Finite());
		if (!a1.Finite())
		{
			LOGFATAL(g_PhysLog, "!a1.Finite()");
			success = false;
		}
		
	/*	BIZ_ASSERT(Abs(a0).All_LT(bigValue));
		if (!Abs(a0).All_LT(bigValue))
		{
			LOGFATAL(g_PhysLog, "!Abs(a0).All_LT(bigValue)");		
			success = false;
		}

		BIZ_ASSERT(Abs(a1).All_LT(bigAng));
		if (!Abs(a1).All_LT(bigAng))
		{
			LOGFATAL(g_PhysLog, "!Abs(a1).All_LT(bigAng)");
			success = false;
		}*/
	}
	return success;
#endif
	return true;
}
#ifndef CFG_SPEED
#pragma optimize("", on)
#endif

void LCP::Update()
{
	//PERF_EVENT("gravity.solver.Update");

	/*if (!Validate())
	{
		BIZ_ASSERT_FAIL("Validate FAILED!");
#ifndef PLATFORM_SPU
		LOGFATAL(g_PhysLog, "Validate FAILED!");
#endif
	}*/

#ifdef ENABLE_RE_RUNS
	if(!Validate())
	{
		m_ReRun = true;
		return;
	}
#endif


	BIZ_ASSERT(Validate());
	
	const Maths::HVec3 zero(Maths::Constants<Maths::HF32>::Zero());

	Spu::MemBaseMarker outerMarker;
	Gravity::C_ConstraintBody** nodeList = (Gravity::C_ConstraintBody**)Spu::FetchAllocUnaligned(m_RigidBodies.m_pElems, sizeof(Gravity::C_ConstraintBody*) * m_RigidBodies.m_numElements);

	C_SpuArrayStreamer<Gravity::C_ConstraintBody, 8, 4> streamer(&nodeList[1], m_RigidBodies.m_numElements - 1);
	
	for(S32 i=1; i< this->m_RigidBodies.m_numElements; ++i)
	{
		Spu::MemBaseMarker marker;
		//SharedPhysics::C_GraphNode* _node = (SharedPhysics::C_GraphNode*)Spu::Get((U32*)m_RigidBodies.m_pElems + i);

		//SharedPhysics::C_GraphNode* node = (SharedPhysics::C_GraphNode*)Spu::FetchAlloc(_node, sizeof(SharedPhysics::C_GraphNode) + sizeof(SharedPhysics::C_RigidBody));
		Gravity::C_ConstraintBody* pLcpBody = streamer.NextElement();
		//C_LCPBody* pLcpBody = (C_LCPBody*)Spu::FetchAlloc(nodeList[i], sizeof(C_LCPBody));;
		SharedPhysics::C_GraphNode* node = &pLcpBody->m_GraphNode;
		
		if(node->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			
			//C_RigidBody* rigid=(C_RigidBody*)Spu::FetchAlloc(node->m_pRigidBody, sizeof(C_RigidBody));
			//C_RigidBody* rigid = (C_RigidBody*)(node + 1);
			SharedPhysics::C_RigidBody* rigid = &pLcpBody->m_RigidBody;

			//const Maths::Vector6& b = bAuxiliary[i];
			//const Maths::Vector6& a = aAuxiliary[i];
			//F32* a = (F32*)Spu::FetchAlloc(&aAuxiliary[i * bufferSize], sizeof(F32) * bufferSize);

			/*const Maths::HVec3 b0(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)b));
			const Maths::HVec3 b1(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)(b+3)));
			const Maths::HVec3 a0(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)a));
			const Maths::HVec3 a1(Maths::Load<Maths::HVec3>((MathsStorage::Float3*)(a+3)));*/

			//Maths::Vector6* b = (Maths::Vector6*)manager.m_Cache.FetchAlloc(&bAuxiliary[ i ], sizeof(Maths::Vector6));
			Maths::Vector6* a = (Maths::Vector6*)manager.m_Cache.FetchAlloc(&aAuxiliary[ i ], sizeof(Maths::Vector6));

			const Maths::HVec3 b0(Maths::ExtractLoVector3(rigid->m_BVector));
			const Maths::HVec3 b1(Maths::ExtractHiVector3(rigid->m_BVector));
			const Maths::HVec3 a0(Maths::ExtractLoVector3(*a));
			const Maths::HVec3 a1(Maths::ExtractHiVector3(*a));

			//DebugBreak();
			/*rigid->m_LinearVelocity = b0 + a0;
			rigid->m_AngularVelocity = b1 + a1;*/
			const Maths::HVec3 newLinVel(b0 + a0);
			const Maths::HVec3 newAngVel(b1 + a1);

			/*BIZ_ASSERT(newLinVel.Finite());
			BIZ_ASSERT(newAngVel.Finite());*/

		/*	const Maths::HVec3 originalV = rigid->m_LinearVelocity;
			const Maths::HVec3 angularV = rigid->m_AngularVelocity;*/

			/*const Maths::HVec3 maxLinSpeed(rigid->m_MaxLinearSpeed);
			const Maths::HVec3 maxAngSpeed(rigid->m_MaxAngularSpeed);*/
			/*rigid->m_LinearVelocity = Maths::Select(newLinVel.Finite(), Maths::Min(maxLinSpeed, Maths::Max(-maxLinSpeed, newLinVel) ), rigid->m_LinearVelocity);
			rigid->m_AngularVelocity = Maths::Select(newAngVel.Finite(), Maths::Min(maxAngSpeed, Maths::Max(-maxAngSpeed, newAngVel) ), rigid->m_AngularVelocity);*/

			rigid->m_LinearVelocity = Maths::Select(newLinVel.Finite(), newLinVel, rigid->m_LinearVelocity);
			rigid->m_AngularVelocity = Maths::Select(newAngVel.Finite(), newAngVel, rigid->m_AngularVelocity);

		/*	Maths::HF32 twoThousand(2000.f);
			BIZ_ASSERT(rigid->m_LinearVelocity.Length().LT(twoThousand));*/



			//if(!node->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_No_Solver_Force))
			{
				rigid->m_Force = rigid->m_Torque = Maths::Constants<Maths::HVec3>::Zero();
			}

			//Adjust linear/angular momentum now to take effect

			/*rigid->m_PreLinearMomentum = rigid->m_LinearMomentum;
			rigid->m_PreAngularMomentum = rigid->m_AngularMomentum;*/
			rigid->m_LinearMomentum = rigid->m_LinearVelocity * rigid->Mass();
			rigid->m_AngularMomentum = rigid->m_AngularVelocity * rigid->m_InertiaTensor;
			BIZ_ASSERT(rigid->m_LinearMomentum.Finite());
			BIZ_ASSERT(rigid->m_AngularMomentum.Finite());
//#ifdef PLATFORM_SPU
//			Spu::Store(node->m_pRigidBody, rigid, sizeof(C_RigidBody));	
//#endif
			streamer.WriteBackElement();
		}
		//SharedPhysics::C_MemoryIterator<C_ContactJacobian, 32> memIter(&jaco);
	}

	//BIZ_ASSERT(Validate());

	manager.StoreWarmStartValues();
}


//void LCP::LCPSolver(C_GraphNode* root, SharedPhysics::C_SafeStack<true>& stack)
void LCP::LCPSolver(C_GraphNode* root)
{
	Spu::MemBaseMarker marker;
#if 0 //def PLATFORM_SPU
	DebugBreak();
#endif
	PERF_EVENT("gravity.solver");
	manager.Reset();
	{
		Spu::MemBaseMarker _marker;
		const U32 size = 1024;
#ifndef PLATFORM_SPU
		char data[size*sizeof(void*)];
#else
		char* data = (char*)Spu::Alloc(size*sizeof(void*));
#endif
		SharedPhysics::C_SafeStack<true> stack(data, size);
		m_NumContacts=ConstructJacobianAndInvertMJT(root, stack);
	}
	//m_NumContactsX6=m_NumContacts*bufferSize;
	//m_NumObjectsX6=m_RigidBodies.m_numElements*bufferSize;
	//bAuxiliary.Resize(m_RigidBodies.m_numElements, CachePageSize);
	aAuxiliary.Resize(m_RigidBodies.m_numElements, CachePageSize); //Inv(m)*Transpose(J)*Lambda
	ConstructAuxiliaryB();
	//ConstructBVector();
	//manager.ConstructBVector(&bAuxiliary[0], m_Time);
	//manager.ConstructBVector(bAuxiliary, m_Time);
	Solve();
	Update();
}

};

