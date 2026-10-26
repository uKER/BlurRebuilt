#ifndef CURVE_REPARAM_HELPERS_H
#define CURVE_REPARAM_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
namespace ReparamHelpers
{
	template<typename CURVE>
	typename CURVE::VecType EvaluateArcLength(
		const CURVE& curve, 
		const C_Reparameterise& reparam,
		F32 arcLength) 
	{
		return curve.Evaluate(reparam.GetU(arcLength));
	}

	template<typename CURVE>
	typename CURVE::VecType EvaluateArcLengthTangent(
		const CURVE& curve, 
		const C_Reparameterise& reparam, 
		F32 arcLength)
	{
		return curve.EvaluateTangent(reparam.GetU(arcLength));
	}

	template<typename REPARAM>
	F32 GetArcLength(const REPARAM& reparam, U32 arcIndex)
	{
		return reparam.GetArcLengthByIndex(arcIndex);
	}

	template<typename CURVE>
	typename CURVE::VecType GetPointValue(
		const CURVE& curve,
		const C_Reparameterise& reparam, 
		U32 arcIndex) 
	{
		return curve.Evaluate(reparam.GetUByIndex(arcIndex));
	}
}
}

#endif
