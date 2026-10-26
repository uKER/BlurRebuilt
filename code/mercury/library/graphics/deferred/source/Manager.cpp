#include "stdafx.h"
#include <system/debug/Blur2PCAutomation.h>
#include "Deferred/Manager.h"

Debug::Bool	g_Tiled_Override_Deferred_Colour	("Graphics.Render.Resolution.Tiled_Override_Deferred_Colour",	false);
Debug::Bool	g_Tiled_Override_Deferred_Depth		("Graphics.Render.Resolution.Tiled_Override_Deferred_Depth",	false);

Debug::Bool	g_Tiled_Enable_Deferred_Colour		("Graphics.Render.Resolution.Tiled_Enable_Deferred_Colour",		true);
Debug::Bool	g_Tiled_Enable_Deferred_Depth		("Graphics.Render.Resolution.Tiled_Enable_Deferred_Depth",		true);

//void DumpDisplaySurface(Display::C_Surface* pSurface);

//#pragma optimize("", off)

namespace Deferred
{

#if USE_DEFERRED_RENDER_BUFFERS

C_Manager::C_Manager() : Display::I_Renderer("DeferredManager"), m_bActive(false), m_iActiveIndex(-1)
{
	m_TextureBufferSize	= 0;
	m_pTextureBuffer	= NULL;
}

C_Manager::~C_Manager()
{
	DestroyRenderer();
}

bool C_Manager::OnCreate()
{
	// we need reserve enough memory for the lights buffer only. on PS3 the normal buffer uses the shared colour surface
	U32 iWidth = Display::Device()->m_DeviceParams.m_BackBufferWidth;
	U32 iHeight = Display::Device()->m_DeviceParams.m_BackBufferHeight;

	// this is system memory so we need to align height to 64 and allocation to 64k aligned
	iWidth = iWidth / 2;
	iHeight = ALIGN_SIZE(iHeight / 2, 64);
	m_TextureBufferSize = Display::GetTexture2DSize(iWidth, iHeight, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8);
	m_TextureBufferSize = ALIGN_SIZE(m_TextureBufferSize, 65536) * 4;

	if (!Display::Device()->AllocMemory(65536, m_TextureBufferSize, &m_pTextureBuffer, Display::E_AllocHeap_System, Display::E_AllocType_Default, Display2::E_MemoryType_Texture))
		return false;

	return true;
}

bool C_Manager::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("Deferred::C_Manager");
	
	MEMORY_TRACKER_PUSH("graphics.renderers.deferred");
	if (!CreateRenderTargets())
	{
		MEMORY_TRACKER_POP("graphics.renderers.deferred");
		return false;
	}
	MEMORY_TRACKER_POP("graphics.renderers.deferred");

	return true;
}

void C_Manager::OnDestroy()
{
	Display::Device()->FreeMemory(m_TextureBufferSize, m_pTextureBuffer, Display::E_AllocHeap_System, Display2::E_MemoryType_Texture);

	m_TextureBufferSize	= 0;
	m_pTextureBuffer		= NULL;
}

void C_Manager::OnDestroyVolatile()
{
	// loop through views and destroy render targets
	for (S32 iView = 0; iView < m_Views.Size(); ++iView)
	{
		C_View& view = m_Views[iView];

		Display2::Device()->DestroyRenderBuffer(&view);
	}
}

bool C_Manager::OnPreResize()
{
	DestroyVolatile();

	return true;
}
	
bool C_Manager::OnPostResize()
{
	return CreateVolatile();
}

bool C_Manager::AddRenderTargetSetup(const C_RenderTargetSetup& setup)
{
	m_Setups.PushBack();
	m_Setups.Back() = setup;

	m_SetupMapping[setup.iIndex]			= m_Setups.Size() - 1;

	return true;
}

void C_Manager::ClearRenderTargetSetups()
{
	m_Setups.Clear();
	m_SetupMapping.Clear();
}

