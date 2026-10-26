#include "StdAfx.h"
#include "gravity/cloths/SoftBody.h"
#include "gravity/sdk/ShareDeserialiser.h"
#include "gravity/sharedphysics/safestack.h"
#include "gravity/narrowphase/primitivetest.inl"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/narrowphase/collisionresult.h"
#include "gravity/narrowphase/AxisBox.inl"
#include "system/perf.h"

namespace Collision
{
#ifndef PLATFORM_SPU
	void C_SoftBodyTree::Import(Gravity::Deserialiser &serial)
	{
		//Read in everything!
		serial >> m_numParticles >> m_numEdges >> m_numTriangles >> m_numNodes;

		m_pParticles = new C_GravityParticle[m_numParticles];
		//Edges don't have triangle reference data in them, we will need to add a new data structure
		//with these in!!!
		m_pEdges = new C_ConstrainedParticleEdge[m_numEdges];
		m_pEdgeIndices = new C_EdgeTriangleIndices[m_numEdges];
		m_pTriangles = new C_SoftBodyTriangle[m_numTriangles];
		m_pNodes = new C_SoftBodyAABBNode[m_numNodes];
		//Import in the vertices

		for(U32 a = 0; a < m_numParticles; ++a)
		{
			C_GravityParticle& particle = m_pParticles[a];
			serial >> particle;
			particle.m_PrePosition = particle.m_CurPosition;
		}

		C_SoftBodyEdge edge;
		for(U32 a = 0; a < m_numEdges; ++a)
		{
			serial >> edge;
			m_pEdges[a].m_IndexA = edge.particleA;
			m_pEdges[a].m_IndexB = edge.particleB;
			m_pEdges[a].m_RestLength = edge.restLength;
			m_pEdgeIndices[a].m_TriangleIndexA = edge.m_TriangleIndexA;
			m_pEdgeIndices[a].m_TriangleIndexB = edge.m_TriangleIndexB;
		}

		for(U32 a = 0; a < m_numTriangles; ++a)
		{
			C_SoftBodyTriangle& tri = m_pTriangles[a];
			serial >> tri;
		}

		for(U32 a = 0; a < m_numNodes; ++a)
		{
			C_SoftBodyAABBNode& node = m_pNodes[a];
			serial >> node;
		}
	}

	/*void C_SoftBodyTree::TimeStep(const Maths::HF32 PCREF time, SharedPhysics::C_SafeStack<true>& stack)
	{
		TimeStep(time)
		UpdateTree(stack);
	}*/

