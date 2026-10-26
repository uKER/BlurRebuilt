#include "stdafx.h"

#include "gravity/narrowphase/SPUManager.h"
#include "gravity/narrowphase/GJK.h"
#include "gravity/narrowphase/Primitive.h"
#include "gravity/narrowphase/Shapes.h"

namespace Collision
{
#ifdef PLATFORM_SPU
	bool SPUManager::QuadraticShape(const C_Convex* convex)
	{
		return convex->m_Type == Collision::E_CONVEX_SPHERE || 
			convex->m_Type == Collision::E_CONVEX_CAPSULE || 
			convex->m_Type == Collision::E_CONVEX_TRIANGLE || 
			convex->m_Type == Collision::E_CONVEX_CONE;
	}

	Maths::HVec3 SPUManager::Support(const C_Shape* pShape, const Maths::HVec3 PCREF dir)
	{
		Spu::MemBaseMarker marker;
		U32 shapeType = Spu::Get((U32*)&pShape->m_ShapeType);
		switch(shapeType)
		{
		case Collision::E_Box_Shape:
			{
				C_BoxShape* pOBB =(C_BoxShape*)Spu::FetchAlloc(pShape, sizeof(C_BoxShape));
				return pOBB->Support(dir);
			}
		case Collision::E_ConvexHull_Shape:
			{
				C_ConvexHullShape* pConvex = (C_ConvexHullShape*)Spu::FetchAlloc(pShape, sizeof(C_ConvexHullShape));
				return pConvex->Support(dir);
			}
		case Collision::E_Sphere_Shape:
			{
				C_SphereShape* pSphere = (C_SphereShape*)Spu::FetchAlloc(pShape, sizeof(C_SphereShape));
				return pSphere->Support(dir);
			}
		case Collision::E_Capsule_Shape:
			{
				C_CapsuleShape* pCapsule = (C_CapsuleShape*)Spu::FetchAlloc(pShape, sizeof(C_CapsuleShape));
				return pCapsule->Support(dir);
			}
		case Collision::E_Cylinder_Shape:
			{
				C_CylinderShape* pCylinder = (C_CylinderShape*)Spu::FetchAlloc(pShape, sizeof(C_CylinderShape));
				return pCylinder->Support(dir);
			}
		case Collision::E_Cone_Shape:
			{
				C_ConeShape* pCone = (C_ConeShape*)Spu::FetchAlloc(pShape, sizeof(C_ConeShape));
				return pCone->Support(dir);
			}
		default:
			assert(0 && "Shape Not Supported");
		}
	}

	Maths::HVec3 SPUManager::Support(const C_Convex* convex, const Maths::HVec3 PCREF dir)
	{
		switch(convex->m_Type)
		{
			case Collision::E_CONVEX_SPHERE:
				{
					C_Sphere* pSphere=(C_Sphere*)convex;
					return pSphere->Support(dir);
				}
			case Collision::E_CONVEX_OBB:
				{
					C_OBB* pOBB=(C_OBB*)convex;
					return pOBB->Support(dir);
				}
			case Collision::E_CONVEX_CAPSULE:
				{
					C_Capsule* pCapsule=(C_Capsule*)convex;
					return pCapsule->Support(dir);
				}
			case Collision::E_CONVEX_CYLINDER:
				{
					C_Cylinder* pCylinder=(C_Cylinder*)convex;
					return pCylinder->Support(dir);
				}
			case Collision::E_CONVEX_CONVEXHULL:
				{
					C_ConvexHull* pConvex=(C_ConvexHull*)convex;
					return pConvex->Support(dir);
				}
			case Collision::E_CONVEX_TRIANGLE:
				{
					C_Triangle* pTriangle=(C_Triangle*)convex;
					return pTriangle->Support(dir);
				}
			case Collision::E_CONVEX_CONE:
				{
					C_Cone* pCone= (C_Cone*)convex;
					return pCone->Support(dir);
				}
			case Collision::E_CONVEX_SPUCONVEXHULL:
				{
					C_SpuConvexHull* pConvex = (C_SpuConvexHull*)convex;
					return pConvex->Support(dir);
				}
			case Collision::E_CONVEX_SPUCONVEXHULL_LOCAL:
				{
					C_LocalSpaceSpuConvexHull* pConvex = (C_LocalSpaceSpuConvexHull*)convex;
					return pConvex->Support(dir);
				}
			default:
				{
					return Maths::HVec3(Maths::ZERO_INIT);
				}
		}
	}

