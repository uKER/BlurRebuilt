#ifndef RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_H
#define RENDER_DYNAMICEFFECT_PARTICLE_RENDERPOLICY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display\Buffer\VertexDeclarations.h"
#include "Dynamic\Particle\Build\Sort.h"
#include "Dynamic\Particle\Enums.h"
#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

namespace Display
{
	class C_Context;
}


namespace DynamicEffect
{

struct VertexBuildConstants;
class RenderNodePtrs;

namespace Particle
{

struct VertexBuildSettings;
class ParticleArray;
class I_ParticleEffectBase;

#ifdef PLATFORM_SPU
template<class T>
class C_SpuDoubleBuffer
{
public:
	void Alloc(U32 size)
	{
		m_pBuffer[0] = (T*)Spu::Alloc(size);
		m_pBuffer[1] = (T*)Spu::Alloc(size);
		m_Size = size;
		m_Index = 0;
	}
	T*	GetBuffer()
	{
		return m_pBuffer[m_Index];
	}
	U32 GetSize()
	{
		return m_Size;
	}
	U32 GetIndex()
	{
		return m_Index;
	}
	void Wait()
	{
		Spu::Wait(m_Index);
	}
	void Swap()
	{
		m_Index ^= 1;
	}
private:
	T*	m_pBuffer[2];
	U32	m_Index;
	U32 m_Size;
};
#endif

INLINE F32 GetAnimationPosition(F32 fUnitLife, F32 fLifeScale, F32 fAnimationDuration)
{
	Maths::VF32	vAnimationDuration		= Maths::MakeVF32(fAnimationDuration);

	//F32 fTotalLifeTime		 = 1.0f / fLifeScale;
	Maths::VF32 vTotalLifeTime	 = Maths::Rcp24( Maths::MakeVF32(fLifeScale) );

	//F32 fCurrentLifeTime		 = fUnitLife * fTotalLifeTime;
	Maths::VF32	vCurrentLifeTime = Maths::MakeVF32(fUnitLife) * vTotalLifeTime;

	//F32 fCurPosInAnim			 = __builtin_ fmodf(fCurrentLifeTime, fAnimationDuration);
	Maths::VF32	vCurPosInAnim	 = Maths::Mod(vCurrentLifeTime, vAnimationDuration);

	//F32 fCurPosInAnim  		 = vCurPosInAnim / fAnimationDuration;
	vCurPosInAnim		 		 = vCurPosInAnim * Maths::Rcp24(vAnimationDuration);


	//printf("life:%f, anim dur:%f, cur pos:%f, unit pos:%f\n", fCurrentLifeTime, fAnimationDuration, fCurPosInAnimDuration, fUnitPosInAnimDuration);
	//return fCurPosInAnimDuration;
	return Maths::GetF32(vCurPosInAnim);
}

INLINE U32 GetAnimatedTextureCoordinate(F32 fLife, U32 iAtlasSize, U32 iAtlasOffset)
{
	F32 fAtlasSize = fLife * (F32)iAtlasSize;
	U32 iCoord = (U32)fAtlasSize;
	return iCoord + iAtlasOffset;
}

class I_ShapePolicy
{
public:
	I_ShapePolicy() {};
	/*
	// render policies must implement these functions
	virtual void			Render(const Batch& batch) const {};
	virtual void			Begin(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect) const = 0;
	virtual void			End(Display::C_Context* pDisplay, const I_ParticleEffectBase* pEffect) const = 0;

	// build functions for batched and non batched, sorted and non sorted
	virtual void* Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const RenderNodePtrs& blocks, volatile void* __restrict pVertices) const = 0;
	virtual void* Build(const VertexBuildSettings* pSettings, const VertexBuildConstants* pConstants, const I_ParticleEffectBase* pEffect, const ParticleArray& sortedList, volatile void* __restrict pVertices) const = 0;
	virtual void* BuildBatch(const RenderNodes& blocks, volatile void* __restrict pVertices) const = 0;
	virtual void* BuildBatch(const ParticleArray& sortedList, volatile void* __restrict pVertices) const = 0;

	virtual U32									GetVertexSize(const I_ParticleEffectBase* pEffect) const = 0;
	virtual Display::C_VertexDeclaration*	GetVertexDeclaration(const I_ParticleEffectBase* pEffec) const = 0;

	virtual U32									GetNumVertices(const I_ParticleEffectBase* pEffect) const = 0;
	virtual U32									GetNumPrimitives(const I_ParticleEffectBase* pEffect) const = 0;
	virtual Display::E_PrimitiveType			GetPrimitiveType(const I_ParticleEffectBase* pEffect) const = 0;*/
};














}
}

#endif
