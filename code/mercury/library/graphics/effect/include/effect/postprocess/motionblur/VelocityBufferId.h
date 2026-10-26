#ifndef EFFECT_POSTPROCESS_VELOCITYBUFFERID_H
#define EFFECT_POSTPROCESS_VELOCITYBUFFERID_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Render/Texture/Downsample.h>
#include <Render/Texture/GaussianBlur.h>

#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
#define USE_MATRIX_TEXTURE
#endif

namespace PostProcess
{

const U32 kMaxVelocityBufferIds = 32;
const U32 kVelocityBufferMatrixTextureCount = 8; // 2 * 4 viewports

enum E_VelocityBufferIdMethod
{
	E_VelocityBufferIdMethod_Alpha = 0,
	E_VelocityBufferIdMethod_Stencil,
	E_VelocityBufferIdMethod_Count
};

class C_Manager;

class C_VelocityBufferId : public Display::I_Renderer
{
public:

	C_VelocityBufferId(C_Manager& manager);
	~C_VelocityBufferId();

	bool	OnCreate();
	void	OnDestroy();

	void	Render() const;
	void	SetConstants() const;

	void	SetMethod(const E_VelocityBufferIdMethod eMethod) const;
	void	SetMaxVelocity(const F32 fMaxVelocity) const;
	void	SetVelocityBuffer(const Display::C_RenderTexture2D* pVelocityBuffer) const;

	void	SetMatrix(const U32 id, const Maths::HMat44& mMovement);
	void	ClearMatrixPalette();

private:
	C_Manager									m_Manager;

	Render::Texture::C_Downsample				m_Downsample;
	Render::Texture::C_GaussianBlur				m_GaussianBlur;

	Display::C_Matrix							m_MatrixPalette[kMaxVelocityBufferIds];

	Display::C_PixelShader						m_PixelShader;
	Display::C_PixelShaderParameter				m_SettingsParameter;
	Display::C_PixelShaderParameter				m_IdMaskParameter;
	Display::C_PixelShaderParameter				m_DepthScaleParameter;

#if !defined(USE_MATRIX_TEXTURE)
	Display::C_PixelShaderParameterArray		m_MatrixParameterArray;
#else
	mutable Display::C_Texture2D				m_MatrixTextures[kVelocityBufferMatrixTextureCount];
	mutable U32									m_MatrixIndex;
#endif

	mutable F32									m_fMaxVelocity;
	mutable E_VelocityBufferIdMethod			m_eMethod;
	mutable const Display::C_RenderTexture2D*	m_pVelocityBuffer;

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_VELOCITYBUFFERID_H
