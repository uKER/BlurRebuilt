#ifndef PARTICLEBASEDFLUID_H
#define PARTICLEBASEDFLUID_H

#include "gravity/cloths/Particle.h"
#include "gravity/sharedPhysics/SortedArray.h"
#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/narrowphase/AxisBox.h"
#include "gravity/sharedphysics/memoryallocator.h"

namespace Collision
{
	class C_FluidParticlePair;
	class C_FluidParticle;
	class C_FluidEdge;
	class C_FluidEdgeTwin;

	class C_FluidParticle : public C_GravityParticle
	{
	public:
		C_FluidParticle(const Maths::HF32 PCREF radius = Maths::Constants<Maths::HF32>::Zero()) : C_GravityParticle(),
			m_pFirstEdge(NULL), m_pFirstTwinEdge(NULL), m_pNext(NULL), m_pPrev(NULL), m_Velocity(Maths::Constants<Maths::HVec3>::Zero())
		{
		}

		virtual ~C_FluidParticle()
		{
		}

		void AddEdge(C_FluidEdge* pEdge);
		void AddTwinEdge(C_FluidEdgeTwin* pEdge);
		void RemoveEdge(C_FluidEdge* pEdge);
		void RemoveTwinEdge(C_FluidEdgeTwin* pEdge);

		bool HasNeighbour(C_FluidParticle* pParticle) const;


		U32 m_Index;
		S32 m_IslandIndex;
		Maths::HVec3 m_Velocity;
		//All neighbors should has spring constrainting the movement
		//SharedPhysics::SortedArray<C_FluidParticlePair> m_Neighbors;
		C_FluidEdge* m_pFirstEdge;
		C_FluidEdgeTwin* m_pFirstTwinEdge;

		//Linked list of particles in an island...
		C_FluidParticle* m_pNext;
		C_FluidParticle* m_pPrev;

	};

	class C_FluidParticlePair
	{
	public:
		C_FluidParticlePair(C_FluidParticle* pParticleA = NULL, C_FluidParticle* pParticleB = NULL): m_pParticleA(pParticleA), m_pParticleB(pParticleB),
			m_ParticlesDirection(Maths::Constants<Maths::HVec3>::Zero()), m_SpringRestLength(Maths::Constants<Maths::HF32>::Zero()),
			m_ParticlesDistance(Maths::Constants<Maths::HF32>::Zero()), m_ParticlesWeight(Maths::Constants<Maths::HF32>::Zero())
		{
		}

		void SetSpringRestLength(const Maths::HF32 PCREF springRestLength)
		{
			m_SpringRestLength = springRestLength;
		}

		bool operator < (const C_FluidParticlePair& other)
		{
			return (m_pParticleA < other.m_pParticleA) || (m_pParticleA == other.m_pParticleA && m_pParticleB < other.m_pParticleB);
		}

		bool operator <= (const C_FluidParticlePair& other)
		{
			return (m_pParticleA < other.m_pParticleA) || (m_pParticleA == other.m_pParticleA && m_pParticleB <= other.m_pParticleB);
		}

		bool operator > (const C_FluidParticlePair& other)
		{
			return (m_pParticleA > other.m_pParticleA) || (m_pParticleA == other.m_pParticleA && m_pParticleB > other.m_pParticleB);
		}

		bool operator >= (const C_FluidParticlePair& other)
		{
			return (m_pParticleA > other.m_pParticleA) || (m_pParticleA == other.m_pParticleA && m_pParticleB >= other.m_pParticleB);
		}

		bool operator == (const C_FluidParticlePair& other)
		{
			return ((m_pParticleA == other.m_pParticleA && m_pParticleB == other.m_pParticleB) 
				|| (m_pParticleA == other.m_pParticleB && m_pParticleB == other.m_pParticleA));
		}

		bool operator != (const C_FluidParticlePair& other)
		{
			return (m_pParticleA != other.m_pParticleA && m_pParticleA != other.m_pParticleB);
		}

		void SetNeighbor(C_FluidParticle* pParticleA, C_FluidParticle* pParticleB)
		{
			m_pParticleA = pParticleA;
			m_pParticleB = pParticleB;
		}

