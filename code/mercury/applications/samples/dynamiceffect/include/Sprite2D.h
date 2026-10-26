#ifndef SPRITE2D_H
#define SPRITE2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Effect2D.h"
#include "Dynamic/Particle/Build\BeginVertices2D.h"
#include "Dynamic/Particle/Shapes\2D\Sprite.h"

namespace DynamicEffect
{

template<class Particle>
class Sprite2DSimpleCreate
{
public:
	void Create(Particle* pParticle, const I_ParticleEffectBase* pEffect, const C_Emitter* pEmitter) {};
};

// inherit from base emitter and create own update and initialisation functions
class C_Sprite2D : public I_ParticleEffect2D<C_Particle2D, ShapePolicy::Sprite2D, BuildPolicy::BeginVertices2D, Sprite2DSimpleCreate>
{
public:
	C_Sprite2D(C_Renderer* pRenderer, const U32 iIndex);
	~C_Sprite2D();
	void				Update();
	void				TestSpawn();
};

}

#endif
