#ifndef DISPLAY_RENDER_RENDER_H
#define DISPLAY_RENDER_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Shader/PixelShader.h"
#include "Display/Texture/Texture.h"

namespace Display
{

namespace Quad
{
	void SetQuadSettings(MathsStorage::UInt2 segments = MathsStorage::UInt2(1, 1), F32 halfPixelScaleX = 1.0f, F32 halfPixelScaleY = 1.0f, bool flipHorizontal = false, bool flipVertical = false);
	void SetQuadBias(F32 biasX = 0.0f, F32 biasY = 0.0f);

	void TextureAddress(const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW);
	void TextureFilter(const E_TextureFilter filterMag, const E_TextureFilter filterMin, const E_TextureFilter filterMip);
	void TextureFilter(const E_TextureFilter filter);
	
	void DrawVertexColour(const Maths::RGBA colour);
	void Draw();

	void DrawFullScreen();
	void DrawFullScreen(const C_Texture* pTexture);
	void DrawFullScreen(const C_Texture* pTexture, const C_PixelShader* pPixelShader);

	void DrawFullScreenDepth();
	void DrawFullScreenDepth(const C_Texture* pColour, const C_Texture* pDepth);
	void DrawFullScreenDepth(const C_Texture* pColour, const C_Texture* pDepth, const C_PixelShader* pPixelShader);

	void DrawFullScreenColour(const Maths::C_RGBA_F32& colour);

	void DrawPartScreen(F32 x1, F32 y1, F32 x2, F32 y2);
	void DrawPartScreen(F32 x1, F32 y1, F32 x2, F32 y2, const C_Texture* pTexture);
	void DrawPartScreen(F32 x1, F32 y1, F32 x2, F32 y2, const C_Texture* pTexture, const C_PixelShader* pPixelShader);

	#ifdef PLATFORM_PS3
	void DrawFullscreenQuad(const F32 du, const F32 dv);
	#endif
	
} // namespace Quad
} // namespace Display

#endif // DISPLAY_RENDER_RENDER_H
