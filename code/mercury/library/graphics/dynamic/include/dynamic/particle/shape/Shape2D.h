#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SHAPE2D_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SHAPE2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"
#include "Beam.h"
#include "BeamCapped.h"
#include "PointSprite.h"
#include "PointSpriteLine.h"
#include "Sprite.h"
#include "SpriteLimitSize.h"
#include "SpriteDirection.h"
#include "None.h"

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
namespace ShapePolicy
{

class Shape2D : public I_ShapePolicy
{
public:
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer);
	static void*	Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer);

#ifndef PLATFORM_SPU
	static void		Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect);
	static void		End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect);
	
	static Display::C_VertexDeclaration*		GetVertexDeclaration(const I_ParticleEffectBase* pEffect);
	static U32									GetVertexSize(const I_ParticleEffectBase* pEffect);
	static E_VertexShader						GetVertexShader(const I_ParticleEffectBase* pEffect);
	static U32									GetNumPrimitives(const I_ParticleEffectBase* pEffect);
	static U32									GetNumVertices(const I_ParticleEffectBase* pEffect);
	static Display::E_PrimitiveType				GetPrimitiveType(const I_ParticleEffectBase* pEffect);
	static MathsStorage::Float4					GetUvScale(const I_ParticleEffectBase* pEffect);
#endif
};

#ifndef PLATFORM_SPU
INLINE void Shape2D::Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::Begin(pDisplay, pEffect);
	case E_ParticleShape_Beam:
		return Beam::Begin(pDisplay, pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::Begin(pDisplay, pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::Begin(pDisplay, pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::Begin(pDisplay, pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::Begin(pDisplay, pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return;
	}
}

INLINE void Shape2D::End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::End(pDisplay, pEffect);
	case E_ParticleShape_Beam:
		return Beam::End(pDisplay, pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::End(pDisplay, pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::End(pDisplay, pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::End(pDisplay, pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::End(pDisplay, pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return;
	}
}

INLINE U32 Shape2D::GetVertexSize(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::GetVertexSize(pEffect);
	case E_ParticleShape_Beam:
		return Beam::GetVertexSize(pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::GetVertexSize(pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::GetVertexSize(pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::GetVertexSize(pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::GetVertexSize(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return 0;
	}
}

INLINE E_VertexShader Shape2D::GetVertexShader(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::GetVertexShader(pEffect);
	case E_ParticleShape_Beam:
		return Beam::GetVertexShader(pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::GetVertexShader(pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::GetVertexShader(pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::GetVertexShader(pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::GetVertexShader(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return E_VertexShader_Unknown;
	}
}

INLINE Display::C_VertexDeclaration* Shape2D::GetVertexDeclaration(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::GetVertexDeclaration(pEffect);
	case E_ParticleShape_Beam:
		return Beam::GetVertexDeclaration(pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::GetVertexDeclaration(pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::GetVertexDeclaration(pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::GetVertexDeclaration(pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::GetVertexDeclaration(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return NULL;
	}
}

INLINE U32 Shape2D::GetNumPrimitives(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::GetNumPrimitives(pEffect);
	case E_ParticleShape_Beam:
		return Beam::GetNumPrimitives(pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::GetNumPrimitives(pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::GetNumPrimitives(pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::GetNumPrimitives(pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::GetNumPrimitives(pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::GetNumPrimitives(pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::GetNumPrimitives(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return 0;
	}
}

INLINE Display::E_PrimitiveType Shape2D::GetPrimitiveType(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::GetPrimitiveType(pEffect);
	case E_ParticleShape_Beam:
		return Beam::GetPrimitiveType(pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::GetPrimitiveType(pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::GetPrimitiveType(pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::GetPrimitiveType(pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::GetPrimitiveType(pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::GetPrimitiveType(pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::GetPrimitiveType(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return Display::E_PrimitiveType_Unknown;
	}
}

INLINE U32 Shape2D::GetNumVertices(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::GetNumVertices(pEffect);
	case E_ParticleShape_Beam:
		return Beam::GetNumVertices(pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::GetNumVertices(pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::GetNumVertices(pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::GetNumVertices(pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::GetNumVertices(pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::GetNumVertices(pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::GetNumVertices(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return 0;
	}
}

INLINE MathsStorage::Float4 Shape2D::GetUvScale(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
	case E_ParticleShape_None:
		return None::GetUvScale(pEffect);
	case E_ParticleShape_Beam:
		return Beam::GetUvScale(pEffect);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::GetUvScale(pEffect);
	case E_ParticleShape_PointSprite:
		return PointSprite::GetUvScale(pEffect);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::GetUvScale(pEffect);
	case E_ParticleShape_Sprite:
		return Sprite::GetUvScale(pEffect);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::GetUvScale(pEffect);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::GetUvScale(pEffect);
	default:
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return MathsStorage::Float4();
	}
}
#endif

INLINE void* Shape2D::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	switch(pSettings->eParticleShape)
	{
	case E_ParticleShape_None:
		return None::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_Beam:
		return Beam::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_PointSprite:
		return PointSprite::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_Sprite:
		return Sprite::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::Build(pSettings, pConstants, sortedList, pBuffer);
	default:
		return NULL;
	}
}

INLINE void* Shape2D::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	// build particles in dynamic vb block
	
	switch(pSettings->eParticleShape)
	{
	case E_ParticleShape_None:
		return None::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_Beam:
		return Beam::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_BeamCapped:
	case E_ParticleShape_BeamCappedNoLifeScale:
		return BeamCapped::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_PointSprite:
		return PointSprite::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_PointSpriteLine:
		return PointSpriteLine::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_Sprite:
		return Sprite::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteLimitSize:
		return SpriteLimitSize::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteDirectional:
		return SpriteDirection::Build(pSettings, pConstants, blocks, pBuffer);
	default:
		return NULL;
	}
}

}
}
}

#endif
