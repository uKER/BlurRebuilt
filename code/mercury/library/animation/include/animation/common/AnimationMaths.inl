#ifndef ANIMATION_ANIMATIONMATHS_INL
#define ANIMATION_ANIMATIONMATHS_INL

#ifdef _MSC_VER
#pragma once
#endif

#if !defined(ANIMATION_ALLOW_INCLUDE) && defined(ANIMATION_SHOW_INCLUDES)
#pragma message("******* Including AnimationMaths.inl outside of animation library")
#endif

//#include <core/maths/hvec/new/IncludeGeneral.h>
//#include <core/maths/hvec/new/IncludeGeneral.inl>
#include <core/maths/hvec/new/IncludeAnimation.h>
#include <core/maths/hvec/new/IncludeAnimation.inl>

namespace Animation
{

	FORCE_INLINE Maths::Quaternion CalculateShortestRotationQuaternion( const Maths::Normal3& fromVector, const Maths::Normal3& toVector )
	{
		using namespace Maths;

		const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
		const Maths::VF32 half = Maths::Const< Maths::VF32 >::Half();
			
		Maths::VF32 cosTheta = fromVector | toVector;
		Maths::Vector3 axis = fromVector ^ toVector;

		// Ensure that sqrts don't get negative inputs.
		cosTheta = Maths::Clamp( cosTheta, -one, one );
		
		// cos theta gives rotation in the range 0-pi degrees.
		Maths::VF32 sinHalfTheta = Maths::Sqrt( Maths::NegMulSub( cosTheta, half, half ) );
		Maths::VF32 cosHalfTheta = Maths::Sqrt( Maths::MulAdd( cosTheta, half, half ) );
		
		#if 1
		//const Maths::VF32 eps = Maths::Const< Maths::VF32 >::Epsilon();
		const Maths::VF32 zero = Maths::Const< Maths::VF32 >::Zero();
		const Maths::VF32 axisLengthSquared = Maths::LengthSquared( axis );
		const Maths::VF32 eps = Maths::Const< Maths::VF32 >::Epsilon();
		Maths::Normal3 normalisedAxis = Maths::Select( axisLengthSquared > eps, Maths::Normalise( axis ), Maths::Const< Maths::Normal3 >::AxisX() );
		#else
		// Comparing a vector3 to zero doesn't seem to work on SPU
		const Maths::Vector3 zeroVec3 = Maths::Const< Maths::Vector3 >::Zero();
		Maths::Normal3 normalisedAxis = Maths::Select( axis != zeroVec3, Maths::Normalise( axis ), Maths::MakeNormal3( axis ) );
		#endif
		
		return Maths::MakeQuaternion( Maths::MakeVec4F32( normalisedAxis * sinHalfTheta, cosHalfTheta ) );
	}
	
	FORCE_INLINE Maths::Quaternion CalculateShortestRotationAboutAxis( const Maths::Normal3& fromVector, const Maths::Normal3& toVector, const Maths::Normal3& axis )
	{
		const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
		const Maths::VF32 half = Maths::Const< Maths::VF32 >::Half();
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
			
		const Maths::VF32 cosTheta = Maths::Clamp(fromVector | toVector, -one, one);
		const Maths::Vector3 perpAxis = fromVector ^ toVector;
		const Maths::VF32 perpAxisDot = perpAxis | axis;
		const Maths::Normal3 rotationAxis = Maths::Select( perpAxisDot >= zero, axis, -axis );
		
		// cos theta gives rotation in the range 0-pi degrees.
		Maths::VF32 sinHalfTheta = Maths::Sqrt(Maths::NegMulSub(cosTheta, half, half));
		Maths::VF32 cosHalfTheta = Maths::Sqrt(Maths::MulAdd(cosTheta, half, half));
		
		// Need to fix sign of axis?
		
		return Maths::MakeQuaternion(Maths::MakeVec4F32(rotationAxis * sinHalfTheta, cosHalfTheta));
	}
	
	/// Calculates rotation where @a localAimAxis aligns with @a aimVector and @a localUpVector is oriented as much as possible to be in the same direction as @a upVector.
	inline Maths::Quaternion CalculateAimRotation(const Maths::Normal3& localAimAxis, const Maths::Normal3& localUpAxis, const Maths::Vector3& aimVector, const Maths::Vector3& upVector)
	{
		const Maths::Normal3 aimDir = Maths::Normalise(aimVector);

		// Subtract any part of up vector parallel to aim vector.
		const Maths::Vector3 perpUpVector = 
			Maths::NegMulSub(
				Maths::MakeVector3(aimDir),
				upVector | aimDir,
				upVector
			);

		const Maths::VF32 upVectorLength2 = Maths::LengthSquared(perpUpVector);
		const Maths::VF32 invLength = Maths::RcpSqrt(upVectorLength2);
		
		const Maths::Quaternion swing = CalculateShortestRotationQuaternion(localAimAxis, aimDir);
		const Maths::Normal3 upVectorAfterSwing = localUpAxis * swing;

		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		const Maths::Normal3 normalisedUpVector =
			Maths::Select(
				upVectorLength2 != zero,
				Maths::MakeNormal3(perpUpVector * invLength),
				upVectorAfterSwing
			);		
			
		// Find twist rotation about modelAimVector that aligns localUpVector with modelTargetUpVector.
		const Maths::Quaternion twist = CalculateShortestRotationAboutAxis(upVectorAfterSwing, normalisedUpVector, aimDir);
		
		// Combine two rotations to produce final rotation.
		Maths::Quaternion finalRotation = swing * twist;
		
		return finalRotation;
	}

	
	extern const Maths::Vec4F32 gFastArcTanConstants;
	
