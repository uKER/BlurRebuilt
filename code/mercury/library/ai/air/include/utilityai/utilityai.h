#ifndef _UTILITY_AI_H
#define _UTILITY_AI_H

#ifdef _MSC_VER
#pragma once
#endif


#include <core/core.h>
#include <core/maths.h>
#include <btl/containers/vector.h>
#include <utility>

namespace airacingtech
{
	typedef	Maths::HVec3	AIVEC;
	typedef	Maths::HF32		AIFLOAT;
	typedef	Maths::V128		AIV128;
	extern	AIVEC			g_BSplneCoeff8[8];

#define MSTOMPH(a)			((a)*(3600.f/1609.f))
#define MSTOKPH(a)		    ((a)*3.6f)
#define MPHTOMS(a)			((a)*0.446f)
#define	KPHTOMS(a)			((a)*0.277f)
#define DEGTORAD(a)         ((a)*0.0174532925f)
#define RADTODEG(a)         ((a)*57.2957795f)

	
	// TODO [Darren] : Test these curvature functions to make sure they are the same results, all suffer from some noise variation probably...
	float CalculateCurvature(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3);
	float CalculateCurvature2(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3);
	float CalculateCurvature3(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3);
	float CalculateApproxRadiusFaster(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3);
	float CalculateApproxRadius(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3);

	Maths::HVec3 HermiteCurveApprox(Maths::HVec3& P1, Maths::HVec3& P2, Maths::HVec3& T1,Maths::HVec3& T2, float t);
	Maths::HVec3 B_Spline_Approx_Calc(Maths::HVec3& P1, Maths::HVec3& P2, Maths::HVec3& P3,Maths::HVec3& P4, Maths::HF32 t);
	void B_Spline_Approx(const Btl::Vector<Maths::HVec3> &pctrlPoints, Btl::Vector<Maths::HVec3>  &samplePoints, float interval, float step);
	void B_Spline_ApproxNonLoopedOpt(const Btl::Vector<Maths::HVec3> &pctrlPoints, Btl::Vector<Maths::HVec3>  &samplePoints, float interval, float step);

	Maths::HVec3  B_Spline_EvaluatePointAheadOfPoint(
                        Maths::HVec3	inputPoint,                                 // perform calculation relative to this point on spline
                        const Maths::HVec3*	pControlPoints,
                        int			    numControlPoints,
						Maths::HF32		distanceAheadOfInputDesired,    // measured along spline
						Maths::HF32		tolerance,
						Maths::HVec3*	pClosestPointOnSplineToInputOut
					);                                  //  allowable error in mapping point to distance values ..

	int	B_Spline_GetSegmentContainingPoint(
                        Maths::HVec3	inputPoint,                                 // perform calculation relative to this point on spline
                        const Maths::HVec3*	pControlPoints,
                        int			    numControlPoints);
					

	template<typename X, typename Y>
	void B_Spline_ApproxNonLooped(const X &pctrlPoints, Y &samplePoints, float interval, float step)
	{

		const float epsilonSpline = 0.01f;
		float threshold = interval - epsilonSpline;
		Maths::HVec3 lastValidPoint;
		bool ReachedEnd = false;

		if (pctrlPoints.Size() > 1)
			lastValidPoint = pctrlPoints[1];//pctrlPoints[0];

		
		int nSteps = static_cast<int>(1.0f / step);

		for(int i = 3; i <(int)pctrlPoints.Size(); i++)
		{
			int k = i;
			int i1 = k-3;
			int i2 = k-2;
			int i3 = k-1;

			Maths::HVec3 v1 = pctrlPoints[i1];	
			Maths::HVec3 v2 = pctrlPoints[i2];
			Maths::HVec3 v3 = pctrlPoints[i3];
			Maths::HVec3 v4 = pctrlPoints[k];

			// Is this any quicker ???
			//for(float j = 0.0f; j <= 1.0f; j += step)
			for(int j=0; j<=nSteps; j++)
			{
				float accStep = j * step;
				//Maths::HVec3 result = B_Spline_Approx_Calc(v1, v2, v3, v4, j);
				Maths::HVec3 result = B_Spline_Approx_Calc(v1, v2, v3, v4, accStep);

				float dist = (lastValidPoint - result).LengthSquared();
				if (dist > threshold)
				{
					samplePoints.PushBack(result);
					lastValidPoint = result;	
				}
			}

		}
	}
	extern const Maths::HF32 three;
	extern const Maths::HF32 threeminus;
	extern const Maths::HF32 six;
	extern const Maths::HF32 four;
	extern const Maths::HF32 coeffvec;
	extern const Maths::HF32 oneVec;


	//template<typename X, typename Y>
	//void B_Spline_ApproxNonLooped_Opt(const X &pctrlPoints, Y &samplePoints, float interval, float step);

