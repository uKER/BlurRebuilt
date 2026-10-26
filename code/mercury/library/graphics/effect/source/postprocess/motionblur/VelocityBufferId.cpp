#include "stdafx.h"
#include "Effect/PostProcess/MotionBlur/VelocityBufferId.h"

#include <Render/Shaders/Constants.h>
#include <Render/State/RenderState.h>
#include <Render/Utils/Globals.h>

EXTERN_SHADER(psVelocityId);

namespace PostProcess
{

C_VelocityBufferId::C_VelocityBufferId( C_Manager& manager ) : m_Manager( manager ), Display::I_Renderer("VelocityBufferId")
{
	m_fMaxVelocity = 1.0f;
	m_eMethod = E_VelocityBufferIdMethod_Stencil;
	m_pVelocityBuffer = 0;
}

C_VelocityBufferId::~C_VelocityBufferId()
{
	DestroyRenderer();
}

bool C_VelocityBufferId::OnCreate()
{
	if (!Display::CreatePixelShaderFromFunc("Shaders/psVelocityId", 0, &m_PixelShader, REF_SHADER(psVelocityId)))
		return false;

	m_SettingsParameter		= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_0, "g_vSettings");
	m_IdMaskParameter		= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_1, "g_vIdMask");
	m_DepthScaleParameter	= m_PixelShader.GetParameter(CP_RENDER_POSTPROCESS_PARAMETERS_2, "g_vDepthScale");

#if !defined(USE_MATRIX_TEXTURE)
	m_MatrixParameterArray	= m_PixelShader.GetParameterArray(CP_RENDER_POSTPROCESS_PARAMETERS_4, "g_mPalette", 4 * kMaxVelocityBufferIds);
#else
	#if PLATFORM_PC
	const U32 flags = Display::E_Usage_Managed;
	#else
	const U32 flags = Display::E_Usage_Video | Display::E_Format_Linear;
	#endif
	for (U32 i = 0; i < kVelocityBufferMatrixTextureCount; ++i)
	{
		if (!Display::Device()->CreateTexture2D(4 * kMaxVelocityBufferIds, 1, 1, flags, Display::E_Format_A32B32G32R32F, &m_MatrixTextures[i]))
			return false;

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(&m_MatrixTextures[i], "_MotionBlurMatrixTexture");
		#endif
	}
	m_MatrixIndex = 0;
#endif

	return true;
}

void C_VelocityBufferId::OnDestroy()
{
	Display::Device()->Destroy(&m_PixelShader);

#if defined(USE_MATRIX_TEXTURE)
	for (U32 i = 0; i < kVelocityBufferMatrixTextureCount; ++i)
	{
		Display::Device()->Destroy(&m_MatrixTextures[i]);
	}
#endif
}

