#include "Stdafx.h"
#include "gravity/cloths/ParticleBasedFluid.h"
#include "gravity/sdk/Body.h"

namespace Collision
{
	C_FluidEdge* C_ParticleBasedFluidSystem::CreateFluidEdge(C_FluidParticle* pParticleA, C_FluidParticle* pParticleB)
	{
		C_FluidEdge* pEdge = m_pEdges->GetNextFreeItem();
		pEdge->m_pInboundParticle = pParticleA;
		pEdge->m_pOutboundParticle = pParticleB;
		return pEdge;
	}

	C_FluidEdgeTwin* C_ParticleBasedFluidSystem::CreateFluidEdgeTwin(C_FluidEdge* pEdge)
	{
		C_FluidEdgeTwin* pTwin = m_pEdgeTwins->GetNextFreeItem();
		pEdge->m_pTwin = pTwin;
		pTwin->m_pFluidEdge = pEdge;
		return pTwin;
	}

	void C_ParticleBasedFluidSystem::ReleaseFluidEdge(C_FluidEdge* pEdge)
	{
		m_pEdges->PushToFree(pEdge);
	}

	void C_ParticleBasedFluidSystem::ReleaseFluidEdgeTwin(C_FluidEdgeTwin* pEdge)
	{
		m_pEdgeTwins->PushToFree(pEdge);
	}

	void C_FluidParticle::AddEdge(C_FluidEdge* pEdge)
	{
		pEdge->m_pNext = m_pFirstEdge;
		pEdge->m_pPrev = NULL;
		if(m_pFirstEdge)
			m_pFirstEdge->m_pPrev = pEdge;
		m_pFirstEdge = pEdge;
	}
	void C_FluidParticle::AddTwinEdge(C_FluidEdgeTwin* pTwin)
	{
		pTwin->m_pNext = m_pFirstTwinEdge;
		pTwin->m_pPrev = NULL;
		if(m_pFirstTwinEdge)
			m_pFirstTwinEdge->m_pPrev = pTwin;
		m_pFirstTwinEdge = pTwin;
	}
	void C_FluidParticle::RemoveEdge(C_FluidEdge* pEdge)
	{
		if(pEdge->m_pNext)
			pEdge->m_pNext->m_pPrev = pEdge->m_pPrev;
		if(pEdge->m_pPrev)
			pEdge->m_pPrev->m_pNext = pEdge->m_pNext;
		if(pEdge == m_pFirstEdge)
			m_pFirstEdge = pEdge->m_pNext;
		pEdge->m_pNext = pEdge->m_pPrev = NULL;
	}
	void C_FluidParticle::RemoveTwinEdge(C_FluidEdgeTwin* pTwin)
	{
		if(pTwin->m_pNext)
			pTwin->m_pNext->m_pPrev = pTwin->m_pPrev;
		if(pTwin->m_pPrev)
			pTwin->m_pPrev->m_pNext = pTwin->m_pNext;
		if(pTwin == m_pFirstTwinEdge)
			m_pFirstTwinEdge = pTwin->m_pNext;
		pTwin->m_pNext = pTwin->m_pPrev = NULL;
	}

	bool C_FluidParticle::HasNeighbour(C_FluidParticle* pParticle) const
	{
		C_FluidEdge* pEdge = m_pFirstEdge;
		while(pEdge)
		{
			if(pEdge->m_pOutboundParticle == pParticle)
				return true;
			pEdge = pEdge->m_pNext;
		}

		C_FluidEdgeTwin* pTwin = m_pFirstTwinEdge;
		while(pTwin)
		{
			if(pTwin->m_pFluidEdge->m_pInboundParticle == pParticle)
				return true;
			pTwin = pTwin->m_pNext;
		}
		return false;
	}