		void SetNeighborParameter(const Maths::HF32 PCREF distance, const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF weight)
		{
			m_ParticlesDistance = distance;
			m_ParticlesDirection = dir;
			m_ParticlesWeight = weight;
		}

		C_FluidParticle* GetNeighbor(C_FluidParticle* pParticle)
		{
			/*if(pParticle == m_pParticleA)
				return m_pParticleB;*/

			return Maths::Select(pParticle == m_pParticleA, m_pParticleB, m_pParticleA);
		}


		C_FluidParticle* m_pParticleA;
		C_FluidParticle* m_pParticleB;
		Maths::HVec3 m_ParticlesDirection;
		Maths::HF32 m_ParticlesDistance;
		Maths::HF32 m_ParticlesWeight;
		Maths::HF32 m_SpringRestLength;
		
	};

	class C_FluidEdgeTwin : public SharedPhysics::C_BlockItem<C_FluidEdgeTwin>
	{
	public:
		C_FluidEdgeTwin* m_pPrev;
		C_FluidEdge* m_pFluidEdge;

		C_FluidEdgeTwin() : m_pPrev(NULL), m_pFluidEdge(NULL)
		{
		}
	};

	class C_FluidEdge : public SharedPhysics::C_BlockItem<C_FluidEdge>
	{
	public:
		C_FluidEdge* m_pPrev;
		C_FluidParticle* m_pInboundParticle;
		C_FluidParticle* m_pOutboundParticle;
		C_FluidEdgeTwin* m_pTwin;

		C_FluidParticlePair m_Pair;

		C_FluidEdge() : m_pPrev(NULL), m_pInboundParticle(NULL), m_pOutboundParticle(NULL),
			m_pTwin(NULL)
		{
		}
	};

	class C_FluidIsland
	{
	public:
		Collision::C_AxisBoxMinMax m_BoundingBox;
		C_FluidParticle* m_pFirstParticle;

		C_FluidIsland() : m_pFirstParticle(NULL)
		{
		}
	};

	typedef SharedPhysics::C_PointerWrapper<C_FluidParticle> FluidParticlePtr;

	class C_ParticleBasedFluidSystem 
	{
	public:
		C_ParticleBasedFluidSystem():m_NumParticles(0), m_Particles(NULL), m_Stiffness(Maths::Constants<Maths::HF32>::Zero()),
			m_StiffnessNear(Maths::Constants<Maths::HF32>::Zero()), m_SpringCoefficient(Maths::Constants<Maths::HF32>::Zero()),
			m_SpringRestLength(Maths::Constants<Maths::HF32>::Zero()), m_PlasticConstant(Maths::Constants<Maths::HF32>::Zero()),
			m_YieldRatio(Maths::Constants<Maths::HF32>::Zero()),m_LinearViscosityConstant(Maths::Constants<Maths::HF32>::Zero()),
			m_QuadraticViscosityConstant(Maths::Constants<Maths::HF32>::Zero()),m_RestDensity(Maths::Constants<Maths::HF32>::Zero()),
			m_ParticleRadius(Maths::Constants<Maths::HF32>::Zero())
		{
			m_pEdges = new SharedPhysics::C_AllocationManager<C_FluidEdge>;
			m_pEdgeTwins = new SharedPhysics::C_AllocationManager<C_FluidEdgeTwin>;
			m_pFluidIslandAllocator = new SharedPhysics::MemoryAllocator<C_FluidIsland>;
		}

		~C_ParticleBasedFluidSystem()
		{
			if(m_Particles)
				delete[] m_Particles;
			delete m_pEdges;
			delete m_pEdgeTwins;
			delete m_pFluidIslandAllocator;
		}

		void Initialise()
		{
			Maths::HVec3 zeroV(Maths::Constants<Maths::HVec3>::Zero());
			for(U32 i=0; i< m_NumParticles; ++i)
			{
				C_FluidParticle& particle = m_Particles[i];
				particle.m_PrePosition = particle.m_CurPosition;
				particle.m_Velocity = zeroV;
			}
		}
		void SetRestDensity(const Maths::HF32 PCREF restDensity)
		{
			m_RestDensity = restDensity;
		}

