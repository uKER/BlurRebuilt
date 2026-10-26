#ifndef DISPLAY_DEVICE_GCM_GCM_DEVICE_H
#define DISPLAY_DEVICE_GCM_GCM_DEVICE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Device/DeviceBase.h"
#include "Display/Device/GCM/GCM.h"
#include "Display/Device/GCM/GCMDeviceParams.h"
#include "Display/Device/GCM/GCMDeviceBuffers.h"
#include "Display/Device/GCM/GCMDeviceStates.h"
#include "Display/Device/GCM/GCMFence.h"
#include "Display/Surface/GCM/GCMSurface.h"
#include "Display/Texture/GCM/GCMTexture2D.h"
#include "Display/Texture/GCM/GCMTexture3D.h"
#include "Display/Texture/GCM/GCMTextureCube.h"
#include "Display/Texture/GCM/GCMRenderTexture2D.h"
#include "Display/Texture/GCM/GCMRenderTextureCube.h"
#include "Display/Shader/GCM/GCMVertexShaderCache.h"
#include "Display/Shader/GCM/GCMVertexShader.h"
#include "Display/Shader/GCM/GCMPixelShader.h"
#include "Display/Buffer/GCM/GCMVertexDeclaration.h"
#include "Display/Buffer/GCM/GCMVertexBuffer.h"
#include "Display/Buffer/GCM/GCMIndexBuffer.h"
#include "Display/Buffer/GCM/GCMCommandBuffer.h"
#include "Display/Buffer/GCM/GCMUserFifo.h"
#include "Display/Buffer/GCM/GCMVertexBufferPatched.h"
#include "Display/Window/GCM/GCMWindow.h"
#include "Display/Buffer/DynamicVertexBuffer.h"
#include "Display/Memory/MemoryTracker.h"
#include "Display/Memory/MemoryManager2.h"
#include "btl/containers/set.h"
#include "btl/allocators/pool.h"
#include "Display/Device/Gcm/GCMDeviceEnums.h"
#include <core/threads/Misc.h>

extern bool g_CrashGPU;

namespace Display
{
namespace GCM
{

#ifdef BUILD_CONFIG_DEBUG
//#define GCM_DEBUG
#endif

typedef Display2::C_RenderBuffer C_RenderBuffer;

//! C_Sampler

class C_Sampler : public C_SamplerBase
{
public:

	C_Sampler();
	~C_Sampler();

	void InvalidateState();

	void SetTexture(const C_Texture* pTexture);
	void SetAddressU(const E_TextureAddress addressU);
	void SetAddressV(const E_TextureAddress addressV);
	void SetAddressW(const E_TextureAddress addressW);
	void SetMagFilter(const E_TextureFilter filterMag);
	void SetMinFilter(const E_TextureFilter filterMin);
	void SetMinLod(const U16 minLod);
	void SetMaxLod(const U16 maxLod);
	void SetBorderColour(const U32 borderColour);
	void Set(const U32 sampler);
	void SetDefaults(const U32 sampler);

public:

	const C_Texture*		m_pTexture;

	// Border colour
	U32						m_BorderColour;
	U32						m_BorderColourDefault;

	S8						m_LodBias;
	S8						m_LodBiasDefault;
	S16						m_gcmLodBias;
	bool					m_gcmTextureFilterDirty;

private:

	bool					m_SetTexture;
	bool					m_SetControl;
	bool					m_SetAddress;
	bool					m_SetFilter;
	bool					m_SetBorderColour;
};

//! C_Stream

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

struct C_DirtySampler
{
	C_DirtySampler();

	void Invalidate();

	bool					m_AddressU;
	bool					m_AddressV;
	bool					m_AddressW;
	bool					m_FilterMag;
	bool					m_FilterMin;
	bool					m_FilterMip;
	bool					m_BorderColour;
	bool					m_LodBias;
	bool					m_Gamma;
};

struct C_DirtyDevice
{
	C_DirtyDevice();

	void Invalidate();

	C_DirtySampler			m_Samplers[E_Sampler_Count];
};

//! C_GCMDevice

class C_GCMDevice : public C_DeviceBase, public C_MemoryTracker
{
public:

