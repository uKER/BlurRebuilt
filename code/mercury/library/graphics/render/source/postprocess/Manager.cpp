#include "StdAfx.h"
#include "Render/PostProcess/Manager.h"

#include "Render/PostProcess/Interface.h"

EXTERN_SHADER(psWeightedFilter4);
EXTERN_SHADER(psWeightedFilter5);
EXTERN_SHADER(psWeightedFilter8);
EXTERN_SHADER(psWeightedFilter9);
EXTERN_SHADER(psWeightedFilter13);
EXTERN_SHADER(psWeightedFilter15);
EXTERN_SHADER(psWeightedFilter16);
EXTERN_SHADER(psWeightedFilter4Gamma);
EXTERN_SHADER(psWeightedFilter5Gamma);
EXTERN_SHADER(psWeightedFilter8Gamma);
EXTERN_SHADER(psWeightedFilter9Gamma);
EXTERN_SHADER(psWeightedFilter13Gamma);
EXTERN_SHADER(psWeightedFilter15Gamma);
EXTERN_SHADER(psWeightedFilter16Gamma);

//void DumpDisplaySurface(Display::C_Surface* pSurface);

//#pragma optimize("", off)

using namespace Display;

namespace PostProcess
{

Debug::Logger &g_Log = Debug::GetLogger("mercury.render.postprocess");
Debug::Bool g_debugPostProcessEnable("Graphics.Render.PostProcess.Enable", true);

C_Manager::C_Manager() :	I_Renderer("PostProcess"),
							m_bActive(false),
							m_pMainTexture(NULL),
							m_pMainTextureDefault(NULL),
							m_pChainTexture(NULL),
							m_pRenderTexture(NULL),
							m_pViewportRestore(NULL),
							m_ReserveWidth(1),
							m_ReserveHeight(1),
							m_pPrimaryTextureBuffer(NULL),
							m_iPrimaryTextureBufferSize(0),
							m_pSecondaryTextureBuffer(NULL),
							m_iSecondaryTextureBufferSize(0),
							m_fFrameTime(0.0333f),
							m_Name("")
{

}

C_Manager::~C_Manager()
{
	DestroyRenderer();

	// delete any post processes which haven't been deleted
	while(m_PostProcesses.Begin() != m_PostProcesses.End())
	{
		I_PostProcess* pPostProcess = (*m_PostProcesses.Begin());
		delete pPostProcess;
	}
}

bool C_Manager::OnCreate()
{
	RESOURCE_MEMORY_ID("PostProcess");
	
	Display::CreatePixelShaderFromFunc("WeightedFilter4", 0, &m_Shaders[E_Shader_WeightedFilter4], REF_SHADER(psWeightedFilter4));
	Display::CreatePixelShaderFromFunc("WeightedFilter5", 0, &m_Shaders[E_Shader_WeightedFilter5], REF_SHADER(psWeightedFilter5));
	Display::CreatePixelShaderFromFunc("WeightedFilter8", 0, &m_Shaders[E_Shader_WeightedFilter8], REF_SHADER(psWeightedFilter8));
	Display::CreatePixelShaderFromFunc("WeightedFilter9", 0, &m_Shaders[E_Shader_WeightedFilter9], REF_SHADER(psWeightedFilter9));
	Display::CreatePixelShaderFromFunc("WeightedFilter13", 0, &m_Shaders[E_Shader_WeightedFilter13], REF_SHADER(psWeightedFilter13));
	Display::CreatePixelShaderFromFunc("WeightedFilter15", 0, &m_Shaders[E_Shader_WeightedFilter15], REF_SHADER(psWeightedFilter15));
	Display::CreatePixelShaderFromFunc("WeightedFilter16", 0, &m_Shaders[E_Shader_WeightedFilter16], REF_SHADER(psWeightedFilter16));
	Display::CreatePixelShaderFromFunc("WeightedFilter4Gamma", 0, &m_Shaders[E_Shader_WeightedFilter4Gamma], REF_SHADER(psWeightedFilter4Gamma));
	Display::CreatePixelShaderFromFunc("WeightedFilter5Gamma", 0, &m_Shaders[E_Shader_WeightedFilter5Gamma], REF_SHADER(psWeightedFilter5Gamma));
	Display::CreatePixelShaderFromFunc("WeightedFilter8Gamma", 0, &m_Shaders[E_Shader_WeightedFilter8Gamma], REF_SHADER(psWeightedFilter8Gamma));
	Display::CreatePixelShaderFromFunc("WeightedFilter9Gamma", 0, &m_Shaders[E_Shader_WeightedFilter9Gamma], REF_SHADER(psWeightedFilter9Gamma));
	Display::CreatePixelShaderFromFunc("WeightedFilter13Gamma", 0, &m_Shaders[E_Shader_WeightedFilter13Gamma], REF_SHADER(psWeightedFilter13Gamma));
	Display::CreatePixelShaderFromFunc("WeightedFilter15Gamma", 0, &m_Shaders[E_Shader_WeightedFilter15Gamma], REF_SHADER(psWeightedFilter15Gamma));
	Display::CreatePixelShaderFromFunc("WeightedFilter16Gamma", 0, &m_Shaders[E_Shader_WeightedFilter16Gamma], REF_SHADER(psWeightedFilter16Gamma));
	
	bool bCreated = true;

	for(PostProcesses::Iterator pIterator = m_PostProcesses.Begin(); pIterator != m_PostProcesses.End(); pIterator++)
	{
		I_PostProcess* pPostProcess = pIterator->GetKey();
		bCreated = bCreated && pPostProcess->Create();
	}

	return true;
}

bool C_Manager::OnCreateVolatile()
{
	MEMORY_TRACKER_PUSH("graphics.renderers.postprocess");

	bool bCreated = CreateRenderTextures();

	for(PostProcesses::Iterator pIterator = m_PostProcesses.Begin(); pIterator != m_PostProcesses.End(); pIterator++)
	{
		I_PostProcess* pPostProcess = pIterator->GetKey();
		bCreated = bCreated && pPostProcess->CreateVolatile();
	}

	MEMORY_TRACKER_POP("graphics.renderers.postprocess");

	return bCreated;
}

void C_Manager::OnDestroy()
{
	for(U32 i = 0; i < E_Shader_Count; ++i)
	{
		if(m_Shaders[i].IsCreated())
			Display::Device()->Destroy(&m_Shaders[i]);
	}

	for(PostProcesses::Iterator pIterator = m_PostProcesses.Begin(); pIterator != m_PostProcesses.End(); pIterator++)
	{
		I_PostProcess* pPostProcess = pIterator->GetKey();
		pPostProcess->Destroy();
	}
}

void C_Manager::OnDestroyVolatile()
{
	// delete textures and render targets
	for(S32 i = 0; i < m_RenderTargets.Size(); ++i)
	{
		for(S32 j = 0; j < m_RenderTargets[i].m_RenderTextures.Size(); ++j)
		{
			if(m_RenderTargets[i].m_RenderTextures[j]->IsCreated())
				Display::Device()->Destroy(m_RenderTargets[i].m_RenderTextures[j]);

			delete m_RenderTargets[i].m_RenderTextures[j];
		}

		if(m_RenderTargets[i].m_pDepthStencilSurface->IsCreated())
			Display::Device()->Destroy(m_RenderTargets[i].m_pDepthStencilSurface);

		delete m_RenderTargets[i].m_pDepthStencilSurface;
	}
	m_RenderTargets.Clear();
	m_RenderTargetMapping.Clear();
	m_ViewMapping.Clear();

	for(PostProcesses::Iterator pIterator = m_PostProcesses.Begin(); pIterator != m_PostProcesses.End(); pIterator++)
	{
		I_PostProcess* pPostProcess = pIterator->GetKey();
		pPostProcess->DestroyVolatile();
	}

}

void C_Manager::AddPostProcess(I_PostProcess* pPostProcess)
{
	assert(m_PostProcesses.Find(pPostProcess) == m_PostProcesses.End());

	m_PostProcesses.Insert(pPostProcess);
}

void C_Manager::RemovePostProcess(I_PostProcess* pPostProcess)
{
	PostProcesses::Iterator it = m_PostProcesses.Find(pPostProcess);
	assert(it != m_PostProcesses.End());
	if(it != m_PostProcesses.End())
		m_PostProcesses.Erase(it);
}

bool C_Manager::CreateRenderTextures()
{
	RESOURCE_MEMORY_ID("PostProcess");

	Display::E_Format eSecondaryFormat = Display::E_Format_A8R8G8B8;
	// [Blur2PC][HDR] Bloom/downsample filtering must retain extended scene radiance.
	if (Display::Device()->GetBackBuffer().GetColourFormat() == Display::E_Format_A16B16G16R16F)
		eSecondaryFormat = Display::E_Format_A16B16G16R16F;

	assert(m_RenderTargets.Size() == 0);
	assert(m_RenderTargetMapping.Size() == 0);
	assert(m_ViewMapping.Size() == 0);

	for(S32 iView = 0; iView < m_Views.Size(); ++iView)
	{
		Display::E_Format eSurfaceFormat = DisplayTextureFormat(m_Views[iView].eFormat);
		Display::E_Format eTextureFormat = eSurfaceFormat;

		// if rendertarget is E_Format_A2B10G10R10F, texture is actually 64 bit (for now)
		if(eTextureFormat == Display::E_Format_A2B10G10R10F)
			eTextureFormat = Display::E_Format_A16B16G16R16F;
	
		bool bCreateTexture = true;
		U32 iMapping = 0;
		
		// go through all current viewports and see if we have textures created for this view size and format already
		for(S32 i = 0; i < m_RenderTargets.Size() && bCreateTexture; ++i)
		{
#if USE_POSTPROCESS_RENDERBUFFERS
			Display2::C_RenderBuffer* pRenderTexture = m_RenderTargets[i].m_RenderTextures[0];
#else
			Display::C_RenderTexture2D* pRenderTexture = m_RenderTargets[i].m_RenderTextures[0];
#endif
			if(pRenderTexture->GetTexture().GetWidth() == m_Views[iView].iWidth
				&& pRenderTexture->GetTexture().GetHeight() == m_Views[iView].iHeight
				&& Display::GetFormatFromTextureFormat(pRenderTexture->GetTexture().GetFormat()) == eTextureFormat)
			{
				iMapping = i;
				bCreateTexture = false;
			}
		}

		if(bCreateTexture)
		{
			// we didn't match this view size and/or format so we need to create some new textures here
			C_RenderTarget postViewport;
			postViewport.iWidth = m_Views[iView].iWidth;
			postViewport.iHeight = m_Views[iView].iHeight;

			U32 iPrimaryUsageFlags = Display::E_Usage_Colour;
			U32 iSecondaryUsageFlags = Display::E_Usage_Colour;
			iSecondaryUsageFlags |= Display::E_Usage_ReadWrite;
			
			if(m_pPrimaryTextureBuffer)
			{
				// if we are overlapping then make sure rendertextures fit within the space provided
				U32 iRequired = Display::GetTexture2DSize(m_Views[iView].iWidth, m_Views[iView].iHeight, iPrimaryUsageFlags, eTextureFormat);
				if(iRequired > m_iPrimaryTextureBufferSize)
				{
					LOGERROR(g_Log, "PostProcessManager: Primary render texture requires %d bytes, current buffer is only %d bytes!\n", iRequired, m_iPrimaryTextureBufferSize);
					return false;
				}

				iPrimaryUsageFlags |= Display::E_Usage_Indirect;
			}

			if(m_pSecondaryTextureBuffer)
			{
				U32 iRequired = 0;				
				for(U32 iDownsample = 0, iScale = 1; iDownsample < m_Views[iView].nDownsamples; ++iDownsample)
				{
					iScale *= 2;
					iRequired += Display::GetTexture2DSize(m_Views[iView].iWidth / iScale, m_Views[iView].iHeight / iScale, iSecondaryUsageFlags, eSecondaryFormat);
					if(iSecondaryUsageFlags & Display::E_Usage_ReadWrite)
						iRequired += Display::GetTexture2DSize(m_Views[iView].iWidth / iScale, m_Views[iView].iHeight / iScale, iSecondaryUsageFlags, eSecondaryFormat);
				}
				if(iRequired > m_iSecondaryTextureBufferSize)
				{
					LOGERROR(g_Log, "PostProcessManager: Secondary render textures require %d bytes, current buffer is only %d bytes!\n", iRequired, m_iSecondaryTextureBufferSize);
					return false;
				}				
		
				iSecondaryUsageFlags |= Display::E_Usage_Indirect;
			}
			
			// add the main full sized texture for this view
			{
				Display::C_SurfaceParams	surfaceParams;

				// get surface parameters which will set this rendertarget after the main rendertarget in EDRAM (if more than 1 viewport)

#if USE_POSTPROCESS_RENDERBUFFERS
				Display2::C_RenderBufferSetup renderBufferSetup;
				renderBufferSetup.m_ColourMemorySize = m_iPrimaryTextureBufferSize;
				renderBufferSetup.m_pColourMemory = m_pPrimaryTextureBuffer;
				Display2::C_RenderBuffer* pRenderTexture = new Display2::C_RenderBuffer();

				pRenderTexture->SetName("_" + m_Name + "PostProcess");
				pRenderTexture->SetWidth(m_Views[iView].iWidth);
				pRenderTexture->SetHeight(m_Views[iView].iHeight);
				pRenderTexture->SetLevels(1);
				pRenderTexture->SetFlags(iPrimaryUsageFlags);
				pRenderTexture->SetMultisample(Display::GetMultisample2(m_Views[iView].eMultisample));
				pRenderTexture->SetColourFormat(0, Display::GetTextureFormat2(eSurfaceFormat));

				Display::C_Surface* pColourSurface = Display::Device()->GetSharedColourSurface(
					pRenderTexture->GetWidth(),
					pRenderTexture->GetHeight(),
					pRenderTexture->GetMultisample(),
					pRenderTexture->GetColourSurfaceFormat(0));

				//LOGINFO(Display::g_Log, "Allocating postprocess colour\n");

				if(pColourSurface)
					pRenderTexture->OverlapColourBuffer(0, pColourSurface->GetSize(), pColourSurface->GetAddress());

				//DumpDisplaySurface(pColourSurface);

				if(!Display2::Device()->CreateRenderBuffer(pRenderTexture, renderBufferSetup))
#else
				Display::C_RenderTexture2D* pRenderTexture = new Display::C_RenderTexture2D;
				if(!Display::Device()->CreateRenderTexture2D(m_Views[iView].iWidth, m_Views[iView].iHeight, 1, iPrimaryUsageFlags, eSurfaceFormat, m_Views[iView].eMultisample, &surfaceParams, pRenderTexture, m_pPrimaryTextureBuffer, m_iPrimaryTextureBufferSize))
#endif
				{
					delete pRenderTexture;
					OnDestroyVolatile();
					return false;
				}
#if !USE_POSTPROCESS_RENDERBUFFERS
				#if TRACK_DISPLAY2_RESOURCES
				Display2::Device()->SetRenderTexture2DName(pRenderTexture, "_" + m_Name + "PostProcess");
				#endif
#endif
				postViewport.m_RenderTextures.PushBack(pRenderTexture);

				Display::E_Format eDepthStencilFormat = Display::Device()->GetBackBuffer().GetDepthStencilFormat();
				postViewport.m_pDepthStencilSurface = new Display::C_Surface;
#if USE_POSTPROCESS_RENDERBUFFERS
				Display::C_Surface* pDepthStencilSurface = Display::Device()->GetSharedDepthStencilSurface(
					m_Views[iView].iWidth,
					m_Views[iView].iHeight,
					m_Views[iView].eMultisample,
					eDepthStencilFormat);

				//LOGINFO(Display::g_Log, "Allocating postprocess depth stencil\n");

				//DumpDisplaySurface(pDepthStencilSurface);
				
				if(!Display::Device()->CreateDepthStencilSurface(m_Views[iView].iWidth, m_Views[iView].iHeight, Display::E_Usage_DepthStencil, eDepthStencilFormat, m_Views[iView].eMultisample, &surfaceParams, postViewport.m_pDepthStencilSurface,
					pDepthStencilSurface ? pDepthStencilSurface->GetAddress() : NULL,
					pDepthStencilSurface ? pDepthStencilSurface->GetSize() : 0))
#else
				if(!Display::Device()->CreateDepthStencilSurface(m_Views[iView].iWidth, m_Views[iView].iHeight, Display::E_Usage_DepthStencil, eDepthStencilFormat, m_Views[iView].eMultisample, &surfaceParams, postViewport.m_pDepthStencilSurface))
#endif
				{
					delete postViewport.m_pDepthStencilSurface;
					OnDestroyVolatile();
					return false;
				}

				#if TRACK_DISPLAY2_RESOURCES
				Display2::Device()->SetSurfaceName(postViewport.m_pDepthStencilSurface, "_" + m_Name + "PostProcess");
				#endif
			}

			U32 iTextureBufferSize = 0;
			U8* pTextureBuffer = m_pSecondaryTextureBuffer;
			if (pTextureBuffer)
				iTextureBufferSize = m_iSecondaryTextureBufferSize;

			// create the required number of downsample textures
			for(U32 iDownsample = 0, iScale = 1; iDownsample < m_Views[iView].nDownsamples; ++iDownsample)
			{
				iScale *= 2;
#if USE_POSTPROCESS_RENDERBUFFERS
				Display2::C_RenderBufferSetup renderBufferSetup;
				renderBufferSetup.m_ColourMemorySize = m_iSecondaryTextureBufferSize - (pTextureBuffer - m_pSecondaryTextureBuffer);
				renderBufferSetup.m_pColourMemory = pTextureBuffer;
				Display2::C_RenderBuffer* pRenderTexture = new Display2::C_RenderBuffer();

				pRenderTexture->SetName("_" + m_Name + "PostProcess");
				pRenderTexture->SetWidth(m_Views[iView].iWidth / iScale);
				pRenderTexture->SetHeight(m_Views[iView].iHeight / iScale);
				pRenderTexture->SetLevels(1);
				pRenderTexture->SetFlags(iSecondaryUsageFlags);
				pRenderTexture->SetMultisample(Display::GetMultisample2(Display::E_Multisample_None));
				pRenderTexture->SetColourFormat(0, Display::GetTextureFormat2(eSecondaryFormat));

				if(!Display2::Device()->CreateRenderBuffer(pRenderTexture, renderBufferSetup))
#else
				Display::C_RenderTexture2D* pRenderTexture = new Display::C_RenderTexture2D;
				if(!Display::Device()->CreateRenderTexture2D(m_Views[iView].iWidth / iScale, m_Views[iView].iHeight / iScale, 1, iSecondaryUsageFlags, eSecondaryFormat, Display::E_Multisample_None, NULL, pRenderTexture, pTextureBuffer, iTextureBufferSize))
#endif
				{
					delete pRenderTexture;
					return false;
				}
#if !USE_POSTPROCESS_RENDERBUFFERS
				#if TRACK_DISPLAY2_RESOURCES
				Display2::Device()->SetRenderTexture2DName(pRenderTexture, "_" + m_Name + "PostProcess");
				#endif
#endif
				postViewport.m_RenderTextures.PushBack(pRenderTexture);
				
				// move the overlapped texture address
				if(pTextureBuffer)
				{
					const Display::C_Texture2D& texture = postViewport.m_RenderTextures.Back()->GetTexture();
					U32 iTextureSize = Display::GetTexture2DSize(texture.GetWidth(), texture.GetHeight(), iSecondaryUsageFlags, Display::GetFormatFromTextureFormat(texture.GetFormat()));
					pTextureBuffer += iTextureSize;
					if(iSecondaryUsageFlags & Display::E_Usage_ReadWrite)
						pTextureBuffer += iTextureSize;
				}
			}

			// store the mapping and add to viewport
			iMapping = m_RenderTargets.Size();
			m_RenderTargets.PushBack(postViewport);
		}

		while(m_RenderTargetMapping.Size() <= m_Views[iView].iIndex)
			m_RenderTargetMapping.PushBack(-1);

		m_RenderTargetMapping[m_Views[iView].iIndex] = iMapping;

		while(m_ViewMapping.Size() <= m_Views[iView].iIndex)
			m_ViewMapping.PushBack(-1);

		m_ViewMapping[m_Views[iView].iIndex] = iView;
	}

	return true;
}

void C_Manager::Begin(S32 iIndex, const Display::C_Viewport& viewport) const
{
	// see if this view wants to have post processing enabled
	bool bViewDisable = (iIndex < 0 || iIndex >= m_RenderTargetMapping.Size() || m_RenderTargetMapping[iIndex] == -1);

	m_bEnabled = g_debugPostProcessEnable && !bViewDisable && IsRendererCreated();

	if(m_bEnabled)
	{
		if(!m_bActive)
		{
			m_bActive = true;

			// one view per post effects Display::C_View which has post effects
			SetView(iIndex);
			// one viewport per unique size post effects view
			m_iRenderTarget = m_RenderTargetMapping[iIndex];
						
			// override viewport position to be at 0,0
			m_ViewportOrigin = viewport;
			m_ViewportOrigin.m_X = 0;
			m_ViewportOrigin.m_Y = 0;
			SetViewport(m_ViewportOrigin);

			Display::Quad::SetQuadSettings();

			m_pViewportRestore = &viewport;
			
			m_pMainTexture = NULL;
		
			if(m_pRenderTexture)
				m_pMainTextureDefault = &m_pRenderTexture->GetTexture();
			else
				m_bEnabled = false;
		}
		else
			LOGWARN( g_Log, "PostProcessManager: Post processing has already been activated\n");
	}
	else if(bViewDisable)
		LOGERROR( g_Log, "PostProcessManager: Invalid view index\n");
}

void C_Manager::SetView(S32 iIndex) const
{
	if(iIndex >= 0 && iIndex < m_ViewMapping.Size())
		m_iView = m_ViewMapping[iIndex];
	else
		LOGERROR( g_Log, "PostProcessManager: Invalid view index\n");
}

void C_Manager::End() const
{
	if(m_bEnabled && m_bActive)
	{
		PERF_EVENT("PostProcess::FinalQuad");
		RestoreColourSurface();
		RestoreViewport();
		SetFinalQuadSettings();


		assert(m_pRenderTexture);
		Display::Quad::DrawFullScreen(&m_pRenderTexture->GetTexture());

	}

	m_bActive = false;
}

void C_Manager::Suspend() const
{
	m_bActive = false;
}

void C_Manager::RestoreViewport() const
{
	Display::Device()->SetViewport(*m_pViewportRestore);
	Display::Device()->SetBlendMode(Display::E_BlendMode_Solid);
}

void C_Manager::RestoreColourSurface() const
{
	Display::Device()->SetColourDepthStencilSurface();
}

void C_Manager::SetFinalQuadSettings() const
{
	if(m_Views[m_iView].bFlip)
		Display::Quad::SetQuadSettings(MathsStorage::UInt2(1, 1), 1.0f, 1.0f, true, true);
	else
		Display::Quad::SetQuadSettings();
}

// sets the current viewport for post processing
void C_Manager::SetViewport() const
{
	SetViewport(m_ViewportOrigin);
}

void C_Manager::SetViewport(const Display::C_Viewport& viewport) const
{
	if(m_bActive)
	{	
		// try and find a render texture which fits this viewport size
		if((m_pRenderTexture = GetRenderTexture(Display::Device()->GetView()->GetIndex(), viewport)))
		{
//#ifndef PLATFORM_PS3
			Display::Device()->SetColourDepthStencilSurface(&m_pRenderTexture->GetSurface(), m_RenderTargets[m_iRenderTarget].m_pDepthStencilSurface);
//#else
			//Display::Device()->SetColourSurface(&m_pRenderTexture->GetSurface());
			//Display::Device()->SetSurface();
//#endif
		}

		Display::Device()->SetViewport(viewport);
		m_Viewport = viewport;
	}
	else
		Display::Device()->SetViewport(viewport);
}

// sets the current render texture
void C_Manager::SetRenderTexture(const Display::C_RenderTexture2D& renderTexture) const
{
	//  make sure render texture size matches viewport size (used for half pixel offsets etc)
	if(renderTexture.GetTexture().GetWidth() != m_Viewport.GetW() || renderTexture.GetTexture().GetHeight() != m_Viewport.GetH())
	{
		m_Viewport.m_W = renderTexture.GetTexture().GetWidth();
		m_Viewport.m_H = renderTexture.GetTexture().GetHeight();

		Display::Device()->SetViewport(m_Viewport);
	}
	m_pRenderTexture = &renderTexture;

//#ifndef PLATFORM_PS3
	Display::Device()->SetColourDepthStencilSurface(&m_pRenderTexture->GetSurface(), m_RenderTargets[m_iRenderTarget].m_pDepthStencilSurface);
//#else
	//Display::Device()->SetColourSurface(&m_pRenderTexture->GetSurface());
	//Display::Device()->SetSurface();
//#endif
}

// call this function for each viewport which requires post effects
bool C_Manager::AddView(const U32 iWidth, const U32 iHeight, const S32 iIndex, const PostProcess::E_RenderTargetFormat eFormat, const Display::E_Multisample eMultisample, const U32 nDownsamples, const bool bFlip)
{
#ifndef CFG_SPEED
	for(S32 i = 0; i < m_Views.Size(); ++i)
		assert(m_Views[i].iIndex != iIndex);
#endif

	m_Views.PushBack();
	m_Views.Back().iWidth = iWidth;
	m_Views.Back().iHeight = iHeight;
	m_Views.Back().iIndex = iIndex;
	m_Views.Back().nDownsamples = nDownsamples;
	m_Views.Back().eFormat = eFormat;
	m_Views.Back().eMultisample = eMultisample;
	m_Views.Back().bFlip = bFlip;

	return true;
}

const Display::C_RenderTexture2D* C_Manager::GetRenderTexture(const U32 iViewIndex, const Display::C_Viewport& viewport) const
{
	// go through the textures created for this view and try and match texture, it should usually be first one
	for(S32 i = 0; i < m_RenderTargets[m_iRenderTarget].m_RenderTextures.Size(); ++i)
	{
		const Display::C_Texture2D& texture = m_RenderTargets[m_iRenderTarget].m_RenderTextures[i]->GetTexture();
		if(viewport.GetW() == texture.GetWidth() && viewport.GetH() == texture.GetHeight())
#if USE_POSTPROCESS_RENDERBUFFERS
			return (const Display::C_RenderTexture2D*)&m_RenderTargets[m_iRenderTarget].m_RenderTextures[i]->m_ColourBuffer[0];
#else
			return m_RenderTargets[m_iRenderTarget].m_RenderTextures[i];
#endif
	}

	return NULL;
}

void C_Manager::SetChainTexture() const
{
	assert(m_pRenderTexture);
	if(m_pMainTexture && &m_pRenderTexture->GetTexture() == m_pMainTextureDefault)
		m_pChainTexture = m_pMainTexture;
	else
		m_pChainTexture = &m_pRenderTexture->GetTexture();
}

void C_Manager::ResolveRenderTexture() const
{
	assert(m_pRenderTexture);
	m_pRenderTexture->Resolve();

	// we may need to rebind surfaces if using double buffered render texture
	if(m_pRenderTexture->GetFlags() & E_Usage_ReadWrite)
	{
		SetRenderTexture(*m_pRenderTexture);
		SetChainTexture();
	}
};

U32 C_Manager::GetMaximumSurfaceSize(const S32 iWidthScale, const S32 iHeightScale, const Display::E_Format eFormat) const
{
	U32 iSurfaceSize = 0;
	
	Display::E_Format eNewFormat;
	eNewFormat = eFormat;
	
	for(S32 i = 0; i < m_Views.Size(); ++i)
	{
		if(eNewFormat == Display::E_Format_Unknown)
			eNewFormat = DisplayTextureFormat(m_Views[i].eFormat);
		const U32 iWidth = m_Views[i].iWidth / iWidthScale;
		const U32 iHeight = m_Views[i].iHeight / iHeightScale;
		iSurfaceSize = Maths::Max(iSurfaceSize, Display::GetSurfaceSize(iWidth, iHeight, 0, eNewFormat, Display::E_Multisample_None));
	}
	iSurfaceSize = Maths::Max(iSurfaceSize, Display::GetSurfaceSize(m_ReserveWidth / iWidthScale, m_ReserveHeight / iHeightScale, 0, eNewFormat, Display::E_Multisample_None));

	return iSurfaceSize;
}

U32 C_Manager::GetMaximumTextureSize(const S32 iWidthScale, const S32 iHeightScale, const Display::E_Format eFormat) const
{
	U32 iTextureSize = 0;
	Display::E_Format eNewFormat;
	eNewFormat = eFormat;
	

	for(S32 i = 0; i < m_Views.Size(); ++i)
	{
		if(eNewFormat == Display::E_Format_Unknown)
			eNewFormat = DisplayTextureFormat(m_Views[i].eFormat);
		if(eNewFormat == Display::E_Format_A2B10G10R10F)
			eNewFormat = Display::E_Format_A16B16G16R16F;
		const U32 iWidth = m_Views[i].iWidth / iWidthScale;
		const U32 iHeight = m_Views[i].iHeight / iHeightScale;
		iTextureSize = Maths::Max(iTextureSize, Display::GetTexture2DSize(iWidth, iHeight, Display::E_Usage_Colour, eNewFormat));
	}
	iTextureSize = Maths::Max(iTextureSize, Display::GetTexture2DSize(m_ReserveWidth / iWidthScale, m_ReserveHeight / iHeightScale, Display::E_Usage_Colour, eNewFormat));
	return iTextureSize;
}

bool C_Manager::CreateUserRenderTextures(const U32 iWidthScale, const U32 iHeightScale, const U32 flags, const Display::E_Format eFormat, const Display::C_SurfaceParams* pSurfaceParams, RenderTextures& renderTextures, U8* pAddress, U32 bufferSize) const
{
	bool bSuccess = true;
	for(S32 i = 0; i < m_RenderTargets.Size(); ++i)
	{
		Display::C_RenderTexture2D* pRenderTexture = new Display::C_RenderTexture2D;
		const U32 iWidth = m_RenderTargets[i].iWidth / iWidthScale;
		const U32 iHeight = m_RenderTargets[i].iHeight / iHeightScale;
		Display::E_Format eNewFormat = eFormat;
		if(eNewFormat == Display::E_Format_Unknown)
			eNewFormat = Display::GetFormatFromSurfaceFormat(m_RenderTargets[i].m_RenderTextures[0]->GetSurface().GetFormat());
		U32 iTextureSize = Display::GetRenderTexture2DSize(iWidth, iHeight, 1, flags, eNewFormat, Display::E_Multisample_None);

#if defined(PLATFORM_PS3)
		if(flags & Display2::E_Flags_ReadWrite)
			iTextureSize *= 2;
#endif

		assert(iTextureSize <= bufferSize);

		bSuccess = bSuccess && Display::Device()->CreateRenderTexture2D(iWidth, iHeight, 1, flags, eNewFormat, Display::E_Multisample_None, pSurfaceParams, pRenderTexture, pAddress, iTextureSize);
		#if TRACK_DISPLAY2_RESOURCES
		if(bSuccess)
			Display2::Device()->SetRenderTexture2DName(pRenderTexture, "_" + m_Name + "PostProcessUser");
		#endif
		renderTextures.PushBack(pRenderTexture);		
	}
	return bSuccess;
}

void C_Manager::DestroyUserRenderTextures(RenderTextures& renderTextures) const
{
	for(S32 i = 0; i < renderTextures.Size(); ++i)
	{
		Display::Device()->Destroy(renderTextures[i]);
		delete renderTextures[i];
	}
	renderTextures.Clear();
}

void C_Manager::SetName(const Util::C_String32& name)
{
	m_Name = name;
}

} // namespace PostProcess
