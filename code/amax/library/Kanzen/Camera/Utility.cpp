//=============================================================================
// Utility.cpp
// 02/02/09
//=============================================================================
//=============================================================================
// Includes
//=============================================================================
#include "stdafx.h"
#include "Kanzen_PCH.h"

namespace Kanzen
{
	using namespace Maths;

	//-----------------------------------------------------------------------------
	// SmoothingFunction
	// A velocity based smoothing function taken from Games Programming Gems IV
	//-----------------------------------------------------------------------------
	F32		
	SmoothingFunction( F32 from, F32 to, F32& vel, F32 smoothTime, F32 timeSlice )
	{
		F32 omega;
		if( smoothTime > 0.01f )
		{
			omega = 2.0f/smoothTime;
		
			F32 x = omega*timeSlice;
			F32 exp = 1.0f/(1.0f+x+0.48f*x*x+0.235f*x*x*x);
			F32 change = from - to;
			F32 temp = (vel+omega*change)*timeSlice;
			vel = (vel-omega*temp)*exp;
			F32 update = (change+temp)*exp;
			if(fabs(update)>fabs(change) && (update*change > 0.0f))
			{
				update *= (fabs(change)/fabs(update));
				vel = 0.0f;
			}
			return to+update;
		}
	    
		vel = 0.0f;
		return to;
	}

	//-----------------------------------------------------------------------------
	// SmoothingAngleFunction
	// The same as 'SmoothingFunction' but with wrapping between +/- PI
	//-----------------------------------------------------------------------------
	F32		
	SmoothingAngleFunction( F32 from, F32 to, F32& vel, F32 smoothTime, F32 timeSlice )
	{
		F32 omega;
		if( smoothTime > 0.01f )
		{
			omega = 2.0f/smoothTime;
		
			F32 x = omega*timeSlice;
			F32 exp = 1.0f/(1.0f+x+0.48f*x*x+0.235f*x*x*x);
			F32 change = from - to;
			if( fabs(change) > Pi )
			{
				change += change < 0.0f ? Pi2 : -Pi2; 
			}
			F32 temp = (vel+omega*change)*timeSlice;
			vel = (vel-omega*temp)*exp;
			return to+(change+temp)*exp;
		}
	    
		vel = 0.0f;
		return to;
	}

	//-----------------------------------------------------------------------------
	// DampingAdjust
	// A time independant damping factor. The damping coeff passed in should be the 
	// damping value assuming a timeslice of 1.0f/60.0f
	//-----------------------------------------------------------------------------
	F32 
	DampingAdjust( F32 dampingCoeff, F32 timeslice )
	{
		BIZ_ASSERT( dampingCoeff >= 0.0f && dampingCoeff <= 1.0f );
		F32 result = ( 1.0f - powf( dampingCoeff, ( timeslice * 60.0f )));   
		return result;
	}

	//-----------------------------------------------------------------------------
	// LerpValue
	//-----------------------------------------------------------------------------
	F32
	LerpValue(F32 period, F32 lag)
	{
		F32 lerp = 1.f - (lag / (lag + period));
		if (lerp > 1.f)
			lerp = 1.f;
		return lerp;
	}

	//-----------------------------------------------------------------------------
	// InterpolateTransform
	// Returns a transform blended between source (0.0f) and destination(1.0f).
	//-----------------------------------------------------------------------------
	HMat43 
	InterpolateTransform( HMat43& source, HMat43& destination, F32 blendValue )
	{
		HMat43 Result;
		HMat33 sourceRotation(source);
		HMat33 destinationRotation(destination);
		C_Quaternion sourceQuat(sourceRotation);
		C_Quaternion destinationQuat(destinationRotation);
		C_Quaternion blendedQuat;
		blendedQuat = sourceQuat.ShortestSlerp( destinationQuat, blendValue );
		Result = blendedQuat;
		Result.AW( source.AW() + (destination.AW() - source.AW()) * blendValue );
		return Result;
	}

	F32			
	CalculateRollOfTransform( const HMat33& transform )
	{
		HVec3 side = transform.AZ() ^ Constants< HVec3 >::UnitAxisPositiveY();
		side.Normalise();

		HVec3 up = side ^ transform.AZ();
		up.Normalise();

		F32 dot = DotProduct( up, transform.AY() );
		F32 angle = acosf( Clamped<F32>( dot, -1.0f, 1.0f ));
		return DotProduct( side, transform.AY() ) < 0.0f? -angle:angle;
	}

	HMat33
	RemoveRoll( const HMat33& Transform )
	{
		Maths::HMat33 result;
		if(fabs(Maths::DotProduct(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(),Transform.AZ())) > 0.98f)
		{
			result = Transform;
		}
		else
		{
			
			result.AZ(Transform.AZ());
			Maths::HVec3 side = Maths::Constants<Maths::HVec3>::UnitAxisPositiveY() ^ Transform.AZ();
			side.Normalise();
			result.AX(side);
			Maths::HVec3 up = Transform.AZ() ^ side; 
			up.Normalise();
			result.AY(up);
		}
		
		return result;
	}
}