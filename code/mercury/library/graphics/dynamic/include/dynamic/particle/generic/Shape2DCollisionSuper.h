#pragma once

#include "Dynamic/Particle/SuperEffect.h"
#include "Dynamic/Particle/Build\All.h"
#include "Dynamic/Particle/Shape/Shape2D.h"

namespace DynamicEffect
{
namespace Particle
{

// inherit from base emitter and create own update and initialisation functions
class Shape2DCollisionSuper : public T_ParticleSuperEffect<C_Particle3D_CP, ShapePolicy::Shape2D, BuildPolicy::T_All>
{
public:
	Shape2DCollisionSuper(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex);
	~Shape2DCollisionSuper();
};


}
}
