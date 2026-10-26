#include "stdafx.h"
#include <utilityai/utilityai.h>

#include <System/debug/I_DebugRender.h>

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

namespace airacingtech
{


	float CalculateSpeedMPSCamber(float curvature, float camber, float co_eff, float gravity) // Defaults...
	{
		float c = Maths::Max(Maths::Abs(curvature), 0.0000001f);
		
		float r = 1.0f / c;

		float vspd = 0.0f;
		float v = 0.0f;

		float v1 = Maths::Sqrt( Maths::Abs( (Maths::Tan(camber) + co_eff / (1.0f - co_eff * Maths::Tan(camber)) ) * r * gravity));		
		float v4 = Maths::Sqrt( Maths::Abs( (Maths::Tan(camber) - co_eff / (1.0f + co_eff * Maths::Tan(camber)) ) * r * gravity));	

		
		float v2 = Maths::Sqrt(gravity * co_eff / c);

		if (curvature < 0.0f)
		{
			
			if (camber < 0.0f)
			{
				vspd = Maths::Min(v1,v4);
				vspd = Maths::Min(vspd,v2);
			}
			else
			{
				vspd = v2;//Maths::Max(v1,v4);
			}

			
		}
		else
		{
			if (camber < 0.0f)
			{
				vspd = v2;//Maths::Max(v1,v4);
			}
			else
			{
				vspd = Maths::Min(v1,v4);
				vspd = Maths::Min(vspd,v2);
			}
		}

		return vspd;

	}


	// CalculateCurvature2 and CalculateCurvature3 are roughly the same functions...
	float CalculateCurvature2(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3)
	{

		float xp = point1.X();
		float yp = point1.Z();
		float x = point2.X();
		float y = point2.Z();
		float xn = point3.X();
		float yn = point3.Z();

		float x1 = xn - x;
		float y1 = yn - y;
		float x2 = xp - x;
		float y2 = yp - y;
		float x3 = xn - xp;
		float y3 = yn - yp;

		float det = x1 * y2 - x2 * y1;
		float n1 = x1 * x1 + y1 * y1;
		float n2 = x2 * x2 + y2 * y2;
		float n3 = x3 * x3 + y3 * y3;
		float nnn = Maths::Sqrt((n1 * n2 * n3));

		float result = 2.0f * det / nnn;

		return result;
	}

	float CalculateCurvature3(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3)
	{
		float	d1, d2, denom, fCurvature,fhill;
		Maths::HVec3	delta1, delta2;
		//
		delta1	= point2 - point1;
		delta2	= point3 - point2;
		d1		=  Maths::Sqrt(delta1.X()*delta1.X() + delta1.Z()*delta1.Z());
		d2		=  Maths::Sqrt(delta2.X()*delta2.X() + delta2.Z()*delta2.Z());
		if (d1*d2>0)
		{
			denom	= d1+d2;

			float	xdot	= 1.0f / denom * (d2/d1*delta1.X() + d1/d2*delta2.X());
			float	ydot	= 1.0f / denom * (d2/d1*delta1.Z() + d1/d2*delta2.Z());
			float	xdotdot =  2.0f / denom * (delta2.X()/d2 - delta1.X()/d1);
			float	ydotdot =  2.0f / denom * (delta2.Z()/d2 - delta1.Z()/d1);
			//
			fCurvature 	= (ydotdot*xdot - xdotdot*ydot);
			fhill		= 2.0f / denom * (delta2.Y()/d2 - delta1.Y()/d1);
		}
		else
		{
			fCurvature = fhill = 0.0f;
		}


		return fCurvature;
	}

	float CalculateCurvature(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3)
	{
		// lets calculate the curvature
		Maths::HVec3 p3 = point3 - point1;

		Maths::HVec3 up(0.0f,1.0f,0.0f);
		Maths::HVec3 n = (p3 ^ up);
		n.Normalise();

		float l = p3.Length() / 2.0f;
		float  t = (point2 - point1) | n;
		float curvature = (t / (l * l + t * t)) * 2.0f; // c = 1.0f/radius

		return curvature;
	}

	// TODO [Darren] : Test these 2 functions to make sure they are the same results...
	float CalculateApproxRadiusFaster(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3)
	{
		Maths::HVec3 p3 = point3 - point1;
		float chord = p3.Length();        
		Maths::HVec3 distanceA = p3;
		distanceA *= 0.5f;  
		distanceA += point1; 

		Maths::HVec3 distA = distanceA - point2;
		float mc = distA.Length();
		float radius = ((mc/2.0f) + (chord/(8.0f*mc)));
		return radius;
	}


	float CalculateApproxRadius(Maths::HVec3& point1, Maths::HVec3& point2, Maths::HVec3& point3)
	{
		Maths::HVec3 p3 = point3 - point1;
		float chord = p3.Length();        
		Maths::HVec3 distanceA = p3;
		distanceA *= 0.5f;  
		distanceA += point1; 

		Maths::HVec3 distA = distanceA - point2;
		float mc = distA.Length();
		float radius = ((mc/2.0f) + ((chord*chord)/(8.0f*mc)));
		return radius;
	}

	//float CalculateSpeedMPS(float curvature, float co_eff, float gravity) // Defaults...
	//{
	//	return Maths::Sqrt(gravity * co_eff / Maths::Abs(curvature)); 
	//}

	Maths::HVec3 HermiteCurveApprox(Maths::HVec3& P1, Maths::HVec3& P2, Maths::HVec3& T1,Maths::HVec3& T2, float t)
	{
		// calculate blending functions
		float b0 =  2.0f*t*t*t - 3.0f*t*t + 1;
		float b1 = -2.0f*t*t*t + 3.0f*t*t;
		float b2 = t*t*t - 2.0f*t*t + t;
		float b3 = t*t*t - t*t;

		Maths::HVec3 result = (P1 * b0) + (P2 * b1) + (T1 * b2) + (T2 * b3);

		return result;
	}

	inline Maths::HF32	VecDist2(Maths::HVec3 a, Maths::HVec3 b) {
		return	(a-b).LengthSquared();
	}

