#ifndef RENDER_DYNAMICEFFECT_TRACK_EMITTER_H
#define RENDER_DYNAMICEFFECT_TRACK_EMITTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Handle.h"

namespace DynamicEffect
{
namespace Track
{

class C_Particle;
class C_TrackRenderer;
class C_Track;
class C_Material;

class C_Emitter : public Ut::ListElementDouble<C_Emitter>
{
public:
	C_Emitter(C_TrackRenderer* pTrackRenderer);
	
	void	SetParameters(bool bActive, const Maths::HF32& fIntensity, const Maths::HVec3& vPosition, const Maths::HVec3& vDirection, const Maths::HF32& fWidth, const Maths::HF32& fStep, const U32 iFrame);
	void	SetColour(const Maths::C_RGBA_F32& vColour);
	void	SetLighting(const Maths::C_RGBA_F32& vLighting);
	void	SetVisibilityFlags(U32 visibilityFlags) {m_VisibilityFlags = visibilityFlags;}
	void	Free();
	void	Destroy();

	Maths::HVec3	GetPosition() const { return m_vPosition; }
	Maths::HVec3	GetDirection() const { return m_vDirection; }

	U32		GetSalt() const { return m_iSalt; }
	
	friend class C_TrackRenderer;

private:
	void							Update();
	void							CreateParticle(C_Particle* pParticle, const Maths::HVec3& vDirection, const Maths::C_RGBA_F32& vColour);
	void							SetMaterial(const C_Material* pMaterial);
	// variables set by user
	bool							m_bActive;
	Maths::HF32						m_fIntensity;
	Maths::HVec3					m_vPosition;
	Maths::HVec3					m_vDirection;
	Maths::HF32						m_fWidth;
	Maths::HF32						m_fStep;
	U32								m_iFrame;
	
	// internal variables
	U32								m_iSalt;
	C_Track*						m_pTrack;
	const C_Material*				m_pMaterial;
	C_TrackRenderer*				m_pTrackRenderer;
	bool							m_bEmitting;
	Maths::HVec3					m_vPosition1;
	Maths::HVec3					m_vPosition2;
	Maths::HVec3					m_vLastPosition;
	Maths::HVec3					m_vLastDirection;
	Maths::C_RGBA_F32				m_vColour;
	Maths::C_RGBA_F32				m_vLighting;
	F32								m_fV;
	U32								m_VisibilityFlags;
};

INLINE void C_Emitter::SetParameters(bool bActive, const Maths::HF32& fIntensity, const Maths::HVec3& vPosition, const Maths::HVec3& vDirection, const Maths::HF32& fWidth, const Maths::HF32& fStep, const U32 iFrame)
{
	m_bActive = bActive;
	m_fIntensity = fIntensity;
	m_vPosition = vPosition;
	m_vDirection = vDirection;
	m_fWidth = fWidth;
	m_fStep = fStep;
	m_iFrame = iFrame;
}

INLINE void C_Emitter::SetColour(const Maths::C_RGBA_F32& vColour)
{
	m_vColour = vColour;
}

INLINE void C_Emitter::SetLighting(const Maths::C_RGBA_F32& vLighting)
{
	m_vLighting = vLighting;
}

typedef DynamicEffect::C_Handle<C_Emitter>			C_EmitterHandle;

} // Track
} // DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACK_EMITTER_H


