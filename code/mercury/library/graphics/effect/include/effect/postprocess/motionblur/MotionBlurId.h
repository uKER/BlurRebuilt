#ifndef EFFECT_POSTPROCESS_MOTIONBLURID_H
#define EFFECT_POSTPROCESS_MOTIONBLURID_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Render/PostProcess/Interface.h>
#include <Render/PostProcess/Manager.h>
#include <Render/Texture/Downsample.h>
#include <Render/Texture/GaussianBlur.h>

#include <Effect/PostProcess/MotionBlur/MotionBlurVelocity.h>
#include <Effect/PostProcess/MotionBlur/VelocityBufferId.h>

namespace PostProcess
{

class C_MotionBlurId : public I_PostProcess
{
public:

	C_MotionBlurId( C_Manager& manager );
	~C_MotionBlurId();

	bool	Create();
	bool	CreateVolatile();
	void	Destroy();
	void	DestroyVolatile();

	void	Update();

	void	Display() const;
	void	SetConstants() const;

	void	SetMethod(const E_VelocityBufferIdMethod eMethod);
	void	SetMatrix(const U32 id, const Maths::HMat44& mMovement);
	void	ClearMatrixPalette();

private:

	PostProcess::C_MotionBlurVelocity		m_MotionBlurVelocity;
	PostProcess::C_VelocityBufferId			m_VelocityBufferId;

	Render::Texture::C_Downsample			m_Downsample;
	Render::Texture::C_GaussianBlur			m_GaussianBlur;

	Display::C_RenderTexture2D				m_VelocityRenderTexture;
	Display::C_RenderTexture2D				m_BlurRenderTexture;
#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
	Display::C_RenderTexture2D				m_BlurXRenderTexture;
#endif

};

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_MOTIONBLURID_H