	C_GCMDevice();
	~C_GCMDevice();

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
	void Unbind() {} // to do: does this need to do anything?
	void Clear(const U32 flags);
	void Clear(const U32 flags, const Maths::RGBA colour);
	void Clear(const U32 flags, const Maths::RGBA colour, const F32 depth, const U8 stencil);
	bool Present(bool flip = true);
	
	// Dynamic vertex buffer management
	const bool BeginVerticesLocked() const;
	void* BeginVertices(const E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, C_VertexDeclaration* pVertexDeclaration, const bool checkUsage = false);
	void EndVertices(void* pVertex = NULL);

	// Primitive drawing
	void DrawPrimitive(const E_PrimitiveType primitiveType, const U32 startVertex, const U32 vertexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 startIndex, const U32 indexCount);
	void DrawIndexedPrimitive(const E_PrimitiveType primitiveType, const U32 baseVertex, const U32 minIndex, const U32 vertexCount, const U32 startIndex, const U32 indexCount);
	
	// State setting
	void SetAlphaTestEnable(const E_Bool alphaTestEnable);
	void SetAlphaTestFunc(const E_CmpFunc alphaTestFunc);
	void SetAlphaTestRef(const U32 alphaTestRef);
	void SetAlphaToMaskEnable(const E_Bool alphaToMaskEnable);
	void SetBlend(const E_Blend blendSrc, const E_Blend blendDst, const E_BlendOp blendOp, const E_Blend blendSrcAlpha, const E_Blend blendDstAlpha, const E_BlendOp blendOpAlpha);
	void SetBlendDst(const E_Blend blendDst);
	void SetBlendDstAlpha(const E_Blend blendDstAlpha);
    void SetBlendEnable(const E_Bool blendEnable);
    void SetBlendMode(const E_BlendMode blendMode);
	void SetBlendOp(const E_BlendOp blendOp);
	void SetBlendOpAlpha(const E_BlendOp blendOpAlpha);
	void SetBlendSrc(const E_Blend blendSrc);
	void SetBlendSrcAlpha(const E_Blend blendSrcAlpha);
	void SetColourWriteEnable(U32 colourWriteEnable);
    void SetCullMode(E_CullMode cullMode);
	void SetCullModeFlip();
	void SetCullModeFlip(const bool cullModeFlip);
	void SetDepthBias(const F32 depthBias);
	void SetDepthEnable(E_Bool depthEnable);
	void SetDepthFlip(const bool depthCmpFlip, const bool depthViewportFlip = true);
	void SetDepthFunc(E_CmpFunc depthFunc);
	void SetDepthWriteEnable(E_Bool depthWriteEnable);
	void SetFillMode(E_FillMode fillMode);
	void SetPointSpriteEnable(const E_Bool pointSpriteEnable);
	void SetIndexBuffer(const C_IndexBuffer* pIndexBuffer);
    void SetPixelShader(const C_PixelShader* pPixelShader);
	void SetPixelShader(const C_PixelShader* pPixelShader, const U8* pMicroCode);
    const C_PixelShader* GetPixelShader();
	void SetPixelShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount);
	void SetPixelShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount);
	void SetPixelShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetPixelShaderParameterF(const C_PixelShaderParameter parameter, const F32* pData);
	void SetPixelShaderParameterArrayF(const C_PixelShaderParameterArray& parameterArray, const F32* pData);
	void SetPixelShaderParameters(const C_PixelShader* pPixelShader);
	void SetPresentInterval(E_PresentInterval presentInterval);
	void SetSlopeScaledBias(const F32 slopeScaledBias);
	void SetStencilState(const C_StencilState& stencilState);
	void SetStencilEnable(E_Bool stencilEnable);
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
	void SetTextureAddressU(const U32 sampler, E_TextureAddress addressU);
	void SetTextureAddressV(const U32 sampler, E_TextureAddress addressV);
	void SetTextureAddressW(const U32 sampler, E_TextureAddress addressW);
	void SetTextureAddressUVW(const U32 sampler, E_TextureAddress addressU, E_TextureAddress addressV, E_TextureAddress addressW);
	void SetTextureFilterMag(const U32 sampler, E_TextureFilter filterMag);
	void SetTextureFilterMin(const U32 sampler, E_TextureFilter filterMin);
	void SetTextureFilterMip(const U32 sampler, E_TextureFilter filterMip);
	void SetTextureFilterMagMinMip(const U32 sampler, E_TextureFilter filterMag, E_TextureFilter filterMin, E_TextureFilter filterMip);
	void SetTextureBorderColour(const U32 sampler, const U32 borderColour);
	void SetTextureLodBias(const U32 sampler, const S8 lodBias);
	void SetTextureGamma(const U32 sampler, const E_Bool gamma);

