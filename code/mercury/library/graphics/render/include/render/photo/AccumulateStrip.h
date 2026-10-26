#ifndef PHOTO_ACCUMULATE_STRIP_H
#define PHOTO_ACCUMULATE_STRIP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Display/Device/DeviceDefines.h>
#include <Display.h>

struct IDirect3DSurface9;

namespace Render
{
namespace Photo
{

using namespace Display;

class C_AccumulateStrip : public Display::I_Renderer
{
public:
	C_AccumulateStrip();
	~C_AccumulateStrip();

	bool OnCreate() OVERRIDE;
	void OnDestroy() OVERRIDE;
	bool OnCreateVolatile() OVERRIDE;
	void OnDestroyVolatile() OVERRIDE;

	C_Texture2D* GetTexture();
	C_Texture2D* StripTexture();
	S32 Width()  { return m_Width; }
	S32 Height() { return m_Height; }
	void Start(S32 sampleCount);
	void AddSample(const Display::C_Texture2D* texture = 0);
	bool LockBitmap(unsigned char*& retBits, S32& retPitch, S32& retWidth, S32& retHeight, S32& retFormat);
	void UnlockBitmap();

private:
	C_RenderTexture2D *m_PhotoTex, *m_StripTex, *m_ScreenStrip;
	S32 m_Width, m_Height;
	C_PixelShader m_PixelShader;
	S32 m_SampleNum, m_SampleCount;
	F32 m_Jitter[256];
	C_Viewport* m_Viewport;
	C_PixelShaderParameter m_AdjustParam;
	ID3D11Texture2D* m_PlainSurface; // [Blur1PCDX11] CPU staging copy of the photo.
	ID3D11DeviceContext* m_LockContext;
	F32 m_CurrentJitter;

	void SetRenderTarget(C_RenderTexture2D* tex, S32 x = 0, S32 y = 0, S32 w = 0, S32 h = 0);
	void ResolveRenderTarget(C_RenderTexture2D* tex);
	void ClearTextures();
	void ScreenToStrip(S32 top, S32 height);
	void PhotoToStrip(S32 top, S32 height);
	void StripToPhoto(S32 top, S32 height, const C_Texture2D* screenTexture);
	void DrawQuad(F32 srcTop, F32 srcBottom, F32 xoff, F32 yoff);
};

} // namespace Photo
} // namespace Render

#endif // PHOTO_ACCUMULATE_STRIP_H
