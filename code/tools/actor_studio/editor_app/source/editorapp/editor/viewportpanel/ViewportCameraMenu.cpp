#include "stdafx.h"

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Core.h>

#include <EditorApp/editor/viewportpanel/ViewportCameraMenu.h>
#include <EditorApp/editor/viewportpanel/ViewportMenuBar.h>
#include <EditorApp/editor/viewportpanel/ViewportPanel.h>
#include <EditorApp/editor/RenderCore.h>
#include <EditorApp/editor/RenderViewport.h>

#include <EditorApp/misc/EventProxy.h>
#include <EditorApp/misc/MenuBuilder.h>
#include <EditorApp/misc/ViewportHelpers.h>

#include <EditorApp/Editor/CameraConnection.h>
#include <EditorApp/Editor/CameraController.h>

#include <EditorApp/Editor/GameMessaging.h>

#include <wx/tokenzr.h>

#include <UserNode/Props/Animation/DgAnimationPropNode.h>

#include <xbdm.h>
using namespace Editor;

C_ViewportCameraMenu::C_ViewportCameraMenu(C_ViewportMenuBar *pMenuBar)
: m_pMenuBar(pMenuBar)
{
}

C_ViewportCameraMenu::~C_ViewportCameraMenu()
{
}

bool C_ViewportCameraMenu::InitialiseUI()
{
	C_MenuBuilderBase *pMenuBuilder = m_pMenuBar->GetMenuBuilder();
	S32 id = -1;

	id = pMenuBuilder->AddMenuEntry("Camera|Perspective",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportCameraMenu::OnSetCameraMode, &C_ViewportCameraMenu::OnUpdateCameraMode);
	m_perspectiveId = id;

	id = pMenuBuilder->AddMenuEntry("Camera|Top",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportCameraMenu::OnSetCameraMode, &C_ViewportCameraMenu::OnUpdateCameraMode);
	m_topId = id;

	id = pMenuBuilder->AddMenuEntry("Camera|Front", 
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportCameraMenu::OnSetCameraMode, &C_ViewportCameraMenu::OnUpdateCameraMode);
	m_frontId = id;

	id = pMenuBuilder->AddMenuEntry("Camera|Side",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportCameraMenu::OnSetCameraMode, &C_ViewportCameraMenu::OnUpdateCameraMode);
	m_sideId = id;

	id = pMenuBuilder->AddMenuEntry("Camera|Attach To Selected Item",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportCameraMenu::OnSetCameraMode, &C_ViewportCameraMenu::OnUpdateCameraMode);
	m_attachId = id;

	pMenuBuilder->AddMenuEntry("Camera|Separator", Editor::E_MenuItem_Separator);
	id = pMenuBuilder->AddMenuEntry("Camera|Synchronise with Game",
		m_pMenuBar->GetViewportPanel(), this, 
		&C_ViewportCameraMenu::OnSetCameraMode, &C_ViewportCameraMenu::OnUpdateCameraSync, E_MenuItem_Check);
	m_syncId = id;

	return true;
}

