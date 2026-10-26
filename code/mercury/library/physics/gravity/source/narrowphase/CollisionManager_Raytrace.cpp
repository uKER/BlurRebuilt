#include "StdAfx.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/CapsuleRay.h"
#include "gravity/narrowphase/SphereRay.inl"
#include "gravity/narrowphase/CylinderRay.h"
#include "gravity/narrowphase/ConeRay.inl"
#include "gravity/narrowphase/ConvexHullRay.h"
#include "gravity/narrowphase/obbray.h"

#include "gravity/narrowphase/CompositeRay.h"
//#include "gravity/narrowphase/CompositeRay.inl"
#include "gravity/narrowphase/OBBRay.h"
#include "gravity/narrowphase/RayTriangle.inl"
#include "gravity/narrowphase/OptimisedAABBTree.inl"
#include "gravity/narrowphase/CharacterControllerPrimitive.h"
#include "gravity/narrowphase/primitive.inl"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/AxisBox.inl"
#include "gravity/narrowphase/GJKRayCastCollision.h"
//#include "gravity/narrowphase/ConeRay.h"

namespace Collision
{


bool C_CollisionManager::FatRayTrace( const Collision::C_CollisionObject* __restrict const pObj,Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	Spu::MemBaseMarker marker;
	C_Shape* pShape = pObj->m_pShape;
	U32 shapeType = Spu::Get((U32*)&pObj->m_pShape->m_ShapeType);
	if(shapeType == E_Continuous_Shape)
	{
		//pShape = (C_Shape*)Spu::FetchAlloc(((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape, sizeof(Collision::C_Shape));
		pShape = (C_Shape*)Spu::Get((INTPTR*)(&((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape));	// [Blur2PC][x64] pointer-wide
		shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
	}
	C_CollisionFilter* pRayFilter = &ray.m_Filter;//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
	//Spu::Wait(1 << 1);
	C_CollisionFilter* pShapeFilter = (C_CollisionFilter*)Spu::FetchAlloc((&pShape->m_Filter), sizeof(C_CollisionFilter));
	
	if(pRayFilter->CanCollide(pShapeFilter))//pShapeFilter
	{
		//E_ShapeType objShape=pShape->GetShape();
		bool bResult = false;

		switch(shapeType)
		{
		case E_Box_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_BoxShape));
				bResult = GJKRayCastBox(pDerivedShape, ray);
				//bResult = CollideOBBFatRay(pDerivedShape, ray);
			}
			break;
		case E_ConvexHull_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_ConvexHullShape));
				//bResult = CollideConvexHullFatRay(pDerivedShape, ray);
				bResult = GJKRayCastConvex(pDerivedShape, ray);
			}
			break;
		case E_Sphere_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_SphereShape));
			//	bResult = CollideSphereFatRay(pDerivedShape, ray);
				bResult = GJKRayCastSphere(pDerivedShape, ray);
			}
			break;
		case E_Capsule_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_CapsuleShape));
			//	bResult = CollideCapsuleFatRay(pDerivedShape, ray);
				bResult = GJKRayCastCapsule(pDerivedShape, ray);
			}
			break;
		case E_Cylinder_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_CylinderShape));
				//bResult = CollideCylinderFatRay(pDerivedShape, ray);
				bResult = GJKRayCastCylinder(pDerivedShape, ray);
			}
			break;
		case E_Composite_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_CompositeShape));
			//	bResult = CollideCompositeFatRay(pDerivedShape, ray, stack, spuCache);
				bResult = GJKRayCastComposite(pDerivedShape, ray, stack, spuCache);
			}
			break;
		case E_InstancedTriangleMesh_Shape:
			{
				C_InstancedTriangleMeshShape* pTriShape = (C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
				if(pTriShape->m_AABBTree.RayTracing<Collision::CollideFatRayTriangleMesh>(ray, stack, spuCache))
				{
					bResult = true;
					ray.m_result.m_pShape = pShape;
				}
				
				break;
			}
		case E_Cloth_Shape:
			{
				C_ClothShape* pClothShape = (C_ClothShape*)Spu::FetchAlloc(pShape, sizeof(C_ClothShape));
			//	bResult = pClothShape->m_Cloth.Raytrace(ray, pObj, stack) || bResult;
				break;
			}
		case E_Cone_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_ConeShape));
				//bResult = CollideConeFatRay(pDerivedShape, ray);
				bResult = GJKRayCastCone(pDerivedShape, ray);
			}
			break;
		case E_Rope_Shape:
			{
				C_RopeShape* pRopeShape = (C_RopeShape*)Spu::FetchAlloc(pShape, sizeof(C_RopeShape));
				//bResult = pRopeShape->m_Rope.Raytrace(ray, pObj, stack) || bResult;
				break;
			}
		case E_FluidMesh_Shape:
			{
				C_FluidMeshShape* pFluidShape = (C_FluidMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_FluidMeshShape));
				if(pFluidShape->m_AABBTree.RayTracing<Collision::CollideFatRayTriangleMesh>(ray, stack, spuCache))
				{
					bResult = true;
					ray.m_result.m_pShape = pShape;
					ray.m_result.m_UserData = pFluidShape->m_UserData;
				}
				break;
			}
		default:
			break;
		}
		if(bResult)
		{
			ray.m_result.m_pNode = pObj->m_pNodePtr;
		}
		return bResult;
	}
	return false;
}