	inline Maths::VF32 FastArcTan2( const Maths::VF32& y, const Maths::VF32& x )
	{
		const Maths::VF32 t = y / x;
		
		const Maths::Vec4F32 c = gFastArcTanConstants;
		const Maths::VF32 k = Maths::X( c );
		const Maths::VF32 piBy2 = Maths::Y( c );
		const Maths::VF32 pi = piBy2 + piBy2;
		const Maths::VF32 one = Maths::Z( c );
		const Maths::VF32 zero = Maths::W( c );

		const Maths::VF32 t2 = t * t;
		
		// if abs(t) < 1
		Maths::VF32 atan_t_le_one = t * Maths::Rcp( Maths::MulAdd( t2, k, one ) );
		const Maths::VF32 atan_t_le_one_x_lt_zero = atan_t_le_one + Maths::Select( y < zero, -pi, pi );
		atan_t_le_one = Maths::Select( x < zero, atan_t_le_one_x_lt_zero, atan_t_le_one );
		
		// if abs(t) >= 1
		const Maths::VF32 atan_t_gt_one = ( piBy2 - t * Maths::Rcp( t2 + k ) ) - Maths::Select( y < zero, pi, zero );
		
		const Maths::VF32 atant = Maths::Select( Maths::Abs(t) <= one, atan_t_le_one, atan_t_gt_one );
		
		Maths::VF32 resultIfZeroX = Maths::Select( y > zero, piBy2, -piBy2 );
		resultIfZeroX = Maths::Select( y == zero, zero, resultIfZeroX );
		
		const Maths::VF32 result = Maths::Select( x == zero, resultIfZeroX, atant );
		
		return result;
	}
	
	
	//
	// C_SphericalLimits
	//
	
	inline Maths::Vector3 C_SphericalLimits::GetLimitVector(
		const Maths::Normal3& axisU,
		const Maths::Normal3& axisV,
		const Maths::Normal3& rotationAxis,	// Rotation axis in UV plane.
		const Maths::Vector4& sinLimits,
		const Maths::Vector4& cosLimits
	)
	{
		const Maths::Normal3 axisW = Maths::Normalise(axisU ^ axisV);
	
		// Calculate the four limit points for +/- U and V axes.
		const Maths::Vector3 limitNegU = Maths::X(sinLimits) * -axisU + Maths::X(cosLimits) * axisW;
		const Maths::Vector3 limitPosU = Maths::Y(sinLimits) * axisU  + Maths::Y(cosLimits) * axisW;
		const Maths::Vector3 limitNegV = Maths::Z(sinLimits) * -axisV + Maths::Z(cosLimits) * axisW;
		const Maths::Vector3 limitPosV = Maths::W(sinLimits) * axisV  + Maths::W(cosLimits) * axisW;
		
		// Stereographically project the four limits onto plane through equator.
		// These four points define extents for an ellipse in projected space.
		const Maths::Vector3 projNegU = C_StereographicProjection::Project(limitNegU, axisW);
		const Maths::Vector3 projPosU = C_StereographicProjection::Project(limitPosU, axisW);
		const Maths::Vector3 projNegV = C_StereographicProjection::Project(limitNegV, axisW);
		const Maths::Vector3 projPosV = C_StereographicProjection::Project(limitPosV, axisW);

		// Calculate u, sin(theta), and v, cos(theta), as defined by the swing axis. The swing
		// axis is a normalised vector in the UV plane.
		const Maths::VF32 u = rotationAxis | axisU;
		const Maths::VF32 v = rotationAxis | axisV;
	
		// Determine dimensions of ellipse quadrant that contains u and v.
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		const Maths::VF32 a = Maths::Select(v >= zero, (projPosU | axisU), (projNegU | -axisU));
		const Maths::VF32 b = Maths::Select(u >= zero, (projPosV | axisV), (projNegV | -axisV));
		

		// Evaluate the radius of the projected-space ellipse.
		// r^2 = a^2b^2 / (a^2sin^2(theta) + b^2 sin^2(theta))
		
		const Maths::VF32 a2 = a*a;
		const Maths::VF32 b2 = b*b;
		const Maths::VF32 r2 = a2*b2 / (a2*v*v + b2*u*u);
		const Maths::VF32 r = Maths::Sqrt(r2);

		// Use the rotation axis to generate the projected space limit point.
		const Maths::Vector3 projLimit = r * rotationAxis;
		
		// Unproject to obtain a vector at the maximum permitted swing angle.
		const Maths::Vector3 limitAxis = C_StereographicProjection::Unproject(projLimit, axisW);
		
		return limitAxis;
	}	
	
	inline Maths::Quaternion LimitRotation(const Maths::Quaternion& rotation, const Maths::VF32& maxAngle)
	{
		Maths::Normal3 axis = Maths::RotationAxis(rotation);
		Maths::VF32 currentAngle = Maths::RotationAngle(rotation);
		Maths::VF32 newAngle = Maths::Clamp(currentAngle, -maxAngle, maxAngle);
		return Maths::MakeUnitQuaternion(axis, newAngle);
	}

}

#endif
