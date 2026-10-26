#ifndef CURVE_BEZIER_CURVE_H
#define CURVE_BEZIER_CURVE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename VECTYPE>
	struct ALIGN16 C_BezierCurveKnot
	{
		typedef VECTYPE VecType;

		F32 GetU(U32 knotIndex, U32 nKnots) const { return (F32)knotIndex/((F32)nKnots-1); }
		void SetU(F32 u) { BIZ_ASSERT(!"Cannot set u position!");}

		//Read only//
		VECTYPE	value;		//Knot value
		
		//Read and potentially write//
		//The vectors below are stored as positions, NOT tangents.
		VECTYPE	ctrlIn;		//Knot inbound control point
		VECTYPE	ctrlOut;	//Knot outbound control point
	};

	struct C_BezierCurveFlags
	{
		C_BezierCurveFlags() {bits = 0;}

		union
		{
			U32 bits;
			struct
			{
				E_ProcessU calcKnotU : 4;
				bool calcCtrlPoints : 1;
			};
		};
	};

	///Bezier curve implementation.
	///
	template<typename VECTYPE, typename STORAGE>
	class C_BezierCurve : public C_InterpolatingCurveBase<VECTYPE, STORAGE>
	{
	public:
		typedef STORAGE StorageType;
		typedef C_BezierCurveFlags FlagsType;

		///Constructor.
		C_BezierCurve(C_BezierCurveFlags flags);

		///Destructor.
		virtual ~C_BezierCurve();

		virtual VECTYPE Evaluate(F32 u) const;

		void Process();

	protected:
		typedef C_InterpolatingCurveBase<VECTYPE, STORAGE>	Base;
		C_BezierCurveFlags	m_flags;
	};

	#include "BezierCurve.inl"
}

#endif
