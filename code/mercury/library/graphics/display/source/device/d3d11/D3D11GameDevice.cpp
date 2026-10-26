// [Blur1PCDX11] Native game renderer, preserving CPU-side view/state handling.
#include "stdafx.h"
#include "Display/Device/D3D11/PCD3D11GameDevice.h"
#include "Display/Timing/GPUTiming.h"
#include "Display/Memory/MemoryGraphics.h"
#include "Display/Shaders/Constants.h"
#include <system/debug/Blur2PCAutomation.h>
namespace Display {
THREAD_LOCAL_STORAGE int g_iOwnerThreadRefCount = 0;
namespace D3D11 {
C_Sampler::C_Sampler()
{
	m_pTexture				= NULL;
	m_LodBias				= 0;
	// Border colour
	m_BorderColour			= 0x00000000;
	m_BorderColourDefault	= 0x00000000;
}

C_Sampler::~C_Sampler()
{
}

C_Stream::C_Stream() :
	m_pVertexBuffer(NULL),
	m_VertexStride(0),
	m_VertexOffset(0)
{
}

C_Stream::~C_Stream()
{
}

C_D3D11GameDevice::C_D3D11GameDevice()
{
	m_BlendHighPrecisionEnable			= E_Bool_Unknown;
	m_SlopeScaledBias					= 0.f;
	m_PresentInterval					= E_PresentInterval_Unknown;
	m_PresentIntervalDefault			= E_PresentInterval_Two;
	m_ForceTextureGamma					= E_Bool_Default;
	m_pVertexDeclaration				= NULL;
	m_pIndexBuffer						= NULL;
	m_pVertexShader						= NULL;
	m_pPixelShader						= NULL;
	m_pd3dDevice						= NULL;
	m_pd3dObject						= NULL;
	m_CurrentBackBuffer					= 0;
	m_CurrentFrontBuffer				= 0;
	m_SwappedFrontBuffer				= 0;
	m_CurrentWindow						= -1;
	
	for(U32 lock = 0; lock < E_PixelShaderConstantLock_Count; ++lock)
	{
		m_PixelShaderConstantLock[lock].enabled = false;
	}

	ZeroMemory(&m_d3dPresentParams, sizeof(m_d3dPresentParams));
}

C_D3D11GameDevice::~C_D3D11GameDevice()
{
	BIZ_ASSERT(m_Windows.Size() == 0);
}

void C_D3D11GameDevice::InvalidateState()
{
	C_DeviceBase::InvalidateState();
	// [Blur1PCDX11] The next geometry submission must bind all requested inputs.
	InvalidateNativeInputBindings();

	m_BlendHighPrecisionEnable = E_Bool_Unknown;
	
	for(U32 sampler = 0; sampler < E_Sampler_Count; ++sampler)
	{
		m_Sampler[sampler].InvalidateState();
	}

	for(U32 sampler = 0; sampler < E_VertexSampler_Count; ++sampler)
	{
		m_VertexSampler[sampler].InvalidateState();
	}

	for(U32 stream = 0; stream < E_Stream_Count; ++stream)
	{
		m_Stream[stream].InvalidateState();
	}

	m_pVertexShader = 0;
	// setting a NULL pixel shader is valid in D3D so set this to something else
	m_pPixelShader = (const Display::C_PixelShader*)0xFFFFFFFF;
	m_pVertexDeclaration = 0;
	m_pIndexBuffer = 0;
}

void C_D3D11GameDevice::DefaultStates()
{
	// alpha test
	SetAlphaTestEnable(m_AlphaTestEnableDefault);
	SetAlphaTestFunc(m_AlphaTestFuncDefault);
	SetAlphaTestRef(m_AlphaTestRefDefault);
	SetAlphaToMaskEnable(m_AlphaToMaskEnableDefault);
	// blend
	SetBlendEnable(m_BlendEnableDefault);
	SetBlendSrc(m_BlendSrcDefault);
	SetBlendDst(m_BlendDstDefault);
	SetBlendOp(m_BlendOpDefault);
	SetBlendSrcAlpha(m_BlendSrcAlphaDefault);
	SetBlendDstAlpha(m_BlendDstAlphaDefault);
	SetBlendOpAlpha(m_BlendOpAlphaDefault);
	SetBlendHighPrecisionEnable(E_Bool_False);
	SetBlendMode(m_BlendModeDefault);
	// colour
	SetColourWriteEnable(m_ColourWriteEnableDefault);
	// cull
	SetCullMode(m_CullModeDefault);
	// depth
	SetDepthEnable(m_DepthEnableDefault);
	SetDepthFunc(m_DepthFuncDefault);
	SetDepthWriteEnable(m_DepthWriteEnableDefault);
	// fill
	SetFillMode(m_FillModeDefault);
	// point sprite
	SetPointSpriteEnable(m_PointSpriteEnableDefault);
	// pixel shader samplers
	for(U32 sampler = 0; sampler < E_Sampler_Count; sampler++)
	{
		SetTextureFilterMag(sampler, m_Sampler[sampler].m_FilterMagDefault);
		SetTextureFilterMin(sampler, m_Sampler[sampler].m_FilterMinDefault);
		SetTextureFilterMip(sampler, m_Sampler[sampler].m_FilterMipDefault);
		SetTextureAddressU(sampler, m_Sampler[sampler].m_AddressUDefault);
		SetTextureAddressV(sampler, m_Sampler[sampler].m_AddressVDefault);
		SetTextureAddressW(sampler, m_Sampler[sampler].m_AddressWDefault);
		SetTextureBorderColour(sampler, m_Sampler[sampler].m_BorderColourDefault);
		SetTextureGamma(sampler, m_Sampler[sampler].m_GammaDefault);
	}
	// vertex shader samplers
	for(U32 sampler = 0; sampler < E_VertexSampler_Count; sampler++)
	{
		SetVertexTextureFilterMag(sampler, m_VertexSampler[sampler].m_FilterMagDefault);
		SetVertexTextureFilterMin(sampler, m_VertexSampler[sampler].m_FilterMinDefault);
		SetVertexTextureFilterMip(sampler, m_VertexSampler[sampler].m_FilterMipDefault);
		SetVertexTextureAddressU(sampler, m_VertexSampler[sampler].m_AddressUDefault);
		SetVertexTextureAddressV(sampler, m_VertexSampler[sampler].m_AddressVDefault);
		SetVertexTextureAddressW(sampler, m_VertexSampler[sampler].m_AddressWDefault);
		SetVertexTextureBorderColour(sampler, m_VertexSampler[sampler].m_BorderColourDefault);
		SetVertexTextureGamma(sampler, m_VertexSampler[sampler].m_GammaDefault);
	}
}

void C_D3D11GameDevice::SetupViewIndex(const S32 viewIndex, const C_ViewSetup& viewSetup)
{
	BIZ_ASSERT(viewIndex >= 0 && viewIndex < m_Views.Size());
	m_Views[viewIndex]->Setup(viewSetup, m_RenderTarget.GetColourWidth(), m_RenderTarget.GetColourHeight());
}

void C_D3D11GameDevice::SetupViewIndex(const S32 viewIndex, const C_ViewSetup& viewSetup, const U32 width, const U32 height)
{
	BIZ_ASSERT(viewIndex >= 0 && viewIndex < m_Views.Size());
	m_Views[viewIndex]->Setup(viewSetup, width, height);
}

void C_D3D11GameDevice::SetViewIndex(const S32 viewIndex, const bool setStates)
{
	if(viewIndex >= 0 && viewIndex < m_Views.Size())
	{
		SetView(m_Views[viewIndex], setStates);
	}
	else
	{
		SetView(NULL, setStates);
	}
}

void C_D3D11GameDevice::SetupView(C_View* pView, const C_ViewSetup& viewSetup)
{
	BIZ_ASSERT(pView);
	pView->Setup(viewSetup, m_RenderTarget.GetColourWidth(), m_RenderTarget.GetColourHeight());
}

void C_D3D11GameDevice::SetupView(C_View* pView, const C_ViewSetup& viewSetup, const U32 width, const U32 height)
{
	BIZ_ASSERT(pView);
	pView->Setup(viewSetup, width, height);
}

void C_D3D11GameDevice::SetView(const C_View* pView, const bool setStates)
{
	BIZ_ASSERT(m_pd3dDevice);

	m_pView = pView;
	
	if(m_pView)
	{
		if(setStates)
		{
			m_Viewport = m_pView->GetViewport();
			
			SetViewport(m_Viewport);
			SetDepthFunc(E_CmpFunc_Default);

			SetViewShaderConstants2D();
			SetViewShaderConstants3D();
		}
	}
	else
	{
		if(setStates)
		{
			SetViewShaderConstants2D();
		}
	}
}

void C_D3D11GameDevice::SetViewShaderConstants()
{
	SetViewShaderConstants2D();
	SetViewShaderConstants3D();
}

void C_D3D11GameDevice::SetViewShaderConstants2D()
{
	if(m_pView)
	{
		const C_ViewSettings2D& viewSettings2D = m_pView->GetViewSettings2D();
		SetVertexShaderConstantF(CV_DISPLAY_CLIP_VECTOR, (F32*)&viewSettings2D.GetClipTransform(), 1);
	}
	else
	{
		const C_Float4 clipTransform(0.001953125f, -0.00260416674f, -1.f, 1.f);
		SetVertexShaderConstantF(CV_DISPLAY_CLIP_VECTOR, (F32*)&clipTransform, 1);
	}
}

void C_D3D11GameDevice::SetViewShaderConstants3D()
{
	if(m_pView)
	{
		const C_ViewSettings3D& viewSettings3D = m_pView->GetViewSettings3D();
		SetVertexShaderConstantF(CV_DISPLAY_VIEW_MATRIX, (F32*)&viewSettings3D.GetViewMatrix(), 4);
		SetVertexShaderConstantF(CV_DISPLAY_PROJ_MATRIX, (F32*)&viewSettings3D.GetProjMatrix(), 4);
		SetVertexShaderConstantF(CV_DISPLAY_VIEWPROJ_MATRIX, (F32*)&viewSettings3D.GetViewProjMatrix(), 4);
		SetVertexShaderConstantF(CV_DISPLAY_VIEW_POSITION, (F32*)&viewSettings3D.GetViewPosition(), 1);
		SetPixelShaderConstantF(CP_DISPLAY_VIEW_POSITION, (F32*)&viewSettings3D.GetViewPosition(), 1);
	}
}

void C_D3D11GameDevice::InitialiseBuffers()
{
    BIZ_ASSERT(m_BackBuffers.Size() > 0);
    m_RenderTarget.m_pColourSurface = &m_BackBuffers[0]->m_ColourSurface;
    m_RenderTarget.m_pDepthStencilSurface = &m_BackBuffers[0]->m_DepthStencilSurface;
}

C_PixelShaderConstantLockKey C_D3D11GameDevice::LockPixelShaderConstants(const U32 startRegister, const U32 endRegister)
{
	C_PixelShaderConstantLockKey lockKey;
	lockKey.index = 0xFFFFFFFF;
		
	if(!CheckPixelShaderConstantsLocked(startRegister, endRegister))
	{
		for(U32 i = 0; i < E_PixelShaderConstantLock_Count; ++i)
		{
			C_PixelShaderConstantLock& lock = m_PixelShaderConstantLock[i];
			if(!lock.enabled)
			{
				lock.enabled = true;
				lock.salt++;
				lock.startRegister = startRegister;
				lock.endRegister = endRegister;

				lockKey.index = i;
				lockKey.salt = lock.salt;

				return lockKey;
			}
		}
	}
	else
	{
		LOGWARN(g_Log, "Shader constants %i to %i are already locked\n", startRegister, endRegister);
		return lockKey;
	}

	LOGWARN(g_Log, "No more shader constant locks available\n");
	return lockKey;	
}

void C_D3D11GameDevice::UnlockPixelShaderConstants(const C_PixelShaderConstantLockKey lockKey)
{
	if(lockKey.index != 0xFFFFFFFF && lockKey.index < E_PixelShaderConstantLock_Count)
	{
		C_PixelShaderConstantLock& lock = m_PixelShaderConstantLock[lockKey.index];
		if(lock.enabled && lock.salt == lockKey.salt)
		{
			lock.enabled = false;
			return;
		}
	}
	
	LOGINFO(g_Log, "Invalid PixelShaderConstantLockKey\n");
}

const S32 C_D3D11GameDevice::AddWindow(const Maths::C_Rect& windowCoords)
{
	C_Window* pWindow = new C_Window(windowCoords);
	m_Windows.PushBack(pWindow);
	return m_Windows.Size()-1;
}

const E_Bool C_D3D11GameDevice::GetForceTextureGamma() const
{ return m_ForceTextureGamma; }

void C_D3D11GameDevice::SetForceTextureGamma(const E_Bool forceTextureGamma)
{
	BIZ_ASSERT(forceTextureGamma == E_Bool_False || forceTextureGamma == E_Bool_True || forceTextureGamma == E_Bool_Default);
	m_ForceTextureGamma = forceTextureGamma;
}

void C_D3D11GameDevice::InitialiseThreadOwnership()
{
	// different case for init as the main thread already has ownership of device
	g_iOwnerThreadRefCount = 1;
	m_ThreadOwnershipMutex.Wait();
}

bool C_D3D11GameDevice::HaveThreadOwnership()
{
	return (g_iOwnerThreadRefCount > 0);
}

bool C_D3D11GameDevice::AcquireThreadOwnership( U32 timeMs )
{
	if (m_ThreadOwnershipMutex.Wait(timeMs))
	{
		if (g_iOwnerThreadRefCount++ == 0)
		{
		}

		return true;
	}

	return false;
}

void C_D3D11GameDevice::ReleaseThreadOwnership()
{
	// Check this thread has ownership before we allow it to release
	BIZ_ASSERT(g_iOwnerThreadRefCount > 0);

	if (--g_iOwnerThreadRefCount == 0)
	{
	}

	m_ThreadOwnershipMutex.Release();
}

// [Blur1PCDX11] Resource ownership delegates to the native Display2 D3D11 device.
bool C_D3D11GameDevice::CreateVertexBuffer(U32 size, U32 flags, C_VertexBuffer* buffer, bool allowFail)
{ return m_Display2.CreateVertexBuffer(size, flags, buffer); }
void C_D3D11GameDevice::DestroyVertexBuffer(C_VertexBuffer* buffer)
{ m_Display2.DestroyVertexBuffer(buffer); }
bool C_D3D11GameDevice::CreateIndexBuffer(U32 size, U32 flags, E_Format format, C_IndexBuffer* buffer, bool allowFail)
{ return m_Display2.CreateIndexBuffer(size, flags,
    format == E_Format_Index32 ? Display2::E_IndexFormat_32 : Display2::E_IndexFormat_16, buffer); }
void C_D3D11GameDevice::DestroyIndexBuffer(C_IndexBuffer* buffer)
{ m_Display2.DestroyIndexBuffer(buffer); }
bool C_D3D11GameDevice::CreateTexture2D(U32 width, U32 height, U32 levels, U32 flags, E_Format format,
                                        C_Texture2D* texture, U8* address, U32 addressSize)
{ return m_Display2.CreateTexture2D(width, height, levels, flags,
    static_cast<Display2::E_TextureFormat>(format), texture, address, addressSize); }
void C_D3D11GameDevice::DestroyTexture2D(C_Texture2D* texture)
{ m_Display2.DestroyTexture(texture); }
bool C_D3D11GameDevice::CreateTexture3D(U32 width, U32 height, U32 depth, U32 levels, U32 flags,
                                        E_Format format, C_Texture3D* texture, U8* address, U32 addressSize)
{ return m_Display2.CreateTexture3D(width, height, depth, levels, flags,
    static_cast<Display2::E_TextureFormat>(format), texture, address, addressSize); }
void C_D3D11GameDevice::DestroyTexture3D(C_Texture3D* texture)
{ m_Display2.DestroyTexture(texture); }
bool C_D3D11GameDevice::CreateTextureCube(U32 width, U32 levels, U32 flags, E_Format format,
                                          C_TextureCube* texture, U8* address, U32 addressSize)
{ return m_Display2.CreateTextureCube(width, levels, flags,
    static_cast<Display2::E_TextureFormat>(format), texture, address, addressSize); }
void C_D3D11GameDevice::DestroyTextureCube(C_TextureCube* texture)
{ m_Display2.DestroyTexture(texture); }
bool C_D3D11GameDevice::CreateColourSurface(U32 width, U32 height, U32 flags, E_Format format,
                                            E_Multisample samples, const C_SurfaceParams* params, C_Surface* surface)
{ return m_Display2.CreateColourSurface(width, height, flags,
    static_cast<Display2::E_SurfaceFormat>(format), static_cast<Display2::E_Multisample>(samples), params, surface); }
bool C_D3D11GameDevice::CreateDepthStencilSurface(U32 width, U32 height, U32 flags, E_Format format,
                                                  E_Multisample samples, const C_SurfaceParams* params, C_Surface* surface)
{ return m_Display2.CreateDepthStencilSurface(width, height, flags,
    static_cast<Display2::E_SurfaceFormat>(format), static_cast<Display2::E_Multisample>(samples), params, surface); }
void C_D3D11GameDevice::DestroyColourSurface(C_Surface* surface)
{ m_Display2.DestroySurface(surface); }
void C_D3D11GameDevice::DestroyDepthStencilSurface(C_Surface* surface)
{ m_Display2.DestroySurface(surface); }
void C_D3D11GameDevice::DestroySurface(C_Surface* surface)
{ m_Display2.DestroySurface(surface); }
bool C_D3D11GameDevice::CreateColourSurface(U32 width, U32 height, U32 flags, D3DFORMAT format,
                                            D3DMULTISAMPLE_TYPE samples, const C_SurfaceParams* params, C_Surface* surface)
{ return CreateColourSurface(width, height, flags, static_cast<E_Format>(format),
                             static_cast<E_Multisample>(samples), params, surface); }
bool C_D3D11GameDevice::CreateDepthStencilSurface(U32 width, U32 height, U32 flags, D3DFORMAT format,
                                                  D3DMULTISAMPLE_TYPE samples, const C_SurfaceParams* params, C_Surface* surface)
{ return CreateDepthStencilSurface(width, height, flags, static_cast<E_Format>(format),
                                   static_cast<E_Multisample>(samples), params, surface); }
void C_D3D11GameDevice::DestroyVertexShader(C_VertexShader* shader)
{ m_Display2.DestroyVertexShader(shader); }
void C_D3D11GameDevice::DestroyPixelShader(C_PixelShader* shader)
{ m_Display2.DestroyPixelShader(shader); }
C_VertexDeclaration* C_D3D11GameDevice::InitVertexDeclaration(S32 count, const C_VertexElement* elements)
{ return m_Display2.CreateVertexDeclaration(elements); }
const U32 C_D3D11GameDevice::GetVertexDeclarationCount() const
{ return m_Display2.GetVertexDeclarationCount(); }
C_VertexDeclaration* C_D3D11GameDevice::GetVertexDeclaration(S32 count, const C_VertexElement* elements)
{ return m_Display2.CreateVertexDeclaration(elements); }
void C_D3D11GameDevice::SetBlendMode(const E_BlendMode blendMode)
{
	if(m_BlendMode != blendMode)
    {
        switch(blendMode)
        {
			case E_BlendMode_Solid:
			{
				SetAlphaTestEnable(E_Bool_False);

				SetBlendEnable(E_Bool_False);

				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_All);
			
			}
			break;

			case E_BlendMode_SolidAlphaTest:
			{
				SetAlphaTestEnable(E_Bool_True);
				SetAlphaTestFunc(E_CmpFunc_Greater);
				SetAlphaTestRef(0x80);

				SetBlendEnable(E_Bool_False);

				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_Alpha:
			{
				SetAlphaTestEnable(E_Bool_True);
				SetAlphaTestFunc(E_CmpFunc_Greater);
				SetAlphaTestRef(0x00);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_SrcAlpha);
				SetBlendDst(E_Blend_InvSrcAlpha);
				SetBlendOp(E_BlendOp_Add);
				SetBlendSrcAlpha(E_Blend_SrcAlpha);
				SetBlendDstAlpha(E_Blend_InvSrcAlpha);
				SetBlendOpAlpha(E_BlendOp_Add);

				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_AlphaPreMultiply:
			{
				SetAlphaTestEnable(E_Bool_False);
				SetAlphaTestFunc(E_CmpFunc_Greater);
				SetAlphaTestRef(0x00);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_One);
				SetBlendDst(E_Blend_InvSrcAlpha);
				SetBlendOp(E_BlendOp_Add);
				SetBlendSrcAlpha(E_Blend_One);
				SetBlendDstAlpha(E_Blend_InvSrcAlpha);
				SetBlendOpAlpha(E_BlendOp_Add);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);

			}
			break;

			case E_BlendMode_Additive:
			{
				SetAlphaTestEnable(E_Bool_True);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_One);
				SetBlendDst(E_Blend_One);
				SetBlendOp(E_BlendOp_Add);
				SetBlendSrcAlpha(E_Blend_One);
				SetBlendDstAlpha(E_Blend_One);
				SetBlendOpAlpha(E_BlendOp_Add);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_AlphaAdditive:
			{
        		SetAlphaTestEnable(E_Bool_True);
				SetAlphaTestFunc(E_CmpFunc_Greater);
				SetAlphaTestRef(0x00);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_SrcAlpha);
				SetBlendDst(E_Blend_One);
				SetBlendOp(E_BlendOp_Add);
				SetBlendSrcAlpha(E_Blend_SrcAlpha);
				SetBlendDstAlpha(E_Blend_One);
				SetBlendOpAlpha(E_BlendOp_Add);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_AlphaSubtractive:
			{
        		SetAlphaTestEnable(E_Bool_True);
				SetAlphaTestFunc(E_CmpFunc_Greater);
				SetAlphaTestRef(0x00);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_SrcAlpha);
				SetBlendDst(E_Blend_One);
				SetBlendOp(E_BlendOp_RevSubtract);
				SetBlendSrcAlpha(E_Blend_SrcAlpha);
				SetBlendDstAlpha(E_Blend_One);
				SetBlendOpAlpha(E_BlendOp_RevSubtract);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_Subtractive:
			{
				SetAlphaTestEnable(E_Bool_False);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_One);
				SetBlendDst(E_Blend_One);
				SetBlendOp(E_BlendOp_RevSubtract);
				SetBlendSrcAlpha(E_Blend_One);
				SetBlendDstAlpha(E_Blend_One);
				SetBlendOpAlpha(E_BlendOp_RevSubtract);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_Multiply:
			{
				SetAlphaTestEnable(E_Bool_False);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_Zero);
				SetBlendDst(E_Blend_SrcColour);
				SetBlendOp(E_BlendOp_Add);
				SetBlendSrcAlpha(E_Blend_Zero);
				SetBlendDstAlpha(E_Blend_SrcColour);
				SetBlendOpAlpha(E_BlendOp_Add);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_AdditiveSmooth:
			{
				SetAlphaTestEnable(E_Bool_True);
				
				SetBlendEnable(E_Bool_True);
				SetBlendSrc(E_Blend_One);
				SetBlendDst(E_Blend_InvSrcColour);
				SetBlendOp(E_BlendOp_Add);
				SetBlendSrcAlpha(E_Blend_One);
				SetBlendDstAlpha(E_Blend_InvSrcColour);
				SetBlendOpAlpha(E_BlendOp_Add);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
			
			}
			break;

			case E_BlendMode_Unknown:
				break;

			default:
			{
				SetAlphaTestEnable(E_Bool_False);
				
				SetBlendEnable(E_Bool_False);
				
				if(m_LuminanceWrite == E_LuminanceWrite_Enabled)
					SetColourWriteEnable(E_ColourWriteEnable_Red|E_ColourWriteEnable_Green|E_ColourWriteEnable_Blue);
				
			}
			break;
        }

