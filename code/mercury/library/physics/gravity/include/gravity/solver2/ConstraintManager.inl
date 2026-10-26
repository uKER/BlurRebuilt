#ifndef CONSTRAINTMANAGER_INL
#define CONSTRAINTMANAGER_INL

#include "gravity/solver2/Constraint.h"

namespace Solver2
{


	__forceinline Maths::VF32 AbsSum(const Maths::VF32 PCREF val)
	{
		return Maths::Abs(val);
	}

	__forceinline Maths::VF32 AbsSum(const Maths::Vector2 PCREF val)
	{
		const Maths::Vector2 absVal(Maths::Abs(val));
		return Maths::X(absVal) + Maths::Y(absVal);
	}

	__forceinline Maths::VF32 AbsSum(const Maths::Vector3 PCREF val)
	{
		const Maths::Vector3 absVal(Maths::Abs(val));
		return Maths::X(absVal) + Maths::Y(absVal) + Maths::Z(absVal);
	}

	__forceinline Maths::VF32 AbsSum(const Maths::Vector4 PCREF val)
	{
		const Maths::Vector4 absVal(Maths::Abs(val));
		return Maths::X(absVal) + Maths::Y(absVal) + Maths::Z(absVal) + Maths::W(absVal);
	}

	__forceinline Maths::VF32 AbsSum(const Maths::Vector5 PCREF val)
	{
		const Maths::Vector5 absVal(Maths::Abs(val));
		return Maths::Element<0>(absVal) + Maths::Element<1>(absVal) + 
			Maths::Element<2>(absVal) + Maths::Element<3>(absVal) + Maths::Element<4>(absVal);
	}

	__forceinline Maths::VF32 AbsSum(const Maths::Vector6 PCREF val)
	{
		const Maths::Vector6 absVal(Maths::Abs(val));
		return Maths::Element<0>(absVal) + Maths::Element<1>(absVal) + 
			Maths::Element<2>(absVal) + Maths::Element<3>(absVal) + Maths::Element<4>(absVal)
			+ Maths::Element<5>(absVal);
	}

