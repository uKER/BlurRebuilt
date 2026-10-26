#ifndef CURVE_HERMITE_HELPERS_H
#define CURVE_HERMITE_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	namespace Helpers
	{
		void ConvertLegacyHermiteTangents(Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots);
		
		#include "HermiteHelpers.inl"
	}
}

#endif

