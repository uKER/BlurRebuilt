#ifndef GRAVITY_SOFTBODY_TREE_H
#define GRAVITY_SOFTBODY_TREE_H

//#include "gravity/Cloths/Cloth.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/sdk/ShareDeserialiser.h"
#include "gravity/narrowphase/Ray.h"
#include "gravity/sharedphysics/SafeStack.h"
#include "gravity/cloths/Particle.h"


namespace Collision
{
	
	/*
	 * Cloth collision detection implemented with a pre-calculated AABB tree based
	 * on the initial state of the cloth. Leaf node has a single cloth triangle in it!
	 *
	 * Based on 
	 */

	class C_CollisionObject;

	class C_SoftBodyEdge
	{
	public:
		Maths::HF32 restLength;
		U32 particleA;
		U32 particleB;
		S32 m_TriangleIndexA, m_TriangleIndexB;
		//F32 restLength;


		bool operator >(const C_SoftBodyEdge& other) const
		{
			return particleA > other.particleA || 
				(particleA == other.particleA && particleB > other.particleB);	
		}

		bool operator <=(const C_SoftBodyEdge& other) const
		{
			return particleA < other.particleA || 
				(particleA == other.particleA && particleB <= other.particleB);	
		}

		bool operator <(const C_SoftBodyEdge& other) const
		{
			return particleA < other.particleA || 
				(particleA == other.particleA && particleB < other.particleB);	
		}

		bool operator == (const C_SoftBodyEdge& other) const
		{
			return (particleA == other.particleA && particleB == other.particleB) || 
				(particleA == other.particleB && particleB == other.particleA);
		}

		template < typename T_Stream > 
		void Serialise( T_Stream& stream ) const
		{
			stream << particleA << particleB << m_TriangleIndexA << m_TriangleIndexB << restLength;
		}

		template < typename T_Stream > 
		void Deserialise( T_Stream& stream )
		{
			stream >> particleA >> particleB >> m_TriangleIndexA >> m_TriangleIndexB >> restLength;
		}

		C_SoftBodyEdge() : restLength(Maths::UNDEFINED)
		{
			particleA = particleB = 0;
			m_TriangleIndexA = m_TriangleIndexB = 0;
			restLength = 0.f;
		}
	};

	class C_SoftBodyTriangle
	{
	public:
		Maths::C_Normal m_Normal;
		union
		{
			U32 m_EdgeIndices[3];
			struct
			{
				U32 m_EdgeIndexA;
				U32 m_EdgeIndexB;
				U32 m_EdgeIndexC;
			};
		};
		union
		{
			U32 m_VertexIndices[3];
			struct
			{
				U32 m_VertexIndexA;
				U32 m_VertexIndexB;
				U32 m_VertexIndexC;
			};
		};

		bool m_Active;

		C_SoftBodyTriangle() : m_Active(true), m_Normal(Maths::UNDEFINED)
		{
		}

		template < typename T_Stream > void Serialise( T_Stream& stream ) const
		{
			stream << m_EdgeIndexA << m_EdgeIndexB << m_EdgeIndexC
				<< m_VertexIndexA << m_VertexIndexB << m_VertexIndexC;
		}
		template < typename T_Stream > void Deserialise( T_Stream& stream )
		{
			stream >> m_EdgeIndexA >> m_EdgeIndexB >> m_EdgeIndexC
				>> m_VertexIndexA >> m_VertexIndexB >> m_VertexIndexC;
		}
	};

	class C_SoftBodyReference
	{
	public:
		Maths::HVec3 m_Reference;
		U32 m_Index;

		C_SoftBodyReference() : m_Reference(Maths::UNDEFINED)
		{
		}
	};

	struct C_EdgeTriangleIndices
	{
		S32 m_TriangleIndexA, m_TriangleIndexB;

		C_EdgeTriangleIndices() : m_TriangleIndexA(-1), m_TriangleIndexB(-1)
		{
		}
	};


	class ALIGN16 C_SurfaceCone
	{
	public:
		Maths::C_Normal m_Normal;
		Maths::HF32 m_CosAngle;
		Maths::HF32 m_Weight;

		C_SurfaceCone() : m_Normal(Maths::Constants<Maths::HVec3>::Zero(), Maths::C_Normal::UNIT_LENGTH_VECTOR),
			m_CosAngle(Maths::Constants<Maths::HF32>::One()), m_Weight(Maths::Constants<Maths::HF32>::Zero())
		{
		}

		void SetNormal(Maths::C_Normal PCREF normal)
		{
			m_Normal = normal;
			m_CosAngle = m_Weight = Maths::Constants<Maths::HF32>::One();
		}

