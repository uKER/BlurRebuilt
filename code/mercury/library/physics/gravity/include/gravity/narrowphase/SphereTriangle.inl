#ifndef __SPHERETRIANGLE_INL__
#define __SPHERETRIANGLE_INL__

#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/GJK.inl"

namespace Collision
{
template <int MaxPatches, int MaxContacts>
bool CollideSphereTriangle(C_Sphere& sphere, const Maths::HVec3& a, const Maths::HVec3& b, const Maths::HVec3& c, C_CollisionResult<MaxPatches, MaxContacts>& results)
{
	Maths::HVec3 contactPoint(Maths::UNDEFINED);
	ClosestPtPointTriangle(sphere.Centre(), a, b, c, contactPoint);
	
	const Maths::HVec3 d(sphere.Centre() - contactPoint);
	const Maths::HF32 dist(d.Length());

	if(dist.GT(sphere.Radius()))
		return false;

	const Maths::C_Normal normal(d);
	const Maths::HF32 penetrationDepth(sphere.Radius() - dist);

	C_ContactPatch<MaxContacts>& patch = results.m_ContactPatches[0];
	patch.m_Normal = normal;
	patch.m_NumContactPoints = 1;

	C_ContactPoint& point=patch.m_ContactPoints[0];
	point.m_ContactPoint = contactPoint;
	//point.m_PenetrationDepth = penetrationDepth;
	Maths::Store(penetrationDepth, &point.m_PenetrationDepth);

	return true;
}

template <int MaxContacts>
bool CollideSphereTriangle(C_Sphere& sphere, const Maths::HVec3& a, const Maths::HVec3& b, const Maths::HVec3& c, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial)
{
	using namespace Maths;
	HVec3 contactPoint(UNDEFINED);
	ClosestPtPointTriangle(sphere.Centre(), a, b, c, contactPoint);

	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactPoint, 0.5f, Maths::RGBA(255, 0, 0, 0));
	
	const HVec3 d(sphere.Centre() - contactPoint);
	const HF32 dist(d.Length());

	if(dist.GT(sphere.Radius()))
		return false;

	//const Maths::C_Normal normal(d);
	const HF32 penetrationDepth(sphere.Radius() - dist);

	//patch.m_Normal = normal;
	//patch.m_NumContactPoints = 1;

	C_ContactPoint& point=patch.m_ContactPoints[patch.m_NumContactPoints++];
	point.m_ContactPoint = contactPoint;
	//point.m_PenetrationDepth = penetrationDepth;
	Store(penetrationDepth, &point.m_PenetrationDepth);
	point.m_PhysicsMaterialIndices = physicsMaterial;
	point.m_EffectMaterialIndices = effectMaterial;

	return true;
}

template <int MaxContacts>
bool CollideSpherePlane(const C_Sphere& sphere, C_Plane& plane, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial)
{

	using namespace Maths;
	const HVec3 center = sphere.Centre();
	const HF32 dist = plane.Distance(center);
	const HF32 radius = sphere.Radius();
	if(dist.GT(radius))
		return false;

	//const Maths::C_Normal normal(d);
	const HF32 penetrationDepth(radius - dist);
	const HVec3 contactPoint = center - dist * plane.m_n;

	//patch.m_NumContactPoints = 1;
	C_ContactPoint& point=patch.m_ContactPoints[patch.m_NumContactPoints++];
	point.m_ContactPoint = contactPoint;
	Store(penetrationDepth, &point.m_PenetrationDepth);
	//point.m_PenetrationDepth = penetrationDepth;
	point.m_PhysicsMaterialIndices = physicsMaterial;
	point.m_EffectMaterialIndices = effectMaterial;

	return true;
}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideSphereTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, 
							   const MathsStorage::HenDN3* const __restrict normals, 
							   const Triangle* const __restrict tris, const U32 numTris, 
							   const U32 triangleStartIndex, 
							   C_CollisionResult<MaxPatches, MaxContacts>& results, 
							   const U32 physicsMaterial, const U32 effectMaterial, const U32 collisionFlags,
							   const Maths::HVec3& low,  const Maths::HVec3& high)
{
	//DebugBreak();
	using namespace Maths;

	const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	//C_SphereShape* pSphereShape((C_SphereShape*)Spu::FetchAlloc(pShape, sizeof(C_SphereShape)));
	C_SphereShape* pSphereShape((C_SphereShape*)pShape);
	const Collision::C_Sphere& sphere(pSphereShape->m_Sphere);

	Maths::HVec3 contactPoint(Maths::UNDEFINED);

	const Maths::HF32 epi(SharedPhysics::Constants::CloseNormal());
	
	bool collide=false;

	MathsStorage::HenDN3 dNorm;

	const Maths::HF32 r2(sphere.Radius() * sphere.Radius());

	const HVec3 range(high - low);

	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri=&tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		Maths::HVec3 norm = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);
		//assert((norm | norm) > zero);
		/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));
		const Maths::HVec3 a=Load<HVec3, MathsStorage::Float3>(f0);
		const Maths::HVec3 b=Load<HVec3, MathsStorage::Float3>(f1);
		const Maths::HVec3 c=Load<HVec3, MathsStorage::Float3>(f2);*/

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);

		const HVec3 a = localA*range + low;
		const HVec3 b = localB*range + low;
		const HVec3 c = localC*range + low;
		/*const HF32 dotA(a | norm);
		const HF32 dotCentre((sphere.Centre() | norm) - dotA);
		if(dotCentre > zero)
			continue;*/

		ClosestPtPointTriangle(sphere.Centre(), a, b, c, contactPoint);
		const Maths::HVec3 d(contactPoint - sphere.Centre());
		const Maths::HF32 distSq(d | d);

		if(distSq.GT(r2))// | ((d | norm).GT(zero)))
			continue;

		const Maths::C_Normal surfaceNormal(d);
		
		collide=true;

		C_ContactPatch<MaxContacts>* pPatch(NULL);
		U32 incr = 0;
		if(results.m_NumContactPatches < MaxPatches)
		{
			pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
			pPatch->m_NumContactPoints = 0;
			pPatch->m_Normal = surfaceNormal;//Maths::C_Normal(norm, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			incr = 1;
		}
		for(U32 j = 0; j < results.m_NumContactPatches; ++j)
		{
			Maths::HF32 dis(results.m_ContactPatches[j].m_Normal.m_V| surfaceNormal);
			if(dis >= epi)
			{
				pPatch = &results.m_ContactPatches[j];
				incr = 0;
				break;
			}
		}
		if(pPatch == NULL)
			continue;
		results.m_NumContactPatches += incr;
		//const Maths::HF32 pDepth(sphere.Radius() - dist.Sqrt());
		const Maths::HF32 pDepth(sphere.Radius() - (d | norm));

		if(pPatch->m_NumContactPoints < MaxContacts)
		{
			C_ContactPoint& point=pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
			point.m_ContactPoint = contactPoint;
			//point.m_PenetrationDepth=pDepth;
			Maths::Store(pDepth, &point.m_PenetrationDepth);
			point.m_HashValue = triangleStartIndex + i;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
		}
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactPoint, 0.5f, RGBA(0, 0, 255, 255));
	}
	
	return collide;

}

