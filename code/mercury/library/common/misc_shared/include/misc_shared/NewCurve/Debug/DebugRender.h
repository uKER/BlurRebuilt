#ifndef CURVE_DEBUG_RENDER_H
#define CURVE_DEBUG_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Curve
{
	template<typename STORAGE>
	static void RenderKnots(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const STORAGE& curve, 
		const Maths::HMat43& transform, 
		const Maths::RGBA& colour);

	template<typename STORAGE>
	static void RenderLinear(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const STORAGE& curve, 
		const Maths::HMat43& transform, 
		const Maths::RGBA& segA,
		const Maths::RGBA& segB);

	static void RenderUStep(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const I_Curve<Maths::HVec3>& curve, 
		F32 paramLength, 
		U32 nSteps,
		const Maths::HMat43& transform, 
		const Maths::RGBA& segA, 
		const Maths::RGBA& segB);
	
	static void RenderUStep(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const I_Curve<F32>& curve,
		F32 paramLength, 
		U32 nSteps,
		const Maths::HMat43& transform, 
		const Maths::RGBA& segA,
		const Maths::RGBA& segB);

	static void RenderArcStep(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const I_Curve<Maths::HVec3>& curve, 
		const C_Reparameterise& reparam,
		F32 paramLength,
		F32 stepSize,
		const Maths::HMat43& transform,
		const Maths::RGBA& segA, 
		const Maths::RGBA& segB);

	static void RenderReparamData(
		DbUtil::I_DebugRenderPtr debugRenderPtr,
		const I_Curve<Maths::HVec3>& curve, 
		const C_Reparameterise& reparam,
		F32 paramLength,
		const Maths::HMat43& transform,
		const Maths::RGBA& segA, 
		const Maths::RGBA& segB);

	#include "DebugRender.inl"
}

#endif
