#pragma once

#include "Dynamic/Particle/Effect3D.h"
#include "Dynamic/Particle/Build\All.h"
#include "Dynamic/Particle/Shape/Shape3D.h"

namespace DynamicEffect
{
namespace Particle
{

// inherit from base emitter and create own update and initialisation functions
class Shape3DSimple : public T_ParticleEffect<C_Particle3D_RV3D, ShapePolicy::Shape3D, BuildPolicy::T_All>
{
public:
	Shape3DSimple(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex);
	~Shape3DSimple();
};


}
}
