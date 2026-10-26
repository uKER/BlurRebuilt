#ifndef __COMPOSITESHAPETRIANGLE_INL__
#define __COMPOSITESHAPETRIANGLE_INL__

#include "gravity/narrowphase/CompositeShapeTriangle.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/OBBTriangle.h"
#include "gravity/narrowphase/SphereTriangle.h"
#include "gravity/narrowphase/CapsuleTriangle.h"
#include "gravity/narrowphase/ConvexTriangle.h"
#include "gravity/narrowphase/ConeTriangle.h"

namespace Collision
{

template < int MaxContacts>
bool CollideCompositeShapeTriangle(const C_Shape* pShape, const Maths::HVec3 PCREF aVex, const Maths::HVec3 PCREF bVex, const Maths::HVec3 PCREF cVex, Collision::C_ContactPatch<MaxContacts>& patch, const U32 _physicsMaterial, const U32 _effectMaterial,
								   const U32 collisionFlags)
{
	//DebugBreak();
	C_CompositeShape* pCShape((C_CompositeShape*)pShape);
	bool newCollide = false;
	bool collided = false;

	const U32 physMat = _physicsMaterial & 0xffff0000;
	const U32 effMat = _effectMaterial & 0xffff0000;

	Collision::C_CompositeShapeIterator iter(pCShape);
	//DebugBreak();
	//for(U32 i=0; i< pCShape->m_NumShapes; ++i)
	while(iter.HasNextShape())
	{
		Spu::MemBaseMarker marker;
		iter.NextShape();
		/*C_ShapeWrapper* wrapper=(C_ShapeWrapper*)Spu::FetchAlloc(pCShape->m_pShapes + i, sizeof(C_ShapeWrapper));
		C_Shape* pSubShape = (C_Shape*)Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_Shape));*/
		C_Shape* pSubShape = iter.GetCurrentShape<C_Shape>();
		//U32 type=Spu::Get((U32*)&wrapper->m_pShape->m_ShapeType);
		U32 type = pSubShape->m_ShapeType;
		
		if(pSubShape->CanCollide(collisionFlags))
		{
			const U32 physicsMaterial = physMat | pSubShape->m_PhysicsMaterialIndex;
			const U32 effectMaterial = effMat | pSubShape->m_EffectMaterialIndex;
			switch(type)
			{
			case E_Box_Shape:
				{

					Collision::AABBTreeTriangle triangle;
					Collision::C_BoxShape* pOBB = (C_BoxShape*)pSubShape;//Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_BoxShape));
					if(CollideOBBTriangle<_MaxContacts>(pOBB->m_Box, aVex, bVex, cVex, patch.m_Normal, patch, triangle, 0, (U32)physicsMaterial, (U32)effectMaterial), collisionFlags)
					{
						newCollide = true;
						//Collision::ReduceContacts<MaxPatches, MaxContacts, 3, 6>(results);
					}
					break;
				}
			case E_ConvexHull_Shape:
				{
					Collision::C_ConvexHullShape* pConvexHull = (C_ConvexHullShape*)pSubShape;//Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_ConvexHullShape));
					if(CollideConvexHullTriangle<_MaxContacts>(pConvexHull->m_Hull, aVex, bVex, cVex, &patch, physicsMaterial, effectMaterial))
					{
						newCollide = true;
						//Collision::ReduceContacts<MaxPatches, MaxContacts, 3, 6>(results);
					}
					break;
				}
			case E_Sphere_Shape:
				{
					Collision::C_SphereShape* pSphere= (C_SphereShape*)pSubShape;//Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_SphereShape));
					if(CollideSphereTriangle<_MaxContacts>(pSphere->m_Sphere, aVex, bVex, cVex, patch, (U32)physicsMaterial, (U32)effectMaterial))
					{
						newCollide = true;
						//Collision::ReduceContacts<MaxPatches, MaxContacts, 3, 6>(results);
					}
					break;
				}

			case E_Capsule_Shape:
				{
					Collision::C_CapsuleShape* pCapsule = (C_CapsuleShape*)pSubShape;//Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_CapsuleShape));
					if(CollideCapsuleTriangle<_MaxContacts>(aVex, bVex, cVex, patch.m_Normal, pCapsule->m_Capsule, patch, physicsMaterial, effectMaterial, 0, 0))
					{
						newCollide = true;
						//Collision::ReduceContacts<MaxPatches, MaxContacts, 3, 6>(results);
					}
					break;
				}

			case E_Cylinder_Shape:
				{
					Collision::C_CylinderShape* pCylinder = (C_CylinderShape*)pSubShape;//Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_CylinderShape));
					if(CollideCylinderTriangle(pCylinder->m_Cylinder, aVex, bVex, cVex, patch, 0, physicsMaterial, effectMaterial))
					{
						newCollide = true;
						//Collision::ReduceContacts<MaxPatches, MaxContacts, 3, 6>(results);
					}
					break;
				}
			case E_Composite_Shape:
				{
					//DebugBreak();
					collided = Collision::CollideCompositeShapeTriangle<MaxContacts>(pSubShape, aVex, bVex, cVex, patch, physicsMaterial, effectMaterial, collisionFlags) || collided; 
					break;
				}
			case E_Cone_Shape:
				{
					Collision::C_ConeShape* pCone = (C_ConeShape*)Spu::FetchAlloc(pSubShape, sizeof(C_ConeShape));
					//if(CollideConeTriangle(pCone->m_Cone, aVex, bVex, cVex, patch, 0, physicsMaterial, effectMaterial))
					//{
					//	newCollide = true;
					//	//Collision::ReduceContacts<MaxPatches, MaxContacts, 3, 6>(results);
					//}
					break;
				}
			}
		}
		if(newCollide)
		{
			collided = true;
			newCollide = false;
			Collision::ReducePatch<MaxContacts, 4>(patch);
		}

	}
	return collided;
}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideCompositeShapeTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 _physicsMaterail, const U32 _effectMaterial,
									   const U32 collisionFlags, const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;
	//C_CompositeShape* pCShape((C_CompositeShape*)Spu::FetchAlloc(pShape, sizeof(C_CompositeShape)));
	C_CompositeShape* pCShape = (C_CompositeShape*)pShape;

