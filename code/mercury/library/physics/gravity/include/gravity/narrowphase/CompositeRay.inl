#ifndef COMPOSITERAY_INL
#define COMPOSITERAY_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/RayTriangle.h"
#include "gravity/narrowphase/ConeRay.inl"


namespace Collision
{

	bool inline CollideCompositeRay(const C_Shape* const shape, C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	Spu::MemBaseMarker marker;
	C_CollisionFilter* pRayFilter = &ray.m_Filter;//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
	Collision::C_CompositeShape* composite=(C_CompositeShape*)shape;
	//bool finalCollide=false;
	bool collide=false;

	Collision::C_CompositeShapeIterator iter(composite);
	//for(U32 i=0; i<composite->m_NumShapes; ++i)
	while(iter.HasNextShape())
	{
		Spu::MemBaseMarker innerMarker;
		iter.NextShape();
/*
		Collision::C_ShapeWrapper* wrapper=(C_ShapeWrapper*)Spu::FetchAlloc(composite->m_pShapes + i, sizeof(C_ShapeWrapper));
		Collision::C_Shape* subShape=(C_Shape*)Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_Shape));*/
		Collision::C_Shape* subShape=iter.GetCurrentShape<C_Shape>();
		//C_CollisionFilter* pShapeFilter = (C_CollisionFilter*)Spu::FetchAlloc((&shape->m_pFilter), sizeof(C_CollisionFilter));
	
		if(pRayFilter->CanCollide(&subShape->m_Filter))
		{
			switch(subShape->GetShape())
			{
			case E_Box_Shape:
				collide= collide | CollideOBBRay(subShape, ray);
				break;
			case E_ConvexHull_Shape:
				if(ray.m_bFatTrace)
					collide=collide | CollideConvexHullFatRay(subShape, ray);
				else
					collide=collide | CollideConvexHullRay(subShape, ray);
				break;
			case E_Sphere_Shape:
				collide=collide | CollideSphereRay(subShape, ray);
				break;
			case E_Capsule_Shape:
				collide=collide | CollideCapsuleRay(subShape, ray);
				break;
			case E_Cylinder_Shape:
				collide=collide | CollideCylinderRay(subShape, ray);
				break;
			case E_Cone_Shape:
				collide = collide | CollideConeRay(subShape, ray);
				break;
			case E_Composite_Shape:
				collide = collide | CollideCompositeRay(subShape, ray, stack, spuCache);
				break;
			case E_InstancedTriangleMesh_Shape:
				{
					C_InstancedTriangleMeshShape* pShape = (C_InstancedTriangleMeshShape*)subShape;//Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
					bool bNewCollide(false);
					if(ray.m_bFatTrace)
					{
						bNewCollide = pShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, stack, spuCache);
					}
					else
					{
						bNewCollide = pShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(ray, stack, spuCache);
					}
					if(bNewCollide)
					{
						collide = true;
						ray.m_result.m_pShape = pShape->m_pPpuPtr;
					}
					break;
				}
			
			default:
				BIZ_ASSERT_MSG(0, "Undefined collision primitive for raytrace!");
				break;
			}
		}
	}
	return collide;
}


	bool inline CollideCompositeRay(const C_Shape* const shape, C_TransparentRay& ray, SharedPhysics::C_SafeStack<true>& stack, C_CollisionFilter* pRayFilter, SharedPhysics::C_SmallSPUCache& spuCache,
		SharedPhysics::C_GraphNode* pGraphNode)
{
	Spu::MemBaseMarker marker;
	//C_CollisionFilter* pRayFilter = &ray.m_Filter;//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
	Collision::C_CompositeShape* composite=(C_CompositeShape*)shape;
	//bool finalCollide=false;
	bool collide=false;

	Collision::C_CompositeShapeIterator iter(composite);
	//for(U32 i=0; i<composite->m_NumShapes; ++i)
	while(iter.HasNextShape())
	{
		Spu::MemBaseMarker innerMarker;
		iter.NextShape();
		//Collision::C_ShapeWrapper* wrapper=(C_ShapeWrapper*)Spu::FetchAlloc(composite->m_pShapes + i, sizeof(C_ShapeWrapper));
//		Collision::C_Shape* subShape=(C_Shape*)Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_Shape));

		Collision::C_Shape* subShape= iter.GetCurrentShape<C_Shape>();
		//C_CollisionFilter* pShapeFilter = (C_CollisionFilter*)Spu::FetchAlloc((&shape->m_pFilter), sizeof(C_CollisionFilter));
	
		if(pRayFilter->CanCollide(&subShape->m_Filter))
		{
			switch(subShape->GetShape())
			{
			case E_Box_Shape:
				collide= collide | CollideOBBRay(subShape, ray, pGraphNode);
				break;
			case E_ConvexHull_Shape:
				if(ray.m_bFatTrace)
					collide = collide | CollideConvexHullFatRay(subShape, ray, pGraphNode);
				else
					collide=collide | CollideConvexHullRay(subShape, ray, pGraphNode);
				break;
			case E_Sphere_Shape:
				collide=collide | CollideSphereRay(subShape, ray, pGraphNode);
				break;
			case E_Capsule_Shape:
				collide=collide | CollideCapsuleRay(subShape, ray, pGraphNode);
				break;
			case E_Cylinder_Shape:
				collide=collide | CollideCylinderRay(subShape, ray, pGraphNode);
				break;
			case E_Composite_Shape:
				collide = collide | CollideCompositeRay(subShape, ray, stack, pRayFilter, spuCache, pGraphNode);
				break;
			case E_InstancedTriangleMesh_Shape:
				{
					C_InstancedTriangleMeshShape* pShape = (C_InstancedTriangleMeshShape*)subShape;//Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_InstancedTriangleMeshShape));
					bool bNewCollide(false);
				
					if(ray.m_bFatTrace)
					{
						bNewCollide = pShape->m_AABBTree.RayTracing<CollideFatRayTriangleMesh>(ray, pRayFilter, pShape->m_pPpuPtr, pGraphNode,  stack, spuCache);
					}
					else
					{
						bNewCollide = pShape->m_AABBTree.RayTracing<CollideRayTriangleMesh>(ray, pRayFilter, pShape->m_pPpuPtr, pGraphNode, stack, spuCache);
					}
					if(bNewCollide)
					{
						collide = true;
					}
					break;
				}
			}
		}
	}
	return collide;
}
};

#endif
