#ifndef EFFECT_SKY_LUMINANCESKYDOME_H
#define EFFECT_SKY_LUMINANCESKYDOME_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Sky/SkyDomeBase.h"

namespace Effect
{
namespace Sky
{

class C_LuminanceSkyDome : public C_SkyDomeBase
{

public:

	C_LuminanceSkyDome();
	~C_LuminanceSkyDome();

	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void Bind(Display::C_Context *pContext) const;
	virtual void Unbind(Display::C_Context *pContext) const;

public:

	const F32				GetRotation() const		{ return m_Rotation; }
	const F32				GetIntensity() const	{ return m_Intensity; }
	const Maths::C_RGBA_F32	GetFogColour() const	{ return m_FogColour; }

	void					SetRotation(const F32 fRotation)				{ m_Rotation = fRotation; }
	void					SetIntensity(const F32 fIntensity)				{ m_Intensity = fIntensity; }
	void					SetFogColour(const Maths::C_RGBA_F32& vColour)	{ m_FogColour = vColour; }
	void					SetSunColour(const Maths::C_RGBA_F32& vColour)	{ m_SunColour = vColour; }
	void					SetTexture(const Util::C_String& name)			{ m_TextureFileName = name; }

	void					SetBaseRotation(const F32 fBaseRotation)		{ m_BaseRotation = fBaseRotation; }

private:

	Display::C_Texture2D			m_Texture;
	Util::C_String					m_TextureFileName;

	Maths::C_RGBA_F32				m_FogColour;
	Maths::C_RGBA_F32				m_SunColour;
	F32								m_BaseRotation; // typically set to sun heading
	F32								m_Rotation;
	F32								m_Intensity;

	Display::C_PixelShaderParameter	m_SkySettingsParam;
	Display::C_PixelShaderParameter	m_FogColourParam;
	Display::C_PixelShaderParameter	m_FogSettingsParam;
	Display::C_PixelShaderParameter	m_AdjustParam;
	Display::C_PixelShaderParameter	m_SunColourParam;
	Display::C_PixelShaderParameter	m_LuminanceParam;

};

}; // namespace Sky
}; // namespace Effect

#endif
