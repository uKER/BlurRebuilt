#ifndef DISPLAY_DEVICE_DEVICE_BASE_H
#define DISPLAY_DEVICE_DEVICE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceInterface.h"
#include "Display/Device/DeviceStates.h"
#include "Display/Device/StencilState.h"
#include "Display/Utils/Renderer.h"
#include "Display/View/View.h"

namespace Display
{

class C_DeviceBase;

//! E_CpuSpeed

enum E_CpuSpeed
{
	E_CpuSpeed_Low,
	E_CpuSpeed_High,
};

//! E_GpuSpeed

enum E_GpuSpeed
{
	E_GpuSpeed_Low,
	E_GpuSpeed_High,
};

//! C_DeviceMutex

class C_DeviceMutex
{
public:

	C_DeviceMutex(C_DeviceBase* pDeviceBase);
	~C_DeviceMutex();

private:

	C_DeviceBase* m_pDeviceBase;
};

//! C_SamplerBase

class C_SamplerBase
{
public:

	C_SamplerBase();
	~C_SamplerBase();

	void InvalidateState();

public:

	// Texture address
    E_TextureAddress	m_AddressU;
	E_TextureAddress	m_AddressUDefault;
	E_TextureAddress	m_AddressV;
	E_TextureAddress	m_AddressVDefault;
	E_TextureAddress	m_AddressW;
	E_TextureAddress	m_AddressWDefault;

	// Texture filer
	E_TextureFilter		m_FilterMag;
	E_TextureFilter		m_FilterMagDefault;
	E_TextureFilter		m_FilterMin;
	E_TextureFilter		m_FilterMinDefault;
	E_TextureFilter		m_FilterMip;
	E_TextureFilter		m_FilterMipDefault;

	E_Bool				m_Gamma;
	E_Bool				m_GammaDefault;
};

//! C_DeviceBase

class C_DeviceBase : public I_Device
{
public:

	C_DeviceBase();
	~C_DeviceBase();

	const bool IsCreated() const;
	const bool InScene() const;
	const bool InParallel() const;

	F32 GetCpuTime() const;
	F32 GetGpuTime() const;

	const E_ShaderProfile GetShaderProfile() const;
	const E_VertexShaderProfile GetVertexShaderProfile() const;
	const E_PixelShaderProfile GetPixelShaderProfile() const;

	const bool IsHiDef() const;
	
	const bool GetWideScreen() const;
	void SetWideScreen(const bool wideScreen);

	const E_RefreshRate GetRefreshRate() const;
	
	const F32			GetAspectRatio() const;
	const E_AspectRatio GetAspectRatioType() const;
	void SetAspectRatioType(const E_AspectRatio aspectRatio);
#ifdef PLATFORM_PC
	void SetAspectRatio(const F32 aspectRatio);	// [Blur2PC]
#endif
	
	const E_LuminanceWrite GetLuminanceWrite() const;
	void SetLuminanceWrite(const E_LuminanceWrite luminanceWrite);

	const E_CullMode GetCullMode() const;
	const E_CullMode GetCullModeDefault() const;
	
	// State management
	void InvalidateState();

	// Colour querying
	const Maths::RGBA GetColourClear() const;

	// Blend querying
	const E_BlendMode GetBlendMode() const;

	// Depth querying
	const F32 GetDepthClear() const;
	const bool GetDepthFlip() const;
	const bool GetDepthViewportFlip() const;
	
	// Stencil querying
	const U8 GetStencilClear() const;

	// View querying
	const C_Viewport& GetViewport() const;
	const C_ViewFrustum& GetViewFrustum() const;
	const C_ViewSettings2D& GetViewSettings2D() const;
	const C_ViewSettings3D& GetViewSettings3D() const;
	
	// View management
	void ClearViews();
	const S32 GetViewCount() const;
	C_View* GetView(const S32 id) const;
	const C_View* GetView() const;
	const S32 AddView(const U32 type, const S32 userIndex);
	
	// Ref counting
	const S32 GetRefCountSurfaces() const;
	const S32 GetRefCountTextures() const;
	const S32 GetRefCountCubeTextures() const;
	const S32 GetRefCountVertexDeclarations() const;
	const S32 GetRefCountVertexBuffers() const;
	const S32 GetRefCountIndexBuffers() const;
	const S32 GetRefCountVertexShaders() const;
	const S32 GetRefCountPixelShaders() const;

	// Renderer management
	void InsertRenderer(I_Renderer* pRenderer);
	void RemoveRenderer(I_Renderer* pRenderer);

	// creation and destruction of permanant assets
	bool CreateRenderers(E_RendererType rendererType = E_RendererType_Unknown);
	bool CreateRenderersVolatile(E_RendererType rendererType = E_RendererType_Unknown);
	void DestroyRenderersVolatile(E_RendererType rendererType = E_RendererType_Unknown);
	void DestroyRenderers(E_RendererType rendererType = E_RendererType_Unknown);