	void C_SoftBodyTree::GetBoundingBox(Collision::C_CRAxisBox& box)
	{
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		Collision::C_AxisBoxMinMax& minMax(this->m_pNodes->m_BoundingBox);
		const Maths::HVec3 centre((minMax.m_min + minMax.m_max)*half);
		const Maths::HVec3 extents(minMax.m_max - centre);
		box.centre = centre;
		box.extents = extents;
	}
//#pragma optimize("", off)
	void C_SoftBodyTree::UpdateTree(SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.ClothTree.UpdateTree");
		//_RecursiveUpdate(m_pNodes);

		const Maths::HVec3 vZero(Maths::Constants<Maths::HVec3>::Zero());
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		Collision::C_SoftBodyAABBNode* pStackTop = m_pNodes;
		S32 size = 1;
		stack.Push(m_pNodes, 0);
		const Maths::HVec3 thickness(m_Thickness);

		for(U32 a = 0; a < m_numParticles; ++a)
		{
			Collision::C_GravityParticle& par = m_pParticles[a];
			par.m_AverageNormal = vZero;
			par.m_NormalCount = zero;
		}

		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_SoftBodyAABBNode* pNode = pStackTop;
			pStackTop = stack.Pop<C_SoftBodyAABBNode>(Maths::Max(size - 1, 0));
			C_SoftBodyAABBNode* pFetchedNode = (C_SoftBodyAABBNode*)Spu::FetchAlloc(pNode, sizeof(C_SoftBodyAABBNode));
			pNode->m_PopCount++;
			if(pFetchedNode->m_NumTriangles > 0)
			{
				//Leaf Node...
				Maths::HF32 triCount(zero);
				pFetchedNode->m_BoundingBox.SetInvalidExtents();
				U32 endIndex = pFetchedNode->m_StartTriangleIndex + pFetchedNode->m_NumTriangles;
				Maths::HVec3 avgNormal(Maths::Constants<Maths::HVec3>::Zero());
				for(U32 b = pFetchedNode->m_StartTriangleIndex; b < endIndex; ++b)
				{
					Spu::MemBaseMarker innerMarker;
					C_SoftBodyTriangle* tri = (C_SoftBodyTriangle*)Spu::FetchAlloc(m_pTriangles + b, sizeof(C_SoftBodyTriangle));
					if(tri->m_Active)
					{
						triCount += one;
						C_GravityParticle* particleA = (C_GravityParticle*)Spu::FetchAlloc(m_pParticles + tri->m_VertexIndexA, sizeof(C_GravityParticle));
						C_GravityParticle* particleB = (C_GravityParticle*)Spu::FetchAlloc(m_pParticles + tri->m_VertexIndexB, sizeof(C_GravityParticle));
						C_GravityParticle* particleC = (C_GravityParticle*)Spu::FetchAlloc(m_pParticles + tri->m_VertexIndexC, sizeof(C_GravityParticle));
											
						const Maths::HVec3 vertA(particleA->m_CurPosition);
						const Maths::HVec3 vertB(particleB->m_CurPosition);
						const Maths::HVec3 vertC(particleC->m_CurPosition);

						const Maths::C_Normal triNormal((vertB - vertA) ^ (vertC - vertA));
						tri->m_Normal = triNormal;

						avgNormal += triNormal;
						particleA->m_AverageNormal += triNormal;
						particleB->m_AverageNormal += triNormal;
						particleC->m_AverageNormal += triNormal;
						particleA->m_NormalCount += one;
						particleB->m_NormalCount += one;
						particleC->m_NormalCount += one;

						pFetchedNode->m_BoundingBox.m_max = Maths::Max(pFetchedNode->m_BoundingBox.m_max, Maths::Max(vertB, Maths::Max(vertC, vertA)) + thickness);
						pFetchedNode->m_BoundingBox.m_min = Maths::Min(pFetchedNode->m_BoundingBox.m_min, Maths::Min(vertB, Maths::Min(vertC, vertA)) - thickness);
					}
				}
				/*pFetchedNode->m_BoundingBox.m_max += thickness;
				pFetchedNode->m_BoundingBox.m_min -= thickness;*/
				pFetchedNode->m_Cone.m_Weight = triCount;
				const Maths::C_Normal surfaceNormal(avgNormal);
				pFetchedNode->m_Cone.m_Normal = surfaceNormal;
				pFetchedNode->m_Cone.m_CosAngle = one;
			
				for(U32 b = pFetchedNode->m_StartTriangleIndex; b < endIndex; ++b)
				{
					Spu::MemBaseMarker innerMarker;
					C_SoftBodyTriangle* tri = (C_SoftBodyTriangle*)Spu::FetchAlloc(m_pTriangles + b, sizeof(C_SoftBodyTriangle));
					pFetchedNode->m_Cone.m_CosAngle = Maths::Min(pFetchedNode->m_Cone.m_CosAngle, tri->m_Normal | surfaceNormal);
				}
			}
			else
			{
				//Not leaf node!
				if(pNode->m_PopCount > 1)
				{
					C_SoftBodyAABBNode* pChildA = (C_SoftBodyAABBNode*)Spu::FetchAlloc(pNode + 1, sizeof(C_SoftBodyAABBNode));
					C_SoftBodyAABBNode* pChildB = (C_SoftBodyAABBNode*)Spu::FetchAlloc(pNode + pNode->m_RightOffset,sizeof(C_SoftBodyAABBNode));

					pFetchedNode->m_BoundingBox.m_max = Maths::Max(pChildA->m_BoundingBox.m_max, pChildB->m_BoundingBox.m_max);
					pFetchedNode->m_BoundingBox.m_min = Maths::Min(pChildA->m_BoundingBox.m_min, pChildB->m_BoundingBox.m_min);

					pFetchedNode->m_Cone.Merge(pChildA->m_Cone, pChildB->m_Cone);
					pFetchedNode->m_PopCount = 0;
				}
				else
				{
					stack.Push(pNode, size++);
					stack.Push(pNode + pNode->m_RightOffset, size++);
					stack.Push(pNode + 1, size++);
					pStackTop = pNode + 1;
				}
			}
#ifdef PLATFORM_SPU
			Spu::Store(pNode, pFetchedNode, sizeof(C_ClothAABBNode));
#endif
		}

