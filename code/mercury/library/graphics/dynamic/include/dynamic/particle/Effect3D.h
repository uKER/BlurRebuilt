#ifndef RENDER_DYNAMICEFFECT_PARTICLE_EFFECT3D_H
#define RENDER_DYNAMICEFFECT_PARTICLE_EFFECT3D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Renderer.h"
#include "Dynamic/Particle/EffectInterface.h"
#include "Dynamic/Particle/SpawnerInterface.h"

namespace SceneGraph
{
	class C_View;
}

namespace DynamicEffect
{
namespace Particle
{

class CustomSpawnerCreationPolicy
{
public:
	typedef CustomSpawnerCreationPolicy		Base;
	static void CreateParticles(U8* pParticleData, S32 num, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
	{
		if(pEmitter->GetSpawner())
		{
			pEmitter->GetSpawner()->CreateParticles(pParticleData, num, random, pEmitter, pSpawnSettings);
		}
	}
	static void CreateParticle(U8* pParticleData, C_Random& random, C_Emitter* pEmitter, SpawnSettings* pSpawnSettings)
	{
		if(pEmitter->GetSpawner())
		{
			pEmitter->GetSpawner()->CreateParticle(pParticleData, random, pEmitter, pSpawnSettings);
		}
	}
	static void CreateBlock(ParticleBlock* pBlock, C_Emitter* pEmitter)
	{
		if(pEmitter->GetSpawner())
		{
			pEmitter->GetSpawner()->CreateBlock(pBlock, pEmitter);
		}
	}
};

void SpawnCustom(C_Emitter* pEmitter, SpawnSettings* pSpawnSettings, C_Random& random, const EmitterUpdateConstants* pUpdateConstants);

class I_ParticleEffect : public I_ParticleEffectBase
{
public:
	typedef I_ParticleEffectBase	Base;

	I_ParticleEffect(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex);
	virtual ~I_ParticleEffect();

	
	EffectUpdateFunctor	GetUpdateFunctor() { return m_pUpdateFunctor; }

	void				DeleteBlock(ParticleBlock* pParticleBlock);

	virtual void		Reset()
	{
		UpdateMemory();		// update memory
		Base::Free();		// free other blocks
	}

	friend class		C_ParticleManager;

protected:
	EffectUpdateFunctor	m_pUpdateFunctor;
private:
	void				UpdateMemory();
};

// base effect class. all types of particle effects derives from this. class handles rendering of different particle types, deping on the primitive policy chosen
template<class ParticleType, class ShapePolicy, template <class Render> class BuildPolicy>
class T_ParticleEffect : public I_ParticleEffect
{
public:
	// use this typedef to improve readability
	typedef I_ParticleEffect	Base;
	typedef ParticleType			Particle;
	
	T_ParticleEffect(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex)
	{	
		m_pParticleFormat = &ParticleType::format;
		m_SpawnSettings.pBlockManager = pManager->GetParticleBlockManager(sizeof(ParticleType));
		assert(sizeof(ParticleType) == m_pParticleFormat->GetElementSize());
	}	
	virtual ~T_ParticleEffect() { };

	// functions which depend on template arguments
	virtual void	OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const;
	virtual void	OnBuild(const C_View& view, C_CompositeRenderNode& node) const;

protected:
	
	// rendering policy for this emitter
	BuildPolicy<ShapePolicy>		m_BuildPolicy;
};

template<class ParticleType, class ShapePolicy, template <class Render> class BuildPolicy>
void T_ParticleEffect<ParticleType, ShapePolicy, BuildPolicy>::OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const
{
	return m_BuildPolicy.Render(view, pDisplay, node, states);
}

template<class ParticleType, class ShapePolicy, template <class Render> class BuildPolicy>
void T_ParticleEffect<ParticleType, ShapePolicy, BuildPolicy>::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.nNumElements = 0;

	for(S32 i = 0; i < node.blocks.Size(); ++i)
	{
		ParticleBlock* pParticleBlock = (ParticleBlock*)node.blocks[i];
		node.nNumElements += pParticleBlock->m_Element.Size();
	}

	m_BuildPolicy.Build(view, node);
}

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EFFECT3D_H