void C_ViewportCameraMenu::OnSetCameraMode(wxCommandEvent& e)
{
	C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();

	S32 id = e.GetId();		
	if (id == m_attachId)
	{
		EditorLib::C_Selection selection;
		EditorLib::C_Core::Instance().GetSelection(selection);

		if (selection.GetItems().Size() != 1)
			return;

		EditorLib::I_DgNode* pNode = 
			EditorLib::C_Core::Instance().GetDgManager().FindNodeById(selection.GetItems()[0].id);

		if (!pNode)
			return;

		I_CameraController *pController = C_RenderCore::Instance().GetCameraController(pViewport);
		C_CameraConnection& camConnection = *C_RenderCore::Instance().GetCameraConnection();

		bool reconnect = camConnection.GetConnectionState()==C_CameraConnection::E_ConnectionState_Connected &&
							camConnection.GetController() == pController;

		if (pController)
			C_RenderCore::Instance().DestroyCameraController(pController);

		if (pNode->IsA(EditorLib::E_ObjType_SceneNode))
		{
			pController = new C_DgNodeCamera(pViewport, pNode);
			static_cast<C_DgNodeCamera*>(pController)->DgNodeDeserialised(*pNode);
		}
		else if (pNode->IsA(EditorLib::E_ObjType_DgAnimationPropNode))
		{
			const Ut::Vector<U32>& components = selection.GetItems()[0].components;
			if (components.Size())
			{
				pController = new C_DgAnimationPropNodeCamera(pViewport, static_cast<EditorLib::C_DgAnimationPropNode*>(pNode), components[0]);
				static_cast<C_DgAnimationPropNodeCamera*>(pController)->DgNodeDeserialised(*pNode);
			}
			else
				pController = NULL;
		}

		C_RenderCore::Instance().SetCameraController(pViewport, pController);
		
		if (reconnect)
			camConnection.SetController(pController);
	}
	else if (id == m_syncId)
	{
		C_CameraConnection& camConnection = *C_RenderCore::Instance().GetCameraConnection();
		I_CameraController *pController = C_RenderCore::Instance().GetCameraController(pViewport);

		if (camConnection.GetConnectionState() == C_CameraConnection::E_ConnectionState_Connected &&
			pController && pController==camConnection.GetController())
		{
			camConnection.Disconnect();
		}
		else
		{
			pViewport->GetCamera().SetType(AftkRender::C_Camera::E_CameraType_Perspective);
			if (camConnection.GetConnectionState() == C_CameraConnection::E_ConnectionState_Unconnected)
			{
				DWORD ip = 0;
				if (wxGetKeyState(WXK_SHIFT))
					ip = 127<<24 | 1;			//use localhost
 				else
					DmGetAltAddress(&ip);		//get ip of default devkit

				if (ip)
					camConnection.Connect(ip);
			}

			if (!pController)
			{
				//No controller for the current viewport. Create a default one.
				pController = new C_SyncCamera(pViewport);
				C_RenderCore::Instance().SetCameraController(pViewport, pController);
			}
			camConnection.SetController(pController);
		}

				
		C_GameMessaging& gameMessaging = *C_RenderCore::Instance().GetGameMessaging();

		if(gameMessaging.GetConnectionState() == C_GameMessaging::E_ConnectionState_Connected)
		{
			gameMessaging.Disconnect();
		}
		else if (gameMessaging.GetConnectionState() == C_GameMessaging::E_ConnectionState_Unconnected)
		{
			DWORD ip = 0;
			if (wxGetKeyState(WXK_SHIFT))
				ip = 127<<24 | 1;			//use localhost
			else
				DmGetAltAddress(&ip);		//get ip of default devkit

			if (ip)
				gameMessaging.Connect(ip);
		}
		
	}
	else
	{
		I_CameraController *pController = C_RenderCore::Instance().GetCameraController(pViewport);
		C_CameraConnection& camConnection = *C_RenderCore::Instance().GetCameraConnection();

		bool reconnect = camConnection.GetConnectionState()==C_CameraConnection::E_ConnectionState_Connected &&
							camConnection.GetController() == pController;

		if (pController)
			C_RenderCore::Instance().DestroyCameraController(pController);

		if (id == m_perspectiveId)
			C_ViewportHelpers::SetViewportCamera(pViewport, C_ViewportHelpers::E_ViewportView_Perspective);
		else if (id == m_topId)
			C_ViewportHelpers::SetViewportCamera(pViewport, C_ViewportHelpers::E_ViewportView_Top);
		else if (id == m_frontId)
			C_ViewportHelpers::SetViewportCamera(pViewport, C_ViewportHelpers::E_ViewportView_Front);
		else if (id == m_sideId)
			C_ViewportHelpers::SetViewportCamera(pViewport, C_ViewportHelpers::E_ViewportView_Side);
		else
		{
			assert("unknown menu item");
			return;
		}

		//attach a new controller
		pController = new C_SyncCamera(pViewport);
		C_RenderCore::Instance().SetCameraController(pViewport, pController);

		if (reconnect)
			camConnection.SetController(pController);		
	}
}

void C_ViewportCameraMenu::OnUpdateCameraMode(wxUpdateUIEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
}

void C_ViewportCameraMenu::OnUpdateCameraSync(wxUpdateUIEvent& e)
{
	const C_RenderViewport *pViewport = m_pMenuBar->GetViewportPanel()->GetViewport();
	C_CameraConnection& camConnection = *C_RenderCore::Instance().GetCameraConnection();
	const I_CameraController *pController = C_RenderCore::Instance().GetCameraController(pViewport);
	
	e.Check(camConnection.GetConnectionState()==C_CameraConnection::E_ConnectionState_Connected &&
			pController && pController == camConnection.GetController());
}