        m_BlendMode = blendMode;
    }
}


void C_Sampler::InvalidateState()
{ C_SamplerBase::InvalidateState(); m_pTexture = nullptr; m_BorderColour = 0; }
void C_Stream::InvalidateState() { m_pVertexBuffer = nullptr; }
ID3D11Device* C_D3D11GameDevice::GetD3DDevice() { return m_pd3dDevice; }
IDXGIFactory1* C_D3D11GameDevice::GetD3DObject() { return m_pd3dObject; }
DXGI_SWAP_CHAIN_DESC& C_D3D11GameDevice::GetD3DPresentParams() { return m_d3dPresentParams; }
const S32 C_D3D11GameDevice::GetBackBufferCount() const { return m_BackBuffers.Size(); }
const C_BackBuffer& C_D3D11GameDevice::GetBackBuffer() const { return *m_BackBuffers[m_CurrentBackBuffer]; }
const C_BackBuffer& C_D3D11GameDevice::GetBackBuffer(S32 index) const { return *m_BackBuffers[index]; }
const S32 C_D3D11GameDevice::GetFrontBufferCount() const { return m_FrontBuffers.Size(); }
const C_FrontBuffer& C_D3D11GameDevice::GetFrontBuffer() const { return *m_FrontBuffers[m_CurrentFrontBuffer]; }
const C_FrontBuffer& C_D3D11GameDevice::GetFrontBuffer(S32 index) const { return *m_FrontBuffers[index]; }
const C_FrontBuffer& C_D3D11GameDevice::GetSwappedFrontBuffer() const { return *m_FrontBuffers[m_SwappedFrontBuffer]; }
const C_RenderTarget& C_D3D11GameDevice::GetRenderTarget() const { return m_RenderTarget; }
const S32 C_D3D11GameDevice::GetWindowCount() const { return m_Windows.Size(); }
C_Window* C_D3D11GameDevice::GetWindow(S32 index) { return m_Windows[index]; }
// [Blur2PC] As C_D3D9Device::BeginScene/EndScene: both reset the device, so each frame starts from the
// default states with every setter cache invalidated. Display2 contexts (the UI) change blend state through
// the plain setters and leave the cached E_BlendMode behind; without the reset, the AlphaPreMultiply the
// front-end composite left cached turned the next frame's SetBlendMode(AlphaPreMultiply) into a no-op, and
// the blob shadow texture drew with blending off (T249). Non-Master builds hid it: C_DebugLayer invalidates.
bool C_D3D11GameDevice::BeginScene()
{
	m_pView = NULL;
	m_InScene = true;
	SetColourSurface();
	SetDepthStencilSurface();
	Reset();
	return true;
}
void C_D3D11GameDevice::EndScene()
{
	m_pView = NULL;
	Reset();
	m_InScene = false;
}
void C_D3D11GameDevice::Reset()
{
	InvalidateState();
	DefaultStates();
	Unbind();
	SetView(m_pView);
}
// [Blur1PCDX11] As C_D3D9Device::Unbind: shaders, declaration, streams, indices and textures only.
// Render targets, viewport and render states stay bound (ClearState would drop them).
void C_D3D11GameDevice::Unbind()
{
    SetVertexShader(NULL);
    SetPixelShader(NULL);
    m_pVertexDeclaration = NULL;
    for (U32 stream = 0; stream < E_Stream_Count; stream++)
        SetVertexBuffer(stream, NULL, 0, 0);
    SetIndexBuffer(NULL);
    for (U32 vertexSampler = 0; vertexSampler < E_VertexSampler_Count; vertexSampler++)
        SetVertexTexture(vertexSampler, NULL);
    for (U32 sampler = 0; sampler < E_Sampler_Count; sampler++)
        SetTexture(sampler, NULL);
}
bool C_D3D11GameDevice::CheckPixelShaderConstantsLocked(U32 start, U32 end)
{
    for (U32 i = 0; i < E_PixelShaderConstantLock_Count; ++i)
        if (m_PixelShaderConstantLock[i].enabled && start <= m_PixelShaderConstantLock[i].endRegister &&
            end >= m_PixelShaderConstantLock[i].startRegister) return true;
    return false;
}
bool C_D3D11GameDevice::CreateDevice()
{ InitialiseThreadOwnership(); return true; }
C_DeviceParams::C_DeviceParams() : m_pd3dDevice(nullptr), m_pd3dObject(nullptr),
    m_Width(640), m_Height(480), m_Multisample(E_Multisample_None),
    m_ColourFormat(E_Format_A8R8G8B8), m_DepthStencilFormat(E_Format_D24S8),
    m_RefreshRate(0), m_Windowed(true), m_CmdShow(1), m_pName(nullptr),
    m_hDC(nullptr), m_hWnd(nullptr), m_hInstance(nullptr), m_UseREF(false),
    m_BeginVerticesSize(1024*256 + 1024*1024*2), m_FreeBeginVerticesSize(1024*256),
    m_PresentInterval(E_PresentInterval_One), m_WindowBufferWidth(640),
    m_WindowBufferHeight(480), m_WindowBufferColourFormat(E_Format_A8R8G8B8),
    m_WindowBufferDepthStencilFormat(E_Format_D24S8), m_WindowBufferMultisample(E_Multisample_2),
    m_ScreenWidth(640), m_ScreenHeight(480) {}
