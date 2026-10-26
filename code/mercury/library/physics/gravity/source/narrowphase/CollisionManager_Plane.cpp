#include "StdAfx.h"

#include <core/maths/hvec/VectorInt.h>
#include <core/maths/hvec/VectorInt.inl>

#include "gravity/narrowphase/CollisionResult.inl"
#include "gravity/sharedphysics/ContactStream.inl"
#include "gravity/sharedphysics/SortedArray.inl"
//#include "gravity/narrowphase/PrimitiveTest.inl"
#include "gravity/narrowphase/CollisionManager.inl"
#include "gravity/narrowphase/ConvexHull.h"
#include "gravity/narrowphase/TriangleTriangle.inl"
#include "gravity/narrowphase/CylinderTriangle.h"
#include "gravity/narrowphase/CharacterControllerPrimitive.h"
#include "gravity/narrowphase/CharacterControllerTriangle.inl"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/narrowphase/CompositeShapeTriangle.h"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/ConvexTriangle.h"
#include "system/perf.h"
//#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/SphereTriangle.inl"


namespace Collision
{


	bool C_CollisionManager::CollideGroundPlane( C_CollisionObject* a, C_CollisionObject*  b, SharedPhysics::C_ContactStream& stream, SharedPhysics::C_SafeStack<true>& stack, Collision::C_CollisionResult<_MaxPatches, _MaxContacts>& results, 
									 EPA& m_Epa)
	{

		//DebugBreak();
		using namespace Maths;

		Spu::MemBaseMarker marker; 
		PERF_EVENT("gravity.planeCollision");
		C_PlaneShape* aShape = (C_PlaneShape*)Spu::FetchAlloc(a->m_pShape, sizeof(C_PlaneShape));
		
		Collision::E_ShapeType bShapeType = (Collision::E_ShapeType)Spu::Get((U32*)&b->m_pShape->m_ShapeType);
		

		//C_Shape* bShape = (C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_Shape));

		

		C_Plane& plane = aShape->m_Plane;

		C_CRAxisBox& axisBox=b->m_TransformedBox;

		


		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(aVex, bVex, cVex, RGBA(0, 255, 0, 0));
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(aVex, cVex, dVex, RGBA(255, 0, 0, 0));

		
		//b->m_pSpuShape = bShape;

		results.m_pObjectA = b;
		results.m_pObjectB = a;

		results.m_pEpa = &m_Epa;

		const U32 physicsMaterial((aShape->PhysicsMaterialIndex() << 16));
		const U32 effectMaterial((aShape->EffectMaterialIndex() << 16));

		results.m_NumContactPatches = 1;
		Collision::C_ContactPatch<_MaxContacts>& patch = results.m_ContactPatches[0];
		patch.m_NumContactPoints = 0;
		patch.m_Normal = -plane.m_n;

		HVec3 aVex(UNDEFINED);
		HVec3 bVex(UNDEFINED);
		HVec3 cVex(UNDEFINED);
		HVec3 dVex(UNDEFINED);
		//Object a is a plane! 
		switch(bShapeType)
		{
		case E_Box_Shape:
			{
				C_BoxShape* boxShape = (C_BoxShape*) Spu::FetchAlloc(b->m_pShape, sizeof(C_BoxShape));
				b->m_pSpuShape = boxShape;
				C_OBB& obb = boxShape->m_Box;
				//b->m_pSpuShape = boxShape;

				//Rejection Test
				const HVec3 support1 = obb.Support(-plane.m_n);
				const HVec3 support2 = obb.Support( plane.m_n);

				const HF32 dist1 = (support1 | plane.m_n) - plane.m_d;
				const HF32 dist2 = (support2 | plane.m_n) - plane.m_d;
				const HF32 sign = dist1*dist2;
				//no intersection
				if(sign.GT(Constants<HF32>::Zero()))
					return false;

				plane.ProjectQuat(axisBox, aVex, bVex, cVex, dVex);

				Collision::AABBTreeTriangle triangle;
				const U32 physicsMaterialIndex =  boxShape->PhysicsMaterialIndex() | physicsMaterial;
				const U32 effectMaterialIndex = boxShape->EffectMaterialIndex() | effectMaterial;

				bool con1 =CollideOBBTriangle<_MaxContacts>(obb, aVex, bVex, cVex, patch.m_Normal, patch, triangle, 0, physicsMaterialIndex, effectMaterialIndex);
				bool con2 =CollideOBBTriangle<_MaxContacts>(obb, aVex, cVex, dVex, patch.m_Normal, patch, triangle, 0, physicsMaterialIndex, effectMaterialIndex);
			
				//if(con1 | con2)
				{
					StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
				}
			
				return true;
			}
		case E_ConvexHull_Shape:
			{
				C_ConvexHullShape* convexShape = (C_ConvexHullShape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_ConvexHullShape));
				b->m_pSpuShape = convexShape;
				//b->m_pSpuShape = convexShape;
				C_ConvexHull& convexHull = convexShape->m_Hull;

				//Rejection Test
				const HVec3 support1 = convexHull.Support(-plane.m_n);
				const HVec3 support2 = convexHull.Support( plane.m_n);

				const HF32 dist1 = (support1 | plane.m_n) - plane.m_d;
				const HF32 dist2 = (support2 | plane.m_n) - plane.m_d;
				const HF32 sign = dist1*dist2;
				//no intersection
				if(sign.GT(Constants<HF32>::Zero()))
					return false;

				plane.ProjectQuat(axisBox, aVex, bVex, cVex, dVex);

				const U32 physicsMaterialIndex =convexShape->PhysicsMaterialIndex() | physicsMaterial;
				const U32 effectMaterialIndex = convexShape->EffectMaterialIndex() | effectMaterial;
				
				
				bool con1 = CollideConvexHullTriangle<_MaxContacts>(convexHull, aVex, bVex, cVex, &patch, physicsMaterialIndex, effectMaterialIndex);
				bool con2 = CollideConvexHullTriangle<_MaxContacts>(convexHull, aVex, cVex, dVex, &patch, physicsMaterialIndex, effectMaterialIndex);
				if(con1 | con2)
				{
					StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
				}

				return true;
			}
		case E_Sphere_Shape:
			{
				C_SphereShape* sphereShape = (C_SphereShape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_SphereShape));
				b->m_pSpuShape = sphereShape;
				//b->m_pSpuShape = sphereShape;
				//Rejection Test
				C_Sphere& sphere = sphereShape->m_Sphere;
				const HVec3 support1 = sphere.Support(-plane.m_n);
				const HVec3 support2 = sphere.Support( plane.m_n);

				const HF32 dist1 = (support1 | plane.m_n) - plane.m_d;
				const HF32 dist2 = (support2 | plane.m_n) - plane.m_d;
				const HF32 sign = dist1*dist2;
				//no intersection
				if(sign.GT(Constants<HF32>::Zero()))
					return false;

				plane.ProjectQuat(axisBox, aVex, bVex, cVex, dVex);

				
				//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(sphere.Centre(), 0.25f, RGBA(0, 255, 0, 0));
				/*bool con1 = CollideSphereTriangle<_MaxContacts>(sphere, aVex, bVex, cVex, patch, physicsMaterial, effectMaterial);
				bool con2 = CollideSphereTriangle<_MaxContacts>(sphere, aVex, cVex, dVex, patch, physicsMaterial, effectMaterial);
				if(con1 | con2)
				{
					StoreContacts<_MaxPatches, _MaxContacts, 3, 4>(b, a, stream, results);
				}*/
				const U32 physicsMaterialIndex =  sphereShape->PhysicsMaterialIndex() | physicsMaterial;
				const U32 effectMaterialIndex = sphereShape->EffectMaterialIndex() | effectMaterial;

				CollideSpherePlane<_MaxContacts>(sphere, plane, patch,  physicsMaterialIndex, effectMaterialIndex);
				StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
				return true;

			}
		case E_Capsule_Shape:
			{
				C_CapsuleShape* capsuleShape = (C_CapsuleShape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_CapsuleShape));
				b->m_pSpuShape = capsuleShape;
				//b->m_pSpuShape = capsuleShape;
				C_Capsule& capsule = capsuleShape->m_Capsule;

