#ifndef RENDER_DYNAMICEFFECT_FOG_EQUATIONS_H
#define RENDER_DYNAMICEFFECT_FOG_EQUATIONS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace DynamicEffect
{
namespace Fog
{

Maths::HF32 GetVolumetricFog( const Maths::HVec3 PCREF vViewVector, const Maths::HVec3 PCREF vSettings)
{
	const Maths::HF32 vZero = Maths::Constants<Maths::HF32>::Zero();
	const Maths::HF32 vOne = Maths::Constants<Maths::HF32>::One();

	Maths::HF32 vFogInt = vViewVector.Length() * vSettings.Z();
	if (Maths::Abs(vViewVector.Y()) > 0.01f)
	{
		Maths::HF32 vT = vSettings.Y() * vViewVector.Y();
		vFogInt = vFogInt * (vOne - expf(-vT)) * vT.Rcp();
	}

	return expf(-vSettings.X() * vFogInt);
}

}; // namespace Fog
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_FOG_EQUATIONS_H
