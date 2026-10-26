#ifndef RENDER_DYNAMICEFFECT_PARTICLE_UPDATEDATA_H
#define RENDER_DYNAMICEFFECT_PARTICLE_UPDATEDATA_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{
namespace Particle
{

class C_Random;
class ParticleBlock;
class ParticleTrailBlock;
class I_ParticleEffectBase;

struct PhysicsSettings;
class C_EmitterList;


struct ALIGN16 DeletedSuperBlock
{
	ParticleBlock*		pBlock;
	Btl::Vector<ParticleTrailBlock*, Btl::VectorParameters_Static<4> >		trailBlocks;
};

struct ALIGN16 EffectUpdateConstants
{
	MathsStorage::Float4A16			globalForce;

	F32								timeStep;
	F32								timeStepRcp;
	F32								sleepSpeedSq;
	const MathsStorage::Float4A16*	pMaterials;
	U32								frameIndex;

	

	ParticleBlock**					ppDeletedBlocks;
	S32*							pDeletedBlocksCount;
	S32								deletedBlocksCapacity;

	DeletedSuperBlock*				pDeletedSuperBlocks;
	S32*							pDeletedSuperBlocksCount;
	S32								deletedSuperBlocksCapacity;

	ParticleBlock**					ppCollideableBlocks;
	S32*							pCollideableBlocksCount;
	S32								collideableBlocksCapacity;

	//C_EmitterList*					pEmitterList;
};

// this is small enough to fit in the task structure
struct ALIGN16 EffectUpdateData
{
	const PhysicsSettings*		pPhysicsSettings;
	const EffectUpdateConstants*		pUpdateConstants;
	U32							numBlocks;
};


typedef Btl::Vector<ParticleTrailBlock**, Btl::VectorParameters_Static<4> >		TrailPtrPtrs;
struct SuperEffects;

// this isn't so we have to allocate
struct ALIGN16 SuperEffectUpdateData
{
	ParticleBlock**				ppBlockEA;
	TrailPtrPtrs				trailBlocksEA;
	const SuperEffects*			pSuperEffects;
	const PhysicsSettings*		pPhysicsSettings;
	const EffectUpdateConstants*		pUpdateConstants;
	U32							numBlocks;
};

struct SpawningEmitter;
struct SpawnSettings;
class I_ParticleEffect;
class I_ParticleSuperEffect;

struct ALIGN16 EmitterUpdateConstants
{
	F32								time;
	F32								timeStep;
	F32								timeStepRcp;
	F32								intensity;
	
	// normal emitters handled by library
	SpawningEmitter*				pSpawningEmitters;
	S32*							pSpawningEmittersCount;
	S32								spawningEmittersCapacity;
	// custom emitters with virtual spawners
	SpawningEmitter*				pSpawningCustomEmitters;
	S32*							pSpawningCustomEmittersCount;
	S32								spawningCustomEmittersCapacity;

	I_ParticleEffect**				ppActivedEffects;
	S32*							pActivedEffectsCount;
	
	I_ParticleSuperEffect**			ppActivedSuperEffects;
	S32*							pActivedSuperEffectsCount;

	C_Random*						pRandom[7];
};

struct EmitterUpdateData
{
	C_Emitter**						ppEmitters;
	U32								numEmitters;
	const EmitterUpdateConstants*	pUpdateConstants;
	
};

struct EffectSpawnData
{
	SpawnSettings*					pSpawnSettings;
	SpawningEmitter*				pSpawningEmitter;
	U32								emitterCount;
};

typedef void (*EffectUpdateFunctor)( ParticleBlock** ppBlockEA, const PhysicsSettings* pPhysicsSettingsEA, const EffectUpdateConstants* pUpdateConstantsEA, U32 numBlocks );
typedef void (*SuperEffectUpdateFunctor)( ParticleBlock** ppBlockEA, TrailPtrPtrs* pTrailBlocksEA, const SuperEffects* pSuperEffects, const PhysicsSettings* pPhysicsSettings, const EffectUpdateConstants* pUpdateConstants, U32 numBlocks );
typedef void (*EffectSpawnFunctor)(SpawningEmitter* pSpawningEmitter, SpawnSettings* pSpawnSettings, S32 count, C_Random& random, const EmitterUpdateConstants* pUpdateConstants);

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_UPDATEDATA_H