	inline Maths::HVec3 B_Spline_Approx_Calc_Opt(const Maths::HVec3& P1, const Maths::HVec3& P2, const Maths::HVec3& P3,const Maths::HVec3& P4, const Maths::HF32 t)
	{
		// [Darren] todo optimise into the vector units for xbox...

#if 0
		float one_t = 1.0f - t;
		float tt = t * t;
		float ttt = tt * t;
		float term1 = one_t * one_t * one_t;
		float term2 = 3.0f * ttt - 6.0f * tt + 4.0f;
		float term3 = -3.0f * ttt + 3.0f * tt + 3.0f * t + 1;

		Maths::HVec3 result2 =  ((P1 * term1) + (P2 * term2) + (P3 * term3) + (P4 * ttt)) * 0.16666f;
		return result2;
#else

		Maths::HF32 t_vec(t);
		Maths::HF32 one_tvec = oneVec - t_vec;
		Maths::HF32 tt_vec = t_vec * t_vec;
		Maths::HF32 ttt_vec = tt_vec * t_vec;
		Maths::HF32 term1vec = one_tvec * one_tvec * one_tvec;
		
		Maths::HF32 term2vec = three * ttt_vec - six * tt_vec + four;
		Maths::HF32 term3vec = threeminus * ttt_vec + three * tt_vec + three * t_vec + oneVec;
		Maths::HVec3 result = ((P1 * term1vec) + (P2 * term2vec) + (P3 * term3vec) + (P4 * ttt_vec)) * coeffvec;
		
		return result;
#endif
	}

	// Calculate control-point coefficients for a given 't' value, packed as a vector
	// intention is to share this calcualtion between spline segments
	inline Maths::HVec4	B_Spline_CalculateCoefficients(Maths::HF32	t) 
	{
		Maths::HF32 t_vec(t);
		Maths::HF32	one_tvec = oneVec - t_vec;
		Maths::HF32 tt_vec = t_vec * t_vec;
		Maths::HF32 ttt_vec = tt_vec * t_vec;
		Maths::HF32 term1vec = one_tvec * one_tvec * one_tvec;
		Maths::HF32 term2vec = three * ttt_vec - six * tt_vec + four;
		Maths::HF32 term3vec = threeminus * ttt_vec + three * tt_vec + three * t_vec + oneVec;

		return	Maths::HVec4(term1vec,term2vec,term3vec,ttt_vec)*coeffvec;
	}

	// matrix x point, specified as axis vectors (TODO - CHECK IF EXISTS IN CORETECH)
	inline	Maths::HVec3
		VecMultiplyPointByAxes(Maths::HVec4 point, Maths::HVec3 ax, Maths::HVec3 ay, Maths::HVec3 az, Maths::HVec3 aw)
	{
		return	point.X()*ax+point.Y()*ay+point.Z()*az+point.W()*aw;
	}

	extern void	
		B_Spline_CalculateCoefficients_X8(Maths::HVec4*	__restrict dst, int numGroupsOf8, const Maths::HF32	tstart, const Maths::HF32	dt);

	extern void	
		MultiplyArrayByAxes_X8(Maths::HVec4*	__restrict dst, 
							Maths::HVec3 ax, 
							Maths::HVec3 ay, 
							Maths::HVec3 az, 
							Maths::HVec3 aw, 
							const Maths::HVec4* __restrict src,
							int		numGroupsOf8);

	template<typename BUFFER>
	void	FilterPointsByDistanceStep(
					Maths::HVec3* lastValidPoint,
					BUFFER& outputPoints,
					const  Maths::HVec3* __restrict srcPoints,
					int numPointsDiv4,
					const Maths::HF32 distance);

	inline Maths::HF32	CalculateSum8(
							const Maths::HF32	values[8])
	{
		const Maths::HF32	v01=values[0]+values[1];
		const Maths::HF32	v23=values[2]+values[3];
		const Maths::HF32	v45=values[4]+values[5];
		const Maths::HF32	v67=values[6]+values[7];
		const Maths::HF32	v0123=v01+v23;
		const Maths::HF32	v4567=v45+v67;
		return	v0123+v4567;
	}

	extern void
		CalculateDistances_X8(
						Maths::HF32*	__restrict		dst,
						const Maths::HVec3*	__restrict	src,
						const Maths::HVec3				firstPoint,
						int	numGroupsOf8);
	extern Maths::HF32
		CalculateDistancesSummed_X8(
						Maths::HF32*	__restrict		dst,
						const Maths::HVec3*	__restrict	src,
						const Maths::HVec3				firstPoint,
						const Maths::HF32				distanceIn,
						int	numGroupsOf8);
							
