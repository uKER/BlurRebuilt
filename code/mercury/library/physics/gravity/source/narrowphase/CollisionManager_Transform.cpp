#include "StdAfx.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/Shapes.h"

namespace Collision
{

	void C_CollisionManager::TransformShapeAndBox(Collision::C_Shape* pShape, const Maths::HMat43& matrix,
		Collision::C_CRAxisBox& box)
	{
		Spu::MemBaseMarker marker;
		U32 shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
		Collision::C_Shape* pSpuShape = NULL;

		switch(shapeType)
		{
		case Collision::E_Continuous_Shape:
			{
				Collision::C_ContinuousShape* pDerivedShape = (Collision::C_ContinuousShape*)Spu::FetchAlloc(pShape, sizeof(C_ContinuousShape));
				pSpuShape = pDerivedShape;
				//pDerivedShape->Transform(matrix);
				pDerivedShape->m_PreviousTransform = pDerivedShape->m_PosRot;
				pDerivedShape->m_PosRot = matrix;
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape, pDerivedShape, sizeof(Collision::C_ContinuousShape), 10);
#endif
				TransformShape(pDerivedShape->m_pDiscreteShape, matrix);
				TransformShape(pDerivedShape->m_pContinuousPrimitive, matrix);
#ifdef PLATFORM_SPU
				Spu::Wait(10);
#endif
				break;
			}
		case Collision::E_Composite_Shape:
			{
				Collision::C_CompositeShape* pCompositeShape = (Collision::C_CompositeShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CompositeShape));
				pSpuShape = pCompositeShape;
				pCompositeShape->m_PosRot = matrix;
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape, pCompositeShape, sizeof(Collision::C_CompositeShape), 10);
#endif
				Collision::C_CompositeShapeIterator iter(pCompositeShape);
				//for(U32 i=0; i < pCompositeShape->m_NumShapes; ++i)
				while(iter.HasNextShape())
				{
					Spu::MemBaseMarker innerMarker;
					iter.NextShape();
					Collision::C_Shape* pShape = iter.GetCurrentShape<Collision::C_Shape>();
					/*C_ShapeWrapper* shapeWrapper=(C_ShapeWrapper*)Spu::FetchAlloc(&pCompositeShape->m_pShapes[i], sizeof(C_ShapeWrapper));
					const Maths::HMat43 absPosRot(shapeWrapper->m_RelativeTransformation * matrix);*/

					const Maths::HMat43 absPosRot(pShape->m_RelativeTransformation * matrix);

					TransformShapeLocal(pShape, absPosRot);
					//Spu::StoreAsync(&pCompositeShape->m_pShapes[i], shapeWrapper, sizeof(C_ShapeWrapper));
				}
#ifdef PLATFORM_SPU
				Spu::Wait(10);
#endif

			break;
			}
		case Collision::E_Box_Shape:
			{
				Collision::C_BoxShape* pDerivedShape = (Collision::C_BoxShape*)Spu::FetchAlloc(pShape, sizeof(C_BoxShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_BoxShape));
#endif
				break;
			}
		case Collision::E_Sphere_Shape:
			{
				Collision::C_SphereShape* pDerivedShape = (Collision::C_SphereShape*)Spu::FetchAlloc(pShape, sizeof(C_SphereShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_SphereShape));
#endif
				break;
			}
		case Collision::E_Capsule_Shape:
			{
				Collision::C_CapsuleShape* pDerivedShape = (Collision::C_CapsuleShape*)Spu::FetchAlloc(pShape, sizeof(C_CapsuleShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_CapsuleShape));
#endif
				break;
			}
		case Collision::E_ConvexHull_Shape:
			{
				Collision::C_ConvexHullShape* pDerivedShape = (Collision::C_ConvexHullShape*)Spu::FetchAlloc(pShape, sizeof(C_ConvexHullShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_ConvexHullShape));
#endif
				break;
			}
		case Collision::E_Cylinder_Shape:
			{
				Collision::C_CylinderShape* pDerivedShape = (Collision::C_CylinderShape*)Spu::FetchAlloc(pShape, sizeof(C_CylinderShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_CylinderShape));
#endif
				break;
			}
		case Collision::E_Cone_Shape:
			{
				Collision::C_ConeShape* pDerivedShape = (Collision::C_ConeShape*)Spu::FetchAlloc(pShape, sizeof(C_ConeShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_ConeShape));
#endif
				break;
			}
		
		case Collision::E_CharacterController_Shape:
			{
				Collision::C_CharacterControllerShape* pDerivedShape = (Collision::C_CharacterControllerShape*)Spu::FetchAlloc(pShape, sizeof(C_CharacterControllerShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_CharacterControllerShape));
#endif
				break;
			}
		case Collision::E_InstancedTriangleMesh_Shape:
			{
				Collision::C_InstancedTriangleMeshShape* pDerivedShape = (Collision::C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_InstancedTriangleMeshShape));