	template <typename Constraint, typename Jacobian, typename InvertMJT>
	void ConstraintManager::ConstructJacobian(SharedPhysics::C_Constraint* ppuConstraint,
		Gravity::C_ConstraintBody* spuConstraintBody1, Gravity::C_ConstraintBody* spuConstraintBody2, /*C_Jacobian** jacobian, C_InvertMJT** invMJT,*/
		SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints)
	{
		//PERF_EVENT("gravity.CreateJacInvMJT");
		C_ConstraintPair<Jacobian, InvertMJT>* _pPair = m_MemoryPool.Allocate<C_ConstraintPair<Jacobian, InvertMJT>, false>();
		
		Jacobian* jaco = &_pPair->m_Jacobian;
		InvertMJT* imjt = &_pPair->m_InvertMJT;
		
#ifdef PLATFORM_SPU
		
		C_ConstraintPair<Jacobian, InvertMJT> pair;
		C_ConstraintPair<Jacobian, InvertMJT>* pPair= &pair;
		Jacobian* j = &pPair->m_Jacobian;
		InvertMJT* i = &pPair->m_InvertMJT;

#else
		C_ConstraintPair<Jacobian, InvertMJT>* pPair = _pPair;
		Jacobian* j = jaco;
		InvertMJT* i = imjt;
		new (j)Jacobian;
		new (i)InvertMJT;
#endif
		
		/*j->m_pRigid1=node1;
		j->m_pRigid2=node2;
		i->m_pRigid1=node1;
		i->m_pRigid2=node2;*/
		j->m_pSpuConstraintBody1 = spuConstraintBody1;
		j->m_pSpuConstraintBody2 = spuConstraintBody2;
		i->m_pSpuConstraintBody1 = spuConstraintBody1;
		i->m_pSpuConstraintBody2 = spuConstraintBody2;

		
		/*F32 slop1 = Spu::Get(&node1->m_Slop);
		F32 slop2 = Spu::Get(&node2->m_Slop);*/
		F32 slop1 = spuConstraintBody1->m_GraphNode.m_Slop;
		F32 slop2 = spuConstraintBody2->m_GraphNode.m_Slop;

		pPair->m_Slop = Maths::Min(slop1, slop2);


		j->m_pPPUPtr = jaco;
		i->m_pPPUPtr = imjt;

		Constraint* constraint = (Constraint*)Spu::FetchAlloc(ppuConstraint, sizeof(Constraint));
		//constraint->m_pPPUConstraint = ppuConstraint;

		//create jacobian and invertMJT, if it is necessary, flip the spu constraint body
		constraint->CreateJacobian(j);
		constraint->CreateInvertMJT(i, j);

		//set the final graph node island index
		j->m_GraphNodeIslandIndexA = j->m_pSpuConstraintBody1->m_GraphNode.m_IslandIndex;
		j->m_GraphNodeIslandIndexB = j->m_pSpuConstraintBody2->m_GraphNode.m_IslandIndex;

		BIZ_ASSERT(j->m_GraphNodeIslandIndexA >= 0);
		BIZ_ASSERT(j->m_GraphNodeIslandIndexB >= 0);

		//constraint->ZeroWarmStartValues();
		pPair->m_Jacobian.SetLambda(pPair->m_Jacobian.GetWarmStartValue(), constraint);

		pPair->m_Jacobian.CalculateBVector(j->m_pSpuConstraintBody1->m_RigidBody.m_BVector,
			j->m_pSpuConstraintBody2->m_RigidBody.m_BVector, pPair->m_Slop, 0.f);
	//	i->SetJacobian(jaco);


#ifdef PLATFORM_SPU

		Spu::Store(_pPair, pPair, sizeof(C_ConstraintPair<Jacobian, InvertMJT>));
#endif
		/**jacobian = jaco;
		*invMJT = imjt;*/

		AddToEnd(constraints, _pPair);
	
	}

