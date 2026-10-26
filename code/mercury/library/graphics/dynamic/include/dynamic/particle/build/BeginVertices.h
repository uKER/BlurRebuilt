#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_BEGINVERTICES_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_BEGINVERTICES_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Particle/Build/Base.h"

namespace DynamicEffect
{
namespace Particle
{

namespace BuildPolicy
{

// xenon specific policy uses begin/end vertices
template<class ShapePolicy>
class T_BeginVertices : public T_Base<ShapePolicy>
{
public:
	T_BeginVertices() {}
	virtual ~T_BeginVertices() {};

	void	Build(const C_View& view, C_CompositeRenderNode& node) const
	{
		// because we are going to insert the vertex data directly into command buffer later we need to push the pointers to the data
		// into the node structure so we can get at the data later
		node.iVertexBuffer = 0xFFFFFFFF;
	}

	virtual void	Render(const C_View& view, Display::C_Context* pDisplay, const C_CompositeRenderNode& node, const U32 states) const
	{
		const I_ParticleEffectBase* pEffect = (const I_ParticleEffectBase*)node.pEffect;
		
		U32 vertexCount = node.nNumElements * ShapePolicy::GetNumPrimitives(pEffect) * ShapePolicy::GetNumVertices(pEffect);
		
		if(vertexCount > 0)
		{
			T_Base<ShapePolicy>::Bind(node, states);
		
			void* pVertices = Display::Device()->BeginVertices(ShapePolicy::GetPrimitiveType(pEffect), vertexCount, ShapePolicy::GetVertexSize(pEffect), ShapePolicy::GetVertexDeclaration(pEffect), true); // This can fail
			
			const VertexBuildConstants* pConstants = &view.GetVertexBuildConstants();
			const VertexBuildSettings* pSettings = &pEffect->GetRenderSettings().vertexBuildSettings;

			if(pVertices)
			{
				if(pEffect->GetRenderSettings().vertexBuildSettings.eParticleSort == E_ParticleSort_PerParticle)
				{
					g_ParticleSortList.Clear();
					SortParticles(pConstants, node.blocks, g_ParticleSortList);
					pVertices = ShapePolicy::Build(pSettings, pConstants, g_ParticleSortList, pVertices);
				}
				else
					pVertices = ShapePolicy::Build(pSettings, pConstants, node.blocks, pVertices);
				Display::Device()->EndVertices(pVertices);
			
			}
			T_Base<ShapePolicy>::Unbind(node, states);
		}
	}
};

} // BuildPolicy
} // Particles
} // DynamicEffect

#endif
