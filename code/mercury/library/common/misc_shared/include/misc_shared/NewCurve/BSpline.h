#ifndef CURVE_BSPLINE_H
#define CURVE_BSPLINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	//http://en.wikipedia.org/wiki/B-spline

	template<typename VECTYPE>
	struct ALIGN16 C_BSplineCv
	{
		//Read only//
		VECTYPE	value;		//Knot value
	};

	struct C_BSplineFlags
	{
	};

	///BSpline curve implementation.
	///
	template<typename VECTYPE, typename STORAGE>
	class C_BSpline : public C_ApproximatingCurveBase<VECTYPE, STORAGE>
	{
	public:
		typedef STORAGE StorageType;
		typedef VECTYPE	VecType;
		typedef C_BSplineFlags FlagsType;

		///Constructor.
		C_BSpline();

		///Destructor.
		virtual ~C_BSpline();

		virtual VECTYPE Evaluate(F32 u) const;

		virtual F32 GetMinU() const;

		virtual F32 GetMaxU() const;

		virtual const VECTYPE& AcquireVertexValue(U32 index) const;

		virtual const void* AcquireVertexData(U32 index) const;

		virtual void ReleaseVertex() const;

		virtual VECTYPE GetVertexValue(U32 index) const;

		virtual S32 GetNumVertices() const;
		
		void Process();

	protected:
		typedef C_ApproximatingCurveBase<VECTYPE, STORAGE>	Base;
	};

	#include "BSpline.inl"
}

#endif