	void C_ParticleBasedFluidSystem::DisConnectNeighbours()
	{
		using namespace Maths;
		const HF32 sqParticleRadius(m_ParticleRadius* m_ParticleRadius);
		const HF32 one(Constants<HF32>::One());

		for(U32 a = 0; a < m_NumParticles; ++a)
		{
			C_FluidParticle* pParticle = &m_Particles[a];
			C_FluidEdge* pEdge = pParticle->m_pFirstEdge;
			while(pEdge)
			{
				C_FluidParticle* pParticleB = pEdge->m_pOutboundParticle;
				const HVec3 r (pParticleB->m_CurPosition - pParticle->m_CurPosition);	
				const Maths::HF32 rsq(r | r);
				if(rsq > sqParticleRadius)
				{
					pParticle->RemoveEdge(pEdge);
					pParticleB->RemoveTwinEdge(pEdge->m_pTwin);
				}
				else
				{
					const HF32 distance( r.Length() );
					const HVec3 dir( r/distance);
					const HF32 weight(one - distance/m_ParticleRadius);
					pEdge->m_Pair.m_SpringRestLength = m_SpringRestLength;
					pEdge->m_Pair.SetNeighborParameter(distance, dir, weight);
				}
				pEdge = pEdge->m_pNext;
			}
		}
	}

	void C_ParticleBasedFluidSystem::ConnectNeighbours()
	{
		using namespace Maths;
		const HF32 sqParticleRadius(m_ParticleRadius* m_ParticleRadius);
		const HF32 one(Constants<HF32>::One());
		//for(S32 i=0; i<m_NumParticles; ++i)
		//{
		//	C_FluidParticle* pParticleA = m_SortedParticles[i];
		//	//Reset the neighbor list
		//	pParticleA->m_Neighbors.m_numElements = 0;
		//	for(U32 j=0; j<m_NumParticles; ++j)
		//	{
		//		if(i == j)
		//			continue;
		//		C_FluidParticle* pParticleB = m_SortedParticles[j];
		//		const HVec3 r (pParticleB->m_CurPosition - pParticleA->m_CurPosition);	
		//		const Maths::HF32 rsq(r | r);
		//		if( (r|r) > sqParticleRadius)
		//			continue;


		//		const HF32 distance( r.Length() );
		//		const HVec3 dir( r/distance);
		//		const HF32 weight(one - distance/m_ParticleRadius);

		//		C_FluidParticlePair pair;
		//		pair.m_SpringRestLength = m_SpringRestLength;
		//		pair.SetNeighbor(pParticleA, pParticleB);
		//		pair.SetNeighborParameter(distance, dir, weight);
		//		pParticleA->m_Neighbors.AddToEnd(&pair);
		//	}
		//}

		for(U32 i=0; i<m_NumParticles; ++i)
		{
			C_FluidParticle* pParticleA = &m_Particles[i];
			//Reset the neighbor list
			//pParticleA->m_Neighbors.m_numElements = 0;
			for(U32 j=i+1; j<m_NumParticles; ++j)
			{
				C_FluidParticle* pParticleB = &m_Particles[j];

				if(!pParticleA->HasNeighbour(pParticleB))
				{
					const HVec3 r (pParticleB->m_CurPosition - pParticleA->m_CurPosition);	
					const Maths::HF32 rsq(r | r);
					if( (r|r) > sqParticleRadius)
						continue;

					const HF32 distance( r.Length() );
					const HVec3 dir( r/distance);
					const HF32 weight(one - distance/m_ParticleRadius);

					C_FluidEdge* pEdge = CreateFluidEdge(pParticleA, pParticleB);
					C_FluidEdgeTwin* pTwin = CreateFluidEdgeTwin(pEdge);
					pEdge->m_Pair.m_SpringRestLength = m_SpringRestLength;
					pEdge->m_Pair.SetNeighbor(pParticleA, pParticleB);
					pEdge->m_Pair.SetNeighborParameter(distance, dir, weight);
					pParticleA->AddEdge(pEdge);
					pParticleB->AddTwinEdge(pTwin);
				}
			}
		}
	}

