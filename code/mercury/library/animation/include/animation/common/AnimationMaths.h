#ifndef ANIMATION_ANIMATIONMATHS_H
#define ANIMATION_ANIMATIONMATHS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/maths/hvec/new/IncludeMinimal.h>
#include <core/maths/hvec/new/IncludeMinimal.inl>

///
/// \file AnimationMaths.h
///
/// Functions and classes for animation maths.
///

namespace Animation
{
	/// Converts a quaternion into a Euler angle triple.
	void QuaternionToEuler( const Maths::Quaternion& q, Maths::Vector3& eulerPHR );
	
	/// Converts a Euler angle triple into a quaternion.
	void QuaternionFromEuler( Maths::Quaternion& q, const Maths::Vector3& eulerPHR );	

	/// Calculates the shortest rotation quaternion to rotate \a fromVector onto \a toVector.
	Maths::Quaternion CalculateShortestRotationQuaternion( const Maths::Normal3& fromVector, const Maths::Normal3& toVector );

	/// Calculates the shortest rotation quaternion to rotate \a fromVector onto \a toVector about @a axis.
	Maths::Quaternion CalculateShortestRotationAboutAxis( const Maths::Normal3& fromVector, const Maths::Normal3& toVector, const Maths::Normal3& axis );
	
	/// Calculates rotation where @a localAimAxis aligns with @a aimVector and @a localUpVector is oriented as much as possible to be in the same direction as @a upVector.
	Maths::Quaternion CalculateAimRotation(const Maths::Normal3& localAimAxis, const Maths::Normal3& localUpAxis, const Maths::Vector3& aimVector, const Maths::Vector3& upVector);
	
	Maths::VF32 FastArcTan2( const Maths::VF32& y, const Maths::VF32& x );
	
	/**
	Utility class for performing stereographic projections which project
	points on a sphere onto a plane. In this implementation, points are
	projected from the sphere's south pole onto a plane through
	its equator. There is a singularity at the south pole.
	
	See http://mathworld.wolfram.com/StereographicProjection.html
	*/
	class C_StereographicProjection
	{
	public:
		/// Perform a stereographic projection of point @a p.
		static Maths::Vector3 Project(const Maths::Vector3& p, const Maths::Normal3& northPole)
		{
			const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
			const Maths::VF32 z = northPole | p;
			const Maths::VF32 k = one / (one + z);
			return Maths::MulAdd(p + Maths::MakeVector3(northPole), k, -Maths::MakeVector3(northPole));
		}

		/// Performs an inverse stereographic projection of point @a q.
		static Maths::Vector3 Unproject(const Maths::Vector3& q, const Maths::Normal3& northPole)
		{
			const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
			const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
			const Maths::VF32 two = Maths::Const<Maths::VF32>::Two();
			const Maths::VF32 r2 = Maths::LengthSquared(q);
			const Maths::VF32 invK = Maths::Select((q | northPole) <= -one, zero, two / (one + r2));
			return Maths::MulAdd(q + northPole, invK, -northPole);
		}
	};
	
	/**
	Utility class for applying spherical limits.
	*/
	class C_SphericalLimits
	{
	public:

		/**
		Returns a point on the stereographic projection plane.
		Uses an stereographically projected ellipse to define a smooth constraint boundary.
		*/
		static Maths::Vector3 GetLimitVector(
			const Maths::Normal3& axisU,
			const Maths::Normal3& axisV,
			const Maths::Normal3& rotationAxis,	// Rotation axis in UV plane.
			const Maths::Vector4& sinLimits,
			const Maths::Vector4& cosLimits
		);
	};
	
	Maths::Quaternion LimitRotation(const Maths::Quaternion& rotation, const Maths::VF32& maxAngle);
}

#include "AnimationMaths.inl"

#endif