				//Rejection Test
				const HVec3 support1 = capsule.Support(-plane.m_n);
				const HVec3 support2 = capsule.Support( plane.m_n);

				const HF32 dist1 = (support1 | plane.m_n) - plane.m_d;
				const HF32 dist2 = (support2 | plane.m_n) - plane.m_d;
				const HF32 sign = dist1*dist2;
				//no intersection
				if(sign.GT(Constants<HF32>::Zero()))
					return false;

				plane.ProjectQuat(axisBox, aVex, bVex, cVex, dVex);

				const U32 physicsMaterialIndex = capsuleShape->PhysicsMaterialIndex() | physicsMaterial;
				const U32 effectMaterialIndex = capsuleShape->EffectMaterialIndex() | effectMaterial;

				
				bool con1 = CollideCapsuleTriangle<_MaxContacts>(aVex, bVex, cVex, plane.m_n, capsule, patch, physicsMaterialIndex, effectMaterialIndex, 0, 0);
				bool con2 = CollideCapsuleTriangle<_MaxContacts>(aVex, cVex, dVex, plane.m_n, capsule, patch, physicsMaterialIndex, effectMaterialIndex, 0, 0);
			//	if(con1|con2)
				{
					StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
				}
				return true;
			}
		case E_Cylinder_Shape:
			{
				C_CylinderShape* cylinderShape = (C_CylinderShape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_CylinderShape));
				b->m_pSpuShape = cylinderShape;
				//b->m_pSpuShape = cylinderShape;
				C_Cylinder& cylinder = cylinderShape->m_Cylinder;
				//Rejection Test
				const HVec3 support1 = cylinder.Support(-plane.m_n);
				const HVec3 support2 = cylinder.Support( plane.m_n);

				const HF32 dist1 = (support1 | plane.m_n) - plane.m_d;
				const HF32 dist2 = (support2 | plane.m_n) - plane.m_d;
				const HF32 sign = dist1*dist2;
				//no intersection
				if(sign.GT(Constants<HF32>::Zero()))
					return false;

				plane.ProjectQuat(axisBox, aVex, bVex, cVex, dVex);

				const U32 physicsMaterialIndex =cylinderShape->PhysicsMaterialIndex() | physicsMaterial;
				const U32 effectMaterialIndex = cylinderShape->EffectMaterialIndex() | effectMaterial;

				
				bool con1 = CollideCylinderTriangle(cylinder, aVex, bVex, cVex, patch, 0, physicsMaterialIndex, effectMaterialIndex);
				bool con2 = CollideCylinderTriangle(cylinder, aVex, cVex, dVex, patch, 0, physicsMaterialIndex, effectMaterialIndex);
				//if(con1 | con2)
				{
					StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
				}
				return true;
			}
		case E_Composite_Shape:
			{
				C_Shape* bShape = (C_Shape*)Spu::FetchAlloc(b->m_pShape, sizeof(C_CompositeShape));
				b->m_pSpuShape = bShape;

				plane.ProjectQuat(axisBox, aVex, bVex, cVex, dVex);

				const U32 physicsMaterialIndex =bShape->PhysicsMaterialIndex() | physicsMaterial;
				const U32 effectMaterialIndex = bShape->EffectMaterialIndex() | effectMaterial;

				bool con1 = CollideCompositeShapeTriangle<_MaxContacts>(bShape, aVex, bVex, cVex, patch, physicsMaterialIndex, effectMaterialIndex,
					aShape->m_Filter.GetCollisionFlags());
				bool con2 = CollideCompositeShapeTriangle<_MaxContacts>(bShape, aVex, cVex, dVex, patch, physicsMaterialIndex, effectMaterialIndex,
					aShape->m_Filter.GetCollisionFlags());
				if(con1|con2)
				{
					StoreContacts<_MaxPatches, _MaxContacts, _DesiredPatches, _DesiredContacts>(b, a, stream, results);
					return true;
				}
				return false;
			}
		default:
			return false;

		}
	}
}
