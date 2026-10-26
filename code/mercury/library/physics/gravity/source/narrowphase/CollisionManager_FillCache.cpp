#include "StdAfx.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/AxisBox.inl"
namespace Collision
{

#if !defined(PLATFORM_SPU) || defined(SPU_FILL_CACHE)
	void C_CollisionManager::FillCache(Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, Collision::C_LineCollisionCache* pCache, 
		Collision::C_CRAxisBox& box, Collision::C_CollisionFilter* pFilter, 
		const Maths::C_Normal PCREF normal, const Maths::HF32 PCREF maxCosAngle, SharedPhysics::C_SafeStack<true>& stack, 
		S32 size, SharedPhysics::C_SmallSPUCache& spuCache)
	{
		U32 shapeType = Spu::Get((U32*)&pShape->m_ShapeType);

		if(shapeType == Collision::E_InstancedTriangleMesh_Shape)
		{
			//Fill cache!!!
			Collision::C_InstancedTriangleMeshShape* pInstancedMesh = (C_InstancedTriangleMeshShape*)Spu::FetchAlloc(pShape, sizeof(C_InstancedTriangleMeshShape));
			pInstancedMesh->m_AABBTree.FillLineCollisionCache(box, *pCache, stack, pFilter, pShape, pGraphNode,
				normal, maxCosAngle, spuCache);
		}
	}
#endif

#if !defined (PLATFORM_SPU) || defined(SPU_FLUID_COLLISION)
	Maths::HF32 C_CollisionManager::GetSubVolume(Collision::C_Shape* _pShape, const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		Spu::MemBaseMarker marker;
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());

		Collision::C_Shape* pShape = _pShape;

		U32 shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
		U32 hasVolume = Spu::Get((U32*)&pShape->m_HasVolume);

		//Collision::C_Shape* pTestShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_Shape));
		
		if(!hasVolume)
		{
			ratio = zero;
			return zero;
		}

		if(shapeType == Collision::E_Continuous_Shape)
		{
			pShape = (Collision::C_Shape*)Spu::Get((INTPTR*)(&((Collision::C_ContinuousShape*)_pShape)->m_pDiscreteShape));	// [Blur2PC][x64] pointer-wide
			U32 shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
			U32 hasVolume = Spu::Get((U32*)&pShape->m_HasVolume);
		}

		switch(shapeType)
		{
		case Collision::E_Box_Shape:
			{
				Collision::C_BoxShape* pBoxShape = (Collision::C_BoxShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_BoxShape));
				return pBoxShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_ConvexHull_Shape:
			{
				Collision::C_ConvexHullShape* pHullShape = (Collision::C_ConvexHullShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConvexHullShape));
				return pHullShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Sphere_Shape:
			{
				Collision::C_SphereShape* pSphereShape = (Collision::C_SphereShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_SphereShape));
				return pSphereShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Capsule_Shape:
			{
				Collision::C_CapsuleShape* pCapsuleShape = (Collision::C_CapsuleShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CapsuleShape));
				return pCapsuleShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Cylinder_Shape:
			{
				Collision::C_CylinderShape* pCylinderShape = (Collision::C_CylinderShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CylinderShape));
				return pCylinderShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Composite_Shape:
			{
				Collision::C_CompositeShape* pCompositeShape = (Collision::C_CompositeShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_CompositeShape));
				HF32 volume(zero);
				HF32 tempRatio(zero);
				HF32 total(zero);
				const Maths::HF32 one(Maths::Constants<HF32>::One());
				Maths::HF32 numShapes(zero);
				HF32 totalVolume(zero);
				Collision::C_CompositeShapeIterator iter(pCompositeShape);
				//for(U32 i=0; i< pCompositeShape->m_NumShapes; ++i )
				while(iter.HasNextShape())
				{
					Spu::MemBaseMarker innerMarker;
					iter.NextShape();
					//Collision::C_ShapeWrapper* pWrapper = (Collision::C_ShapeWrapper*)Spu::FetchAlloc(pCompositeShape->m_pShapes + i, sizeof(Collision::C_ShapeWrapper));
					//Collision::C_Shape* pShape = (Collision::C_Shape*)Spu::FetchAlloc(pWrapper->m_pShape, sizeof(Collision::C_Shape));
					//U32 bHasVolume = Spu::Get((U32*)&pWrapper->m_pShape->m_HasVolume);
					
					//TODO - get all this working on SPU!!!
					Collision::C_Shape* pShape = iter.GetCurrentShape<Collision::C_Shape>();
					if(pShape->m_HasVolume)
					{
						Maths::HVec3 centre(zero);
						tempRatio = zero;
						const Maths::HF32 submergedVolume = GetSubVolume2(pShape, p, normal, centre, tempRatio);
						volume += submergedVolume;
						centroid += centre * submergedVolume;
						totalVolume += submergedVolume / tempRatio;
						total += tempRatio;
						numShapes += one;
					}
				}
				ratio = volume/totalVolume;
				centroid = centroid/volume;
				return volume;
			}
		case Collision::E_Cone_Shape:
			{
				Collision::C_ConeShape* pConeShape = (Collision::C_ConeShape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_ConeShape));
				return pConeShape->GetSubVolume(p, normal, centroid, ratio);
			}
		default:
			return zero;

		}
	}