		void SetStiffness(const Maths::HF32 PCREF stiffness)
		{
			m_Stiffness = stiffness;
		}

		void SetStiffnessNear(const Maths::HF32 PCREF stiffnessNear)
		{
			m_StiffnessNear = stiffnessNear;
		}

		void SetParticleRadius(const Maths::HF32 PCREF radius)
		{
			m_ParticleRadius = radius;
			m_SpringRestLength = radius;
		}

		void SetSpringCoefficient(const Maths::HF32 PCREF springCoefficient)
		{
			m_SpringCoefficient = springCoefficient;
		}

		void SetExternalConstants(Gravity::C_ExternalConstants* pConstants)
		{
			m_pExternalConstants = pConstants;
		}

	/*	void SetSpringRestLength(const Maths::HF32 PCREF springRestLength)
		{
			m_SpringRestLength = springRestLength;
		}*/

		void SetPlasticConstant(const Maths::HF32 PCREF plasticConstant)
		{
			m_PlasticConstant = plasticConstant;
		}

		void SetYieldRatio(const Maths::HF32 PCREF yieldRatio)
		{
			m_YieldRatio = yieldRatio;
		}

		void SetLinearViscosityConstants(const Maths::HF32 PCREF linearViscosityConstant)
		{
			m_LinearViscosityConstant = linearViscosityConstant;
		}

		void SetQuadraticViscosityConstants(const Maths::HF32 PCREF quadraticViscosityConstant)
		{
			m_QuadraticViscosityConstant = quadraticViscosityConstant;
		}

		void SetNumParticles(const U32 numParticles)
		{
			m_NumParticles = numParticles;
			m_Particles = new C_FluidParticle[m_NumParticles];
		}


		void ApplyViscosity(const Maths::HF32 PCREF dt);
		void AdjustSprings(const Maths::HF32 PCREF dt);
		void ApplySpringDisplacements(const Maths::HF32 PCREF dt);
		void DoubleDensityRelaxation(const Maths::HF32 PCREF dt);
		void ResolveCollisions();

		U32 ConstructIslands(SharedPhysics::C_SafeStack<true>& stack);

		void InitialiseNeighbours();
		void UpdateNeighbours();
		void ConnectNeighbours();
		void DisConnectNeighbours();

		void TimeStep(const Maths::HF32 PCREF dt);

		C_FluidEdge* CreateFluidEdge(C_FluidParticle* pParticleA, C_FluidParticle* pParticleB);
		C_FluidEdgeTwin* CreateFluidEdgeTwin(C_FluidEdge* pEdge);

		void ReleaseFluidEdge(C_FluidEdge* pEdge);
		void ReleaseFluidEdgeTwin(C_FluidEdgeTwin* pEdge);

		Maths::HF32 m_Stiffness; //used to calculate the pressure between two particles
		Maths::HF32 m_StiffnessNear;

		Maths::HF32 m_SpringCoefficient;
		Maths::HF32 m_SpringRestLength;
		Maths::HF32 m_PlasticConstant;
		Maths::HF32 m_YieldRatio; //used to calculate the tolerable deformation

		Maths::HF32 m_LinearViscosityConstant;	// used to calculate the viscosity of the fluid
		Maths::HF32 m_QuadraticViscosityConstant;

		Maths::HF32 m_RestDensity; 
		Maths::HF32 m_ParticleRadius;

		//SharedPhysics::SortedArray<C_FluidParticle *> m_SortedParticles;

		C_FluidParticle* m_Particles;
		U32 m_NumParticles;
		Gravity::C_ExternalConstants* m_pExternalConstants;

	private:
		SharedPhysics::C_AllocationManager<C_FluidEdge>* m_pEdges;
		SharedPhysics::C_AllocationManager<C_FluidEdgeTwin>* m_pEdgeTwins;
		SharedPhysics::MemoryAllocator<C_FluidIsland>* m_pFluidIslandAllocator;
	};
}

#endif