	extern Maths::HF32
		CalculateDistanceSum_X8(
						const Maths::HVec3*	__restrict	src,
						const Maths::HVec3				firstPoint,
						const Maths::HF32				distanceIn,
						int	numGroupsOf8);

	
	void B_Spline_ApproxNonLooped_Stepped(
		Maths::HVec3*	pSamplePointsOut,
		int*				pNumSamplePointsOut,

		const Maths::HVec3*	pCtrlPoints, 
		int					numCtrlPoints,
		int					maxSamplePointsOut,
		float				desiredInterval,
		Maths::HVec3		origin,
		Maths::HF32			bailoutDistance);

	void B_Spline_ApproxNonLooped_Iterative(
		Maths::HVec3*	pSamplePointsOut,
		int*				pNumSamplePointsOut,

		const Maths::HVec3*	pCtrlPoints, 
		int					numCtrlPoints,
		int					maxSamplePointsOut,
		float				desiredInterval,
		Maths::HVec3		origin,
		Maths::HF32			bailoutDistance);

	AIFLOAT	B_Spline_GetLengthOfRange(AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3, AIFLOAT tstart, AIFLOAT tend);
	inline AIFLOAT B_Spline_GetLength(AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3){
		// length from 0-1 TODO -  opt with precalc factors!
		return	B_Spline_GetLengthOfRange(
			ctrl0,ctrl1,ctrl2,ctrl3, AIFLOAT(0.f), AIFLOAT(1.f));
	}
	AIFLOAT	B_Spline_GetTFromLength(
				AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3,
				AIFLOAT	length);
	inline Maths::HF32	EvalBSplinePointAheadByDist_X8(
					Maths::HVec3&	nextPointOut,
					const Maths::HVec3	c0,
					const Maths::HVec3	c1,
					const Maths::HVec3	c2,
					const Maths::HVec3	c3,
					const Maths::HF32		tstart,
					const Maths::HF32		dt,
					const Maths::HVec3		prevPoint,
					const Maths::HF32		distStepSquared);
	inline Maths::HF32	EvalBSplinePointAheadByDist_X4(
					Maths::HVec3&	nextPointOut,
					const Maths::HVec3	c0,
					const Maths::HVec3	c1,
					const Maths::HVec3	c2,
					const Maths::HVec3	c3,
					const Maths::HF32		tstart,
					const Maths::HF32		dt,
					const Maths::HVec3		prevPoint,
					const Maths::HF32		distStepSquared);


