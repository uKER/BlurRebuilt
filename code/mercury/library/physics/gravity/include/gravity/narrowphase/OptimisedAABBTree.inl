#ifndef OPTIMISEDAABBTREE_INL
#define OPTIMISEDAABBTREE_INL

#include "gravity/narrowphase/OptimisedAABBTree.h"

namespace Collision
{


#if (!defined PLATFORM_SPU) || defined (SPU_MESH_COLLISION)

#ifndef PLATFORM_SPU
	template <int MaxPatches, int MaxContacts, bool (*TriCollisions)(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const tris, const U32 numTris, 
		const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)>
bool OptimisedAABBTree::Collide(Collision::C_CollisionObject * obj, 
								Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, 
								SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
#else
template <int MaxPatches, int MaxContacts>
bool OptimisedAABBTree::Collide(Collision::C_CollisionObject * obj, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_SafeStack<true>& stack,
								bool (*TriCollisions)(C_Shape* pShape, 
								const MathsStorage::Fixed3_S16<14>* const  verts, 
								const MathsStorage::HenDN3 * const normals, 
								const Collision::AABBTreeTriangle* const tris, 
								const U32 numTris, 
								const U32 triangleStartIndex, 
								Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, 
								const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags,
								const Maths::HVec3& low, const Maths::HVec3& high), SharedPhysics::C_SmallSPUCache& spuCache)
#endif
{
	//const Collision::C_CRAxisBox box=obj->m_TransformedBox;
	stack.Push( m_pNodes, 0 );
	OptimisedAABBNode* pStackTop(m_pNodes);
	S32 size = 1;
	bool collide = false;
	Collision::C_Shape* shape = obj->m_pSpuShape;
	const U32 physicsMaterialIndex(shape->PhysicsMaterialIndex());
	const U32 effectMaterialIndex(shape->EffectMaterialIndex());
	res.m_NumContactPatches = 0;
	//C_AxisBoxMinMax box1, box2;
#ifndef PLATFORM_PC
	BIZ_ASSERT((((U32)m_pNodes) & (SharedPhysics::SmallCachePageSize - 1)) == 0);
	BIZ_ASSERT((((U32)m_pNodes) & (sizeof(OptimisedAABBNode) - 1)) == 0);
#endif
	Collision::C_AxisBoxMinMax nodeBox;

	//Collision::C_CollisionFilter* pFilter = (Collision::C_CollisionFilter*)Spu::FetchAlloc((&shape->m_pFilter), sizeof(C_CollisionFilter));
	//Maths::HMat43 rot;
	//rot.SetIdentity();
	while(size--)
	{
		Spu::MemBaseMarker marker;
		OptimisedAABBNode* node = pStackTop;
#ifndef PLATFORM_PC
		BIZ_ASSERT((((U32)node) & (sizeof(OptimisedAABBNode) - 1)) == 0);
#endif
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size - 1, 0) );
		//OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		
		fetchedNode->m_BoundingBox.LoadBox(nodeBox, m_Low, m_High);
		if(nodeBox.Intersects(obj->m_TransformedBox))
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
				bool b1(box1.Intersects(obj->m_TransformedBox));
				size += S32(b1);
				stack.Push( node2, size );
				bool b2(box2.Intersects(obj->m_TransformedBox));
				size += S32(b2);
				pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b2, node2, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b1, node1, pStackTop));*/
				stack.Push(node1, size++);
				stack.Push(node2, size++);
				pStackTop = node2;
			}
			else
			{
				//Loop through triangles/triangle headers...
				//node->m_BoundingBox.LoadBox(box1, m_Low, m_High);
				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(box1.m_min, box1.m_max, rot, Maths::RGBA(255, 0, 0));
				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				U32 offset = 0;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
				/*
				C_TriangleHeader* header = (C_TriangleHeader*)Spu::FetchAllocUnalignedAsync(m_Triangles.GetHeader(fetchedNode->m_TriangleStartIndex), sizeof(C_TriangleHeader), 2);*/
				U32 numTris(0);
				//fetchedNode->m_BoundingBox.LoadBox(b, m_Low, m_High);
				Spu::Wait(2);
				
				
				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a+=numTris)
				{
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					numTris = header->m_NumTriangles;
					offset += sizeof(C_TriangleHeader);
					if(shape->m_Filter.CanCollide(header->m_CollisionFlags))
					{
						U32 physicsMaterial = physicsMaterialIndex | (header->m_PhysicsMaterialIndex << 16);
						U32 effectMaterial = effectMaterialIndex | (header->m_EffectMaterialIndex << 16);
						/*collide |= TriCollisions( obj->m_pShape, m_pVertices, m_pNormals, 
							m_Triangles.GetTriangles(offset), header->m_NumTriangles, offset, res, 
							physicsMaterial, effectMaterial, header->m_CollisionFlags);*/

						BIZ_ASSERT(offset < m_NumTriangles);
						/*AABBTreeTriangle* pTempTriangles = m_Triangles.GetTriangles(offset);
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)Spu::FetchAllocUnaligned(pTempTriangles, sizeof(AABBTreeTriangle)*numTris);*/
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];
					/*	collide |= TriCollisions( obj->m_pShape, pVertices, m_pNormals, 
							pTriangles, numTris, offset, res, physicsMaterial, effectMaterial, 
							header->m_CollisionFlags, b.m_min, b.m_max, fetchedNode->m_VerticeStartIndex);*/

						collide |= TriCollisions( obj->m_pSpuShape, pVertices, m_pNormals, 
							pTriangles, numTris, offset, res, physicsMaterial, effectMaterial, 
							header->m_CollisionFlags, nodeBox.m_min, nodeBox.m_max);
					}
					offset += header->m_NumTriangles * sizeof(AABBTreeTriangle);
				}					
			}
		}
	}
	
	return collide;
}

