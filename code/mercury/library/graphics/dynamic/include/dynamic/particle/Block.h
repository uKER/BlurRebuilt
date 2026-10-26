#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BLOCK_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BLOCK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Dynamic/Block.h>

#ifndef PLATFORM_SPU
#include <SceneGraph/GatherNode.h>
#endif

namespace DynamicEffect
{
namespace Particle
{

class I_ParticleSuperEffect;

class ALIGN16 ParticleTrailBlock
{
public:
	void Clear()
	{
		m_Element.Clear();
	}
	
	I_ParticleSuperEffect*		m_pEffect;
	U32								m_VectorPosition;
	S32								m_EffectIndex;
	ElementVector					m_Element;
};

class ALIGN16 ParticleBlock : public C_RenderNode
{
public:
	ParticleBlock() : C_RenderNode(), m_fTime(0.0f), m_fExpiryTime(0.0f), m_pInfo(NULL), m_FrameIndex(0), m_VisibilityFlags(0XFFFFFFFF) {};

	void Clear()
	{
		m_Element.Clear();
	}

	void Reset()
	{
		m_Element.Clear();
		C_RenderNode::Reset();
	}

	void Destroy()
	{
	};

#ifndef PLATFORM_SPU
	// SceneGraph::I_GatherNode integration
	void	OnGather(const SceneGraph::C_View& view) const { Gather(view); }
	void	Gather(const SceneGraph::C_View& view) const;
#endif

	void	CalculateExtents(F32 cullDistance, U32 vertexSize);
	void	SetVisibilityFlags(U32 visibilityFlags) {m_VisibilityFlags = visibilityFlags;} 
	void*	NewParticle() { return m_Element.PushBack(); }
	void*	NewParticles(S32 num) { return m_Element.PushBack(num); }
	void	FreeParticle(const U32 i);

	U32					m_iSalt;				// so emitters can track blocks effectively - this is intentionally not initialised!
	mutable F32			m_fTime;				// time this block was last drawn for garbage collection
	mutable F32			m_fExpiryTime;			// time offscreen before this block expires	
	void*				m_pInfo;				// pointer to a structure containing extra info, type is situation dependant
	U32					m_VectorPosition;
	ElementVector		m_Element;
	U32					m_FrameIndex;
	U32					m_VisibilityFlags;
};

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_BLOCK_H
