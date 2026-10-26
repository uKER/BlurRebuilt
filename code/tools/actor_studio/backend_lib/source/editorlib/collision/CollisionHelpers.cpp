#include "stdafx.h"

#include <Gravity/NarrowPhase/Primitive.h>
#include <Gravity/NarrowPhase/CollisionResult.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Gravity/NarrowPhase/Primitive.inl>

#include <EditorLib/Collision/Ray.h>
#include <EditorLib/Collision/CollisionResult.h>
#include <EditorLib/Collision/CollisionStructs.h>

namespace EditorLib
{
namespace CollisionHelpers
{

void NearestPointOnEdge(
	C_NearestPointResult& result, 
	const Vector3& point, 
	const C_CollisionMesh* pMesh,
	U32 triIdx,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	Maths::HMat44 m = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toObjectSpace);
	Maths::HMat44 wM = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toWorldSpace);
	Maths::HVec3 p(point.x, point.y, point.z);
	p = p*m;

	const U32* pTri = &pMesh->pTriIndices[triIdx*3];
	
	Maths::C_Vertex a = Maths::Load<Maths::C_Vertex>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[0]]);
	Maths::C_Vertex b = Maths::Load<Maths::C_Vertex>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[1]]);
	Maths::C_Vertex c = Maths::Load<Maths::C_Vertex>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[2]]);
	Maths::HVec3 closestPoint;

	C_CollisionEdge ab;
	ab.begin = a;
	ab.end = b;
	C_CollisionEdge bc;
	bc.begin = b;
	bc.end = c;
	C_CollisionEdge ca;
	ca.begin = c;
	ca.end = a;

	//Edge 0
	Maths::HVec3 cp;
	Collision::ClosestPtPointSegment(p,	ab.begin, ab.end, cp);
	Maths::HVec3 delta = cp - p;
	Maths::HF32 distanceSquared = delta.LengthSquared();
	result.adjacentIdx1 = pTri[0];
	result.adjacentIdx2 = pTri[1];
	closestPoint = cp;

	//Edge 1
	Collision::ClosestPtPointSegment(p,	bc.begin, bc.end, cp);
	delta = cp - p;
	Maths::HF32 tmp = delta.LengthSquared();
	if (tmp < distanceSquared)
	{
		distanceSquared = tmp;
		result.adjacentIdx1 = pTri[1];
		result.adjacentIdx2 = pTri[2];
		closestPoint = cp;
	}

	//Edge 2
	Collision::ClosestPtPointSegment(p,	ca.begin, ca.end, cp);
	delta = cp - p;
	tmp = delta.LengthSquared();
	if (tmp < distanceSquared)
	{
		distanceSquared = tmp;
		result.adjacentIdx1 = pTri[2];
		result.adjacentIdx2 = pTri[0];
		closestPoint = cp;
	}

	closestPoint = closestPoint*wM;
	Maths::Store(closestPoint, (MathsStorage::Float3*)&result.point);
	//Ensure our returned distance is actually in world space.
	result.distanceSquared = closestPoint.DistanceSquared(Maths::HVec3(point.x, point.y, point.z));
}

void NearestPoint(
	C_NearestPointResult& result, 
	const Vector3& point, 
	const C_CollisionMesh* pMesh,
	U32 triIdx,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	Maths::HMat44 m = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toObjectSpace);
	Maths::HMat44 wM = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toWorldSpace);
	Maths::HVec3 p(point.x, point.y, point.z);
	p = p*m;

	const U32* pTri = &pMesh->pTriIndices[triIdx*3];
	
	Maths::HVec3 a = Maths::Load<Maths::HVec3>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[0]]);
	Maths::HVec3 b = Maths::Load<Maths::HVec3>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[1]]);
	Maths::HVec3 c = Maths::Load<Maths::HVec3>((const MathsStorage::Float3*)&pMesh->pVertices[pTri[2]]);
	Maths::HVec3 closestPoint;
	
	//V 0
	Maths::HVec3 delta = a - p;
	Maths::HF32 distanceSquared = delta.LengthSquared();
	result.adjacentIdx1 = pTri[2];
	result.adjacentIdx2 = pTri[1];
	closestPoint = a;

	//V 1
	delta = b - p;
	Maths::HF32 tmp = delta.LengthSquared();
	if (tmp < distanceSquared)
	{
		distanceSquared = tmp;
		result.adjacentIdx1 = pTri[0];
		result.adjacentIdx2 = pTri[2];
		closestPoint = b;
	}

	//V 2
	delta = c - p;
	tmp = delta.LengthSquared();
	if (tmp < distanceSquared)
	{
		distanceSquared = tmp;
		result.adjacentIdx1 = pTri[1];
		result.adjacentIdx2 = pTri[0];
		closestPoint = c;
	}

	closestPoint = closestPoint*wM;
	Maths::Store(closestPoint, (MathsStorage::Float3*)&result.point);
	result.distanceSquared = closestPoint.DistanceSquared(Maths::HVec3(point.x, point.y, point.z));
}