#endif


#if (!defined PLATFORM_SPU) || defined SPU_RAYTRACE || defined SPU_TRANSPARENT_RAYTRACE
//#pragma optimize("", off)
template<bool (*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const __restrict normals, const Collision::AABBTreeTriangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
							   const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterail, Collision::C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high)>
							   bool OptimisedAABBTree::RayTracing(Collision::C_Ray &ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	//PERF_EVENT("Gravity.MeshRaytrace");
	using namespace Maths;

	//Debug::Logger& g_Log = Debug::GetLogger("gravity");

	const HF32 half(Maths::Constants<HF32>::Half());
	stack.Push(m_pNodes, 0);
	OptimisedAABBNode* pStackTop(m_pNodes);
	S32 size = 1;
	bool collide = false;
	//Maths::HMat33 tra(Maths::UNDEFINED);
	//tra.SetIdentity();
	HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);
	//U32 color = 0;
	//C_AxisBoxMinMax box1, box2;
	C_AxisBoxMinMax nodeBox;
	Spu::MemBaseMarker marker;
	C_CollisionFilter* rayFilter = &ray.m_Filter;//(Collision::C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(Collision::C_CollisionFilter));
	const Maths::HVec3 radius(ray.m_Radius);
	while(size--)
	{
		Spu::MemBaseMarker innerMarker;
		OptimisedAABBNode* node = pStackTop;
		//OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		fetchedNode->m_BoundingBox.LoadBox(nodeBox, m_Low, m_High);
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size-1, 0) );
		
		if(IntersectRayAABBMinMax(ray.m_start, ray.m_result.m_contact, nodeBox.m_min - radius, nodeBox.m_max + radius))
		{
			//color += 15;
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);
				stack.Push(node1, size);
				//stack.Push(node2, size++);
				size += 2;
				pStackTop = node2;
			}
			else
			{
				//Do collision detection
				//manager.GetCollider(meshObj, obj);
				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dBox(box1.m_min, box1.m_max, identity, Maths::RGBA(0, 0, 255,255));
				U32 numTris(0);
				const U32 numTriangles= fetchedNode->m_TriangleSizeIndices;

				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				U32 offset = 0;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
	
				/*Collision::C_AxisBoxMinMax b;
				fetchedNode->m_BoundingBox.LoadBox(b, m_Low, m_High);*/
				Spu::Wait(2);
			

				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a += numTris)
				{
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					numTris = header->m_NumTriangles;
					offset += sizeof(C_TriangleHeader);
					if(rayFilter->CanCollide(header->m_CollisionFlags))
					{
						//PERF_EVENT("Gravity.TriangleTrace");
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];
						collide |= TriCollisions(pVertices, m_pNormals, pTriangles, offset, header->m_NumTriangles, 
							header->m_CollisionFlags, header->m_UserDataFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, 
							ray, nodeBox.m_min, nodeBox.m_max);
					}
					offset += sizeof(AABBTreeTriangle) * header->m_NumTriangles;
				}
			}
		}
	}
	//LOGINFO(g_Log, "NumTrisTested = %i\n", numTrisTested);
	return collide;
}
//#pragma optimize("", on)

