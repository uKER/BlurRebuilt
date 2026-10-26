#include "stdafx.h"

#include "Dynamic/Particle/Shape/PointSprite.h"

// dynamic effect level includes
#include "Dynamic/Node.h"
#include "Dynamic/RenderNode.h"
#include "Dynamic/VertexBuildConstants.h"

// particle level includes
#include "Dynamic/Particle/Particle.h"
#include "Dynamic/Particle/Block.h"
#include "Dynamic/Particle/VertexBuildSettings.h"

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
void PointSprite::Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	pDisplay->SetPointSpriteEnable(Display::E_Bool_True);
	pEffect->GetManager()->SetPixelShader(pDisplay, E_PixelShader_PointSprite);
}

void PointSprite::End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	pDisplay->SetPointSpriteEnable(Display::E_Bool_False);
}
#endif

}
} // ShapePolicy
} // Particles