void NearestPoint(
	C_NearestPointResult& result, 
	const Vector3& point,
	const Vector3* pVertices,
	U32 nVertices,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	Vector3 objSpacePoint = Vector4(point.x, point.y, point.z, 1.f) * toObjectSpace;
	Vector3 closestPoint;
	F32 localDistanceSquared = FLT_MAX;
	S32 adjacent1;
	S32 adjacent2;

	for (U32 i = 0; i < nVertices; i++)
	{
		Vector3 d = pVertices[i] - objSpacePoint;
		F32 distSq = d.x*d.x + d.y*d.y + d.z*d.z;
		if (distSq < localDistanceSquared)
		{
			localDistanceSquared = distSq;
			closestPoint = pVertices[i];
			if (i == 0)
				adjacent1 = nVertices -1;
			else
				adjacent1 = i-1;
			if (i == nVertices-1)
				adjacent2 = 0;
			else
				adjacent2 = i+1;
		}
	}

	closestPoint = Vector4(closestPoint.x, closestPoint.y, closestPoint.z, 1.f) * toWorldSpace;
	result.point = closestPoint;
	Vector3 d = closestPoint - point;
	result.distanceSquared = d.x*d.x + d.y*d.y + d.z*d.z;
}

void NearestPoint(
	C_NearestPointResult& result, 
	const Vector3& point,
	const C_CollisionMesh* pMesh,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	NearestPoint(result, point, pMesh->pVertices, pMesh->nVertices, toObjectSpace, toWorldSpace);
}

void NearestPoint(
	C_NearestPointResult& result, 
	const Vector3& point,
	const C_CollisionSpline* pSpline,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	NearestPoint(result, point, pSpline->pVertices, pSpline->nVertices, toObjectSpace, toWorldSpace);
}

void NearestPoint(
	C_NearestPointResult& result, 
	const C_RayResult& target)
{
	//Find closest vertex to intersection point.
	if (target.pCollisionInstance->pSource->GetType() == 
		EditorLib::E_CollisionSourceType_Mesh)
	{
		EditorLib::C_CollisionMesh* pMesh = 
			(EditorLib::C_CollisionMesh*)target.pCollisionInstance->pSource;

		NearestPoint(
			result, 
			target.intersectionPoint,
			pMesh,
			target.pCollisionInstance->toObjectSpace,
			target.pCollisionInstance->toWorldSpace);

		result.pDgNode = target.pDgNode;
		result.pCollisionInstance = target.pCollisionInstance;
	}
}

bool NearestPoint(
	C_NearestPointResult& result, 
	const C_Ray& ray,
	const Vector3* pVertices,
	U32 nVertices,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	bool found = false;
	Maths::HMat44 m = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toObjectSpace);
	Maths::HMat44 wM = Maths::Load<Maths::HMat44>((const MathsStorage::Float44*)&toWorldSpace);
	Maths::C_Vertex hP(ray.origin.x, ray.origin.y, ray.origin.z);
	Maths::C_Vertex hQ(ray.end.x, ray.end.y, ray.end.z);
	hP = hP*m;
	hQ = hQ*m;

	F32 localDistanceSquared = FLT_MAX;
	Vector3 closestPoint;
	Vector3 closestVertex;
	S32 adjacent1;
	S32 adjacent2;
	for (U32 i = 0; i < nVertices; i++)
	{
		Maths::C_Vertex p = Maths::Load<Maths::C_Vertex>((const MathsStorage::Float3*)&pVertices[i]);
		Maths::C_Normal normal;
		Maths::HF32 localDistance;
		Maths::C_Vertex cp;
		F32 t = Maths::LineClosestPoint(hP, hQ, p, cp, normal, localDistance);
		if (t < 0 || t > 1.f)
			continue;

		localDistance = localDistance*localDistance;
		if (localDistance < localDistanceSquared)
		{
			found = true;
			localDistanceSquared = localDistance;
			closestVertex = pVertices[i];
			//Store closest point as we'll need this to calculate distance later.
			Maths::Store(cp, (MathsStorage::Float3*)&closestPoint);
			if (i == 0)
				adjacent1 = nVertices -1;
			else
				adjacent1 = i-1;
			if (i == nVertices-1)
				adjacent2 = 0;
			else
				adjacent2 = i+1;
		}
	}

	if (!found)
		return false;

	closestVertex = Vector4(closestVertex.x, closestVertex.y, closestVertex.z, 1.f) * toWorldSpace;
	closestPoint = Vector4(closestPoint.x, closestPoint.y, closestPoint.z, 1.f) * toWorldSpace;
	Vector3 d = closestPoint - closestVertex;
	
	result.adjacentIdx1 = adjacent1;
	result.adjacentIdx2 = adjacent2;
	result.point = closestVertex;
	result.distanceSquared = d.x*d.x + d.y*d.y + d.z*d.z;

	return true;
}

bool NearestPoint(
	C_NearestPointResult& result, 
	const C_Ray& ray,
	const C_CollisionMesh* pMesh,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	return NearestPoint(result, ray, pMesh->pVertices, pMesh->nVertices, toObjectSpace, toWorldSpace);
}

bool NearestPoint(
	C_NearestPointResult& result, 
	const C_Ray& ray,
	const C_CollisionSpline* pSpline,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	return NearestPoint(result, ray, pSpline->pVertices, pSpline->nVertices, toObjectSpace, toWorldSpace);
}

bool NearestPoint(
	C_NearestPointResult& result, 
	const C_Ray& ray,
	const C_CollisionProxy* pProxy,
	const Matrix44& toObjectSpace,
	const Matrix44& toWorldSpace)
{
	Vector3 centre;
	Maths::Store(pProxy->boundingSphere.Centre(), (MathsStorage::Float3*)&centre);
	return NearestPoint(result, ray, &centre, 1, toObjectSpace, toWorldSpace);
}

}

}