	Collision::C_CRAxisBox& box(pCShape->m_LocalBoundingBox);
	const HMat33 rotation(pCShape->m_PosRot.Rotation());
	const HVec3 position(pCShape->m_PosRot.Translation() + box.centre * rotation);
	
	const HVec3 range(high - low);
	bool collided = false;

	const U32 physMat = _physicsMaterail & 0xffff0000;
	const U32 effMat = _effectMaterial & 0xffff0000;

	Collision::C_CompositeShapeIterator iter(pCShape);

	for(U32 a = 0; a < numTris; ++a)
	{

		{
			Spu::MemBaseMarker marker;
			/*const Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + a, sizeof(Triangle));
			MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
			const Maths::HVec3 vertA(Maths::Load<Maths::HVec3>(f0));
			MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
			const Maths::HVec3 vertB(Maths::Load<Maths::HVec3>(f1));
			MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));
			const Maths::HVec3 vertC(Maths::Load<Maths::HVec3>(f2));*/

			const Triangle* tri = &tris[a];
			const HVec3 localA(&verts[tri->m_IndexA ]);
			const HVec3 localB(&verts[tri->m_IndexB ]);
			const HVec3 localC(&verts[tri->m_IndexC ]);
			const HVec3 vertA = localA*range + low;
			const HVec3 vertB = localB*range + low;
			const HVec3 vertC = localC*range + low;

			//Test triangle vs bounding box...
			if(!CollideConvexOBBTriangle(vertA, vertB, 
				vertC, position, box.extents,
					rotation))
					continue;

		}
		

		//C_CollisionResult<MaxPatches, MaxContacts> tempResults;
		bool newCollide = false;
		
		//for(U32 i=0; i< pCShape->m_NumShapes; ++i)
		iter.Reset();
		while(iter.HasNextShape())
		{
			Spu::MemBaseMarker marker;

			iter.NextShape();
			/*C_ShapeWrapper* wrapper=(C_ShapeWrapper*)Spu::FetchAlloc(pCShape->m_pShapes + i, sizeof(C_ShapeWrapper));
			C_Shape* pShape = (C_Shape*)Spu::FetchAlloc(wrapper->m_pShape, sizeof(C_Shape));*/
			C_Shape* pSubShape = iter.GetCurrentShape<C_Shape>();
			E_ShapeType type=pSubShape->m_ShapeType;
			const U32 physicsMaterail = physMat | pSubShape->m_PhysicsMaterialIndex;
			const U32 effectMaterial = effMat | pSubShape->m_EffectMaterialIndex;

			if(!pSubShape->CanCollide(collisionFlags))
				continue;
			switch(type)
			{
			case E_Box_Shape:
				{
					newCollide = CollideOBBTriangleMesh<MaxPatches, MaxContacts, Triangle>(pSubShape, verts, normals, tris + a, 1, triangleStartIndex + a, results, physicsMaterail, effectMaterial, collisionFlags, low, high);
					break;
				}
			case E_ConvexHull_Shape:
				{
					newCollide = CollideConvexHullTriangleMesh<MaxPatches, MaxContacts, Triangle> (pSubShape, verts, normals, tris + a, 1, triangleStartIndex + a, results, physicsMaterail, effectMaterial, collisionFlags, low, high);
					break;
				}
			case E_Sphere_Shape:
				{
					newCollide = CollideSphereTriangleMesh<MaxPatches, MaxContacts, Triangle>(pSubShape, verts, normals, tris + a, 1, triangleStartIndex + a, results, physicsMaterail, effectMaterial, collisionFlags, low, high);
					break;
				}

			case E_Capsule_Shape:
				{
					newCollide = CollideCapsuleTriangleMesh<MaxPatches, MaxContacts, Triangle>(pSubShape, verts, normals, tris + a, 1, triangleStartIndex + a, results, physicsMaterail, effectMaterial, collisionFlags, low, high);
					break;
				}

			case E_Cylinder_Shape:
				{
					newCollide = CollideCylinderTriangleMesh<MaxPatches, MaxContacts, Triangle>(pSubShape, verts, normals, tris + a, 1, triangleStartIndex + a, results,physicsMaterail, effectMaterial, collisionFlags, low, high);
					break;
				}
			case E_Cone_Shape:
				{

					newCollide = CollideConeTriangleMesh<MaxPatches, MaxContacts, Triangle>(pSubShape, verts, normals, tris + a, 1, triangleStartIndex + a, results,physicsMaterail, effectMaterial, collisionFlags, low, high);
					break;

				}
			case E_Composite_Shape:
				collided = Collision::CollideCompositeShapeTriangleMesh<MaxPatches, MaxContacts, Triangle>(
					pSubShape, verts, normals, tris + a, 1, triangleStartIndex + a, results, physicsMaterail, effectMaterial, collisionFlags, low, high) || collided; 
				break;
			}
		
			if(newCollide)
			{
				Collision::ReduceContacts<MaxPatches, MaxContacts, _DesiredPatches, _DesiredContacts>(results);
				collided = true;
				newCollide = false;
			}
		}
	}

	//Collision::ReduceContacts<MaxPatches, MaxContacts, 3, 6>(results);
	
	return collided;
}
};

#endif
