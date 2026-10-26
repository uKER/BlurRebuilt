#pragma once

#include "Dynamic/Particle/Effect3D.h"
#include "Dynamic/Particle/Build\All.h"
#include "Dynamic/Particle/Shape/SpriteRotating.h"

namespace DynamicEffect
{
namespace Particle
{



// inherit from base emitter and create own update and initialisation functions
class SpriteRotatingSpline : public T_ParticleEffect<C_Particle3D_R1D_P, ShapePolicy::SpriteRotating, BuildPolicy::T_All>
{
public:
	SpriteRotatingSpline(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex);
	~SpriteRotatingSpline();
};

}
}