	void C_ParticleBasedFluidSystem::UpdateNeighbours()
	{
		/*SharedPhysics::C_S32SafeStack stack(1024);
		m_SortedParticles.QuickSort(stack, 10);*/

		DisConnectNeighbours();
		ConnectNeighbours();
	}

	

	

	void C_ParticleBasedFluidSystem::InitialiseNeighbours()
	{
		//for(U32 i=0; i<m_NumParticles; ++i)
		//{
		//	C_FluidParticle* particle = (C_FluidParticle*) (&m_Particles[i]);
		//	//FluidParticlePtr ptr(par);
		//	m_SortedParticles.Insert(&particle);
		//}

		/*SharedPhysics::C_S32SafeStack stack(1024);
		m_SortedParticles.QuickSort(stack, 10);*/

		ConnectNeighbours();
	}

	void C_ParticleBasedFluidSystem::ApplyViscosity(const Maths::HF32 PCREF dt)
	{
		using namespace Maths;

		const HF32 zero(Constants<HF32>::Zero());
		const HF32 half(Constants<HF32>::Half());
		const HVec3 zeroV(Constants<HVec3>::Zero());

		for(U32 i=0; i<m_NumParticles; ++i)
		{
			U32 numNeighbors = 0;
			C_FluidParticle& parI = (C_FluidParticle&)m_Particles[i];
			//C_FluidParticlePair** neighbors = GetNeighbors(&parI, numNeighbors);
			//for(S32 j=0; j<parI.m_Neighbors.m_numElements; ++j)
			//{
			//	//C_FluidParticlePair* pPair = neighbors[j];
			//	C_FluidParticlePair* pPair = &parI.m_Neighbors[j];
			//	C_FluidParticle& parJ = *pPair->GetNeighbor(&parI);
			//	if(parI.m_Index > parJ.m_Index)
			//		continue;
			//	
			//	//Calculate the inward radial velocity magnitude
			//	const HF32 u = (parI.m_Velocity - parJ.m_Velocity)|pPair->m_ParticlesDirection;
			//	const Scalar_Bool con(u.GT(zero));
			//	
			//	//Linear and quadratic impulses
			//	const HF32 a(dt*pPair->m_ParticlesWeight*half);
			//	const HF32 b(m_LinearViscosityConstant * u + m_QuadraticViscosityConstant * u * u);
			//	const HVec3 div = Select(con, a * b * pPair->m_ParticlesDirection, zeroV);
			//	parI.m_Velocity -= div;
			//	parJ.m_Velocity += div;
			//}

			C_FluidEdge* pEdge = parI.m_pFirstEdge;
			while(pEdge)
			{
				//C_FluidParticlePair* pPair = neighbors[j];
				C_FluidParticle& parJ = *pEdge->m_pOutboundParticle;
				C_FluidParticlePair* pPair = &pEdge->m_Pair;
				//Calculate the inward radial velocity magnitude
				const HF32 u = (parI.m_Velocity - parJ.m_Velocity)|pPair->m_ParticlesDirection;
				const Scalar_Bool con(u.GT(zero));
				
				//Linear and quadratic impulses
				const HF32 a(dt*pPair->m_ParticlesWeight*half);
				const HF32 b(m_LinearViscosityConstant * u + m_QuadraticViscosityConstant * u * u);
				const HVec3 div = Select(con, a * b * pPair->m_ParticlesDirection, zeroV);
				parI.m_Velocity -= div;
				parJ.m_Velocity += div;
				pEdge = pEdge->m_pNext;
			}
		}
	}