	template <typename Constraint, typename Jacobian, typename InvertMJT>
	void ConstraintManager::ConstructJacobian(SharedPhysics::C_Constraint* ppuConstraint,
		Gravity::C_ConstraintBody * spuConstraintBody1, Gravity::C_ConstraintBody* spuConstraintBody2, 
		SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& staticConstraints)
	{
		//PERF_EVENT("gravity.CreateJacInvMJT");
		C_ConstraintPair<Jacobian, InvertMJT>* _pPair = m_MemoryPool.Allocate<C_ConstraintPair<Jacobian, InvertMJT>, false>();
		//Jacobian* jaco = m_MemoryPool.Allocate<Jacobian, false>();
		//InvertMJT* imjt = m_MemoryPool.Allocate<InvertMJT, false>();

		Jacobian* jaco = &_pPair->m_Jacobian;
		InvertMJT* imjt = &_pPair->m_InvertMJT;
		
#ifdef PLATFORM_SPU

		C_ConstraintPair<Jacobian, InvertMJT> pair;
		C_ConstraintPair<Jacobian, InvertMJT>* pPair = &pair;
		Jacobian* j = &pPair->m_Jacobian;
		InvertMJT* i = &pPair->m_InvertMJT;

#else
		C_ConstraintPair<Jacobian, InvertMJT>* pPair = _pPair;
		Jacobian* j = jaco;
		InvertMJT* i = imjt;
		new (j)Jacobian;
		new (i)InvertMJT;
#endif
		
		/*j->m_pRigid1=node1;
		j->m_pRigid2=node2;
		i->m_pRigid1=node1;
		i->m_pRigid2=node2;*/

		j->m_pSpuConstraintBody1=spuConstraintBody1;
		j->m_pSpuConstraintBody2=spuConstraintBody2;
		i->m_pSpuConstraintBody1=spuConstraintBody1;
		i->m_pSpuConstraintBody2=spuConstraintBody2;

		/*F32 slop1 = Spu::Get(&node1->m_Slop);
		F32 slop2 = Spu::Get(&node2->m_Slop);*/
		F32 slop1 = spuConstraintBody1->m_GraphNode.m_Slop;
		F32 slop2 = spuConstraintBody2->m_GraphNode.m_Slop;

		pPair->m_Slop = Maths::Min(slop1, slop2);

		j->m_pPPUPtr = jaco;
		i->m_pPPUPtr = imjt;

		Constraint* constraint = (Constraint*)Spu::FetchAlloc(ppuConstraint, sizeof(Constraint));
		//constraint->m_pPPUConstraint = ppuConstraint;

		constraint->CreateJacobian(j);
		constraint->CreateInvertMJT(i, j);


		//set the final graph node island index
		j->m_GraphNodeIslandIndexA = j->m_pSpuConstraintBody1->m_GraphNode.m_IslandIndex;
		j->m_GraphNodeIslandIndexB = j->m_pSpuConstraintBody2->m_GraphNode.m_IslandIndex;

		BIZ_ASSERT(j->m_GraphNodeIslandIndexA >= 0);
		BIZ_ASSERT(j->m_GraphNodeIslandIndexB >= 0);

		pPair->m_Jacobian.SetLambda(pPair->m_Jacobian.GetWarmStartValue(), constraint);

		pPair->m_Jacobian.CalculateBVector(j->m_pSpuConstraintBody1->m_RigidBody.m_BVector,
			j->m_pSpuConstraintBody2->m_RigidBody.m_BVector, pPair->m_Slop, 0.f);
		
		//i->SetJacobian(jaco);

#ifdef PLATFORM_SPU
		Spu::Store(_pPair, pPair, sizeof(C_ConstraintPair<Jacobian, InvertMJT>));
#endif
		/**jacobian = jaco;
		*invMJT = imjt;*/

		SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >* constraintArray = 
			SharedPhysics::Fast_Select<SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* > >(j->m_StaticConstraint, &staticConstraints, &constraints);

		AddToEnd(*constraintArray, _pPair);
	}

	template <typename Jacobian, typename InvertMJT>
	void ConstraintManager::StoreWarmStartValues(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& arr)
	{
		if(arr.m_numElements == 0)
			return;

		Spu::MemBaseMarker marker;
		C_ConstraintPair<Jacobian, InvertMJT>** pList = (C_ConstraintPair<Jacobian, InvertMJT>**)Spu::FetchAllocUnaligned(arr.m_pElems, sizeof(C_ConstraintPair<Jacobian, InvertMJT>*) * arr.m_numElements);

		SharedPhysics::C_SpuArrayStreamer<C_ConstraintPair<Jacobian, InvertMJT>, 8, 4 > streamer(pList, arr.m_numElements);
		for(S32 a = 0; a < arr.m_numElements; ++a)
		{
			Spu::MemBaseMarker marker;
			//C_ConstraintPair<Jacobian, InvertMJT>* pair = (C_ConstraintPair<Jacobian, InvertMJT>*)Spu::FetchAlloc(pList[a], sizeof(C_ConstraintPair<Jacobian, InvertMJT>));
			C_ConstraintPair<Jacobian, InvertMJT>* pair = streamer.NextElement();
			SharedPhysics::C_Constraint* pConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(pair->m_Jacobian.m_pConstraint, sizeof(SharedPhysics::C_Constraint));
			pConstraint->StoreWarmstartValues(pair->m_Jacobian._m_Lambda);
#ifdef PLATFORM_SPU
			Spu::Store(pair->m_Jacobian.m_pConstraint, pConstraint, sizeof(SharedPhysics::C_Constraint));
#endif
		} 

	/*	if(arr.m_numElements == 0)
			return;

		Spu::MemBaseMarker marker;
		C_ConstraintPair<Jacobian, InvertMJT>** pList = (C_ConstraintPair<Jacobian, InvertMJT>**)Spu::FetchAllocUnaligned(arr.m_pElems, sizeof(C_ConstraintPair<Jacobian, InvertMJT>*) * arr.m_numElements);

		for(S32 a = 0; a < arr.m_numElements; ++a)
		{
			Spu::MemBaseMarker marker;
			C_ConstraintPair<Jacobian, InvertMJT>* pair = (C_ConstraintPair<Jacobian, InvertMJT>*)Spu::FetchAlloc(pList[a], sizeof(C_ConstraintPair<Jacobian, InvertMJT>));
			SharedPhysics::C_Constraint* pConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(pair->m_Jacobian.m_pConstraint, sizeof(SharedPhysics::C_Constraint));
			pConstraint->StoreWarmstartValues(pair->m_Jacobian._m_Lambda);

#ifdef PLATFORM_SPU
			Spu::Store(pair->m_Jacobian.m_pConstraint, pConstraint, sizeof(SharedPhysics::C_Constraint));
#endif
		}*/
	}