bool C_CollisionManager::RayTrace( const Collision::C_CollisionObject* __restrict const pObj,Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	Spu::MemBaseMarker marker;
	C_Shape* pShape = pObj->m_pShape;
	U32 shapeType = Spu::Get((U32*)&pObj->m_pShape->m_ShapeType);
	if(shapeType == E_Continuous_Shape)
	{
		//pShape = (C_Shape*)Spu::FetchAlloc(((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape, sizeof(Collision::C_Shape));
		pShape = (C_Shape*)Spu::Get((INTPTR*)(&((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape));	// [Blur2PC][x64] pointer-wide
		shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
	}
	C_CollisionFilter* pRayFilter = &ray.m_Filter;//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
	//Spu::Wait(1 << 1);
	C_CollisionFilter* pShapeFilter = (C_CollisionFilter*)Spu::FetchAlloc((&pShape->m_Filter), sizeof(C_CollisionFilter));
	
	if(pRayFilter->CanCollide(pShapeFilter))//pShapeFilter
	{
		//E_ShapeType objShape=pShape->GetShape();
		bool bResult = false;

		switch(shapeType)
		{
		case E_Box_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_BoxShape));
				bResult = CollideOBBRay(pDerivedShape, ray);
			}
			break;
		case E_ConvexHull_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_ConvexHullShape));
				bResult =  CollideConvexHullRay(pDerivedShape, ray);
			}
			break;
		case E_Sphere_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_SphereShape));
				bResult = CollideSphereRay(pDerivedShape, ray);
			}
			break;
		case E_Capsule_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_CapsuleShape));
				bResult = CollideCapsuleRay(pDerivedShape, ray);
			}
			break;
		case E_Cylinder_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_CylinderShape));
				bResult = CollideCylinderRay(pDerivedShape, ray);
			}
			break;
		case E_Composite_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_CompositeShape));
				bResult = CollideCompositeRay(pDerivedShape, ray, stack, spuCache);
			}
			break;
		case E_InstancedTriangleMesh_Shape:
			{
				C_InstancedTriangleMeshShape* pTriShape = (C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
				if(pTriShape->m_AABBTree.RayTracing<Collision::CollideRayTriangleMesh>(ray, stack, spuCache))
				{
					bResult = true;
					ray.m_result.m_pShape = pShape;
				}
				break;
			}
		case E_Cloth_Shape:
			{
				C_ClothShape* pClothShape = (C_ClothShape*)Spu::FetchAlloc(pShape, sizeof(C_ClothShape));
				bResult = pClothShape->m_Cloth.Raytrace(ray, pObj, stack) || bResult;
				break;
			}
		case E_Cone_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(C_ConeShape));
				bResult = CollideConeRay(pDerivedShape, ray);
			}
			break;
		case E_Rope_Shape:
			{
				C_RopeShape* pRopeShape = (C_RopeShape*)Spu::FetchAlloc(pShape, sizeof(C_RopeShape));
				bResult = pRopeShape->m_Rope.Raytrace(ray, pObj, stack) || bResult;
				break;
			}
		case E_FluidMesh_Shape:
			{
				C_FluidMeshShape* pFluidShape = (C_FluidMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_FluidMeshShape));
				if(pFluidShape->m_AABBTree.RayTracing<Collision::CollideRayTriangleMesh>(ray, stack, spuCache))
				{
					bResult = true;
					ray.m_result.m_pShape = pShape;
					ray.m_result.m_UserData = pFluidShape->m_UserData;
				}
				break;
			}
		default:
			break;
		}
		if(bResult)
		{
			ray.m_result.m_pNode = pObj->m_pNodePtr;
		}
		return bResult;
	}
	return false;
}

