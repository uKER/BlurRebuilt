#include "stdafx.h"
#include <core\containers\sortedbinarytree.h>
#include "Display/Device/DeviceBase.h"

#ifdef CFG_GOLD
//#define DISPLAY_DEBUG_RENDERER 1
#endif

namespace Display
{

E_CpuSpeed C_DeviceBase::m_CpuSpeed	= E_CpuSpeed_High;
E_GpuSpeed C_DeviceBase::m_GpuSpeed	= E_GpuSpeed_High;

F32	g_AspectRatio[E_AspectRatio_Count] = { 4.f/3.f, 5.f/4.f, 16.f/9.f, 16.f/10.f };

// C_DeviceMutex

C_DeviceMutex::C_DeviceMutex(C_DeviceBase* pDeviceBase) :
	m_pDeviceBase(pDeviceBase)
{
	assert(m_pDeviceBase);
	m_pDeviceBase->MutexWait();
}

C_DeviceMutex::~C_DeviceMutex()
{
	assert(m_pDeviceBase);
	m_pDeviceBase->MutexRelease();
}

// C_SamplerBase

C_SamplerBase::C_SamplerBase()
{
	// Texture address
    m_AddressU			= E_TextureAddress_Unknown;
	m_AddressUDefault	= E_TextureAddress_Wrap;
	m_AddressV			= E_TextureAddress_Unknown;
	m_AddressVDefault	= E_TextureAddress_Wrap;
	m_AddressW			= E_TextureAddress_Unknown;
	m_AddressWDefault	= E_TextureAddress_Wrap;
	// Texture filer
	m_FilterMag			= E_TextureFilter_Unknown;
	m_FilterMagDefault	= E_TextureFilter_Linear;
	m_FilterMin			= E_TextureFilter_Unknown;
	m_FilterMinDefault	= E_TextureFilter_Linear;
	m_FilterMip			= E_TextureFilter_Unknown;
	m_FilterMipDefault	= E_TextureFilter_None;
	// Texture gamma
	m_Gamma				= E_Bool_Unknown;
	m_GammaDefault		= E_Bool_False;
}

C_SamplerBase::~C_SamplerBase()
{
}

// C_DeviceBase

C_DeviceBase::C_DeviceBase() :
	m_Created(false),
	m_InScene(false),
	m_InParallel(false),
	m_CpuTime(0.0f),
	m_GpuTime(0.0f),
	m_pView(NULL)
{
	m_HiDef							= false;
	m_WideScreen					= false;
	m_RefreshRate					= E_RefreshRate_60HZ;
	m_AspectRatio					= E_AspectRatio_16_9;
#ifdef PLATFORM_PC
	m_PCAspectRatio					= 16.f/9.f;	// [Blur2PC] retail 0x005a4f90
#endif
	m_ShaderProfile					= E_ShaderProfile_3_0;
	m_VertexShaderProfile			= E_VertexShaderProfile_Unknown;
	m_PixelShaderProfile			= E_PixelShaderProfile_Unknown;
	// alpha test
	m_AlphaTestEnable				= E_Bool_Unknown;
	m_AlphaTestEnableDefault		= E_Bool_False;
	m_AlphaTestFunc					= E_CmpFunc_Unknown;
	m_AlphaTestFuncDefault			= E_CmpFunc_Always;
	m_AlphaTestRef					= UINT_MAX;
	m_AlphaTestRefDefault			= 0;

	m_AlphaToMaskEnable				= E_Bool_Unknown;
	m_AlphaToMaskEnableDefault		= E_Bool_False;

	// blend
	m_BlendEnable					= E_Bool_Unknown;
	m_BlendEnableDefault			= E_Bool_False;
	m_BlendSrc						= E_Blend_Unknown;
	m_BlendSrcDefault				= E_Blend_One;
	m_BlendSrcAlpha					= E_Blend_Unknown;
	m_BlendSrcAlphaDefault			= E_Blend_One;
	m_BlendDst						= E_Blend_Unknown;
	m_BlendDstDefault				= E_Blend_Zero;
	m_BlendDstAlpha					= E_Blend_Unknown;
	m_BlendDstAlphaDefault			= E_Blend_Zero;
	m_BlendOp						= E_BlendOp_Unknown;
	m_BlendOpDefault				= E_BlendOp_Add;
	m_BlendOpAlpha					= E_BlendOp_Unknown;
	m_BlendOpAlphaDefault			= E_BlendOp_Add;
	m_BlendMode						= E_BlendMode_Unknown;
	m_BlendModeDefault				= E_BlendMode_Solid;
	// colour
	m_ColourClear					= Maths::RGBA(0, 0, 0, 0);
	m_ColourWriteEnable				= 0;
	m_ColourWriteEnableDefault		= E_ColourWriteEnable_Red | E_ColourWriteEnable_Green | E_ColourWriteEnable_Blue;
	// cull
	m_CullMode						= E_CullMode_Unknown;
	m_CullModeDefault				= E_CullMode_Ccw;
	// depth
	m_DepthBias						= 0.f;
	m_DepthClear					= 1.f;
	m_DepthCmpFlip					= false;
	m_DepthViewportFlip				= false;
	m_DepthEnable					= E_Bool_Unknown;
	m_DepthEnableDefault			= E_Bool_True;
	m_DepthFunc						= E_CmpFunc_Unknown;
	m_DepthFuncDefault				= E_CmpFunc_LessEqual;
	m_DepthWriteEnable				= E_Bool_Unknown;
	m_DepthWriteEnableDefault		= E_Bool_True;
	// fill
	m_FillMode						= E_FillMode_Unknown;
	m_FillModeDefault				= E_FillMode_Solid;
	// point sprite
	m_PointSpriteEnable				= E_Bool_Unknown;
	m_PointSpriteEnableDefault		= E_Bool_False;
	// luminance
	m_LuminanceWrite				= E_LuminanceWrite_Unknown;
	m_LuminanceWriteDefault			= E_LuminanceWrite_Disabled;
	// stencil
	m_StencilClear					= 0;
	m_StencilEnable					= E_Bool_Unknown;
	m_StencilEnableDefault			= E_Bool_False;
	// ref counting
	m_RefCountSurfaces				= 0;
	m_RefCountTextures				= 0;
	m_RefCountCubeTextures			= 0;
	m_RefCountVertexDeclarations	= 0;
	m_RefCountVertexBuffers			= 0;
	m_RefCountIndexBuffers			= 0;
	m_RefCountVertexShaders			= 0;
	m_RefCountPixelShaders			= 0;
}

C_DeviceBase::~C_DeviceBase()
{
	assert(!m_Created && "The device must be destroyed before it is deleted");

	AssertEverythingHasBeenDestroyed();

	ClearViews();
}

void C_DeviceBase::AssertEverythingHasBeenDestroyed()
{
	assert(m_RefCountSurfaces				== 0 && "There are still created surfaces in the system");
	assert(m_RefCountTextures				== 0 && "There are still created textures in the system");
	assert(m_RefCountCubeTextures			== 0 && "There are still created cube textures in the system");
	//assert(m_RefCountVertexDeclarations		== 0 && "There are still created vertex declarations in the system");
	assert(m_RefCountVertexBuffers			== 0 && "There are still created vertex buffers in the system");
	assert(m_RefCountIndexBuffers			== 0 && "There are still created index buffers in the system");
	assert(m_RefCountVertexShaders			== 0 && "There are still created vertex shaders in the system");
	assert(m_RefCountPixelShaders			== 0 && "There are still created pixel shaders in the system");
	//assert(m_Renderers.Size()				== 0 && "There are still created renderers in the system");
}

void C_DeviceBase::InvalidateState()
{
	// alpha
	m_AlphaTestEnable		= E_Bool_Unknown;
	m_AlphaTestFunc			= E_CmpFunc_Unknown;
	#ifdef GRAPHICS_GCM
	m_AlphaTestRef			= 255;
	#else
	m_AlphaTestRef			= UINT_MAX;
	#endif

	m_AlphaToMaskEnable		= E_Bool_Unknown;

	// blend
	m_BlendEnable			= E_Bool_Unknown;
	m_BlendSrc				= E_Blend_Unknown;
	m_BlendDst				= E_Blend_Unknown;
	m_BlendOp				= E_BlendOp_Unknown;
	m_BlendSrcAlpha			= E_Blend_Unknown;
	m_BlendDstAlpha			= E_Blend_Unknown;
	m_BlendOpAlpha			= E_BlendOp_Unknown;
	m_BlendMode				= E_BlendMode_Unknown;
	// colour
	m_ColourWriteEnable		= 0;
	// cull
	m_CullMode				= E_CullMode_Unknown;
	m_CullModeFlip			= false;
	// depth
	m_DepthEnable			= E_Bool_Unknown;
	m_DepthFunc				= E_CmpFunc_Unknown;
	m_DepthWriteEnable		= E_Bool_Unknown;
	// fill
	m_FillMode				= E_FillMode_Unknown;
	// luminance
	m_LuminanceWrite		= E_LuminanceWrite_Unknown;
	// stencil
	m_StencilEnable			= E_Bool_Unknown;
}

void C_DeviceBase::ClearViews()
{
	m_ViewMutex.Wait();

	for(S32 i=0; i<m_Views.Size(); i++)
	{
		delete m_Views[i];
	}
	
	m_Views.Destroy();

	m_ViewMutex.Release();
}

const S32 C_DeviceBase::AddView(const U32 type, const S32 userIndex)
{
	m_ViewMutex.Wait();

	C_View* pView = new C_View(m_Views.Size(), type, userIndex);

	m_Views.PushBack(pView);

	m_ViewMutex.Release();

	return m_Views.Size() - 1;
}

void C_DeviceBase::InsertRenderer(I_Renderer* pRenderer)
{
	C_DeviceMutex deviceMutex(this);

	assert(pRenderer);
	m_Renderers.Insert(pRenderer);
}

void C_DeviceBase::RemoveRenderer(I_Renderer* pRenderer)
{
	C_DeviceMutex deviceMutex(this);

	assert(pRenderer);
	m_Renderers.Erase(pRenderer);
}

bool C_DeviceBase::CreateRenderers(E_RendererType rendererType)
{
	bool success = true;

	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		if(rendererType == E_RendererType_Unknown || rendererType == pRenderer->GetRendererType())
		{
			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Creating renderer %s\n", pRenderer->GetRendererName().Chars());
			#endif

			if(!pRenderer->Create())
				success = false;

			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Created renderer %s (%i)\n", pRenderer->GetRendererName().Chars(), success);
			#endif
		}
		it++;
	}

