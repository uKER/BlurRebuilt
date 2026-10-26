#ifndef DISPLAY_DEVICE_DEVICE_BASE_INL
#define DISPLAY_DEVICE_DEVICE_BASE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE void C_SamplerBase::InvalidateState()
{
	#ifdef PLATFORM_PS3
	m_AddressU			= m_AddressUDefault;
	m_AddressV			= m_AddressVDefault;
	m_AddressW			= m_AddressWDefault;
	m_FilterMag			= m_FilterMagDefault;
	m_FilterMin			= m_FilterMinDefault;
	m_FilterMip			= m_FilterMipDefault;
	m_Gamma				= m_GammaDefault;
	#else
	m_AddressU			= E_TextureAddress_Unknown;
	m_AddressV			= E_TextureAddress_Unknown;
	m_AddressW			= E_TextureAddress_Unknown;
	m_FilterMag			= E_TextureFilter_Unknown;
	m_FilterMin			= E_TextureFilter_Unknown;
	m_FilterMip			= E_TextureFilter_Unknown;
	m_Gamma				= E_Bool_Unknown;
	#endif
}

INLINE const bool C_DeviceBase::IsCreated() const
{
	return m_Created;
}

INLINE const bool C_DeviceBase::InScene() const
{
	return m_InScene;
}

INLINE const bool C_DeviceBase::InParallel() const
{
	return m_InParallel;
}

INLINE F32 C_DeviceBase::GetCpuTime() const
{
	return m_CpuTime;
}

INLINE F32 C_DeviceBase::GetGpuTime() const
{
	return m_GpuTime;
}

INLINE const E_ShaderProfile C_DeviceBase::GetShaderProfile() const
{
	return m_ShaderProfile;
}

INLINE const E_VertexShaderProfile C_DeviceBase::GetVertexShaderProfile() const
{
	return m_VertexShaderProfile;
}

INLINE const E_PixelShaderProfile C_DeviceBase::GetPixelShaderProfile() const
{
	return m_PixelShaderProfile;
}

INLINE const bool C_DeviceBase::IsHiDef() const
{
	return m_HiDef;
}

INLINE const bool C_DeviceBase::GetWideScreen() const
{
	return m_WideScreen;
}

INLINE void C_DeviceBase::SetWideScreen(const bool wideScreen)
{
	m_WideScreen = wideScreen;
}

INLINE const E_RefreshRate C_DeviceBase::GetRefreshRate() const
{
	return m_RefreshRate;
}

INLINE const E_AspectRatio C_DeviceBase::GetAspectRatioType() const
{
	return m_AspectRatio;
}

INLINE void C_DeviceBase::SetAspectRatioType(const E_AspectRatio aspectRatio)
{
	m_AspectRatio = aspectRatio;
}

#ifdef PLATFORM_PC
// [Blur2PC]
INLINE void C_DeviceBase::SetAspectRatio(const F32 aspectRatio)
{
	m_PCAspectRatio = aspectRatio;
}
#endif

INLINE const E_LuminanceWrite C_DeviceBase::GetLuminanceWrite() const
{
	return m_LuminanceWrite;
}

INLINE void C_DeviceBase::SetLuminanceWrite(const E_LuminanceWrite luminanceWrite)
{
	m_LuminanceWrite = luminanceWrite == E_LuminanceWrite_Default ? m_LuminanceWriteDefault : luminanceWrite;
	m_BlendMode = E_BlendMode_Unknown;
}

INLINE const E_CullMode C_DeviceBase::GetCullMode() const
{
	return m_CullMode;
}

INLINE const E_CullMode C_DeviceBase::GetCullModeDefault() const
{
	return m_CullModeDefault;
}

INLINE const Maths::RGBA C_DeviceBase::GetColourClear() const
{
	return m_ColourClear;
}

INLINE const E_BlendMode C_DeviceBase::GetBlendMode() const
{
	return m_BlendMode;
}

INLINE const F32 C_DeviceBase::GetDepthClear() const
{
	return m_DepthCmpFlip ? 1.f - m_DepthClear : m_DepthClear;
}

INLINE const bool C_DeviceBase::GetDepthFlip() const
{
	return m_DepthCmpFlip;
}

INLINE const bool C_DeviceBase::GetDepthViewportFlip() const
{
	return m_DepthViewportFlip;
}

INLINE const U8 C_DeviceBase::GetStencilClear() const
{
	return m_StencilClear;
}

INLINE const C_Viewport& C_DeviceBase::GetViewport() const
{
	//assert(m_pView);
	//return m_pView->GetViewport();
	return m_Viewport;
}

INLINE const C_ViewFrustum& C_DeviceBase::GetViewFrustum() const
{
	assert(m_pView);
	return m_pView->GetViewFrustum();
}

INLINE const C_ViewSettings2D& C_DeviceBase::GetViewSettings2D() const
{
	assert(m_pView);
	return m_pView->GetViewSettings2D();
}

INLINE const C_ViewSettings3D& C_DeviceBase::GetViewSettings3D() const
{
	assert(m_pView);
	return m_pView->GetViewSettings3D();
}

INLINE const S32 C_DeviceBase::GetViewCount() const
{
	return m_Views.Size();
}

INLINE C_View* C_DeviceBase::GetView(const S32 id) const
{
	assert(id >= 0 && id < m_Views.Size());
	return m_Views[id];
}

INLINE const C_View* C_DeviceBase::GetView() const
{
	//assert(m_pView);
	return m_pView;
}

INLINE const S32 C_DeviceBase::GetRefCountSurfaces() const
{
	return m_RefCountSurfaces;
}

INLINE const S32 C_DeviceBase::GetRefCountTextures() const
{
	return m_RefCountTextures;
}

INLINE const S32 C_DeviceBase::GetRefCountCubeTextures() const
{
	return m_RefCountCubeTextures;
}

INLINE const S32 C_DeviceBase::GetRefCountVertexDeclarations() const
{
	return m_RefCountVertexDeclarations;
}

INLINE const S32 C_DeviceBase::GetRefCountVertexBuffers() const
{
	return m_RefCountVertexBuffers;
}

INLINE const S32 C_DeviceBase::GetRefCountIndexBuffers() const
{
	return m_RefCountIndexBuffers;
}

INLINE const S32 C_DeviceBase::GetRefCountVertexShaders() const
{
	return m_RefCountVertexShaders;
}

INLINE const S32 C_DeviceBase::GetRefCountPixelShaders() const
{
	return m_RefCountPixelShaders;
}

INLINE const E_CpuSpeed C_DeviceBase::GetCpuSpeed()
{
	return m_CpuSpeed;
}

INLINE const E_GpuSpeed C_DeviceBase::GetGpuSpeed()
{
	return m_GpuSpeed;
}

}; // namespace Display

#endif // DISPLAY_DEVICE_DEVICE_BASE_INL