bool C_Manager::AddView(const U32 iWidth, const U32 iHeight, const S32 iIndex, const S32 iRenderTargetIndex)
{
	m_Views.PushBack();
	m_Views.Back().SetWidth(iWidth);
	m_Views.Back().SetHeight(iHeight);
	m_Views.Back().iIndex				= iIndex;
	m_Views.Back().iRenderTargetIndex	= iRenderTargetIndex;

	m_ViewMapping[iIndex]				= m_Views.Size() - 1;

	return true;
}

void C_Manager::ClearViews()
{
	m_Views.Clear();
	m_ViewMapping.Clear();
}

void C_Manager::Begin(const S32 iIndex)
{
	assert(!m_bActive);

	m_iActiveIndex = m_ViewMapping[iIndex];

	const C_View& view = m_Views[m_iActiveIndex];

	if (view.GetColourSurface(0))
		Display::Device()->SetColourSurface(0, view.GetColourSurface(0));
	if (view.GetColourSurface(1))
		Display::Device()->SetColourSurface(1, view.GetColourSurface(1));
	if (view.GetColourSurface(2))
		Display::Device()->SetColourSurface(2, view.GetColourSurface(2));
	if (view.GetColourSurface(3))
		Display::Device()->SetColourSurface(3, view.GetColourSurface(3));
 	if (view.GetDepthStencilSurface())
		Display::Device()->SetDepthStencilSurface(view.GetDepthStencilSurface());
 
#if defined(PLATFORM_PS3)
	Display::Device()->SetSurface();
#endif
 
	if (view.GetDepthStencilSurface())
	{
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	}
	else
	{
		Display::Device()->SetDepthEnable(Display::E_Bool_False);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	}
 
	m_bActive = true;
}

void C_Manager::End()
{
	assert(m_bActive);

	const C_View& view = m_Views[m_iActiveIndex];

#if 0 // TODO: Fix me!
	if (view.renderTextures[0])
		view.renderTextures[0]->Resolve();
	if (view.renderTextures[1])
		view.renderTextures[1]->Resolve();
	if (view.renderTextures[2])
		view.renderTextures[2]->Resolve();
	if (view.renderTextures[3])
		view.renderTextures[3]->Resolve();
#endif

	m_bActive = false;
}

const Display::C_Surface* C_Manager::GetColourSurface(const S32 iViewIndex, const S32 iColourIndex)
{
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	return view.GetColourSurface(iColourIndex);
}

const Display::C_Surface* C_Manager::GetDepthStencilSurface(const S32 iViewIndex)
{
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	return view.GetDepthStencilSurface();
}

const Display::C_Texture2D* C_Manager::GetColourTexture(const S32 iViewIndex, const S32 iColourIndex)
{
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	return view.GetColourTexture(iColourIndex);
}
 
const Display::C_Texture2D* C_Manager::GetDepthStencilTexture(const S32 iViewIndex)
{
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	return view.GetDepthStencilTexture();
}

