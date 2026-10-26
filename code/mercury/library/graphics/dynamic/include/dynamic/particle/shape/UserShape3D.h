#ifndef RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_USERSHAPE3D_H
#define RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_USERSHAPE3D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"

// 
#include <system/Serialise.h>
#include <system/serialise/BinarySerialiseAutoEndian.h>
#include <system/serialise/C_FileBlock.h>

#include "Dynamic/Particle/Tools_Shared/UserShape3DData.h"
#include "Dynamic/Particle/Tools_Shared/UserShape3DVariants.h"
#include "Dynamic/Particle/Tools_Shared/UserParticleContainer.h"
#include "Dynamic/Particle/UserParticleLibrary.h"

#ifndef PLATFORM_SPU
#include "Dynamic/Particle/EffectInterface.h"
#endif

namespace DynamicEffect
{
namespace Particle
{
namespace ShapePolicy
{

class UserShape3D : public I_ShapePolicy
{
public:
	typedef Display::Vertex_XYZ_HDR_UV_N		ParticleVertex;
	typedef volatile ParticleVertex* __restrict	ParticleVertexPtr;

	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer);
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer) { return (void*)pBuffer; }

#ifndef PLATFORM_SPU
	static void		Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect) {};
	static void		End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect) {};

	static U32 GetVertexSize(const I_ParticleEffectBase* pEffect)
	{
		return sizeof(ParticleVertex);
	}
	static Display::C_VertexDeclaration* GetVertexDeclaration(const I_ParticleEffectBase* pEffect)
	{
		return ParticleVertex::m_pDeclaration;
	}
	static U32 GetNumPrimitives(const I_ParticleEffectBase* pEffect)
	{
		//Remove this checking in final version.
		assert(pEffect && pEffect->GetRenderSettings().vertexBuildSettings.iParticleLibraryIndex != -1);
		
		const C_UserShape3DVariants& variants = C_UserParticleLibrary::Instance().GetShape3DVariants(
			pEffect->GetRenderSettings().vertexBuildSettings.iParticleLibraryIndex);
		return variants.GetNumPrimitives();
	}
	static U32 GetNumVertices(const I_ParticleEffectBase* pEffect)
	{
		return m_nNumVertices;
	}
	static E_VertexShader GetVertexShader(const I_ParticleEffectBase* pEffect)
	{
		return E_VertexShader_NdotL;
	}
	static Display::E_PrimitiveType GetPrimitiveType(const I_ParticleEffectBase* pEffect)
	{
		return m_ePrimitiveType;
	}
	static const MathsStorage::Float4 GetUvScale(const I_ParticleEffectBase* pEffect)
	{
		return MathsStorage::Float4(1.0f, 1.0f, 1.0f, 1.0f);
	}
private:
	static const U32 m_nNumVertices = 3;

	static const Display::E_PrimitiveType m_ePrimitiveType = Display::E_PrimitiveType_TriangleList;
#endif
};


}
}
}

#endif