#endif


#if !defined (PLATFORM_SPU) || defined(SPU_FLUID_COLLISION)
	Maths::HF32 C_CollisionManager::GetSubVolume2(Collision::C_Shape* pShape, const Maths::HVec3 PCREF p, const Maths::C_Normal PCREF normal, Maths::HVec3& centroid, Maths::HF32& ratio)
	{
		Spu::MemBaseMarker marker;
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());
		U32 shapeType = pShape->m_ShapeType;
		U32 hasVolume = pShape->m_HasVolume;

		//Collision::C_Shape* pTestShape = (Collision::C_Shape*)Spu::FetchAlloc(pShape, sizeof(Collision::C_Shape));
		if(!hasVolume)
		{
			ratio = zero;
			return zero;
		}
		switch(shapeType)
		{
		case Collision::E_Box_Shape:
			{
				Collision::C_BoxShape* pBoxShape = (Collision::C_BoxShape*)pShape;
				return pBoxShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_ConvexHull_Shape:
			{
				Collision::C_ConvexHullShape* pHullShape = (Collision::C_ConvexHullShape*)pShape;
				return pHullShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Sphere_Shape:
			{
				Collision::C_SphereShape* pSphereShape = (Collision::C_SphereShape*)pShape;
				return pSphereShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Capsule_Shape:
			{
				Collision::C_CapsuleShape* pCapsuleShape = (Collision::C_CapsuleShape*)pShape;
				return pCapsuleShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Cylinder_Shape:
			{
				Collision::C_CylinderShape* pCylinderShape = (Collision::C_CylinderShape*)pShape;
				return pCylinderShape->GetSubVolume(p, normal, centroid, ratio);
			}
		case Collision::E_Composite_Shape:
			{
				Collision::C_CompositeShape* pCompositeShape = (Collision::C_CompositeShape*)pShape;
				HF32 volume(zero);
				HF32 tempRatio(zero);
				HF32 total(zero);
				const Maths::HF32 one(Maths::Constants<HF32>::One());
				Maths::HF32 numShapes(zero);
				HF32 totalVolume(zero);
				Collision::C_CompositeShapeIterator iter(pCompositeShape);
				//for(U32 i=0; i< pCompositeShape->m_NumShapes; ++i )
				while(iter.HasNextShape())
				{
					Spu::MemBaseMarker innerMarker;
					iter.NextShape();
					//Collision::C_ShapeWrapper* pWrapper = (Collision::C_ShapeWrapper*)Spu::FetchAlloc(pCompositeShape->m_pShapes + i, sizeof(Collision::C_ShapeWrapper));
					//Collision::C_Shape* pShape = (Collision::C_Shape*)Spu::FetchAlloc(pWrapper->m_pShape, sizeof(Collision::C_Shape));
					//U32 bHasVolume = Spu::Get((U32*)&pWrapper->m_pShape->m_HasVolume);
					
					//TODO - get all this working on SPU!!!
					Collision::C_Shape* pShape = iter.GetCurrentShape<Collision::C_Shape>();
					if(pShape->m_HasVolume)
					{
						Maths::HVec3 centre(zero);
						tempRatio = zero;
						const Maths::HF32 submergedVolume = GetSubVolume2(pShape, p, normal, centre, tempRatio);
						volume += submergedVolume;
						centroid += centre * submergedVolume;
						totalVolume += submergedVolume / tempRatio;
						total += tempRatio;
						numShapes += one;
					}
				}
				ratio = volume/totalVolume;
				centroid = centroid/volume;
				return volume;
			}
		case Collision::E_Cone_Shape:
			{
				Collision::C_ConeShape* pConeShape = (Collision::C_ConeShape*)pShape;
				return pConeShape->GetSubVolume(p, normal, centroid, ratio);
			}
		default:
			return zero;

		}
	}
#endif

}