bool C_Manager::CreateRenderTargets()
{
	Display::Device()->AcquireThreadOwnership();

	U8* pTextureBuffer = m_pTextureBuffer;
	
	// loop through views and create render targets based on their setups
	for (S32 iView = 0; iView < m_Views.Size(); ++iView)
	{
		C_View& view = m_Views[iView];

		const S32 iSetupIndex = m_SetupMapping[view.iRenderTargetIndex];
		const C_RenderTargetSetup& setup = m_Setups[iSetupIndex];

		for (S32 i = 0; i < 4; ++i)
		{
			if (setup.eColourFormat[i] != Display::E_Format_Unknown)
			{
				view.SetColourFormat(i, Display::GetTextureFormat2(setup.eColourFormat[i]));

				if (setup.bColourTiling)
				{
					Display::C_Surface* pColourSurface = Display::Device()->GetSharedColourSurface(
						view.GetWidth(),
						view.GetHeight(),
						view.GetMultisample(),
						view.GetColourSurfaceFormat(0),
						setup.eColourAllocHeap == Display::E_AllocHeap_System ? true : false);
					
					//LOGINFO(Display::g_Log, "Allocating deferred colour\n");

					if (pColourSurface)
						view.OverlapColourBuffer(i, pColourSurface->GetSize(), pColourSurface->GetAddress());

					//DumpDisplaySurface(pColourSurface);
				}

				if (!view.m_pColourAddress[i])
				{
					if (setup.eColourAllocHeap == Display::E_AllocHeap_System)
						view.SetColourFlags(i, view.GetColourFlags(i) | Display::E_Usage_System);

					if (Display::Device()->GetWindowCount() > 1)
						view.SetColourTileEnable(i, true); // Make sure tiling is on for splitscreen as we don't use the spu proggy to make the light buffer!
				}
			}
		}

		if (setup.eDepthStencilFormat != Display::E_Format_Unknown)
		{
			view.SetDepthStencilFormat(Display::GetTextureFormat2(setup.eDepthStencilFormat));

			if (setup.bDepthTiling)
			{
				Display::C_Surface* pDepthStencilSurface = Display::Device()->GetSharedDepthStencilSurface(
					view.GetWidth(),
					view.GetHeight(),
					view.GetMultisample(),
					view.GetDepthStencilSurfaceFormat(),
					setup.eDepthAllocHeap == Display::E_AllocHeap_System ? true : false);
				
				//LOGINFO(Display::g_Log, "Allocating deferred depth stencil\n");

				if (pDepthStencilSurface)
					view.OverlapDepthStencilBuffer(pDepthStencilSurface->GetSize(), pDepthStencilSurface->GetAddress());

				//DumpDisplaySurface(pDepthStencilSurface);
			}

			if (!view.m_pDepthStencilAddress)
			{
				if (setup.eDepthAllocHeap == Display::E_AllocHeap_System)
					view.SetDepthStencilFlags(view.GetDepthStencilFlags() | Display::E_Usage_System);
			}
		}

		Util::C_String renderBufferName = "";
		renderBufferName.Printf("_Deferred%i", iView);
		view.SetName(renderBufferName.Chars());

		
		Display2::C_RenderBufferSetup renderBufferSetup;
		// if this view isn't using tiling then allocate memory out of the buffer we reserved earlier
		if(!setup.bColourTiling)
		{
			// work out offset alignment
			U32 colourSize = Display::GetTexture2DSize(view.GetWidth(), ALIGN_SIZE(view.GetHeight(), 64), 1, Display::E_Usage_Video, setup.eColourFormat[0]);
			colourSize = ALIGN_SIZE(colourSize, 65536);
			// set texture
			renderBufferSetup.m_pColourMemory = pTextureBuffer;
			renderBufferSetup.m_ColourMemorySize = colourSize;
			pTextureBuffer += colourSize;
		}
		if(!Display2::Device()->CreateRenderBuffer(&view, renderBufferSetup))
		{
			Display::Device()->ReleaseThreadOwnership();

			return false;
		}
	}

	Display::Device()->ReleaseThreadOwnership();

	return true;
}

#else // USE_DEFERRED_RENDER_BUFFERS

C_Manager::C_Manager() : Display::I_Renderer("DeferredManager"), m_bActive(false), m_iActiveIndex(-1)
{
	m_RenderTextureBufferSize	= 0;
	m_DepthTextureBufferSize	= 0;
	m_pRenderTextureBuffer		= NULL;
	m_pDepthTextureBuffer		= NULL;
}

C_Manager::~C_Manager()
{
	DestroyRenderer();
}

bool C_Manager::OnCreate()
{
	if (m_Views.Size() == 0 || m_Setups.Size() == 0)
		return false;

	const C_View& view = m_Views[0];
	const C_RenderTargetSetup& setup = m_Setups[0];


	return true;
}

