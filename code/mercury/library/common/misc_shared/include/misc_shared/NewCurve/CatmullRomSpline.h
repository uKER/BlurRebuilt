#ifndef CURVE_CATMULLROM_SPLINE_H
#define CURVE_CATMULLROM_SPLINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename VECTYPE>
	struct ALIGN16 C_CatmullRomSplineKnot : C_Knot<VECTYPE>
	{
		typedef VECTYPE VecType;
		VECTYPE	tangent;	//Inbound and outbound tangent. Both are of the same magnitude and direction.
	};

	struct C_CatmullRomSplineFlags
	{
		C_CatmullRomSplineFlags() { bits = 0; }

		union
		{
			U32 bits;
			struct
			{
				E_ProcessU calcKnotU : 4;
			};
		};
	};

	///CatmullRom spline implementation.
	///
	///* C1 continuous.
	///* Interpolates all points.
	///* Each point has a single tangent vector. This is automatically calculated based upon its adjacent points. 
	///* All knots have a tension value of 0.5.
	///* Can automatically place each point in parameter space.
	///
	template<typename VECTYPE, typename STORAGE>
	class C_CatmullRomSpline : public C_InterpolatingCurveBase<VECTYPE, STORAGE>
	{
	public:
		typedef STORAGE StorageType;
		typedef VECTYPE	VecType;
		typedef C_CatmullRomSplineFlags FlagsType;

		///Constructor.
		C_CatmullRomSpline(C_CatmullRomSplineFlags flags);

		///Destructor.
		virtual ~C_CatmullRomSpline();

		virtual VECTYPE Evaluate(F32 u) const;

		const C_CatmullRomSplineFlags& GetFlags() const;

		virtual void Process();

	protected:
		typedef C_InterpolatingCurveBase<VECTYPE, STORAGE>	Base;
		C_CatmullRomSplineFlags	m_flags;
		Maths::HMat44			m_basis;
	};

	#include "CatmullRomSpline.inl"
}

#endif