bool C_CollisionManager::RayTraceWater( const Collision::C_Shape* __restrict const pPpuShape,Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack , SharedPhysics::C_SmallSPUCache& spuCache)
{
	Spu::MemBaseMarker marker;
	C_Shape* pShape = (C_Shape*)Spu::FetchAlloc(pPpuShape, sizeof(C_Shape));
	C_CollisionFilter* pRayFilter = &ray.m_Filter;
	if(pRayFilter->CanCollide(&pShape->m_Filter))//pShapeFilter
	{
		E_ShapeType objShape=pShape->GetShape();
		bool bResult = false;

		switch(objShape)
		{
		//case E_Composite_Shape:
		//	{
		//		C_CompositeShape* pCompositeShape = (C_CompositeShape*)Spu::FetchAlloc(pShape->m_pPpuPtr, sizeof(C_CompositeShape));
		//		C_CompositeShapeIterator iter(pCompositeShape);
		//		//for(U32 a = 0; a < pCompositeShape->m_NumShapes; ++a)
		//		while(iter.HasNextShape())
		//		{
		//			Spu::MemBaseMarker _marker;
		//			iter.NextShape();
		//			Collision::C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
		//			//C_ShapeWrapper* pWrapper = (C_ShapeWrapper*)Spu::FetchAlloc(&pCompositeShape->m_pShapes[a], sizeof(C_ShapeWrapper));
		//			bResult = RayTraceWater(pShape, ray, stack, spuCache) || bResult;
		//		}
		//		
		//	}
		//	break;
		case E_FluidMesh_Shape:
			{
				C_FluidMeshShape* pFluidShape = (C_FluidMeshShape*)Spu::FetchAlloc(pPpuShape, sizeof(C_FluidMeshShape));
				if(pFluidShape->m_AABBTree.RayTracing<Collision::CollideRayTriangleMesh>(ray, stack, spuCache))
				{
					bResult = true;
					ray.m_result.m_pShape = pFluidShape->m_pPpuPtr;
				}
				break;
			}
		}
		return bResult;
	}
	return false;

}


bool C_CollisionManager::RayTraceWater( const Collision::C_CollisionObject* __restrict const pObj,Collision::C_Ray& ray, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache )
{
	Spu::MemBaseMarker marker;
	C_Shape* pShape = (C_Shape*)Spu::FetchAlloc(pObj->m_pShape, sizeof(C_Shape));
	C_CollisionFilter* pRayFilter = &ray.m_Filter;
	//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
	//Spu::Wait(1 << 1);
	//C_CollisionFilter* pShapeFilter = (C_CollisionFilter*)Spu::FetchAlloc((&pShape->m_pFilter), sizeof(C_CollisionFilter));
	
	if(pRayFilter->CanCollide(&pShape->m_Filter))//pShapeFilter
	{
		E_ShapeType objShape=pShape->GetShape();
		bool bResult = false;

		switch(objShape)
		{
		//case E_Composite_Shape:
		//	{
		//		C_CompositeShape* pCompositeShape = (C_CompositeShape*)Spu::FetchAlloc(pShape->m_pPpuPtr, sizeof(C_CompositeShape));
		//		Collision::C_CompositeShapeIterator iter(pCompositeShape);
		//		//for(U32 a = 0; a < pCompositeShape->m_NumShapes; ++a)
		//		while(iter.HasNextShape())
		//		{
		//			Spu::MemBaseMarker _marker;
		//			iter.NextShape();
		//			Collision::C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
		//			//C_ShapeWrapper* pWrapper = (C_ShapeWrapper*)Spu::FetchAlloc(&pCompositeShape->m_pShapes[a], sizeof(C_ShapeWrapper));

		//			bResult = RayTraceWater(pShape, ray, stack, spuCache) || bResult;
		//		}
		//		
		//	}
		//	break;
		case E_FluidMesh_Shape:
			{
				Spu::MemBaseMarker marker;
				C_FluidMeshShape* pFluidShape = (C_FluidMeshShape*)Spu::FetchAlloc(pShape->m_pPpuPtr, sizeof(C_FluidMeshShape));
				if(pFluidShape->m_AABBTree.RayTracing<Collision::CollideRayTriangleMesh>(ray, stack, spuCache))
				{
					bResult = true;
					ray.m_result.m_pShape = pShape->m_pPpuPtr;
				}
				marker.Restore();
				break;
			}
		}
		if(bResult)
		{
			ray.m_result.m_pNode = pObj->m_pNodePtr;
		}
		return bResult;
	}
	return false;
}