template <int MaxPatches, int MaxContacts, class Triangle>
bool CollideSphereTriangleMesh(C_SphereProxy& proxy, 
							   const MathsStorage::Fixed3_S16<14>* const __restrict verts, 
							   const MathsStorage::HenDN3* const __restrict normals, 
							   const Triangle* const __restrict tris, const U32 numTris, 
							   const U32 triangleStartIndex, 
							   C_CollisionResult<MaxPatches, MaxContacts>& results, 
							   const U32 physicsMaterial, const U32 effectMaterial,
							   const U32 collisionFlags,
							   const Maths::HVec3& low, const Maths::HVec3& high)
{
	using namespace Maths;

	const HF32 epi(SharedPhysics::Constants::CloseNormal());
	const HF32 zero(Maths::Constants<Maths::HF32>::Zero());
	const Collision::C_Sphere& sphere(proxy.m_Sphere);
	const HMat33 orientation(proxy.m_PosRot.Rotation());
	const HVec3 position(proxy.m_PosRot.Translation());
	

	HVec3 contactPoint(Maths::UNDEFINED);

	
	
	bool collide=false;

	MathsStorage::HenDN3 dNorm;

	const HF32 r2(sphere.Radius() * sphere.Radius());

	const HVec3 range(high - low);

	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri = &tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		Maths::HVec3 norm = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);
		//assert((norm | norm) > zero);
		/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));
		const Maths::HVec3 a=Load<HVec3, MathsStorage::Float3>(f0);
		const Maths::HVec3 b=Load<HVec3, MathsStorage::Float3>(f1);
		const Maths::HVec3 c=Load<HVec3, MathsStorage::Float3>(f2);*/

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);
		const HVec3 a(localA*range + low);
		const HVec3 b(localB*range + low);
		const HVec3 c(localC*range + low);

		const HF32 dotA(a | norm);
		const HF32 dotCentre((sphere.Centre() | norm) - dotA);
		if(dotCentre > zero)
			continue;

		ClosestPtPointTriangle(sphere.Centre(), a, b, c, contactPoint);
		const HVec3 d(contactPoint - sphere.Centre());
		const HF32 dist(d | d);

		if(dist.GT(r2))
			continue;

		const C_Normal untransformedSurfaceNormal(d);
		const C_Normal surfaceNormal(untransformedSurfaceNormal * orientation, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		
		collide=true;

		C_ContactPatch<MaxContacts>* pPatch(NULL);
		U32 incr = 0;
		if(results.m_NumContactPatches < MaxPatches)
		{
			pPatch = &results.m_ContactPatches[results.m_NumContactPatches];
			pPatch->m_NumContactPoints = 0;
			pPatch->m_Normal = surfaceNormal;//Maths::C_Normal(norm * orientation, Maths::C_Normal::UNIT_LENGTH_VECTOR);
			incr = 1;
		}
		for(U32 j = 0; j < results.m_NumContactPatches; ++j)
		{
			Maths::HF32 dis(results.m_ContactPatches[j].m_Normal.m_V| surfaceNormal);
			if(dis >= epi)
			{
				pPatch = &results.m_ContactPatches[j];
				incr = 0;
				break;
			}
		}
		if(pPatch == NULL)
			continue;
		results.m_NumContactPatches += incr;
		//const Maths::HF32 pDepth(sphere.Radius() - dist.Sqrt());
		const Maths::HF32 pDepth(sphere.Radius() - (d | untransformedSurfaceNormal));
		if(pPatch->m_NumContactPoints < MaxContacts)
		{
			C_ContactPoint& point=pPatch->m_ContactPoints[pPatch->m_NumContactPoints++];
			point.m_ContactPoint = contactPoint * orientation + position;
			//point.m_PenetrationDepth=pDepth;
			Store(pDepth, &point.m_PenetrationDepth);
			point.m_HashValue = triangleStartIndex + i;
			point.m_PhysicsMaterialIndices = physicsMaterial;
			point.m_EffectMaterialIndices = effectMaterial;
		}
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(contactPoint, 0.5f, RGBA(0, 0, 255, 255));
	}
	
	return collide;

}