	// creation and destruction of temporary assets
	bool InitialiseRenderers();
	void ShutdownRenderers();
	void ResetRenderers();

	bool PreResizeRenderers();
	bool PostResizeRenderers();

	void MutexWait();
	void MutexRelease();

	static const E_CpuSpeed GetCpuSpeed();
	static void SetCpuSpeed(const E_CpuSpeed cpuSpeed);

	static const E_GpuSpeed GetGpuSpeed();
	static void SetGpuSpeed(const E_GpuSpeed gpuSpeed);

protected:

	void AssertEverythingHasBeenDestroyed();

protected:

	bool										m_HiDef;
	bool										m_WideScreen;
	E_RefreshRate								m_RefreshRate;
	E_AspectRatio								m_AspectRatio;
#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC keeps the device aspect as a float (C_DeviceBase +0x8, 16/9 from the
	// constructor 0x005a4f90) that follows the back buffer; GetAspectRatio returns it (T254)
	F32											m_PCAspectRatio;
#endif

	E_ShaderProfile								m_ShaderProfile;
	E_VertexShaderProfile						m_VertexShaderProfile;
	E_PixelShaderProfile						m_PixelShaderProfile;

	// Alpha test
	E_Bool										m_AlphaTestEnable;
	E_Bool										m_AlphaTestEnableDefault;
	E_CmpFunc									m_AlphaTestFunc;
	E_CmpFunc									m_AlphaTestFuncDefault;
	U32											m_AlphaTestRef;
	U32											m_AlphaTestRefDefault;

	E_Bool										m_AlphaToMaskEnable;
	E_Bool										m_AlphaToMaskEnableDefault;

	// Blend
	E_Bool										m_BlendEnable;
	E_Bool										m_BlendEnableDefault;
	E_Blend										m_BlendSrc;
	E_Blend										m_BlendSrcDefault;
	E_Blend										m_BlendSrcAlpha;
	E_Blend										m_BlendSrcAlphaDefault;
	E_Blend										m_BlendDst;
	E_Blend										m_BlendDstDefault;
	E_Blend										m_BlendDstAlpha;
	E_Blend										m_BlendDstAlphaDefault;
	E_BlendOp									m_BlendOp;
	E_BlendOp									m_BlendOpDefault;
	E_BlendOp									m_BlendOpAlpha;
	E_BlendOp									m_BlendOpAlphaDefault;
	E_BlendMode									m_BlendMode;
	E_BlendMode									m_BlendModeDefault;

	// Colour
	Maths::RGBA									m_ColourClear;
	U32											m_ColourWriteEnable;
	U32											m_ColourWriteEnableDefault;
	
	// Cull mode
	E_CullMode									m_CullMode;
	E_CullMode									m_CullModeDefault;
	bool										m_CullModeFlip;

	// Depth
	F32											m_DepthBias;
	bool										m_DepthCmpFlip;
	bool										m_DepthViewportFlip;
	F32											m_DepthClear;
	E_Bool										m_DepthEnable;
	E_Bool										m_DepthEnableDefault;
	E_CmpFunc									m_DepthFunc;
	E_CmpFunc									m_DepthFuncDefault;
	E_Bool										m_DepthWriteEnable;
	E_Bool										m_DepthWriteEnableDefault;
	
	// Fill mode
	E_FillMode									m_FillMode;
	E_FillMode									m_FillModeDefault;

	// Point sprites
	E_Bool										m_PointSpriteEnable;
	E_Bool										m_PointSpriteEnableDefault;

	// Luminance
	E_LuminanceWrite							m_LuminanceWrite;
	E_LuminanceWrite							m_LuminanceWriteDefault;

	// Stencil
	U8											m_StencilClear;
	E_Bool										m_StencilEnable;
	E_Bool										m_StencilEnableDefault;

	// View management
	C_Viewport									m_Viewport;
	const C_View*								m_pView;
	Ut::UtVector<Ut::DynamicVector<C_View*> >	m_Views;

	// Ref counting
	S32											m_RefCountSurfaces;
	S32											m_RefCountTextures;
	S32											m_RefCountCubeTextures;
	S32											m_RefCountVertexDeclarations;
	S32											m_RefCountVertexBuffers;
	S32											m_RefCountIndexBuffers;
	S32											m_RefCountVertexShaders;
	S32											m_RefCountPixelShaders;

	// Renderer management
	Ut::UtSet<I_Renderer*>						m_Renderers;

	bool										m_Created;
	bool										m_InScene;
	bool										m_InParallel; // Denotes whether this device is running in parrellel with another one.

	MT::Mutex									m_Mutex;
	MT::Mutex									m_ViewMutex;


	F32											m_CpuTime;
	F32											m_GpuTime;
	
	static E_CpuSpeed							m_CpuSpeed;
	static E_GpuSpeed							m_GpuSpeed;
};

}; // namespace Display

#include "Display/Device/DeviceBase.inl"

#endif // DISPLAY_DEVICE_DEVICE_BASE_H