	// Vertex shader samplers
	void SetVertexTexture(const U32 sampler, const C_Texture* pTexture);
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
	void SetVertexShader(const C_VertexShader* pVertexShader);
	const C_VertexShader* GetVertexShader();
	void SetVertexShaderConstantB(const U32 startRegister, const BOOL* pData, const U32 registerCount);
	void SetVertexShaderConstantI(const U32 startRegister, const S32* pData, const U32 registerCount);
	void SetVertexShaderConstantF(const U32 startRegister, const F32* pData, const U32 registerCount);
	void SetVertexShaderParameterF(const C_VertexShaderParameter startParameter, const F32* pData, const U32 parameterCount);
	void SetViewport(const C_Viewport& viewport);
	
	// Surface management
	void SetColourDepthStencilSurface();
	void SetColourDepthStencilSurface(const C_Surface* pColourSurface, const C_Surface* pDepthStencilSurface);
	void SetColourSurface();
	void SetDepthStencilSurface();
	void SetColourSurface(const C_Surface* pSurface);
	void SetColourSurface(const U32 index, const C_Surface* pSurface);
	void SetDepthStencilSurface(const C_Surface* pSurface);
	bool CreateSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroySurface(C_Surface* pSurface);

	// Colour surface management
	bool CreateColourSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyColourSurface(C_Surface* pSurface);

