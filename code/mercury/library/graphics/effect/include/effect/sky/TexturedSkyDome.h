#ifndef EFFECT_SKY_TEXTUREDSKYDOME_H
#define EFFECT_SKY_TEXTUREDSKYDOME_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Display/context/SamplerState.h>
#include "Effect/Sky/SkyDomeBase.h"

namespace Display
{
class Context;
}

namespace Effect
{
namespace Sky
{

enum E_TexturedShader
{
	E_TexturedShader_Solid = 0,
	E_TexturedShader_Alpha,
	E_TexturedShader_Count
};

class C_TexturedSkyDome : public C_SkyDomeBase
{

public:

	C_TexturedSkyDome();
	~C_TexturedSkyDome();

	virtual bool OnCreate();
	virtual void OnDestroy();

	virtual void Bind(Display::C_Context *pContext) const;
	virtual void Unbind(Display::C_Context *pContext) const;

public:

	void					SetFogColour(const Maths::C_RGBA_F32& vColour)	{ m_FogColour = vColour; }
	
	void					SetRotation(const F32 fRotation)				{ m_Rotation = fRotation; }
	void					SetIntensity(const F32 fIntensity)				{ m_Intensity = fIntensity; }
	void					SetTexture(const Util::C_String& name)			{ m_TextureFileName = name; }
	void					SetAlpha(const bool bAlpha)						{ m_bAlpha = bAlpha; }

	void					SetBaseRotation(const F32 fBaseRotation)		{ m_BaseRotation = fBaseRotation; }

	void					ReloadTexture();
	Util::C_String			GetCurrentTextureFilename()						{ return m_TextureFileName; }

private:

	Display::C_Texture2D				m_Texture;
	Util::C_String						m_TextureFileName;
	Maths::C_RGBA_F32					m_FogColour;
	F32									m_Rotation;
	F32									m_BaseRotation; // typically set to sun heading
	F32									m_Intensity;
	Display::C_PixelShader				m_PixelShaders[E_TexturedShader_Count];
	Display::C_PixelShaderParameter		m_psSkySettingsParam[E_TexturedShader_Count];
	Display::C_PixelShaderParameter		m_psFogColourParam[E_TexturedShader_Count];
	Display::C_PixelShaderParameter		m_psLuminanceParam[E_TexturedShader_Count];

	bool								m_bAlpha;
	Display::C_SamplerState				m_SamplerState;

};

}; // namespace Sky
}; // namespace Effect

#endif
