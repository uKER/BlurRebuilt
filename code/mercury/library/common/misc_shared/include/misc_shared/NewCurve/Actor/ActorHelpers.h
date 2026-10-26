#ifndef CURVE_ACTOR_HELPERS_H
#define CURVE_ACTOR_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	namespace Actor
	{
		template<typename VECTYPE>
		F32 InterpolateKeyData(
			const VECTYPE& keyPosA, 
			F32 uA,
			const VECTYPE& keyPosB, 
			F32 uB,
			const VECTYPE& pos);

		template<typename VECTYPE>
		VECTYPE InterpolatePosLinear(
			const VECTYPE& keyPosA, 
			F32 uA,
			const VECTYPE& keyPosB, 
			F32 uB,
			const VECTYPE& pos, 
			F32& splinePos);



		template<typename CURVE>
		F32	FindNearestPoint(
			const CURVE& curve,
			const C_Reparameterise& reparam,
			const typename CURVE::VecType& position, 
			typename CURVE::VecType& closestPos);

		template<typename CURVE>
		F32	FindNearestPointLinear(
			const CURVE& curve,
			const C_Reparameterise& reparam,
			const typename CURVE::VecType& position, 
			typename CURVE::VecType& closestPos);



		template<typename CURVE>
		S32	FollowerFindNearestReparamIndex(
			const CURVE& curve,
			const C_Reparameterise& reparam,
			const typename CURVE::VecType& position, 
			U32 startIndex);

		template<typename CURVE>
		S32	FollowerFindNearestReparamIndexForwardsAndBackwards(
			const CURVE& curve,
			const C_Reparameterise& reparam,
			const typename CURVE::VecType& position, 
			U32 startIndex);

		template<typename CURVE>
		S32	FollowerFindNearestKnotIndex(
			const CURVE& curve,
			const typename CURVE::VecType& position, 
			U32 startIndex);

		template<typename CURVE>
		S32	FollowerFindStartKnotIndex(
			const CURVE& curve,
			const typename CURVE::VecType& position);



		template<typename CURVE>
		F32	FollowerFindNearestPoint(
			const CURVE& curve,
			const C_Reparameterise& reparam,
			const typename CURVE::VecType& position, 
			U32 closestKey, 
			typename CURVE::VecType& closestPos, 
			typename CURVE::VecType& tangent);

		template<typename CURVE>
		F32	FollowerFindNearestPointLinear(
			const CURVE& curve,
			const typename CURVE::VecType& position, 
			U32 closestKey, 
			typename CURVE::VecType& closestPos, 
			typename CURVE::VecType& tangent);



		template<typename CURVE>
		F32	FollowerGetArcDistance(
			const CURVE& curve,
			const C_Reparameterise& reparam,
			const typename CURVE::VecType& position, 
			U32 closestKey);

		template<typename CURVE>
		F32	FollowerGetArcDistanceLinear(
			const CURVE& curve,
			const typename CURVE::VecType& position, 
			U32 closestKey);



		template<typename CURVE>
		void GetForwardFacingSplinePos(
			Maths::C_QuatTrans& quatTrans,
			const CURVE& curve,
			F32 u,
			F32 uNext,
			bool clampEnd = true);

		#include "ActorHelpers.inl"
	}
}

#endif

