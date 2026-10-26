#ifndef DYNAMIC_PARTICLE_SPAWNERINTERFACE_H
#define DYNAMIC_PARTICLE_SPAWNERINTERFACE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Util
{
	class C_Random;
}

namespace DynamicEffect
{
namespace Particle
{
class C_Random;
class I_ParticleEffectBase;
class C_Emitter;
class ParticleBlock;
struct SpawnSettings;
struct ParticleElement;

class I_Spawner
{
public:
	I_Spawner() {}
	~I_Spawner() {}

	virtual U32		GetSpawnAmount() { return 0; }

	virtual void	CreateParticle(U8* pParticles, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
	virtual void	CreateParticles(U8* pParticles, S32 num, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings);
	virtual void	CreateBlock(ParticleBlock* pBlock, const C_Emitter* pEmitter);

protected:

	void			CreateElement(U8* pParticle, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, const ParticleElement& element);
};

}
}

#endif // DYNAMIC_PARTICLE_SPAWNERINTERFACE_H