bool C_CollisionManager::RayTrace( const Collision::C_CollisionObject* __restrict const pObj,Collision::C_TransparentRay& ray, Collision::C_CollisionFilter* pRayFilter, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	Spu::MemBaseMarker marker;
	C_Shape* pShape = pObj->m_pShape;
	U32 shapeType = Spu::Get((U32*)&pObj->m_pShape->m_ShapeType);
	if(shapeType == E_Continuous_Shape)
	{
		//pShape = (C_Shape*)Spu::FetchAlloc(((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape, sizeof(Collision::C_Shape));
		pShape = (C_Shape*)Spu::Get((INTPTR*)(&((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape));	// [Blur2PC][x64] pointer-wide
		shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
	}
	//C_CollisionFilter* pRayFilter = &ray.m_Filter;//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
	//Spu::Wait(1 << 1);
	C_CollisionFilter* pShapeFilter = (C_CollisionFilter*)Spu::FetchAlloc((&pShape->m_Filter), sizeof(C_CollisionFilter));
	
	if(pRayFilter->CanCollide(pShapeFilter))//pShapeFilter
	{
		bool bResult = false;

		switch(shapeType)
		{
		case E_Box_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_BoxShape));
				bResult = CollideOBBRay(pDerivedShape, ray, pObj->m_pNodePtr) || bResult;
			}
			break;
		case E_Capsule_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CapsuleShape));
				bResult = CollideCapsuleRay(pDerivedShape, ray, pObj->m_pNodePtr) || bResult;
			}
			break;
		case E_Composite_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CompositeShape));
				bResult = CollideCompositeRay(pDerivedShape, ray, stack, pRayFilter, spuCache, pObj->m_pNodePtr) || bResult;
			}
			break;
		case E_ConvexHull_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConvexHullShape));
				bResult =  CollideConvexHullRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
				break;
			}
		case E_Cylinder_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CylinderShape));
				bResult = CollideCylinderRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
			}
			break;
		case E_Sphere_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_SphereShape));
				bResult = CollideSphereRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
			}
			break;
		case E_InstancedTriangleMesh_Shape:
			{
				C_InstancedTriangleMeshShape* pMeshShape = (C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
				if(pMeshShape->m_AABBTree.RayTracing<Collision::CollideRayTriangleMesh>(ray, pRayFilter, pShape, pObj->m_pNodePtr, stack, spuCache))
				{
					bResult = true;
				}
				break;
			}
		case E_FluidMesh_Shape:
			{
				C_FluidMeshShape* pFluidShape = (C_FluidMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_FluidMeshShape));
				if(pFluidShape->m_AABBTree.RayTracing<Collision::CollideRayTriangleMesh>(ray, pRayFilter, pShape, pObj->m_pNodePtr, stack, spuCache))
				{
					bResult = true;
				}
				break;
			}
		case E_Cone_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConeShape));
				bResult = CollideConeRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
				break;
			}
		}
		return bResult;
	}

	return false;
}