#endif
				break;
			}
		case Collision::E_FluidMesh_Shape:
			{
				Collision::C_FluidMeshShape* pDerivedShape = (Collision::C_FluidMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_FluidMeshShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_FluidMeshShape));
#endif
				break;
			}
		case Collision::E_Cloth_Shape:
			{
				Collision::C_ClothShape* pDerivedShape = (Collision::C_ClothShape*)Spu::FetchAlloc(pShape, sizeof(C_ClothShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_ClothShape));
#endif
				break;
			}
		case Collision::E_Fluid_Shape:
			{
				Collision::C_FluidShape* pDerivedShape = (Collision::C_FluidShape*)Spu::FetchAlloc(pShape, sizeof(C_FluidShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_FluidShape));
#endif
				break;
			}
		case Collision::E_Rope_Shape:
			{
				Collision::C_RopeShape* pDerivedShape = (Collision::C_RopeShape*)Spu::FetchAlloc(pShape, sizeof(C_RopeShape));
				pSpuShape = pDerivedShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_RopeShape));
#endif
				break;
			}
		default:
			return;
		}

		//pShape->Transform(matrix);
		pSpuShape->TransformBox(box);
	}


	void C_CollisionManager::TransformShape(Collision::C_Shape* pShape, const Maths::HMat43& matrix)
	{
		Spu::MemBaseMarker marker;
		U32 shapeType = Spu::Get((U32*)&pShape->m_ShapeType);

		switch(shapeType)
		{
		case Collision::E_Composite_Shape:
			{
				Collision::C_CompositeShape* pCompositeShape = (Collision::C_CompositeShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CompositeShape));
				pCompositeShape->m_PosRot = matrix;
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape, pCompositeShape, sizeof(Collision::C_CompositeShape), 10);
#endif
				Collision::C_CompositeShapeIterator iter(pCompositeShape);
				//for(U32 i=0; i < pCompositeShape->m_NumShapes; ++i)
				while(iter.HasNextShape())
				{
					Spu::MemBaseMarker innerMarker;
					iter.NextShape();

					/*C_ShapeWrapper* shapeWrapper=(C_ShapeWrapper*)Spu::FetchAlloc(&pCompositeShape->m_pShapes[i], sizeof(C_ShapeWrapper));
					const Maths::HMat43 absPosRot(shapeWrapper->m_RelativeTransformation * matrix);*/
					Collision::C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
					const Maths::HMat43 absPosRot(pShape->m_RelativeTransformation * matrix);
					TransformShapeLocal(pShape, absPosRot);
				}
#ifdef PLATFORM_SPU
				Spu::Wait(10);
#endif

			break;
			}
		case Collision::E_Box_Shape:
			{
				Collision::C_BoxShape* pDerivedShape = (Collision::C_BoxShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_BoxShape));
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_BoxShape));
#endif
				break;
			}
		case Collision::E_Sphere_Shape:
			{
				Collision::C_SphereShape* pDerivedShape = (Collision::C_SphereShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_SphereShape));
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_SphereShape));
#endif
				break;
			}
		case Collision::E_Capsule_Shape:
			{
				Collision::C_CapsuleShape* pDerivedShape = (Collision::C_CapsuleShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CapsuleShape));
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_CapsuleShape));
#endif
				break;
			}
		case Collision::E_ConvexHull_Shape:
			{
				Collision::C_ConvexHullShape* pDerivedShape = (Collision::C_ConvexHullShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConvexHullShape));
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_ConvexHullShape));
#endif
				break;
			}
		case Collision::E_Cylinder_Shape:
			{
				Collision::C_CylinderShape* pDerivedShape = (Collision::C_CylinderShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CylinderShape));
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_CylinderShape));
#endif
				break;
			}
		case Collision::E_Cone_Shape:
			{
				Collision::C_ConeShape* pDerivedShape = (Collision::C_ConeShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConeShape));
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_ConeShape));
#endif
				break;
			}
		
		case Collision::E_InstancedTriangleMesh_Shape:
			{
				Collision::C_InstancedTriangleMeshShape* pDerivedShape = (Collision::C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_InstancedTriangleMeshShape));
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::Store(pShape, pDerivedShape, sizeof(Collision::C_InstancedTriangleMeshShape));
#endif
				break;
			}
		default:
			return;
		}
	}


	void C_CollisionManager::TransformShapeLocal(Collision::C_Shape* pShape, const Maths::HMat43& matrix)
	{
		Spu::MemBaseMarker marker;
		U32 shapeType = pShape->m_ShapeType;

		switch(shapeType)
		{
		case Collision::E_Composite_Shape:
			{
				Collision::C_CompositeShape* pCompositeShape = (Collision::C_CompositeShape*)pShape;
				pCompositeShape->m_PosRot = matrix;
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pCompositeShape, sizeof(Collision::C_CompositeShape), 10);
#endif
				Collision::C_CompositeShapeIterator iter(pCompositeShape);
				//for(U32 i=0; i < pCompositeShape->m_NumShapes; ++i)
				while(iter.HasNextShape())
				{
					Spu::MemBaseMarker innerMarker;
					iter.NextShape();

					/*C_ShapeWrapper* shapeWrapper=(C_ShapeWrapper*)Spu::FetchAlloc(&pCompositeShape->m_pShapes[i], sizeof(C_ShapeWrapper));
					const Maths::HMat43 absPosRot(shapeWrapper->m_RelativeTransformation * matrix);*/
					Collision::C_Shape* pShape = iter.GetCurrentShape<C_Shape>();
					const Maths::HMat43 absPosRot(pShape->m_RelativeTransformation * matrix);
					TransformShapeLocal(pShape, absPosRot);
				}
#ifdef PLATFORM_SPU
				Spu::Wait(10);
#endif

			break;
			}
		case Collision::E_Box_Shape:
			{
				Collision::C_BoxShape* pDerivedShape = (Collision::C_BoxShape*)pShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pDerivedShape, sizeof(Collision::C_BoxShape), 10);
#endif
				break;
			}
		case Collision::E_Sphere_Shape:
			{
				Collision::C_SphereShape* pDerivedShape = (Collision::C_SphereShape*)pShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pDerivedShape, sizeof(Collision::C_SphereShape), 10);
