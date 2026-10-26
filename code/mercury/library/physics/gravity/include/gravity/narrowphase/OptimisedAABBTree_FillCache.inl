#ifndef OPTIMISED_AABB_TREE_FILL_CACHE_INL
#define OPTIMISED_AABB_TREE_FILL_CACHE_INL

#include "gravity/narrowphase/OptimisedAABBTree.h"
#include "gravity/narrowphase/LineCollisionCache.h"
#include "gravity/narrowphase/CollisionFilter.inl"


namespace Collision
{

inline bool IntersectAABBTriangle(Collision::C_CRAxisBox& box, const Maths::HVec3 PCREF a, const Maths::HVec3 PCREF b, const Maths::HVec3 PCREF c)
{
	using namespace Maths;
	const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
	const HVec3 ca(box.centre);
	const HVec3 e(box.extents);
	
	//Translate triangle as conceptually moving AABB to origin
	const HVec3 v0(a-ca); 
	const HVec3 v1(b-ca);
	const HVec3 v2(c-ca);

	//Compute edge vectors for triangle
	const HVec3 f0(v1-v0);
	const HVec3 f1(v2-v1);
	const HVec3 f2(v0-v2);
	

	const HVec3 absF0(Maths::Abs(f0));
	const HVec3 crossP00(v0 ^ f0);
	const HVec3 crossP01(v2 ^ f0);

	const HVec3 crossRA0(e.Y()*absF0.Z() + e.Z()*absF0.Y(),
			e.X()*absF0.Z()+e.Z()*absF0.X(),
			e.X()*absF0.Y()+e.Y()*absF0.X());

	const Maths::HVec3 vMin0(Maths::Min(crossP00, crossP01));
	const Maths::HVec3 vMax0(Maths::Max(crossP00, crossP01));

	if(Maths::AnyTrue3((vMax0 < (-crossRA0)) | (vMin0 > crossRA0)))
		return false;

	const HVec3 absF1(Maths::Abs(f1));
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01
	const HVec3 crossP10(v0^f1);
	const HVec3 crossP11(v1^f1);

	const HVec3 crossRA1(e.Y()*absF1.Z() + e.Z()*absF1.Y(),
			e.X()*absF1.Z()+e.Z()*absF1.X(),
			e.X()*absF1.Y()+e.Y()*absF1.X());


	const HVec3 vMax1(Max(crossP10, crossP11));
	const HVec3 vMin1(Min(crossP10, crossP11));

	if(Maths::AnyTrue3((vMax1 < (-crossRA1)) | (vMin1 > crossRA1)))
		return false;


	const HVec3 absF2(Maths::Abs(f2));
	//Test axis a01=u0 X f1, all triangle vertexs project onto axis a01
	const HVec3 crossP20(v0^f2);
	const HVec3 crossP21(v1^f2);


	const HVec3 vMax2(Max(crossP20, crossP21));
	const HVec3 vMin2(Min(crossP20, crossP21));

	const HVec3 crossRA2(e.Y()*absF2.Z() + e.Z()*absF2.Y(),
			e.X()*absF2.Z()+e.Z()*absF2.X(),
			e.X()*absF2.Y()+e.Y()*absF2.X());

	if(Maths::AnyTrue3((vMax2 < (-crossRA2)) | (vMin2 > crossRA2)))
		return false;
	
	//Test plane separation test - probably not that useful!
	const HVec3 n(f0^f1);
	const HF32 d(n|v0);
	const HVec3 absn(Abs(n));
	//const HF32 r=ex*Abs(n.X())+ey*Abs(n.Y())+ez*Abs(n.Z());
	const HF32 r(e|absn);
	//const HF32 s=(n|aabb.c)-d;
	return Abs(d).LE(r);	
}

void OptimisedAABBTree::FillLineCollisionCache(Collision::C_CRAxisBox& box, C_LineCollisionCache& cache, 
					SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
					Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const Maths::C_Normal PCREF negativeNormal,
					const Maths::HF32 PCREF maxCosAngle, SharedPhysics::C_SmallSPUCache& spuCache)
{

	stack.Push( m_pNodes, 0 );
	OptimisedAABBNode* pStackTop(m_pNodes);
	S32 size = 1;
	//C_AxisBoxMinMax box1, box2;
	C_AxisBoxMinMax nodeBox;
	MathsStorage::HenDN3 dNorm;
	//Collision::C_CollisionFilter* pFilter = (Collision::C_CollisionFilter*)Spu::FetchAlloc((&shape->m_pFilter), sizeof(C_CollisionFilter));
	//Maths::HMat43 rot;
	//rot.SetIdentity();
	while(size--)
	{
		Spu::MemBaseMarker marker;
		OptimisedAABBNode* node = pStackTop;
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size - 1, 0) );
		//OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));