	return success;
}

bool C_DeviceBase::CreateRenderersVolatile(E_RendererType rendererType)
{
	bool success = true;

	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		if(rendererType == E_RendererType_Unknown || rendererType == pRenderer->GetRendererType())
		{
			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Creating volatile renderer %s\n", pRenderer->GetRendererName().Chars());
			#endif

			if(!pRenderer->CreateVolatile())
			{
				success = false;
				LOGERROR(g_Log, "CreateVolatile failed for renderer %s\n", pRenderer->GetRendererName().Chars());	// [Blur2PC]
			}

			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Created volatile renderer %s (%i)\n", pRenderer->GetRendererName().Chars(), success);
			#endif
		}
		it++;
	}

	return success;
}

void C_DeviceBase::DestroyRenderersVolatile(E_RendererType rendererType)
{
	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		if(rendererType == E_RendererType_Unknown || rendererType == pRenderer->GetRendererType())
		{
			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Destorying volatile renderer %s\n", pRenderer->GetRendererName().Chars());
			#endif

			pRenderer->DestroyVolatile();

			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Destroyed volatile renderer %s\n", pRenderer->GetRendererName().Chars());
			#endif
		}
		it++;
	}
}

void C_DeviceBase::DestroyRenderers(E_RendererType rendererType)
{
	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		if(rendererType == E_RendererType_Unknown || rendererType == pRenderer->GetRendererType())
		{
			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Destroying renderer %s\n", pRenderer->GetRendererName().Chars());
			#endif

			pRenderer->Destroy();

			#if DISPLAY_DEBUG_RENDERER
			LOGINFO(g_Log, "Destroyed renderer %s\n", pRenderer->GetRendererName().Chars());
			#endif
		}
		it++;
	}
}

