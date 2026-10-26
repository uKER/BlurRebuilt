#ifndef EFFECT_SKY_ATMOSPHERICPARAMETERS_H
#define EFFECT_SKY_ATMOSPHERICPARAMETERS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{
namespace Sky
{

	struct C_AtmosphericParameters
{
	Display::C_Float4	m_vSunDirection;
	Display::C_Float4	m_vRayleighCoeffs;
	Display::C_Float4	m_vMieCoeffs;
	Display::C_Float4	m_vRayleighSunIntensity;
	Display::C_Float4	m_vMieSunIntensity;
	Display::C_Float4	m_vMieG;
	F32					m_fInnerRadius;
	F32					m_fOuterRadius;
	F32					m_fScale;
	F32					m_fFieldOfView;
};

struct C_AtmosphericTask
{
	C_AtmosphericParameters	m_Parameters;
	U32*					m_pLinePosition;
	U32						m_LinesToUpdate;
	U8*						m_pMieBuffer;
	U8*						m_pRayleighBuffer;
	U32						m_iMiePitch;
	U32						m_iRayleighPitch;
	Display::C_Float4*		m_pMieColour;
	Display::C_Float4*		m_pRayleighColour;
	U32						m_iBufferWidth;
	U32						m_iBufferHeight;
	U32						m_iSamples;
};

}; // namespace Sky
}; // namespace Effect

#endif
