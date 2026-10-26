#ifndef __GRAVITY_RAYTRIANGLE_INL__
#define __GRAVITY_RAYTRIANGLE_INL__


#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/SweptTest.h"


namespace Collision
{

	inline bool IntersectRayPlane(Collision::C_Ray& ray, Collision::C_Shape* pShape)
	{
		Collision::C_PlaneShape* pPlane = (Collision::C_PlaneShape*)pShape;
		Maths::HVec3 hitPoint(Maths::UNDEFINED), planePoint(Maths::UNDEFINED);

		if(Collision::IntersectSegmentPlaneRadius(ray.m_start, ray.m_end, pPlane->m_Plane, ray.m_Radius,
				hitPoint, planePoint))
		{
			const Maths::HF32 distance((hitPoint - ray.m_start) | ray.m_Direction);
			ray.m_result.m_distance = distance;
			ray.m_result.m_TriangleNormal = pPlane->m_Plane.m_n;
			ray.m_result.m_PhysicsMaterialIndex = pPlane->m_PhysicsMaterialIndex;
			ray.m_result.m_EffectMaterialIndex = pPlane->m_EffectMaterialIndex;
			ray.m_end = hitPoint;
			ray.m_result.m_contact = planePoint;
			ray.m_result.m_pShape = pShape;
			if(ray.m_bFatTrace)
			{
				ray.m_result.m_normal = hitPoint - planePoint;
			}
			else
			{
				ray.m_result.m_normal = pPlane->m_Plane.m_n;
			}
			return true;
		}
		return false;
	}


	/*template <class Triangle>
	bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high)*/
	template <class Triangle>
	bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const verts, const MathsStorage::HenDN3 * const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;

		bool collide(false);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 disEps(SharedPhysics::Constants::CloseNormal());
		Maths::HVec3 temp(ray.m_result.m_contact);
		Maths::HVec3 contact(ray.m_result.m_contact);
		//MathsStorage::HenDN3 dummy;
		//MathsStorage::HenDN3* tempNormal = &dummy;
		MathsStorage::HenDN3 dNorm;
		Maths::C_Normal normal(ray.m_result.m_normal);
		U32 triangleIndex(0);
		const HVec3 range(high - low);
		const Maths::HVec3 rayStart(ray.m_start);
		const Maths::HVec3 rayDirection(ray.m_Direction);
		// [Blur2PC] Construct the full-width mask; scalar VBool is only one byte.
		const Maths::Scalar_Bool doubleSided(ray.m_bDoubleSided);
		const Maths::HF32 maxCosAngle(ray.m_MaxCosAngle);

