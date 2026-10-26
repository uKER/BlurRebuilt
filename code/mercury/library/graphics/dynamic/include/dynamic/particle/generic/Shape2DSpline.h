#pragma once

#include "Dynamic/Particle/Effect3D.h"
#include "Dynamic/Particle/Build/All.h"
#include "Dynamic/Particle/Shape/Shape2D.h"

namespace DynamicEffect
{
namespace Particle
{



// inherit from base emitter and create own update and initialisation functions
class Shape2DSpline : public T_ParticleEffect<C_Particle3D_P, ShapePolicy::Shape2D, BuildPolicy::T_All>
{
public:
	Shape2DSpline(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex);
	~Shape2DSpline();
};


}
}
