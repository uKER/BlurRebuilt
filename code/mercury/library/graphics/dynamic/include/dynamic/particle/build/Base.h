#ifndef RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_BASE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_BUILDPOLICY_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Shaders/vsConstants.h"
#include "Display/Context/Context.h"

namespace DynamicEffect
{
namespace Particle
{
namespace BuildPolicy
{
// base policy
template<class ShapePolicy>
class T_Base
{
public:
	
	void Bind(Display::C_Context* pDisplay, const C_CompositeRenderNode& renderNode, const U32 states) const
	{
		const I_ParticleEffectBase* pEffect = (const I_ParticleEffectBase*)renderNode.pEffect;

		if(states & E_States_BindEffect)
		{
			const RenderSettings& renderSettings = pEffect->GetRenderSettings();
			// set render states based on the definition file
			pEffect->GetManager()->SetMaterial(pDisplay, renderSettings);
			
			// set render states based on the shape type
			ShapePolicy::Begin(pDisplay, pEffect);

			// bind vertex shader
			pEffect->GetManager()->SetVertexShader(pDisplay, renderSettings.vertexShader, E_VertexShaderVariation_Default);

			// set declaration
			pDisplay->SetVertexDeclaration(renderSettings.pVertexDeclaration);

			// set uv scale
			pDisplay->SetVertexShaderConstantF(CV_RENDER_SETTINGS, (F32*)&renderSettings.uvScale, 1);
		}

		if(states & E_States_BindEffectVolatile)
		{
			// set render states based on the batch (eg lights etc)
			/*E_VertexShaderVariation vertexShaderVariation;
			if(renderNode.flags != 0)
			{
				vertexShaderVariation = E_VertexShaderVariation_PointLights;
				pEffect->GetRenderer()->SetLightConstants4(pDisplay, renderNode.iFlags);
			}
			else
				vertexShaderVariation = E_VertexShaderVariation_Default;*/
			
			// bind vertex shader
			pEffect->GetManager()->SetVertexShader(pDisplay, pEffect->GetRenderSettings().vertexShader, E_VertexShaderVariation_Default);
		}
	}

	void Unbind(Display::C_Context* pDisplay, const C_CompositeRenderNode& renderNode, const U32 states) const
	{
		if(states & E_States_UnbindEffect)
		{
			ShapePolicy::End(pDisplay, (const I_ParticleEffectBase*)renderNode.pEffect);
		}
	}
};

} // BuildPolicy
} // Particles
} // DynamicEffect

#endif