#endif


#if (!defined PLATFORM_SPU) || defined SPU_TRANSPARENT_RAYTRACE
//#pragma optimize("", off)

template<bool (*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const normals, const Collision::AABBTreeTriangle* const tris, const U32 triangleStartIndex, const U32 numTris, 
							   const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterail, Collision::C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
							   const Maths::HVec3& low, const Maths::HVec3& high)>
							   bool OptimisedAABBTree::RayTracing(Collision::C_TransparentRay &ray, Collision::C_CollisionFilter* pRayFilter, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	//PERF_EVENT("Gravity.MeshRaytrace");
	using namespace Maths;

	//Debug::Logger& g_Log = Debug::GetLogger("gravity");

	const HF32 half(Maths::Constants<HF32>::Half());
	stack.Push(m_pNodes, 0);
	OptimisedAABBNode* pStackTop(m_pNodes);
	S32 size = 1;
	bool collide = false;
	//Maths::HMat33 tra(Maths::UNDEFINED);
	//tra.SetIdentity();
	HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);
	//U32 color = 0;
	//C_AxisBoxMinMax box1, box2;
	C_AxisBoxMinMax nodeBox;

	Spu::MemBaseMarker marker;
	const Maths::HVec3 radius(ray.m_Radius);
	//C_CollisionFilter* rayFilter = &ray.m_Filter;//(Collision::C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(Collision::C_CollisionFilter));

	while(size--)
	{
		Spu::MemBaseMarker innerMarker;
		OptimisedAABBNode* node = pStackTop;
		//OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		//C_AxisBoxMinMax& box=node->m_BoundingBox;
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size-1, 0) );
		fetchedNode->m_BoundingBox.LoadBox(nodeBox, m_Low, m_High);
		
		if(IntersectRayAABBMinMax(ray.m_start, ray.m_end, nodeBox.m_min - radius, nodeBox.m_max + radius))
		{
			//color += 15;
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);
				//OptimisedAABBNode* _node1 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(OptimisedAABBNode));
				//OptimisedAABBNode* _node1 = (OptimisedAABBNode*)spuCache.FetchAllocData(node1, sizeof(OptimisedAABBNode));
				//
				//OptimisedAABBNode* _node2 = (OptimisedAABBNode*)spuCache.FetchAllocData(node2, sizeof(OptimisedAABBNode));
				////OptimisedAABBNode* _node2 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(OptimisedAABBNode));
				//_node1->m_BoundingBox.LoadBox(box1, m_Low, m_High, ray.m_Radius);
				//_node2->m_BoundingBox.LoadBox(box2, m_Low, m_High, ray.m_Radius);
				//S32 b1(IntersectRayAABBMinMax(ray.m_start, ray.m_end, box1.m_min, box1.m_max, tmin));
				//S32 b2(IntersectRayAABBMinMax(ray.m_start, ray.m_end, box2.m_min, box2.m_max, tmin2));
				//stack.Push(node2, size);
				//size += b2;
				//stack.Push(node1, size);
				//size += b1;
				//pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b1!=0, node1, 
				//	SharedPhysics::Fast_Select<OptimisedAABBNode>(b2!=0, node2, pStackTop));
				stack.Push(node1, size++);
				stack.Push(node2, size++);
				pStackTop = node2;
			}
			else
			{
				//Do collision detection
				//manager.GetCollider(meshObj, obj);
				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				U32 offset = 0;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
				/*
				C_TriangleHeader* header = (C_TriangleHeader*)Spu::FetchAllocUnalignedAsync(m_Triangles.GetHeader(fetchedNode->m_TriangleStartIndex), sizeof(C_TriangleHeader), 2);*/
				U32 numTris(0);

			/*	Collision::C_AxisBoxMinMax nodeBox;
				fetchedNode->m_BoundingBox.LoadBox(nodeBox, m_Low, m_High);*/

				Spu::Wait(2);
							
				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a += numTris)
				{
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					numTris = header->m_NumTriangles;
					offset += sizeof(C_TriangleHeader);

					if(pRayFilter->CanCollide(header->m_CollisionFlags))
					{
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];
						//AABBTreeTriangle* tris = (AABBTreeTriangle*)Spu::FetchAlloc(m_Triangles.GetTriangles(offset), sizeof(AABBTreeTriangle) * header->m_NumTriangles);
						/*collide |= TriCollisions(pVertices, m_pNormals, pTriangles, offset, header->m_NumTriangles, header->m_CollisionFlags, 
							header->m_UserDataFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, 
							ray, pShape, pGraphNode, b.m_min, b.m_max, fetchedNode->m_VerticeStartIndex);	*/

						collide |= TriCollisions(pVertices, m_pNormals, pTriangles, offset, header->m_NumTriangles, header->m_CollisionFlags, 
							header->m_UserDataFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, 
							ray, pShape, pGraphNode, nodeBox.m_min, nodeBox.m_max);	
					}
					offset += sizeof(AABBTreeTriangle) * header->m_NumTriangles;
				}
			}
		}
	}
	//LOGINFO(g_Log, "NumTrisTested = %i\n", numTrisTested);
	return collide;
}
//#pragma optimize("", on)