	void	B_Spline_Calc_X8(
				Maths::HVec3*	dst,
				Maths::HVec3	c0,
				Maths::HVec3	c1,
				Maths::HVec3	c2,
				Maths::HVec3	c3,
				Maths::HF32	t0,
				Maths::HF32	dt)
	{
		
		Maths::HF32	dt2=dt+dt;
		Maths::HF32	dt3=dt2+dt;
		Maths::HF32	dt4=dt2+dt2;
		Maths::HF32	dt5=dt2+dt3;
		Maths::HF32	dt6=dt3+dt3;
		Maths::HF32	dt7=dt3+dt4;

		dst[0]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0);
		dst[1]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0+dt);
		dst[2]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0+dt2);
		dst[3]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0+dt3);
		dst[4]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0+dt4);
		dst[5]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0+dt5);
		dst[6]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0+dt6);
		dst[7]=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0+dt7);
	}


	
    inline      Maths::V128 VecOr(Maths::V128 a,Maths::V128 b) {
#if defined( _SPU_ )
     return      spu_or(     a,b   );
#elif defined( _PPU_ )
     return vec_vor(   a,b );
#elif defined( _VMX128_ )
     return __vor(     a,b );
#elif defined( _SSE_ )
     return SIMD_OR(   a,b );
#else
     Maths::V128 ret;
	(U32&)ret.m_X = (U32&)a.m_X | (U32&)b.m_X;
	(U32&)ret.m_Y = (U32&)a.m_Y | (U32&)b.m_Y;
	(U32&)ret.m_Z = (U32&)a.m_Z | (U32&)b.m_Z;
	(U32&)ret.m_W = (U32&)a.m_W | (U32&)b.m_W;
	return      ret;
#endif
	}

	inline      Maths::V128 VecAnd(Maths::V128 a,Maths::V128 b) {
#if defined( _SPU_ )
    return      spu_and(    a,b   );
#elif defined( _PPU_ )
    return vec_and(   a,b );
#elif defined( _VMX128_ )
    return __vand(    a,b );
#elif defined( _SSE_ )
    return SIMD_AND(  a,b );
#else
    Maths::V128 ret;
	(U32&)ret.m_X = (U32&)a.m_X & (U32&)b.m_X;
	(U32&)ret.m_Y = (U32&)a.m_Y & (U32&)b.m_Y;
	(U32&)ret.m_Z = (U32&)a.m_Z & (U32&)b.m_Z;
	(U32&)ret.m_W = (U32&)a.m_W & (U32&)b.m_W;
	return      ret;
#endif
      }

      inline      Maths::V128 VecXor(Maths::V128 a,Maths::V128 b) {
#if defined( _SPU_ )
      return      spu_xor(    a,b   );
#elif defined( _PPU_ )
      return vec_xor(   a,b );
#elif defined( _VMX128_ )
      return __vxor(    a,b );
#elif defined( _SSE_ )
      return SIMD_XOR(  a,b );
#else   
     Maths::V128 ret;
	(U32&)ret.m_X = (U32&)a.m_X ^ (U32&)b.m_X;
	(U32&)ret.m_Y = (U32&)a.m_Y ^ (U32&)b.m_Y;
	(U32&)ret.m_Z = (U32&)a.m_Z ^ (U32&)b.m_Z;
	(U32&)ret.m_W = (U32&)a.m_W ^ (U32&)b.m_W;
	return      ret;
#endif
      }

      inline      Maths::V128 VecNot(Maths::V128 a) {
#if defined( _SPU_ )
        return      spu_nor(    a,a   );
#elif defined( _PPU_ )
        return vec_nor(   a,a );
#elif defined( _VMX128_ )
        return __vnor(    a,a );
#elif defined( _SSE_ )
        return SIMD_NOR(  a,a );
#else   
     Maths::V128 ret;
	(U32&)ret.m_X = !(U32&)a.m_X;
	(U32&)ret.m_Y = !(U32&)a.m_Y;
	(U32&)ret.m_Z = !(U32&)a.m_Z;
	(U32&)ret.m_W = !(U32&)a.m_W;
	return      ret;
#endif
      }

	inline AIV128	VecOr(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d)
	{
		return	VecOr(VecOr(a,b),VecOr(c,d));
	}
	inline AIV128	VecOr( AIV128	a,AIV128	b,AIV128	c) {
		return	VecOr(VecOr(a,b),c);
	}

	inline AIV128	VecOr(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d,
							AIV128	e,
							AIV128	f,
							AIV128	g,
							AIV128	h
							)
	{
		return	VecOr(VecOr(a,b,c,d),VecOr(e,f,g,h));
	}
	inline AIV128	VecOr(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d,
							AIV128	e,
							AIV128	f
							)
	{
		return	VecOr(VecOr(a,b,c,d),VecOr(e,f));
	}
	inline AIV128	VecOr(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d,
							AIV128	e,
							AIV128	f,
							AIV128	g
							)
	{
		return	VecOr(VecOr(a,b,c,d),VecOr(e,f,g));
	}

	inline AIV128	VecAnd(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d)
	{
		return	VecAnd(VecAnd(a,b),VecAnd(c,d));
	}
	inline AIV128	VecAnd( AIV128	a,AIV128	b,AIV128	c) {
		return	VecAnd(VecAnd(a,b),c);
	}
	inline AIV128	VecAnd(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d,
							AIV128	e,
							AIV128	f
							)
	{
		return	VecAnd(VecAnd(a,b,c,d),VecAnd(e,f));
	}

	inline AIV128	VecAnd(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d,
							AIV128	e,
							AIV128	f,
							AIV128	g,
							AIV128	h
							)
	{
		return	VecAnd(VecAnd(a,b,c,d),VecAnd(e,f,g,h));
	}

	inline AIV128	VecAnd(	AIV128	a,
							AIV128	b,
							AIV128	c,
							AIV128	d,
							AIV128	e,
							AIV128	f,
							AIV128	g
							)
	{
		return	VecAnd(VecAnd(a,b,c,d),VecAnd(e,f,g));
	}
	inline AIV128	VecSelectFromList7(
						AIV128	b0,AIV128	b1,AIV128	b2,AIV128	b3,AIV128	b4,AIV128	b5,AIV128	b6,
						AIV128	v0,	AIV128	v1,AIV128	v2,	AIV128	v3,	AIV128	v4,AIV128	v5,	AIV128	v6)
	{
		return	VecOr(
					VecAnd(b0,v0),
					VecAnd(b0,v1),
					VecAnd(b1,v2),
					VecAnd(b2,v2),
					VecAnd(b3,v3),
					VecAnd(b4,v4),
					VecAnd(b5,v5),
					VecAnd(b6,v6)
					);
	}
	inline	AIV128	VecSelectFromList8(
						AIV128	b0,AIV128	b1,AIV128	b2,AIV128	b3,AIV128	b4,AIV128	b5,AIV128	b6, AIV128	b7,
						AIV128	v0,	AIV128	v1,AIV128	v2,	AIV128	v3,	AIV128	v4,AIV128	v5,	AIV128	v6, AIV128	v7)
	{
		return	VecOr(
						VecAnd(b0, v0),
						VecAnd(b1, v1),
						VecAnd(b2, v2),
						VecAnd(b3, v3),
						VecAnd(b4, v4),
						VecAnd(b5, v5),
						VecAnd(b6, v6),
						VecAnd(b7, v7)
				);
	};


	// TODO - X4 Version will be more suitable for PPU?
	inline Maths::HF32	EvalBSplinePointAheadByDist_X4(
					Maths::HVec3&	newPointOut,
					const Maths::HVec3	c0,
					const Maths::HVec3	c1,
					const Maths::HVec3	c2,
					const Maths::HVec3	c3,
					const Maths::HF32		tstart,
					const Maths::HF32		dt,
					const Maths::HVec3	lastPoint,
					const Maths::HF32		distStepSquared)
	{
		Maths::HF32		t0=tstart;



		Maths::HF32	dt2=dt+dt;
		Maths::HF32	dt3=dt2+dt;
		Maths::HF32	dt4=dt2+dt2;
		Maths::HF32	t1=t0+dt;
		Maths::HF32	t2=t0+dt2;
		Maths::HF32	t3=t0+dt3;

		Maths::HVec3	v0=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0);
		Maths::HVec3	v1=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t1);
		Maths::HVec3	v2=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t2);
		Maths::HVec3	v3=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t3);

		Maths::HF32	d0 = VecDist2(lastPoint, v0);
		Maths::HF32	d1 = VecDist2(lastPoint, v1);
		Maths::HF32	d2 = VecDist2(lastPoint, v2);
		Maths::HF32	d3 = VecDist2(lastPoint, v3);

		Maths::Scalar_Bool	b0 = d0>distStepSquared;
		Maths::Scalar_Bool	b1 = d1>distStepSquared;
		Maths::Scalar_Bool	b2 = d2>distStepSquared;
		Maths::Scalar_Bool	b3 = d3>distStepSquared;
		// only leave a '1' at the point where the distance was breached

