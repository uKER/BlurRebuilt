#ifndef CURVE_DEBUG_RENDER_HERMITE_SPLINE_H
#define CURVE_DEBUG_RENDER_HERMITE_SPLINE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename CURVE>
	static void RenderHermiteSpline3D(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const CURVE& curve, 
		F32 paramLength, 
		U32 nSteps,
		const Maths::HMat43& transform, 
		const Maths::RGBA& segA, 
		const Maths::RGBA& segB,
		const Maths::RGBA& tangent);

	template<typename CURVE>
	static void RenderHermiteSpline1D(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const CURVE& curve, 
		F32 paramLength, 
		U32 nSteps,
		const Maths::HMat43& transform,
		const Maths::RGBA& segA, 
		const Maths::RGBA& segB,
		const Maths::RGBA& tangent);

	#include "RenderHermiteSpline.inl"
}

#endif