	template<typename X, typename Y>
	void B_Spline_ApproxNonLooped_Opt(const X &pctrlPoints, Y &samplePoints, float interval, float step,
								Maths::HVec3 origin,
								Maths::HF32 bailoutDistance)
	{

		const float epsilonSpline = 0.01f;
		Maths::HF32 threshold = interval - epsilonSpline;
		Maths::HVec3 lastValidPoint;
		bool ReachedEnd = false;

		if (pctrlPoints.Size() > 1)
			lastValidPoint = pctrlPoints[1];//pctrlPoints[0];

		
		int nSteps = static_cast<int>(1.0f / step);
		// round up for 4 unrolls
		nSteps = (nSteps+3)&(~3);
		float fStep = 1.0f/(float)nSteps;

		Maths::HF32	bailoutDist2 = Maths::Square(bailoutDistance);
		// [1] calculate spline scaling factors..

		const int MaxSteps=32;
		assert(nSteps<=MaxSteps);
		Maths::V128			bspline_coeff[MaxSteps];
		B_Spline_CalculateCoefficients_X8((Maths::HVec4*)bspline_coeff, (nSteps+7)/8, 0.f, 1.f/(nSteps));

		for(int i = 3; i <(int)pctrlPoints.Size(); i++)
		{
			Maths::HVec3 v1 = pctrlPoints[i-3];	
			Maths::HVec3 v2 = pctrlPoints[i-2];
			Maths::HVec3 v3 = pctrlPoints[i-1];
			Maths::HVec3 v4 = pctrlPoints[i];

			// emit no more points once this is exceeded - only need to finely subdiv the spline locally.
			if ((lastValidPoint-origin).LengthSquared()>=bailoutDist2)
				break;	

			// [1] Calculate local vertices for this spline segment

// todo - time method1 vs method2 on ppu
#define BSPLINE_APPROX_NONLOOPED_OPT_METHOD2
#ifdef BSPLINE_APPROX_NONLOOPED_OPT_METHOD1
			Maths::V128			vertices[MaxSteps];
			MultiplyArrayByAxes_X8((Maths::HVec4*) vertices, v1,v2,v3,v4,   (Maths::HVec4*) bspline_coeff, (nSteps+7)/8);
			FilterPointsByDistanceStep(&lastValidPoint, samplePoints, (Maths::HVec3*) vertices, nSteps,threshold);
#endif
#ifdef BSPLINE_APPROX_NONLOOPED_OPT_METHOD2
			Maths::HVec4*	__restrict src_coeff=(Maths::HVec4*) bspline_coeff;
			for(int j=0; j<=nSteps; j+=4,src_coeff+=4)
			{
				// [1] Calculate 4 points on the spline using precalculated coefficients..
				// TODO - extract this into a previous loop - array matrix multiply - 

				Maths::HVec3 result3 =VecMultiplyPointByAxes(src_coeff[3], v1,v2,v3,v4);
				Maths::HVec3 result1 =VecMultiplyPointByAxes(src_coeff[1], v1,v2,v3,v4);
				Maths::HVec3 result0 =VecMultiplyPointByAxes(src_coeff[0], v1,v2,v3,v4);
				Maths::HVec3 result2 =VecMultiplyPointByAxes(src_coeff[2], v1,v2,v3,v4);

				// [2] Calculate all the distances 
				//Latency 12 cycles means this is fine
				Maths::HF32 distP3 = (lastValidPoint-result3).LengthSquared();
				Maths::HF32 distP0 = (lastValidPoint-result0).LengthSquared();
				Maths::HF32 distP1 = (lastValidPoint-result1).LengthSquared();
				Maths::HF32 distP2 = (lastValidPoint-result2).LengthSquared();
				//Maths::VF32 vdist01 = (result0-result1).LengthSquared();
				//Maths::VF32 vdist02 = (result0-result2).LengthSquared();
				Maths::HF32 dist03 = (result0-result3).LengthSquared();
				//Maths::VF32 vdist12 = (result1-result2).LengthSquared();
				Maths::HF32 dist13 = (result1-result3).LengthSquared();
				Maths::HF32 dist23 = (result2-result3).LengthSquared();
				// TODO - is there a data-table solution for this...

				// [1] dist for ZERO points emited,
				if (distP3<=threshold)					// over 50% of time this is true.
					continue;
				// TODO - CHANGE RESOLUTION
#define EMIT_POINT(X) lastValidPoint = X; samplePoints.PushBack(X);
				// [2] Cases for ONE point emited - try to order the branches into a tree.
				if (distP0>threshold & dist03<=threshold){	// emit points 2 or 3, or 
					EMIT_POINT(result0);	// emit 2 only
					continue;
				}else
				if (distP1>threshold & dist13<=threshold){	// emit points 2 or 3, or 
					EMIT_POINT(result1);	// emit 2 only
					continue;
				}else
				if (distP2>threshold & dist23<=threshold){	// emit points 2 or 3, or 
					EMIT_POINT(result2);	// emit 2 only
					continue;
				}else
				if (distP3>threshold){	// Emit point 3 only ?
					EMIT_POINT(result3);
					continue;
				}
				// TODO - CHANGE RESOLUTION & REDO.
				// - halve 'jstep', halve vfStep1-4, 


				// catch all, N-points emited.. general purpose method.
				if ((lastValidPoint - result0).LengthSquared() > threshold) 
				{
					EMIT_POINT(result0);
				} 
				if ((lastValidPoint - result1).LengthSquared() > threshold)
				{
					EMIT_POINT(result1);
				}
				if ((lastValidPoint - result2).LengthSquared() > threshold)
				{
					EMIT_POINT(result2);
				}
				if ((lastValidPoint - result3).LengthSquared() > threshold)
				{
					EMIT_POINT(result3);
				}
#undef EMIT_POINT
			}
#endif
		}
	}
	template<typename BUFFER>
	void	FilterPointsByDistanceStep(
					Maths::HVec3* pLastValidPoint,
					BUFFER& outputPoints,
					const  Maths::HVec3* __restrict srcPoints, 
					int numPoints,
					const Maths::HF32 distSquared)
	{
		if (!numPoints)
			return;
		Maths::HVec3	lastValidPoint = *pLastValidPoint;
		const  Maths::HVec3* __restrict	src = srcPoints;
		const  Maths::HVec3* __restrict	end = srcPoints + numPoints;

		// [1] BATCHED x8 CASE FOR BULK OF LOOP
		while ((src+8)<end) 
		{
			// calculate next 4 distances ...

			Maths::HF32	dist7=(lastValidPoint-src[7]).LengthSquared();
			Maths::HF32	dist6=(lastValidPoint-src[6]).LengthSquared();
			Maths::HF32	dist5=(lastValidPoint-src[5]).LengthSquared();
			Maths::HF32	dist4=(lastValidPoint-src[4]).LengthSquared();
			Maths::HF32	dist3=(lastValidPoint-src[3]).LengthSquared();
			Maths::HF32	dist1=(lastValidPoint-src[2]).LengthSquared();
			Maths::HF32	dist2=(lastValidPoint-src[1]).LengthSquared();
			Maths::HF32	dist0=(lastValidPoint-src[0]).LengthSquared();

			if (dist7<=distSquared) {
				src+=8;
				continue;
			}

			// We know one of the points oversteps - which is it..
			// this is 3 vmath branches per output vertex :(
			// TODO - super optimal version using multiple vector comparisons to calculate offset for us.
			int	emit_index;
			if (dist3>distSquared) {
				if (dist1>distSquared) {
					if (dist0>distSquared)
						emit_index=0;
					else
						emit_index=1;
				} else 
				{
					if (dist2>distSquared)
						emit_index=2;
					else
						emit_index=3;
				}
			} else {
				if (dist5>distSquared) {
					if (dist4>distSquared)
						emit_index=4;
					else
						emit_index=5;
				} else 
				{
					if (dist6>distSquared)
						emit_index=6;
					else
						emit_index=7;
				}
			}

			// so - emit this point & step on.
			lastValidPoint=src[emit_index];
			src+=emit_index+1;
			outputPoints.PushBack( lastValidPoint);
		}

		// [2] BATCHED x4 CASE FOR REMAINDER
		while ((src+4)<end) 
		{
			// calculate next 4 distances ...

			Maths::HF32	dist3=(lastValidPoint-src[3]).LengthSquared();
			Maths::HF32	dist1=(lastValidPoint-src[2]).LengthSquared();
			Maths::HF32	dist2=(lastValidPoint-src[1]).LengthSquared();
			Maths::HF32	dist0=(lastValidPoint-src[0]).LengthSquared();

			if (dist3<=distSquared) {
				src+=4;
				continue;
			}

			// We know one of the points oversteps - which is it..
			// this is 3 vmath branches per output vertex :(
			// TODO - super optimal version using multiple vector comparisons to calculate offset for us.
			int	emit_index;
			if (dist1>distSquared) {
				if (dist0>distSquared)
					emit_index=0;
				else
					emit_index=1;
			} else 
			{
				if (dist2>distSquared)
					emit_index=2;
				else
					emit_index=3;
			}
			// so - emit this point & step on.
			lastValidPoint=src[emit_index];
			src+=emit_index+1;
			outputPoints.PushBack( lastValidPoint);
		}
		// [3] CATCHALL CASE FOR FINAL POINTS (if any more need emitting)
		Maths::HVec3	finalPoint = end[-1];
		if ((finalPoint-lastValidPoint).LengthSquared()>distSquared) {
			while (src<end) 
			{
				Maths::HVec3	v0 = src[0];
				Maths::HF32	dist = (lastValidPoint-v0).LengthSquared();
				Maths::HF32	final_dist = (finalPoint-v0).LengthSquared();
				if (dist>distSquared) 
				{
					lastValidPoint = v0;
					outputPoints.PushBack(lastValidPoint);
					if ((finalPoint-lastValidPoint).LengthSquared()<=distSquared)
						break;
					
				}
				src++;

			}
		}		

		*pLastValidPoint = lastValidPoint;
	};

