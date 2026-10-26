#ifndef DISPLAY_RENDER_TEXTURE_RENDER_H
#define DISPLAY_RENDER_TEXTURE_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/Device.h"

namespace Display
{

enum E_TextureRenderPixelShader
{
	E_TextureRenderPixelShader_RGB = 0,
	E_TextureRenderPixelShader_A,
	E_TextureRenderPixelShader_Count
};

class C_TextureRender : public I_Renderer
{
public:

	C_TextureRender();
	~C_TextureRender();

	bool IsCreated() const;

	bool OnCreate();
	void OnDestroy();

	void Draw();

	Util::C_String GetDetails(const S32 centerTexture, const S32 totalTextures, Display2::C_Texture* pTexture, Display2::C_ResourceInfo* pResourceInfo) const;
	Util::C_String GetTypeName(const Display2::E_ResourceType type) const;
	Util::C_String GetFormatName(const Display2::E_TextureFormat format) const;

public:

	C_PixelShader	m_PixelShader[E_TextureRenderPixelShader_Count];

private:

	bool			m_Created;
};

extern C_TextureRender* g_pTextureRender;
void TextureRender(C_TextureRender* pTextureRender);
C_TextureRender* TextureRender();
bool TextureRenderExists();

}; // namespace Display

#include "Display/Render/TextureRender.inl"

#endif // DISPLAY_RENDER_TEXTURE_RENDER_H
