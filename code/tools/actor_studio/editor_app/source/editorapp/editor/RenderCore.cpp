#include "stdafx.h"

#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/RenderViewport.h>

#include <EditorApp/Editor/CameraConnection.h>
#include <EditorApp/Editor/CameraController.h>
#include <EditorApp/Editor/GameMessaging.h>

#include <EditorApp/Editor/Manipulators/Manipulator.h>

#include <EditorApp/App.h>
#include <EditorApp/Properties/CameraProperties.h>
#include <EditorApp/Components/CameraPropertyPanel.h>

namespace Editor
{


C_RenderCore::C_RenderCore()
: m_pRenderApplication(0)
, m_pCameraConnection(0)
, m_pRenderScene(0)
, m_pRenderPipeline(0)
, m_pManipulatorManager(0)
, m_pGameMessaging(0)
{
}

C_RenderCore::~C_RenderCore()
{
	Shutdown();
}

bool C_RenderCore::Initialise(HWND hwnd)
{
	if (m_pRenderApplication)
		return true;

	m_pRenderApplication = new AftkRender::C_Application();
	if (!m_pRenderApplication)
		return false;

	m_pRenderApplication->m_DeviceParams.m_Multisample = Display::E_Multisample_None;
	m_pRenderApplication->m_DeviceParams.m_hWnd = hwnd;

	RECT r;
	if (!::GetWindowRect(hwnd, &r))
	{
		SAFE_DELETE_PTR(m_pRenderApplication);
		return false;
	}

	if (!m_pRenderApplication->Create(r.right-r.left, r.bottom-r.top))
	{
		SAFE_DELETE_PTR(m_pRenderApplication);
		return false;
	}

	C_CameraConnection::NetworkInit();
	m_pCameraConnection = new C_CameraConnection();

	C_GameMessaging::NetworkInit();
	m_pGameMessaging = new C_GameMessaging();

	m_pRenderScene = new C_RenderScene();
	m_pRenderPipeline = new C_RenderPipeline();

	m_pRenderPipeline->CreateRenderers();

	m_pManipulatorManager = new C_ManipulatorManager();

	return true;
}

bool C_RenderCore::Shutdown()
{
	if (m_pRenderApplication)
	{
		m_pRenderPipeline->DestroyRenderers();

		while (m_pViewports.Size())
			UnregisterViewport(m_pViewports[0]);

		m_pCameraConnection->Disconnect();
		C_CameraConnection::NetworkExit();
		SAFE_DELETE_PTR(m_pCameraConnection);

		m_pGameMessaging->Disconnect();
		C_GameMessaging::NetworkExit();
		SAFE_DELETE_PTR(m_pGameMessaging);

		SAFE_DELETE_PTR(m_pRenderApplication);

		SAFE_DELETE_PTR(m_pRenderScene);
		SAFE_DELETE_PTR(m_pRenderPipeline);

		SAFE_DELETE_PTR(m_pManipulatorManager);
	}
	return true;
}

AftkRender::C_ViewContext* C_RenderCore::RegisterViewport(C_RenderViewport* pViewport)
{
	AftkRender::C_ViewContext *pContext = m_pRenderApplication->GetViewContextFromHWND((HWND)pViewport->GetHWND());
	if (pContext)
	{
		bool found = false;
		for (S32 i=0; i<m_pViewports.Size(); ++i)
			if (m_pViewports[i] == pViewport)
				found = true;

		if (!found)
		{
			m_pViewports.PushBack(pViewport);
			m_pCameraControllers.PushBack((I_CameraController*)0);
		}
		return pContext;
	}

	wxSize size = pViewport->GetSize();

	Display::C_DeviceParams params;
	params.m_Width = size.GetX();
	params.m_Height = size.GetY();
	params.m_Multisample = Display::E_Multisample_None;
	params.m_hWnd = (HWND) pViewport->GetHandle();

	m_pViewports.PushBack(pViewport);
	m_pCameraControllers.PushBack((I_CameraController*)0);

	return m_pRenderApplication->AddViewContext(params);
}


void C_RenderCore::UnregisterViewport(Editor::C_RenderViewport *pViewport)
{
	S32 index = -1;
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		if (pViewport == m_pViewports[i])
			index = i;

	if (index != -1)
	{
		m_pRenderApplication->DestroyViewContext(&pViewport->GetViewContext());
		pViewport->m_pViewContext = 0;
		m_pViewports.EraseNonOrdered(index);

		SAFE_DELETE_PTR(m_pCameraControllers[index]);
		m_pCameraControllers.EraseNonOrdered(index);
	}
}

bool C_RenderCore::ResizeViewport(U32 w, U32 h, Editor::C_RenderViewport *pViewport)
{
	if (m_pRenderApplication)
		return m_pRenderApplication->Resize(w, h, &pViewport->GetViewContext());

	return false;
}

bool C_RenderCore::BeginRendering(const C_RenderViewport *pViewport)
{
	I_CameraController *pController = GetCameraController(pViewport);
	if (pController)
	{
		pController->Update();

		if (pController == m_pCameraConnection->GetController())
		{
			CameraMessage msg;
			if (pController->Send(&msg))
				m_pCameraConnection->Send(msg);
		}
	}
	return m_pRenderApplication->BeginRendering(&pViewport->GetViewContext());
}

void C_RenderCore::EndRendering(const C_RenderViewport *pViewport)
{
	m_pRenderApplication->EndRendering(&pViewport->GetViewContext());
}

const RenderViewportArray& C_RenderCore::GetRenderViewports() const
{
	return m_pViewports;
}

void C_RenderCore::DirtyAllViewports()
{
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		m_pViewports[i]->SetDirty(true);
}

void C_RenderCore::RefreshAllViewports()
{
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		m_pViewports[i]->Render();
}

C_CameraConnection* C_RenderCore::GetCameraConnection()
{
	return m_pCameraConnection;
}

I_CameraController* C_RenderCore::GetCameraController(const C_RenderViewport *pViewport)
{
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		if (m_pViewports[i] == pViewport)
			return m_pCameraControllers[i];

	return 0;
}

const I_CameraController* C_RenderCore::GetCameraController(const C_RenderViewport *pViewport) const
{
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		if (m_pViewports[i] == pViewport)
			return m_pCameraControllers[i];

	return 0;
}

void C_RenderCore::SetCameraController(const C_RenderViewport* pViewport, I_CameraController *pController)
{
	S32 index = -1;
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		if (m_pViewports[i] == pViewport)
			index = i;

	if (index != -1)
	{
		delete m_pCameraControllers[index];
		m_pCameraControllers[index] = pController;
	}
}

void C_RenderCore::DestroyCameraController(I_CameraController *pController)
{
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		if (m_pCameraControllers[i] == pController)
			SAFE_DELETE_PTR(m_pCameraControllers[i]);

	if (m_pCameraConnection && m_pCameraConnection->GetController() == pController)
		m_pCameraConnection->SetController(0);
}

void C_RenderCore::DestroyAllCameraControllers()
{
	for (S32 i=0; i<m_pViewports.Size(); ++i)
		SAFE_DELETE_PTR(m_pCameraControllers[i]);

	if (m_pCameraConnection)
		m_pCameraConnection->SetController(0);
}

C_GameMessaging* C_RenderCore::GetGameMessaging()
{
	return m_pGameMessaging;
}

C_RenderScene& C_RenderCore::GetRenderScene()
{
	return *m_pRenderScene;
}

const C_RenderScene& C_RenderCore::GetRenderScene() const
{
	return *m_pRenderScene;
}

C_RenderPipeline& C_RenderCore::GetRenderPipeline()
{
	return *m_pRenderPipeline;
}

const C_RenderPipeline& C_RenderCore::GetRenderPipeline() const
{
	return *m_pRenderPipeline;
}

C_ManipulatorManager& C_RenderCore::GetManipulatorManager()
{
	return *m_pManipulatorManager;
}

const C_ManipulatorManager&	C_RenderCore::GetManipulatorManager() const
{
	return *m_pManipulatorManager;
}

void C_RenderCore::FrameSelection(C_RenderViewport *pViewport) const
{
	const Maths::C_AxisBox& aabb = m_pRenderScene->GetSelectionBounds();
	if (aabb.m_Min[0] > aabb.m_Max[0])
		return FrameScene(pViewport);

	AftkRender::C_Camera& cam = pViewport->GetCamera();
	Maths::HVec3 viewVector = cam.GetFocusPosition() - cam.GetPosition();
	viewVector.Normalise();
	if (cam.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
	{
		AftkRender::Utility::FrameObjectPerspective(cam, aabb, viewVector);
		cam.SetPosition(cam.GetPosition() - 2.f*viewVector);
	}
	else
	{
		AftkRender::Utility::FrameObjectOrthographic(cam, aabb, viewVector);
		//move camera out of scene...
		{
			F32 radius = 10.f;
			{
				const Maths::C_AxisBox& worldaabb = m_pRenderScene->GetWorldBounds();

				if (worldaabb.m_Min[0] < worldaabb.m_Max[0])
					radius = 0.5f * worldaabb.Diagonal().Length();
			}
			cam.SetPosition(cam.GetFocusPosition() - radius*viewVector);
		}
		cam.SetHeightExtent(cam.GetHeightExtent() + 2.f);
	}
	pViewport->Refresh();
	C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
		? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
	if (pProperties)
		pProperties->AttachViewport(pViewport);
}

void C_RenderCore::FrameScene(C_RenderViewport *pViewport) const
{
	Maths::C_AxisBox worldaabb = m_pRenderScene->GetWorldBounds();
	if (worldaabb.m_Min[0] > worldaabb.m_Max[0])
	{
		worldaabb.m_Min[0] = worldaabb.m_Min[1] = worldaabb.m_Min[2] = -10.f;
		worldaabb.m_Max[0] = worldaabb.m_Max[1] = worldaabb.m_Max[2] = 10.f;
	}

	AftkRender::C_Camera& cam = pViewport->GetCamera();
	Maths::HVec3 viewVector = cam.GetFocusPosition() - cam.GetPosition();
	viewVector.Normalise();
	if (cam.GetType() == AftkRender::C_Camera::E_CameraType_Perspective)
	{
		AftkRender::Utility::FrameObjectPerspective(cam, worldaabb, viewVector);
		cam.SetPosition(cam.GetPosition() - 2.f*viewVector);
	}
	else
	{
		AftkRender::Utility::FrameObjectOrthographic(cam, worldaabb, viewVector);
		cam.SetHeightExtent(cam.GetHeightExtent() + 2.f);
	}

	pViewport->Refresh();
	C_CameraProperties *pProperties = wxGetApp().GetCameraPropertyPanel() 
		? wxGetApp().GetCameraPropertyPanel()->GetCameraPropertiesPtr() : 0;
	if (pProperties)
		pProperties->AttachViewport(pViewport);
}

}