	// Depth stencil surface management
	bool CreateDepthStencilSurface(const U32 width, const U32 height, const U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_Surface* pSurface, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyDepthStencilSurface(C_Surface* pSurface);
	
	// Texture 2d management
	bool CreateTexture2D(const U32 width, const U32 height, const U32 levels, const U32 flags, const E_Format format, C_Texture2D* pTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyTexture2D(C_Texture2D* pTexture);

	// Texture 3d management
	bool CreateTexture3D(const U32 width, const U32 height, const U32 depth, const U32 levels, const U32 flags, const E_Format format, C_Texture3D* pTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyTexture3D(C_Texture3D* pTexture);

	// Texture cube management
	bool CreateTextureCube(const U32 width, const U32 levels, const U32 flags, const E_Format format, C_TextureCube* pTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyTextureCube(C_TextureCube* pTexture);

	// Render texture 2d management
	bool CreateRenderTexture2D(const U32 width, const U32 height, const U32 levels, U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTexture2D* pRenderTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyRenderTexture2D(C_RenderTexture2D* pRenderTexture);

	// Render texture cube management
	bool CreateRenderTextureCube(const U32 width, const U32 levels, U32 flags, const E_Format format, const E_Multisample multisample, const C_SurfaceParams* pParams, C_RenderTextureCube* pRenderTexture, U8* pAddress = NULL, const U32 addressSize = 0);
	void DestroyRenderTextureCube(C_RenderTextureCube* pRenderTexture);

	void DestroyRenderBuffer(C_RenderBuffer* pRenderBuffer);
	
	// Vertex buffer management
	bool CreateVertexBuffer(const U32 size, const U32 flags, C_VertexBuffer* pVertexBuffer, bool allowFail = false);
	void DestroyVertexBuffer(C_VertexBuffer* pVertexBuffer);

	// Index buffer management
	bool CreateIndexBuffer(const U32 size, const U32 flags, const E_Format format, C_IndexBuffer* pIndexBuffer, bool allowFail = false);
	void DestroyIndexBuffer(C_IndexBuffer* pIndexBuffer);

	// Vertex shader management
	bool CreateVertexShader(const U32 size, const S32* pData, const U32 flags, C_VertexShader* pVertexShader);
	void DestroyVertexShader(C_VertexShader* pVertexShader);

	// Pixel shader management
	bool CreatePixelShader(const U32 size, const S32* pData, const U32 flags, C_PixelShader* pPixelShader);
	void DestroyPixelShader(C_PixelShader* pPixelShader);

	// Shader constant management - doesnt do anything in GCM
	C_PixelShaderConstantLockKey	LockPixelShaderConstants(const U32 startRegister, const U32 endRegister) { return C_PixelShaderConstantLockKey(); }
	void							UnlockPixelShaderConstants(const C_PixelShaderConstantLockKey lockKey) { }
	bool							CheckPixelShaderConstantsLocked(const U32 startRegister, const U32 endRegister) { return false; }

	// Vertex shader cache management
	void InvalidateVertexShaderCache();

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

	// Memory management
	U32 GetFreeResourceMemoryMemory();

	// Block until idle
	void BlockUntilIdle();

	/**************************************************************************************************
	- Public platform specific functions.
	- Not safe to use by game as these functions are not guaranteed to be cross-platform.
	- Wrap functions calls with define GRAPHICS_GCM.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// Buffer management
	const S32 GetBackBufferCount() const;
	const C_BackBuffer& GetBackBuffer() const;
	const C_BackBuffer& GetBackBuffer(const S32 backBufferIndex) const;
	const S32 GetFrontBufferCount() const;
	const C_FrontBuffer& GetFrontBuffer() const;
	const C_FrontBuffer& GetFrontBuffer(const S32 frontBufferIndex) const;
	const C_FrontBuffer& GetSwappedFrontBuffer() const;
	const C_RenderTarget& GetRenderTarget() const;

	// This is used by the automation to get the necessary information to take a screenshot with requiring GCM debug to be loaded
	const void* GetScreenshotInfo(U32* width, U32* height, U32* pitch, U32* colour, const void** bpLocation, const void** ptrToPtrToMemory) const;

	// Cg management
	CGprofile GetCGProfileVS();
	CGprofile GetCGProfilePS();
	CGcontext GetCGContext();
	
	// Memory management
	const U32 GetMemoryAlignment(const E_ResourceType type, const U32 flags);
	bool AllocResourceMemory(const E_ResourceType type, U32& flags, const U32 size, U8** ppAddress, const S32 systemID, E_AllocType allocType = E_AllocType_Default);
	void FreeResourceMemory(const E_ResourceType type, const U32 flags, const U32 size, U8* pAddress, const S32 systemID);
	bool AllocMemory(const U32 align, const U32 size, U8** ppAddress, const E_AllocHeap allocHeap, const E_AllocType allocType, const Display2::E_MemoryType memoryType = Display2::E_MemoryType_Unknown);
	void FreeMemory(const U32 size, U8* pAddress, const E_AllocHeap allocHeap, const Display2::E_MemoryType memoryType = Display2::E_MemoryType_Unknown);
	
	// Surface management
	void SetSurface();
	void SetColourSurfaces(const C_Surface* pSurface0 = NULL, const C_Surface* pSurface1 = NULL, const C_Surface* pSurface2 = NULL, const C_Surface* pSurface3 = NULL);
	void SetBackBuffer();
	void SetFrontBuffer();
	
	void Finish();

	void ResolveSurface(const C_Surface* pSurface, const C_RenderTexture2D* pTarget);
	void ResolveDepthStencilSurface(const C_Surface* pSurface, const C_RenderTexture2D* pTarget);

	// Blocker management
	U32* InsertBlocker(const U32 count);
	U32* GetBlocker(const U32 count);
	void SetBlocker(const U32* handle);

	// Fence management
	U32 InsertFence();
	U32 GetCurrentFence();
	bool IsFencePassed(const U32 value);
	void BlockOnFence(const U32 value);
	
	void WriteFence();
	volatile U32* GetFenceAddress();
	volatile U32* GetWriteFenceAddress();

	U32 InsertFence(const E_Fence eFence);
	U32 GetCurrentFence(const E_Fence eFence);
	bool IsFencePassed(const E_Fence eFence, const U32 value);
	void BlockOnFence(const E_Fence eFence, const U32 value);
	volatile U32* GetFenceAddress(const E_Fence eFence);

	#ifdef GCM_DEBUG
	INLINE void CheckForErrors();
	#endif

	C_MemoryBuffer* GetCmdBufMemory();
	C_MemoryBuffer* GetVideoMemory();
	C_MemoryBuffer* GetSystemMemory();
	C_MemoryBuffer* GetMainMemory();

	CellGcmTexture* AllocTextureHeader();
	void FreeTextureHeader(CellGcmTexture*& pgcmTexture);

	void ResizeSurfaces();
	void ResizeWindows();
	bool CreateWindows();
	void DestroyWindows();

	// Window management
	void ClearWindows();
	const S32 GetWindowCount() const;
	C_Window* GetWindow(const S32 windowIndex);
	const S32 AddWindow(const Maths::C_Rect& windowCoords);
	void SetWindow(const S32 windowIndex, const bool aa);
	void BeginWindow();
	void EndWindow();
	S32 GetWindowIndex();
	void RestoreWindow(const S32 windowIndex, const U32 colourWrite = E_ColourWriteEnable_All);

	void SetPixelShaderEmbeddedConstantF(const uint32_t dstOffset, const float* pData, const uint32_t sizeInWords, const uint32_t location);

	U32 CalculateMemorySize();

	void SetMaxBeginVerticesUsage(const U32 beginVerticesUsage);
	void SetUserMaxBeginVerticesUsage(const F32 beginVerticesUsageScale);

	static bool InSystemOSMenu();
	static void EnterSystemOSMenu();
	static void LeaveSystemOSMenu();

	void SetScissorTest(const E_Bool scissorTestEnable);
	void SetScissorRect(const U32 x, const U32 y, const U32 w, const U32 h);

	bool CreateDownsampleShaders();
	void DestroyDownsampleShaders();
	void SetupDownsample(const E_Downsample downsample, C_Texture* pSrcTexture);

	C_TextureSampler& GetTextureSampler(const U32 sampler);
	C_TextureAddress& GetTextureAddress(const U32 sampler);
	C_TextureControl& GetTextureControl(const U32 sampler);
	C_TextureFilter& GetTextureFilter(const U32 sampler);

	void SetTextureCached(const U32 sampler, const C_Texture* pTexture);
	void SetTextureAddressCached(const U32 sampler, const C_TextureAddress& textureAddress);
	void SetTextureControlCached(const U32 sampler, const C_TextureControl& textureControl);
	void SetTextureFilterCached(const U32 sampler, const C_TextureFilter& textureFilter);

	void SetTextureUncached(const U32 sampler, const CellGcmTexture* pgcmTexture);
	void SetTextureAddressUncached(const U32 sampler, const C_TextureAddress& textureAddress);
	void SetTextureControlUncached(const U32 sampler, const C_TextureControl& textureControl);
	void SetTextureFilterUncached(const U32 sampler, const C_TextureFilter& textureFilter);
	void SetTextureBorderColourUncached(const U32 sampler, const U32 borderColour);
	
	C_Texture2D *GetNullTexture();

	const U32 GetMaxAniso() const;
	void SetMaxAniso(const U32 maxAniso);

	void SetGpuGroupMarker(const U32 group);
	void SetGpuPassAndTypeMarker(const U32 pass, const U32 type);

	// Z Cull and S Cull
	void CreateZCull(U32& index, U32& start, const U32 size, const U32 offset, const U32 width, const U32 height, const E_Multisample multisample);
	void DestroyZCull(const U32 index, const U32 start);

	void ReloadZCull(const bool bSCull = true);
	void ReloadSCull();

	void DumpMemoryHeaps();

	bool IsMainThread() const;

	void CheckCanRender();

	// Control ownership of rendering between threads
	void InitialiseThreadOwnership();
	bool AcquireThreadOwnership( U32 timeMs = WAIT_TIME_INFINITE );
	void ReleaseThreadOwnership();
	bool HaveThreadOwnership();

	bool AADisabled() const;
	void EnableAA();
	void DisableAA();

	C_Surface* GetSharedColourSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat colourFormat, bool main = false);
	C_Surface* GetSharedColourSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, bool main = false);
	C_Surface* GetSharedDepthStencilSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat depthStencilFormat, bool main = false);
	C_Surface* GetSharedDepthStencilSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format depthStencilFormat, bool main = false);

	bool CreateMainSharedRenderBuffer(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, const E_Format depthStencilFormat);
	void DestroyMainSharedRenderBuffer();
	
	C_RenderBuffer* GetMainSharedRenderBuffer();
	C_Surface* GetMainSharedColourSurface();
	C_Texture* GetMainSharedColourTexture();
	C_Surface* GetMainSharedColourSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat colourFormat);
	C_Surface* GetMainSharedColourSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat);
	C_Surface* GetMainSharedDepthStencilSurface();
	C_Texture* GetMainSharedDepthStencilTexture();
	C_Surface* GetMainSharedDepthStencilSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat depthStencilFormat);
	C_Surface* GetMainSharedDepthStencilSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format depthStencilFormat);

	bool CreateLocalSharedRenderBuffer(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat, const E_Format depthStencilFormat);
	void DestroyLocalSharedRenderBuffer();
	
	C_RenderBuffer* GetLocalSharedRenderBuffer();
	C_Surface* GetLocalSharedColourSurface();
	C_Texture* GetLocalSharedColourTexture();
	C_Surface* GetLocalSharedColourSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat colourFormat);
	C_Surface* GetLocalSharedColourSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format colourFormat);
	C_Surface* GetLocalSharedDepthStencilSurface();
	C_Texture* GetLocalSharedDepthStencilTexture();
	C_Surface* GetLocalSharedDepthStencilSurface(const U32 width, const U32 height, const Display2::E_Multisample multisample, const Display2::E_SurfaceFormat depthStencilFormat);
	C_Surface* GetLocalSharedDepthStencilSurface(const U32 width, const U32 height, const E_Multisample multisample, const E_Format depthStencilFormat);

	bool IsZcullUsed() const;
	bool IsScullUsed() const;

	bool IsZcullEnabled() const;
	bool IsScullEnabled() const;
	
	void SetZcullEnable(bool zcullEnable, bool scullEnable);

	void SetInvalidateVertexCache();

	void CopyBackToFrontBuffer();

protected:

	/**************************************************************************************************
	- Protected cross-platform functions.
	- Basically anything here must be the interface.
	**************************************************************************************************/

	// Platform specific device management
	bool CreateDevice();
	bool CreateDeviceVolatile();
	void DestroyDeviceVolatile();
	void DestroyDevice();

	// Vertex declaration management
	const U32 GetVertexDeclarationCount() const;
	C_VertexDeclaration* GetVertexDeclaration(const S32 vertexElementCount, const C_VertexElement* pVertexElements);
	C_VertexDeclaration* InitVertexDeclaration(const S32 numVertexElements, const C_VertexElement* vertexElements);

	/**************************************************************************************************
	- Protected platform specific functions.
	- Basically anything here dont put in the interface.
	**************************************************************************************************/

	// TODO: Add functions here.

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

	bool CreateSurfaces();
	void DestroySurfaces();

	void WaitForFlip();

	void StartTimingFrame();
	void EndTimingFrame();

	void SwapFrontBuffers(bool flip = true);

	void CreateFlipEvent();
	void DestroyFlipEvent();

public:

	void SyncCpuGpu();

	void SetVertexDeclaration();

public:

	C_DirtyDevice						m_DirtyFlags;

	Display2::C_Device					m_Display2;

	bool								m_SetVertexDeclaration;

	C_DeviceParams						m_DeviceParams;
	
protected:

	F32									m_SlopeScaledBias;

	U32									m_MaxAniso;

	E_PresentInterval					m_PresentInterval;
	E_PresentInterval					m_PresentIntervalDefault;

	const C_VertexShader*				m_pVertexShader;
	const C_PixelShader*				m_pPixelShader;
	C_Sampler							m_Sampler[E_Sampler_Count];
	C_Sampler							m_VertexSampler[E_VertexSampler_Count];
	const C_VertexDeclaration*			m_pVertexDeclaration;
	C_Stream							m_Stream[E_Stream_Count];
	const C_IndexBuffer*				m_pIndexBuffer;
	C_DynamicVertexBuffer				m_DynamicVertexBuffer;
	U32									m_CurrentProfileVS;
	U32									m_CurrentProfilePS;
	U32									m_LatestProfileVS;
	U32									m_LatestProfilePS;
	CGprofile							m_cgLatestProfileVS;
	CGprofile							m_cgLatestProfilePS;
	CGcontext							m_cgContext;

	C_RenderTarget						m_RenderTarget;

	S32									m_CurrentBackBuffer;
	Ut::Vector<C_BackBuffer*>			m_BackBuffers;
	
	S32									m_CurrentFrontBuffer;
	S32									m_SwappedFrontBuffer;
	Ut::Vector<C_FrontBuffer*>			m_FrontBuffers;
	Display2::C_SharedTiledArea			m_FrontBufferTiledArea;

	C_MemoryBuffer						m_CmdBufMemory;
	C_MemoryBuffer						m_VideoMemory;
	C_MemoryBuffer						m_SystemMemory;

	C_VertexShaderCache					m_VertexShaderCache;

	U8*									m_pBlockerMemory;
	C_BlockerPool						m_BlockerPool;
	C_FenceInfo							m_Fence[E_Fence_Count];
	U32*								m_GpuFenceAddress;
	U32									m_GpuFenceOffset;
	U32									m_GpuFenceLocation;

	C_Texture2D							m_NullTexture;

	S32									m_CurrentWindow;
	Ut::Vector<C_Window*>				m_Windows;
	Display2::C_SharedTiledArea			m_WindowsColourTiledArea;
	Display2::C_SharedTiledArea			m_WindowsDepthTiledArea;

	bool								m_UniqueBackBuffer;

	U32									m_BeginVerticesUsage;
	U32									m_MaxBeginVerticesUsage;

	U32									m_UserBeginVerticesUsage;
	U32									m_UserMaxBeginVerticesUsage;

	bool								m_BeginVerticesLocked;

	#ifndef CFG_GOLD
	U8*									m_BeginVerticesStart;
	U32									m_BeginVerticesSize;
	#endif

private:

#ifndef CFG_GOLD
	const void*							m_ptrToPtrToFrontbufferMemory;		// Used by screenshot code in the automation
#endif

	uint32_t							m_FinishRef;

	static bool							m_InSystemOSMenu;

	C_PixelShader						m_psDownsample[E_Downsample_Count];
	C_PixelShader						m_psZCull;
	C_VertexShader						m_vsZCull;

	C_TextureSampler					m_TextureSampler[E_Sampler_Count];

	Time::C_Time 						m_CpuTimeBegin;
	Time::C_Time 						m_CpuTimeEnd;
	Time::C_Time 						m_GpuTimeBegin;
	Time::C_Time 						m_GpuTimeEnd;
	U32									m_GpuTimeIndex;

	MT::Mutex							m_ThreadOwnershipMutex;

	bool								m_DisableAA;
	
	C_RenderBuffer						m_MainSharedRenderBuffer;
	C_RenderBuffer						m_LocalSharedRenderBuffer;

	bool								m_ZcullEnable;
	bool								m_ScullEnable;
	
	bool								m_InvalidateVertexCache;
};

}; // namespace GCM
}; // namespace Display

#ifdef CFG_DEBUG
#define DONT_INLINE_GCM_DEVICE_INL
#endif

#ifndef DONT_INLINE_GCM_DEVICE_INL
#include "Display/Device/GCM/GCMDevice.inl"
#endif

#endif // DISPLAY_DEVICE_GCM_GCM_DEVICE_H
