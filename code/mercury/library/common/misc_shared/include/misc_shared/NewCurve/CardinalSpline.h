#ifndef CURVE_CARDINAL_SPLINE_H
#define CURVE_CARDINAL_SPLINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename VECTYPE>
	struct ALIGN16 C_CardinalSplineKnot : C_Knot<VECTYPE>
	{
		typedef VECTYPE VecType;
		VECTYPE	tangent;		//Inbound and outbound tangent. Both are of the same magnitude and direction.
		F32		tangentScale;
	};

	struct C_CardinalSplineFlags
	{
		C_CardinalSplineFlags() { bits = 0; }

		union
		{
			U32 bits;
			struct
			{
				E_ProcessU calcKnotU : 4;
			};
		};
	};

	///Cardinal spline implementation.
	///
	///* C1 continuous.
	///* Interpolates all points.
	///* Each point has a single tangent vector. This is automatically calculated based upon its adjacent points. 
	///* Tangents multiplied by seperately stored scale value.
	///* Can automatically place each point in parameter space.
	///
	///If you really need to have seperate in/out tangents per point take a look at the Hermite curve class.
	template<typename VECTYPE, typename STORAGE>
	class C_CardinalSpline : public C_InterpolatingCurveBase<VECTYPE, STORAGE>
	{
	public:
		typedef STORAGE StorageType;
		typedef VECTYPE	VecType;
		typedef C_CardinalSplineFlags FlagsType;

		///Constructor.
		C_CardinalSpline(C_CardinalSplineFlags flags);

		///Destructor.
		virtual ~C_CardinalSpline();

		virtual VECTYPE Evaluate(F32 u) const;

		void Process();

	protected:
		typedef C_InterpolatingCurveBase<VECTYPE, STORAGE>	Base;
		C_CardinalSplineFlags	m_flags;
	};

	#include "CardinalSpline.inl"
}

#endif