#endif
				break;
			}
		case Collision::E_Capsule_Shape:
			{
				Collision::C_CapsuleShape* pDerivedShape = (Collision::C_CapsuleShape*)pShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pDerivedShape, sizeof(Collision::C_CapsuleShape), 10);
#endif
				break;
			}
		case Collision::E_ConvexHull_Shape:
			{
				Collision::C_ConvexHullShape* pDerivedShape = (Collision::C_ConvexHullShape*)pShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pDerivedShape, sizeof(Collision::C_ConvexHullShape), 10);
#endif
				break;
			}
		case Collision::E_Cylinder_Shape:
			{
				Collision::C_CylinderShape* pDerivedShape = (Collision::C_CylinderShape*)pShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pDerivedShape, sizeof(Collision::C_CylinderShape), 10);
#endif
				break;
			}
		case Collision::E_Cone_Shape:
			{
				Collision::C_ConeShape* pDerivedShape = (Collision::C_ConeShape*)pShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pDerivedShape, sizeof(Collision::C_ConeShape), 10);
#endif
				break;
			}
		
		case Collision::E_InstancedTriangleMesh_Shape:
			{
				Collision::C_InstancedTriangleMeshShape* pDerivedShape = (Collision::C_InstancedTriangleMeshShape*)pShape;
				pDerivedShape->Transform(matrix);
#ifdef PLATFORM_SPU
				Spu::StoreAsync(pShape->m_pPpuPtr, pDerivedShape, sizeof(Collision::C_InstancedTriangleMeshShape), 10);
#endif
				break;
			}
		default:
			return;
		}
	}


}