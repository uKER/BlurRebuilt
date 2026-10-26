#ifndef EFFECT_SKY_HDRSKYDOME_H
#define EFFECT_SKY_HDRSKYDOME_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Effect/Sky/SkyDomeBase.h"

namespace Effect
{
namespace Sky
{

class C_HDRSkyDome : public C_SkyDomeBase
{

public:

	C_HDRSkyDome();
	~C_HDRSkyDome();

	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void Bind(Display::C_Context *pContext) const;
	virtual void Unbind(Display::C_Context *pContext) const;

public:

	void					SetFogColour(const Maths::C_RGBA_F32& vColour)	{ m_FogColour = vColour; }
	
	void					SetRotation(const F32 fRotation)				{ m_Rotation = fRotation; }
	void					SetIntensity(const F32 fIntensity)				{ m_Intensity = fIntensity; }
	void					SetTextureLDR(const Util::C_String& name)		{ m_TextureLDRFileName = name; }
	void					SetTextureHDR(const Util::C_String& name)		{ m_TextureHDRFileName = name; }

	void					SetBaseRotation(const F32 fBaseRotation)		{ m_BaseRotation = fBaseRotation; }

	void					ReloadTextures();

private:

	Display::C_Texture2D			m_TextureLDR;
	Display::C_Texture2D			m_TextureHDR;
	Util::C_String					m_TextureLDRFileName;
	Util::C_String					m_TextureHDRFileName;

	Maths::C_RGBA_F32				m_FogColour;
	F32								m_BaseRotation; // typically set to sun heading
	F32								m_Rotation;
	F32								m_Intensity;

	Display::C_PixelShaderParameter	m_SkySettingsParam;
	Display::C_PixelShaderParameter	m_FogColourParam;
	Display::C_PixelShaderParameter	m_FogSettingsParam;
	Display::C_PixelShaderParameter	m_AdjustParam;
	Display::C_PixelShaderParameter	m_PixelSizeParam;
	Display::C_PixelShaderParameter	m_LuminanceParam;

};

}; // namespace Sky
}; // namespace Effect

#endif // EFFECT_SKY_HDRSKYDOME_H