bool C_Manager::OnCreateVolatile()
{
	RESOURCE_MEMORY_ID("Deferred::C_Manager");
	Blur2PCAutomation::LogF("DX11 Deferred OnCreateVolatile views=%d", m_Views.Size());
	
	MEMORY_TRACKER_PUSH("graphics.renderers.deferred");
	if (!CreateRenderTargets())
	{
		MEMORY_TRACKER_POP("graphics.renderers.deferred");
		return false;
	}
	MEMORY_TRACKER_POP("graphics.renderers.deferred");

	return true;
}

void C_Manager::OnDestroy()
{

	m_RenderTextureBufferSize	= 0;
	m_DepthTextureBufferSize	= 0;
	m_pRenderTextureBuffer		= NULL;
	m_pDepthTextureBuffer		= NULL;
}

void C_Manager::OnDestroyVolatile()
{
	// loop through views and destroy render targets
	for (S32 iView = 0; iView < m_Views.Size(); ++iView)
	{
		C_View& view = m_Views[iView];

		const S32 iSetupIndex = m_SetupMapping[view.iRenderTargetIndex];
		const C_RenderTargetSetup& setup = m_Setups[iSetupIndex];

		for (S32 i = 0; i < 4; ++i)
		{
			if (view.renderTextures[i])
			{
#if defined(PLATFORM_PS3)
				const U32 iSize = view.renderTextures[i]->GetSurface().GetSize();
				Display::Device()->Destroy(view.renderTextures[i]);
				if(setup.bColourTiling)
				{
					#if TRACK_DISPLAY2_PROTECTED_MEMORY
					//Display2::Device()->RemoveProtectedMemory(iSize, view.pColourAddress[i]);
					#endif
					Display::Device()->DestroyTile(view.iColourTileIndex[i], view.iColourTileBase[i], &view.colourTileInfo[i]);
				}
				Display::Device()->FreeMemory(iSize, view.pColourAddress[i], setup.eColourAllocHeap, Display2::E_MemoryType_Surface);
#else
				Display::Device()->Destroy(view.renderTextures[i]);
#endif
			}
			delete view.renderTextures[i];
			view.renderTextures[i] = 0;
		}
 
		if (view.pDepthStencilSurface)
		{
#if defined(PLATFORM_PS3)
			const U32 iSize = view.pDepthStencilSurface->GetSize();
#endif
			Display::Device()->Destroy(view.pDepthStencilSurface);
			delete view.pDepthStencilSurface;
			view.pDepthStencilSurface = 0;

			if (view.pDepthStencilTexture)
			{
				Display::Device()->Destroy(view.pDepthStencilTexture);
				delete view.pDepthStencilTexture;
				view.pDepthStencilTexture = NULL;
			}

#if defined(PLATFORM_PS3)
			if(setup.bDepthTiling)    
			{
				#if TRACK_DISPLAY2_PROTECTED_MEMORY
				//Display2::Device()->RemoveProtectedMemory(iSize, view.pDepthAddress);
				#endif
				Display::Device()->DestroyTile(view.iDepthTileIndex, view.iDepthTileBase, &view.depthTileInfo);
			}
			if(setup.eDepthAllocHeap == Display::E_AllocHeap_Video)
				Display::Device()->DestroyZCull(view.iZCullIndex, view.iZCullStart);
			Display::Device()->FreeMemory(iSize, view.pDepthAddress, setup.eDepthAllocHeap, Display2::E_MemoryType_Surface);
#endif
		}
 
	}
}

bool C_Manager::AddRenderTargetSetup(const C_RenderTargetSetup& setup)
{
	m_Setups.PushBack();
	m_Setups.Back() = setup;

	m_SetupMapping[setup.iIndex]			= m_Setups.Size() - 1;

	return true;
}

void C_Manager::ClearRenderTargetSetups()
{
	m_Setups.Clear();
	m_SetupMapping.Clear();
}