	template <typename Jacobian, typename InvertMJT>
	void ConstraintManager::ProcessConstraint(C_ConstraintStream<Jacobian, InvertMJT>* buffer, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, Maths::VF32& totalDif, Maths::VF32& newTotDif)
	{

		buffer->Wait();

		C_ConstraintPair<Jacobian, InvertMJT>* pPair = buffer->GetConstraintPair();

		Jacobian* j = &pPair->m_Jacobian;
		InvertMJT* i = &pPair->m_InvertMJT;

			
		Maths::Vector6* a1 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[ pPair->m_RigidBodyAIndex ], sizeof(Maths::Vector6));
		Maths::Vector6* a2 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[ pPair->m_RigidBodyBIndex ], sizeof(Maths::Vector6));

		/*Maths::Vector6& a1 = aAuxiliary[pPair->m_RigidBodyAIndex];
		Maths::Vector6& a2 = aAuxiliary[pPair->m_RigidBodyBIndex];*/

		newTotDif = j->UpdateLambda(*a1, *a2, i->_m_Diagonal, i->_m_Difference, totalDif);
		i->CalculateInvertMJTLambda(*a1, *a2);

		m_Cache.Store(&aAuxiliary[ pPair->m_RigidBodyAIndex ], a1, sizeof(Maths::Vector6));
		m_Cache.Store(&aAuxiliary[ pPair->m_RigidBodyBIndex ], a2, sizeof(Maths::Vector6));
			
		buffer->WriteBackConstraint();

	}


	//template <typename Jacobian, typename InvertMJT, typename VectorType>
	//__forceinline Maths::VF32 ConstraintManager::UpdateConstraint(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, const Maths::VF32 PCREF totalDif)
	//{
	//	using namespace Maths;

	//	VF32 newTotDif = totalDif;
	//	//VF32 newTotDif2 = totalDif;

	//	if(constraints.m_numElements == 0)
	//		return newTotDif;

	//	Spu::MemBaseMarker marker;
	//	C_ConstraintPair<Jacobian, InvertMJT>** pList = (C_ConstraintPair<Jacobian, InvertMJT>**)Spu::FetchAllocUnaligned(constraints.m_pElems, sizeof(C_ConstraintPair<Jacobian, InvertMJT>*) * constraints.m_numElements);

	//	SharedPhysics::C_SpuArrayStreamer<C_ConstraintPair<Jacobian, InvertMJT>, 8, 4> streamer(pList, constraints.m_numElements);


	//	while(streamer.HasNextElement())
	//	{
	//		C_ConstraintPair<Jacobian, InvertMJT>* pPair = streamer.NextElement();

	//		Maths::Vector6& a1 = *(Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[pPair->m_RigidBodyAIndex], sizeof(Maths::Vector6));
	//		//a1 = *_a1;
	//		Maths::Vector6& a2 = *(Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[pPair->m_RigidBodyBIndex], sizeof(Maths::Vector6));
	//		//a2 = *_a2;
	//		
	//		Jacobian* j = &pPair->m_Jacobian;
	//		InvertMJT* i = &pPair->m_InvertMJT;
	//		{
	//			VectorType dif = j->UpdateLambda(a1, a2, i->_m_Diagonal);
	//			newTotDif += AbsSum(dif);
	//			a1 +=(i->_m_IMJT1 * dif);
	//			a2 +=(i->_m_IMJT2 * dif);
	//		}
	//		streamer.WriteBackElement();
	//	}
	//	

	//	return newTotDif;
	//}

	template <typename Jacobian, typename InvertMJT, typename VectorType>
	inline Maths::VF32 ConstraintManager::UpdateConstraint(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, const Maths::VF32 PCREF totalDif)
	{
		using namespace Maths;

		VF32 newTotDif = totalDif;
		//VF32 newTotDif2 = totalDif;

		if(constraints.m_numElements == 0)
			return newTotDif;

		Spu::MemBaseMarker marker;
		C_ConstraintPair<Jacobian, InvertMJT>** pList = (C_ConstraintPair<Jacobian, InvertMJT>**)Spu::FetchAllocUnaligned(constraints.m_pElems, sizeof(C_ConstraintPair<Jacobian, InvertMJT>*) * constraints.m_numElements);

		SharedPhysics::C_SpuArrayStreamer<C_ConstraintPair<Jacobian, InvertMJT>, 8, 4> streamer(pList, constraints.m_numElements);


		C_ConstraintPair<Jacobian, InvertMJT>* pPair = streamer.NextElement();

		bool bContinue = true;
		while(bContinue)
		{
			Maths::Vector6* _a1 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[pPair->m_RigidBodyAIndex], sizeof(Maths::Vector6));
			Maths::Vector6* _a2 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[pPair->m_RigidBodyBIndex], sizeof(Maths::Vector6));
			Maths::Vector6 a1(*_a1);
			Maths::Vector6 a2(*_a2);
			S32 indexA = pPair->m_RigidBodyAIndex;
			S32 indexB = pPair->m_RigidBodyBIndex;
			while(bContinue & (pPair->m_RigidBodyAIndex == indexA) & (pPair->m_RigidBodyBIndex == indexB))
			{
				Jacobian* j = &pPair->m_Jacobian;
				InvertMJT* i = &pPair->m_InvertMJT;
				{
					VectorType dif = j->UpdateLambda(a1,a2, i->_m_Diagonal);
					newTotDif += AbsSum(dif);
					a1 +=(i->_m_IMJT1 * dif);
					a2 +=(i->_m_IMJT2 * dif);
					/*i->_m_Difference = dif;
					i->CalculateInvertMJTLambda(a1, a2);*/
				}
				streamer.WriteBackElement();
				/*if(!streamer.HasNextElement())
				{
					bContinue = false;
					break;
				}*/
				bContinue = streamer.HasNextElement();
				pPair = streamer.NextElement();
			}
			*_a1 = a1;
			*_a2 = a2;
		}
		

		return newTotDif;
	}

	template <typename Jacobian, typename InvertMJT, typename VectorType>
	inline Maths::VF32 ConstraintManager::UpdateStaticConstraint(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, const Maths::VF32 PCREF totalDif)
	{
		using namespace Maths;

		VF32 newTotDif = totalDif;
		//VF32 newTotDif2 = totalDif;

		if(constraints.m_numElements == 0)
			return newTotDif;

		Spu::MemBaseMarker marker;
		C_ConstraintPair<Jacobian, InvertMJT>** pList = (C_ConstraintPair<Jacobian, InvertMJT>**)Spu::FetchAllocUnaligned(constraints.m_pElems, sizeof(C_ConstraintPair<Jacobian, InvertMJT>*) * constraints.m_numElements);

		SharedPhysics::C_SpuArrayStreamer<C_ConstraintPair<Jacobian, InvertMJT>, 8, 4> streamer(pList, constraints.m_numElements);

		C_ConstraintPair<Jacobian, InvertMJT>* pPair = streamer.NextElement();

		bool bContinue = true;
		while(bContinue)
		{
			Maths::Vector6* _a1 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[pPair->m_RigidBodyAIndex], sizeof(Maths::Vector6));
			Maths::Vector6 a1(*_a1);
			S32 index = pPair->m_RigidBodyAIndex;
			while(bContinue & (pPair->m_RigidBodyAIndex == index))
			{
				Jacobian* j = &pPair->m_Jacobian;
				InvertMJT* i = &pPair->m_InvertMJT;
				{
					VectorType dif = j->UpdateStaticLambda(a1,i->_m_Diagonal);
					newTotDif += AbsSum(dif);
					a1 +=(i->_m_IMJT1 * dif);
					//i->_m_Difference = dif;
				}
				streamer.WriteBackElement();
				/*if(!streamer.HasNextElement())
				{
					bContinue = false;
					break;
				}*/
				bContinue = streamer.HasNextElement();
				pPair = streamer.NextElement();
			}
			*_a1 = a1;
		}
		return newTotDif;
	}



	template <typename Jacobian, typename InvertMJT>
	void ConstraintManager::ProcessBVecWarmDiagonalValue(C_ConstraintStream<Jacobian, InvertMJT>* buffer, SharedPhysics::Array<Maths::Vector6>& aAuxiliary, SharedPhysics::Array<Maths::Vector6>& bAuxiliary)
	{
		buffer->Wait();

		C_ConstraintPair<Jacobian, InvertMJT>* pPair = buffer->GetConstraintPair();

		/*pPair->m_RigidBodyAIndex = Spu::Get((U32*)&pPair->m_Jacobian.m_pRigid1->m_IslandIndex);
		pPair->m_RigidBodyBIndex = Spu::Get((U32*)&pPair->m_Jacobian.m_pRigid2->m_IslandIndex);*/

		pPair->m_RigidBodyAIndex = pPair->m_Jacobian.m_GraphNodeIslandIndexA;
		pPair->m_RigidBodyBIndex = pPair->m_Jacobian.m_GraphNodeIslandIndexB;

		Maths::Vector6* b0 = (Maths::Vector6*)m_Cache.FetchAlloc(&bAuxiliary[ pPair->m_RigidBodyAIndex ], sizeof(Maths::Vector6));
		Maths::Vector6* b1 = (Maths::Vector6*)m_Cache.FetchAlloc(&bAuxiliary[ pPair->m_RigidBodyBIndex ], sizeof(Maths::Vector6));

		//Calculate B Vector
 		pPair->m_Jacobian.CalculateBVector( *b0, *b1, pPair->m_Slop, 0.f);

		//Set Warm Start Value
		//pPair->m_Jacobian.SetLambda(pPair->m_Jacobian.GetWarmStartValue());
		pPair->m_InvertMJT._m_Difference = pPair->m_Jacobian._m_Lambda;

		Maths::Vector6* a0 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[ pPair->m_RigidBodyAIndex ], sizeof(Maths::Vector6));
		Maths::Vector6* a1 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[ pPair->m_RigidBodyBIndex ], sizeof(Maths::Vector6));

		pPair->m_InvertMJT.CalculateInvertMJTLambda(*a0, *a1);

		//Calculate Diagonal Vector
		pPair->m_InvertMJT.CalculateDiagonalVector(&pPair->m_Jacobian);

		m_Cache.Store(&bAuxiliary[ pPair->m_RigidBodyAIndex ], b0, sizeof(Maths::Vector6));
		m_Cache.Store(&bAuxiliary[ pPair->m_RigidBodyBIndex ], b1, sizeof(Maths::Vector6));
		m_Cache.Store(&aAuxiliary[ pPair->m_RigidBodyAIndex ], a0, sizeof(Maths::Vector6));
		m_Cache.Store(&aAuxiliary[ pPair->m_RigidBodyBIndex ], a1, sizeof(Maths::Vector6));

		buffer->WriteBackConstraint();

	}


	template <typename Jacobian, typename InvertMJT>
	void ConstraintManager::BVecWarmDiagonal(SharedPhysics::SortedArray<C_ConstraintPair<Jacobian, InvertMJT>* >& constraints, 
		SharedPhysics::Array<Maths::Vector6>& aAuxiliary/*, SharedPhysics::Array<Maths::Vector6>& bAuxiliary*/)
	{
		if(constraints.m_numElements == 0)
			return;

		Spu::MemBaseMarker marker;
		C_ConstraintPair<Jacobian, InvertMJT>** pList = (C_ConstraintPair<Jacobian, InvertMJT>**)Spu::FetchAllocUnaligned(constraints.m_pElems, sizeof(C_ConstraintPair<Jacobian, InvertMJT>*) * constraints.m_numElements);

		SharedPhysics::C_SpuArrayStreamer<C_ConstraintPair<Jacobian, InvertMJT>, 8, 4> streamer(pList, constraints.m_numElements);

		for(S32 i=0; i<constraints.m_numElements; ++i)
		{
			//Spu::MemBaseMarker _marker;
			//C_ConstraintPair<Jacobian, InvertMJT>* pPair = (C_ConstraintPair<Jacobian, InvertMJT>*)Spu::FetchAlloc(pList[i], sizeof(C_ConstraintPair<Jacobian, InvertMJT>));
			//pPair->m_RigidBodyAIndex = Spu::Get((U32*)&pPair->m_Jacobian.m_pRigid1->m_IslandIndex);
			//pPair->m_RigidBodyBIndex = Spu::Get((U32*)&pPair->m_Jacobian.m_pRigid2->m_IslandIndex);

			C_ConstraintPair<Jacobian, InvertMJT>* pPair = streamer.NextElement();
			pPair->m_RigidBodyAIndex = pPair->m_Jacobian.m_GraphNodeIslandIndexA;
			pPair->m_RigidBodyBIndex = pPair->m_Jacobian.m_GraphNodeIslandIndexB;

			//Maths::Vector6* b0 = (Maths::Vector6*)m_Cache.FetchAlloc(&bAuxiliary[ pPair->m_RigidBodyAIndex ], sizeof(Maths::Vector6));
			//Maths::Vector6* b1 = (Maths::Vector6*)m_Cache.FetchAlloc(&bAuxiliary[ pPair->m_RigidBodyBIndex ], sizeof(Maths::Vector6));

			////Calculate B Vector
	 	//	pPair->m_Jacobian.CalculateBVector( *b0, *b1, pPair->m_Slop, 0.f);

			//Set Warm Start Value
			//pPair->m_Jacobian.SetLambda(pPair->m_Jacobian.GetWarmStartValue());
			pPair->m_InvertMJT._m_Difference = pPair->m_Jacobian._m_Lambda;

			Maths::Vector6* a0 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[ pPair->m_RigidBodyAIndex ], sizeof(Maths::Vector6));
			Maths::Vector6* a1 = (Maths::Vector6*)m_Cache.FetchAlloc(&aAuxiliary[ pPair->m_RigidBodyBIndex ], sizeof(Maths::Vector6));

			pPair->m_InvertMJT.CalculateInvertMJTLambda(*a0, *a1);

			//Calculate Diagonal Vector
			pPair->m_InvertMJT.CalculateDiagonalVector(&pPair->m_Jacobian);

			/*m_Cache.Store(&bAuxiliary[ pPair->m_RigidBodyAIndex ], b0, sizeof(Maths::Vector6));
			m_Cache.Store(&bAuxiliary[ pPair->m_RigidBodyBIndex ], b1, sizeof(Maths::Vector6));*/
			m_Cache.Store(&aAuxiliary[ pPair->m_RigidBodyAIndex ], a0, sizeof(Maths::Vector6));
			m_Cache.Store(&aAuxiliary[ pPair->m_RigidBodyBIndex ], a1, sizeof(Maths::Vector6));
//#ifdef PLATFORM_SPU
			//Spu::Store(pList[i], pPair, sizeof(C_ConstraintPair<Jacobian, InvertMJT>));
//#endif
			streamer.WriteBackElement();
		}


