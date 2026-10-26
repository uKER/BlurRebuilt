#ifndef EFFECT_SKY_PREETHAMSKYDOME_H
#define EFFECT_SKY_PREETHAMSKYDOME_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Effect/Sky/SkyDomeBase.h"

namespace Effect
{
namespace Sky
{

class C_PreethamSkyDome : public C_SkyDomeBase
{
public:

	C_PreethamSkyDome();
	~C_PreethamSkyDome();

	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void Bind(Display::C_Context *pContext) const;
	virtual void Unbind(Display::C_Context *pContext) const;

public:

	void SetSunHeading(const F32 fSunHeading)		{ m_SunHeading = fSunHeading; }
	void SetSunElevation(const F32 fSunElevation)	{ m_SunElevation = fSunElevation; }

private:

	Display::C_PixelShaderParameter	m_SettingsParam;
	Display::C_PixelShaderParameter	m_LuminanceParam;

private:

	F32								m_SunHeading;
	F32								m_SunElevation;

};

} // namespace Sky
} // namespace Effect

#endif // EFFECT_SKY_PREETHAMSKYDOME_H