	//According to the deformation to add/remove spring and ajust spring rest length
	void C_ParticleBasedFluidSystem::AdjustSprings(const Maths::HF32 PCREF dt)
	{
		using namespace Maths;
		
		const HF32 one(Constants<HF32>::One());
		const HF32 zero(Constants<HF32>::Zero());
		
		const HF32 half( Constants<HF32>::Half() );
		const HF32 a( dt*dt*m_SpringCoefficient *half );

		for(U32 i=0; i< m_NumParticles; ++i)
		{
			U32 numNeighbors = 0;
			C_FluidParticle& parI = m_Particles[i];
			//C_FluidParticlePair** neighbors = GetNeighbors(&parI, numNeighbors);
			//for(S32 j=0; j< parI.m_Neighbors.m_numElements; ++j)
			//{
			//	C_FluidParticlePair* pPair = &parI.m_Neighbors[j];

			//	C_FluidParticle& parJ = *pPair->GetNeighbor(&parI);
			//	if(parI.m_Index > parJ.m_Index)
			//		continue;

			//	//Tolerable deformation = yield ratio * rest length
			//	const HF32 d(m_YieldRatio * pPair->m_SpringRestLength);
			//	
			//	const HF32 dif = pPair->m_ParticlesDistance - pPair->m_SpringRestLength;

			//	//if the Abs(dif) > tolerable deformation, then change the rest length of the spring pair
			//	const Scalar_Bool con(Abs(dif).LT(d));
			//	const HF32 div = Select(con, dt*m_PlasticConstant*(dif - Sign(dif) * d), zero);
			//	pPair->m_SpringRestLength += div;

			//	const HF32 b(one - pPair->m_SpringRestLength/m_ParticleRadius);
			//	const HF32 c(pPair->m_SpringRestLength - pPair->m_ParticlesDistance );
			//	const HVec3 displacement = a *( b/c )*pPair->m_ParticlesDirection ;

			//	pPair->m_pParticleA->m_CurPosition -=displacement;
			//	pPair->m_pParticleB->m_CurPosition +=displacement;

			//}

			C_FluidEdge* pEdge = parI.m_pFirstEdge;
			while(pEdge)
			{
				C_FluidParticlePair* pPair = &pEdge->m_Pair;

				C_FluidParticle& parJ = *pEdge->m_pInboundParticle;

				//Tolerable deformation = yield ratio * rest length
				const HF32 d(m_YieldRatio * pPair->m_SpringRestLength);
				
				const HF32 dif = pPair->m_ParticlesDistance - pPair->m_SpringRestLength;

				//if the Abs(dif) > tolerable deformation, then change the rest length of the spring pair
				const Scalar_Bool con(Abs(dif).LT(d));
				const HF32 div = Select(con, dt*m_PlasticConstant*(dif - Sign(dif) * d), zero);
				pPair->m_SpringRestLength += div;

				const HF32 b(one - pPair->m_SpringRestLength/m_ParticleRadius);
				const HF32 c(pPair->m_SpringRestLength - pPair->m_ParticlesDistance );
				const HVec3 displacement = a *( b/c )*pPair->m_ParticlesDirection ;

				pPair->m_pParticleA->m_CurPosition -=displacement;
				pPair->m_pParticleB->m_CurPosition +=displacement;
				pEdge = pEdge->m_pNext;

			}
		}
	}

	/*
		To simulate the elastic behaviour, we add springs between the neighboring particles
	*/
	void C_ParticleBasedFluidSystem::ApplySpringDisplacements(const Maths::HF32 PCREF dt)
	{
		using namespace Maths;

		const HF32 half( Constants<HF32>::Half() );
		const HF32 a( dt*dt*m_SpringCoefficient *half );
		const HF32 one( Constants<HF32>::One() );


		for(U32 i=0; i< m_NumParticles; ++i)
		{
			U32 numNeighbors = 0;
			C_FluidParticle& parI = m_Particles[i];
			//C_FluidParticlePair** neighbors = GetNeighbors(&parI, numNeighbors);
			/*for(S32 j=0; j< parI.m_Neighbors.m_numElements; ++j)
			{
				C_FluidParticlePair* pPair = &parI.m_Neighbors[j];
				C_FluidParticle& parJ = *pPair->GetNeighbor(&parI);
				if(parI.m_Index > parJ.m_Index)
					continue;
				
				const HF32 b(one - pPair->m_SpringRestLength/m_ParticleRadius);
				const HF32 c(pPair->m_SpringRestLength - pPair->m_ParticlesDistance );
				const HVec3 displacement = a *( b/c )*pPair->m_ParticlesDirection ;

				pPair->m_pParticleA->m_CurPosition -=displacement;
				pPair->m_pParticleB->m_CurPosition +=displacement;
				
			}*/

			C_FluidEdge* pEdge = parI.m_pFirstEdge;

			while(pEdge)
			{
				C_FluidParticlePair* pPair = &pEdge->m_Pair;
				C_FluidParticle& parJ = *pEdge->m_pOutboundParticle;
				
				const HF32 b(one - pPair->m_SpringRestLength/m_ParticleRadius);
				const HF32 c(pPair->m_SpringRestLength - pPair->m_ParticlesDistance );
				const HVec3 displacement = a *( b/c )*pPair->m_ParticlesDirection ;

				pPair->m_pParticleA->m_CurPosition -=displacement;
				pPair->m_pParticleB->m_CurPosition +=displacement;
				pEdge = pEdge->m_pNext;
				
			}
		}

	}