bool C_CollisionManager::FatRayTrace( const Collision::C_CollisionObject* __restrict const pObj,Collision::C_TransparentRay& ray, Collision::C_CollisionFilter* pRayFilter, SharedPhysics::C_SafeStack<true>& stack, SharedPhysics::C_SmallSPUCache& spuCache)
{
	Spu::MemBaseMarker marker;
	C_Shape* pShape = pObj->m_pShape;
	U32 shapeType = Spu::Get((U32*)&pObj->m_pShape->m_ShapeType);
	if(shapeType == E_Continuous_Shape)
	{
		//pShape = (C_Shape*)Spu::FetchAlloc(((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape, sizeof(Collision::C_Shape));
		pShape = (C_Shape*)Spu::Get((INTPTR*)(&((Collision::C_ContinuousShape*)pShape)->m_pDiscreteShape));	// [Blur2PC][x64] pointer-wide
		shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
	}
	//C_CollisionFilter* pRayFilter = &ray.m_Filter;//(C_CollisionFilter*)Spu::FetchAlloc(ray.m_pFilter, sizeof(C_CollisionFilter));
	//Spu::Wait(1 << 1);
	C_CollisionFilter* pShapeFilter = (C_CollisionFilter*)Spu::FetchAlloc((&pShape->m_Filter), sizeof(C_CollisionFilter));
	
	if(pRayFilter->CanCollide(pShapeFilter))//pShapeFilter
	{
		bool bResult = false;

		switch(shapeType)
		{
		case E_Box_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_BoxShape));
				//bResult = CollideOBBRay(pDerivedShape, ray, pObj->m_pNodePtr) || bResult;
				bResult = GJKRayCastBox(pDerivedShape, ray, pObj->m_pNodePtr) || bResult;
			}
			break;
		case E_Capsule_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CapsuleShape));
				//bResult = CollideCapsuleRay(pDerivedShape, ray, pObj->m_pNodePtr) || bResult;
				bResult = GJKRayCastCapsule(pDerivedShape, ray, pObj->m_pNodePtr) || bResult;
			}
			break;
		case E_Composite_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CompositeShape));
				//bResult = CollideCompositeRay(pDerivedShape, ray, stack, pRayFilter, spuCache, pObj->m_pNodePtr) || bResult;
				bResult = GJKRayCastComposite(pDerivedShape, ray, stack, pRayFilter, spuCache, pObj->m_pNodePtr) || bResult;
			}
			break;
		case E_ConvexHull_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConvexHullShape));
				/*bResult = CollideConvexHullFatRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;*/
				bResult = GJKRayCastConvex(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
				break;
			}
		case E_Cylinder_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CylinderShape));
				/*bResult = CollideCylinderRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;*/
				bResult = GJKRayCastCylinder(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
			}
			break;
		case E_Sphere_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_SphereShape));
				/*bResult = CollideSphereRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;*/
				bResult = GJKRayCastSphere(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
			}
			break;
		case E_InstancedTriangleMesh_Shape:
			{
				C_InstancedTriangleMeshShape* pMeshShape = (C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
				
				if(pMeshShape->m_AABBTree.RayTracing<Collision::CollideFatRayTriangleMesh>(ray, pRayFilter, pShape, pObj->m_pNodePtr, stack, spuCache))
				{
					bResult = true;
				}
				break;
			}
		case E_FluidMesh_Shape:
			{
				C_FluidMeshShape* pFluidShape = (C_FluidMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_FluidMeshShape));
				if(pFluidShape->m_AABBTree.RayTracing<Collision::CollideFatRayTriangleMesh>(ray, pRayFilter, pShape, pObj->m_pNodePtr, stack, spuCache))
				{
					bResult = true;
				}
				break;
			}
		case E_Cone_Shape:
			{
				Collision::C_Shape* pDerivedShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConeShape));
				//bResult = CollideConeRay(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
				bResult = GJKRayCastCone(pDerivedShape, ray, pObj->m_pNodePtr)||bResult;
				break;
			}
		}
		return bResult;
	}

	return false;
}



}
