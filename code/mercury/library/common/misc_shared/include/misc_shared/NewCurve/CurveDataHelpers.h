#ifndef CURVE_CURVE_DATA_HELPERS_H
#define CURVE_CURVE_DATA_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	namespace Helpers
	{
		template<typename STORAGE>
		void ReverseKnots(STORAGE& storage);

		#include "CurveDataHelpers.inl"
	}

}

#endif
