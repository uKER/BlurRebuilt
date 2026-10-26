#ifndef RENDER_DYNAMICEFFECT_PARTICLE_SHAPE3D_H
#define RENDER_DYNAMICEFFECT_PARTICLE_SHAPE3D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ShapePolicyInterface.h"
#include "Cube.h"
#include "Pyramid.h"
#include "Quad.h"
#include "QuadAngleFade.h"
#include "QuadAngleFadeLimitSize.h"
#include "UserShape3D.h"

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
// This Shape3D policy includes the cube, pyramid, and quad policies with data driven selection

class Shape3D : public I_ShapePolicy
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
INLINE void Shape3D::Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return;
	}
}

INLINE void Shape3D::End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return;
	}
}

INLINE U32 Shape3D::GetVertexSize(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return 0;
	}
}

INLINE E_VertexShader Shape3D::GetVertexShader(const I_ParticleEffectBase* pEffect)
{
	return E_VertexShader_NdotL;
}

INLINE Display::C_VertexDeclaration* Shape3D::GetVertexDeclaration(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return NULL;
	}
}

INLINE U32 Shape3D::GetNumPrimitives(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return 0;
	}
}

INLINE Display::E_PrimitiveType Shape3D::GetPrimitiveType(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return Display::E_PrimitiveType_Unknown;
	}
}

INLINE U32 Shape3D::GetNumVertices(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return 0;
	}
}


INLINE MathsStorage::Float4 Shape3D::GetUvScale(const I_ParticleEffectBase* pEffect)
{
	switch(pEffect->GetRenderSettings().vertexBuildSettings.eParticleShape)
	{
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
		BIZ_ASSERT_FAIL("Unsupported particle type");
		return MathsStorage::Float4();
	}
}
#endif


INLINE void* Shape3D::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const ParticleArray& sortedList, volatile void* __restrict pBuffer)
{
	switch(pSettings->eParticleShape)
	{
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
		return UserShape3D::Build(pSettings, pConstants, sortedList, pBuffer);
	default:
		return NULL;
	}
}


INLINE void* Shape3D::Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pBuffer)
{
	// build particles in dynamic vb block
	
	switch(pSettings->eParticleShape)
	{
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
		return UserShape3D::Build(pSettings, pConstants, blocks, pBuffer);
	default:
		return NULL;
	}
}

}
}
}

#endif
