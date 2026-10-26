// [Blur1PCDX11] Game-facing native D3D11 device interface.
#ifndef DISPLAY_DEVICE_D3D11_D3D11_GAME_DEVICE_H
#define DISPLAY_DEVICE_D3D11_D3D11_GAME_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <d3d11.h>
#include <dxgi.h>
#include <d3d9.h> // Legacy asset enum values only; no D3D9 device.
#include "Display/Device/DeviceBase.h"
#include "Display/Device/D3D11/D3D11Resources.h"
#include "Display/Device/D3D11/D3D11GameDeviceParams.h"
#include "Display/Device/D3D11/D3D11DeviceBuffers.h"
#include "Display/Window/D3D11/D3D11Window.h"
#include "Display/Memory/MemoryTracker.h"
#include "Display/Buffer/DynamicVertexBuffer.h"
#include "Display/Buffer/D3D11/D3D11CommandBuffer.h"
#include "Display/Memory/D3D11/D3D11RingBuffer.h"
#include <core/threads/Misc.h>

namespace Display
{
namespace D3D11
{

// [Blur1PCDX11] Native FX passes assign only shaders, so FX defers its pass
// Apply (shaders, constant buffers, views, samplers) until the device next
// binds one of those itself or draws. Every such device call flushes first,
// which keeps the original order of bindings.
// [Blur1PCDX11] FX11 skips rebinding vertex/pixel state it already set
// (EffectRuntime.cpp). The device reports its own binds of those stages:
// stages bit 0 vertex, bit 1 pixel; what 1 shader, 2 constant buffers,
// 4 samplers, 8 shader resource views.
}}
void D3DX11InvalidateBoundState(unsigned stages, unsigned what);
void D3DX11InvalidateBoundConstantBuffers(unsigned stages, unsigned start, unsigned count);
namespace Display { namespace D3D11 {
// g_FlushNativeFX returns true when it applied a pass.
extern bool (*g_FlushNativeFX)();
void FlushNativeFX();

class C_Sampler : public C_SamplerBase
{
public:

	C_Sampler();
	~C_Sampler();

	void InvalidateState();

public:

    const C_Texture*	m_pTexture;

	// Border colour
	U32					m_BorderColour;
	U32					m_BorderColourDefault;

	S8					m_LodBias;
};

class C_Stream
{
public:

	C_Stream();
	~C_Stream();

	void InvalidateState();

public:

	const C_VertexBuffer*	m_pVertexBuffer;
	U32						m_VertexStride;
	U32						m_VertexOffset;
};

class C_D3D11GameDevice : public C_DeviceBase, public C_MemoryTracker
{
public:

	C_D3D11GameDevice();
	~C_D3D11GameDevice();
	void SetNativeFXActive(bool active);
	// [Blur1PCDX11] Reset IA binding memos after context resets or raw diagnostic draws.
	void InvalidateNativeInputBindings();
	// [Blur2PC][HDR] Display calibration occupies b12, outside legacy b0-b7.
	virtual void BindHDROutputConstants() {}

