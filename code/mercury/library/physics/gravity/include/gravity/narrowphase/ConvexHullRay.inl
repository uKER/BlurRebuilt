#ifndef CONVEXHULLRAY_INL
#define CONVEXHULLRAY_INL

#include "gravity/narrowphase/Shapes.h"
#include "gravity/narrowphase/PrimitiveTest.h"
#include "gravity/narrowphase/SweptTest.h"

namespace Collision
{

bool inline CollideConvexHullRay(const C_Shape* const __restrict shape, C_Ray& ray)
{
	using namespace Maths;

	const HF32 zero(Constants<HF32>::Zero());
	Spu::MemBaseMarker marker;
	C_ConvexHullShape* cShape=(C_ConvexHullShape*)shape;
	Collision::C_ConvexHull& hull=cShape->m_Hull;

	Collision::C_SpuConvexHull spuConvex(cShape->m_Hull, true);

	const HVec3 center=hull.Centre();
	const HMat33& mat(hull.Orientation());
	HVec3 contact(UNDEFINED);
	HVec3 start(mat*(ray.m_start-center));
	HVec3 end(mat*(ray.m_end-center));
	HVec3 normal(UNDEFINED);
	bool collide=false;

	const U32 numTriangles = spuConvex.GetNumTriangles();

	for(U32 i=0; (i<numTriangles) & (!collide); ++i)
	{
		//Collision::C_ConvexTriangle* tri = &spuConvex.m_pTriangles[i];
		Collision::C_ConvexTriangle* tri = &spuConvex.GetTriangle(i);
		const HVec3 norm(Maths::Load<Maths::HVec3>(&tri->m_Normal));
		const HVec3 a(spuConvex.GetLocalVertex(tri->m_VertexA));
		const HVec3 b(spuConvex.GetLocalVertex(tri->m_VertexB));
		const HVec3 c(spuConvex.GetLocalVertex(tri->m_VertexC));


		
		//if(IntersectTriangleSegment(a, b, c, start, end, contact))
		const Maths::Scalar_Bool bRes(IntersectTriangleSegmentSB(a, b, c, start, end, contact) & (norm | (end - start)).LT(zero));
		{
			end=Maths::Select(bRes, contact, end);
			normal=Maths::Select(bRes, norm, normal);
			//collide=true;
			collide = collide || bool(bRes);
		}
	}

	if(collide)
	{
		const HVec3 fContact((end * mat)+center);
		ray.m_result.m_contact=fContact;
		ray.m_end=fContact;
		ray.m_result.m_distance=(end-start).Length();
		ray.m_result.m_normal=Maths::C_Normal(normal * mat, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		ray.m_result.m_TriangleNormal = ray.m_result.m_normal;
		ray.m_result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		ray.m_result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		ray.m_result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		ray.m_result.m_UserData = shape->m_UserData;

	}
	return collide;
}

bool inline CollideConvexHullFatRay(const C_Shape* const __restrict shape, C_Ray& ray)
{
	using namespace Maths;

	Spu::MemBaseMarker marker;
	C_ConvexHullShape* cShape=(C_ConvexHullShape*)shape;
	Collision::C_ConvexHull& hull=cShape->m_Hull;

	Collision::C_SpuConvexHull spuConvex(cShape->m_Hull, true);


	const HVec3 center=spuConvex.Centre();
	const HMat33& mat(spuConvex.Orientation());
	HVec3 start(mat*(ray.m_start-center));
	HVec3 end(mat*(ray.m_end-center));

	C_Normal direction(mat * ray.m_Direction, Maths::C_Normal::UNIT_LENGTH_VECTOR);

	bool collide(false);
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
	HVec3 temp(end);
	HVec3 contact(end);
	C_Normal normal(mat * ray.m_result.m_normal), resultNormal(Maths::UNDEFINED);
	U32 triangleIndex(0);
	HF32 dis(ray.m_result.m_distance), distance(ray.m_result.m_distance);

	const U32 numTriangles = spuConvex.GetNumTriangles();
	for(U32 i=0; i<numTriangles; ++i)
	{
		Spu::MemBaseMarker marker;
		Collision::C_ConvexTriangle* tri = &spuConvex.GetTriangle(i);
		const HVec3 a(spuConvex.GetLocalVertex(tri->m_VertexA));
		const HVec3 b(spuConvex.GetLocalVertex(tri->m_VertexB));
		const HVec3 c(spuConvex.GetLocalVertex(tri->m_VertexC));

		const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&tri->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

		const HF32 _dis((a|_norm) + ray.m_Radius);
		const HF32 disStart((start | _norm) - _dis);
		const HF32 disEnd((end | _norm) - _dis);

		//The ray is on the same side of the plane, no intersection
		if((disStart*disEnd).GT(zero))
			continue;
		
		
		Maths::C_Normal norm(_norm, Maths::C_Normal::UNIT_LENGTH_VECTOR);

		const Maths::HF32 cosAngle(norm|direction);

		/*if(Maths::Abs(cosAngle).LT(Maths::Constants<Maths::HF32>::Epsilon()) | cosAngle.GT(ray.m_MaxCosAngle))
			continue;*/

		C_RayCastTriangle triangle(a, b, c, norm);

		const Maths::Scalar_Bool bScal(IntersectTriangleSegmentSB(triangle, start, temp, 
			direction, ray.m_Radius, contact, dis, resultNormal));

		const Maths::Scalar_Bool bEarlier(bScal & dis.LT(distance));
		temp = Maths::Select(bEarlier, start + direction * dis, temp);
		distance = Maths::Select(bEarlier, dis, distance);
		dis = distance;
		bool bCon(bEarlier);
		normal = Maths::Select(bEarlier, resultNormal, normal);
		triangleIndex = Maths::Select(bCon, i, triangleIndex);
		collide |= bCon;
	}
	if(collide)
	{
		/*ray.m_result.m_contact = temp * spuConvex.m_Orientation + center;
		ray.m_result.m_normal = normal * spuConvex.m_Orientation;*/

		ray.m_result.m_contact = temp * mat + center;
		ray.m_result.m_normal = normal * mat;
		ray.m_result.m_distance = distance;
		ray.m_result.m_TriangleIndex = triangleIndex;
		ray.m_result.m_TriangleNormal = Maths::C_Normal(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&spuConvex.GetTriangle(triangleIndex).m_Normal) * mat, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		ray.m_result.m_PhysicsMaterialIndex = cShape->m_PhysicsMaterialIndex;
		ray.m_result.m_EffectMaterialIndex = cShape->m_EffectMaterialIndex;
		ray.m_result.m_UserData = cShape->m_UserData;
		ray.m_end = ray.m_start + ray.m_Direction * distance;
		ray.m_result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
	}
	return collide;

}



bool inline CollideConvexHullRay(const C_Shape* const __restrict shape, C_TransparentRay& ray,
								 SharedPhysics::C_GraphNode* pGraphNode)
{
	using namespace Maths;

	Spu::MemBaseMarker marker;
	C_ConvexHullShape* cShape=(C_ConvexHullShape*)shape;
	Collision::C_ConvexHull& hull=cShape->m_Hull;

	Collision::C_SpuConvexHull spuConvex(hull, true);

	const HVec3 center=hull.Centre();
	const HMat33& mat(hull.Orientation());
	HVec3 contact(UNDEFINED);
	HVec3 start(mat*(ray.m_start-center));
	HVec3 end(mat*(ray.m_end-center));
	HVec3 normal(UNDEFINED);
	bool collide=false;

	const U32 numTriangles = spuConvex.GetNumTriangles();

	for(U32 i=0; i<numTriangles; ++i)
	{
		Collision::C_ConvexTriangle* tri = &spuConvex.GetTriangle(i);
		HVec3 a(spuConvex.GetLocalVertex(tri->m_VertexA));
		HVec3 b(spuConvex.GetLocalVertex(tri->m_VertexB));
		HVec3 c(spuConvex.GetLocalVertex(tri->m_VertexC));
		
		//if(IntersectTriangleSegment(a, b, c, start, end, contact))
		const Maths::Scalar_Bool bRes(IntersectTriangleSegmentSB(a, b, c, start, end, contact));
		{
			end=Maths::Select(bRes, contact, end);
			normal=Maths::Select(bRes, Maths::Load<Maths::HVec3>(&tri->m_Normal), normal);
			//collide=true;
			collide = collide || bool(bRes);
		}
	}

	if(collide)
	{
		Collision::C_RayTracingResult result;
		const HVec3 fContact((end * mat)+center);
		result.m_contact=fContact;
		end=fContact;
		result.m_distance=(end-start).Length();
		result.m_normal=Maths::C_Normal(normal * mat, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		result.m_TriangleNormal = result.m_normal;
		result.m_PhysicsMaterialIndex = shape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = shape->m_EffectMaterialIndex;
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_CollisionFlags = shape->m_Filter.GetCollisionFlags();
		result.m_UserData = shape->m_UserData;
		result.m_pNode = pGraphNode;
		ray.StoreResult(result);

	}
	return collide;
}

bool inline CollideConvexHullFatRay(const C_Shape* const __restrict shape, C_TransparentRay& ray,
									SharedPhysics::C_GraphNode* pGraphNode)
{
	using namespace Maths;

	Spu::MemBaseMarker marker;
	C_ConvexHullShape* cShape=(C_ConvexHullShape*)shape;
	Collision::C_ConvexHull& hull=cShape->m_Hull;

	Collision::C_SpuConvexHull spuConvex(cShape->m_Hull, true);


	const HVec3 center=spuConvex.Centre();
	const HMat33& mat(spuConvex.Orientation());
	HVec3 start(mat*(ray.m_start-center));
	HVec3 end(mat*(ray.m_end-center));

	C_Normal direction(mat * ray.m_Direction, Maths::C_Normal::UNIT_LENGTH_VECTOR);

	bool collide(false);
	const HF32 zero(Constants<HF32>::Zero());
	const HF32 disEps(SharedPhysics::Constants::CloseNormal());
	HVec3 temp(end);
	HVec3 contact(end);
	C_Normal normal(Maths::UNDEFINED), resultNormal(Maths::UNDEFINED);
	U32 triangleIndex(0);
	HF32 dis(ray.m_maxDistance), distance(ray.m_maxDistance);

	const U32 numTriangles = spuConvex.GetNumTriangles();
	for(U32 i=0; i<numTriangles; ++i)
	{
		Spu::MemBaseMarker marker;
		Collision::C_ConvexTriangle* tri = &spuConvex.GetTriangle(i);
		const HVec3 a(spuConvex.GetLocalVertex(tri->m_VertexA));
		const HVec3 b(spuConvex.GetLocalVertex(tri->m_VertexB));
		const HVec3 c(spuConvex.GetLocalVertex(tri->m_VertexC));

		const Maths::C_Normal _norm(Load<Maths::HVec3, MathsStorage::HenDN3>(&tri->m_Normal), Maths::C_Normal::UNIT_LENGTH_VECTOR);

		const HF32 _dis((a|_norm) + ray.m_Radius);
		const HF32 disStart((start | _norm) - _dis);
		const HF32 disEnd((end | _norm) - _dis);

		//The ray is on the same side of the plane, no intersection
		if((disStart*disEnd).GT(zero))
			continue;


		Maths::C_Normal norm(_norm, Maths::C_Normal::UNIT_LENGTH_VECTOR);

		const Maths::HF32 cosAngle(norm|direction);

		/*if(Maths::Abs(cosAngle).LT(Maths::Constants<Maths::HF32>::Epsilon()) | cosAngle.GT(ray.m_MaxCosAngle))
		continue;*/

		C_RayCastTriangle triangle(a, b, c, norm);

		const Maths::Scalar_Bool bScal(IntersectTriangleSegmentSB(triangle, start, end, 
			direction, ray.m_Radius, contact, dis, resultNormal));

		const Maths::Scalar_Bool bEarlier(bScal & dis.LT(distance));
		temp = Maths::Select(bEarlier, start + direction * dis, temp);
		distance = Maths::Select(bEarlier, dis, distance);
		dis = distance;
		bool bCon(bEarlier);
		normal = Maths::Select(bEarlier, resultNormal, normal);
		triangleIndex = Maths::Select(bCon, i, triangleIndex);
		collide |= bCon;
	}
	if(collide)
	{
		Collision::C_RayTracingResult result;
		result.m_contact = temp * mat + center;
		result.m_normal = normal * mat;
		result.m_distance = distance;
		result.m_TriangleIndex = triangleIndex;
		result.m_TriangleNormal = Maths::C_Normal(Maths::Load<Maths::HVec3, MathsStorage::HenDN3>(&spuConvex.GetTriangle(triangleIndex).m_Normal) * mat, Maths::C_Normal::UNIT_LENGTH_VECTOR);
		result.m_PhysicsMaterialIndex = cShape->m_PhysicsMaterialIndex;
		result.m_EffectMaterialIndex = cShape->m_EffectMaterialIndex;
		result.m_UserData = cShape->m_UserData;
		result.m_pShape = const_cast<C_Shape*>(shape->m_pPpuPtr);
		result.m_pNode = pGraphNode;
		ray.StoreResult(result);
	}
	return collide;

}


};

#endif
