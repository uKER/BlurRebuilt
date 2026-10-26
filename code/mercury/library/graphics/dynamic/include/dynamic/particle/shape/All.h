#ifndef RENDER_DYNAMICEFFECT_PARTICLE_ALL_H
#define RENDER_DYNAMICEFFECT_PARTICLE_ALL_H

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
#include "SpriteRotatingDefault.h"
#include "SpriteRotatingLimitSize.h"
#include "SpriteRotatingSoft.h"
#include "SpriteRotatingSoftLimitSize.h"
#include "Cube.h"
#include "Pyramid.h"
#include "Quad.h"
#include "QuadAngleFade.h"
#include "QuadAngleFadeLimitSize.h"
#if !PLATFORM_SPU
#include "UserShape3D.h"
#endif

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

class All : public I_ShapePolicy
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

INLINE E_ParticleShape CheckLimitSize(E_ParticleShape shape)
{
#ifndef PLATFORM_SPU
	// Global variable to turn off the 'LimitSize' option:
	if (!::DynamicEffect::Particle::g_LimitScreenSize)
	{
		switch (shape)
		{
			case E_ParticleShape_SpriteLimitSize:
				shape = E_ParticleShape_Sprite;
			break;
			case E_ParticleShape_SpriteRotatingLimitSize:
				shape = E_ParticleShape_SpriteRotating;
			break;
			case E_ParticleShape_SpriteRotatingSoftLimitSize:
				shape = E_ParticleShape_SpriteRotatingSoft;
			break;
			case E_ParticleShape_QuadAngleFadeLimitSize:
				shape = E_ParticleShape_QuadAngleFade;
			break;
		}
	}
#endif
	return shape;
}

