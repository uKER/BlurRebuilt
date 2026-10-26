#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_DYNAMICVERTEXBUFFER_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_DYNAMICVERTEXBUFFER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Build/Base.h"
#include "Dynamic/Particle/Build/Sort.h"
#include "Dynamic/Particle/Build/JobQueueParams.h"

namespace DynamicEffect
{
namespace Particle
{

namespace BuildPolicy
{

// xenon specific policy uses begin/end vertices
template<class ShapePolicy>
class T_DynamicVertexBuffer : public T_Base<ShapePolicy>
{
public:
	T_DynamicVertexBuffer() {}
	virtual ~T_DynamicVertexBuffer() {};

	void	Build(const C_View& view, C_CompositeRenderNode& node) const
	{
		PERF_EVENT("Particle.BuildDVB");
			
		const I_ParticleEffectBase* pEffect = (const I_ParticleEffectBase*)node.pEffect;

		U32 vertices = node.nNumElements * pEffect->GetRenderSettings().vertices;
			
		if(vertices > 0)
		{
			C_DynamicVertexBufferBlock block = pEffect->GetRenderer()->GetDynamicVertexBuffer()->Begin("Emitter", pEffect->GetRenderSettings().vertexSize, vertices);
	
			const VertexBuildConstants* pConstants = &view.GetVertexBuildConstants();
			const VertexBuildSettings* pSettings = &pEffect->GetRenderSettings().vertexBuildSettings;

			if(block.pVertices)
			{
				if(pEffect->GetRenderSettings().vertexBuildSettings.eParticleSort == E_ParticleSort_PerParticle)
				{
					g_ParticleSortList.Clear();
					SortParticles(pConstants, node.blocks, g_ParticleSortList);
					ShapePolicy::Build(pSettings, pConstants, g_ParticleSortList, block.pVertices);
				}
				else
				{
					ShapePolicy::Build(pSettings, pConstants, node.blocks, block.pVertices);
				}
				view.AddDynamicVertexBufferBlock(block);
			}
			// store index to the vertex buffer block for later
			node.iVertexBuffer = block.blockIndex;
			return;
		}
		
		node.iVertexBuffer = 0xFFFFFFFF;
	}

	virtual void	Render(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const
	{
		const I_ParticleEffectBase* pEffect = (const I_ParticleEffectBase*)node.pEffect;
		
		if(node.iVertexBuffer != 0xFFFFFFFF)
		{
			T_Base<ShapePolicy>::Bind(pDisplay, node, states);
			// draw the block using draw primitive block
			pEffect->GetRenderer()->GetDynamicVertexBuffer()->DrawPrimitiveBlock(pDisplay, pEffect->GetRenderSettings().primitiveType, node.iVertexBuffer);
			T_Base<ShapePolicy>::Unbind(pDisplay, node, states);
		}
	}
};

} // BuildPolicy
} // Particles
} // DynamicEffect

#endif
