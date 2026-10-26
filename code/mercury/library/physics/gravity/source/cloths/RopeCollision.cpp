#include "StdAfx.h"
#include "gravity/cloths/Rope.h"
#include "gravity/cloths/RopeCollision.h"
#include "gravity/sdk/ShareDeserialiser.h"
#include "gravity/sharedphysics/safestack.h"
#include "gravity/narrowphase/primitivetest.inl"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/narrowphase/collisionresult.h"
#include "gravity/narrowphase/AxisBox.inl"
#include "gravity/narrowphase/CapsuleRay.inl"

namespace Collision
{


	void C_RopeTree::RecursiveConstruct(C_RopeAABBNode* pNode, U32 startParticle, U32 numParticles, U32 startEdge, U32 numEdges, U32 edgesPerNode, U32 index)
	{
		pNode->m_BoundingBox.SetInvalidExtents();
		const Maths::HVec3 radius(m_Thickness);
		for(U32 a = startEdge; a < numEdges; ++a)
		{
			pNode->m_BoundingBox.m_min = Maths::Min(m_pParticles[m_pEdges[a].m_IndexA].m_CurPosition, pNode->m_BoundingBox.m_min);
			pNode->m_BoundingBox.m_min = Maths::Min(m_pParticles[m_pEdges[a].m_IndexB].m_CurPosition, pNode->m_BoundingBox.m_min);

			pNode->m_BoundingBox.m_max = Maths::Max(m_pParticles[m_pEdges[a].m_IndexA].m_CurPosition, pNode->m_BoundingBox.m_max);
			pNode->m_BoundingBox.m_max = Maths::Max(m_pParticles[m_pEdges[a].m_IndexB].m_CurPosition, pNode->m_BoundingBox.m_max);
		}

		pNode->m_BoundingBox.m_max += radius;
		pNode->m_BoundingBox.m_min -= radius;

		//Loop throug the edge list and put them evenly on one side and another...
		if(numEdges <= edgesPerNode)
		{
			pNode->m_NumEdges = numEdges;
			pNode->m_NumVertices = numParticles;
			pNode->m_StartEdgeIndex = startEdge;
			pNode->m_StartVertexIndex = startParticle;
		}
		else
		{
			//Split the edge data...
			U32 leftNumEdges = numEdges/2;
			U32 rightNumEdges = numEdges - leftNumEdges;

			Collision::C_AxisBoxMinMax leftBox;
			U32 leftIndex = startParticle;
			for(U32 a = startEdge; a < leftNumEdges; ++a)
			{
				leftIndex = Maths::Max(leftIndex, Maths::Max(m_pEdges[a].m_IndexA, m_pEdges[a].m_IndexB));
			}

			pNode->m_NumEdges = 0;
			pNode->m_NumVertices = 0;
			pNode->m_StartVertexIndex = 0;

			C_RopeAABBNode* pLeftNode = pNode + 1;
			m_numNodes++;
			RecursiveConstruct(pLeftNode, startParticle, leftIndex - startParticle, startEdge, leftNumEdges, edgesPerNode, index + 1);
			//Work out offset to right node...
			C_RopeAABBNode* pRightNode = &m_pNodes[m_numNodes++];

			pNode->m_RightOffset = pRightNode - pNode;

			RecursiveConstruct(pRightNode, leftIndex + 1, numParticles - leftIndex - 1, startEdge + leftNumEdges, rightNumEdges, edgesPerNode, index + 2);
		}

	}

	U32 CalculateNumNodes(U32 numEdges, U32 edgesPerNode)
	{
		if(numEdges <= edgesPerNode)
			return 1;
		return 1 + CalculateNumNodes(numEdges/2, edgesPerNode) + CalculateNumNodes(numEdges - (numEdges/2), edgesPerNode);
	}

	void C_RopeTree::CalculateRopeTree()
	{
		//OK. Work out the rope tree data. Assuming that we have a fixed number of nodes in the tree...

		//1st, calculate the number of particles in the tree...We will use this to decide how many nodes...

		const U32 edgesPerNode = 6; //Arbitrary number for the moment...

		U32 numNodes = CalculateNumNodes(m_numEdges, edgesPerNode);

		m_pNodes = new C_RopeAABBNode[numNodes];
		m_numNodes = 1;

		//OK. Do a top down split now...

		RecursiveConstruct(m_pNodes, 0, m_numParticles, 0, m_numEdges, edgesPerNode, 0);
	}


	void C_RopeTree::GetBoundingBox(Collision::C_CRAxisBox& box)
	{
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Collision::C_AxisBoxMinMax& minMax(this->m_pNodes->m_BoundingBox);
		const Maths::HVec3 centre((minMax.m_min + minMax.m_max)*half);
		const Maths::HVec3 extents(minMax.m_max - centre);
		box.centre = centre;
		box.extents = extents;
	}