#endif

#if (!defined PLATFORM_SPU) || defined (SPU_DYNAMIC_COLLISION)

#ifndef PLATFORM_SPU
template <int MaxPatches, int MaxContacts, typename CollisionProxy, bool (*TriCollisions)(CollisionProxy& proxy, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const tris, const U32 numTris, 
																						  const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags, const Maths::HVec3& low, 
																						  const Maths::HVec3& high)>
	bool C_InstancedAABBTree::Collide(C_CollisionObject *obj, CollisionProxy& proxy, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_SafeStack<true>& stack)
#else
	template <int MaxPatches, int MaxContacts, typename CollisionProxy>
	bool C_InstancedAABBTree::Collide(C_CollisionObject * obj, CollisionProxy& proxy, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, SharedPhysics::C_SafeStack<true>& stack,
		bool (*TriCollisions)(CollisionProxy& proxy, const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const normals, const Collision::AABBTreeTriangle* const  tris, const U32 numTris, const U32 triangleStartIndex, Collision::C_CollisionResult<MaxPatches, MaxContacts>& res, const U32 physicsMaterial, const U32 effectMaterial,
		const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high))

#endif
{
	
	Spu::MemBaseMarker marker;
	//const Collision::C_CRAxisBox box=obj->m_TransformedBox;
	if(!m_pTree)
		return false;
	OptimisedAABBTree* pTree = (OptimisedAABBTree*)Spu::FetchAlloc(m_pTree, sizeof(OptimisedAABBTree));
	//assert(pTree->m_NumNodes > 0 && "Error - a mesh with no polygons has been instanced!");
	if(pTree->m_NumNodes == 0)
		return false;
	stack.Push( pTree->m_pNodes, 0 );
	OptimisedAABBNode* pStackTop(pTree->m_pNodes);
	S32 size = 1;
	bool collide = false;
	Collision::C_Shape* pObjShape = (Collision::C_Shape*)Spu::FetchAlloc(obj->m_pShape, sizeof(Collision::C_Shape));
	const U32 physicsMaterialIndex(pObjShape->PhysicsMaterialIndex());
	const U32 effectMaterialIndex(pObjShape->EffectMaterialIndex());
	//res.m_NumContactPatches = 0;
	//C_AxisBoxMinMax box1, box2;
	C_AxisBoxMinMax nodeBox;
	const Maths::HVec3 low(pTree->m_Low);
	const Maths::HVec3 high(pTree->m_High);


	C_CRAxisBox box;
	obj->m_TransformedBox.Transform(m_PosRot.InverseRotTrans(), box);
	while(size--)
	{
		Spu::MemBaseMarker innerMarker;
		OptimisedAABBNode* node = pStackTop;
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size - 1, 0) );
		fetchedNode->m_BoundingBox.LoadBox(nodeBox, pTree->m_Low, pTree->m_High);
		
		if(nodeBox.Intersects(box))
		{
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);
				/*OptimisedAABBNode* fetchedNode1 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* fetchedNode2 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(OptimisedAABBNode));
				fetchedNode1->m_BoundingBox.LoadBox(box1, low, high);
				fetchedNode2->m_BoundingBox.LoadBox(box2, low, high);
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
				//Loop through triangles/triangle headers...
				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(pTree->m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(pTree->m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
				U32 offset = 0;
				
				U32 numTris(0);

				/*Collision::C_AxisBoxMinMax nodeBox;
				fetchedNode->m_BoundingBox.LoadBox(nodeBox, pTree->m_Low, pTree->m_High);*/

				Spu::Wait(2);

				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a+=numTris)
				{
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					offset += sizeof(C_TriangleHeader);
					numTris = header->m_NumTriangles;
			
					if(pObjShape->CanCollide(header->m_CollisionFlags))
					{
						U16 physicsMaterial = physicsMaterialIndex | (header->m_PhysicsMaterialIndex << 16);
						U16 effectMaterial = effectMaterialIndex | (header->m_EffectMaterialIndex << 16);

						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];
						/*collide |= TriCollisions( proxy, pVertices, pTree->m_pNormals, pTriangles, 
							header->m_NumTriangles, offset, res, physicsMaterial, effectMaterial, 
							header->m_CollisionFlags, b.m_min, b.m_max, fetchedNode->m_VerticeStartIndex );*/

						collide |= TriCollisions( proxy, pVertices, pTree->m_pNormals, pTriangles, 
							header->m_NumTriangles, offset, res, physicsMaterial, effectMaterial, 
							header->m_CollisionFlags, nodeBox.m_min, nodeBox.m_max);
					}
					offset += header->m_NumTriangles * sizeof(AABBTreeTriangle);
				}					
			}
		}
	}
	
	return collide;
}