C_DeviceParams::~C_DeviceParams() {}
// [Blur1PCDX11] As D3D9DeviceParams.cpp: Setup assigns only on Xenon. On PC the sizes and window
// multisample come from LoadSettings/SetDisplaySize before C_ApplicationBase::SetupDeviceParams calls
// this with its 1280x720 defaults; assigning them here discarded the configured resolution and MSAA.
void C_DeviceParams::Setup(U32,U32,U32,U32,E_Multisample,U32,U32,E_Multisample) {}

namespace { bool IsDXBC(const S32* data,U32 size)
{ return data && size>=4 && *reinterpret_cast<const U32*>(data)==0x43425844; } }
// The generated registry matches each old built-in blob to its SM5 compilation.
// The D3D9 bytecode is never submitted to the D3D11 device.
const uint8_t* FindBuiltinShader(const void* legacy,size_t* bytes);
bool C_D3D11GameDevice::CreateVertexShader(U32 size,const C_ShaderData& data,U32 flags,C_VertexShader* shader)
{
    size_t bytes=0;const uint8_t* native=FindBuiltinShader(&data,&bytes);
    if(!native || !bytes)
    {
        static unsigned misses = 0;
        if (misses++ < 12) Blur2PCAutomation::LogF("DX11 built-in VS missing %p size=%u", &data, size);
        return false;
    }
    return m_Display2.CreateVertexShader(static_cast<U32>(bytes),native,flags,shader);
}
bool C_D3D11GameDevice::CreatePixelShader(U32 size,const C_ShaderData& data,U32 flags,C_PixelShader* shader)
{
    size_t bytes=0;const uint8_t* native=FindBuiltinShader(&data,&bytes);
    if(!native || !bytes)
    {
        static unsigned misses = 0;
        if (misses++ < 12) Blur2PCAutomation::LogF("DX11 built-in PS missing %p size=%u", &data, size);
        return false;
    }
    return m_Display2.CreatePixelShader(static_cast<U32>(bytes),native,flags,shader);
}
bool C_D3D11GameDevice::CreateVertexShader(U32 size,const S32* data,U32 flags,C_VertexShader* shader)
{ return IsDXBC(data,size) && m_Display2.CreateVertexShader(size,reinterpret_cast<const U8*>(data),flags,shader); }
bool C_D3D11GameDevice::CreatePixelShader(U32 size,const S32* data,U32 flags,C_PixelShader* shader)
{ return IsDXBC(data,size) && m_Display2.CreatePixelShader(size,reinterpret_cast<const U8*>(data),flags,shader); }


