#ifndef CURVE_NURB_SPLINE_H
#define CURVE_NURB_SPLINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	//About Nonuniform Rational B-Splines - NURBS
	//http://www.gamedev.net/reference/articles/article420.asp

	struct C_NurbSplineFlags
	{
		C_NurbSplineFlags() { bits = 0; }
		union
		{
			U32 bits;
			struct
			{
				bool generatePhantomKnots : 1;
			};
		};
	};

	///Nurb spline curve implementation.
	///
	template<typename VECTYPE, typename STORAGE>
	class C_NurbSpline : public C_ApproximatingCurveBase<VECTYPE, STORAGE>
	{
	public:
		typedef STORAGE StorageType;
		typedef VECTYPE	VecType;

		///Constructor.
		C_NurbSpline(U32 degree);
		typedef C_NurbSplineFlags FlagsType;

		///Destructor.
		virtual ~C_NurbSpline();

		virtual VECTYPE Evaluate(F32 u) const;

		virtual VECTYPE EvaluateLinear(F32 u) const;

		virtual F32 GetMinU() const;

		virtual F32 GetMaxU() const;

		virtual const VECTYPE& AcquireVertexValue(U32 index) const;

		virtual const void* AcquireVertexData(U32 index) const;

		virtual void ReleaseVertex() const;

		virtual S32 GetNumVertices() const;

		virtual VECTYPE GetVertexValue(U32 index) const;

		U32 GetDegree() const;

		F32 GetKnot(U32 index) const;

		U32 GetNumKnots() const;
	
		U32 GetNumSpans() const;

		void Process();

	protected:
		typedef C_ApproximatingCurveBase<VECTYPE, STORAGE>	Base;
		///Calculate blend weight
		///\param i knot index
		///\param k degree
		///\param t parameter value
		F32 CoxDeBoor(U32 i, U32 degree, F32 t) const;

		///Number of points per segment-1
		U32					m_degree;
		C_NurbSplineFlags	m_flags;
	};

	#include "NurbSpline.inl"
}

#endif
