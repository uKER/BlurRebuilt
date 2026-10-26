#ifndef EDITOR_LIB_COLLISION_RESULT_H
#define EDITOR_LIB_COLLISION_RESULT_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <EditorLib/Misc/Definitions.h>

namespace EditorLib
{
	struct I_CollisionInstance;

	struct C_CollisionResult
	{
		C_CollisionResult()
		{
			distanceSquared = FLT_MAX;
		}

		bool operator< (const C_CollisionResult& rhs) const
		{
			return distanceSquared < rhs.distanceSquared;
		}

		F32						distanceSquared;
	};

	struct C_NearestPointResult : C_CollisionResult
	{
		C_NearestPointResult() : C_CollisionResult()
		{
			memset(&point, 0, sizeof(Vector3));
			pDgNode = 0;
			pCollisionInstance = 0;
			
			adjacentIdx1 = -1;
			adjacentIdx2 = -1;
		}

		bool operator< (const C_NearestPointResult& rhs) const
		{
			return distanceSquared < rhs.distanceSquared;
		}

		Vector3					point;
		I_DgNode*				pDgNode;
		const I_CollisionInstance*	
								pCollisionInstance;
		S32						adjacentIdx1;
		S32						adjacentIdx2;
	};

	struct C_RayResult : C_CollisionResult
	{
		C_RayResult() : C_CollisionResult()
		{
			triIndex = -1;
			memset(&intersectionPoint, 0, sizeof(Vector3));
			pDgNode = 0;
			pCollisionInstance = 0;
		}

		bool operator< (const C_RayResult& rhs) const
		{
			return distanceSquared < rhs.distanceSquared;
		}

		S32						triIndex;
		Vector3					intersectionPoint;
		I_DgNode*				pDgNode;
		const I_CollisionInstance*	
								pCollisionInstance;
	};
}

#endif