bool C_D3D11GameDevice::CreateRenderTextureCube(U32 width,U32 levels,U32 flags,E_Format format,
    E_Multisample samples,const C_SurfaceParams* params,C_RenderTextureCube* target,U8* address,U32 addressSize)
{
    if(!target || target->IsCreated() || samples!=E_Multisample_None)return false;
    flags|=E_Usage_Colour;
    if(!m_Display2.CreateTextureCube(width,levels,flags,static_cast<Display2::E_TextureFormat>(format),
        &target->m_Texture,address,addressSize))return false;
    target->m_Flags=flags;
    for(U32 face=0;face<6;++face)
    {
        target->m_Surfaces[face].Resize(levels);
        for(U32 level=0;level<levels;++level)
        {
            C_Surface& surface=target->m_Surfaces[face][level];
            surface.m_pd3dSurface=target->m_Texture.GetGPUTextureCube();
            surface.m_Subresource=face*levels+level;
            surface.m_pd3dSurface->AddRef();
        }
    }
    return true;
}
void C_D3D11GameDevice::DestroyRenderTextureCube(C_RenderTextureCube* target)
{
    if(!target)return;
    for(U32 face=0;face<6;++face)
    {for(S32 level=0;level<target->m_Surfaces[face].Size();++level)
        m_Display2.DestroySurface(&target->m_Surfaces[face][level]);
     target->m_Surfaces[face].Clear();}
    m_Display2.DestroyTexture(&target->m_Texture);target->m_Flags=0;
}
} }
