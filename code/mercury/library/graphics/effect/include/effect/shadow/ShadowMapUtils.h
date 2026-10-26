#ifndef EFFECT_SHADOW_MAP_UTILS_H
#define EFFECT_SHADOW_MAP_UTILS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace Effect
{

struct C_ExponentialFilterShaders
{
	bool Create();
	void Destroy();

	Display::C_PixelShader	m_ExponentialFilterX5Shader;
	Display::C_PixelShader	m_ExponentialFilterX7Shader;
	Display::C_PixelShader	m_ExponentialFilterY5Shader;
	Display::C_PixelShader	m_ExponentialFilterY7Shader;

	Display::C_PixelShaderParameter	m_ExponentialFilterX5Parameter;
	Display::C_PixelShaderParameter	m_ExponentialFilterX7Parameter;
	Display::C_PixelShaderParameter	m_ExponentialFilterY5Parameter;
	Display::C_PixelShaderParameter	m_ExponentialFilterY7Parameter;
};

class C_ExponentialFilter
{
public:

	C_ExponentialFilter();
	~C_ExponentialFilter();

	void	ExponentialFilter5(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const;
	void	ExponentialFilter7(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const;

private:

	static S32							m_RefCount;
	static C_ExponentialFilterShaders	m_Shaders;

};

struct C_VarianceFilterShaders
{
	bool Create();
	void Destroy();

	Display::C_PixelShader	m_VarianceFilterX5Shader;
	Display::C_PixelShader	m_VarianceFilterX7Shader;
	Display::C_PixelShader	m_VarianceFilterY5Shader;
	Display::C_PixelShader	m_VarianceFilterY7Shader;
	Display::C_PixelShader	m_VarianceResolveShader;

	Display::C_PixelShaderParameter	m_VarianceFilterX5Parameter;
	Display::C_PixelShaderParameter	m_VarianceFilterX7Parameter;
	Display::C_PixelShaderParameter	m_VarianceFilterY5Parameter;
	Display::C_PixelShaderParameter	m_VarianceFilterY7Parameter;
};

class C_VarianceFilter
{
public:

	C_VarianceFilter();
	~C_VarianceFilter();

	void	VarianceFilter5(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const;
	void	VarianceFilter7(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pInt, const Display::C_RenderTexture2D* pSrc) const;
	void	VarianceResolve(const Display::C_RenderTexture2D* pDst, const Display::C_RenderTexture2D* pSrc) const;

private:

	static S32						m_RefCount;
	static C_VarianceFilterShaders	m_Shaders;

};

}; // namespace Effect

#endif // EFFECT_SHADOW_MAP_UTILS_H