// ADD THESE METHODS TO Maths::Scalar_Bool - accesors to allow scalar bool to be used in proper low level logic
//    #if defined( _SIMD_ )
//    NOALIAS __forceinline const V128            GetV128() const __restrict                  { return m_V; }
//    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_V = src; }
//    #else
//    NOALIAS __forceinline const V128            GetV128() const __restrict                  { const V128 v = { m_V[0], m_V[1], m_V[2], 0.0f }; return v; }
//    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { *this = HVec3(src); }
//    #endif

		Maths::V128	vb3 = VecXor(b3.GetV128(), b2.GetV128());
		Maths::V128	vb2 = VecXor(b2.GetV128(), b1.GetV128());
		Maths::V128	vb1 = VecXor(b1.GetV128(), b0.GetV128());
		Maths::V128	vb0 = b0.GetV128();

		Maths::V128	vresult	=	
				VecOr(
						VecAnd(vb0, v0.GetV128()),
						VecAnd(vb1, v1.GetV128()),
						VecAnd(vb2, v2.GetV128()),
						VecAnd(vb3, v3.GetV128())
				);
		Maths::V128	tresult=VecOr(
					VecAnd(vb0, t0.GetV128()),
					VecAnd(vb1, t1.GetV128()),
					VecAnd(vb2, t2.GetV128()),
					VecAnd(vb3, t3.GetV128())
				);


		newPointOut.SetV128(vresult);
		return	Maths::HF32(tresult);
	};

	inline AIFLOAT	VecLengthBetweenPoints(
						AIVEC	v0,
						AIVEC	v1,
						AIVEC	v2,
						AIVEC	v3,
						AIVEC	v4,
						AIVEC	v5,
						AIVEC	v6,
						AIVEC	v7)
	{
		return	((v0-v1).Length()+(v1-v2).Length())+((v2-v3).Length()+(v3-v4).Length())+((v4-v6).Length()+(v6-v7).Length());
	}

	inline Maths::HF32	B_Spline_GetArcLength(AIVEC	c0,AIVEC	c1,AIVEC	c2,AIVEC	c3, AIFLOAT tlo, AIFLOAT thi)
	{
			AIFLOAT	dt = thi-tlo;

			return	VecLengthBetweenPoints(
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	tlo),
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	tlo+AIFLOAT(1.f/7.f)*dt),
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	tlo+AIFLOAT(1.f/7.f)*dt),
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	tlo+AIFLOAT(1.f/7.f)*dt),
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	tlo+AIFLOAT(1.f/7.f)*dt),
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	tlo+AIFLOAT(1.f/7.f)*dt),
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	tlo+AIFLOAT(1.f/7.f)*dt),
				B_Spline_Approx_Calc_Opt(c0,c1,c2,c3,	thi)
			);
			
				
	};

	inline AIFLOAT  B_Spline_ArcLength_PrecalcCoeff8(AIVEC	c0,AIVEC	c1,AIVEC	c2,AIVEC	c3, const Maths::HVec4* coeff)
	{
		return VecLengthBetweenPoints(
			VecMultiplyPointByAxes(coeff[0], c0,c1,c2,c3),
			VecMultiplyPointByAxes(coeff[1], c0,c1,c2,c3),
			VecMultiplyPointByAxes(coeff[2], c0,c1,c2,c3),
			VecMultiplyPointByAxes(coeff[3], c0,c1,c2,c3),
			VecMultiplyPointByAxes(coeff[4], c0,c1,c2,c3),
			VecMultiplyPointByAxes(coeff[5], c0,c1,c2,c3),
			VecMultiplyPointByAxes(coeff[6], c0,c1,c2,c3),
			VecMultiplyPointByAxes(coeff[7], c0,c1,c2,c3));
	}


	// TODO - X4 Version will be more suitable for PPU?
	inline Maths::HF32	EvalBSplinePointAheadByDist_X8(
					Maths::HVec3&	newPointOut,
					const Maths::HVec3	c0,
					const Maths::HVec3	c1,
					const Maths::HVec3	c2,
					const Maths::HVec3	c3,
					const Maths::HF32		tstart,
					const Maths::HF32		dt,
					const Maths::HVec3	lastPoint,
					const Maths::HF32		distStepSquared)
	{
		Maths::HF32		t0=tstart;


		// prime

		Maths::HF32	dt2=dt+dt;
		Maths::HF32	dt3=dt2+dt;
		Maths::HF32	dt4=dt2+dt2;
		Maths::HF32	t1=t0+dt;
		Maths::HF32	t2=t0+dt2;
		Maths::HF32	t3=t0+dt3;
		Maths::HF32	t4=t0+dt4;
		Maths::HF32	t5=t4+dt;
		Maths::HF32	t6=t4+dt2;
		Maths::HF32	t7=t4+dt3;


		Maths::HVec3	v0=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t0);
		Maths::HVec3	v1=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t1);
		Maths::HVec3	v2=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t2);
		Maths::HVec3	v3=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t3);
		Maths::HVec3	v4=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t4);
		Maths::HVec3	v5=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t5);
		Maths::HVec3	v6=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t6);
		Maths::HVec3	v7=B_Spline_Approx_Calc_Opt(c0,c1,c2,c3, t7);

		Maths::HF32	d0 = VecDist2(lastPoint, v0);
		Maths::HF32	d1 = VecDist2(lastPoint, v1);
		Maths::HF32	d2 = VecDist2(lastPoint, v2);
		Maths::HF32	d3 = VecDist2(lastPoint, v3);
		Maths::HF32	d4 = VecDist2(lastPoint, v4);
		Maths::HF32	d5 = VecDist2(lastPoint, v5);
		Maths::HF32	d6 = VecDist2(lastPoint, v6);
		Maths::HF32	d7 = VecDist2(lastPoint, v7);

		Maths::Scalar_Bool	b0 = d0>distStepSquared;
		Maths::Scalar_Bool	b1 = d1>distStepSquared;
		Maths::Scalar_Bool	b2 = d2>distStepSquared;
		Maths::Scalar_Bool	b3 = d3>distStepSquared;
		Maths::Scalar_Bool	b4 = d4>distStepSquared;
		Maths::Scalar_Bool	b5 = d5>distStepSquared;
		Maths::Scalar_Bool	b6 = d6>distStepSquared;
		Maths::Scalar_Bool	b7 = d7>distStepSquared;
		// only leave a '1' at the point where the distance was breached