bool C_Manager::AddView(const U32 iWidth, const U32 iHeight, const S32 iIndex, const S32 iRenderTargetIndex)
{
	Blur2PCAutomation::LogF("DX11 Deferred AddView index=%d setup=%d size=%ux%u", iIndex, iRenderTargetIndex, iWidth, iHeight);
	m_Views.PushBack();
	m_Views.Back().iWidth				= iWidth;
	m_Views.Back().iHeight				= iHeight;
	m_Views.Back().iIndex				= iIndex;
	m_Views.Back().iRenderTargetIndex	= iRenderTargetIndex;

	m_Views.Back().renderTextures[0]	= 0;
	m_Views.Back().renderTextures[1]	= 0;
	m_Views.Back().renderTextures[2]	= 0;
	m_Views.Back().renderTextures[3]	= 0;
 	
	m_Views.Back().pDepthStencilSurface	= 0;
	m_Views.Back().pDepthStencilTexture	= 0;
 
	m_ViewMapping[iIndex]				= m_Views.Size() - 1;

	return true;
}

void C_Manager::ClearViews()
{
	m_Views.Clear();
	m_ViewMapping.Clear();
}

void C_Manager::Begin(const S32 iIndex)
{
	assert(!m_bActive);

	m_iActiveIndex = m_ViewMapping[iIndex];

	const C_View& view = m_Views[m_iActiveIndex];

	if (view.renderTextures[0])
		Display::Device()->SetColourSurface(0, &view.renderTextures[0]->GetSurface());
	if (view.renderTextures[1])
		Display::Device()->SetColourSurface(1, &view.renderTextures[1]->GetSurface());
	if (view.renderTextures[2])
		Display::Device()->SetColourSurface(2, &view.renderTextures[2]->GetSurface());
	if (view.renderTextures[3])
		Display::Device()->SetColourSurface(3, &view.renderTextures[3]->GetSurface());
 	if (view.pDepthStencilSurface)
		Display::Device()->SetDepthStencilSurface(view.pDepthStencilSurface);
 
#if defined(PLATFORM_PS3)
	Display::Device()->SetSurface();
#endif
 
	if (view.pDepthStencilSurface)
	{
		Display::Device()->SetDepthEnable(Display::E_Bool_True);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_True);
	}
	else
	{
		Display::Device()->SetDepthEnable(Display::E_Bool_False);
		Display::Device()->SetDepthWriteEnable(Display::E_Bool_False);
	}
 
	m_bActive = true;
}

void C_Manager::End()
{
	assert(m_bActive);

	const C_View& view = m_Views[m_iActiveIndex];

	if (view.renderTextures[0])
		view.renderTextures[0]->Resolve();
	if (view.renderTextures[1])
		view.renderTextures[1]->Resolve();
	if (view.renderTextures[2])
		view.renderTextures[2]->Resolve();
	if (view.renderTextures[3])
		view.renderTextures[3]->Resolve();

	m_bActive = false;
}

const Display::C_Surface* C_Manager::GetColourSurface(const S32 iViewIndex, const S32 iColourIndex)
{
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	if (view.renderTextures[iColourIndex])
		return &view.renderTextures[iColourIndex]->GetSurface();
	else
		return 0;
}

const Display::C_Surface* C_Manager::GetDepthStencilSurface(const S32 iViewIndex)
{
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	return view.pDepthStencilSurface;
}

const Display::C_Texture2D* C_Manager::GetColourTexture(const S32 iViewIndex, const S32 iColourIndex)
{
	static int dx11DeferredGetLogCount = 0;
	if (dx11DeferredGetLogCount++ < 16)
		Blur2PCAutomation::LogF("DX11 Deferred GetColour view=%d colour=%d views=%d", iViewIndex, iColourIndex, m_Views.Size());
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	if (view.renderTextures[iColourIndex])
		return &view.renderTextures[iColourIndex]->GetTexture();
	else
		return 0;
}


const Display::C_Texture2D* C_Manager::GetDepthStencilTexture(const S32 iViewIndex)
{
	const S32 iMappedIndex = m_ViewMapping[iViewIndex];
	const C_View& view = m_Views[iMappedIndex];
	return view.pDepthStencilTexture;
}