#ifndef PLATFORM_SPU
INLINE void All::Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimitSize(shape);

	switch(shape)
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
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::Begin(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::Begin(pDisplay, pEffect);
	case E_ParticleShape_Cube:
		return Cube::Begin(pDisplay, pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::Begin(pDisplay, pEffect);
	case E_ParticleShape_Quad:
		return Quad::Begin(pDisplay, pEffect);
	case E_ParticleShape_QuadAngleFade:
		return QuadAngleFade::Begin(pDisplay, pEffect);
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return QuadAngleFadeLimitSize::Begin(pDisplay, pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::Begin(pDisplay, pEffect);
	default:
		return;
	}
}

INLINE void All::End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimitSize(shape);

	switch(shape)
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
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::End(pDisplay, pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::End(pDisplay, pEffect);
	case E_ParticleShape_Cube:
		return Cube::End(pDisplay, pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::End(pDisplay, pEffect);
	case E_ParticleShape_Quad:
		return Quad::End(pDisplay, pEffect);
	case E_ParticleShape_QuadAngleFade:
		return QuadAngleFade::End(pDisplay, pEffect);
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return QuadAngleFadeLimitSize::End(pDisplay, pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::End(pDisplay, pEffect);
	default:
		return;
	}
}

INLINE U32 All::GetVertexSize(const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimitSize(shape);

	switch(shape)
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
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::GetVertexSize(pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::GetVertexSize(pEffect);
	case E_ParticleShape_Cube:
		return Cube::GetVertexSize(pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::GetVertexSize(pEffect);
	case E_ParticleShape_Quad:
	case E_ParticleShape_QuadAngleFade:
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return Quad::GetVertexSize(pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::GetVertexSize(pEffect);
	default:
		return 0;
	}
}

INLINE E_VertexShader All::GetVertexShader(const I_ParticleEffectBase* pEffect)
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
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::GetVertexShader(pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::GetVertexShader(pEffect);
	case E_ParticleShape_Cube:
		return Cube::GetVertexShader(pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::GetVertexShader(pEffect);
	case E_ParticleShape_Quad:
	case E_ParticleShape_QuadAngleFade:
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return Quad::GetVertexShader(pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::GetVertexShader(pEffect);
	default:
		return E_VertexShader_Unknown;
	}
}

INLINE Display::C_VertexDeclaration* All::GetVertexDeclaration(const I_ParticleEffectBase* pEffect)
{
	E_ParticleShape shape = pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape;
	shape = CheckLimitSize(shape);

	switch(shape)
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
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::GetVertexDeclaration(pEffect);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::GetVertexDeclaration(pEffect);
	case E_ParticleShape_Cube:
		return Cube::GetVertexDeclaration(pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::GetVertexDeclaration(pEffect);
	case E_ParticleShape_Quad:
	case E_ParticleShape_QuadAngleFade:
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return Quad::GetVertexDeclaration(pEffect);
	case E_ParticleShape_Mesh:
		return Cube::GetVertexDeclaration(pEffect);
	default:
		return NULL;
	}
}

INLINE U32 All::GetNumPrimitives(const I_ParticleEffectBase* pEffect)
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
	case E_ParticleShape_SpriteRotating:
	case E_ParticleShape_SpriteRotatingLimitSize:
	case E_ParticleShape_SpriteRotatingSoft:
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingDefault::GetNumPrimitives(pEffect);
	case E_ParticleShape_Cube:
		return Cube::GetNumPrimitives(pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::GetNumPrimitives(pEffect);
	case E_ParticleShape_Quad:
	case E_ParticleShape_QuadAngleFade:
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return Quad::GetNumPrimitives(pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::GetNumPrimitives(pEffect);
	default:
		return 0;
	}
}

INLINE Display::E_PrimitiveType All::GetPrimitiveType(const I_ParticleEffectBase* pEffect)
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
	case E_ParticleShape_SpriteRotating:
	case E_ParticleShape_SpriteRotatingLimitSize:
	case E_ParticleShape_SpriteRotatingSoft:
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingDefault::GetPrimitiveType(pEffect);
	case E_ParticleShape_Cube:
		return Cube::GetPrimitiveType(pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::GetPrimitiveType(pEffect);
	case E_ParticleShape_Quad:
	case E_ParticleShape_QuadAngleFade:
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return Quad::GetPrimitiveType(pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::GetPrimitiveType(pEffect);
	default:
		return Display::E_PrimitiveType_Unknown;
	}
}

INLINE U32 All::GetNumVertices(const I_ParticleEffectBase* pEffect)
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
	case E_ParticleShape_SpriteRotating:
	case E_ParticleShape_SpriteRotatingLimitSize:
	case E_ParticleShape_SpriteRotatingSoft:
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingDefault::GetNumVertices(pEffect);
	case E_ParticleShape_Cube:
		return Cube::GetNumVertices(pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::GetNumVertices(pEffect);
	case E_ParticleShape_Quad:
	case E_ParticleShape_QuadAngleFade:
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return Quad::GetNumVertices(pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::GetNumVertices(pEffect);
	default:
		return 0;
	}
}

INLINE MathsStorage::Float4 All::GetUvScale(const I_ParticleEffectBase* pEffect)
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
	case E_ParticleShape_SpriteRotating:
	case E_ParticleShape_SpriteRotatingLimitSize:
	case E_ParticleShape_SpriteRotatingSoft:
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingDefault::GetUvScale(pEffect);
	case E_ParticleShape_Cube:
		return Cube::GetUvScale(pEffect);
	case E_ParticleShape_Pyramid:
		return Pyramid::GetUvScale(pEffect);
	case E_ParticleShape_Quad:
	case E_ParticleShape_QuadAngleFade:
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return Quad::GetUvScale(pEffect);
	case E_ParticleShape_Mesh:
		return UserShape3D::GetUvScale(pEffect);
	default:
		return MathsStorage::Float4();
	}
}
#endif

INLINE void* All::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	E_ParticleShape shape = pSettings->eParticleShape;
	shape = CheckLimitSize(shape);

	switch(shape)
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
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_Cube:
		return Cube::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_Pyramid:
		return Pyramid::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_Quad:
		return Quad::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_QuadAngleFade:
		return QuadAngleFade::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return QuadAngleFadeLimitSize::Build(pSettings, pConstants, sortedList, pBuffer);
	case E_ParticleShape_Mesh:
	#if !PLATFORM_SPU
		return UserShape3D::Build(pSettings, pConstants, sortedList, pBuffer);
	#else
		return Cube::Build(pSettings, pConstants, sortedList, pBuffer);
	#endif
	default:
		return NULL;
	}
}

INLINE void* All::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	E_ParticleShape shape = pSettings->eParticleShape;
	shape = CheckLimitSize(shape);

	// build particles in dynamic vb block
	
	switch(shape)
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
	case E_ParticleShape_SpriteRotating:
		return SpriteRotatingDefault::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteRotatingLimitSize:
		return SpriteRotatingLimitSize::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteRotatingSoft:
		return SpriteRotatingSoft::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_SpriteRotatingSoftLimitSize:
		return SpriteRotatingSoftLimitSize::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_Cube:
		return Cube::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_Pyramid:
		return Pyramid::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_Quad:
		return Quad::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_QuadAngleFade:
		return QuadAngleFade::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_QuadAngleFadeLimitSize:
		return QuadAngleFadeLimitSize::Build(pSettings, pConstants, blocks, pBuffer);
	case E_ParticleShape_Mesh:
	#if !PLATFORM_SPU
		return UserShape3D::Build(pSettings, pConstants, blocks, pBuffer);
	#else
		return Cube::Build(pSettings, pConstants, blocks, pBuffer);
	#endif
	default:
		return NULL;
	}
}

}
}
}

#endif