bool C_DeviceBase::InitialiseRenderers()
{
	bool success = true;

	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		if(!pRenderer->Initialise())
			success = false;
		it++;
	}

	return success;
}

void C_DeviceBase::ShutdownRenderers()
{
	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		pRenderer->Shutdown();
		it++;
	}
}

void C_DeviceBase::ResetRenderers()
{
	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		pRenderer->Reset();
		it++;
	}
}

bool C_DeviceBase::PreResizeRenderers()
{
	bool success = true;

	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		if(!pRenderer->PreResize())
			success = false;
		it++;
	}

	return success;
}

bool C_DeviceBase::PostResizeRenderers()
{
	bool success = true;

	Ut::UtSet<I_Renderer*>::Iterator it = m_Renderers.Begin();
	while(it != m_Renderers.End())
	{
		I_Renderer*	pRenderer = *it;
		if(!pRenderer->PostResize())
			success = false;
		it++;
	}

	return success;
}

void C_DeviceBase::MutexWait()
{
	m_Mutex.Wait();
}

void C_DeviceBase::MutexRelease()
{
	m_Mutex.Release();
}

void C_DeviceBase::SetCpuSpeed(const E_CpuSpeed cpuSpeed)
{
	m_CpuSpeed = cpuSpeed;
}

void C_DeviceBase::SetGpuSpeed(const E_GpuSpeed gpuSpeed)
{
	m_GpuSpeed = gpuSpeed;
}

const F32 C_DeviceBase::GetAspectRatio() const
{
#ifdef PLATFORM_PC
	return m_PCAspectRatio;	// [Blur2PC]
#else
	return g_AspectRatio[m_AspectRatio];
#endif
}

}; // namespace Display
