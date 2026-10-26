#ifndef RENDER_TEXTURE_DOWNSAMPLE_H
#define RENDER_TEXTURE_DOWNSAMPLE_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Texture
{

struct C_DownsampleShaders
{
	bool Create();
	void Destroy();

	Display::C_PixelShader	m_Downsample1TapShader;
	Display::C_PixelShader	m_Downsample4TapShader;
	Display::C_PixelShader	m_Downsample16TapShader;

	Display::C_PixelShaderParameter	m_Downsample1TapParameter;
	Display::C_PixelShaderParameter	m_Downsample4TapParameter;
	Display::C_PixelShaderParameter	m_Downsample16TapParameter;
};

class C_Downsample
{
public:

	C_Downsample();
	~C_Downsample();

	void Downsample(const Display::C_RenderTexture2D* pDst, const Display::C_Texture2D* pSrc) const;

private:

	static S32					m_RefCount;
	static C_DownsampleShaders	m_Shaders;

};

}; // namespace Texture
}; // namespace Render

#endif // RENDER_TEXTURE_DOWNSAMPLE_H