	AIFLOAT	B_Spline_GetTOfPoint(AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3, AIVEC point, int numIterations=2);


	float CalculateSpeedMPSCamber(float curvature, float camber, float co_eff = 2.0f, float gravity = 9.81f); // Defaults...
	


	inline float CalculateSpeedMPS(float curvature, float co_eff = 2.0f, float gravity = 9.81f) // Defaults...
	{
		float c = Maths::Max(Maths::Abs(curvature), 0.0000001f);
		return Maths::Sqrt(gravity * co_eff / c); 

		// curvature is 1.0f / radius
		// v = sqrt(radius * gravity * friction)
		// v = sqrt((radius * gravity * (sin(angle) - friction * cos(angle))) / (cos(angle) + friction * sin(anlge))  )

	}

	inline float CalculateSpeedMPSY(float curvature, float curvatureH, float co_eff = 2.0f, float gravity = 9.81f) // Defaults...
	{
		float c = Maths::Max(Maths::Abs(curvature), 0.0000001f);
		float cc = Maths::Max(c + curvatureH, 0.000001f);
		return Maths::Sqrt(gravity * co_eff / cc ); 
	}


	inline float BrakingDistance(float vi, float vf, float decelleration, float error = 6.0f)
	{
		// NOTE : Assuming divide by zero cannot happen
		float distance = (((vf * vf) - (vi * vi)) / ((2.0f * decelleration) + 0.000001f)) + error; // 2 car lengths.....
		return distance;
	}

	inline float BrakingDistance(float vi, float vf, float co_eff, float gravity , float error)
	{
		float distance = (((vi * vi) -  (vf * vf)) / ((2.0f * co_eff * gravity))) + error; // 2 car lengths.....
		return distance;
	}
 
	inline Maths::HF32 SideOfLine(Maths::HVec3& testPoint, Maths::HVec3& startPoint, Maths::HVec3& endPoint)
	{
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());
		Maths::HVec3 direction = endPoint - startPoint;

		Maths::HVec3  mySideDirection(direction.Z(), zero, -direction.X());
		Maths::HVec3 offset2 = testPoint - endPoint;
		//offset2.Y(zero); // - don't need it! We're dot producting with 0 on Y-component anyway