// ADD THESE METHODS TO Maths::Scalar_Bool - accesors to allow scalar bool to be used in proper low level logic
//    #if defined( _SIMD_ )
//    NOALIAS __forceinline const V128            GetV128() const __restrict                  { return m_V; }
//    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { m_V = src; }
//    #else
//    NOALIAS __forceinline const V128            GetV128() const __restrict                  { const V128 v = { m_V[0], m_V[1], m_V[2], 0.0f }; return v; }
//    NOALIAS __forceinline void                  SetV128( const V128 PCREF src ) __restrict  { *this = HVec3(src); }
//    #endif

		Maths::V128	vb7 = VecXor(b7.GetV128(), b6.GetV128());
		Maths::V128	vb6 = VecXor(b6.GetV128(), b5.GetV128());
		Maths::V128	vb5 = VecXor(b5.GetV128(), b4.GetV128());
		Maths::V128	vb4 = VecXor(b4.GetV128(), b3.GetV128());
		Maths::V128	vb3 = VecXor(b3.GetV128(), b2.GetV128());
		Maths::V128	vb2 = VecXor(b2.GetV128(), b1.GetV128());
		Maths::V128	vb1 = VecXor(b1.GetV128(), b0.GetV128());
		Maths::V128	vb0 = b0.GetV128();

		Maths::V128	vresult	= VecSelectFromList8(vb0,vb1,vb2,vb3,vb4,vb5,vb6,vb7, 
							v0.GetV128(),
							v1.GetV128(),
							v2.GetV128(),
							v3.GetV128(),
							v4.GetV128(),
							v5.GetV128(),
							v6.GetV128(),
							v7.GetV128());
		Maths::V128	tresult=VecOr(
					VecAnd(vb0, t0.GetV128()),
					VecAnd(vb1, t1.GetV128()),
					VecAnd(vb2, t2.GetV128()),
					VecAnd(vb3, t3.GetV128()),
					VecAnd(vb4, t4.GetV128()),
					VecAnd(vb5, t5.GetV128()),
					VecAnd(vb6, t6.GetV128()),
					VecAnd(vb7, t7.GetV128())
				);


		newPointOut.SetV128(vresult);
		return	Maths::HF32(tresult);
	};

	void B_Spline_ApproxNonLooped_Stepped(
		Maths::HVec3*	pSamplePointsOut,
		int*				pNumSamplePointsOut,

		const Maths::HVec3*	pCtrlPoints, 
		int					numCtrlPoints,
		int					maxSamplePointsOut,
		float				desiredInterval,
		Maths::HVec3		origin,
		Maths::HF32			bailoutDistance)
	{
		Maths::HVec3*	pDstEnd = pSamplePointsOut+maxSamplePointsOut;
		Maths::HVec3*	pDst=pSamplePointsOut;

		const Maths::HVec3*	pCtrlPointsEnd=pCtrlPoints+numCtrlPoints-4;

		Maths::HF32		dt=0.05f;
		Maths::HF32		distStepSquared=desiredInterval*desiredInterval;
		Maths::HF32		t=dt;
		Maths::HVec3	lastPoint = B_Spline_Approx_Calc_Opt(pCtrlPoints[0],pCtrlPoints[1],pCtrlPoints[2],pCtrlPoints[3], t);
		Maths::HVec3	nextPoint;
		while  (pDst<pDstEnd ) 
		{
			// TODO - add t delta table for uneven spacing (exp), 
			//	and ability to iterate this for 2 stage refinement.
			t = EvalBSplinePointAheadByDist_X8(
						nextPoint,
						pCtrlPoints[0],pCtrlPoints[1],pCtrlPoints[2],pCtrlPoints[3],
						t,
						dt,
						lastPoint,
						distStepSquared);
			if (t < oneVec) 
			{
				*pDst = lastPoint=nextPoint;
				pDst++;
				continue;
			}
			else {
				// move to next spline control point, & discard the point that just got emitted
				t-=oneVec;
				pCtrlPoints++;
				if (pCtrlPoints>=pCtrlPointsEnd)
					break;
			}

			
		}
		*pNumSamplePointsOut = pDst-pSamplePointsOut;

	}
	void B_Spline_ApproxNonLooped_Iterative(
		Maths::HVec3*	pSamplePointsOut,
		int*				pNumSamplePointsOut,

		const Maths::HVec3*	pCtrlPoints, 
		int					numCtrlPoints,
		int					maxSamplePointsOut,
		float				desiredInterval,
		Maths::HVec3		origin,
		Maths::HF32			bailoutDistance)
	{
		Maths::HVec3*	pDstEnd = pSamplePointsOut+maxSamplePointsOut;
		Maths::HVec3*	pDst=pSamplePointsOut;

		const Maths::HVec3*	pCtrlPointsEnd=pCtrlPoints+numCtrlPoints-4;

		Maths::HF32		dt=0.125f;
		Maths::HF32		distStepSquared=desiredInterval*desiredInterval;
		Maths::HF32		t=dt;
		Maths::HVec3	lastPoint = B_Spline_Approx_Calc_Opt(pCtrlPoints[0],pCtrlPoints[1],pCtrlPoints[2],pCtrlPoints[3], t);
		Maths::HVec3	nextPoint;
		Maths::HF32		fine_dt=dt*0.25f;
		while  (pDst<pDstEnd ) 
		{
			// TODO - add t delta table for uneven spacing (exp), 
			//	and ability to iterate this for 2 stage refinement.
			t = EvalBSplinePointAheadByDist_X4(
					nextPoint,
					pCtrlPoints[0],pCtrlPoints[1],pCtrlPoints[2],pCtrlPoints[3],
					t,
					dt,
					lastPoint,
					distStepSquared);

			t = EvalBSplinePointAheadByDist_X4(
					nextPoint,
					pCtrlPoints[0],pCtrlPoints[1],pCtrlPoints[2],pCtrlPoints[3],
					t-fine_dt*3.f,
					fine_dt,
					lastPoint,
					distStepSquared);

			if (t < oneVec) 
			{
				*pDst = lastPoint=nextPoint;
				pDst++;
				continue;
			}
			else {
				// move to next spline control point, & discard the point that just got emitted
				t-=oneVec;
				pCtrlPoints++;
				if (pCtrlPoints>=pCtrlPointsEnd)
					break;
			}

			
		}
		*pNumSamplePointsOut = pDst-pSamplePointsOut;

	}


	const Maths::HF32 three(3.0f);
	const Maths::HF32 threeminus(-3.0f);
	const Maths::HF32 six(6.0f);
	const Maths::HF32 four(4.0f);
	const Maths::HF32 coeffvec(0.16666f);
	const Maths::HF32 oneVec(1.0f);

	void	B_Spline_CalculateCoefficients_X8(Maths::HVec4*	__restrict dst, int numGroupsOf8, const Maths::HF32	tstart, const Maths::HF32	dt)
	{
		Maths::HF32	t	=tstart;
		const Maths::HF32	dt2	=dt+dt;
		const Maths::HF32	dt3	=dt2+dt;
		const Maths::HF32	dt4	=dt2+dt2;
		const Maths::HF32	dt5	=dt2+dt3;
		const Maths::HF32	dt6	=dt3+dt3;
		const Maths::HF32	dt7	=dt3+dt4;
		const Maths::HF32	dt8	=dt4+dt4;

		int i;
		for (i=0; i<numGroupsOf8; i++,t+=dt8, dst+=8) 
		{
			// TODO - SOA CALCULATION OF COEFFICIENTS
			dst[0] = B_Spline_CalculateCoefficients(t);
			dst[1] = B_Spline_CalculateCoefficients(t+dt);
			dst[2] = B_Spline_CalculateCoefficients(t+dt2);
			dst[3] = B_Spline_CalculateCoefficients(t+dt3);
			dst[4] = B_Spline_CalculateCoefficients(t+dt4);
			dst[5] = B_Spline_CalculateCoefficients(t+dt5);
			dst[6] = B_Spline_CalculateCoefficients(t+dt6);
			dst[7] = B_Spline_CalculateCoefficients(t+dt7);
		}
	};

	void	MultiplyArrayByAxes_X8(Maths::HVec4*	__restrict dst, 
							Maths::HVec3	ax, 
							Maths::HVec3	ay, 
							Maths::HVec3	az, 
							Maths::HVec3	aw, 
							const Maths::HVec4* __restrict src,
							int		numGroupsOf8)
	{
		int	i;
		for (i=0; i<numGroupsOf8; i++, dst+=8, src+=8) 
		{
			dst[0] = VecMultiplyPointByAxes(src[0], ax,ay,az,aw);
			dst[1] = VecMultiplyPointByAxes(src[1], ax,ay,az,aw);
			dst[2] = VecMultiplyPointByAxes(src[2], ax,ay,az,aw);
			dst[3] = VecMultiplyPointByAxes(src[3], ax,ay,az,aw);
			dst[4] = VecMultiplyPointByAxes(src[4], ax,ay,az,aw);
			dst[5] = VecMultiplyPointByAxes(src[5], ax,ay,az,aw);
			dst[6] = VecMultiplyPointByAxes(src[6], ax,ay,az,aw);
			dst[7] = VecMultiplyPointByAxes(src[7], ax,ay,az,aw);
		}
	}

	void
		CalculateDistancesToPoint_X8(
						Maths::HF32*	__restrict dst,
						const Maths::HVec3*	__restrict src,
						const Maths::HVec3		point,
						int	numGroupsOf8)
	{
		int	i;
		for (i=0; i<numGroupsOf8; i++, dst+=8,src+=8) 
		{
			dst[0] = (src[0]-point).Length();
			dst[1] = (src[1]-point).Length();
			dst[2] = (src[2]-point).Length();
			dst[3] = (src[3]-point).Length();
			dst[4] = (src[4]-point).Length();
			dst[5] = (src[5]-point).Length();
			dst[6] = (src[6]-point).Length();
			dst[7] = (src[7]-point).Length();
		}
	};
	void
		CalculateDistances_X8(
						Maths::HF32*	__restrict dst,
						const Maths::HVec3*	__restrict src,
						const Maths::HVec3		firstPoint0,
						int	numGroupsOf8)
	{
		int	i;
		Maths::HVec3		firstPoint=firstPoint0;
		for (i=0; i<numGroupsOf8; i++, dst+=8,src+=8) 
		{
			dst[0] = (src[0]-firstPoint).Length();
			dst[1] = (src[1]-src[0]).Length();
			dst[2] = (src[2]-src[1]).Length();
			dst[3] = (src[3]-src[2]).Length();
			dst[4] = (src[4]-src[3]).Length();
			dst[5] = (src[5]-src[4]).Length();
			dst[6] = (src[6]-src[5]).Length();
			dst[7] = (src[7]-src[6]).Length();
			firstPoint=src[7];
		}
	};
	Maths::HF32
		CalculateDistancesSummed_X8(
						Maths::HF32*	__restrict dst,
						const Maths::HVec3*	__restrict src,
						const Maths::HVec3		firstPointIn,
						const Maths::HF32		distancePrev,
						int	numGroupsOf8)
	{
		int	i;
		Maths::HVec3		firstPoint=firstPointIn;
		for (i=0; i<numGroupsOf8; i++, dst+=8,src+=8) 
		{
			
			Maths::HF32	d0 = (src[0]-firstPoint).Length()+distancePrev;
			Maths::HF32	d1 = (src[1]-src[0]).Length();
			Maths::HF32	d2 = (src[2]-src[1]).Length();
			Maths::HF32	d3 = (src[3]-src[2]).Length();
			Maths::HF32	d4 = (src[4]-src[3]).Length();
			Maths::HF32	d5 = (src[5]-src[4]).Length();
			Maths::HF32	d6 = (src[6]-src[5]).Length();
			Maths::HF32	d7 = (src[7]-src[6]).Length();

			Maths::HF32	d01=d0+d1;
			Maths::HF32	d23=d2+d3;
			Maths::HF32	d45=d4+d5;
			Maths::HF32	d67=d6+d7;
			//<stall>
			Maths::HF32	d0123=d01+d23;
			Maths::HF32	d4567=d45+d67;
			Maths::HF32	d456=d45+d6;
			//<stall>
			dst[0]	= d0;
			dst[1]	= d01;
			dst[2]	= d01+d2;
			dst[3]	= d0123;
			dst[4]	= d0123	+ d4;
			dst[5]	= d0123	+ d45;
			dst[6]	= d0123	+ d456;

			Maths::HF32	distancePrev = d0123+d4567;
			dst[7] = distancePrev;
			
			firstPoint=src[7];
		}
		return	distancePrev;
	};

