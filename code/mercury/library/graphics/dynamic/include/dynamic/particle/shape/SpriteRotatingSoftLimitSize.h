#ifndef RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_SPRITEROTATINGSOFTLIMITSIZE_H
#define RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_SPRITEROTATINGSOFTLIMITSIZE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"
#include "Dynamic/TextureCoords.h"

namespace DynamicEffect
{
namespace Particle
{
namespace ShapePolicy
{

class SpriteRotatingSoftLimitSize : public I_ShapePolicy
{
public:
	typedef Display::Vertex_XYZ_HDR_UV2 ParticleVertex;
	//typedef volatile ParticleVertex* __restrict	ParticleVertexPtr;
	typedef volatile ParticleVertex* ParticleVertexPtr;
	
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer);
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer);

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
	static E_VertexShader GetVertexShader(const I_ParticleEffectBase* pEffect);

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
#ifndef PLATFORM_PC
	static const U32 m_nNumPrimitives = 1;
	static const U32 m_nNumVertices = 4;
	static const Display::E_PrimitiveType	m_ePrimitiveType = Display::E_PrimitiveType_QuadList;
#else
	static const U32 m_nNumPrimitives = 2;
	static const U32 m_nNumVertices = 3;
	static const Display::E_PrimitiveType	m_ePrimitiveType = Display::E_PrimitiveType_TriangleList;
#endif
#endif
};


}
}
}

#endif
