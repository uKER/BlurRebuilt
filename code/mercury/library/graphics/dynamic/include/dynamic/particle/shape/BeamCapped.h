#ifndef RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_BEAMCAPPED_H
#define RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_BEAMCAPPED_H

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

class BeamCapped : public I_ShapePolicy
{
public:
	typedef Display::Vertex_XYZ_HDR_SUV ParticleVertex;
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
	static E_VertexShader GetVertexShader(const I_ParticleEffectBase* pEffect)
	{
		return E_VertexShader_UV;
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
		return MathsStorage::Float4(0.000030517578125f, 0.000030517578125f, 1.0f, 1.0f);
	}
private:
	// use triangle lists if no support for quads
	static const U32 m_nNumPrimitives = 4;
	static const U32 m_nNumVertices = 3;
	static const Display::E_PrimitiveType	m_ePrimitiveType = Display::E_PrimitiveType_TriangleList;
#endif
};

}
} // ShapePolicy
} // Particles

#endif
