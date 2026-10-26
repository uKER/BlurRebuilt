#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SPRITEROTATING_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SPRITEROTATING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"
#include "SpriteRotatingDefault.h"
#include "SpriteRotatingLimitSize.h"
#include "SpriteRotatingSoft.h"
#include "SpriteRotatingSoftLimitSize.h"

// dynamic effect level includes
#include "Dynamic/Node.h"
#include "Dynamic/RenderNode.h"
#include "Dynamic/VertexBuildConstants.h"

// particle level includes
#include "Dynamic/Particle/Particle.h"
#include "Dynamic/Particle/Block.h"
#include "Dynamic/Particle/VertexBuildSettings.h"

#ifndef PLATFORM_SPU
#include "Dynamic/Particle/EffectInterface.h"
#endif

namespace DynamicEffect
{
namespace Particle
{
extern Debug::Bool g_LimitScreenSize;

namespace ShapePolicy
{
// This SpriteRotating policy includes the cube, pyramid, and quad policies with data driven selection
class SpriteRotating : public I_ShapePolicy
{
public:
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer);
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer);
	
#ifndef PLATFORM_SPU
	static void		Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect);
	static void		End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect);

	static Display::C_VertexDeclaration*			GetVertexDeclaration(const I_ParticleEffectBase* pEffect);
	static U32										GetVertexSize(const I_ParticleEffectBase* pEffect);
	static E_VertexShader							GetVertexShader(const I_ParticleEffectBase* pEffect);
	static U32										GetNumPrimitives(const I_ParticleEffectBase* pEffect);
	static U32										GetNumVertices(const I_ParticleEffectBase* pEffect);
	static Display::E_PrimitiveType					GetPrimitiveType(const I_ParticleEffectBase* pEffect);
	static MathsStorage::Float4						GetUvScale(const I_ParticleEffectBase* pEffect);
#endif
	static E_ParticleShape CheckLimit(E_ParticleShape shape);

private:
	ShapePolicy::SpriteRotatingDefault				m_SpriteRotatingDefault;
	ShapePolicy::SpriteRotatingLimitSize			m_SpriteRotatingLimitSize;
	ShapePolicy::SpriteRotatingSoft					m_SpriteRotatingSoft;
	ShapePolicy::SpriteRotatingSoftLimitSize			m_SpriteRotatingSoftLimitSize;
};

#ifndef PLATFORM_SPU
INLINE void SpriteRotating::Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimit(shape);

	switch(shape)
	{
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::Begin(pDisplay, pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return;
	}
}

INLINE void SpriteRotating::End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimit(shape);

	switch(shape)
	{
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::End(pDisplay, pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return;
	}
}

INLINE U32 SpriteRotating::GetVertexSize(const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimit(shape);

	switch(shape)
	{
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::GetVertexSize(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return 0;
	}
}

INLINE E_VertexShader SpriteRotating::GetVertexShader(const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;

	switch(shape)
	{
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::GetVertexShader(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return E_VertexShader_Unknown;
	}
}

INLINE Display::C_VertexDeclaration* SpriteRotating::GetVertexDeclaration(const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimit(shape);

	switch(shape)
	{
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::GetVertexDeclaration(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return NULL;
	}
}

INLINE U32 SpriteRotating::GetNumPrimitives(const I_ParticleEffectBase* pEffect)
{
	return SpriteRotatingDefault::GetNumPrimitives(pEffect);
}

INLINE Display::E_PrimitiveType SpriteRotating::GetPrimitiveType(const I_ParticleEffectBase* pEffect)
{
	return SpriteRotatingDefault::GetPrimitiveType(pEffect);
}

INLINE U32 SpriteRotating::GetNumVertices(const I_ParticleEffectBase* pEffect)
{
	return SpriteRotatingDefault::GetNumVertices(pEffect);
}

INLINE MathsStorage::Float4 SpriteRotating::GetUvScale(const I_ParticleEffectBase* pEffect)
{
	return SpriteRotatingDefault::GetUvScale(pEffect);
}
#endif

INLINE void* SpriteRotating::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	E_ParticleShape shape = pSettings->eParticleShape;
	shape = CheckLimit(shape);

	switch(shape)
	{
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::Build(pSettings, pConstants, sortedList, pBuffer);
	default:
		return NULL;
	}
}

INLINE void* SpriteRotating::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	E_ParticleShape shape = pSettings->eParticleShape;
	shape = CheckLimit(shape);

	// build particles in dynamic vb block
	
	switch(shape)
	{
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::Build(pSettings, pConstants, blocks, pBuffer);
	default:
		return NULL;
	}
}

INLINE E_ParticleShape SpriteRotating::CheckLimit(E_ParticleShape shape)
{
#ifndef PLATFORM_SPU
	if (!g_LimitScreenSize)
	{
		switch (shape)
		{
			case E_ParticleShape_SpriteRotatingLimitSize:
				shape = E_ParticleShape_SpriteRotating;
			break;
			case E_ParticleShape_SpriteRotatingSoftLimitSize:
				shape = E_ParticleShape_SpriteRotatingSoft;
			break;
		}
	}
#endif
	return shape;
}

}
}
}

#endif
