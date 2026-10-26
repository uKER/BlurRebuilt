#include <nexus_app/stdafx.h>

//Nexus includes
#include <nexus_app/NexusSystem.h>
#include <nexus_app/DisplayManager.h>

#include "../../resource.h"
#include "..\..\include\nexus_app\NexusProgressDialog.h"
#include "..\..\include\nexus_app\mainfrm.h"
#include <nexus_app/ViewportParent.h>

using namespace Nexus;

C_DisplayManager::C_DisplayManager()
{
	m_isCreated = false;
	m_pSceneRenderer = 0;
}

C_DisplayManager::~C_DisplayManager()
{
	Destroy();
}

Tu::Status C_DisplayManager::Create(CWnd* pWnd)
{
	if (m_isCreated) 
		return Tu::Status::SUCCESS;

	m_pCWnd = pWnd;

	Tu::Status status;
	m_d3dWindow.AttachToWindow(0, m_pCWnd->m_hWnd);

	//Connect the renderer with the rendering settings stored within this class.
	m_pSceneRenderer = new NexusRender::SceneRenderer();
	if (!m_pSceneRenderer)
		return Tu::Status::OUT_OF_MEMORY;

	m_pSceneRenderer->SetSettingsPtr(&m_settings);
	status = m_pSceneRenderer->Create(
		m_d3dWindow.DevicePtr(), 
		g_pNexusSystem->GetCorePtr()->GetAftkScenePtr(),
		&g_pNexusSystem->GetCorePtr()->GetBoundsFilter(),
		g_pNexusSystem->GetCorePtr()->GetPrintMessageCallback(),
		g_pNexusSystem->GetCorePtr()->GetPrintWarningCallback(),
		g_pNexusSystem->GetCorePtr()->GetPrintErrorCallback());
	if (!status) return status;

	//Register callbacks.
	g_pNexusSystem->GetCorePtr()->SetOnSceneChangedCallback(OnUpdateScene);
	g_pNexusSystem->GetCorePtr()->SetOnSceneGeometryChangedCallback(OnUpdateGeometry);
	g_pNexusSystem->GetCorePtr()->SetOnSceneCropSheetDatabaseChangedCallback(OnUpdateCropSheetDatabase);

	//Connect the renderer with the rendering settings stored within this class.
	m_pSceneRenderer->SetSettingsPtr(&m_settings);

	//Create viewports
	for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
	{
		if (!g_pViewportParent->GetViewportPtr(i))
			continue;
		assert(!g_pViewportParent->GetViewportPtr(i)->GetDisplayPtr()->IsCreated());
		g_pViewportParent->GetViewportPtr(i)->GetDisplayPtr()->Create(g_pViewportParent->GetViewportPtr(i));
	}

	m_isCreated = true;

	return Tu::Status::SUCCESS;
}

void C_DisplayManager::Destroy()
{
	if (m_pSceneRenderer)
	{
		m_pSceneRenderer->Destroy();
		delete m_pSceneRenderer;
		m_pSceneRenderer = 0;
	}

	m_d3dWindow.Destroy();
	m_isCreated = false;
}

bool C_DisplayManager::IsCreated() const
{
	return m_isCreated;
}

