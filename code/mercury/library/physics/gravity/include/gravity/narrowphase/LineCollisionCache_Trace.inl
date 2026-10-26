#ifndef LINE_COLLISION_CACHE_TRACE_INL
#define LINE_COLLISION_CACHE_TRACE_INL

#include "gravity/narrowphase/LineCollisionCache.h"
#include "gravity/narrowphase/Ray.h"
#include "gravity/narrowphase/Primitive.inl"
#include "gravity/narrowphase/shape.h"
#include "gravity/narrowphase/shapes.h"
#include "gravity/narrowphase/CollisionFilter.inl"
#include "gravity/narrowphase/RayTriangle.inl"
#include <system/perf.h>


namespace Collision
{

	bool C_LineCollisionCache::Trace(Collision::C_Ray& ray, C_SingleTriangleCache* pHitTriangle)
	{
		//PERF_EVENT("Gravity.TraceCache");
		using namespace Maths;
		const HF32 zero(Constants<HF32>::Zero());

		Maths::HF32 tMin(Maths::UNDEFINED);

		if(pHitTriangle && pHitTriangle->m_bHit)
		{
			//Test against the hit triangle...
			C_CachedTriangle& tri = pHitTriangle->m_Triangle;
			if(ray.m_Filter.CanCollide(tri.m_CollisionFlags))
			{

				const Maths::HF32 _dist((tri.m_Normal|tri.m_VertexA) + ray.m_Radius);
				const Maths::HF32 disStart((ray.m_start|tri.m_Normal) - _dist);
				const Maths::HF32 disEnd((ray.m_end|tri.m_Normal) - _dist);
				if((disStart*disEnd).LE(zero) & (tri.m_Normal | ray.m_Direction).LE(ray.m_MaxCosAngle))
				{
					Maths::HVec3 contact(Maths::UNDEFINED);
					Maths::HF32 distance(Maths::UNDEFINED);
					Maths::C_Normal resultNormal(Maths::UNDEFINED);
					if(ray.m_bFatTrace)
					{
						C_RayCastTriangle triangle(tri.m_VertexA, tri.m_VertexB, tri.m_VertexC, tri.m_Normal);
						if(IntersectTriangleSegmentLite(triangle, ray.m_start, ray.m_end, 
							ray.m_Direction, ray.m_Radius, contact, distance, resultNormal))
						{
							ray.m_result.m_contact = contact;
							ray.m_result.m_normal = resultNormal;
							ray.m_result.m_distance = distance;
							ray.m_result.m_TriangleIndex = tri.m_TriangleIndex;
							ray.m_result.m_TriangleNormal = tri.m_Normal;
							ray.m_result.m_CollisionFlags = tri.m_CollisionFlags;
							ray.m_result.m_PhysicsMaterialIndex = tri.m_PhysicsMaterialFlags;
							ray.m_result.m_EffectMaterialIndex = tri.m_EffectMaterialFlags;
							ray.m_result.m_UserData = tri.m_UserData;
							ray.m_end = contact;
							ray.m_result.m_pShape = tri.m_pShape;
							ray.m_result.m_pNode = tri.m_pGraphNode;
							return true;
						}
					}
					else
					{
						const HVec3 vertA(tri.m_VertexA);
						const HVec3 vertB(tri.m_VertexB);
						const HVec3 vertC(tri.m_VertexC);

						if(IntersectTriangleSegmentSB(vertA, vertB, vertC, 
							ray.m_start, ray.m_result.m_contact, contact))
						{
							ray.m_result.m_contact = contact;
							ray.m_result.m_normal = tri.m_Normal;
							ray.m_result.m_distance = (ray.m_start - ray.m_result.m_contact).Length();
							ray.m_result.m_TriangleIndex = tri.m_TriangleIndex;
							ray.m_result.m_TriangleNormal = tri.m_Normal;
							ray.m_result.m_CollisionFlags = tri.m_CollisionFlags;
							ray.m_result.m_PhysicsMaterialIndex = tri.m_PhysicsMaterialFlags;
							ray.m_result.m_EffectMaterialIndex = tri.m_EffectMaterialFlags;
							ray.m_result.m_UserData = tri.m_UserData;
							ray.m_end = contact;
							ray.m_result.m_pShape = tri.m_pShape;
							ray.m_result.m_pNode = tri.m_pGraphNode;
							return true;
						}
					}
				}
			}
			pHitTriangle->m_bHit = false;
		}



		bool bHit = false;
		if(m_pPlaneShape)
		{
			Spu::MemBaseMarker _planeMarker;
			Collision::C_PlaneShape* pPlaneShape = (Collision::C_PlaneShape*)Spu::FetchAlloc(m_pPlaneShape, sizeof(Collision::C_PlaneShape));
			bHit = IntersectRayPlane(ray, pPlaneShape);
		}

		//if(g_UseCollisionCache)
		{
			HVec3 contact(UNDEFINED);
			S32 hitIndex(-1);

			if(ray.m_bFatTrace)
			{
				const Maths::HVec3 radius(ray.m_Radius);
				bool collide = false;
				const Maths::HF32 disEps(SharedPhysics::Constants::CloseNormal());
				Maths::HVec3 temp(ray.m_result.m_contact);
				Maths::HVec3 contact(ray.m_result.m_contact);
				Maths::C_Normal normal(ray.m_result.m_normal), resultNormal(Maths::UNDEFINED);
				Maths::C_Normal triNormal(ray.m_result.m_TriangleNormal);
				U32 triangleIndex(0);
				Maths::HF32 dis(ray.m_result.m_distance), distance(ray.m_result.m_distance);
				U32 triIndex = 0;

				//for(a = m_Low; a != m_High; a = (a + 1)%m_CacheSize)
				for(U32 a = m_Low; a < m_High; a ++)
				{
					C_CachedTriangle& tri = m_pTriangles[a];
					if(ray.m_Filter.CanCollide(tri.m_CollisionFlags))
					{

						const Maths::HF32 _dist((tri.m_Normal|tri.m_VertexA));
						const Maths::HF32 disStart((ray.m_start|tri.m_Normal) - _dist);
						const Maths::HF32 disEnd((ray.m_end|tri.m_Normal) - _dist);

						const Maths::HVec3 vMin(Maths::Min(tri.m_VertexA, Maths::Min(tri.m_VertexB, tri.m_VertexC)) - radius);
						const Maths::HVec3 vMax(Maths::Max(tri.m_VertexA, Maths::Max(tri.m_VertexB, tri.m_VertexC)) + radius);
						const Maths::HF32 cosAngle(tri.m_Normal | ray.m_Direction);


						if((((disStart*disEnd).GT(zero) & Abs(disStart).GT(ray.m_Radius) & 
							Abs(disEnd).GT(ray.m_Radius)) | 
							cosAngle.GT(ray.m_MaxCosAngle))
							|(!Collision::IntersectRayAABBMinMax(ray.m_start, ray.m_end, vMin, vMax, tMin)))
							continue;

						C_RayCastTriangle triangle(tri.m_VertexA, tri.m_VertexB, tri.m_VertexC, tri.m_Normal);
						const Maths::Scalar_Bool bScal(IntersectTriangleSegmentSB(triangle, ray.m_start, ray.m_end, 
							ray.m_Direction, ray.m_Radius, contact, dis, resultNormal));

						const Maths::Scalar_Bool bEarlier(bScal & dis.LT(distance));
						temp = Maths::Select(bEarlier, contact, temp);
						distance = Maths::Select(bEarlier, dis, distance);
						triNormal = Maths::Select(bEarlier, tri.m_Normal, triNormal);
						dis = distance; // new swept sphere test needs this
						bool bCon(bEarlier);
						if(bCon)
						{
							normal = resultNormal;
							triangleIndex = tri.m_TriangleIndex;
							triIndex = a;
							collide = true;
						}
					}
				}
				if(collide)
				{
					ray.m_result.m_contact = temp;
					ray.m_result.m_normal = normal;
					ray.m_result.m_distance = distance;
					ray.m_result.m_TriangleIndex = triangleIndex;
					ray.m_result.m_TriangleNormal = triNormal;
					C_CachedTriangle& tri = m_pTriangles[triIndex];
					ray.m_result.m_CollisionFlags = tri.m_CollisionFlags;
					ray.m_result.m_PhysicsMaterialIndex = tri.m_PhysicsMaterialFlags;
					ray.m_result.m_EffectMaterialIndex = tri.m_EffectMaterialFlags;
					ray.m_result.m_UserData = tri.m_UserData;
					ray.m_end = ray.m_result.m_contact;
					ray.m_result.m_pShape = tri.m_pShape;
					ray.m_result.m_pNode = tri.m_pGraphNode;
					if(pHitTriangle)
					{
						pHitTriangle->m_Triangle = tri;
						pHitTriangle->m_bHit = true;
					}
					return true;
				}
			}
			else
			{
				//for(U32 a = m_Low; a != m_High; a = (a + 1)%m_CacheSize)
				for(U32 a = m_Low; a < m_High; ++a)
				{
					C_CachedTriangle& tri = m_pTriangles[a];
					if(ray.m_Filter.CanCollide(tri.m_CollisionFlags))
					{
						const Maths::HF32 _dist((tri.m_Normal|tri.m_VertexA) + ray.m_Radius);
						const Maths::HF32 disStart((ray.m_start|tri.m_Normal) - _dist);
						const Maths::HF32 disEnd((ray.m_end|tri.m_Normal) - _dist);
						if((disStart*disEnd).GT(zero))
							continue;
						//Trace it to see if we hit it!
						const HVec3 vertA(tri.m_VertexA);
						const HVec3 vertB(tri.m_VertexB);
						const HVec3 vertC(tri.m_VertexC);

						const Scalar_Bool bb(IntersectTriangleSegmentSB(vertA, vertB, vertC, 
							ray.m_start, ray.m_result.m_contact, contact));
						ray.m_result.m_contact = Maths::Select(bb, contact, ray.m_result.m_contact);
						hitIndex = Select(bool(bb), (int)a, hitIndex);
					}
				}
			}
			if(hitIndex >= 0)
			{
				C_CachedTriangle& tri = m_pTriangles[hitIndex];
				ray.m_result.m_normal = C_Normal(tri.m_Normal, C_Normal::UNIT_LENGTH_VECTOR);
				ray.m_result.m_distance = (ray.m_start - ray.m_result.m_contact).Length();
				ray.m_result.m_TriangleIndex = tri.m_TriangleIndex;
				ray.m_result.m_PhysicsMaterialIndex = tri.m_PhysicsMaterialFlags;
				ray.m_result.m_EffectMaterialIndex = tri.m_EffectMaterialFlags;
				ray.m_result.m_UserData = tri.m_UserData;
				ray.m_result.m_pNode = tri.m_pGraphNode;
				ray.m_result.m_pShape = tri.m_pShape;
				ray.m_end = ray.m_result.m_contact;
				if(pHitTriangle)
				{
					pHitTriangle->m_Triangle = tri;
					pHitTriangle->m_bHit = true;
				}
				return true;
			}
		}
		return bHit;
	}
}

#endif