template <class Triangle>
bool ContinuousSphereTriangleMesh(C_Shape* pShape, const MathsStorage::Fixed3_S16<14>* const __restrict verts, const MathsStorage::HenDN3* const __restrict normals, const Triangle* const __restrict tris, const U32 numTris, const U32 triangleStartIndex, 
								  const Maths::HVec3 PCREF r, Maths::HF32& maxLambda, Maths::HF32& lambda, Maths::HVec3& nor, Maths::HVec3& closestA, Maths::HVec3& closestB, 
								   const Maths::HVec3& low,  const Maths::HVec3& high)
{

	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	const HVec3 zeroV(Constants<HVec3>::Zero());
	const HF32 half(Constants<HF32>::Half());
	

	C_ContinuousShape* pContinuous = (C_ContinuousShape*)Spu::FetchAlloc(pShape, sizeof(C_ContinuousShape));
	C_SphereShape* pSphereShape((C_SphereShape*)Spu::FetchAlloc(pContinuous->m_pContinuousPrimitive, sizeof(C_SphereShape)));
	Collision::C_Sphere& sphere(pSphereShape->m_Sphere);
	const Maths::HF32 minusHalfRadius(-sphere.Radius());
	
	bool collide=false;

	MathsStorage::HenDN3 dNorm;
	const HVec3 range(high - low);

	for(U32 i=0; i<numTris; ++i)
	{
		Spu::MemBaseMarker marker;
		//Triangle* tri = (Triangle*)Spu::FetchAllocUnaligned(tris + i, sizeof(Triangle));
		const Triangle* tri =&tris[i];
		//((U32&)dNorm) = Spu::Get((U32*)(normals + tri->m_NormalIndex));
		dNorm = tri->m_SurfaceNormal;
		Maths::HVec3 norm = -Load<HVec3, MathsStorage::HenDN3>(&dNorm);
		//assert((norm | norm) > zero);

		const Maths::HF32 dif(r | norm);
		if(dif > minusHalfRadius)
			continue;

		/*MathsStorage::Float3* f0 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexA, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f1 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexB, sizeof(MathsStorage::Float3));
		MathsStorage::Float3* f2 = (MathsStorage::Float3*)Spu::FetchAllocUnaligned(verts + tri->m_IndexC, sizeof(MathsStorage::Float3));
		
		
		Collision::C_Triangle triangle;
		triangle.m_VertexA=Load<HVec3, MathsStorage::Float3>(f0);
		triangle.m_VertexB=Load<HVec3, MathsStorage::Float3>(f1);
		triangle.m_VertexC=Load<HVec3, MathsStorage::Float3>(f2);*/

		const HVec3 localA(&verts[tri->m_IndexA ]);
		const HVec3 localB(&verts[tri->m_IndexB ]);
		const HVec3 localC(&verts[tri->m_IndexC ]);
		Collision::C_Triangle triangle;
		triangle.m_VertexA = localA*range + low;
		triangle.m_VertexB = localB*range + low;
		triangle.m_VertexC = localC*range + low;
		//lambda = zero;

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dTriangleOutline(triangle.m_VertexA, triangle.m_VertexB, triangle.m_VertexC, Maths::RGBA(255, 0, 0));

		if(Collision::GJKRayCast<Collision::C_Sphere, Collision::C_Triangle>(sphere, triangle, r, maxLambda, lambda, nor, closestA, closestB))
		{
			nor = norm;
			maxLambda = lambda;
			collide = true;
		}

		/*if(Collision::GJKRayCastPenteraction<Collision::C_Sphere, Collision::C_Triangle>(sphere, triangle, r, maxLambda, lambda, nor, closestA, closestB, epa))
		{
			nor = norm;
			maxLambda = lambda;
			collide = true;
		}*/
	}
	
	return collide;

}

};

#endif