	void C_ParticleBasedFluidSystem::DoubleDensityRelaxation(const Maths::HF32 PCREF dt)
	{
		using namespace Maths;

		HF32 zero(Constants<HF32>::Zero());
		HF32 half(Constants<HF32>::Half());
		HVec3 zeroV(Constants<HVec3>::Zero());
		
		
		for(U32 i=0; i<m_NumParticles; ++i)
		{
			//Compute density and near density
			HF32 density(zero);
			HF32 densityNear(zero);

			C_FluidParticle& parI = m_Particles[i];
			U32 numNeighbors = 0; 
			//C_FluidParticlePair** neighbors = GetNeighbors(&parI, numNeighbors); 

			/*for(S32 j=0; j<parI.m_Neighbors.m_numElements; ++j)
			{
				C_FluidParticlePair* pPair = &parI.m_Neighbors[j];
				density += pPair->m_ParticlesWeight * pPair->m_ParticlesWeight;
				densityNear += pPair->m_ParticlesWeight * pPair->m_ParticlesWeight * pPair->m_ParticlesWeight;
				
			}*/

			C_FluidEdge* pEdge = parI.m_pFirstEdge;
			while(pEdge)
			{
				C_FluidParticlePair* pPair = &pEdge->m_Pair;
				density += pPair->m_ParticlesWeight * pPair->m_ParticlesWeight;
				densityNear += pPair->m_ParticlesWeight * pPair->m_ParticlesWeight * pPair->m_ParticlesWeight;
				pEdge = pEdge->m_pNext;
				
			}

			//Compute the pressure and near pressure
			const HF32 pressure = m_Stiffness *( density - m_RestDensity);
			const HF32 pressureNear = m_StiffnessNear * densityNear;

			const HF32 sqDt(dt*dt);
			HVec3 dx(zeroV);
			//Apply displacements
			//for(S32 j=0; j<parI.m_Neighbors.m_numElements; ++j)
			//{
			//	C_FluidParticlePair* pPair = &parI.m_Neighbors[j];
			//	C_FluidParticle& parJ = *pPair->GetNeighbor(&parI);

			//	const HVec3 displacement = sqDt *(pressure * pPair->m_ParticlesWeight + pressureNear * pPair->m_ParticlesWeight*pPair->m_ParticlesWeight)* pPair->m_ParticlesDirection;
			//		//Change the neighbor's position
			//	const HVec3 div(displacement * half);
			//	parJ.m_CurPosition += div;
			//	dx -= div;
			//}

			pEdge = parI.m_pFirstEdge;
			while(pEdge)
			{
				C_FluidParticlePair* pPair = &pEdge->m_Pair;
				C_FluidParticle& parJ = *pEdge->m_pOutboundParticle;

				const HVec3 displacement = sqDt *(pressure * pPair->m_ParticlesWeight + pressureNear * pPair->m_ParticlesWeight*pPair->m_ParticlesWeight)* pPair->m_ParticlesDirection;
					//Change the neighbor's position
				const HVec3 div(displacement * half);
				parJ.m_CurPosition += div;
				dx -= div;
				pEdge = pEdge->m_pNext;
			}

			//Update the current particle's position
			parI.m_CurPosition += dx;

		}
	}

