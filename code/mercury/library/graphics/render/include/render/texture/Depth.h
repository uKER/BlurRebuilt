#ifndef RENDER_TEXTURE_DEPTH_H
#define RENDER_TEXTURE_DEPTH_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Render
{
namespace Texture
{

struct C_DepthShaders
{
	bool Create();
	void Destroy();

	Display::C_PixelShader					m_WorldNormalShader;
	Display::C_PixelShaderParameterArray	m_WorldNormalMatrixParameter;
	Display::C_PixelShaderParameter			m_WorldNormalScaleParameter;
	Display::C_PixelShaderParameter			m_WorldNormalSettingsParameter;
};

class C_Depth
{
public:

	C_Depth();
	~C_Depth();

	void WorldNormal(const Display::C_RenderTexture2D* pDst, const Display::C_Texture2D* pSrc, const Display::C_Float4& vDepthScale, const Display::C_Matrix& mToWorld) const;

private:

	static S32				m_RefCount;
	static C_DepthShaders	m_Shaders;

};

}; // namespace Texture
}; // namespace Render

#endif // RENDER_TEXTURE_DEPTH_H