	/**************************************************************************************************
	- Public cross-platform functions.
	- Safe to use by game as these functions are guaranteed to be cross-platform.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	void InvalidateState();
	void DefaultStates();
	bool BeginScene();
	void EndScene();
	void Reset();
	void Unbind();
	void Clear(const U32 flags);
	void Clear(const U32 flags, const Maths::RGBA colour);
	void Clear(const U32 flags, const Maths::RGBA colour, const F32 depth, const U8 stencil);
	bool Present();

	// Primitive drawing
	void DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 startIndex, const U32 indexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);
	
	// State setting
	void SetAlphaTestEnable(const E_Bool alphaTestEnable);
	void SetAlphaTestFunc(const E_CmpFunc alphaTestFunc);
	void SetAlphaTestRef(const U32 alphaTestRef);
	void SetAlphaToMaskEnable(const E_Bool alphaTestEnable);
	void SetBlend(const E_Blend blendSrc, const E_Blend blendDst, const E_BlendOp blendOp, const E_Blend blendSrcAlpha, const E_Blend blendDstAlpha, const E_BlendOp blendOpAlpha);
	void SetBlendDst(const E_Blend blendDst);
	void SetBlendDstAlpha(const E_Blend blendDst);
	void SetBlendEnable(const E_Bool blendEnable);
	void SetBlendMode(const E_BlendMode blendMode);
	void SetBlendOp(const E_BlendOp blendOp);
	void SetBlendOpAlpha(const E_BlendOp blendOp);
	void SetBlendSrc(const E_Blend blendSrc);
	void SetBlendSrcAlpha(const E_Blend blendSrc);
	void SetColourWriteEnable(U32 colourWriteEnable);
	void SetCullMode(E_CullMode cullMode);
	void SetCullModeFlip();
	void SetCullModeFlip(const bool cullModeFlip);
	void SetDepthBias(const F32 depthBias);
	void SetDepthEnable(E_Bool depthEnable);
	void SetDepthFlip(const bool depthCmpFlip, const bool depthViewportFlip = true);
	void SetDepthFunc(E_CmpFunc depthFunc);
	void SetDepthWriteEnable(const E_Bool depthWriteEnable);
	void SetFillMode(E_FillMode fillMode);
	void SetPointSpriteEnable(const E_Bool pointSpriteEnable);
	void SetIndexBuffer(const C_IndexBuffer* pIndexBuffer);
	void SetPixelShader(const C_PixelShader* pPixelShader);
	const C_PixelShader* GetPixelShader();
	void SetPixelShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount);
	void SetPixelShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount);
	void SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetPixelShaderParameterF(const C_PixelShaderParameter parameter, const F32* pData);
	void SetPixelShaderParameterArrayF(const C_PixelShaderParameterArray& parameterArray, const F32* pData);
	void SetPixelShaderParameters(const C_PixelShader* pPixelShader);
	void SetPresentInterval(E_PresentInterval presentInterval);
	void SetScissorTest(const E_Bool scissorTestEnable);
	void SetScissorRect(const U32 x, const U32 y, const U32 w, const U32 h);
	void SetSlopeScaledBias(const F32 slopeScaledBias);
	void SetStencilState(const Display::C_StencilState& stencilState);
	void SetStencilEnable(const E_Bool stencilEnable);
	void SetStencilFail(const E_StencilOp stencilFail);
	void SetStencilDepthFail(const E_StencilOp stencilDepthFail);
	void SetStencilPass(const E_StencilOp stencilPass);
	void SetStencilFunc(const E_CmpFunc stencilFunc);
	void SetStencilRef(const S32 stencilRef);
	void SetStencilMask(const U32 stencilMask);
	void SetStencilWriteMask(const U32 stencilWriteMask);
	void SetStencilCcwEnable(const E_Bool stencilCcwEnable);
	void SetStencilCcwFail(const E_StencilOp stencilFail);
	void SetStencilCcwDepthFail(const E_StencilOp stencilDepthFail);
	void SetStencilCcwPass(const E_StencilOp stencilPass);
	void SetStencilCcwFunc(const E_CmpFunc stencilFunc);
	void SetStencilCcwRef(const S32 stencilRef);
	void SetStencilCcwMask(const U32 stencilMask);
	void SetStencilCcwWriteMask(const U32 stencilWriteMask);
	
	// Pixel shader samples
	void SetTexture(const U32 sampler, const C_Texture* pTexture);
	void SetNativeTexture(U32 sampler, ID3D11Resource* texture);
	void SetTextureAddressU(const U32 sampler, E_TextureAddress addressU);
	void SetTextureAddressV(const U32 sampler, E_TextureAddress addressV);
	void SetTextureAddressW(const U32 sampler, E_TextureAddress addressW);
	void SetTextureAddressUVW(const U32 sampler, const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW);
	void SetTextureFilterMag(const U32 sampler, E_TextureFilter magFilter);
	void SetTextureFilterMin(const U32 sampler, E_TextureFilter minFilter);
	void SetTextureFilterMip(const U32 sampler, E_TextureFilter mipFilter);
	void SetTextureFilterMagMinMip(const U32 sampler, const E_TextureFilter filterMag, const E_TextureFilter filterMin, const E_TextureFilter filterMip);
	void SetTextureBorderColour(const U32 sampler, const U32 borderColour);
	void SetTextureLodBias(const U32 sampler, const S8 lodBias);
	void SetTextureGamma(const U32 sampler, E_Bool gamma);

	// Vertex shader samplers
	void SetVertexTexture(const U32 sampler, const C_Texture* pTexture);
	void SetNativeVertexTexture(U32 sampler, ID3D11Resource* texture);
	void SetVertexTextureAddressU(const U32 sampler, E_TextureAddress addressU);
	void SetVertexTextureAddressV(const U32 sampler, E_TextureAddress addressV);
	void SetVertexTextureAddressW(const U32 sampler, E_TextureAddress addressW);
	void SetVertexTextureAddressUVW(const U32 sampler, const E_TextureAddress addressU, const E_TextureAddress addressV, const E_TextureAddress addressW);
	void SetVertexTextureFilterMag(const U32 sampler, E_TextureFilter magFilter);
	void SetVertexTextureFilterMin(const U32 sampler, E_TextureFilter minFilter);
	void SetVertexTextureFilterMip(const U32 sampler, E_TextureFilter mipFilter);
	void SetVertexTextureFilterMagMinMip(const U32 sampler, const E_TextureFilter filterMag, const E_TextureFilter filterMin, const E_TextureFilter filterMip);
	void SetVertexTextureBorderColour(const U32 sampler, const U32 borderColour);
	void SetVertexTextureLodBias(const U32 sampler, const S8 lodBias);
	void SetVertexTextureGamma(const U32 sampler, E_Bool gamma);
	
	void SetVertexBuffer(const U32 stream, const C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 stride);
	void SetVertexDeclaration(const C_VertexDeclaration* pVertexDeclaration);
	const C_VertexShader* GetVertexShader();
	void SetVertexShader(const C_VertexShader* pVertexShader);
	void SetVertexShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount);
	void SetVertexShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount);
	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetVertexShaderParameterF(const C_VertexShaderParameter startParameter, const F32* pData, const U32 parameterCount);
	void SetViewport(const C_Viewport& viewport);

	// Surface management
	void SetColourDepthStencilSurface();
	void ReportTextureStats(const char* tag, ID3D11Resource* resource, unsigned index);  // [Blur1PCDX11] diagnostics
	ID3D11Resource* GetBoundTexture(U32 slot) const;  // [Blur1PCDX11] diagnostics
	void ReportPixelConstants(U32 start, U32 count) const;  // [Blur1PCDX11] diagnostics
	void SetColourDepthStencilSurface(const C_Surface* pColourSurface, const C_Surface* pDepthStencilSurface);
	void SetColourSurface();
	void SetColourSurface(const C_Surface* pSurface);
	void SetColourSurface(const U32 index, const C_Surface* pSurface);
	void SetDepthStencilSurface();
	void SetDepthStencilSurface(const C_Surface* pSurface);
	bool CreateColourSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface);
	bool CreateColourSurface(const U32 width, const U32 height, const U32 flags, const D3DFORMAT format, const D3DMULTISAMPLE_TYPE multisample, const C_SurfaceParams* pParams, C_Surface* pSurface);
	void DestroyColourSurface(C_Surface* pSurface);
	bool CreateDepthStencilSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface);
	bool CreateDepthStencilSurface(const U32 width, const U32 height, const U32 flags, const D3DFORMAT format, const D3DMULTISAMPLE_TYPE multisample, const C_SurfaceParams* pParams, C_Surface* pSurface);
	void DestroyDepthStencilSurface(C_Surface* pSurface);
	void DestroySurface(C_Surface* pSurface);

	// Texture 2d management
	bool CreateTexture2D(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_Format format, C_Texture2D* pTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyTexture2D(C_Texture2D* pTexture);

	// Texture 3d management
	bool CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_Format format, C_Texture3D* pTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyTexture3D(C_Texture3D* pTexture);

	// Texture cube management
	bool CreateTextureCube(const U32 width, const U32 levels, const U32 flags, const E_Format format, C_TextureCube* pTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyTextureCube(C_TextureCube* pTexture);

	// Render texture cube management
	bool CreateRenderTextureCube(const U32 width, const U32 levels, U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTextureCube* pRenderTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyRenderTextureCube(C_RenderTextureCube* pRenderTexture);

	// Vertex buffer management
	bool CreateVertexBuffer(const U32 size, const U32 flags, C_VertexBuffer* pVertexBuffer, bool allowFail = false);
	void DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer);

	// Index buffer management
	bool CreateIndexBuffer(const U32 size, const U32 flags, const E_Format format, C_IndexBuffer* pIndexBuffer, bool allowFail = false);
	void DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer);

	// Vertex shader management
	#ifdef PLATFORM_PC
	bool CreateVertexShader(const U32 size, const C_ShaderData& data, const U32 flags, C_VertexShader* pVertexShader);
	#endif
	bool CreateVertexShader(const U32 size, const S32* pData, const U32 flags, C_VertexShader* pVertexShader);
	void DestroyVertexShader(C_VertexShader* pVertexShader);

	// Pixel shader management
	#ifdef PLATFORM_PC
	bool CreatePixelShader(const U32 size, const C_ShaderData& data, const U32 flags, C_PixelShader* pPixelShader);
	#endif
	bool CreatePixelShader(const U32 size, const S32* pData, const U32 flags, C_PixelShader* pPixelShader);
	void DestroyPixelShader(C_PixelShader* pPixelShader);

	// Shader constant management
	C_PixelShaderConstantLockKey	LockPixelShaderConstants(const U32 startRegister, const U32 endRegister);
	void							UnlockPixelShaderConstants(const C_PixelShaderConstantLockKey lockKey);
	bool							CheckPixelShaderConstantsLocked(const U32 startRegister, const U32 endRegister);
	
	// View management
	void SetupViewIndex(const S32 viewIndex, const C_ViewSetup& viewSetup);
	void SetupViewIndex(const S32 viewIndex, const C_ViewSetup& viewSetup, const U32 width, const U32 height);
	void SetViewIndex(const S32 viewIndex, const bool setStates = true);
	void SetupView(C_View* pView, const C_ViewSetup& viewSetup);
	void SetupView(C_View* pView, const C_ViewSetup& viewSetup, const U32 width, const U32 height);
	void SetView(const C_View* pView, const bool setStates = true);
	void SetViewShaderConstants();
	void SetViewShaderConstants2D();
	void SetViewShaderConstants3D();
	
	/**************************************************************************************************
	- Public platform specific functions.
	- Not safe to use by game as these functions are not guaranteed to be cross-platform.
	- Wrap functions calls with define GRAPHICS_D3D9.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	void SetBlendHighPrecisionEnable(const E_Bool blendHighPrecisionEnable);

	// Buffer management
	const S32 GetBackBufferCount() const;
	const C_BackBuffer& GetBackBuffer() const;
	const C_BackBuffer& GetBackBuffer(const S32 backBufferIndex) const;
	const S32 GetFrontBufferCount() const;
	const C_FrontBuffer& GetFrontBuffer() const;
	const C_FrontBuffer& GetFrontBuffer(const S32 frontBufferIndex) const;
	const C_FrontBuffer& GetSwappedFrontBuffer() const;
	const C_RenderTarget& GetRenderTarget() const;

	bool IsSet(const C_VertexBuffer* pVertexBuffer) const;

	// Window management
	const S32 GetWindowCount() const;
	C_Window* GetWindow(const S32 windowIndex);
	const S32 AddWindow(const Maths::C_Rect& windowCoords);

	void InitialiseBuffers();

	ID3D11Device* GetD3DDevice();
	IDXGIFactory1* GetD3DObject();

	DXGI_SWAP_CHAIN_DESC& GetD3DPresentParams();

	const E_Bool GetForceTextureGamma() const;
	void SetForceTextureGamma(const E_Bool forceTextureGamma);

	// Control ownership of rendering between threads
	void InitialiseThreadOwnership();
	bool AcquireThreadOwnership( U32 timeMs = WAIT_TIME_INFINITE );
	void ReleaseThreadOwnership();
	bool HaveThreadOwnership();

protected:

	/**************************************************************************************************
	- Protected cross-platform functions.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	// Device management
	bool CreateDevice();
	bool CreateDeviceVolatile();
	void DestroyDeviceVolatile();
	void DestroyDevice();

	// Vertex declaration management
	const U32 GetVertexDeclarationCount() const;
	C_VertexDeclaration* GetVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements);
	C_VertexDeclaration* InitVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements);

	/**************************************************************************************************
	- Protected platform specific functions.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// Surface management
	bool CreateColourSurface();
	void DestroyColourSurface();
	bool CreateDepthStencilSurface();
	void DestroyDepthStencilSurface();

private:

	/**************************************************************************************************
	- Private cross-platform functions.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	// TODO: Add functions here.

	/**************************************************************************************************
	- Private platform specific functions.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// Device management
	void DestroyD3D11();

public:

	Display2::C_Device					m_Display2;

	C_DeviceParams						m_DeviceParams;


	E_Bool								m_BlendHighPrecisionEnable;
	F32									m_SlopeScaledBias;
	E_PresentInterval					m_PresentInterval;
	E_PresentInterval					m_PresentIntervalDefault;
	E_Bool								m_ForceTextureGamma;

	const C_VertexShader*				m_pVertexShader;
	const C_PixelShader*				m_pPixelShader;
	C_Sampler							m_Sampler[E_Sampler_Count];
	C_Sampler							m_VertexSampler[E_VertexSampler_Count];
	const C_VertexDeclaration*			m_pVertexDeclaration;
	C_Stream							m_Stream[E_Stream_Count];
	const C_IndexBuffer*				m_pIndexBuffer;
	ID3D11Device*					m_pd3dDevice;
	IDXGIFactory1*							m_pd3dObject;

	DXGI_SWAP_CHAIN_DESC				m_d3dPresentParams;

	C_PixelShaderConstantLock			m_PixelShaderConstantLock[E_PixelShaderConstantLock_Count];

	C_RenderTarget						m_RenderTarget;

	S32									m_CurrentBackBuffer;
	Ut::Vector<C_BackBuffer*>			m_BackBuffers;
	
	S32									m_CurrentFrontBuffer;
	S32									m_SwappedFrontBuffer;
	Ut::Vector<C_FrontBuffer*>			m_FrontBuffers;

	S32									m_CurrentWindow;
	Ut::Vector<C_Window*>				m_Windows;


	MT::Mutex							m_ThreadOwnershipMutex;
};

}; // namespace D3D11
using namespace D3D11; // [Blur1PCDX11] Match the game facade's selected backend.
}; // namespace Display

#endif // DISPLAY_DEVICE_D3D11_D3D11_GAME_DEVICE_H
