#ifndef CURVE_BOUNDS_H
#define CURVE_BOUNDS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	//TODO: Make this work for angular 1D curves.
	template<typename VECTYPE>
	class C_CurveBounds
	{
	public:
		C_CurveBounds();
		~C_CurveBounds();

		void Process(const I_Curve<VECTYPE>* pCurve, const C_Reparameterise* pReparam);

		const Maths::C_AxisBox& GetAabb() const;

	private:
		Maths::C_AxisBox m_aabb;
	};

	#include "CurveBounds.inl"
}

#endif
