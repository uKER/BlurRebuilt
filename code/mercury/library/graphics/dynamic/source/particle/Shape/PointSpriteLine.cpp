#include "stdafx.h"

#include "Dynamic/Particle/Shape/PointSpriteLine.h"

// dynamic effect level includes
#include "Dynamic/Node.h"
#include "Dynamic/RenderNode.h"
#include "Dynamic/VertexBuildConstants.h"

// particle level includes
#include "Dynamic/Particle/Particle.h"
#include "Dynamic/Particle/Block.h"
#include "Dynamic/Particle/VertexBuildSettings.h"
#include "Dynamic/Particle/shape/Point.h"

#ifndef PLATFORM_SPU
#include "Dynamic/Particle/EffectInterface.h"
#include "Display/Context/Context.h"
#endif

namespace DynamicEffect
{
namespace Particle
{
namespace ShapePolicy
{

#ifndef PLATFORM_SPU
void PointSpriteLine::Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	pDisplay->SetPointSpriteEnable(Display::E_Bool_True);
	//cellGcmSetPointSize(2.0f);
	pEffect->GetManager()->SetPixelShader(pDisplay, E_PixelShader_PointSprite);
}

void PointSpriteLine::End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	pDisplay->SetPointSpriteEnable(Display::E_Bool_False);
}
#endif

}
} // ShapePolicy
} // Particles

