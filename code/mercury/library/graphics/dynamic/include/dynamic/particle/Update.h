#ifndef RENDER_DYNAMICEFFECT_PARTICLE_UPDATE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_UPDATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Defines.h"
#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
#include "Dynamic/Particle/UpdateXM.h"
#else
#include "Dynamic/Particle/UpdateHV.h"
#endif

#include "Dynamic/Particle/UpdateData.h"
#include "Dynamic/Particle/PhysicsSettings.h"
#include "Dynamic/Particle/Block.h"

#include <system/scheduler/jobqueue.h>

namespace DynamicEffect
{
namespace Particle
{

INLINE void FetchInputs(ParticleBlock**& ppBlockEA, const PhysicsSettings* pPhysicsSettingsEA, const EffectUpdateConstants* pUpdateConstantsEA, ParticleBlock**& ppBlockLS, const PhysicsSettings*& pPhysicsSettingsLS, const EffectUpdateConstants*& pUpdateConstantsLS, U32 numBlocks)
{
#ifdef PLATFORM_SPU
	PhysicsSettings* pPhysicsSettings = (PhysicsSettings*)Spu::FetchAllocAsync(pPhysicsSettingsEA, sizeof(PhysicsSettings), 0);
	EffectUpdateConstants* pUpdateConstants = (EffectUpdateConstants*)Spu::FetchAllocAsync(pUpdateConstantsEA, sizeof(EffectUpdateConstants), 0);

	ppBlockEA = (ParticleBlock**)Spu::FetchAllocAsync(ppBlockEA, (sizeof(ParticleBlock*) * numBlocks + 15) & ~15, 0);
	ppBlockLS = (ParticleBlock**)Spu::Alloc((sizeof(ParticleBlock*) * numBlocks + 15) & ~15);
	Spu::Wait(0);

	pUpdateConstants->pMaterials = (MathsStorage::Float4A16*)Spu::FetchAllocAsync(pUpdateConstants->pMaterials, sizeof(MathsStorage::Float4A16) * 64, 0);

	// alloc space for all blocks
	ParticleBlock* pBlocks = (ParticleBlock*)Spu::Alloc(sizeof(ParticleBlock) * numBlocks);
	// fetch blocks to local memory
	for(U32 i = 0; i < numBlocks; ++i)
		ppBlockLS[i] = (ParticleBlock*)Spu::FetchAsync(&pBlocks[i], ppBlockEA[i], sizeof(ParticleBlock), 0);
	Spu::Wait(0);

	pPhysicsSettingsLS = pPhysicsSettings;
	pUpdateConstantsLS = pUpdateConstants;
#else
	ppBlockLS = ppBlockEA;
	pPhysicsSettingsLS = pPhysicsSettingsEA;
	pUpdateConstantsLS = pUpdateConstantsEA;
#endif
}

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_UPDATE_H
