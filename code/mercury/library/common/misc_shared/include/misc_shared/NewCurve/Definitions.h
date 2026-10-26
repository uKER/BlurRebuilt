#ifndef CURVE_DEFINITIONS_H
#define CURVE_DEFINITIONS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
//	#define DISABLE_NON_1D_UPARAM

	#define CURVE_ACQUIRE_MAX 12

	extern const Maths::HMat44 kBezierBasis;
	extern const Maths::HMat44 kHermiteBasis;
	extern const Maths::HMat44 kBSplineBasis;

	enum E_ProcessU
	{
		E_ProcessU_None,
#ifndef DISABLE_NON_1D_UPARAM
		E_ProcessU_Regular,
		E_ProcessU_ArcLengthLinear
#endif
	};

	extern const F32 kReparamResLow;
	extern const F32 kReparamResMedium;
	extern const F32 kReparamResHigh;
	extern const F32 kReparamResDefault;

#define DEF(e, str) e,
	enum E_ReparamRes
	{
		#include "EReparamRes.h"
		E_ReparamRes_Count
	};
#undef DEF

	const char* GetReparamString(U32 i);
}

#endif