		fetchedNode->m_BoundingBox.LoadBox(nodeBox, m_Low, m_High);
		if(nodeBox.Intersects(box))
		{
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);
				//OptimisedAABBNode* _node1 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(OptimisedAABBNode));
				//OptimisedAABBNode* _node2 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(OptimisedAABBNode));

				/*OptimisedAABBNode* _node1 = (OptimisedAABBNode*)spuCache.FetchAllocData(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)spuCache.FetchAllocData(node2, sizeof(OptimisedAABBNode));

				_node1->m_BoundingBox.LoadBox(box1, m_Low, m_High);
				_node2->m_BoundingBox.LoadBox(box2, m_Low, m_High);
				stack.Push( node1, size );
				bool b1(box1.Intersects(box));
				size += S32(b1);
				stack.Push( node2, size );
				bool b2(box2.Intersects(box));
				size += S32(b2);
				pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b2, node2, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b1, node1, pStackTop));*/

				stack.Push( node1, size++ );
				stack.Push( node2, size++ );
				pStackTop = node2;

			}
			else
			{
				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				U32 offset = 0;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
				/*
				C_TriangleHeader* header = (C_TriangleHeader*)Spu::FetchAllocUnalignedAsync(m_Triangles.GetHeader(fetchedNode->m_TriangleStartIndex), sizeof(C_TriangleHeader), 2);*/
				U32 numTris(0);

				const Maths::HVec3 range(nodeBox.m_max - nodeBox.m_min);

				Spu::Wait(2);

				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a+=numTris)
				{
					/*Spu::MemBaseMarker innerMarker;*/
					//C_TriangleHeader* header = (C_TriangleHeader*)Spu::FetchAllocUnaligned(m_Triangles.GetHeader(offset), sizeof(C_TriangleHeader));
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					numTris = header->m_NumTriangles;
					offset += sizeof(C_TriangleHeader);

					if(pFilter->CanCollide(header->m_CollisionFlags))
					{
						//Collision::AABBTreeTriangle* tris = m_Triangles.GetTriangles(offset);

						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];

						for(U32 b = 0; b < numTris; ++b)
						{
							/*Spu::MemBaseMarker __marker;*/
							//Collision::AABBTreeTriangle* tri = (AABBTreeTriangle*)Spu::FetchAllocUnaligned(tris + b, sizeof(AABBTreeTriangle));
							Collision::AABBTreeTriangle* tri = &pTriangles[b];
							Collision::C_AxisBoxMinMax triBox;
							/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + tri->m_IndexA, sizeof(MathsStorage::Float3*));
							MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + tri->m_IndexB, sizeof(MathsStorage::Float3*));
							MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(m_pVertices + tri->m_IndexC, sizeof(MathsStorage::Float3*));

							Maths::HVec3 v0(Maths::Load<Maths::HVec3>(f0));
							Maths::HVec3 v1(Maths::Load<Maths::HVec3>(f1));
							Maths::HVec3 v2(Maths::Load<Maths::HVec3>(f2));*/

							const Maths::HVec3 localA(&pVertices[tri->m_IndexA]);
							const Maths::HVec3 localB(&pVertices[tri->m_IndexB]);
							const Maths::HVec3 localC(&pVertices[tri->m_IndexC]);
							Maths::HVec3 v0(localA*range + nodeBox.m_min);
							Maths::HVec3 v1(localB*range + nodeBox.m_min);
							Maths::HVec3 v2(localC*range + nodeBox.m_min);

							triBox.m_max = Maths::Max(v0, Maths::Max(v1, v2));
							triBox.m_min = Maths::Min(v0, Maths::Min(v1, v2));

							if(triBox.Intersects(box) &&
								IntersectAABBTriangle(box, v0, v1, v2))
							{
								dNorm = tri->m_SurfaceNormal;
								Maths::C_Normal normal = Maths::C_Normal(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
								if((normal | negativeNormal) > maxCosAngle)
									continue;
								if(cache.Full())
									return;
								cache.AddTriangle(v0, v1, v2, normal, pShape, pGraphNode, a + fetchedNode->m_TriangleStartIndex, header->m_CollisionFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, header->m_UserDataFlags);
								
							}

						}
					}
					offset += header->m_NumTriangles * sizeof(AABBTreeTriangle);					
				}					
			}
		}
	}
}

