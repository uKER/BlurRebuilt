#ifndef GRAVITY_ROPE_COLLISION_H
#define GRAVITY_ROPE_COLLISION_H

#include "gravity/Cloths/Rope.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/sdk/ShareDeserialiser.h"
#include "gravity/narrowphase/Ray.h"
#include "gravity/sharedphysics/SafeStack.h"
#include "gravity/sdk/ShareDeserialiser.h"


namespace Collision
{
	const U32 RopeContactSize = 64;
	class C_CollisionObject;

	class C_RopeAABBNode
	{
	public:
		Collision::C_AxisBoxMinMax m_BoundingBox;
		U32 m_StartVertexIndex;
		U32 m_NumVertices;
		U32 m_NumEdges;
		union
		{
			U32 m_StartEdgeIndex;
			U32 m_RightOffset;
		};
		U32 m_PopCount;

		template <typename T_Stream>
		void Deserialise(T_Stream& stream)
		{
			stream >> m_BoundingBox >> m_NumEdges >> m_StartEdgeIndex 
				>> m_StartVertexIndex >> m_NumVertices;
		}

		C_RopeAABBNode() : m_PopCount(0)
		{
		}
	};

	class ALIGN16 C_RopeTree : public C_Rope
	{
		//Only thing this introduces is a hierarchy!
	public:
		C_RopeAABBNode* m_pNodes;
		U32 m_numNodes;

		C_RopeTree() : m_pNodes(NULL), m_numNodes(0)
		{
		}

		virtual ~C_RopeTree()
		{
			if(m_pNodes != NULL)
			{
				delete [] m_pNodes;
			}
			m_pNodes = NULL;
		}

		C_RopeTree& operator = (const C_RopeTree& other)
		{
			m_CompressCoefficient = other.m_CompressCoefficient;
			//m_Damping = other.m_Damping;
			m_pEdges = other.m_pEdges;
			m_pExternalConstants = other.m_pExternalConstants;
			m_MaxSpeedSq = other.m_MaxSpeedSq;
			m_numEdges = other.m_numEdges;
			m_numIterations = other.m_numIterations;
			m_numNodes = other.m_numNodes;
			m_pNodes = other.m_pNodes;
			m_StretchCoefficient = other.m_StretchCoefficient;
			m_Thickness = other.m_Thickness;
			m_numParticles = other.m_numParticles;

			if(m_pNodes)
				delete[] m_pNodes;

			if(m_pEdges)
				delete[] m_pEdges;

			if(m_pParticles)
				delete[] m_pParticles;

			m_pNodes = new C_RopeAABBNode[m_numNodes];
			m_pEdges = new C_ConstrainedParticleEdge[m_numEdges];
			m_pParticles = new C_GravityParticle[m_numParticles];

			for(U32 a = 0; a < m_numNodes; ++a)
			{
				m_pNodes[a] = other.m_pNodes[a];
			}

			for(U32 a =0; a< m_numEdges; ++a)
			{
				m_pEdges[a] = other.m_pEdges[a];
			}

			for(U32 a = 0; a < m_numParticles; ++a)
			{
				m_pParticles[a] = other.m_pParticles[a];
			}
			return *this;
		}

		void GetBoundingBox(Collision::C_CRAxisBox& box);

		void Import(Gravity::Deserialiser& serial);

		bool Raytrace(Collision::C_Ray& ray, const Collision::C_CollisionObject* const __restrict pObject, SharedPhysics::C_SafeStack<true>& stack);


		bool CollideParticles(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
		SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<SoftBodyContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
		SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_RopeTree& tree,  const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts));

		void UpdateTree(SharedPhysics::C_SafeStack<true>& stack);

		void TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack);

		void CalculateRopeTree();

	private:
		void RecursiveConstruct(C_RopeAABBNode* m_pNodes, U32 startParticle, U32 numParticles, U32 startEdge, U32 numEdges, U32 edgesPerNode, U32 index);

	};




	

	bool SphereRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts);

	bool CapsuleRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts);

	bool BoxRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts);

	bool CylinderRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts);

	bool ConeRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts);

	bool ConvexHullRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts);

	bool CompositeRopeParticles(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pRopeNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_RopeTree& tree,
	const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts);

};


#endif