		return offset2 | mySideDirection;
	}

	inline float FrictionModifier(float speed, float frictionMin, float frictionMax, float frictionSpeedMax)
	{
		// all in mps.....
		float f = Maths::Min(1.0f, Maths::Abs(speed) / (frictionSpeedMax) );
		float value = frictionMin * (1.0f - f) + frictionMax * f;	
	
		return value;
	}

	inline int DistanceInfrontBehind(int nearestSample, int sample, int trackLength)	
	{


		// +ve if in front of the <sample> point
		int mysample=nearestSample;
		int sep=mysample-sample;
		int tracklen= (int)trackLength;
		int testCond1 = (int)trackLength / 2;

		if (sep>testCond1)
		{
			sep-=tracklen;

			if (sep>testCond1)		// repeated for when I bias the test sample point
			{
				sep-=tracklen;
			}
		}

		if (sep<-testCond1)
		{
			sep+=tracklen;

			if (sep<-testCond1)
			{
				sep+=tracklen;
			}

		}

		return (sep);
	}

	inline float DistanceInfrontBehindF(float testSample, float sample)	
	{

		// +ve if in front of the <sample> point, otherwise negative.....
		float mysample=testSample;
		float sep=mysample-sample;
		float tracklen= 1.0f;
		float testCond1 = 0.5f;

		if (sep>testCond1)
		{
			sep-=tracklen;

			if (sep>testCond1)		// repeated for when I bias the test sample point
			{
				sep-=tracklen;
			}
		}

		if (sep<-testCond1)
		{
			sep+=tracklen;

			if (sep<-testCond1)
			{
				sep+=tracklen;
			}

		}

		return (sep);
	}

	template <typename T> T SmoothCD(const T &from, const T &to, T &vel, float smoothTime, float timeDelta)// higher the decay, smaller the result......
	{
		float omega = 2.0f/smoothTime;
		float x = omega*timeDelta;
		float exp = 1.0f/(1.0f+x+0.48f*x*x+0.235f*x*x*x);
		T change = from - to;
		T temp = (vel+omega*change)*timeDelta;
		vel = (vel - omega*temp)*exp;  
		return to + (change+temp)*exp;
	}

	template <typename T> T SmoothExp(const T& from, const T& to, float timeStep, float decayTime) // higher the decay, smaller the result......
	{
		float f = exp(-timeStep / decayTime);
		return from * f + to * (1.f - f);
	}

	inline float RandomInRange (const float fMin, const float fMax)
	{
		// TODO: seed rand.....
		int iRand = rand();

		float fRand = (float)iRand / (float)(RAND_MAX);

		float fScale = fMax - fMin;

		return fMin + (fRand * fScale);
	}

	inline int RandomInRange (const int iMin, const int iMax)
	{
		return (int)(RandomInRange((float)iMin, (float)iMax));
	}


	inline Maths::HVec3 RotateXZ( float angle, const Maths::HVec3& u)
	{
		float ssin = sin(-angle);
		float ccos = cos(-angle);

		float x = u.X() * ccos + u.Z() * ssin;
		float z = -u.X() * ssin + u.Z() * ccos;

		return Maths::HVec3( x, 0.0f, z);
	}

	//  returns positive if v2 is clockwise of v1, minus if anticlockwise
	inline int Vector2DSign(Maths::HVec3& v1, Maths::HVec3& v2)
	{
		if (v1.Z() * v2.X() > v1.X() * v2.Z())
		{ 
			return 1;
		}
		else 
		{
			return -1;
		}
	}

	inline float AngleBetween(const Maths::HVec3& Left, const Maths::HVec3&Right)
	{
		
		Maths::HVec3 v1 = Left; 
		Maths::HVec3 v2 = Right;
		v1.Normalise();
		v2.Normalise();
		float dot = v1 | v2;
		
		if(Maths::Abs(dot) >= 1.0f)
		{
			dot = 0.999f;
		}

		return acosf(dot);
	}



	inline bool QuadraticFormula(const float& a, const float& b, const float& c, float &u0, float &u1)
	{
		const float q = b*b - 4.0f*a*c;

		if (q>= 0.0f)
		{
			const float sq = Maths::Sqrt(q);				
			const float d = 1.0f / (2.0f*a);

			u0 = (-b + sq) * d;
			u1 = (-b - sq) * d;
			return true;
		}
		else
		{
			// complex roots
			return false;
		}
	}

	inline bool SphereSphereSweep(const float& ra,	Maths::HVec3& A0, Maths::HVec3& A1, const float& rb, Maths::HVec3& B0, Maths::HVec3& B1, float& u0,	float& u1, const float& tTime )
	{
		const Maths::HVec3 va = A1 - A0;
		//vector from A0 to A1

		const Maths::HVec3 vb = B1 - B0;
		//vector from B0 to B1

		const Maths::HVec3 AB = B0 - A0;
		//vector from A0 to B0

		const Maths::HVec3 vab = vb - va;
		//relative velocity 

		const float rab = ra + rb;

		const float a = vab | vab;
		//u*u coefficient

		const float b = 2.0f * (vab | AB);
		//u coefficient

		const float rabSQ = rab * rab;

		const float ABdotAB = AB | AB;

		const float c = ABdotAB - rabSQ;
		//constant term
		//check if they're currently overlapping
		if( ABdotAB <= rabSQ )
		{
			u0 = 0.01f;
			u1 = 0.01f;
			return true;
		}

		//check if they hit each other 
		if( QuadraticFormula( a, b, c, u0, u1 ) )
		{
			if( u0 > u1 )
				std::swap( u0, u1 ); // this could potentially cause problems on some stl imp
			u0 *=tTime;
			return true;
		}


		u0 = 1000.0f;

		return false;

	}

	template <class T> class Smoother
	{
	public:

		Smoother(int SampleSize, T ZeroValue):m_ZeroValue(ZeroValue),m_Next(0)
		{
			for(int i=0; i<SampleSize; i++)
			{
				m_List.PushBack(ZeroValue);
			}
		}

		T Update(const T& MostRecentValue)
		{  
			m_List[m_Next++] = MostRecentValue;

			if (m_Next == m_List.Size()) 
			{
				m_Next = 0;
			}

			T sum = m_ZeroValue;


			for(int i=0; i<m_List.Size(); ++i)
			{
				sum +=m_List[i];
			}

			return (T)(sum / m_List.Size());
		}

	private:
		Btl::Vector<T>	m_List;
		int           m_Next;
		T             m_ZeroValue;
	};


	typedef struct {float x; float y; float z;} Point;
	typedef Point PtArray[99];
	typedef Point BezArray[12];
	typedef Point SamplePoints[100];


	inline void BezierForm(int NumCtlPoints, PtArray p, BezArray c)
	{

		//   Setup Bezier coefficient array once for each control polygon. 
		int k; long n, choose=0;

		n= NumCtlPoints -1;

		for(k = 0; k <= n; k++) 
		{
			if (k == 0) 
				choose = 1;
			else if (k == 1) 
				choose = n;
			else 
				choose = choose *(n-k+1)/k;
			c[k].x = p[k].x *choose;
			c[k].y = p[k].y *choose;
			c[k].z = p[k].z *choose;   // use for 3-D curves 
		}
	}

	inline void BezierCurve(int NumCtlPoints, BezArray c, Point *pt, float t)
	{
		// Return Point pt(t), t <= 0 <= 1 from C, given the number
		//    of Points in control polygon. BezierForm must be called
		//    once for any given control polygon.
		int k, n;
		float t1, tt, u;
		BezArray b;

		n = NumCtlPoints -1;  
		u =t;
		b[0].x = c[0].x;
		b[0].y = c[0].y;
		b[0].z = c[0].z; //       for 3-D curves 

		for(k =1; k <=n; k++) 
		{
			b[k].x = c[k].x *u;
			b[k].y = c[k].y *u;
			b[k].z = c[k].z *u; //    for 3-D curves 
			u =u*t;
		}

		(*pt).x = b[n].x; 
		(*pt).y = b[n].y;
		(*pt).z = b[n].z;
		t1 = 1-t;
		tt = t1;
		for(k =n-1; k >=0; k--) 
		{
			(*pt).x += b[k].x *tt;
			(*pt).y += b[k].y *tt;
			(*pt).z += b[k].z *tt;     // Again, 3-D 
			tt =tt*t1;
		}
	}

	
	inline void BezierCurveSetupOpt(int stepSize, const Btl::Vector<Maths::HVec3>  &ctrlPoints, Btl::Vector<Maths::HVec3> &samplepoints, int sample)
	{


		int   k;
		PtArray pn;
		BezArray bc;
		Point pt;
		PtArray	m_pts;

		int numPoints = (int)ctrlPoints.Size();

		for(int i=0; i<numPoints; i++)
		{
			pn[i].x = ctrlPoints[i].X();
			pn[i].y = ctrlPoints[i].Y();
			pn[i].z = ctrlPoints[i].Z();
		}

		BezierForm(numPoints, pn, bc);

		k = sample;
		BezierCurve(numPoints, bc, &pt, (float)k/stepSize);
		m_pts[k].x = pt.x;
		m_pts[k].y = pt.y;
		m_pts[k].z = pt.z;
		Maths::HVec3 spoint(pt.x,pt.y,pt.z);
		samplepoints.PushBack(spoint);
	}

	inline void BezierCurveSetup(int stepSize, const Btl::Vector<Maths::HVec3 > &ctrlPoints, Btl::Vector<Maths::HVec3 >	&samplepoints)
	{

		int   k;
		PtArray pn;
		BezArray bc;
		Point pt;
		PtArray	m_pts;

		int numPoints = (int)ctrlPoints.Size();

		for(int i=0; i<numPoints; i++)
		{
			pn[i].x = ctrlPoints[i].X();
			pn[i].y = ctrlPoints[i].Y();
			pn[i].z = ctrlPoints[i].Z();
		}

		BezierForm(numPoints, pn, bc);

		for(k=0; k<=stepSize; k++) 
		{
			BezierCurve(numPoints, bc, &pt, (float)k/stepSize);
			m_pts[k].x = pt.x;
			m_pts[k].y = pt.y;
			m_pts[k].z = pt.z;

			Maths::HVec3 spoint(pt.x,pt.y,pt.z);
			samplepoints.PushBack(spoint);
		}

	}

	inline Maths::HF32 DistToLine(Maths::HVec3& testPoint, Maths::HVec3& startPoint, Maths::HVec3& endPoint)
	{

		//float dist= 0.0;

		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());

		Maths::HVec3 direction = endPoint - startPoint;
		//direction.Y(0.0f); //- don't need this, we're just loading it below!

		Maths::HVec3  mySideDirection(direction.Z(), zero, -direction.X());
		mySideDirection.Normalise();

		Maths::HVec3 offset2 = testPoint - endPoint;
		//offset2.Y(0.0f); // - don't need this, we're dot producting with a vector with 0 in its Y-component!

		return offset2 | mySideDirection;

	}



