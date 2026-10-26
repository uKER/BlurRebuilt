#ifndef UIPHOTOMODE_H
#define UIPHOTOMODE_H

#include <ui/Graphics/UIColourClasses.h>

#ifdef _MSC_VER
#pragma once
#endif

namespace UI
{

// -------------
// C_UiPhotoMode
// -------------
class C_UiPhotoMode
{
public:
	void					UpdateFilters();

	void					SetBrightness( const F32 fBrightness )			{ m_fBrightness = fBrightness; }
	void					SetContrast( const F32 fContrast )				{ m_fContrast = fContrast; }
	void					SetSaturation( const F32 fSaturation )			{ m_fSaturation = fSaturation; }
	void					SetHintColour( const C_UIColour& hintColour )	{ m_HintColour = hintColour; }
	void					SetHintAmount( const F32 fHintAmount )			{ m_fHintAmount = fHintAmount; }
	void					SetShutterAngle( const F32 fShutterAngle )		{ m_fShutterAngle = fShutterAngle; }
	void					SetIDepth( const F32 fIDepth )					{ m_fIDepth = fIDepth; }
	void					SetAperture( const F32 fAperture )				{ m_fAperture = fAperture; }

public:
	static C_UiPhotoMode&	Instance()		{ return m_sInstance; }
private:
	static C_UiPhotoMode	m_sInstance;

private:
	C_UiPhotoMode();

private:
	F32						m_fBrightness;
	F32						m_fContrast;
	F32						m_fSaturation;
	C_UIColour				m_HintColour;
	F32						m_fHintAmount;
	F32						m_fShutterAngle;
	F32						m_fIDepth;
	F32						m_fAperture;

	F32 m_PrevIdepth, m_PrevAperture, m_PrevShutterAngle;
};

}; // namespace UI

#endif /* UIPHOTOMODE_H */
