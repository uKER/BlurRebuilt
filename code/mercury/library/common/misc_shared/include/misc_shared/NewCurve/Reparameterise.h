#ifndef CURVE_REPARAMETERISE_H
#define CURVE_REPARAMETERISE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include "curvehelpers.h"

namespace Curve
{
	template<typename VECTYPE>
	class I_Curve;

	struct C_ReparamPoint
	{
		C_ReparamPoint() {}
		C_ReparamPoint(F32 u_, F32 arcLength_) : u(u_), arcLength(arcLength_) {}

		///Distance to this point along the curve in parameter space.
		F32 u;
		///Distance to this point along the curve in physical space.
		F32 arcLength;
	};

	//TODO: Make this work for angular 1D curves.
	class C_Reparameterise
	{
	public:
		C_Reparameterise();
		~C_Reparameterise();

		template<typename VECTYPE>
		void BuildTable(const I_Curve<VECTYPE>* pCurve,	U32 nSubDivisions);

		//Return arc length of entire curve.
		F32 GetArcLength() const;

		//Return arc length up to the parameter given.
		F32 GetArcLength(F32 u) const;

		F32 GetArcLengthByIndex(U32 index) const;

		F32 GetUByIndex(U32 index) const;

		//Return u parameter given a distance along the spline.
		F32 GetU(F32 arcLength) const;

		U32 GetNumPoints() const;

		C_ReparamPoint GetPoint(U32 index) const;

	private:
		U32 GetUInterval(F32 u) const;
		U32 GetLengthInterval(F32 arcLength) const;
		S32 FindUInterval(F32 arcLength, S32 low, S32 high) const;

		Btl::Vector<F32> m_arcLengthByUInterval;
		Btl::Vector<U32> m_uByLengthInterval;
		F32 m_uMin;
		F32 m_uMax;
		F32 m_uScale;
		F32 m_arcLength;
		F32 m_nSubDivisions;
	};

	#include "Reparameterise.inl"
}

#endif