	Maths::HVec3 SPUManager::SupportMargin(const C_Convex* convex, const Maths::HVec3 PCREF dir, const Maths::HF32 PCREF margin)
	{
		switch(convex->m_Type)
		{
			case Collision::E_CONVEX_SPHERE:
				{
					C_Sphere* pSphere=(C_Sphere*)convex;
					return pSphere->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_OBB:
				{
					C_OBB* pOBB=(C_OBB*)convex;
					return pOBB->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_CAPSULE:
				{
					C_Capsule* pCapsule=(C_Capsule*)convex;
					return pCapsule->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_CYLINDER:
				{
					C_Cylinder* pCylinder=(C_Cylinder*)convex;
					return pCylinder->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_CONVEXHULL:
				{
					C_ConvexHull* pConvex=(C_ConvexHull*)convex;
					return pConvex->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_TRIANGLE:
				{
					C_Triangle* pTriangle=(C_Triangle*)convex;
					return pTriangle->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_CONE:
				{
					C_Cone* pCone= (C_Cone*)convex;
					return pCone->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_SPUCONVEXHULL:
				{
					C_SpuConvexHull* pConvex = (C_SpuConvexHull*)convex;
					return pConvex->SupportMargin(dir, margin);
				}
			case Collision::E_CONVEX_SPUCONVEXHULL_LOCAL:
				{
					C_LocalSpaceSpuConvexHull* pConvex = (C_LocalSpaceSpuConvexHull*)convex;
					return pConvex->SupportMargin(dir, margin);
				}
			default:
				{
					return Maths::HVec3(Maths::ZERO_INIT);
				}
		}
	}


	/*Maths::HVec3 SPUManager::Centre(const C_Convex* convex)
	{
		switch(convex->m_Type)
		{
			case Collision::E_CONVEX_SPHERE:
				{
					C_Sphere* pSphere=(C_Sphere*)convex;
					return pSphere->Centre();
				}
			case Collision::E_CONVEX_OBB:
				{
					C_OBB* pOBB=(C_OBB*)convex;
					return pOBB->Centre();
				}
			case Collision::E_CONVEX_CAPSULE:
				{
					C_Capsule* pCapsule=(C_Capsule*)convex;
					return pCapsule->Centre();
				}
			case Collision::E_CONVEX_CYLINDER:
				{
					C_Cylinder* pCylinder=(C_Cylinder*)convex;
					return pCylinder->Centre();
				}
			case Collision::E_CONVEX_CONVEXHULL:
				{
					C_ConvexHull* pConvex=(C_ConvexHull*)convex;
					return pConvex->Centre();
				}
			case Collision::E_CONVEX_TRIANGLE:
				{
					C_Triangle* pTriangle=(C_Triangle*)convex;
					return pTriangle->Centre();
				}
			case Collision::E_CONVEX_CONE:
				{
					C_Cone* pCone= (C_Cone*)convex;
					return pCone->Centre();
				}
			case Collision::E_CONVEX_SPUCONVEXHULL:
				{
					C_SpuConvexHull* pConvex=(C_SpuConvexHull*)convex;
					return pConvex->Centre();
				}
			case Collision::E_CONVEX_SPUCONVEXHULL_LOCAL:
				{
					//We're in the local space - so the centre should be zero
					return Maths::Constants<Maths::HVec3>::Zero();
				}
			default:
				{
					return Maths::HVec3(Maths::ZERO_INIT);
				}
		}
	}*/

	//void SPUManager::WarmStart(const C_Convex* convex, const Maths::HVec3 PCREF warmStart)
	//{
	//	switch(convex->m_Type)
	//	{
	//		case Collision::E_CONVEX_SPHERE:
	//			{
	//				C_Sphere* pSphere=(C_Sphere*)convex;
	//				pSphere->WarmStart(warmStart);
	//			}
	//		case Collision::E_CONVEX_OBB:
	//			{
	//				C_OBB* pOBB=(C_OBB*)convex;
	//				pOBB->WarmStart(warmStart);
	//			}
	//		case Collision::E_CONVEX_CAPSULE:
	//			{
	//				C_Capsule* pCapsule=(C_Capsule*)convex;
	//				pCapsule->WarmStart(warmStart);
	//			}
	//		case Collision::E_CONVEX_CYLINDER:
	//			{
	//				C_Cylinder* pCylinder=(C_Cylinder*)convex;
	//				pCylinder->WarmStart(warmStart);
	//			}
	//		case Collision::E_CONVEX_CONVEXHULL:
	//			{
	//				C_ConvexHull* pConvex=(C_ConvexHull*)convex;
	//				pConvex->WarmStart(warmStart);
	//			}
	//		case Collision::E_CONVEX_TRIANGLE:
	//			{
	//				C_Triangle* pTriangle=(C_Triangle*)convex;
	//				pTriangle->WarmStart(warmStart);
	//			}
	//		case Collision::E_CONVEX_CONE:
	//			{
	//				C_Cone* pCone= (C_Cone*)convex;
	//				pCone->WarmStart(warmStart);
	//			}
	//		case Collision::E_CONVEX_SPUCONVEXHULL:
	//			{
	//				C_SpuConvexHull* pConvex=(C_SpuConvexHull*)convex;
	//				pConvex->WarmStart(warmStart);
	//			}
	//		default:
	//			{
	//				BIZ_ASSERT(0);
	//			}
	//	}
	//}


	//Maths::HVec3 SPUManager::WarmStart(const C_Convex* convex)
	//{
	//	switch(convex->m_Type)
	//	{
	//		case Collision::E_CONVEX_SPHERE:
	//			{
	//				C_Sphere* pSphere=(C_Sphere*)convex;
	//				return pSphere->WarmStart();
	//			}
	//		case Collision::E_CONVEX_OBB:
	//			{
	//				C_OBB* pOBB=(C_OBB*)convex;
	//				return pOBB->WarmStart();
	//			}
	//		case Collision::E_CONVEX_CAPSULE:
	//			{
	//				C_Capsule* pCapsule=(C_Capsule*)convex;
	//				return pCapsule->WarmStart(t);
	//			}
	//		case Collision::E_CONVEX_CYLINDER:
	//			{
	//				C_Cylinder* pCylinder=(C_Cylinder*)convex;
	//				return pCylinder->WarmStart();
	//			}
	//		case Collision::E_CONVEX_CONVEXHULL:
	//			{
	//				C_ConvexHull* pConvex=(C_ConvexHull*)convex;
	//				return pConvex->WarmStart();
	//			}
	//		case Collision::E_CONVEX_TRIANGLE:
	//			{
	//				C_Triangle* pTriangle=(C_Triangle*)convex;
	//				return pTriangle->WarmStart();
	//			}
	//		case Collision::E_CONVEX_CONE:
	//			{
	//				C_Cone* pCone= (C_Cone*)convex;
	//				return pCone->WarmStart();
	//			}
	//		case Collision::E_CONVEX_SPUCONVEXHULL:
	//			{
	//				C_SpuConvexHull* pConvex=(C_SpuConvexHull*)convex;
	//				return pConvex->WarmStart();
	//			}
	//		default:
	//			{
	//				BIZ_ASSERT(0);
	//			}
	//	}
	//}

#endif
}