		for(U32 a = 0; a < this->m_numParticles; ++a)
		{
			Collision::C_GravityParticle& par = m_pParticles[a];
			if(par.m_Active)
			{
				//par.m_AverageNormal /= par.m_NormalCount;
				par.m_AverageNormal.Normalise();
			}
		}

	}

//#pragma optimize("", on)
#endif

	bool C_SoftBodyTree::Raytrace(Collision::C_Ray& ray, const Collision::C_CollisionObject* const __restrict pObject, SharedPhysics::C_SafeStack<true>& stack)
	{
		PERF_EVENT("Gravity.ClothRaytrace");
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		C_SoftBodyAABBNode* pStackTop = m_pNodes;
		stack.Push(m_pNodes, 0);
		S32 size = 1;
		Maths::HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);
		bool collide = false;
		while(size--)
		{
			Spu::MemBaseMarker marker;
			C_SoftBodyAABBNode* pNode = pStackTop;
			pStackTop = stack.Pop<C_SoftBodyAABBNode>(Maths::Max(size - 1, 0));

			C_SoftBodyAABBNode* pFetchedNode = (C_SoftBodyAABBNode*)Spu::FetchAlloc(pNode, sizeof(C_SoftBodyAABBNode));
			if(IntersectRayAABBMinMax(ray.m_start, ray.m_result.m_contact, pFetchedNode->m_BoundingBox.m_min, pFetchedNode->m_BoundingBox.m_max))
			{
				if(pFetchedNode->m_NumTriangles == 0)
				{
					//Interior node!
					//Pick whether or not the ray hits either child!
					C_SoftBodyAABBNode* node1(pNode + pFetchedNode->m_RightOffset);
					stack.Push(node1, size);
					size += 2;
					pStackTop = pNode + 1;
				}
				else
				{
					//Do triangle collision!
					Spu::MemBaseMarker triMarker;
					Maths::HVec3 rayEnd = ray.m_result.m_contact;
					Maths::HF32 distance(ray.m_result.m_distance);
					Maths::C_Normal normal(ray.m_result.m_TriangleNormal);
					Maths::HVec3 tempHit(Maths::UNDEFINED);
					U32 triangleIndex = 0;

					bool hit = false;
					U32 endIndex = pFetchedNode->m_NumTriangles + pFetchedNode->m_StartTriangleIndex;
					for(U32 a = pFetchedNode->m_StartTriangleIndex; a < endIndex; ++a)
					{
						Spu::MemBaseMarker marker;
						C_SoftBodyTriangle* tri = (C_SoftBodyTriangle*)Spu::FetchAlloc(m_pTriangles + a, sizeof(C_SoftBodyTriangle));
						if(tri->m_Active)
						{
							
							C_GravityParticle* particleA = (C_GravityParticle*)Spu::FetchAllocAsync(m_pParticles + tri->m_VertexIndexA, sizeof(C_GravityParticle), 2);
							C_GravityParticle* particleB = (C_GravityParticle*)Spu::FetchAllocAsync(m_pParticles + tri->m_VertexIndexB, sizeof(C_GravityParticle), 2);
							C_GravityParticle* particleC = (C_GravityParticle*)Spu::FetchAllocAsync(m_pParticles + tri->m_VertexIndexC, sizeof(C_GravityParticle), 2);

							Spu::Wait(2);
							//Raytrace against the triangle. NB - the triangle normal will be whichever normal is pointing
							//in the opposite direction. Because of this, we need bi-directional raytracing!
							const Maths::HF32 direction(tri->m_Normal | ray.m_Direction);
							const Maths::Scalar_Bool flipRay(direction.GT(zero));
							const Maths::HVec3 rayP(Maths::Select(flipRay, rayEnd, ray.m_start));
							const Maths::HVec3 rayQ(Maths::Select(flipRay, ray.m_start, rayEnd));
							
							const Maths::Scalar_Bool isHit = Collision::IntersectTriangleSegmentSB(particleA->m_CurPosition, particleB->m_CurPosition, particleC->m_CurPosition,
									rayP, rayQ, tempHit);
							normal = Maths::Select(isHit, Maths::Select(flipRay, -tri->m_Normal, tri->m_Normal), normal);

							bool bIsHit(isHit);
							Maths::HF32 tempDistance(Maths::Abs((tempHit - ray.m_start) | ray.m_Direction));
							rayEnd = Maths::Select(isHit, tempHit, rayEnd);
							distance = Maths::Select(isHit, tempDistance, distance);
							triangleIndex = Maths::Select(bIsHit, a, triangleIndex);
							collide = collide | bIsHit;
							hit = hit | bIsHit;
						}
					}

					if(hit)
					{
						ray.m_result.m_contact = rayEnd;
						ray.m_result.m_distance = distance;
						ray.m_result.m_TriangleNormal = normal;
						ray.m_result.m_normal = normal;
						ray.m_result.m_TriangleIndex = triangleIndex;
						ray.m_result.m_pNode = pObject->m_pNodePtr;
						ray.m_result.m_pShape = pObject->m_pShape;
					}
				}
			}
		}
		return collide;
	}

	void C_SoftBodyTree::RemoveEdge(U32 edgeIndex)
	{
		Collision::C_ConstrainedParticleEdge& edge = m_pEdges[edgeIndex];
		//m_pEdges[edgeIndex] = m_pEdges[m_NumEdges - 1];
		edge.m_Active = false;

		Collision::C_GravityParticle& parA = m_pParticles[edge.m_IndexA];
		Collision::C_GravityParticle& parB = m_pParticles[edge.m_IndexB];

		U32 endA = this->m_pParticleHeader[edge.m_IndexA + 1];
		U32 endB = this->m_pParticleHeader[edge.m_IndexB + 1];
	
		U32 numActive = 0;
		for(U32 a = m_pParticleHeader[edge.m_IndexA]; a < endA; ++a)
		{
			if(m_pParticleEdgeTable[a] == edgeIndex)
			{
				m_pParticleEdgeTable[a] = -1;
			}
			else if (m_pParticleEdgeTable[a] != -1)
			{
				++numActive;
			}
		}
		if(numActive == 0)
		{
			parA.m_Active = false;
		}

		numActive = 0;
		for(U32 a = m_pParticleHeader[edge.m_IndexB]; a < endB; ++a)
		{
			if(m_pParticleEdgeTable[a] == edgeIndex)
			{
				m_pParticleEdgeTable[a] = -1;
			}
			else if (m_pParticleEdgeTable[a] != -1)
			{
				++numActive;
			}
		}
		if(numActive == 0)
		{
			parB.m_Active = false;
		}

		//Now remove the triangles...

		Collision::C_EdgeTriangleIndices& inds = m_pEdgeIndices[edgeIndex];
		Collision::C_SoftBodyTriangle& triA = m_pTriangles[inds.m_TriangleIndexA];
		Collision::C_SoftBodyTriangle& triB = m_pTriangles[inds.m_TriangleIndexB];

		triA.m_Active = false;
		triB.m_Active = false;
	}

	
	//bool C_ClothTree::CollideAndResolve(Collision::C_CollisionObject* pObject, SharedPhysics::C_SafeStack<true>& stack, bool (*ColFunc)(Collision::C_Shape* pShape, Maths::HVec3 PCREF vertA,
	//	Maths::HVec3 PCREF vertB, Maths::HVec3 PCREF vertC, Maths::C_Normal PCREF normal, Maths::HVec3& contact, Maths::C_Normal& resultNormal, Maths::HF32& penetration))
	//{
	//	/*
	//	 * (1) Get bounding box for object
	//	 * (2) Find triangles to test against
	//	 * (3) Do collision detection
	//	 * (4) If hit, project cloth outwards
	//	 */

	//	C_ClothAABBNode* pStackTop = m_pNodes;
	//	S32 size = 1;
	//	stack.Push(m_pNodes, 0);

	//	Maths::HVec3 contact(Maths::UNDEFINED);
	//	Maths::C_Normal normal(Maths::UNDEFINED);
	//	Maths::HF32 penetration(Maths::UNDEFINED);

	//	bool bResult = false;

	//	while(size--)
	//	{
	//		C_ClothAABBNode* pNode = pStackTop;
	//		pStackTop = stack.Pop<C_ClothAABBNode>(Maths::Max(size - 1, 0));

	//		if(pNode->m_NumTriangles == 0)
	//		{
	//			//Test to see if we hit the next node down!
	//			C_ClothAABBNode* node1(pNode + pNode->m_RightOffset);
	//			C_ClothAABBNode* node2(pNode + 1);
	//			S32 b1(node1->m_BoundingBox.Intersects(pObject->m_TransformedBox));
	//			S32 b2(node2->m_BoundingBox.Intersects(pObject->m_TransformedBox));
	//			stack.Push(node2, size);
	//			size += b2;
	//			stack.Push(node1, size);
	//			size += b1;
	//			pStackTop = SharedPhysics::Fast_Select<C_ClothAABBNode>(b1!=0, node1, 
	//				SharedPhysics::Fast_Select<C_ClothAABBNode>(b2!=0, node2, pStackTop));
	//		}
	//		else
	//		{
	//			//We have a hit! Loop through all the triangles and call collide on them. If we've hit it,
	//			//resolve the contact using a projection-based approach!
	//			U32 endIndex = pNode->m_StartTriangleIndex + pNode->m_NumTriangles;
	//			for(U32 a = pNode->m_StartTriangleIndex; a < endIndex; ++a)
	//			{
	//				C_ClothTriangle& tri = m_Triangles[a];
	//				Maths::HVec3 vertA(m_Particles[tri.m_VertexIndexA].m_CurPosition);
	//				Maths::HVec3 vertB(m_Particles[tri.m_VertexIndexB].m_CurPosition);
	//				Maths::HVec3 vertC(m_Particles[tri.m_VertexIndexC].m_CurPosition);
	//				if(ColFunc(pObject->m_pShape, vertA, vertB, vertC, tri.m_Normal, contact, normal, penetration))
	//				{
	//					bResult = true;
	//					//Project the triangle out by its penetration/normal!
	//					//NB - normal always pointing from object to cloth!

	//					/*m_Particles[tri.m_VertexIndexA].m_PrePosition = m_Particles[tri.m_VertexIndexA].m_CurPosition;
	//					m_Particles[tri.m_VertexIndexB].m_PrePosition = m_Particles[tri.m_VertexIndexB].m_CurPosition;
	//					m_Particles[tri.m_VertexIndexC].m_CurPosition = m_Particles[tri.m_VertexIndexC].m_CurPosition;*/
	//					
	//					m_Particles[tri.m_VertexIndexA].m_CurPosition = vertA + normal * penetration;
	//					m_Particles[tri.m_VertexIndexB].m_CurPosition = vertB + normal * penetration;
	//					m_Particles[tri.m_VertexIndexC].m_CurPosition = vertC + normal * penetration;
	//				}	
	//			}

	//		}
	//	}
	//	return bResult;
	//}

	bool C_SoftBodyTree::Collide(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
		SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<SoftBodyContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
		SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_SoftBodyTree& tree, Collision::C_SoftBodyTriangle& tri, const U32 index, C_ParticleContactBuffer<SoftBodyContactSize>& contacts))
	{
		C_SoftBodyAABBNode* pStackTop = m_pNodes;
		S32 size = 1;
		stack.Push(m_pNodes, 0);

		Maths::HVec3 contact(Maths::UNDEFINED);
		Maths::C_Normal normal(Maths::UNDEFINED);
		Maths::HF32 penetration(Maths::UNDEFINED);

		bool bResult = false;

		while(size--)
		{
			C_SoftBodyAABBNode* pNode = pStackTop;
			pStackTop = stack.Pop<C_SoftBodyAABBNode>(Maths::Max(size - 1, 0));

			if(pNode->m_NumTriangles == 0)
			{
				//Test to see if we hit the next node down!
				C_SoftBodyAABBNode* node1(pNode + pNode->m_RightOffset);
				C_SoftBodyAABBNode* node2(pNode + 1);
				S32 b1(node1->m_BoundingBox.Intersects(pObject->m_TransformedBox));
				S32 b2(node2->m_BoundingBox.Intersects(pObject->m_TransformedBox));
				stack.Push(node2, size);
				size += b2;
				stack.Push(node1, size);
				size += b1;
				pStackTop = SharedPhysics::Fast_Select<C_SoftBodyAABBNode>(b1!=0, node1, 
					SharedPhysics::Fast_Select<C_SoftBodyAABBNode>(b2!=0, node2, pStackTop));
			}
			else
			{
				//We have a hit! Loop through all the triangles and call collide on them. If we've hit it,
				//resolve the contact using a projection-based approach!
				U32 endIndex = pNode->m_StartTriangleIndex + pNode->m_NumTriangles;
				for(U32 a = pNode->m_StartTriangleIndex; a < endIndex; ++a)
				{
					C_SoftBodyTriangle& tri = m_pTriangles[a];
					if(tri.m_Active)
						bResult = ColFunc(pObject->m_pShape, pThisNode, pObject->m_pNodePtr, *this, tri, a, contacts) | bResult;

					//Collision::C_ClothConstraint constraint;
					//constraint.m_Contact = contact;
					//constraint.m_EscapeVector = normal;
					//constraint.m_TriangleIndex = a;
					//constraint.m_Penetration = penetration;
					//constraint.m_Stiffness = m_Thickness;
					//constraint.m_pNodePtr = pObject->m_pNodePtr;
				}

			}
		}
		return bResult;
	}


		bool C_SoftBodyTree::CollideParticles(Collision::C_CollisionObject* pObject, SharedPhysics::C_GraphNode* pThisNode,
		SharedPhysics::C_SafeStack<true>& stack, C_ParticleContactBuffer<SoftBodyContactSize>& contacts, bool (*ColFunc)(Collision::C_Shape* pShape, 
		SharedPhysics::C_GraphNode* pClothNode, SharedPhysics::C_GraphNode* pGraphNode, 
		Collision::C_SoftBodyTree& tree,  const U32 startIndex, const U32 numVerts, C_ParticleContactBuffer<SoftBodyContactSize>& contacts))
	{
		C_SoftBodyAABBNode* pStackTop = m_pNodes;
		S32 size = 1;
		stack.Push(m_pNodes, 0);

		Maths::HVec3 contact(Maths::UNDEFINED);
		Maths::C_Normal normal(Maths::UNDEFINED);
		Maths::HF32 penetration(Maths::UNDEFINED);

		bool bResult = false;

		while(size--)
		{
			C_SoftBodyAABBNode* pNode = pStackTop;
			pStackTop = stack.Pop<C_SoftBodyAABBNode>(Maths::Max(size - 1, 0));

			if(pNode->m_NumTriangles == 0)
			{
				//Test to see if we hit the next node down!
				C_SoftBodyAABBNode* node1(pNode + pNode->m_RightOffset);
				C_SoftBodyAABBNode* node2(pNode + 1);
				S32 b1(node1->m_BoundingBox.Intersects(pObject->m_TransformedBox));
				S32 b2(node2->m_BoundingBox.Intersects(pObject->m_TransformedBox));
				stack.Push(node2, size);
				size += b2;
				stack.Push(node1, size);
				size += b1;
				pStackTop = SharedPhysics::Fast_Select<C_SoftBodyAABBNode>(b1!=0, node1, 
					SharedPhysics::Fast_Select<C_SoftBodyAABBNode>(b2!=0, node2, pStackTop));
			}
			else
			{
				//We have a hit! Loop through all the triangles and call collide on them. If we've hit it,
				//resolve the contact using a projection-based approach!
				bResult = ColFunc(pObject->m_pShape, pThisNode, pObject->m_pNodePtr, *this, pNode->m_StartVertexIndex, pNode->m_NumVertices, contacts) || bResult;
			}
		}
		return bResult;
	}

	void C_SoftBodyTree::InitialiseEdgeTriangleIndices()
	{
		if(m_pEdgeIndices)
			delete [] m_pEdgeIndices;
		m_pEdgeIndices = new C_EdgeTriangleIndices[m_numEdges];

		//Loop through all the triangles and attach them to the edge...

		for(U32 a = 0; a < m_numTriangles; ++a)
		{
			Collision::C_SoftBodyTriangle& tri = m_pTriangles[a];
			C_EdgeTriangleIndices& indA = m_pEdgeIndices[tri.m_EdgeIndexA];
			C_EdgeTriangleIndices& indB = m_pEdgeIndices[tri.m_EdgeIndexB];
			C_EdgeTriangleIndices& indC = m_pEdgeIndices[tri.m_EdgeIndexC];

			if(indA.m_TriangleIndexA == -1)
				indA.m_TriangleIndexA = a;
			else
				indA.m_TriangleIndexB = a;

			if(indB.m_TriangleIndexA == -1)
				indB.m_TriangleIndexA = a;
			else
				indB.m_TriangleIndexB = a;

			if(indC.m_TriangleIndexA == -1)
				indC.m_TriangleIndexA = a;
			else
				indC.m_TriangleIndexB = a;
		}

	}
};