#define	BSPLINE_EVAL_T_RANGE(ctrl0,ctrl1,ctrl2,ctrl3, tlo,thi) \
		AIFLOAT	dt = thi-tlo;\
		AIFLOAT	t0 = tlo;\
		AIFLOAT	t1 = tlo+AIFLOAT(1.f/7.f)*dt;\
		AIFLOAT	t2 = tlo+AIFLOAT(2.f/7.f)*dt;\
		AIFLOAT	t3 = tlo+AIFLOAT(3.f/7.f)*dt;\
		AIFLOAT	t4 = tlo+AIFLOAT(4.f/7.f)*dt;\
		AIFLOAT	t5 = tlo+AIFLOAT(5.f/7.f)*dt;\
		AIFLOAT	t6 = tlo+AIFLOAT(6.f/7.f)*dt;\
		AIFLOAT	t7 = thi;\
		AIVEC	v0 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t0);\
		AIVEC	v1 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t1);\
		AIVEC	v2 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t2);\
		AIVEC	v3 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t3);\
		AIVEC	v4 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t4);\
		AIVEC	v5 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t5);\
		AIVEC	v6 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t6);\
		AIVEC	v7 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t7);
#define BSPLINE_GET_DISTANCES \
		AIFLOAT	d0=(v0-v1).Length(); \
		AIFLOAT	d1=(v1-v2).Length(); \
		AIFLOAT	d2=(v2-v3).Length(); \
		AIFLOAT	d3=(v3-v4).Length(); \
		AIFLOAT	d4=(v4-v5).Length(); \
		AIFLOAT	d5=(v5-v6).Length(); \
		AIFLOAT	d6=(v6-v7).Length();

	AIFLOAT	B_Spline_GetLengthOfRange(AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3, AIFLOAT tlo, AIFLOAT thi) 
	{
		BSPLINE_EVAL_T_RANGE(ctrl0,ctrl1,ctrl2,ctrl3, tlo,thi)
		BSPLINE_GET_DISTANCES
		return	((d0+d1)+(d2+d3))+((d4+d5)+d6);
	}

	AIFLOAT	B_Spline_GetTFromLength(
				AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3,
				AIFLOAT	length)
	{
		AIFLOAT	tret;
		int	i;
		AIFLOAT	tlo(0.f);
		AIFLOAT	thi(1.f);
		for (i=0; i<2; i++) 
		{
			// TODO - OPT USING FULL RANGE 
			BSPLINE_EVAL_T_RANGE(ctrl0,ctrl1,ctrl2,ctrl3, tlo,thi)
			BSPLINE_GET_DISTANCES
			AIFLOAT	d01=d0+d1;
			AIFLOAT	d23=d2+d3;
			AIFLOAT	d45=d4+d5;
			AIFLOAT	d012=d01+d2;
			AIFLOAT	d456=d45+d6;

			AIFLOAT	d0123=d01+d23;

			AIFLOAT	dist_till_t0	=	0.f;
			AIFLOAT	dist_till_t1	=	d0;
			AIFLOAT	dist_till_t2	=	d01;
			AIFLOAT	dist_till_t3	=	d012;
			AIFLOAT	dist_till_t4	=	d0123;
			AIFLOAT	dist_till_t5	=	d0123+d4;
			AIFLOAT	dist_till_t6	=	d0123+d45;
			AIFLOAT	dist_till_t7	=	d0123+d456;

			// USE SELECTS TO GET DISTANCES UP TILL POINT
			AIV128	b0	= (length>AIFLOAT(0.f)).GetV128();
			AIV128	b1	= (length>dist_till_t1).GetV128();
			AIV128	b2	= (length>dist_till_t2).GetV128();
			AIV128	b3	= (length>dist_till_t3).GetV128();
			AIV128	b4	= (length>dist_till_t4).GetV128();
			AIV128	b5	= (length>dist_till_t5).GetV128();
			AIV128	b6	= (length>dist_till_t6).GetV128();
			AIV128	b7	= (length>dist_till_t7).GetV128();

			AIV128	s0= VecXor(b0,b1);
			AIV128	s1 = VecXor(b1,b2);
			AIV128	s2 = VecXor(b2,b3);
			AIV128	s3 = VecXor(b3,b4);
			AIV128	s4 = VecXor(b4,b5);
			AIV128	s5 = VecXor(b5,b6);
			AIV128	s6 = VecXor(b6,b7);
			AIV128	s7 = b7;			// next assumed false.

			tret.SetV128(VecSelectFromList8(
						s0,s1,s2,s3,s4,s5,s6,s7,
						t0.GetV128(),t1.GetV128(),t2.GetV128(),t3.GetV128(),t4.GetV128(),t5.GetV128(),t6.GetV128(),t7.GetV128()));

			if (i>=1)
				return	tret;

			AIFLOAT	sublen;
			sublen.SetV128(VecSelectFromList8(
						s0,s1,s2,s3,s4,s5,s6,s7,
						dist_till_t0.GetV128(),
						dist_till_t1.GetV128(),
						dist_till_t2.GetV128(),
						dist_till_t3.GetV128(),
						dist_till_t4.GetV128(),
						dist_till_t5.GetV128(),
						dist_till_t6.GetV128(),
						dist_till_t7.GetV128()
						)
					);

			AIFLOAT dtsub = (thi-tlo)*(1.f/8.f);
			tlo = tret;
			thi = tret + dtsub;

			length-=sublen;
		};
		return	tret;

	}


	AIFLOAT	B_Spline_GetTOfPoint(AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3, AIVEC point, int numIterations) 
	{
		int	i;
		AIFLOAT	tlo =	0.f;
		AIFLOAT	thi	=	1.f;
		AIFLOAT	tret;

		for (i=0; i<numIterations; i++) 
		{
			// evaluate some points on the spline, select the closer one.
			AIFLOAT	dt = thi-tlo;
			AIFLOAT	t0 = tlo + AIFLOAT(1.f/16.f)*dt;
			AIFLOAT	t1 = tlo + AIFLOAT(3.f/16.f)*dt;
			AIFLOAT	t2 = tlo + AIFLOAT(5.f/16.f)*dt;
			AIFLOAT	t3 = tlo + AIFLOAT(7.f/16.f)*dt;
			AIFLOAT	t4 = tlo + AIFLOAT(9.f/16.f)*dt;
			AIFLOAT	t5 = tlo + AIFLOAT(11.f/16.f)*dt;
			AIFLOAT	t6 = tlo + AIFLOAT(13.f/16.f)*dt;
			AIFLOAT	t7 = tlo + AIFLOAT(15.f/16.f)*dt;

			AIVEC	v0 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t0);
			AIVEC	v1 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t1);
			AIVEC	v2 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t2);
			AIVEC	v3 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t3);
			AIVEC	v4 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t4);
			AIVEC	v5 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t5);
			AIVEC	v6 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t6);
			AIVEC	v7 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t7);

			AIFLOAT	d0	= VecDist2(v0,point);
			AIFLOAT	d1	= VecDist2(v1,point);
			AIFLOAT	d2	= VecDist2(v2,point);
			AIFLOAT	d3	= VecDist2(v3,point);
			AIFLOAT	d4	= VecDist2(v4,point);
			AIFLOAT	d5	= VecDist2(v5,point);
			AIFLOAT	d6	= VecDist2(v6,point);
			AIFLOAT	d7	= VecDist2(v7,point);

			// TODO - Look into an SOA method of doing this (pick key from smallest score value)
			AIFLOAT	mind01 = d0.Min(d1);
			AIFLOAT	mind23 = d2.Min(d3);
			AIFLOAT	mind45 = d4.Min(d5);
			AIFLOAT	mind67 = d6.Min(d7);

			AIFLOAT	mind0123 = mind01.Min(mind23);
			AIFLOAT	mind4567 = mind45.Min(mind67);

			// Todo - investigate saving registers using AND-Complement?
			AIV128	bIs0123Closest	=	(mind0123<mind4567).GetV128();
			AIV128	bIs01CloserThan23	=	(mind01<mind23).GetV128();
			AIV128	bIs45CloserThan67	=	(mind45<mind67).GetV128();

			AIV128	bIs4567Closest	=	VecNot(bIs0123Closest);
			AIV128	bIs23CloserThan01	=	VecNot(bIs01CloserThan23);
			AIV128	bIs67CloserThan45	=	VecNot(bIs45CloserThan67);

			AIV128	bIs0CloserThan1	=	(d0<d1).GetV128();
			AIV128	bIs2CloserThan3	=	(d2<d3).GetV128();
			AIV128	bIs4CloserThan5	=	(d4<d5).GetV128();
			AIV128	bIs6CloserThan7	=	(d6<d7).GetV128();

			AIV128	bIs01Closest	=	VecAnd(bIs0123Closest, bIs01CloserThan23);
			AIV128	bIs23Closest	=	VecAnd(bIs0123Closest, bIs23CloserThan01);

			AIV128	bIs45Closest	=	VecAnd(bIs4567Closest, bIs45CloserThan67);
			AIV128	bIs67Closest	=	VecAnd(bIs4567Closest, bIs67CloserThan45);

			AIV128	bIs0Closest		=	VecAnd(bIs01Closest,	bIs0CloserThan1);
			AIV128	bIs1Closest		=	VecAnd(bIs01Closest,	VecNot(bIs0CloserThan1));
			AIV128	bIs2Closest		=	VecAnd(bIs23Closest,	bIs2CloserThan3);
			AIV128	bIs3Closest		=	VecAnd(bIs23Closest,	VecNot(bIs2CloserThan3));
			AIV128	bIs4Closest		=	VecAnd(bIs45Closest,	bIs4CloserThan5);
			AIV128	bIs5Closest		=	VecAnd(bIs45Closest,	VecNot(bIs4CloserThan5));
			AIV128	bIs6Closest		=	VecAnd(bIs67Closest,	bIs6CloserThan7);
			AIV128	bIs7Closest		=	VecAnd(bIs67Closest,	VecNot(bIs6CloserThan7));

			tret.SetV128(VecSelectFromList8(
						bIs0Closest,
						bIs1Closest,
						bIs2Closest,
						bIs3Closest,
						bIs4Closest,
						bIs5Closest,
						bIs6Closest,
						bIs7Closest,
						t0.GetV128(),t1.GetV128(),t2.GetV128(),t3.GetV128(),t4.GetV128(),t5.GetV128(),t6.GetV128(),t7.GetV128()));
			AIFLOAT	dtsub=dt*(1.f/16.f);
			tlo = tret-dtsub;
			thi = tret+dtsub;

		};
		return	tret;
	}


	int	LinearSpline_GetSegmentContainingPoint(const AIVEC* keys, int numPoints, AIVEC point)
	{
		int	i;
		int	indexOut=0;
		AIFLOAT	minDist=FLT_MAX;
		AIFLOAT	fIndex=1.01f,fIndexOut;
		// TODO - VERSION THAT DOESN'T ASSUME WE STARTED NEAR MINIMA!!

		// remaining vertices
		for (i=0; i<(numPoints-1); i++) 
		{
			AIFLOAT	dist = (keys[i]-point).Length() + (keys[i+1]-point).Length();
			if (dist<minDist) 
			{
				minDist=dist;
				indexOut = i;
			} else
				break;
		}
		return	indexOut;
	}
	
	Maths::HVec3  B_Spline_EvaluatePointAheadOfPoint(
                        Maths::HVec3	inputPoint,                                 // perform calculation relative to this point on spline
                        const Maths::HVec3*	pControlPoints,
                        int			    numControlPoints,
						Maths::HF32		distanceAheadOfInputDesired,    // measured along spline
						Maths::HF32		tolerance,
						Maths::HVec3*	pClosestPointOnSplineToInputOut
						)                                  //  allowable error in mapping point to distance values ..
	{
		assert(numControlPoints<32);

		AIV128	buffer[8];
		B_Spline_CalculateCoefficients_X8((Maths::HVec4*)buffer, 1, 0.f, (1.f/7.f));

		int	baseIndex	=	LinearSpline_GetSegmentContainingPoint(
								pControlPoints+1, numControlPoints-3, inputPoint);

		const AIVEC*	src=pControlPoints+baseIndex;
		const AIVEC*	srcEnd=pControlPoints+numControlPoints-4;
		//int	i;


		AIFLOAT	start_t		= B_Spline_GetTOfPoint(src[0],src[1],src[2],src[3], inputPoint);
		AIFLOAT	distOfPoint	= B_Spline_GetLengthOfRange(src[0],src[1],src[2],src[3], AIFLOAT(0.f), start_t);
		*pClosestPointOnSplineToInputOut	=	B_Spline_Approx_Calc_Opt(src[0],src[1],src[2],src[3], start_t);

		AIFLOAT	targetDist	= distOfPoint	+ distanceAheadOfInputDesired;

		while (src<srcEnd) {
			AIFLOAT	lengthOfSegment = B_Spline_ArcLength_PrecalcCoeff8(src[0],src[1],src[2],src[3], (Maths::HVec4*)buffer);
			
			if (targetDist<lengthOfSegment)
				break;
			targetDist	-=	lengthOfSegment;
			src++;
		}
		AIFLOAT	target_t=B_Spline_GetTFromLength(src[0],src[1],src[2],src[3], targetDist);

//#define DEBUGDRAW_LinearSpline_GetSegmentContainingPoint
#ifdef DEBUGDRAW_LinearSpline_GetSegmentContainingPoint
		AIVEC	inputPointOnSpline = B_Spline_Approx_Calc_Opt(src[0],src[1],src[2],src[3], start_t);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCross( inputPointOnSpline, 0.25f,Maths::RGBA(0xffff00000));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCross( inputPoint, 0.5f,Maths::RGBA(0xffff000ff));
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( src[1], src[2],Maths::RGBA(0xffff00000));
		for (i=0; i<numControlPoints; i++) {
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dCross( pControlPoints[i], 0.05f,Maths::RGBA(0xffff00000));
		if (i>0)
				DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine( pControlPoints[i-1],pControlPoints[i], Maths::RGBA(0xff0000080));
		}
#endif

		return	B_Spline_Approx_Calc_Opt(src[0],src[1],src[2],src[3], target_t);
	}



	AIFLOAT	B_Spline_GetTOfPoint_BROKEN(AIVEC	ctrl0, AIVEC ctrl1, AIVEC ctrl2, AIVEC ctrl3, AIVEC point, int numIterations) 
	{
		// calculate 8 points on the spline, get the 2 points enclosing,
		// TODO - final stage should be linear approximation!
		int	i;
		AIFLOAT	tlo =	0.f;
		AIFLOAT	thi	=	1.f;
		
		for (i=0; i<numIterations; i++) {
			// TODO - try, for more precision, calculating first/last points & select them in from the prev iteration;
			AIFLOAT	dt = thi-tlo;
			AIFLOAT	t0 = tlo;
			AIFLOAT	t1 = tlo+AIFLOAT(1.f/7.f)*dt;
			AIFLOAT	t2 = tlo+AIFLOAT(2.f/7.f)*dt;
			AIFLOAT	t3 = tlo+AIFLOAT(3.f/7.f)*dt;
			AIFLOAT	t4 = tlo+AIFLOAT(4.f/7.f)*dt;
			AIFLOAT	t5 = tlo+AIFLOAT(5.f/7.f)*dt;
			AIFLOAT	t6 = tlo+AIFLOAT(6.f/7.f)*dt;
			AIFLOAT	t7 = thi;

			AIVEC	v0 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t0);
			AIVEC	v1 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t1);
			AIVEC	v2 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t2);
			AIVEC	v3 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t3);
			AIVEC	v4 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t4);
			AIVEC	v5 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t5);
			AIVEC	v6 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t6);
			AIVEC	v7 = B_Spline_Approx_Calc_Opt(ctrl0,ctrl1,ctrl2,ctrl3,  t7);

			AIFLOAT	d0	= VecDist2(v0,point);
			AIFLOAT	d1	= VecDist2(v1,point);
			AIFLOAT	d2	= VecDist2(v2,point);
			AIFLOAT	d3	= VecDist2(v3,point);
			AIFLOAT	d4	= VecDist2(v4,point);
			AIFLOAT	d5	= VecDist2(v5,point);
			AIFLOAT	d6	= VecDist2(v6,point);
			AIFLOAT	d7	= VecDist2(v7,point);

			AIV128	b0	= (d0<d1).GetV128();
			AIV128	b1	= (d1<d2).GetV128();
			AIV128	b2	= (d2<d3).GetV128();
			AIV128	b3	= (d3<d4).GetV128();
			AIV128	b4	= (d4<d5).GetV128();
			AIV128	b5	= (d5<d6).GetV128();
			AIV128	b6	= (d6<d7).GetV128();

			b0	=	VecXor(b0,b1);
			b1	=	VecXor(b1,b2);
			b2	=	VecXor(b2,b3);
			b3	=	VecXor(b3,b4);
			b4	=	VecXor(b4,b5);
			b5	=	VecXor(b5,b6);
			// Select the two T values that enclose us.
			AIV128	bBefore	= VecAnd(b0,b1,b2,b3,b4,b5);
			AIV128	bAfter	= VecNot(VecOr(b0,b1,b2,b3,b4,b5));

			// TODO - EXTEND FOR CASE WHERE VERTEX IS OUTSIDE!
			// base on bBefore, bAfter
			tlo.SetV128(VecSelectFromList8(
								VecOr(bBefore,b0),b0,b1,b2,b3,b4,b5,bAfter,
								t0.GetV128(),t1.GetV128(),t2.GetV128(),t3.GetV128(),t4.GetV128(),t5.GetV128(),t6.GetV128(),t7.GetV128()));

			thi.SetV128(VecSelectFromList8(
								bBefore,b0,b1,b2,b3,b4,b5,VecOr(b5,bAfter),
								t0.GetV128(),t1.GetV128(),t2.GetV128(),t3.GetV128(),t4.GetV128(),t5.GetV128(),t6.GetV128(),t7.GetV128()));
		}
		
		return	(tlo+thi)*AIFLOAT(0.5f);
	}


	Maths::HVec3 B_Spline_Approx_Calc(Maths::HVec3& P1, Maths::HVec3& P2, Maths::HVec3& P3,Maths::HVec3& P4, Maths::HF32	t)
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

	void B_Spline_Approx(const Btl::Vector<Maths::HVec3 > &pctrlPoints, Btl::Vector<Maths::HVec3 > &samplePoints, float interval, float step)
	{

		// TODO : simple optimise using length squared....
		const float epsilonSpline = 0.01f;
		Maths::HVec3 lastValidPoint;
		bool ReachedEnd = false;

		const Btl::Vector<Maths::HVec3>  &ctrlPoints = pctrlPoints;

		for(int i = 0; i <(int)ctrlPoints.Size() + 8; i++)
		{
			int k = i;
			if (k >= (int)ctrlPoints.Size())
				k -= (int)ctrlPoints.Size();

			int i1 = k-3;
			if (i1 < 0)
				i1 += (int)ctrlPoints.Size();
			int i2 = k-2;
			if (i2 < 0)
				i2 += (int)ctrlPoints.Size();
			int i3 = k-1;
			if (i3 < 0)
				i3 += (int)ctrlPoints.Size();

			Maths::HVec3 v1 = ctrlPoints[i1];	
			Maths::HVec3 v2 = ctrlPoints[i2];
			Maths::HVec3 v3 = ctrlPoints[i3];
			Maths::HVec3 v4 = ctrlPoints[k];

			for(float j = 0.0f; j <= 1.0f; j += step)
			{
				Maths::HVec3 result = B_Spline_Approx_Calc(v1, v2, v3, v4, j);

				if ((int)samplePoints.Size() == 0)
				{
					samplePoints.PushBack(result);
					lastValidPoint = result;
				}
				else
				{
					float dist = (lastValidPoint - result).Length();

					if (dist > (interval - epsilonSpline))
					{
						if ((int)samplePoints.Size() > 10)
						{
							Maths::HVec3 p1 = samplePoints[0];
							Maths::HVec3 p2 = result;

							float StartEndDist = (p1 - p2).Length();

							if (StartEndDist < 1.25f)
							{
								ReachedEnd = true; 
							}			
						}

						if (ReachedEnd == false)
						{	
							samplePoints.PushBack(result);
							lastValidPoint = result;
						}
					}
				}
			}
		}
	}





	void B_Spline_ApproxNonLoopedOpt(const Btl::Vector<Maths::HVec3>  &pctrlPoints, Btl::Vector<Maths::HVec3>  &samplePoints, float interval, float step)
	{


		// TODO : simple optimise using length squared....
		const float epsilonSpline = 0.05f;//0.02f; // 2 cm, what if we use 5 cm
		Maths::HVec3 lastValidPoint;
		bool ReachedEnd = false;

		if (pctrlPoints.Size() > 1)
			lastValidPoint = pctrlPoints[1];

		static const float check1 = interval - epsilonSpline;
		static const float check2 = interval + epsilonSpline;

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

			float j = 0.5f;// 0.5f;
			float mini = 0.0f;
			float start = 0.0f;
			float end = 1.0f;
			int countIters = 0;
			float diff = 1.0f;

			do 
			{
				countIters++;
				Maths::HVec3 result = B_Spline_Approx_Calc(v1, v2, v3, v4, j);
				float dist = (lastValidPoint - result).LengthSquared();

				float res = dist - interval;
				 
				if (dist > check1 && dist < check2)
				{
					// we have our result....
					samplePoints.PushBack(result);
					lastValidPoint = result;
					end = 1.0f;
					start = j;
					j = ((end - start) / 2.0f) + start;
					
				}
				else if (dist > check2)
				{
					end = j;
					//j = (end - start) / 2.0f;
					j = ((end - start) / 2.0f) + start;
				}
				else 
				{
					start = Maths::Max(j,start);
					j = ((end - start) / 2.0f) + start;
				}

				diff = 1.0f - j;
				

			} while (diff > epsilonSpline);


			int stop = 0;
		}
	}


}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif

