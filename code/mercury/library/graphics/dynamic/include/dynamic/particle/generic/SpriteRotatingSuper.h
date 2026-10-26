#pragma once

#include "Dynamic/Particle/SuperEffect.h"
#include "Dynamic/Particle/Build\All.h"
#include "Dynamic/Particle/Shape/SpriteRotating.h"

namespace DynamicEffect
{
namespace Particle
{

// inherit from base emitter and create own update and initialisation functions
class SpriteRotatingSuper : public T_ParticleSuperEffect<C_Particle3D_R1D, ShapePolicy::SpriteRotating, BuildPolicy::T_All>
{
public:
	SpriteRotatingSuper(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex);
	~SpriteRotatingSuper();
};


}
}