		void Merge(const C_SurfaceCone& coneA, const C_SurfaceCone& coneB)
		{
			const Maths::HF32 sum(coneA.m_Weight + coneB.m_Weight);
			const Maths::HVec3 sumNormal(coneA.m_Normal.m_V * coneA.m_Weight + 
				coneB.m_Normal.m_V * coneB.m_Weight);
			m_Normal = Maths::C_Normal(sumNormal/sum, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			m_Weight = sum;
			const Maths::HF32 cosA((coneA.m_Normal*coneA.m_CosAngle) | m_Normal);
			const Maths::HF32 cosB((coneB.m_Normal*coneB.m_CosAngle) | m_Normal);
			m_CosAngle = Maths::Min(cosA, Maths::Min(cosB, Maths::Min(coneA.m_CosAngle, coneB.m_CosAngle)));
		}
	};

	class ALIGN16 C_SoftBodyAABBNode
	{
	public:
		Collision::C_AxisBoxMinMax m_BoundingBox;
		C_SurfaceCone m_Cone;
		//If num triangles == 0, this is an interior node, otherwise this is a leaf node
		U32 m_NumTriangles;
		U32 m_StartVertexIndex;
		U32 m_NumVertices;
		F32 m_FNumTriangles;
		union
		{
			U32 m_StartTriangleIndex;
			U32 m_RightOffset;
		};
		//U32 m_StartParticleIndex;
		//U32 m_NumParticles;
		S32 m_ParentIndex;
		U32 m_PopCount;
		
		template <typename T_Stream>
		void Deserialise(T_Stream& stream)
		{

			stream >> m_BoundingBox >> m_NumTriangles >> m_StartTriangleIndex >> m_ParentIndex 
				>> m_StartVertexIndex >> m_NumVertices;
			m_FNumTriangles = (F32)m_NumTriangles;
		}

		C_SoftBodyAABBNode() : m_PopCount(0)
		{
		}
	};

	class ALIGN16 C_SoftBodyTree : public C_ConstrainedParticleSystem
	{
		//Only thing this introduces is a hierarchy!
	public:
		C_SoftBodyAABBNode* m_pNodes;
		C_SoftBodyTriangle* m_pTriangles;
		C_EdgeTriangleIndices* m_pEdgeIndices;

		U32 m_numNodes;
		U32 m_numTriangles;
		

		C_SoftBodyTree(const U32 type, const U32 numIteration = 5) : C_ConstrainedParticleSystem(type, numIteration), m_pNodes(NULL), m_numNodes(0), 
			m_numTriangles(0), m_pTriangles(NULL), m_pEdgeIndices(NULL)
		{
		}

		virtual ~C_SoftBodyTree()
		{
			if(m_pNodes != NULL)
				delete [] m_pNodes;
			m_pNodes = NULL;

			if(m_pTriangles != NULL)
				delete[] m_pTriangles;
			m_pTriangles = NULL;
			
			if(m_pEdgeIndices != NULL)
				delete [] m_pEdgeIndices;
			m_pEdgeIndices = NULL;
		}

		
		void RemoveEdge(U32 edgeIndex);

		void InitialiseEdgeTriangleIndices();

		void GetBoundingBox(Collision::C_CRAxisBox& box);

		void Import(Gravity::Deserialiser& serial);

		bool Raytrace(Collision::C_Ray& ray, const Collision::C_CollisionObject* const __restrict pObject, SharedPhysics::C_SafeStack<true>& stack);

		//bool Raytrace(Collision::C_TransparentRay& ray, const Collision::C_CollisionObject* const __restrict pObject, SharedPhysics::C_SafeStack<true>& stack);

		//For now - this method simply finds all contacts and moves cloth out of contact!
		//For now - do this iteratively - find a point of contact & resolve it, then find the next one...
		/*bool CollideAndResolve(Collision::C_CollisionObject* pObject, SharedPhysics::C_SafeStack<true>& stack, bool (*ColFunc)(Collision::C_Shape* pShape, Maths::HVec3 PCREF vertA,
			Maths::HVec3 PCREF vertB, Maths::HVec3 PCREF vertC, Maths::C_Normal PCREF normal, Maths::HVec3& contact, Maths::C_Normal& resultNormal, Maths::HF32& penetration));*/


		bool Collide(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
		SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<SoftBodyContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
		SharedPhysics::C_GraphNode* pNode, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_SoftBodyTree& tree, Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts));

		bool CollideParticles(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
		SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<SoftBodyContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
		SharedPhysics::C_GraphNode* pNode, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_SoftBodyTree& tree,  const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts));

		void UpdateTree(SharedPhysics::C_SafeStack<true>& stack);

		//void TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack);
	};



	//Particle collision function
	bool SphereSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool CapsuleSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts); 

	bool BoxSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool CylinderSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNodee, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool ConeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool ConvexHullSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool CompositeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
	const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	//Triangle collision function
	bool SphereSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool CapsuleSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool BoxSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool CylinderSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool ConeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool ConvexHullSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

	bool CompositeSoftBodyTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pSoftBodyNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_SoftBodyTree& tree,
		Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts);

};


#endif

