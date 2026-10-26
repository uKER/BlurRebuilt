#ifndef RENDER_DYNAMICEFFECT_STRIP_EMITTER_H
#define RENDER_DYNAMICEFFECT_STRIP_EMITTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Emitter.h"

namespace DynamicEffect
{
class C_RenderNode;

namespace Strip
{

class C_Manager;
class I_StripEffectBase;

enum E_EmitterType
{
	E_EmitterType_Trail,
	E_EmitterType_Instant
};

struct EmitterDefinition
{
	I_StripEffectBase*			pEffect;
	E_EmitterType				eType;
	F32							fWidth;
	F32							fWidthVariation;
	F32							fElementLife;
	F32							fElementLifeVariation;
	F32							fSegmentLength;
	F32							fTextureRepeatLength;
	
	F32							fStripLife;
	F32							fStripLifeVariation;

	bool						bUpdateStrip;			// the strip as a whole requires an update loop
	bool						bUpdateElements;		// the individual elements of the strip require updates

	MathsStorage::Float4A16		vSpawnOffset;

	MathsStorage::Float4A16		vColour1;	// primary colour
	MathsStorage::Float4A16		vColour2;	// secondary colour

	friend class C_Emitter;

	void						Validate();
private:
	F32							fTextureRepeatDelta;
};


class C_Emitter : public Ut::ListElementDouble<C_Emitter>, public C_EmitterBase<C_RenderNode>
{
public:
	C_Emitter() { assert(0); } // should never happen, needed for UtList though
	C_Emitter(C_Manager* pManager);
	void						Destroy() {};
	virtual ~C_Emitter();

	void						Free();
	void						Reset();
	
	void						SetDefinition(const EmitterDefinition* pDefinition) { m_pDefinition = pDefinition; }
	const EmitterDefinition*	GetDefinition() const { return m_pDefinition; }

	//BlockBase*					GetBlock() const { return m_pStrip; }

	U32							GetSpawnSegments() const { return m_nSegments; }

	void						SetPositionPrevious(const Maths::HVec3& vPosition);
	C_Node						GetWorldTransform() const;

	Maths::HVec3				GetSpawnPosition() const;
	F32							GetSpawnTexCoord() const;

	friend class C_Manager;

protected:
	void						Update(const F32 fFrameTime, const F32 fFrameRate);

	const EmitterDefinition*	m_pDefinition;
	C_Manager*					m_pManager;
	
	//BlockBase*					m_pStrip;

	U32							m_nSegments;

	MathsStorage::Float4A16		m_vPosition;
	MathsStorage::Float4A16		m_vPositionPrevious;
	MathsStorage::Float4A16		m_vPositionPreviousEmit;


	static Maths::HVec3			m_vStart;
	static Maths::HVec3			m_vEnd;
	static Maths::HF32			m_fInterpolate;
	static Maths::HF32			m_fInterpolateDelta;
	
	mutable F32					m_fTexCoord;
};

FORCE_INLINE void C_Emitter::SetPositionPrevious(const Maths::HVec3& vPosition)
{
	Maths::Store(vPosition, &m_vPosition);
	Maths::Store(vPosition, &m_vPositionPrevious);
	Maths::Store(vPosition, &m_vPositionPreviousEmit);
}

}
}

#endif // RENDER_DYNAMICEFFECT_PARTICLE_EMITTER_H
