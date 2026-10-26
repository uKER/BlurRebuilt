//=============================================================================
// Utility.h
// 02/02/09
//=============================================================================
#ifndef _CAMERAUTILITY_H
#define _CAMERAUTILITY_H

namespace Kanzen
{
	class SmoothingData
	{
	public:
		void Reset()
		{
			currentValue = 0.0f;
			targetValue = 0.0f;
			velocity = 0.0f;
		}

		F32 currentValue;
		F32 targetValue;
		F32 velocity;
	};

	F32		
	SmoothingFunction( F32 from, F32 to, F32& vel, F32 smoothTime, F32 timeSlice );

	F32		
	SmoothingAngleFunction( F32 from, F32 to, F32& vel, F32 smoothTime, F32 timeSlice );

	F32 
	DampingAdjust( F32 dampingCoeff, F32 timeslice );

	F32
	LerpValue(F32 period, F32 lag);

	Maths::HMat43 
	InterpolateTransform( Maths::HMat43& source, Maths::HMat43& destination, F32 blendValue );

	F32			
	CalculateRollOfTransform( const Maths::HMat33& transform );

	Maths::HMat33
	RemoveRoll( const Maths::HMat33& Transform );
}

#endif // _CAMERAUTILITY_H