	void C_ParticleBasedFluidSystem::ResolveCollisions()
	{
	}

	//Construct particle islands and maintain bounding boxes
	U32 C_ParticleBasedFluidSystem::ConstructIslands(SharedPhysics::C_SafeStack<true>& stack)
	{
		//Go through all the particles and flag them as in no island, then go through and 
		//place them in their respective islands...

		for(U32 a = 0; a < m_NumParticles; ++a)
		{
			m_Particles[a].m_IslandIndex = -1;
			m_Particles[a].m_pNext = m_Particles[a].m_pPrev = NULL;
		}

		//Number of islands...
		U32 islandIndex = 0;

		SharedPhysics::C_MemoryIterator<C_FluidIsland> iter(m_pFluidIslandAllocator);
		
		for(U32 a = 0; a < m_NumParticles; ++a)
		{
			C_FluidParticle& par = m_Particles[a];
			if(par.m_IslandIndex == -1)
			{
				//Create an island...
				S32 size = 1;
				stack.Push(&par, 0);
				C_FluidParticle* pStackTop = &par;

				C_FluidIsland* pIsland = iter.Create();
				pIsland->m_pFirstParticle = &par;
				par.m_pNext = par.m_pPrev = NULL;

				while(size--)
				{
					C_FluidParticle* pParticle = pStackTop;
					pStackTop = stack.Pop<C_FluidParticle>(Maths::Max(0, size-1));
					pParticle->m_IslandIndex = islandIndex;
					C_FluidEdge* pEdge = pParticle->m_pFirstEdge;
					while(pEdge)
					{
						C_FluidParticle* pNextParticle = pEdge->m_pOutboundParticle;
						if(pNextParticle->m_IslandIndex == -1)
						{
							pNextParticle->m_pNext = pIsland->m_pFirstParticle;
							pIsland->m_pFirstParticle->m_pPrev = pNextParticle;
							pIsland->m_pFirstParticle = pNextParticle;

							pNextParticle->m_IslandIndex = islandIndex;
							pStackTop = pNextParticle;
							stack.Push(pNextParticle, size++);
						}
						pEdge = pEdge->m_pNext;
					}
				}
			}
		}
		return islandIndex;
	}

	void C_ParticleBasedFluidSystem::TimeStep(const Maths::HF32 PCREF dt)
	{
		using namespace Maths;
		const HVec3 gravity = m_pExternalConstants->m_Gravity;
		for(U32 i=0; i<m_NumParticles; ++i)
		{
			C_FluidParticle& par = m_Particles[i];
			//Apply external force, just gravity in this moment
			par.m_Velocity +=dt*gravity;
		}
		//Modify velocities with pair wise viscosity impulise
		ApplyViscosity(dt);

		for(U32 i=0; i<m_NumParticles; ++i)
		{
			C_FluidParticle& par = (C_FluidParticle&)m_Particles[i];
			//Save previous position
			par.m_PrePosition = par.m_CurPosition;
			//Advance to predicted position
			par.m_CurPosition += dt* par.m_Velocity;
		}

		//Add and remove springs, change rest lengths
		AdjustSprings(dt);

		//To simulate elastic behavior, add spring between pairs of neighboring particles
		//ApplySpringDisplacements(dt);

		//Calcualte the pressure between particles and using double density to reduce the particle clustering,
		//and also provide sufface tension effects
		DoubleDensityRelaxation(dt);

		//Do Collision Detection
		ResolveCollisions();

		UpdateNeighbours();

		//Update the velocity based on the final position
		for(U32 i = 0; i< m_NumParticles; ++i)
		{
			C_FluidParticle& par = m_Particles[i];
			par.m_Velocity = (par.m_CurPosition - par.m_PrePosition)/dt;
		}
	}
}
