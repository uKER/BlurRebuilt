#ifndef CURVE_DEBUG_RENDER_BEZIER_CURVE_H
#define CURVE_DEBUG_RENDER_BEZIER_CURVE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename STORAGE>
	static void RenderBezierCurve(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const C_BezierCurve<Maths::HVec3, STORAGE>& curve, 
		F32 paramLength, 
		U32 nSteps,
		const Maths::HMat43& transform, 
		const Maths::RGBA& segA, 
		const Maths::RGBA& segB,
		const Maths::RGBA& tangent);

	#include "RenderBezierCurve.inl"
}

#endif