	void C_RopeTree::Import(Gravity::Deserialiser& serial)
	{
		//Read in everything!
		serial >> m_numParticles >> m_numEdges >> m_numNodes;

		m_pParticles = new C_GravityParticle[m_numParticles];
		m_pEdges = new C_ConstrainedParticleEdge[m_numEdges];
		m_pNodes = new C_RopeAABBNode[m_numNodes];


		for(U32 a = 0; a < m_numParticles; ++a)
		{
			C_GravityParticle& particle = m_pParticles[a];
			serial >> particle;
			particle.m_PrePosition = particle.m_CurPosition;
		}

		C_RopeEdge edge;
		for(U32 a = 0; a < m_numEdges; ++a)
		{
			serial >> edge;
			m_pEdges[a].m_IndexA = edge.m_IndexA;
			m_pEdges[a].m_IndexB = edge.m_IndexB;
			m_pEdges[a].m_RestLength = edge.m_RestLength;
		}

		for(U32 a = 0; a < m_numNodes; ++a)
		{
			C_RopeAABBNode& node = m_pNodes[a];
			serial >> node;
		}
	}

	void C_RopeTree::UpdateTree(SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.RopeTree.UpdateTree");
		//_RecursiveUpdate(m_pNodes);

		const Maths::HVec3 vZero(Maths::Constants<Maths::HVec3>::Zero());
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Collision::C_RopeAABBNode* pStackTop = m_pNodes;
		S32 size = 1;
		stack.Push(m_pNodes, 0);
		const Maths::HVec3 thickness(m_Thickness);

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_RopeAABBNode* pNode = pStackTop;
			pStackTop = stack.Pop<C_RopeAABBNode>(Maths::Max(size - 1, 0));
			C_RopeAABBNode* pFetchedNode = (C_RopeAABBNode*)Spu::FetchAlloc(pNode, sizeof(C_RopeAABBNode));
			pNode->m_PopCount++;
			if(pFetchedNode->m_NumEdges > 0)
			{
				//Leaf Node...
				pFetchedNode->m_BoundingBox.SetInvalidExtents();
				U32 endIndex = pFetchedNode->m_StartEdgeIndex + pFetchedNode->m_NumEdges;
				for(U32 b = pFetchedNode->m_StartEdgeIndex; b < endIndex; ++b)
				{
					Spu::MemBaseMarker innerMarker;
					C_ConstrainedParticleEdge* pEdge = (C_ConstrainedParticleEdge*)Spu::FetchAlloc(m_pEdges + b, sizeof(C_ConstrainedParticleEdge));;
					//if(tri->m_Active)
					{
						C_GravityParticle* particleA = (C_GravityParticle*)Spu::FetchAlloc(m_pParticles + pEdge->m_IndexA, sizeof(C_GravityParticle));
						C_GravityParticle* particleB = (C_GravityParticle*)Spu::FetchAlloc(m_pParticles + pEdge->m_IndexB, sizeof(C_GravityParticle));
											
						const Maths::HVec3 vertA(particleA->m_CurPosition);
						const Maths::HVec3 vertB(particleB->m_CurPosition);
						pFetchedNode->m_BoundingBox.m_max = Maths::Max(vertA, Maths::Max(vertB, pFetchedNode->m_BoundingBox.m_max));
						pFetchedNode->m_BoundingBox.m_min = Maths::Min(vertA, Maths::Min(vertB, pFetchedNode->m_BoundingBox.m_min));
					}
				} 
				pFetchedNode->m_BoundingBox.m_max += thickness;
				pFetchedNode->m_BoundingBox.m_min -= thickness;
#ifdef PLATFORM_SPU
			Spu::Store(pNode, pFetchedNode, sizeof(C_RopeAABBNode));
#endif
			}
			else
			{
				//Not leaf node!
				if(pNode->m_PopCount > 1)
				{
					C_RopeAABBNode* pChildA = (C_RopeAABBNode*)Spu::FetchAlloc(pNode + 1, sizeof(C_RopeAABBNode));
					C_RopeAABBNode* pChildB = (C_RopeAABBNode*)Spu::FetchAlloc(pNode + pNode->m_RightOffset,sizeof(C_RopeAABBNode));

					pFetchedNode->m_BoundingBox.m_max = Maths::Max(pChildA->m_BoundingBox.m_max, pChildB->m_BoundingBox.m_max);
					pFetchedNode->m_BoundingBox.m_min = Maths::Min(pChildA->m_BoundingBox.m_min, pChildB->m_BoundingBox.m_min);

					pFetchedNode->m_PopCount = 0;

#ifdef PLATFORM_SPU
					Spu::Store(pNode, pFetchedNode, sizeof(C_RopeAABBNode));
#endif
				}
				else
				{
					stack.Push(pNode, size++);
					stack.Push(pNode + pNode->m_RightOffset, size++);
					stack.Push(pNode + 1, size++);
					pStackTop = pNode + 1;
				}
			}
		}

	}


	bool C_RopeTree::Raytrace(Collision::C_Ray& ray, const Collision::C_CollisionObject* const __restrict pObject, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.RopeRaytrace");
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		C_RopeAABBNode* pStackTop = m_pNodes;
		stack.Push(m_pNodes, 0);
		S32 size = 1;
		Maths::HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);
		bool collide = false;
		Maths::HVec3 hitPoint(Maths::UNDEFINED);
		Maths::C_Normal normal(Maths::UNDEFINED);
		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_RopeAABBNode* pNode = pStackTop;
			pStackTop = stack.Pop<C_RopeAABBNode>(Maths::Max(size - 1, 0));

			C_RopeAABBNode* pFetchedNode = (C_RopeAABBNode*)Spu::FetchAlloc(pNode, sizeof(C_RopeAABBNode));
			if(pFetchedNode->m_NumEdges == 0)
			{
				//Interior node!
				//Pick whether or not the ray hits either child!

				C_RopeAABBNode* node1(pNode + pFetchedNode->m_RightOffset);
				C_RopeAABBNode* _node1 = (C_RopeAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(C_RopeAABBNode));
				C_RopeAABBNode* node2(pNode + 1);
				C_RopeAABBNode* _node2 = (C_RopeAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(C_RopeAABBNode));
				S32 b1(IntersectRayAABBMinMax(ray.m_start, ray.m_result.m_contact, _node1->m_BoundingBox.m_min, _node1->m_BoundingBox.m_max, tmin));
				S32 b2(IntersectRayAABBMinMax(ray.m_start, ray.m_result.m_contact, _node2->m_BoundingBox.m_min, _node2->m_BoundingBox.m_max, tmin2));
				stack.Push(node2, size);
				size += b2;
				stack.Push(node1, size);
				size += b1;
				pStackTop = SharedPhysics::Fast_Select<C_RopeAABBNode>(b1!=0, node1, 
					SharedPhysics::Fast_Select<C_RopeAABBNode>(b2!=0, node2, pStackTop));
			}
			else
			{
				//Do triangle collision!
				Spu::MemBaseMarker triMarker;
				Maths::HVec3 rayEnd = ray.m_result.m_contact;
				Maths::HF32 distance(ray.m_result.m_distance);
				Maths::C_Normal normal(ray.m_result.m_normal);
				Maths::HVec3 tempHit(Maths::UNDEFINED);
				U32 triangleIndex = 0;

				bool hit = false;
				U32 endIndex = pFetchedNode->m_NumEdges + pFetchedNode->m_StartEdgeIndex;
				for(U32 a = pFetchedNode->m_StartEdgeIndex; a < endIndex; ++a)
				{
					Spu::MemBaseMarker marker;
					C_ConstrainedParticleEdge* pEdge = (C_ConstrainedParticleEdge*)Spu::FetchAlloc(m_pEdges + a, sizeof(C_ConstrainedParticleEdge));
					//if(pEdge->m_Active)
					{
						C_GravityParticle* particleA = (C_GravityParticle*)Spu::FetchAlloc(m_pParticles + pEdge->m_IndexA, sizeof(C_GravityParticle));
						C_GravityParticle* particleB = (C_GravityParticle*)Spu::FetchAlloc(m_pParticles + pEdge->m_IndexB, sizeof(C_GravityParticle));

						if(CollideCapsuleRay(particleA->m_CurPosition, particleB->m_CurPosition, m_Thickness, ray.m_start, ray.m_end, hitPoint, normal))
						{
							ray.m_end = hitPoint;
							ray.m_result.m_contact = hitPoint;
							ray.m_result.m_normal = normal;
							ray.m_result.m_TriangleNormal = normal;
							ray.m_result.m_distance = (ray.m_result.m_contact - ray.m_start) | ray.m_Direction;
							ray.m_result.m_pShape = pObject->m_pShape;
							ray.m_result.m_pNode = pObject->m_pNodePtr;
							collide = true;
						}

					}
				}
			}
		}
		return collide;
	}


	bool C_RopeTree::CollideParticles(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
	SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<RopeContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
	SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pGraphNode, 
	Collision::C_RopeTree& tree,  const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<RopeContactSize>& contacts))
	{
		return false;
	}

	

	void C_RopeTree::TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack)
	{
		C_Rope::TimeStep(time);
		UpdateTree(stack);
	}


}