		for(U32 i=0; i<numTris; ++i)
		{
			Spu::MemBaseMarker marker;
			//const Triangle* tempTri=(const Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
			const Triangle* tempTri = &tris[i];
			//((U32&)dNorm) = Spu::Get( (U32*)normals + tempTri->m_NormalIndex);
			dNorm = tempTri->m_SurfaceNormal;
			const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);

			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);
			const HVec3 a = localA*range + low;
			const HVec3 _b = localB*range + low;
			const HVec3 _c = localC*range + low;
			
			const Maths::HF32 _cosAngle(_norm | rayDirection);
			Maths::Scalar_Bool con (doubleSided & _cosAngle.GT(zero));
			Maths::C_Normal norm(Maths::Select(con, -_norm, _norm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			const Maths::HF32 cosAngle(Select(con, -_cosAngle, _cosAngle));

			const HVec3 b = Select(con, _c, _b);
			const HVec3 c = Select(con, _b, _c);

			const Maths::Scalar_Bool bScal(cosAngle.LT(maxCosAngle) & IntersectTriangleSegmentSB(a, b, c, rayStart, temp, contact));

			const bool bCon(bScal);
			temp = Maths::Select(bScal, contact, temp);
			normal = Maths::C_Normal(Maths::Select(bScal, norm, normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			triangleIndex = Maths::Select(bCon, i, triangleIndex);
			collide |= bCon;
		}
		if(collide)
		{
			ray.m_result.m_contact = temp;
			ray.m_result.m_normal = normal;
			ray.m_result.m_distance = (temp - ray.m_start) | rayDirection;
			ray.m_result.m_TriangleIndex = triangleStartIndex + sizeof(Triangle) * triangleIndex;
			ray.m_result.m_TriangleNormal = normal;
			ray.m_result.m_CollisionFlags = collisionFlags;
			ray.m_result.m_PhysicsMaterialIndex = physicsMaterial;
			ray.m_result.m_EffectMaterialIndex = effectMaterial;
			ray.m_result.m_UserData = userData;
			ray.m_end = ray.m_result.m_contact;
			return true;
		}
		return false;
	}

		template <class Triangle>
	bool CollideFatRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_Ray& ray, const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;
		bool collide(false);
		const HF32 zero(Constants<HF32>::Zero());
		const HF32 disEps(SharedPhysics::Constants::CloseNormal());
		const HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		HVec3 temp(ray.m_result.m_contact);
		HVec3 contact(ray.m_result.m_contact);
		C_Normal normal(ray.m_result.m_normal), resultNormal(Maths::UNDEFINED);
		U32 triangleIndex(0);
		HF32 dis(ray.m_result.m_distance), distance(ray.m_result.m_distance), tMin(Maths::UNDEFINED);
		MathsStorage::HenDN3 dNorm;
		const HVec3 range(high - low);
		const Maths::HVec3 radius(ray.m_Radius);
		const Maths::HF32 rayRadius(ray.m_Radius);

		const HVec3 rayStart(ray.m_start);
		const HVec3 rayEnd(ray.m_result.m_contact);
		const Maths::C_Normal rayDirection(ray.m_Direction);
		//const Maths::Scalar_Bool doubleSided(ray.m_bDoubleSided ? Maths::Constants<Maths::Scalar_Bool>::True() : Maths::Constants<Maths::Scalar_Bool>::False());
		// [Blur2PC] Construct the full-width mask; scalar VBool is only one byte.
		const Maths::Scalar_Bool doubleSided(ray.m_bDoubleSided);
		const Maths::HF32 maxCosAngle(ray.m_MaxCosAngle);

		for(U32 i=0; i<numTris; ++i)
		{
			Spu::MemBaseMarker marker;
			const Triangle* tempTri = &tris[i];
			dNorm = tempTri->m_SurfaceNormal;
			const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
						
			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);

			const HVec3  a(localA*range + low);
			const HVec3 _b(localB*range + low);
			const HVec3 _c(localC*range + low);
			
			const Maths::HVec3 vMin(Maths::Min(a, Maths::Min(_b, _c)) - radius);
			const Maths::HVec3 vMax(Maths::Max(a, Maths::Max(_b, _c)) + radius);

			const HF32 _dis((a|_norm)/* + ray.m_Radius*/);
			const HF32 disStart((rayStart | _norm) - _dis);

			const Maths::HF32 _cosAngle(_norm|rayDirection);

			const Maths::Scalar_Bool con ( doubleSided & _cosAngle.GT(zero));
			const Maths::HF32 cosAngle(Select(con, -_cosAngle, _cosAngle));

			const HF32 disEnd((rayEnd | _norm) - _dis);

			const HVec3 b(Select(con, _c, _b));
			const HVec3 c(Select(con, _b, _c));

			const Maths::C_Normal norm(Maths::Select(con, -_norm, _norm), Maths::C_Normal::UNIT_LENGTH_VECTOR);

			const Maths::Scalar_Bool bCrossPlane((disStart * disEnd).LE(zero) | 
				Abs(disStart).LE(rayRadius) | Abs(disEnd).LE(rayRadius));
			const Maths::Scalar_Bool bHitBox((cosAngle.LE(maxCosAngle)) & 
				Collision::IntersectRayAABBMinMax(rayStart, rayEnd,
				vMin, vMax));

			if(bCrossPlane & bHitBox)
			{
				C_RayCastTriangle tri(a, b, c, norm);
				const Maths::Scalar_Bool bScal(IntersectTriangleSegmentSB(tri, rayStart, temp, 
 					rayDirection, ray.m_Radius, contact, dis, resultNormal));
				const Maths::Scalar_Bool bEarlier(bScal & dis.LT(distance));
				temp = Maths::Select(bEarlier, rayStart + rayDirection * dis, temp);
				distance = Maths::Select(bEarlier, dis, distance);
				dis = distance; // new swept sphere test needs this
				bool bCon(bEarlier);
				normal = Maths::Select(bEarlier, resultNormal, normal);
				triangleIndex = Maths::Select(bCon, i, triangleIndex);
				collide |= bCon;
			}
		}
		if(collide)
		{
			ray.m_result.m_contact = temp;
			ray.m_result.m_normal = normal;
			ray.m_result.m_distance = distance;
			ray.m_result.m_TriangleIndex = triangleStartIndex + sizeof(Triangle) * triangleIndex;
			const Triangle* tempTri = tris + triangleIndex;
			dNorm = tempTri->m_SurfaceNormal;
			ray.m_result.m_TriangleNormal = Maths::C_Normal(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			ray.m_result.m_CollisionFlags = collisionFlags;
			ray.m_result.m_PhysicsMaterialIndex = physicsMaterial;
			ray.m_result.m_EffectMaterialIndex = effectMaterial;
			ray.m_result.m_UserData = userData;
			ray.m_end = ray.m_start + ray.m_Direction * distance;
		}
		return collide;
	}


	inline bool IntersectRayPlane(Collision::C_TransparentRay& ray, Collision::C_Shape* pShape)
	{
		Collision::C_PlaneShape* pPlane = (Collision::C_PlaneShape*)pShape;
		Maths::HVec3 hitPoint(Maths::UNDEFINED), planePoint(Maths::UNDEFINED);

		if(Collision::IntersectSegmentPlaneRadius(ray.m_start, ray.m_end, pPlane->m_Plane, ray.m_Radius,hitPoint, planePoint))
		{
			Collision::C_RayTracingResult result;
			const Maths::HF32 distance((hitPoint - ray.m_start) | ray.m_Direction);
			result.m_distance = distance;
			result.m_TriangleNormal = pPlane->m_Plane.m_n;
			result.m_PhysicsMaterialIndex = pPlane->m_PhysicsMaterialIndex;
			result.m_EffectMaterialIndex = pPlane->m_EffectMaterialIndex;
			result.m_contact = planePoint;
			result.m_pShape = pShape;
			if(ray.m_bFatTrace)
			{
				result.m_normal = hitPoint - planePoint;
			}
			else
			{
				result.m_normal = pPlane->m_Plane.m_n;
			}

			ray.StoreResult(result);
			return true;
		}
		return false;
	}


	/*
		Transparent normal ray tracing
	*/
	template <class Triangle>
	bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
		const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;

		bool collide(false);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 disEps(SharedPhysics::Constants::CloseNormal());
		MathsStorage::HenDN3 dNorm;
		Maths::HVec3 contact(Maths::UNDEFINED);

		const Maths::HVec3 rayStart(ray.m_start);
		const Maths::HVec3 rayEnd(ray.m_end);
		const Maths::HVec3 rayDirection(ray.m_Direction);
		// [Blur2PC] Construct the full-width mask; scalar VBool is only one byte.
		const Maths::Scalar_Bool doubleSided(ray.m_bDoubleSided);
		const Maths::HF32 maxCosAngle(ray.m_MaxCosAngle);

		const HVec3 range(high - low);
		for(U32 i=0; i<numTris; ++i)
		{
			const Triangle* tempTri = &tris[i];
			dNorm = tempTri->m_SurfaceNormal;
			const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);

			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);
			const HVec3 a = localA*range + low;
			const HVec3 _b = localB*range + low;
			const HVec3 _c = localC*range + low;
			const Maths::HF32 _cosAngle(_norm | rayDirection);
			const Scalar_Bool con(doubleSided & _cosAngle.GT(zero));
			C_Normal norm(Select(con, -_norm, _norm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			const Maths::HF32 cosAngle(Select(con, -_cosAngle, _cosAngle));
			const HVec3 b = Select(con, _c, _b);
			const HVec3 c = Select(con, _b, _c);

			if(cosAngle.LE(maxCosAngle) & IntersectTriangleSegmentSB(a, b, c, rayStart, rayEnd, contact))
			{
				Collision::C_RayTracingResult result;
				result.m_contact = contact;
				result.m_normal = norm;
				result.m_distance = (contact - rayStart) | rayDirection;
				result.m_TriangleIndex = triangleStartIndex + sizeof(Triangle) * i;
				result.m_TriangleNormal = norm;
				result.m_CollisionFlags = collisionFlags;
				result.m_PhysicsMaterialIndex = physicsMaterial;
				result.m_EffectMaterialIndex = effectMaterial;
				result.m_pNode= pGraphNode;
				result.m_pShape = pShape;
				result.m_UserData = userData;
				ray.StoreResult(result);
				collide = true;
			}
		}
		
		return collide;
	}


	template <class Triangle>
	bool CollideRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const  verts, const MathsStorage::HenDN3 * const  normals, const Triangle* const  tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
		const Maths::HVec3& low, const Maths::HVec3& high, const Maths::HMat43& matrix)
	{
		using namespace Maths;

		bool collide(false);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 disEps(SharedPhysics::Constants::CloseNormal());
		MathsStorage::HenDN3 dNorm;
		Maths::HVec3 contact(Maths::UNDEFINED);

		const Maths::HMat33 rotation(matrix.Rotation());
		const Maths::HVec3 rayStart(ray.m_start);
		const Maths::HVec3 rayEnd(ray.m_end);
		const Maths::HVec3 rayDirection(ray.m_Direction);
		// [Blur2PC] Construct the full-width mask; scalar VBool is only one byte.
		const Maths::Scalar_Bool doubleSided(ray.m_bDoubleSided);
		const Maths::HF32 maxCosAngle(ray.m_MaxCosAngle);

		const HVec3 range(high - low);
		for(U32 i=0; i<numTris; ++i)
		{
			const Triangle* tempTri = &tris[i];
			dNorm = tempTri->m_SurfaceNormal;
			const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);

			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);
			const HVec3 a = localA*range + low;
			const HVec3 _b = localB*range + low;
			const HVec3 _c = localC*range + low;
			const Maths::HF32 _cosAngle(_norm | rayDirection);
			const Scalar_Bool con(doubleSided & _cosAngle.GT(zero));
			const Maths::HF32 cosAngle(Select(con, -_cosAngle, _cosAngle));
			C_Normal norm(Select(con, -_norm, _norm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			const HVec3 b = Select(con, _c, _b);
			const HVec3 c = Select(con, _b, _c);

			if(cosAngle.LE(maxCosAngle) & IntersectTriangleSegmentSB(a, b, c, rayStart, rayEnd, contact))
			{
				Collision::C_RayTracingResult result;
				const Maths::HVec3 worldContact(contact * matrix);
				result.m_contact = worldContact;
				const Maths::C_Normal transNorm(norm * rotation);
				result.m_normal = transNorm;
				result.m_distance = (worldContact - rayStart) | rayDirection;
				result.m_TriangleIndex = triangleStartIndex + sizeof(Triangle) * i;
				result.m_TriangleNormal = transNorm;
				result.m_CollisionFlags = collisionFlags;
				result.m_PhysicsMaterialIndex = physicsMaterial;
				result.m_EffectMaterialIndex = effectMaterial;
				result.m_pNode= pGraphNode;
				result.m_pShape = pShape;
				result.m_UserData = userData;
				ray.StoreResult(result);
			}
		}
		
		return collide;
	}



	/*
		Transparent fat ray tracing
	*/


		template <class Triangle>
	bool CollideFatRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
		const Maths::HVec3& low, const Maths::HVec3& high)
	{
		using namespace Maths;
		bool collide(false);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 disEps(SharedPhysics::Constants::CloseNormal());
		Maths::HVec3 contact(Maths::UNDEFINED);
		HF32 dis(Maths::UNDEFINED), tMin(Maths::UNDEFINED);
		Maths::C_Normal resultNormal(Maths::UNDEFINED);
		MathsStorage::HenDN3 dNorm;
		const HVec3 range(high - low);
		const Maths::HVec3 radius(ray.m_Radius + Maths::Constants<Maths::HF32>::Epsilon());
		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());
		// [Blur2PC] Construct the full-width mask; scalar VBool is only one byte.
		const Maths::Scalar_Bool doubleSided(ray.m_bDoubleSided);
		const Maths::HF32 rayRadius(ray.m_Radius);
		const Maths::HF32 maxCosAngle(ray.m_MaxCosAngle);
		const Maths::HVec3 rayStart(ray.m_start);
		const Maths::HVec3 rayEnd(ray.m_end);
		const Maths::C_Normal rayDirection(ray.m_Direction);


		for(U32 i=0; i<numTris; ++i)
		{
			Spu::MemBaseMarker marker;
			const Triangle* tempTri = &tris[i];
			dNorm = tempTri->m_SurfaceNormal;
			const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			
			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);
			const HVec3 a = localA*range + low;
			const HVec3 _b(localB*range + low);
			const HVec3 _c(localC*range + low);

			const Maths::HF32 _dis((a | _norm));
			const Maths::HF32 disStart((rayStart | _norm) - _dis);

			const Maths::HF32 disEnd((rayEnd | _norm) - _dis);

			const Maths::HF32 _cosAngle(_norm | rayDirection);

			const Scalar_Bool con(doubleSided & _cosAngle.GT(zero));
			const Maths::HF32 cosAngle(Select(con, -_cosAngle, _cosAngle));
			Maths::C_Normal norm(Maths::Select(con, -_norm, _norm), Maths::C_Normal::UNIT_LENGTH_VECTOR);

			const HVec3 b = Select(con, _c, _b);
			const HVec3 c = Select(con, _b, _c);

			const Maths::HVec3 vMin(Maths::Min(a, Maths::Min(b, c)) - radius);
			const Maths::HVec3 vMax(Maths::Max(a, Maths::Max(b, c)) + radius);
			
			
			const Maths::Scalar_Bool bCrossPlane((disStart * disEnd).LE(zero) | 
				Abs(disStart).LE(rayRadius) | Abs(disEnd).LE(rayRadius));
			const Maths::Scalar_Bool bHitBox((cosAngle.LE(maxCosAngle)) & 
				Collision::IntersectRayAABBMinMax(rayStart, rayEnd,
				vMin, vMax));

			
			/*if(Collision::IntersectRayAABBMinMax(rayStart, rayEnd,
				vMin, vMax, tMin))*/
			if(bCrossPlane & bHitBox)
			{
				C_RayCastTriangle tri(a, b, c, norm);

				if(IntersectTriangleSegmentSB(tri, rayStart, rayEnd, 
 					rayDirection, rayRadius, contact, dis, resultNormal))
				{
					Collision::C_RayTracingResult result;
					result.m_contact = rayStart + rayDirection * dis;
					result.m_normal = resultNormal;
					result.m_distance = dis;
					result.m_TriangleIndex = triangleStartIndex + sizeof(Triangle) * i;
					result.m_TriangleNormal = norm;
					result.m_CollisionFlags = collisionFlags;
					result.m_PhysicsMaterialIndex = physicsMaterial;
					result.m_EffectMaterialIndex = effectMaterial;
					result.m_pNode = pGraphNode;
					result.m_pShape = pShape;
					result.m_UserData = userData;
					ray.StoreResult(result);
					collide = true;
				}
			}
		}
		return collide;
	}

		template <class Triangle>
	bool CollideFatRayTriangleMesh(const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3 * const __restrict normals, const Triangle* const __restrict tris, const U32 triangleStartIndex, const U32 numTris, 
		const U32 collisionFlags, const U32 userData, const U16 physicsMaterial, const U16 effectMaterial, C_TransparentRay& ray, Collision::C_Shape* pShape, SharedPhysics::C_GraphNode* pGraphNode, 
		const Maths::HVec3& low, const Maths::HVec3& high, const Maths::HMat43& matrix)
	{
		using namespace Maths;
		bool collide(false);
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		const Maths::HF32 disEps(SharedPhysics::Constants::CloseNormal());
		Maths::HVec3 contact(Maths::UNDEFINED);
		HF32 dis(Maths::UNDEFINED), tMin(Maths::UNDEFINED);
		Maths::C_Normal resultNormal(Maths::UNDEFINED);
		MathsStorage::HenDN3 dNorm;
		const HVec3 range(high - low);
		const Maths::HVec3 radius(ray.m_Radius + Maths::Constants<Maths::HF32>::Epsilon());
		const Maths::HF32 epsilon(Maths::Constants<Maths::HF32>::Epsilon());

		const Maths::HMat33 rotation(matrix.Rotation());
		// [Blur2PC] Construct the full-width mask; scalar VBool is only one byte.
		const Maths::Scalar_Bool doubleSided(ray.m_bDoubleSided);
		const Maths::C_Normal rayDirection(ray.m_Direction);
		const Maths::HVec3 rayStart(ray.m_start);
		const Maths::HVec3 rayEnd(ray.m_end);
		const Maths::HF32 rayRadius(ray.m_Radius);
		const Maths::HF32 maxCosAngle(ray.m_MaxCosAngle);

		for(U32 i=0; i<numTris; ++i)
		{
			Spu::MemBaseMarker marker;
			const Triangle* tempTri = &tris[i];
			dNorm = tempTri->m_SurfaceNormal;
			const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&dNorm), Maths::C_Normal::UNIT_LENGTH_VECTOR);
			
			const HVec3 localA(&verts[tempTri->m_IndexA ]);
			const HVec3 localB(&verts[tempTri->m_IndexB ]);
			const HVec3 localC(&verts[tempTri->m_IndexC ]);
			const HVec3 a (localA*range + low);
			const HVec3 _b(localB*range + low);
			const HVec3 _c(localC*range + low);

			const Maths::HF32 _dis((a | _norm));
			const Maths::HF32 disStart((rayStart | _norm) - _dis);

			const Maths::HF32 disEnd((rayEnd | _norm) - _dis);

			const Maths::HF32 _cosAngle(rayDirection | _norm);

			const Scalar_Bool con(doubleSided & _cosAngle.GT(zero));
			const Maths::HF32 cosAngle(Select(con, -_cosAngle, _cosAngle));

			Maths::C_Normal norm(Maths::Select(con, -_norm, _norm), Maths::C_Normal::UNIT_LENGTH_VECTOR);

			const HVec3 b = Select(con, _c, _b);
			const HVec3 c = Select(con, _b, _c);

			const Maths::HVec3 vMin(Maths::Min(a, Maths::Min(b, c)) - radius);
			const Maths::HVec3 vMax(Maths::Max(a, Maths::Max(b, c)) + radius);

			const Maths::Scalar_Bool bCrossPlane((disStart * disEnd).LE(zero) | 
				Abs(disStart).LE(rayRadius) | Abs(disEnd).LE(rayRadius));
			const Maths::Scalar_Bool bHitBox((cosAngle.LE(maxCosAngle)) & 
				Collision::IntersectRayAABBMinMax(rayStart, rayEnd,
				vMin, vMax));

			
			/*if(Collision::IntersectRayAABBMinMax(rayStart, rayEnd,
				vMin, vMax, tMin))*/
			if(bCrossPlane & bHitBox)
			{
			
				C_RayCastTriangle tri(a, b, c, norm);

				if(IntersectTriangleSegmentSB(tri, rayStart, rayEnd, 
 					rayDirection, rayRadius, contact, dis, resultNormal))
				{
					Collision::C_RayTracingResult result;
					result.m_contact = (rayStart + rayDirection * dis) * matrix;
					result.m_normal = resultNormal * rotation;
					result.m_distance = dis;
					result.m_TriangleIndex = triangleStartIndex + sizeof(Triangle) * i;
					result.m_TriangleNormal = norm * rotation;
					result.m_CollisionFlags = collisionFlags;
					result.m_PhysicsMaterialIndex = physicsMaterial;
					result.m_EffectMaterialIndex = effectMaterial;
					result.m_pNode = pGraphNode;
					result.m_pShape = pShape;
					result.m_UserData = userData;
					ray.StoreResult(result);
					collide = true;
				}
			}
		}
		return collide;
	}
};


#endif
