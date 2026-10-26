#ifndef RENDER_DYNAMICEFFECT_TRACER_GENERIC_SHAPEBEAMCAPPED_H
#define RENDER_DYNAMICEFFECT_TRACER_GENERIC_SHAPEBEAMCAPPED_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Tracer/EffectInterface.h"
#include "Dynamic/Tracer/Tracer.h"
#include "Dynamic/Tracer/Shape/BeamCapped.h"

namespace DynamicEffect
{
namespace Tracer
{

class ShapeBeamCapped : public I_Effect<Tracer, BeamCappedPolicy>
{
public:

	typedef I_Effect<Tracer, BeamCappedPolicy> Base;

	ShapeBeamCapped(C_Renderer* pRenderer, I_DynamicEffectManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex) {}

	void Spawn(const C_Node* pNode, const Maths::C_RGBA_F32& vColour);
	void Update();

};

}; // namespace Tracer
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACER_GENERIC_SHAPEBEAMCAPPED_H
