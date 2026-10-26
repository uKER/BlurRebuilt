#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_ALL_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_ALL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Build/DynamicVertexBuffer.h"
#include "Dynamic/Particle/Build/JobQueueDynamicVertexBuffer.h"

namespace DynamicEffect
{
namespace Particle
{

#ifndef CFG_SPEED
extern Debug::Bool	g_MultiThreadedVertexBuild;
#endif

namespace BuildPolicy
{

#ifdef CFG_SPEED
// in speed we define the policy to just be dynamic vertex buffer
template<class ShapePolicy>
class T_All : public T_JobQueueDynamicVertexBuffer<ShapePolicy> {};
#else
template<class ShapePolicy>
class T_All
{
public:
	T_All() {}
	virtual ~T_All() {}

	void	Build(const C_View& view, C_CompositeRenderNode& node) const
	{
		const I_ParticleEffectBase* pEffect = (const I_ParticleEffectBase*)node.pEffect;
		if(pEffect->GetRenderer()->JobQueueActive() & g_MultiThreadedVertexBuild)
			m_JobQueueDynamicVertexBuffer.Build(view, node);
		else
			m_DynamicVertexBuffer.Build(view, node);
	}

	virtual void	Render(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const
	{
		const I_ParticleEffectBase* pEffect = (const I_ParticleEffectBase*)node.pEffect;
		if(pEffect->GetRenderer()->JobQueueActive() & g_MultiThreadedVertexBuild)
			m_JobQueueDynamicVertexBuffer.Render(view, pDisplay, node, states);
		else
			m_DynamicVertexBuffer.Render(view, pDisplay, node, states);
	}

private:

	T_DynamicVertexBuffer<ShapePolicy>				m_DynamicVertexBuffer;
	T_JobQueueDynamicVertexBuffer<ShapePolicy>		m_JobQueueDynamicVertexBuffer;
};
#endif

} // BuildPolicy
} // Particles
} // DynamicEffect

#endif
