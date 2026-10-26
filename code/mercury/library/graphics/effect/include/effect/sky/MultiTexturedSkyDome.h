#ifndef EFFECT_SKY_MULTITEXTUREDSKYDOME_H
#define EFFECT_SKY_MULTITEXTUREDSKYDOME_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Sky/SkyDomeBase.h"

namespace Effect
{
namespace Sky
{

class C_MultiTexturedSkyDome : public C_SkyDomeBase
{

public:

	C_MultiTexturedSkyDome();
	~C_MultiTexturedSkyDome();

	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void Bind(Display::C_Context *pContext) const;
	virtual void Unbind(Display::C_Context *pContext) const;

public:

	void					SetFogColour(const Maths::C_RGBA_F32& vColour)	{ m_FogColour = vColour; }
	
	void					SetRotation(const F32 fRotation)				{ m_BgRotation = fRotation; m_FgRotation = fRotation; }
	void					SetIntensity(const F32 fIntensity)				{ m_BgIntensity = fIntensity; m_FgIntensity = fIntensity; }
	void					SetTexture(const Util::C_String& name)			{ m_BgTextureFileName = name; m_FgTextureFileName = name; }

	void					SetBgRotation(const F32 fRotation)				{ m_BgRotation = fRotation; }
	void					SetBgIntensity(const F32 fIntensity)			{ m_BgIntensity = fIntensity; }
	void					SetBgTexture(const Util::C_String& name)		{ m_BgTextureFileName = name; }

	void					SetFgRotation(const F32 fRotation)				{ m_FgRotation = fRotation; }
	void					SetFgIntensity(const F32 fIntensity)			{ m_FgIntensity = fIntensity; }
	void					SetFgTexture(const Util::C_String& name)		{ m_FgTextureFileName = name; }
	
	void					SetBaseRotation(const F32 fBaseRotation)		{ m_BaseRotation = fBaseRotation; }

	void					ReloadTextures();

private:

	Display::C_Texture2D			m_BgTexture;
	Util::C_String					m_BgTextureFileName;
	Display::C_Texture2D			m_FgTexture;
	Util::C_String					m_FgTextureFileName;

	Maths::C_RGBA_F32				m_FogColour;
	F32								m_BaseRotation; // typically set to sun heading
	F32								m_BgRotation;
	F32								m_FgRotation;
	F32								m_BgIntensity;
	F32								m_FgIntensity;

	Display::C_PixelShaderParameter	m_SkySettingsParam;
	Display::C_PixelShaderParameter	m_FogColourParam;
	Display::C_PixelShaderParameter	m_FogSettingsParam;
	Display::C_PixelShaderParameter	m_AdjustParam;
	Display::C_PixelShaderParameter	m_BlackSettingsParam;
	Display::C_PixelShaderParameter	m_LuminanceParam;

};

}; // namespace Sky
}; // namespace Effect

#endif