void C_VelocityBufferId::Render() const
{
	if (!m_Manager.Enabled())
		return;

	PERF_EVENT("VelocityBufferId");

	const Display::C_Surface* pColourSurface = Display::Device()->GetRenderTarget().GetColourSurface();
	const Display::C_Surface* pDepthStencilSurface = Display::Device()->GetRenderTarget().GetDepthStencilSurface();
	
	#ifndef PLATFORM_PS3
	Display::Device()->SetColourDepthStencilSurface(&m_pVelocityBuffer->GetSurface(), 0);
	#else
	Display::Device()->SetColourSurface(&m_pVelocityBuffer->GetSurface());
	Display::Device()->SetSurface();
	#endif

	Display::Device()->SetTexture(0, Render::GetGlobalDepthStencilBuffer());
	Display::Device()->SetTextureAddress(0, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(0, Display::E_TextureFilter_Point);

	if (E_VelocityBufferIdMethod_Alpha == m_eMethod)
	{
		Display::Device()->SetTexture(1, m_Manager.GetChainTexture());
		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	}
	else if (E_VelocityBufferIdMethod_Stencil == m_eMethod)
	{
	#if PLATFORM_PS3
	
		CellGcmTexture* pSrcGcmTexture = (CellGcmTexture*) Render::GetGlobalDepthStencilBuffer()->GetGPUTexture();

		CellGcmTexture gcmTexture;
		gcmTexture.format		= CELL_GCM_TEXTURE_A8R8G8B8 | CELL_GCM_TEXTURE_LN | CELL_GCM_TEXTURE_NR;
		gcmTexture.mipmap		= 1;
		gcmTexture.dimension	= CELL_GCM_TEXTURE_DIMENSION_2;
		gcmTexture.cubemap		= CELL_GCM_FALSE;
		gcmTexture.remap		=	CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
									CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
									CELL_GCM_TEXTURE_REMAP_FROM_B << 6 |
									CELL_GCM_TEXTURE_REMAP_FROM_G << 4 |
									CELL_GCM_TEXTURE_REMAP_FROM_R << 2 |
									CELL_GCM_TEXTURE_REMAP_FROM_A;
		gcmTexture.width		= pSrcGcmTexture->width;
		gcmTexture.height		= pSrcGcmTexture->height;
		gcmTexture.depth		= pSrcGcmTexture->depth;
		gcmTexture.location		= pSrcGcmTexture->location;
		gcmTexture.pitch		= pSrcGcmTexture->pitch;
		gcmTexture.offset		= pSrcGcmTexture->offset;
		
		Display::Device()->SetTextureUncached(1, &gcmTexture);

	#endif

		Display::Device()->SetTextureAddress(1, Display::E_TextureAddress_Clamp);
		Display::Device()->SetTextureFilter(1, Display::E_TextureFilter_Point);
	}

	SetConstants();

	Display::Device()->SetVertexShader(&Display::VertexShader_XY::m_VertexShader);
	Display::Device()->SetPixelShader(&m_PixelShader);

	Display::Quad::DrawFullScreen();

	m_pVelocityBuffer->Resolve();

	for (U32 i = 0; i <= 2; ++i)
	{
		Display::Device()->SetTexture(i, 0);
		Display::Device()->SetTextureAddress(i, Display::E_TextureAddress_Default);
		Display::Device()->SetTextureFilter(i, Display::E_TextureFilter_Default);
	}


	#ifndef PLATFORM_PS3
	Display::Device()->SetColourDepthStencilSurface(pColourSurface, pDepthStencilSurface);
	#else
	Display::Device()->SetColourSurface(pColourSurface);
	Display::Device()->SetDepthStencilSurface(pDepthStencilSurface);
	Display::Device()->SetSurface();
	#endif
}

void C_VelocityBufferId::SetConstants() const
{
	Display::C_Float4 vSettings(m_fMaxVelocity, 0.25f / (F32) kMaxVelocityBufferIds, 0.0f, 0.0f);
	Display::Device()->SetPixelShaderParameterF(m_SettingsParameter, (F32*)&vSettings);

	Display::C_Float4 vIdMask;
	if (E_VelocityBufferIdMethod_Alpha == m_eMethod)
	{
		vIdMask = Display::C_Float4(0.0f, 0.0f, 0.0f, 1.0f);
	}
	else if (E_VelocityBufferIdMethod_Stencil == m_eMethod)
	{
		vIdMask = Display::C_Float4(0.0f, 0.0f, 1.0f, 0.0f);
	}
	Display::Device()->SetPixelShaderParameterF(m_IdMaskParameter, (F32*)&vIdMask);

	Display::C_Float4 vDepthScale = Render::C_RenderState::Instance().GetDepthScaleShaderParameter();
	Display::Device()->SetPixelShaderParameterF(m_DepthScaleParameter, (F32*)&vDepthScale);

	const Display::C_View* pView = Display::Device()->GetView();
	const Display::C_ViewSettings3D& viewSettings3D = pView->GetViewSettings3D();
	const Display::C_BackBuffer& backBuffer = Display::Device()->GetBackBuffer();

	// compute motion blur matrices

	// first matrix transforms from uv * depth space to projected space
	Maths::HMat44 mTexCoord0;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
			
		mTemp._11 = 2.0f;
		mTemp._31 = -1.0f;
		mTemp._22 = -2.0f;
		mTemp._32 = 1.0f;
			
		mTexCoord0 = Maths::Load<Maths::HMat44>(&mTemp);
	}

	// second matrix takes into account field of view and aspect ratio to get position in view space
	Maths::HMat44 mFovAspect;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);

		Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrix()));

		mTemp._11 = 1.f / viewSettings3D.GetProjMatrix()._11;
		mTemp._22 = 1.f / viewSettings3D.GetProjMatrix()._22;

		// if the viewport has been flipped we need to negate these
		if(m_Manager.GetView().bFlip)
		{
			mTemp._11 *= -1.0f;
			mTemp._22 *= -1.0f;
		}

		mFovAspect = Maths::Load<Maths::HMat44>(&mTemp);
	}

	// third matrix is just the projection matrix
	Maths::HMat44 mProjection(Maths::Load<Maths::HMat44>(&viewSettings3D.GetProjMatrixPrev()));

	// fourth matrix is projected space to texture * depth space transform (sorta - depth in W)
	Maths::HMat44 mTexCoord1;
	{
		Display::C_Matrix mTemp;
		Display::MatrixSetIdentity(&mTemp);
		mTemp._11 = 0.5f;
		mTemp._41 = 0.5f;
		mTemp._22 = -0.5f;
		mTemp._42 = 0.5f;

		mTexCoord1 = Maths::Load<Maths::HMat44>(&mTemp);
	}

	// set up matrix palette, the first matrix is always the identity for no motion blur
	Display::C_Matrix mPalette[kMaxVelocityBufferIds];
	for (U32 i = 0; i < kMaxVelocityBufferIds; ++i)
	{
		// calculate movement matrix
		Maths::HMat44 mWorldMove(Maths::Load<Maths::HMat44>(&m_MatrixPalette[i]));
		Maths::HMat44 mViewInverse(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrix()));
		Maths::HMat44 mViewPrev(Maths::Load<Maths::HMat44>(&viewSettings3D.GetViewMatrixPrev()));
		Maths::HMat44 mMove(mViewInverse.Inverse() * mWorldMove * mViewPrev);
		if (i == kMaxVelocityBufferIds - 1)
			mMove.SetIdentity();

		Maths::HMat44 mFinal;
		mFinal = mFinal * mTexCoord0;
		mFinal = mFinal * mFovAspect;
		mFinal = mFinal * mMove;
		mFinal = mFinal * mProjection;
		mFinal = mFinal * mTexCoord1;
		Maths::Store(mFinal, &mPalette[i]);
	}

