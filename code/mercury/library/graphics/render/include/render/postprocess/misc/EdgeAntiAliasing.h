#ifndef RENDER_POSTPROCESS_EDGEANTIALIASING_H
#define RENDER_POSTPROCESS_EDGEANTIALIASING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Render/PostProcess/Interface.h>

namespace PostProcess
{

enum E_EdgeAntiAliasingShader
{
	E_EdgeAntiAliasingShader_Depth = 0,
	E_EdgeAntiAliasingShader_DepthNormal,
	E_EdgeAntiAliasingShader_Count
};

class C_EdgeAntiAliasing : public I_PostProcess
{
public:

	C_EdgeAntiAliasing(C_Manager& manager) : I_PostProcess(manager), m_pNormalTexture(0) {}
	~C_EdgeAntiAliasing() {}

	bool Create();
	void Destroy();
	bool CreateVolatile() { return true; }
	void DestroyVolatile() {}
	void Update() {}

	void Display() const;

	void SetNormalTexture(const Display::C_Texture2D* pNormalTexture) const { m_pNormalTexture = pNormalTexture; }

private:

	Display::C_PixelShader			m_PixelShader[E_EdgeAntiAliasingShader_Count];

	Display::C_PixelShaderParameter	m_PixelSizeParameter[E_EdgeAntiAliasingShader_Count];
	Display::C_PixelShaderParameter	m_SettingsParameter[E_EdgeAntiAliasingShader_Count];
	Display::C_PixelShaderParameter	m_DepthParameter[E_EdgeAntiAliasingShader_Count];

	mutable const Display::C_Texture2D*		m_pNormalTexture;

};

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_EDGEANTIALIASING_H