void C_InstancedAABBTree::FillLineCollisionCache(Collision::C_CRAxisBox& _box, C_LineCollisionCache& cache, 
					SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionFilter* pFilter,
					Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const Maths::C_Normal PCREF negativeNormal,
					const Maths::HF32 PCREF maxCosAngle, SharedPhysics::C_SmallSPUCache& spuCache)
{

	OptimisedAABBTree* pTree = (OptimisedAABBTree*)Spu::FetchAlloc(m_pTree, sizeof(OptimisedAABBTree));
	stack.Push( pTree->m_pNodes, 0 );
	OptimisedAABBNode* pStackTop(pTree->m_pNodes);
	S32 size = 1;
	//C_AxisBoxMinMax box1, box2;
	C_AxisBoxMinMax nodeBox;
	MathsStorage::HenDN3 dNorm;
	const Maths::HMat33 rotation = m_PosRot.Rotation();

	
	Collision::C_CRAxisBox box;
	_box.Transform(m_PosRot.InverseRotTrans(), box);
	//box.centre = rotation * _box.centre + m_PosRot.Translation();
	//box.extents = rotation * _box.extents;
	//Collision::C_CollisionFilter* pFilter = (Collision::C_CollisionFilter*)Spu::FetchAlloc((&shape->m_pFilter), sizeof(C_CollisionFilter));
	//Maths::HMat43 rot;
	//rot.SetIdentity();
	while(size--)
	{
		Spu::MemBaseMarker marker;
		OptimisedAABBNode* node = pStackTop;
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size - 1, 0) );
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		
		fetchedNode->m_BoundingBox.LoadBox(nodeBox, pTree->m_Low, pTree->m_High);
		if(nodeBox.Intersects(box))
		{
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);
				/*OptimisedAABBNode* _node1 = (OptimisedAABBNode*)spuCache.FetchAllocData(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)spuCache.FetchAllocData(node2, sizeof(OptimisedAABBNode));
				_node1->m_BoundingBox.LoadBox(box1, pTree->m_Low, pTree->m_High);
				_node2->m_BoundingBox.LoadBox(box2, pTree->m_Low, pTree->m_High);
				stack.Push( node1, size );
				bool b1(box1.Intersects(box));
				size += S32(b1);
				stack.Push( node2, size );
				bool b2(box2.Intersects(box));
				size += S32(b2);
				pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b2, node2, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b1, node1, pStackTop));*/
				stack.Push(node1, size++);
				stack.Push(node2, size++);
				pStackTop = node2;
			}
			else
			{
			
				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(pTree->m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(pTree->m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
				U32 offset = 0;
				
				U32 numTris(0);

				const Maths::HVec3 range(nodeBox.m_max - nodeBox.m_min);

				Spu::Wait(2);

				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a+=numTris)
				{
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					offset += sizeof(C_TriangleHeader);
					numTris = header->m_NumTriangles;
					if(pFilter->CanCollide(header->m_CollisionFlags))
					{
						//Collision::AABBTreeTriangle* tris = pTree->m_Triangles.GetTriangles(offset);
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];

						for(U32 b = 0; b < numTris; ++b)
						{
							Spu::MemBaseMarker __marker;
							//Collision::AABBTreeTriangle* tri = (AABBTreeTriangle*)Spu::FetchAllocUnaligned(tris + b, sizeof(AABBTreeTriangle));
							Collision::AABBTreeTriangle* tri = &pTriangles[b];
							Collision::C_AxisBoxMinMax triBox;
							/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(pTree->m_pVertices + tri->m_IndexA, sizeof(MathsStorage::Float3*));
							MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(pTree->m_pVertices + tri->m_IndexB, sizeof(MathsStorage::Float3*));
							MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(pTree->m_pVertices + tri->m_IndexC, sizeof(MathsStorage::Float3*));*/

							/*Maths::HVec3 v0(Maths::Load<Maths::HVec3>(f0));
							Maths::HVec3 v1(Maths::Load<Maths::HVec3>(f1));
							Maths::HVec3 v2(Maths::Load<Maths::HVec3>(f2));*/
							const Maths::HVec3 localA(&pVertices[tri->m_IndexA]);
							const Maths::HVec3 localB(&pVertices[tri->m_IndexB]);
							const Maths::HVec3 localC(&pVertices[tri->m_IndexC]);
							Maths::HVec3 v0(localA*range + nodeBox.m_min);
							Maths::HVec3 v1(localB*range + nodeBox.m_min);
							Maths::HVec3 v2(localC*range + nodeBox.m_min);

							triBox.m_max = Maths::Max(v0, Maths::Max(v1, v2));
							triBox.m_min = Maths::Min(v0, Maths::Min(v1, v2));

							if(triBox.Intersects(box) && 
								IntersectAABBTriangle(box, v0, v1, v2))
							{
								dNorm = tri->m_SurfaceNormal;
								Maths::C_Normal n = Maths::C_Normal(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm)* rotation, Maths::C_Normal::UNIT_LENGTH_VECTOR);								
								if((n | negativeNormal) > maxCosAngle)
									continue;

								if(cache.Full())
									return;
								cache.AddTriangle(v0 * m_PosRot, v1 * m_PosRot, v2 * m_PosRot, n, pShape, pGraphNode, a + fetchedNode->m_TriangleStartIndex, header->m_CollisionFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, header->m_UserDataFlags);
								
							}

						}
					}
					offset += header->m_NumTriangles * sizeof(AABBTreeTriangle);
				}					
			}
		}
	}

}
}

#endif