//		if(constraints.m_numElements == 0)
//			return;
//
//		Spu::MemBaseMarker marker;
//		C_ConstraintPair<Jacobian, InvertMJT>** pList = (C_ConstraintPair<Jacobian, InvertMJT>**)Spu::FetchAllocUnaligned(constraints.m_pElems, sizeof(C_ConstraintPair<Jacobian, InvertMJT>*) * constraints.m_numElements);
//
//
//#ifdef PLATFORM_SPU
//		C_ConstraintStreamer<Jacobian, InvertMJT>* _streamer = (C_ConstraintStreamer<Jacobian, InvertMJT>*)Spu::Alloc(sizeof(C_ConstraintStreamer<Jacobian, InvertMJT>));;
//		C_ConstraintStreamer<Jacobian, InvertMJT>& streamer = *_streamer;
//		streamer.Initialise();
//#else
//		C_ConstraintStreamer<Jacobian, InvertMJT> streamer;
//#endif
//
//		S32 i = 0;
//		if(constraints.m_numElements >= 4)
//		{
//
//		//Kick off first one streaming in...
//			C_ConstraintStream<Jacobian, InvertMJT>* frontBuffer = streamer.CurrentStreamBuffer();
//
//			frontBuffer[0].SetConstraintPair(pList[0]);
//			frontBuffer[1].SetConstraintPair(pList[1]);
//			frontBuffer[2].SetConstraintPair(pList[2]);
//			frontBuffer[3].SetConstraintPair(pList[3]);
//
//			for(S32 k=4; k <= constraints.m_numElements; k+=4)
//			{
//				i = k;
//				Spu::MemBaseMarker _marker;
//			
//				C_ConstraintStream<Jacobian, InvertMJT>* backBuffer = streamer.BackStreamBuffer();
//
//				streamer.WaitBackAll();
//
//				ProcessBVecWarmDiagonalValue(&frontBuffer[0], aAuxiliary, bAuxiliary);
//				ProcessBVecWarmDiagonalValue(&frontBuffer[1], aAuxiliary, bAuxiliary);
//				ProcessBVecWarmDiagonalValue(&frontBuffer[2], aAuxiliary, bAuxiliary);
//				ProcessBVecWarmDiagonalValue(&frontBuffer[3], aAuxiliary, bAuxiliary);
//
//
//				if((k+3) < constraints.m_numElements)
//				{
//					
//					backBuffer[0].SetConstraintPair(pList[k]);
//					backBuffer[1].SetConstraintPair(pList[k+1]);
//					backBuffer[2].SetConstraintPair(pList[k+2]);
//					backBuffer[3].SetConstraintPair(pList[k+3]);
//				}
//
//				streamer.FlipStreamBuffer();
//
//				frontBuffer = streamer.CurrentStreamBuffer();
//
//			}
//		}
//
//
//		if(i < constraints.m_numElements)
//		{
//
//			C_ConstraintStream<Jacobian, InvertMJT>* pStream = streamer.CurrentStream();
//			pStream->SetConstraintPair(pList[i]);
//
//			for(S32 k=i+1; k < constraints.m_numElements; ++k)
//			{
//				Spu::MemBaseMarker _marker;
//	
//				streamer.FlipStream();
//
//				//Check to make sure the next stream has finished writing back its data...
//				C_ConstraintStream<Jacobian, InvertMJT>* pNextStream = streamer.CurrentStream();
//				pNextStream->Wait();
//				pNextStream->SetConstraintPair(pList[k]);
//				
//				//Wait for current stream to finish reading
//				ProcessBVecWarmDiagonalValue(pStream, aAuxiliary, bAuxiliary);
//				pStream = pNextStream;
//
//			}
//
//			//Finish last one...
//			ProcessBVecWarmDiagonalValue(pStream, aAuxiliary, bAuxiliary);
//			
//		}
//		streamer.WaitAll();
	} 



}

#endif
