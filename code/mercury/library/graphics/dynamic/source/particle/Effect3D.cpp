#include "stdafx.h"

#include "Dynamic/Particle/Effect3D.h"
#include "Dynamic/Particle/Spawn.h"

namespace DynamicEffect
{
namespace Particle
{

I_ParticleEffect::I_ParticleEffect(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex)
: I_ParticleEffectBase(pRenderer, pManager, iIndex, iTypeIndex)
{
	m_pManager->AddEffect(this);
}

I_ParticleEffect::~I_ParticleEffect()
{
	m_pManager->RemoveEffect(this);
}

void I_ParticleEffect::UpdateMemory()
{

#ifndef CFG_SPEED
	// check whether blocks is about to be resized
	assert(m_Allocated <= m_SpawnSettings.blocks.Capacity());
	assert(m_Allocated <= m_nBlocks);
#endif

	S32 requestedBlocks = m_nBlocks;
	m_nBlocks = m_nBlocks > m_SpawnSettings.blocks.Capacity() ? m_SpawnSettings.blocks.Capacity() : m_nBlocks;
	
	// tell blocks how big it should potentially be
	m_SpawnSettings.blocks.Size(requestedBlocks);
	m_SpawnSettings.blocks.Update();

	S32 blocks = m_nBlocks;

	// remove NULL blocks from end
	while(m_nBlocks > 0 && m_SpawnSettings.blocks[m_nBlocks - 1] == NULL)
		--m_nBlocks;

	// remove NULL blocks from middle
	for(S32 i = 0; i < m_nBlocks; ++i)
	{
		if(m_SpawnSettings.blocks[i] == NULL)
		{
			S32 end = --m_nBlocks;
			ParticleBlock* pBlock = m_SpawnSettings.blocks[end];
			assert(pBlock);
			pBlock->m_VectorPosition = i;
			m_SpawnSettings.blocks[i] = pBlock;

			// remove NULL blocks from end
			while(m_nBlocks > 0 && m_SpawnSettings.blocks[m_nBlocks - 1] == NULL)
				--m_nBlocks;
		}
		else
		{
			assert(m_SpawnSettings.blocks[i]->m_pEffect == this);
		}
	}
	m_SpawnSettings.blocks.Size(m_nBlocks);

	if(m_nBlocks == 0 && m_ActivePosition != -1)
		m_pManager->RemoveActiveEffect(this);

#ifndef CFG_SPEED
	assert(m_nBlocks == m_Allocated);
#endif
}

void I_ParticleEffect::DeleteBlock(ParticleBlock* pParticleBlock)
{
	I_ParticleEffectBase::DeleteBlock(pParticleBlock);

	if(m_nBlocks == 0 && m_ActivePosition != -1)
		m_pManager->RemoveActiveEffect(this);
}

void SpawnCustom(C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, C_Random& random, const EmitterUpdateConstants* pUpdateConstants)
{
	SpawnT<CustomSpawnerCreationPolicy>(pEmitter, pSpawnSettings, random, pUpdateConstants);
}

} // Particle
} // DynamicEffect