bool C_Manager::CreateRenderTargets()
{
	Display::Device()->AcquireThreadOwnership();
	//
	// 360
	// 



#if defined(PLATFORM_PS3)
	U8 currentTileIndex = Display::Device()->m_CurrentTileIndex;
	U16 currentTileBase = Display::Device()->m_CurrentTileBase;

	//U8* pDepthTextureBuffer = m_pDepthTextureBuffer;
	//U8* pRenderTextureBuffer = m_pRenderTextureBuffer;

	// loop through views and create render targets based on their setups
	for (S32 iView = 0; iView < m_Views.Size(); ++iView)
	{
		C_View& view = m_Views[iView];

		const U32 alignedWidth = GCM_ALIGN_SIZE(view.iWidth, 64);
		const U32 alignedHeight = GCM_ALIGN_SIZE(view.iHeight, 64);

		const S32 iSetupIndex = m_SetupMapping[view.iRenderTargetIndex];
		const C_RenderTargetSetup& setup = m_Setups[iSetupIndex];

		for (S32 i = 0; i < 4; ++i)
		{
			view.renderTextures[i] = 0;

			if (setup.eColourFormat[i] != Display::E_Format_Unknown)
			{
				U32 flags = Display::E_Usage_Colour | Display::E_Usage_Indirect;
				
				U32 iPitch = 0, iSize = 0;
				Display2::GetSurfaceSizeAndPitch(alignedWidth, alignedHeight, setup.bColourTiling ? Display::E_Format_Tiled | flags : flags, Display::GetSurfaceFormatFromFormat(setup.eColourFormat[i]), (Display2::E_Multisample)setup.eMultisample, iSize, iPitch);

				if (!Display::Device()->AllocMemory(65536, iSize, &view.pColourAddress[i], setup.eColourAllocHeap, Display::E_AllocType_Default, Display2::E_MemoryType_Surface))
				{
					BIZ_ASSERT_FAIL("Failed to allocate colour buffer memory!");
					Display::Device()->ReleaseThreadOwnership();
					return false;
				}

				if (setup.eColourAllocHeap == Display::E_AllocHeap_System)
					flags |= Display::E_Usage_System;
				
				Display::C_RenderTexture2D* pRenderTexture = new Display::C_RenderTexture2D;		
				if (!Display::Device()->CreateRenderTexture2D(view.iWidth, view.iHeight, 1, setup.bColourTiling ? Display::E_Format_Tiled | flags : flags, setup.eColourFormat[i], setup.eMultisample, 0, pRenderTexture, view.pColourAddress[i], iSize))
				{
					Display::Device()->ReleaseThreadOwnership();
					return false;
				}
				view.renderTextures[i] = pRenderTexture;
				
				#if TRACK_DISPLAY2_RESOURCES
				Display2::Device()->SetRenderTexture2DName(pRenderTexture, "_DeferredRT");
				#endif

				const Display::C_Surface* pSurface = &pRenderTexture->GetSurface();
				if(setup.bColourTiling)
				{
					view.iColourTileIndex[i] = currentTileIndex;
					view.iColourTileBase[i] = currentTileBase;
					
					view.colourTileInfo[i].name = "DeferredColour";
					Display::Device()->CreateTileFromIndexAndBase(view.iColourTileIndex[i], view.iColourTileBase[i], (setup.eColourAllocHeap == Display::E_AllocHeap_Video) ? CELL_GCM_LOCATION_LOCAL : CELL_GCM_LOCATION_MAIN, pSurface->GetOffset(), iSize, iPitch, CELL_GCM_COMPMODE_DISABLED, 0, &view.colourTileInfo[i]);
					
					U32 alignSize = (iSize+(65535))&~(65535);
					U16 baseSize = alignSize/65536;

					currentTileIndex++;
					currentTileBase += baseSize;

					#if TRACK_DISPLAY2_PROTECTED_MEMORY
					//Display2::Device()->InsertProtectedMemory(iSize, view.pColourAddress[i], "DeferredColour");
					#endif
				}
			}
		}

		view.pDepthStencilSurface = 0;
		view.pDepthStencilTexture = 0;
		if (setup.eDepthStencilFormat != Display::E_Format_Unknown)
		{
			U32 flags = Display::E_Usage_DepthStencil | Display::E_Usage_Indirect;
			
			U32 iPitch = 0, iSize = 0;
			Display2::GetSurfaceSizeAndPitch(alignedWidth, alignedHeight, setup.bDepthTiling ? Display::E_Format_Tiled | flags : flags, Display::GetSurfaceFormatFromFormat(setup.eDepthStencilFormat), (Display2::E_Multisample)setup.eMultisample, iSize, iPitch);
			
			if (!Display::Device()->AllocMemory(65536, iSize, &view.pDepthAddress, setup.eDepthAllocHeap, Display::E_AllocType_Default, Display2::E_MemoryType_Surface))
			{
				BIZ_ASSERT_FAIL("Failed to allocate depth buffer memory!");
				Display::Device()->ReleaseThreadOwnership();
				return false;
			}

			if (setup.eDepthAllocHeap == Display::E_AllocHeap_System)
				flags |= Display::E_Usage_System;

			Display::C_Surface* pSurface = new Display::C_Surface;
			if (!Display::Device()->CreateDepthStencilSurface(view.iWidth, view.iHeight, setup.bDepthTiling ? Display::E_Format_Tiled | flags : flags, setup.eDepthStencilFormat, setup.eMultisample, 0, pSurface, view.pDepthAddress, iSize))
			{
				Display::Device()->ReleaseThreadOwnership();
				return false;
			}
			view.pDepthStencilSurface = pSurface;

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetSurfaceName(pSurface, "_DeferredDS");
			#endif

			U32 iWidthScale = 1;
			U32 iHeightScale = 1;
			if (setup.eMultisample == Display::E_Multisample_2)
			{
				iWidthScale = 2;
			}
			else if (setup.eMultisample == Display::E_Multisample_4)
			{
				iWidthScale = 2;
				iHeightScale = 2;
			}

			Display::C_Texture2D* pTexture2D = new Display::C_Texture2D;
			if (!Display::Device()->CreateTexture2D(view.iWidth * iWidthScale, view.iHeight * iHeightScale, 1, setup.bDepthTiling ? Display::E_Format_Tiled | flags : flags, Display::E_Format_A8R8G8B8, pTexture2D, (U8*)pSurface->GetAddress(), iSize))
			{
				Display::Device()->ReleaseThreadOwnership();
				return false;
			}
			
			pTexture2D->m_gcmTexture.remap =
				CELL_GCM_TEXTURE_REMAP_REMAP << 14 |
				CELL_GCM_TEXTURE_REMAP_REMAP << 12 |
				CELL_GCM_TEXTURE_REMAP_REMAP << 10 |
				CELL_GCM_TEXTURE_REMAP_REMAP << 8 |
				CELL_GCM_TEXTURE_REMAP_FROM_G << 6 |
				CELL_GCM_TEXTURE_REMAP_FROM_R << 4 |
				CELL_GCM_TEXTURE_REMAP_FROM_A << 2 |
				CELL_GCM_TEXTURE_REMAP_FROM_B;
			view.pDepthStencilTexture = pTexture2D;
			
			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetTextureName(pTexture2D, "_DeferredDepthStencil");
			#endif

			if(setup.bDepthTiling)
			{
				view.iDepthTileIndex = currentTileIndex;
				view.iDepthTileBase = currentTileBase;

				view.depthTileInfo.name = "DeferredDepthStencil";
				Display::Device()->CreateTileFromIndexAndBase(view.iDepthTileIndex, view.iDepthTileBase, (setup.eDepthAllocHeap == Display::E_AllocHeap_Video) ? CELL_GCM_LOCATION_LOCAL : CELL_GCM_LOCATION_MAIN, pSurface->GetOffset(), iSize, iPitch, CELL_GCM_COMPMODE_DISABLED, 1, &view.depthTileInfo);
				
				U32 alignSize = (iSize+(65535))&~(65535);
				U16 baseSize = alignSize/65536;

				currentTileIndex++;
				currentTileBase += baseSize;

				#if TRACK_DISPLAY2_PROTECTED_MEMORY
				//Display2::Device()->InsertProtectedMemory(iSize, view.pDepthAddress, "DeferredDepthStencil");
				#endif
			}
			if(setup.eDepthAllocHeap == Display::E_AllocHeap_Video)
			{
				Display::Device()->CreateZCull(view.iZCullIndex, view.iZCullStart, iSize, pSurface->GetOffset(), view.iWidth, view.iHeight, setup.eMultisample);
			}
		}
	}
	//
	// PC and other
	//
#else
	// loop through views and create render targets based on their setups
	for (S32 iView = 0; iView < m_Views.Size(); ++iView)
	{
		C_View& view = m_Views[iView];

		const S32 iSetupIndex = m_SetupMapping[view.iRenderTargetIndex];
		const C_RenderTargetSetup& setup = m_Setups[iSetupIndex];
		Blur2PCAutomation::LogF("DX11 Deferred CreateRT view=%d setup=%d fmt0=%d fmt1=%d depth=%d", view.iIndex, iSetupIndex, setup.eColourFormat[0], setup.eColourFormat[1], setup.eDepthStencilFormat);

		for (S32 i = 0; i < 4; ++i)
		{
			view.renderTextures[i] = 0;
			if (setup.eColourFormat[i] != Display::E_Format_Unknown)
			{
				Display::C_RenderTexture2D* pRenderTexture = new Display::C_RenderTexture2D;	
  
				if (!Display::Device()->CreateRenderTexture2D(view.iWidth, view.iHeight, 1, Display::E_Usage_Colour, setup.eColourFormat[i], setup.eMultisample, 0, pRenderTexture))
				{
					Blur2PCAutomation::LogF("DX11 Deferred CreateRT failed view=%d slot=%d", view.iIndex, i);
					Display::Device()->ReleaseThreadOwnership();
					return false;
				}

				view.renderTextures[i] = pRenderTexture;

				#if TRACK_DISPLAY2_RESOURCES
				Display2::Device()->SetRenderTexture2DName(pRenderTexture, "_DeferredRT");
				#endif
			}
		}

 
		view.pDepthStencilSurface = 0;
		view.pDepthStencilTexture = 0;
		if (setup.eDepthStencilFormat != Display::E_Format_Unknown)
		{
			Display::C_Surface* pSurface = new Display::C_Surface;
			if (!Display::Device()->CreateDepthStencilSurface(view.iWidth, view.iHeight, Display::E_Usage_DepthStencil | Display::E_Usage_Indirect, setup.eDepthStencilFormat, setup.eMultisample, 0, pSurface))
			{
				Blur2PCAutomation::LogF("DX11 Deferred depth surface failed view=%d format=%d", view.iIndex, setup.eDepthStencilFormat);
				Display::Device()->ReleaseThreadOwnership();
				return false;
			}
			view.pDepthStencilSurface = pSurface;

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetSurfaceName(pSurface, "_DeferredDS");
			#endif

			Display::C_Texture2D* pTexture2D = new Display::C_Texture2D;
			if (!Display::Device()->CreateTexture2D(view.iWidth, view.iHeight, 1, Display::E_Usage_Video, setup.eDepthStencilFormat, pTexture2D))
			{
				Blur2PCAutomation::LogF("DX11 Deferred depth texture failed view=%d format=%d", view.iIndex, setup.eDepthStencilFormat);
				Display::Device()->ReleaseThreadOwnership();
				return false;
			}
			view.pDepthStencilTexture = pTexture2D;

			#if TRACK_DISPLAY2_RESOURCES
			Display2::Device()->SetTextureName(pTexture2D, "_DeferredDepthStencil");
			#endif
		}
 

	}
#endif
	Display::Device()->ReleaseThreadOwnership();

	return true;
}

#endif // USE_DEFERRED_RENDER_BUFFERS

}; // namespace Deferred
