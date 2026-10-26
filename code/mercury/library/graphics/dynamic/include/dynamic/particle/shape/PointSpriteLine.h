#ifndef RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_POINTSPRITELINE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_POINTSPRITELINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"
#include "PointLine.h"

namespace DynamicEffect
{
namespace Particle
{
namespace ShapePolicy
{

class PointSpriteLine : public PointLine
{
public:

#ifndef PLATFORM_SPU
	static void		Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect);
	static void		End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect);
	
	static E_VertexShader GetVertexShader(const I_ParticleEffectBase* pEffect)
	{
		return E_VertexShader_PointSprite;
	}
#endif
};

}
}
}

#endif