#endif

#if (!defined PLATFORM_SPU) || defined SPU_RAYTRACE || defined SPU_TRANSPARENT_RAYTRACE || defined SPU_FLUID_COLLISION

	template<bool (*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const normals, const Collision::AABBTreeTriangle* const tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, Collision::C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high)>
		bool C_InstancedAABBTree::RayTracing(Collision::C_Ray &ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	using namespace Maths;

	//DebugBreak();
	Spu::MemBaseMarker marker;
	Collision::OptimisedAABBTree* tree = (Collision::OptimisedAABBTree*)Spu::FetchAlloc(m_pTree, sizeof(OptimisedAABBTree));
	if(tree->m_NumNodes == 0)
		return false;

	const HVec3 trans(m_PosRot.Translation());
	const HMat33 rot(m_PosRot.Rotation());

	//Transform ray to local space!!!
	ray.m_start = rot * (ray.m_start - trans);
	ray.m_end = rot * (ray.m_end - trans);
	ray.m_result.m_contact = rot * (ray.m_result.m_contact - trans);
	ray.m_Direction = Maths::C_Normal(rot * ray.m_Direction, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	//ray.m_result.m_normal = Maths::C_Normal(rot * ray.m_result.m_normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);

	const HF32 half(Constants<HF32>::Half());
	
	C_CollisionFilter* rayFilter = &ray.m_Filter;//(Collision::C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(Collision::C_CollisionFilter));
	
	stack.Push(tree->m_pNodes, 0);
	OptimisedAABBNode* pStackTop(tree->m_pNodes);
	S32 size = 1;
	bool collide = false;
	//Maths::HMat33 tra(Maths::UNDEFINED);
	//tra.SetIdentity();
	HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);
	//U32 color = 0;
	//C_AxisBoxMinMax box1, box2;
	C_AxisBoxMinMax nodeBox;

	const HVec3 low(tree->m_Low);
	const HVec3 high(tree->m_High);
	const Maths::HVec3 radius(ray.m_Radius);

	//SharedPhysics::C_SmallSPUCache spuCache(2);

	while(size--)
	{
		Spu::MemBaseMarker innerMarker;
		OptimisedAABBNode* node = pStackTop;
		//OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		//C_AxisBoxMinMax& box=node->m_BoundingBox;
		fetchedNode->m_BoundingBox.LoadBox(nodeBox, low, high);
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size-1, 0) );
		
		if(IntersectRayAABBMinMax(ray.m_start, ray.m_result.m_contact, nodeBox.m_min - radius, nodeBox.m_max + radius))
		{
			//color += 15;
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);

				//OptimisedAABBNode* _node1 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(OptimisedAABBNode));
				//OptimisedAABBNode* _node2 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(OptimisedAABBNode));

				/*OptimisedAABBNode* _node1 = (OptimisedAABBNode*)spuCache.FetchAllocData(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)spuCache.FetchAllocData(node2, sizeof(OptimisedAABBNode));

				_node1->m_BoundingBox.LoadBox(box1, low, high, ray.m_Radius);
				_node2->m_BoundingBox.LoadBox(box2, low, high, ray.m_Radius);
				S32 b1(IntersectRayAABBMinMax(ray.m_start, ray.m_result.m_contact, box1.m_min, box1.m_max, tmin));
				S32 b2(IntersectRayAABBMinMax(ray.m_start, ray.m_result.m_contact, box2.m_min, box2.m_max, tmin2));
				stack.Push(node2, size);
				size += b2;
				stack.Push(node1, size);
				size += b1;
				pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b1!=0, node1, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b2!=0, node2, pStackTop));*/
				stack.Push(node1, size++);
				stack.Push(node2, size++);
				pStackTop = node2;
			}
			else
			{
				//Do collision detection
				//manager.GetCollider(meshObj, obj);
				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(tree->m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(tree->m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
				U32 offset = 0;
				
				U32 numTris(0);

				/*Collision::C_AxisBoxMinMax nodeBox;
				fetchedNode->m_BoundingBox.LoadBox(nodeBox, tree->m_Low, tree->m_High);*/

				Spu::Wait(2);
				
				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a += numTris)
				{
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					offset += sizeof(C_TriangleHeader);
					numTris = header->m_NumTriangles;
					if(rayFilter->CanCollide(header->m_CollisionFlags))
					{
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];
						//AABBTreeTriangle* tris = (AABBTreeTriangle*)Spu::FetchAlloc(m_Triangles.GetTriangles(offset), sizeof(AABBTreeTriangle) * header->m_NumTriangles);
						/*collide = TriCollisions(pVertices, tree->m_pNormals, pTriangles, offset, header->m_NumTriangles, 
							header->m_CollisionFlags, header->m_UserDataFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, 
							ray, b.m_min, b.m_max, fetchedNode->m_VerticeStartIndex) || collide;*/

						collide = TriCollisions(pVertices, tree->m_pNormals, pTriangles, offset, header->m_NumTriangles, 
							header->m_CollisionFlags, header->m_UserDataFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, 
							ray, nodeBox.m_min, nodeBox.m_max) | collide;
					}
					offset += sizeof(AABBTreeTriangle) * header->m_NumTriangles;
				}
			}
		}
	}

	ray.m_start = ray.m_start* rot + trans;
	ray.m_end = ray.m_end * rot + trans;
	ray.m_result.m_contact = ray.m_result.m_contact * rot + trans;
	ray.m_Direction = Maths::C_Normal(ray.m_Direction * rot, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	if(collide)
	{
		ray.m_result.m_normal = Maths::C_Normal(ray.m_result.m_normal * rot, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		ray.m_result.m_TriangleNormal = Maths::C_Normal(ray.m_result.m_TriangleNormal * rot, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	}
	return collide;
}

#endif

#if (!defined PLATFORM_SPU) || defined SPU_TRANSPARENT_RAYTRACE

template<bool (*TriCollisions)(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3* const normals, const Collision::AABBTreeTriangle* const tris, const U32 triangleStartIndex, const U32 numTris, 
							   const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, Collision::C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, const Maths::HVec3& low, const Maths::HVec3& high,
							   const Maths::HMat43& matrix)>
							   bool C_InstancedAABBTree::RayTracing(Collision::C_TransparentRay &ray, Collision::C_CollisionFilter* pRayFilter, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	using namespace Maths;

	//DebugBreak();
	Spu::MemBaseMarker marker;
	Collision::OptimisedAABBTree* tree = (Collision::OptimisedAABBTree*)Spu::FetchAlloc(m_pTree, sizeof(OptimisedAABBTree));
	if(tree->m_NumNodes == 0)
		return false;

	const HVec3 trans(m_PosRot.Translation());
	const HMat33 rot(m_PosRot.Rotation());

	//Transform ray to local space!!!
	ray.m_start = rot * (ray.m_start - trans);
	ray.m_end = rot * (ray.m_end - trans);
	BIZ_ASSERT(ray.m_result != NULL);
	ray.m_result[0].m_contact = rot * (ray.m_result[0].m_contact - trans);
	ray.m_Direction = Maths::C_Normal(rot * ray.m_Direction, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	//ray.m_result.m_normal = Maths::C_Normal(rot * ray.m_result.m_normal, Maths::C_Normal::UNIT_LENGTH_VECTOR);

	const HF32 half(Constants<HF32>::Half());

	//C_CollisionFilter* rayFilter = &ray.m_Filter;//(Collision::C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(Collision::C_CollisionFilter));
	
	stack.Push(tree->m_pNodes, 0);
	OptimisedAABBNode* pStackTop(tree->m_pNodes);
	S32 size = 1;
	bool collide = false;
	//Maths::HMat33 tra(Maths::UNDEFINED);
	//tra.SetIdentity();
	HF32 tmin(Maths::UNDEFINED), tmin2(Maths::UNDEFINED);
	//U32 color = 0;
	//C_AxisBoxMinMax box1, box2;
	C_AxisBoxMinMax nodeBox;

	const HVec3 low(tree->m_Low);
	const HVec3 high(tree->m_High);
	const Maths::HVec3 radius(ray.m_Radius);
	//SharedPhysics::C_SmallSPUCache spuCache(2);

	while(size--)
	{
		Spu::MemBaseMarker innerMarker;
		OptimisedAABBNode* node = pStackTop;
		//OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node, sizeof(OptimisedAABBNode));
		OptimisedAABBNode* fetchedNode = (OptimisedAABBNode*)spuCache.FetchAllocData(node, sizeof(OptimisedAABBNode));
		//C_AxisBoxMinMax& box=node->m_BoundingBox;
		pStackTop = stack.Pop<OptimisedAABBNode>( Maths::Max(size-1, 0) );
		fetchedNode->m_BoundingBox.LoadBox(nodeBox, low, high);
		
		if(IntersectRayAABBMinMax(ray.m_start, ray.m_end, nodeBox.m_min - radius, nodeBox.m_max + radius))
		{
			//color += 15;
			if(fetchedNode->m_TriangleSizeIndices == 0)
			{
				
				OptimisedAABBNode* node1(node + fetchedNode->m_RightChildOffset);
				OptimisedAABBNode* node2(node + 1);

				/*OptimisedAABBNode* _node1 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)Spu::FetchAllocUnaligned(node2, sizeof(OptimisedAABBNode));*/
				/*OptimisedAABBNode* _node1 = (OptimisedAABBNode*)spuCache.FetchAllocData(node1, sizeof(OptimisedAABBNode));
				OptimisedAABBNode* _node2 = (OptimisedAABBNode*)spuCache.FetchAllocData(node2, sizeof(OptimisedAABBNode));

				_node1->m_BoundingBox.LoadBox(box1, low, high, ray.m_Radius);
				_node2->m_BoundingBox.LoadBox(box2, low, high, ray.m_Radius);
				S32 b1(IntersectRayAABBMinMax(ray.m_start, ray.m_end, box1.m_min, box1.m_max, tmin));
				S32 b2(IntersectRayAABBMinMax(ray.m_start, ray.m_end, box2.m_min, box2.m_max, tmin2));
				stack.Push(node2, size);
				size += b2;
				stack.Push(node1, size);
				size += b1;
				pStackTop = SharedPhysics::Fast_Select<OptimisedAABBNode>(b1!=0, node1, 
					SharedPhysics::Fast_Select<OptimisedAABBNode>(b2!=0, node2, pStackTop));*/
				stack.Push(node1, size++);
				stack.Push(node2, size++);
				pStackTop = node2;
			}
			else
			{
				//Do collision detection
				//manager.GetCollider(meshObj, obj);
				Spu::MemBaseMarker innerMarker;
				U32 startOffset = fetchedNode->m_TriangleStartIndex;
				MathsStorage::Fixed3_S16<14>* pVertices = (MathsStorage::Fixed3_S16<14>*)Spu::FetchAllocUnalignedAsync(tree->m_pVertices + fetchedNode->m_VerticeStartIndex, sizeof(MathsStorage::Fixed3_S16<14>)*fetchedNode->m_NumVertices, 2);
				U8* pOffsetData = (U8*)Spu::FetchAllocUnalignedAsync(tree->m_Triangles.GetHeader(startOffset), sizeof(U8) * fetchedNode->m_TriangleStreamLength, 2); 
				U32 offset = 0;
				
				U32 numTris(0);

				
				/*Collision::C_AxisBoxMinMax nodeBox;
				fetchedNode->m_BoundingBox.LoadBox(nodeBox, tree->m_Low, tree->m_High);*/

				Spu::Wait(2);
				
				for(U32 a = 0; a < fetchedNode->m_TriangleSizeIndices; a += numTris)
				{
					C_TriangleHeader* header = (C_TriangleHeader*)&pOffsetData[offset];
					offset += sizeof(C_TriangleHeader);
					numTris = header->m_NumTriangles;
					if(pRayFilter->CanCollide(header->m_CollisionFlags))
					{
						AABBTreeTriangle* pTriangles = (AABBTreeTriangle*)&pOffsetData[offset];//AABBTreeTriangle* tris = (AABBTreeTriangle*)Spu::FetchAlloc(m_Triangles.GetTriangles(offset), sizeof(AABBTreeTriangle) * header->m_NumTriangles);
						/*collide = TriCollisions(pVertices, tree->m_pNormals, pTriangles, offset, header->m_NumTriangles, 
							header->m_CollisionFlags, header->m_UserDataFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, 
							ray, pShape, pGraphNode, b.m_min, b.m_max, fetchedNode->m_VerticeStartIndex) || collide;*/
						collide = TriCollisions(pVertices, tree->m_pNormals, pTriangles, offset, header->m_NumTriangles, 
							header->m_CollisionFlags, header->m_UserDataFlags, header->m_PhysicsMaterialIndex, header->m_EffectMaterialIndex, 
							ray, pShape, pGraphNode, nodeBox.m_min, nodeBox.m_max, m_PosRot) | collide;
					}
					offset += sizeof(AABBTreeTriangle) * header->m_NumTriangles;
				}
			}
		}
	}


	ray.m_start = ray.m_start* rot + trans;
	ray.m_end = ray.m_end * rot + trans;

	/*Collision::C_RayTracingResult result;
	result.m_contact = ray.m_result[0].m_contact * rot + trans;
	ray.m_Direction = Maths::C_Normal(ray.m_Direction * rot, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	if(collide)
	{
		result.m_normal = Maths::C_Normal(result.m_normal * rot, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		result.m_TriangleNormal = Maths::C_Normal(result.m_TriangleNormal * rot, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		ray.StoreResult(result);
	}*/
	return collide;
}

#endif


};

#endif

