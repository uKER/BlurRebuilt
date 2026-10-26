#ifndef EDITOR_LIB_COLLISION_HELPERS_H
#define EDITOR_LIB_COLLISION_HELPERS_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	namespace CollisionHelpers
	{
		inline bool IntersectTriangleRay(
			const Maths::HVec3& p0, 
			const Maths::HVec3& p1, 
			const Maths::HVec3& p2,
			const Maths::HVec3& origin, 
			const Maths::HVec3& dir, 
			Maths::HVec3& res);

		void NearestPointOnEdge(
			C_NearestPointResult& result, 
			const Vector3& point, 
			const C_CollisionMesh* pMesh,
			U32 triIdx,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace);

		//Determine the nearest point to a previous ray intersection.
		void NearestPoint(
			C_NearestPointResult& result, 
			const C_RayResult& target);

		void NearestPoint(
			C_NearestPointResult& result, 
			const Vector3& point, 
			const C_CollisionMesh* pMesh,
			U32 triIdx,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace);

		void NearestPoint(
			C_NearestPointResult& result, 
			const Vector3& point,
			const C_CollisionMesh* pMesh,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace);

		void NearestPoint(
			C_NearestPointResult& result, 
			const Vector3& point,
			const C_CollisionSpline* pSpline,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace);

		bool NearestPoint(
			C_NearestPointResult& result, 
			const C_Ray& ray,
			const C_CollisionMesh* pMesh,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace);

		bool NearestPoint(
			C_NearestPointResult& result, 
			const C_Ray& ray,
			const C_CollisionSpline* pSpline,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace);

		bool NearestPoint(
			C_NearestPointResult& result, 
			const C_Ray& ray,
			const C_CollisionProxy* pProxy,
			const Matrix44& toObjectSpace,
			const Matrix44& toWorldSpace);
	}
}

#include "CollisionHelpers.inl"

#endif
