#ifndef GRAVITY_CLOTH_COLLISION_H
#define GRAVITY_CLOTH_COLLISION_H

#include "gravity/Cloths/Cloth.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/sdk/ShareDeserialiser.h"
#include "gravity/narrowphase/Ray.h"
#include "gravity/sharedphysics/SafeStack.h"


namespace Collision
{
	
	/*
	 * Cloth collision detection implemented with a pre-calculated AABB tree based
	 * on the initial state of the cloth. Leaf node has a single cloth triangle in it!
	 *
	 * Based on 
	 */

	class C_CollisionObject;

	class ALIGN16 C_SurfaceCone
	{
	public:
		Maths::C_Normal m_Normal;
		Maths::HF32 m_CosAngle;
		Maths::HF32 m_Weight;

		C_SurfaceCone() : m_Normal(Maths::Constants<Maths::HVec3>::Zero(), Maths::C_Normal::UNIT_LENGTH_VECTOR),
			m_CosAngle(Maths::Constants<Maths::HF32>::One())
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

	class ALIGN16 C_ClothAABBNode
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

		C_ClothAABBNode() : m_PopCount(0)
		{
		}
	};

	class ALIGN16 C_ClothTree : public C_Cloth
	{
		//Only thing this introduces is a hierarchy!
	public:
		C_ClothAABBNode* m_pNodes;
		U32 m_NumNodes;

		C_ClothTree() : m_pNodes(NULL), m_NumNodes(0)
		{
		}

		virtual ~C_ClothTree()
		{
			if(!m_ExternallyOwned && m_pNodes != NULL)
			{
				delete [] m_pNodes;
			}
			m_pNodes = NULL;
		}

		C_ClothTree& operator = (const C_ClothTree& other)
		{
			m_CompressCoefficient = other.m_CompressCoefficient;
			m_Damping = other.m_Damping;
			m_pEdges = other.m_pEdges;
			m_ExternallyOwned = true;
			m_pExternalConstants = other.m_pExternalConstants;
			m_MaxSpeedSq = other.m_MaxSpeedSq;
			m_NumEdges = other.m_NumEdges;
			m_NumIterations = other.m_NumIterations;
			m_NumNodes = other.m_NumNodes;
			m_NumTriangles = other.m_NumTriangles;
			m_pNodes = other.m_pNodes;
			m_StretchCoefficient = other.m_StretchCoefficient;
			m_Thickness = other.m_Thickness;
			m_Triangles = other.m_Triangles;

			SetNumParticles(other.m_numParticles);
			for(U32 a = 0; a < m_numParticles; ++a)
			{
				this->m_Particles[a] = other.m_Particles[a];
			}
			return *this;
		}

		void GetBoundingBox(Collision::C_CRAxisBox& box);

		void Import(Gravity::Deserialiser& serial);

		bool Raytrace(Collision::C_Ray& ray, const Collision::C_CollisionObject* const __restrict pObject, SharedPhysics::C_SafeStack<true>& stack);

		//bool Raytrace(Collision::C_TransparentRay& ray, const Collision::C_CollisionObject* const __restrict pObject, SharedPhysics::C_SafeStack<true>& stack);

		//For now - this method simply finds all contacts and moves cloth out of contact!
		//For now - do this iteratively - find a point of contact & resolve it, then find the next one...
		/*bool CollideAndResolve(Collision::C_CollisionObject* pObject, SharedPhysics::C_SafeStack<true>& stack, bool (*ColFunc)(Collision::C_Shape* pShape, Maths::HVec3 PCREF vertA,
			Maths::HVec3 PCREF vertB, Maths::HVec3 PCREF vertC, Maths::C_Normal PCREF normal, Maths::HVec3& contact, Maths::C_Normal& resultNormal, Maths::HF32& penetration));*/


		bool Collide(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
		SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<ClothContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
		SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_ClothTree& tree, Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts));

		bool CollideParticles(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
		SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<ClothContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
		SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_ClothTree& tree,  const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts));

		void UpdateTree(SharedPhysics::C_SafeStack<true>& stack);

		void TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack);
	};



	//Particle collision function
	bool SphereClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool CapsuleClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts); 

	bool BoxClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool CylinderClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool ConeClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool ConvexHullClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool CompositeClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
	const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<ClothContactSize>& contacts);

	//Triangle collision function
	bool SphereClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool CapsuleClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool BoxClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool CylinderClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode, 
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool ConeClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool ConvexHullClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts);

	bool CompositeClothTriangle(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pClothNode,
		SharedPhysics::C_GraphNode* pNode, Collision::C_ClothTree& tree,
		Collision::C_ClothTriangle& tri, const U32 index, C_ParticleContactBuffer<ClothContactSize>& contacts);

};


#endif

