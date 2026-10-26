#ifndef RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_NONE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_NONE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"

namespace DynamicEffect
{
namespace Particle
{
namespace ShapePolicy
{

class None : public I_ShapePolicy
{
public:
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer) { return NULL; }
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer) { return NULL; }
	
#ifndef PLATFORM_SPU
	static void		Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect) {};
	static void		End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect) {};

	static U32 GetVertexSize(const I_ParticleEffectBase* pEffect)
	{
		return 0;
	}
	static Display::C_VertexDeclaration* GetVertexDeclaration(const I_ParticleEffectBase* pEffect)
	{
		return NULL;
	}
	static E_VertexShader GetVertexShader(const I_ParticleEffectBase* pEffect)
	{
		return E_VertexShader_Unknown;
	}
	static U32	GetNumPrimitives(const I_ParticleEffectBase* pEffect)
	{
		return m_nNumPrimitives;
	}
	static U32 GetNumVertices(const I_ParticleEffectBase* pEffect)
	{
		return m_nNumVertices;
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
	static const U32 m_nNumPrimitives = 0;
	static const U32 m_nNumVertices = 0;
	static const Display::E_PrimitiveType	m_ePrimitiveType = Display::E_PrimitiveType_Unknown;
#endif	
};

}
}
}

#endif