//	inline bool LineIntersection(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, Maths::HVec3 &intersectionPoint)
//	{
//#ifdef _XENON
//		//PixEvent pix(0xffffffff, " LineIntersection()");
//#endif
//
//		float b1, b2, a1, a2 = 0.0f;
//		float cp             = 0.0f;
//		float p1, p2, p3, p4 = 0.0f;
//		float xi, yi;
//
//		// Look for intersection  
//		b1 = (y2 - y1) / (x2 - x1);
//		b2 = (v2 - v1) / (u2 - u1);            
//
//		a1 = y1-b1*x1;
//		a2 = v1-b2*u1;
//
//		xi = -(a1-a2)/(b1-b2);
//		yi = a1+b1*xi;
//
//		p1 = (x1-xi)*(xi-x2);
//		p2 = (u1-xi)*(xi-u2);
//		p3 = (y1-yi)*(yi-y2);
//		p4 = (v1-yi)*(yi-v2);
//
//		cp = (p1 + p2 + p3 + p4) / 4.0f;
//
//		// if it crosses then determine the distance to cross point 
//		if( p1 > 0.0f && p2 > 0.0f && p3 > 0.0f && p4 > 0.0f )
//		{
//			intersectionPoint.X(xi);
//			intersectionPoint.Y(0.0f);
//			intersectionPoint.Z(yi);
//
//			return true;   
//		}
//
//		return false;
//	}

	inline bool LineIntersection(const Maths::HF32 PCREF x1, const Maths::HF32 PCREF y1, const Maths::HF32 PCREF x2, 
		const Maths::HF32 PCREF y2, const Maths::HF32 PCREF u1, const Maths::HF32 PCREF v1, const Maths::HF32 PCREF u2, 
		const Maths::HF32 PCREF v2, Maths::HVec3 &intersectionPoint)
	{

		/*float b1, b2, a1, a2 = 0.0f;
		float cp             = 0.0f;
		float p1, p2, p3, p4 = 0.0f;
		float xi, yi;*/
		const Maths::HF32 quarter(Maths::Constants<Maths::HF32>::Quarter());
		const Maths::HF32 zero(Maths::Constants<Maths::HF32>::Zero());

		// Look for intersection  
		const Maths::HF32 b1 = (y2 - y1) / (x2 - x1);
		const Maths::HF32 b2 = (v2 - v1) / (u2 - u1);            

		const Maths::HF32 a1 = y1-b1*x1;
		const Maths::HF32 a2 = v1-b2*u1;

		const Maths::HF32 xi = -(a1-a2)/(b1-b2);
		const Maths::HF32 yi = a1+b1*xi;

		const Maths::HF32 p1 = (x1-xi)*(xi-x2);
		const Maths::HF32 p2 = (u1-xi)*(xi-u2);
		const Maths::HF32 p3 = (y1-yi)*(yi-y2);
		const Maths::HF32 p4 = (v1-yi)*(yi-v2);

		//const Maths::HF32 cp = (p1 + p2 + p3 + p4) * quarter; // - we don't use this ?

		// if it crosses then determine the distance to cross point 
		const Maths::Scalar_Bool bResult(p1.GT(zero) & p2.GT(zero) & p3.GT(zero) & p4.GT(zero));
		intersectionPoint = Maths::Select(bResult, Maths::HVec3(xi, zero, yi), intersectionPoint);
		return bResult;
		/*if(  )
		{
			intersectionPoint.X(xi);
			intersectionPoint.Y(0.0f);
			intersectionPoint.Z(yi);

			return true;   
		}

		return false;*/
	}


	
	


}





#endif //_UTILITY_AI_H