#if !defined(USE_MATRIX_TEXTURE)
	Display::Device()->SetPixelShaderParameterArrayF(m_MatrixParameterArray, (F32*)mPalette);
#else
	Display::C_LockedRect lockedRect = m_MatrixTextures[m_MatrixIndex].Lock(0);
	if (lockedRect.pBits)
	{
		memcpy(lockedRect.pBits, mPalette, sizeof(F32) * 16 * kMaxVelocityBufferIds);
	}
	m_MatrixTextures[m_MatrixIndex].Unlock(0);

	Display::Device()->SetTexture(2, &m_MatrixTextures[m_MatrixIndex]);
	Display::Device()->SetTextureAddress(2, Display::E_TextureAddress_Clamp);
	Display::Device()->SetTextureFilter(2, Display::E_TextureFilter_Point);

	m_MatrixIndex = (m_MatrixIndex + 1) % kVelocityBufferMatrixTextureCount;
#endif
}

void C_VelocityBufferId::SetMethod(const E_VelocityBufferIdMethod eMethod) const
{
	m_eMethod = eMethod;
}

void C_VelocityBufferId::SetMaxVelocity(const F32 fMaxVelocity) const
{
	m_fMaxVelocity = fMaxVelocity;
}

void C_VelocityBufferId::SetVelocityBuffer(const Display::C_RenderTexture2D* pVelocityBuffer) const
{
	m_pVelocityBuffer = pVelocityBuffer;
}

void C_VelocityBufferId::SetMatrix(const U32 id, const Maths::HMat44& mMovement)
{
	assert(0 <= id && id < kMaxVelocityBufferIds);
	Maths::Store(mMovement, &m_MatrixPalette[id]);
}

void C_VelocityBufferId::ClearMatrixPalette()
{
	Display::C_Matrix mIdentity;
	Display::MatrixSetIdentity(&mIdentity);

	for (U32 i = 0; i < kMaxVelocityBufferIds; ++i)
	{
		m_MatrixPalette[i] = mIdentity;
	}
}

}; // namespace PostProcess
