#ifndef CURVE_POLYLINE_H
#define CURVE_POLYLINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	struct C_PolyLineFlags
	{
		C_PolyLineFlags() { bits = 0; }

		union
		{
			U32 bits;
			struct
			{
				E_ProcessU calcKnotU : 4;
			};
		};
	};

	///Linear curve (Polyline) implementation.
	///
	template<typename VECTYPE, typename STORAGE>
	class C_PolyLine : public C_InterpolatingCurveBase<VECTYPE, STORAGE>
	{
	public:
		typedef STORAGE StorageType;
		typedef VECTYPE	VecType;
		typedef C_PolyLineFlags FlagsType;

		///Constructor.
		C_PolyLine(C_PolyLineFlags flags);

		///Destructor.
		virtual ~C_PolyLine();

		virtual VECTYPE Evaluate(F32 u) const;

		bool Process();

	protected:
		typedef C_InterpolatingCurveBase<VECTYPE, STORAGE>	Base;
		C_PolyLineFlags m_flags;
	};

	#include "PolyLine.inl"
}

#endif
