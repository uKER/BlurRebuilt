#ifndef CURVE_NURB_SPLINE_DATA_H
#define CURVE_NURB_SPLINE_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename VECTYPE>
	struct ALIGN16 C_NurbsCv
	{
		//Read only//
		VECTYPE	value;				//Value
		F32		weight;				//Value weight
	};


	typedef Btl::Vector<F32> KnotVector;

	template<typename VECTYPE>
	class C_NurbSplineData
	{
	public:
		C_NurbSplineData();

		~C_NurbSplineData();

		C_NurbsCv<VECTYPE> GetCv(U32 index) const;

		void SetCv(U32 index, const C_NurbsCv<VECTYPE>& cv);

		F32 GetKnot(U32 index) const;

		void SetKnot(U32 index, F32 knot);

		///Required
		S32 GetNumCvs() const;

		///Required
		S32 GetNumKnots() const;

		Btl::Vector<C_NurbsCv<VECTYPE> >& GetCvs();

		const Btl::Vector<C_NurbsCv<VECTYPE> >& GetCvs() const;

		KnotVector& GetKnots();

		const KnotVector& GetKnots() const;

	private:
		Btl::Vector<C_NurbsCv<VECTYPE> >		m_cvs;
		KnotVector								m_knots;
	};

	#include "NurbSplineData.inl"
}

#endif
