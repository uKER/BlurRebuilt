#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SUPEREFFECT_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SUPEREFFECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Effect3D.h"
#include "Dynamic/Particle/MathsUtil.h"
#include "Dynamic/Particle/Super.h"

namespace DynamicEffect
{
namespace Particle
{

void	FreeParticle(U32 i, ParticleBlock* pBlock, TrailPtrs& trailPtrs, C_EmitterList* pEmitterList);

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
void	UpdateTrails(const SuperEffects* pSuperEffects, S32 i, TrailPtrs& trailPtrs, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateTrails(const SuperEffects* pSuperEffects, S32 i, TrailPtrs& trailIterators, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateDeaths(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateDeaths(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateCollisions(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, XMVECTOR vTimeStep, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateCollisionsOnce(const SuperEffects* pSuperEffects, XMVECTOR vPosition, XMVECTOR qOrientation, XMVECTOR vVelocity, XMVECTOR vPlanePosition, XMVECTOR vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags);
#else
void	UpdateTrails(const SuperEffects* pSuperEffects, S32 i, TrailPtrs& trailPtrs, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateTrails(const SuperEffects* pSuperEffects, S32 i, TrailPtrs& trailIterators, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateDeaths(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateDeaths(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateCollisions(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const Maths::HF32& vTimeStep, const C_Node* pParentTransform, U32 visibilityFlags);
void	UpdateCollisionsOnce(const SuperEffects* pSuperEffects, const Maths::HVec3& vPosition, const Maths::C_Quaternion& qOrientation, const Maths::HVec3& vVelocity, const Maths::HVec3& vPlanePosition, const Maths::HVec3& vPlaneNormal, const C_Node* pParentTransform, U32 visibilityFlags);
#endif

class I_ParticleSuperEffect : public I_ParticleEffectBase
{
public:
	typedef I_ParticleEffectBase	Base;

	I_ParticleSuperEffect(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex);
	virtual ~I_ParticleSuperEffect();

	void	AddChild(E_EffectChild type, const EmitterDefinition& definition, const MathsStorage::Float4A16& settings);
	void	Free();

	virtual void		Reset()
	{
		UpdateMemory();		// update memory
		Free();				// free trail blocks
		Base::Free();		// free other blocks
	}

	friend class C_ParticleManager;
	void						SpawnCustom(C_Emitter* pEmitter, C_Random& random, SpawnSettings* pSpawnSettings, SuperEffects* pSuperEffects, const EmitterUpdateConstants* pUpdateConstants);
	SuperEffectUpdateFunctor	GetUpdateFunctor() { return m_pUpdateFunctor; }

	SuperEffects								m_SuperEffects;

protected:

	void	UpdateMemory();
	void	DeleteBlock(ParticleBlock* pBlock);
	void	DeleteTrailBlock(TrailBlock* pTrailDelete);

	SuperEffectUpdateFunctor					m_pUpdateFunctor;
};

template<class ParticleType, class ShapePolicy, template <class Render> class BuildPolicy>
class T_ParticleSuperEffect : public I_ParticleSuperEffect
{
public:
	// use this typedef to improve readability
	typedef I_ParticleSuperEffect	Base;
	typedef ParticleType				Particle;

	T_ParticleSuperEffect(C_Renderer* pRenderer, C_ParticleManager* pManager, const U32 iIndex, const U32 iTypeIndex) : Base(pRenderer, pManager, iIndex, iTypeIndex)
	{
		m_pParticleFormat = &ParticleType::format;
		m_SpawnSettings.pBlockManager = pManager->GetParticleBlockManager(sizeof(ParticleType));
		assert(sizeof(ParticleType) == m_pParticleFormat->GetElementSize());
	}
	virtual ~T_ParticleSuperEffect() { Free(); }
	
	virtual void	OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const;
	virtual void	OnBuild(const C_View& view, C_CompositeRenderNode& node) const;
protected:

	ParticleBlock*				BeginCreate(U32 nNum, const C_Emitter* pEmitter, TrailPtrs& trailPtrs);

	// rendering policy for this emitter
	BuildPolicy<ShapePolicy>	m_BuildPolicy;
};

template<class ParticleType, class ShapePolicy, template <class Render> class BuildPolicy>
void T_ParticleSuperEffect<ParticleType, ShapePolicy, BuildPolicy>::OnRender(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const
{
	return m_BuildPolicy.Render(view, pDisplay, node, states);
}

template<class ParticleType, class ShapePolicy, template <class Render> class BuildPolicy>
void T_ParticleSuperEffect<ParticleType, ShapePolicy, BuildPolicy>::OnBuild(const C_View& view, C_CompositeRenderNode& node) const
{
	node.nNumElements = 0;

	for(S32 i = 0; i < node.blocks.Size(); ++i)
	{
		ParticleBlock* pParticleBlock = (ParticleBlock*)node.blocks[i];
		node.nNumElements += pParticleBlock->m_Element.Size();
	}

	m_BuildPolicy.Build(view, node);
}

} // Particle
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_PARTICLE_SUPEREFFECT_H