Tu::Status C_DisplayManager::ResetDevice()
{
	HRESULT hr;
	// Focus lost
	if (!m_pSceneRenderer->PreResetDevice())
	{
		//Try again next time
		return Tu::Status::SUCCESS;
	}

	for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
	{
		//Ensure camera zoom level is accurate.
		if (!g_pViewportParent->GetViewportPtr(i))
			continue;
		g_pViewportParent->GetViewportPtr(i)->GetDisplayPtr()->PreResetDevice();
	}

	hr = m_d3dWindow.HandleLostDevice();

	if (hr == -2005532292)
	{
		//Out of video memory.
		return Tu::Status(Tu::Status::FAILURE, "Out of video memory.");
	}
	else if (FAILED(hr))
	{
		return Tu::Status(Tu::Status::FAILURE, "Unknown D3D error.");
	}
	else
	{
		for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
		{
			//Ensure camera zoom level is accurate.
			if (!g_pViewportParent->GetViewportPtr(i))
				continue;

			g_pViewportParent->GetViewportPtr(i)->GetDisplayPtr()->PostResetDevice();
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status C_DisplayManager::PostResetDevice()
{
	NexusProgressDialog dlg;
	Tu::Status status = dlg.ExecuteTask(UpdateSceneTask);
	return status;
}

Tu::Status C_DisplayManager::ReloadDeviceTask(void* pProgress, const StringArray& arguments)
{
	g_pNexusSystem->GetCorePtr()->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	pProgressSystem->SetTitle("Reloading graphics data...");

	Tu::Status status = g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->PostResetDevice((Progress*)pProgress);
	g_pNexusSystem->GetCorePtr()->Unlock();
	return status;
}

Tu::Status C_DisplayManager::LoadTexturesTask(void* pProgress, const StringArray& arguments)
{
	g_pNexusSystem->GetCorePtr()->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	pProgressSystem->SetTitle("Loading textures...");

	Tu::Status status = g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnLoadTextures((Progress*)pProgress);
	g_pNexusSystem->GetCorePtr()->Unlock();
	return status;
}

Tu::Status C_DisplayManager::LoadCropSheetTexturesTask(void* pProgress, const StringArray& arguments)
{
	g_pNexusSystem->GetCorePtr()->Lock();
	Progress* pProgressSystem = (Progress*)pProgress;
	pProgressSystem->SetTitle("Loading crop sheet textures...");

	Tu::Status status = g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnLoadCropSheetTextures((Progress*)pProgress);
	g_pNexusSystem->GetCorePtr()->Unlock();
	return status;
}

Tu::Status C_DisplayManager::UpdateSceneTask(void* pProgress, const StringArray& arguments)
{
	return OnUpdateScene((Progress*)pProgress);
}

Tu::Status C_DisplayManager::OnLoadTextures()
{
	Tu::Status status;
	if (!m_pSceneRenderer)
	{
		return Tu::Status::FAILURE;
	}

	//If we've enabled texturing and we've not yet loaded any textures, then let's do so.
	if (!m_pSceneRenderer->TexturesLoaded())
	{
		NexusProgressDialog dlg;
		status = dlg.ExecuteTask(LoadTexturesTask);
		if (!status) return status;
	}

	return Tu::Status::SUCCESS;
}

Tu::Status C_DisplayManager::OnLoadCropSheetTextures()
{
	Tu::Status status;
	if (!m_pSceneRenderer)
	{
		return Tu::Status::FAILURE;
	}

	//If we've enabled texturing and we've not yet loaded any textures, then let's do so.
	if (!m_pSceneRenderer->CropSheetTexturesLoaded())
	{
		NexusProgressDialog dlg;
		status = dlg.ExecuteTask(LoadCropSheetTexturesTask);
		if (!status) return status;
	}
	return Tu::Status::SUCCESS;
}

Tu::Status C_DisplayManager::OnUpdateGeometry(Progress* pProgress)
{
	pProgress->Begin();
	Tu::Status status = g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnUpdateGeometry((Progress*)pProgress);
	pProgress->End();
	return status;
}

Tu::Status C_DisplayManager::OnUpdateTextures(Progress* pProgress)
{
	pProgress->Begin();
	Tu::Status status = g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnUpdateTextures((Progress*)pProgress);
	pProgress->End();
	return status;
}

Tu::Status C_DisplayManager::OnUpdateScene(Progress* pProgress)
{
	Tu::Status status;

	status = g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnUpdateScene((Progress*)pProgress);
	if (!status) return status;

	//Attempt to load a crop sheet database if one is available.
	std::string candidateCropSheetFileName(
		Tu::ReplaceFileExtension(g_pNexusSystem->GetCorePtr()->GetSceneFileName(), "cs"));
	if (Tu::FileExists(candidateCropSheetFileName))
	{
		status = g_pViewportParent->GetDisplayManagerPtr()->GetRendererPtr()->OnUpdateCropSheetDatabase(candidateCropSheetFileName);
		if (!status) 
			return status;
	}

	return status;
}

Tu::Status C_DisplayManager::OnUpdateCropSheetDatabase(Progress* pProgress)
{
//	pRender->OnUpdateCropSheet(pProgress);
	return Tu::Status::SUCCESS;
}

void C_DisplayManager::Clear()
{
	if (m_pSceneRenderer)
		m_pSceneRenderer->Clear();
}

const NexusRender::C_D3DWindow* C_DisplayManager::GetD3DWindowPtr() const
{
	return &m_d3dWindow;
}

NexusRender::C_D3DWindow* C_DisplayManager::GetD3DWindowPtr()
{
	return &m_d3dWindow;
}

const NexusRender::SceneRenderer* C_DisplayManager::GetRendererPtr() const
{
	return m_pSceneRenderer;
}

NexusRender::SceneRenderer* C_DisplayManager::GetRendererPtr()
{
	return m_pSceneRenderer;
}

void C_DisplayManager::SetSettings(const NexusRender::SceneRendererSettings& settings)
{
	m_settings = settings;
}

NexusRender::SceneRendererSettings& C_DisplayManager::GetSettings()
{
	return m_settings;
}

const NexusRender::SceneRendererSettings& C_DisplayManager::GetSettings() const
{
	return m_settings;
}

void C_DisplayManager::FrameAll()
{
	for (U32 i = 0; i < NEXUS_NUM_VIEWPORTS; i++)
	{
		if (!g_pViewportParent->GetViewportPtr(i))
			continue;

		g_pViewportParent->GetViewportPtr(i)->GetDisplayPtr()->FrameAll();
	}
}

C_ViewportDisplay* C_DisplayManager::GetViewportDisplayPtr(U32 index)
{
	return g_pViewportParent->GetViewportPtr(index)->GetDisplayPtr();
}
