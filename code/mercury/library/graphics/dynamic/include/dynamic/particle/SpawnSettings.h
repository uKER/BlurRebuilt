#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SPAWNSETTINGS_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SPAWNSETTINGS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Enums.h"

namespace DynamicEffect
{

template <class BlockType>
class VectorBlockManager;

namespace Particle
{
class C_Emitter;
class ParticleBlock;
class I_ParticleEffectBase;

struct SuperEffects;

struct SpawningEmitter
{
	FORCE_INLINE const bool operator<(const SpawningEmitter& other) const
	{
		return pEffect < other.pEffect;
	}

	C_Emitter*				pEmitter;
	I_ParticleEffectBase*	pEffect;
};

INLINE INTPTR RadixKey(const SpawningEmitter& info)	// [Blur2PC][x64] the whole address groups emitters by effect
{
	return (INTPTR)info.pEffect;
}

struct ALIGN16 SpawnSettings
{
	MathsStorage::Float2				lodFade;
	F32									cullDistance;
	U32									vertexSize;
	U32									atlasSize;					// the number of rows/columns of sprites in texture
	U32									atlasOffset;
	S32									meshCount;
	F32									meshScale;
	S32									nodeType;
	E_ParticleShape						particleShape;
	I_ParticleEffectBase*				pEffect;
	VectorBlockManager<ParticleBlock>*	pBlockManager;
#ifndef CFG_SPEED
	S32*								pAllocated;
#endif
	S32*								pBlocks;
	S32*								pBlocksLocal;
	AtomicVector<ParticleBlock*>		blocks;
	SuperEffects*						pSuperEffects;								
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_SPAWNSETTINGS_H
