/////////////////////////////////////////////////////////////
// LiveryEditorUI
/////////////////////////////////////////////////////////////

#include "VehiclePCH.h"

#include "Vehicle/Livery/LiveryEditorUI.h"
#include "Vehicle/Livery/LiveryEditor.h"
#include "Vehicle/Livery/Debug.h"

#include <camera2.h>
#include <camera2/stock/FlyCamera.h>

#include "Framework/Application/UserControls.h"
#include "Framework/Application/Application.h"

#include <Input/Device.h>
#include <Input/DeviceManager.h>
#include <Input/GameInputMapping.h>

#if defined(TURN_OFF_VEHICLE_OPTIMISATIONS)
	#pragma optimize("", off)
#endif

namespace Vehicle
{

namespace Livery
{

/* 
===============================================================
					class C_LiveryEditorUI

NOTES: 
===============================================================
*/

C_LiveryEditorUI C_LiveryEditorUI::s_instance;

/*
===============
NAME: C_LiveryEditorUI

NOTE: 
===============
*/
C_LiveryEditorUI::C_LiveryEditorUI()
{
	m_pCamera		= NULL;
}

/*
===============
NAME: ~C_LiveryEditorUI

NOTE: 
===============
*/
C_LiveryEditorUI::~C_LiveryEditorUI()
{
	
}

/*
===============
NAME: Initialise

NOTE: Initialise
===============
*/
void C_LiveryEditorUI::Initialise()
{
	assert(!m_pCamera);

	m_pCamera						= new Cam2::C_FlyCamera;

	Cam2::C_DebugFeed& debugFeed	= Framework::C_Application::GetInstance()->GetDebugFeed(0);
	debugFeed.ShowFeed(m_pCamera);

	m_CameraPosition	= Maths::HVec3(4.0f, 1.5f, -3.0f);
	m_CameraLookAt		= Maths::HVec3(0.0f, 0.0f, 0.0f);

	m_pCamera->SetPosition(m_CameraPosition);
	m_pCamera->LookAt(m_CameraLookAt);

	m_Mode							= E_UIMode_Menu;
	m_EditMode						= E_UIEditMode_None;

	m_MenuActive					= false;
	m_CameraRotateActive			= true;
	m_CameraZoomActive				= true;
		
	//Create menu hierarchy
	{
		C_Menu* pMainMenu = new C_Menu();
		_snprintf(pMainMenu->m_Title.m_String, lengthof(pMainMenu->m_Title.m_String), "Main Menu");
		m_pMenus.PushBack(pMainMenu);

		m_pActiveMenu = pMainMenu;

		//Items
		{
			C_MenuItem* pItem = new C_MenuItem();
			_snprintf(pItem->m_String, lengthof(pItem->m_String), "Cycle Canvas");	
			pItem->m_Action = E_UIMenuAction_CycleCanvas;
			pMainMenu->m_pItems.PushBack(pItem);
			pItem->m_Selected = true;
			m_pActiveMenu->Reset();
		}

		//Sub-menus
		{
			C_Menu* pEditLayer = new C_Menu();
			_snprintf(pEditLayer->m_Title.m_String, lengthof(pEditLayer->m_Title.m_String), "Edit Layer");
			pEditLayer->m_pParent = pMainMenu;
			m_pMenus.PushBack(pEditLayer);
			pMainMenu->m_pSubMenus.PushBack(pEditLayer);

			//Items
			{
				C_MenuItem* pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Add Layer");	
				pItem->m_Action = E_UIMenuAction_AddLayer;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Promote Layer");	
				pItem->m_Action = E_UIMenuAction_LayerPromote;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Demote Layer");	
				pItem->m_Action = E_UIMenuAction_LayerDemote;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Move Layer");	
				pItem->m_Action = E_UIMenuAction_LayerMove;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Scale Layer");	
				pItem->m_Action = E_UIMenuAction_LayerScale;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Rotate Layer");	
				pItem->m_Action = E_UIMenuAction_LayerRotate;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Change Colour");	
				pItem->m_Action = E_UIMenuAction_ChangeColour;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Copy Layer");	
				pItem->m_Action = E_UIMenuAction_CopyLayer;
				pEditLayer->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Paste Layer");	
				pItem->m_Action = E_UIMenuAction_PasteLayer;
				pEditLayer->m_pItems.PushBack(pItem);
			}

			//Sub-menu - "Change Texture"
			{	
				C_Menu* pChangeTexture = new C_Menu();
				_snprintf(pChangeTexture->m_Title.m_String, lengthof(pChangeTexture->m_Title.m_String), "Change Texture");	
				pChangeTexture->m_pParent = pEditLayer;
				m_pMenus.PushBack(pChangeTexture);
				pEditLayer->m_pSubMenus.PushBack(pChangeTexture);

				C_MenuItem* pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Next");	
				pItem->m_Action = E_UIMenuAction_NextTexture;
				pChangeTexture->m_pItems.PushBack(pItem);

				pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Previous");	
				pItem->m_Action = E_UIMenuAction_PreviousTexture;
				pChangeTexture->m_pItems.PushBack(pItem);
			}

			//Sub-menu - "Remove Layer"
			{	
				C_Menu* pRemoveLayer = new C_Menu();
				_snprintf(pRemoveLayer->m_Title.m_String, lengthof(pRemoveLayer->m_Title.m_String), "Remove Layer");	
				pRemoveLayer->m_pParent = pEditLayer;
				m_pMenus.PushBack(pRemoveLayer);
				pEditLayer->m_pSubMenus.PushBack(pRemoveLayer);

				C_MenuItem* pItem = new C_MenuItem();
				_snprintf(pItem->m_String, lengthof(pItem->m_String), "Are you sure?");	
				pItem->m_Action = E_UIMenuAction_RemoveLayer;
				pItem->m_Colour = Maths::RGBA(255, 0, 0, 255);
				pRemoveLayer->m_pItems.PushBack(pItem);
			}
		}

		//Sub-menu - "Save"
		{	
			C_Menu* pSave = new C_Menu();
			_snprintf(pSave->m_Title.m_String, lengthof(pSave->m_Title.m_String), "Save");	
			pSave->m_pParent = pMainMenu;
			m_pMenus.PushBack(pSave);
			pMainMenu->m_pSubMenus.PushBack(pSave);

			C_MenuItem* pItem = new C_MenuItem();
			_snprintf(pItem->m_String, lengthof(pItem->m_String), "Overwite 'dontship/liveries/temp_livery.livery' - Are you sure?");
			pItem->m_Action = E_UIMenuAction_Save;
			pItem->m_Colour = Maths::RGBA(255, 0, 0, 255);
			pSave->m_pItems.PushBack(pItem);
		}

		//Sub-menu - "Load"
		{	
			C_Menu* pLoad = new C_Menu();
			_snprintf(pLoad->m_Title.m_String, lengthof(pLoad->m_Title.m_String), "Load");	
			pLoad->m_pParent = pMainMenu;
			m_pMenus.PushBack(pLoad);
			pMainMenu->m_pSubMenus.PushBack(pLoad);

			C_MenuItem* pItem = new C_MenuItem();
			_snprintf(pItem->m_String, lengthof(pItem->m_String), "Load 'dontship/liveries/temp_livery.livery' - Are you sure?");
			pItem->m_Action = E_UIMenuAction_Load;
			pItem->m_Colour = Maths::RGBA(255, 0, 0, 255);
			pLoad->m_pItems.PushBack(pItem);
		}
	}
}

/*
===============
NAME: Shutdown

NOTE: Shutdown
===============
*/
void C_LiveryEditorUI::Shutdown()
{
	Framework::C_Application::GetInstance()->GetDebugFeed(0).ShowFeed(0);

	if (m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = NULL;
	}

	for (int iMenu = 0; iMenu < m_pMenus.Size(); iMenu++)
	{
		for (int iItem = 0; iItem < m_pMenus[iMenu]->m_pItems.Size(); iItem++)
		{
			delete m_pMenus[iMenu]->m_pItems[iItem];
		}

		m_pMenus[iMenu]->m_pItems.Clear();

		delete m_pMenus[iMenu];
	}

	m_pMenus.Clear();

	m_pActiveMenu = NULL;
}

/*
===============
NAME: Update

NOTE: Update
===============
*/
void C_LiveryEditorUI::Update()
{
	UpdateCamera();

	C_LiveryEditor::Instance().Update();
}

/*
===============
NAME: UpdateControls

NOTE: Updates camera/edit controls
===============
*/
void C_LiveryEditorUI::UpdateControls()
{
	if (Input::GetGameInput(0)->GetButtonAutoRepeat(E_LiveryAppButton_Toggle, 10, 5))
	{
		if (m_Mode != E_UIMode_Menu)
			m_Mode = E_UIMode_Menu;
		else
			m_Mode = E_UIMode_Fly;
	}
	if (Input::GetGameInput(0)->GetButtonAutoRepeat(E_LiveryAppButton_NextLayer, 10, 5))
	{
		C_LiveryEditor::Instance().NextLayer();
	}
	else if (Input::GetGameInput(0)->GetButtonAutoRepeat(E_LiveryAppButton_PrevLayer, 10, 5))
	{
		C_LiveryEditor::Instance().PreviousLayer();
	}
	
	m_CameraRotateActive = true;
	m_CameraZoomActive = true;

	switch (m_Mode)
	{
	case E_UIMode_Fly:
		{
			//Menu is not active in fly-mode
			m_MenuActive = false;

			//Fly controls			
		}
		break;
	case E_UIMode_Menu:
		{
			//Menu is active when, er, in menu mode
			m_MenuActive = true;

			if (Input::GetGameInput(0)->GetButtonAutoRepeat(E_LiveryAppButton_MenuDown, 10, 5))
			{
				for (int iItem = 0; iItem < m_pActiveMenu->m_pItems.Size(); iItem++)
					m_pActiveMenu->m_pItems[iItem]->m_Selected = false;
				for (int iMenu = 0; iMenu < m_pActiveMenu->m_pSubMenus.Size(); iMenu++)
					m_pActiveMenu->m_pSubMenus[iMenu]->m_Selected = false;

				m_pActiveMenu->m_Index++;

				if (m_pActiveMenu->m_Index < m_pActiveMenu->m_pItems.Size())
				{
					m_pActiveMenu->m_pItems[m_pActiveMenu->m_Index]->m_Selected = true;
					m_pActiveMenu->m_pActiveItem = m_pActiveMenu->m_pItems[m_pActiveMenu->m_Index];
					m_pActiveMenu->m_pActiveSubMenu = NULL;
				}
				else 
				{
					int index = m_pActiveMenu->m_Index - m_pActiveMenu->m_pItems.Size();
					if (index < m_pActiveMenu->m_pSubMenus.Size())
					{
						m_pActiveMenu->m_pSubMenus[index]->m_Selected = true;
						m_pActiveMenu->m_pActiveItem = NULL;
						m_pActiveMenu->m_pActiveSubMenu = m_pActiveMenu->m_pSubMenus[index];
					}
					else
					{
						m_pActiveMenu->m_Index = 0;
						if (m_pActiveMenu->m_pItems.Size() != 0)
						{
							m_pActiveMenu->m_pItems[0]->m_Selected = true;
							m_pActiveMenu->m_pActiveItem = m_pActiveMenu->m_pItems[m_pActiveMenu->m_Index];
							m_pActiveMenu->m_pActiveSubMenu = NULL;
						}
						else if (m_pActiveMenu->m_pSubMenus.Size() != 0)
						{
							m_pActiveMenu->m_pSubMenus[0]->m_Selected = true;
							m_pActiveMenu->m_pActiveItem = NULL;
							m_pActiveMenu->m_pActiveSubMenu = m_pActiveMenu->m_pSubMenus[index];
						}
					}
				}
			}
			
			if (Input::GetGameInput(0)->GetButtonAutoRepeat(E_LiveryAppButton_MenuUp, 10, 5))
			{
				for (int iItem = 0; iItem < m_pActiveMenu->m_pItems.Size(); iItem++)
					m_pActiveMenu->m_pItems[iItem]->m_Selected = false;
				for (int iMenu = 0; iMenu < m_pActiveMenu->m_pSubMenus.Size(); iMenu++)
					m_pActiveMenu->m_pSubMenus[iMenu]->m_Selected = false;

				m_pActiveMenu->m_Index--;

				if (m_pActiveMenu->m_Index >= 0 && m_pActiveMenu->m_Index < m_pActiveMenu->m_pItems.Size())
				{
					m_pActiveMenu->m_pItems[m_pActiveMenu->m_Index]->m_Selected = true;
					m_pActiveMenu->m_pActiveSubMenu = NULL;
					m_pActiveMenu->m_pActiveItem = m_pActiveMenu->m_pItems[m_pActiveMenu->m_Index];

				}
				else if (m_pActiveMenu->m_Index >= 0)
				{
					int index = m_pActiveMenu->m_Index - m_pActiveMenu->m_pItems.Size();
					if (index < m_pActiveMenu->m_pSubMenus.Size())
					{
						m_pActiveMenu->m_pSubMenus[index]->m_Selected = true;
						m_pActiveMenu->m_pActiveSubMenu = m_pActiveMenu->m_pSubMenus[index];
						m_pActiveMenu->m_pActiveItem = NULL;
					}					
				}
				else
				{
					m_pActiveMenu->m_Index = 0;
					if (m_pActiveMenu->m_pSubMenus.Size() != 0)
					{
						int index = m_pActiveMenu->m_pSubMenus.Size()-1;
						m_pActiveMenu->m_pSubMenus[index]->m_Selected = true;
						m_pActiveMenu->m_pActiveItem = NULL;
						m_pActiveMenu->m_pActiveSubMenu = m_pActiveMenu->m_pSubMenus[index];
						m_pActiveMenu->m_Index = m_pActiveMenu->m_pItems.Size() + index;
					}
					else if (m_pActiveMenu->m_pItems.Size() != 0)
					{
						int index = m_pActiveMenu->m_pItems.Size()-1;						
						m_pActiveMenu->m_pItems[index]->m_Selected = true;
						m_pActiveMenu->m_pActiveItem = m_pActiveMenu->m_pItems[index];
						m_pActiveMenu->m_pActiveSubMenu = NULL;
						m_pActiveMenu->m_Index = index;
					} 
				}
			}

			if (Input::GetGameInput(0)->GetButtonAutoRepeat(E_LiveryAppButton_MenuNext, 10, 5))
			{
				if (m_pActiveMenu->m_pActiveItem)
					ProcessAction();
				else if (m_pActiveMenu->m_pActiveSubMenu)
				{
					m_pActiveMenu = m_pActiveMenu->m_pActiveSubMenu;
					m_pActiveMenu->Reset();
				}
			}

			if (Input::GetGameInput(0)->GetButtonAutoRepeat(E_LiveryAppButton_MenuBack, 10, 5))
			{
				if (m_pActiveMenu->m_pParent)
				{
					m_pActiveMenu = m_pActiveMenu->m_pParent;
				}
			}
		}
		break;
	case E_UIMode_Edit:
		{
			//Menu is not active in edit-mode
			m_MenuActive = false;

			UpdateEditMode();
		}
		break;	
	}
}

/*
===============
NAME: ProcessAction

NOTE: Processes an action
===============
*/
void C_LiveryEditorUI::ProcessAction()
{
	if (m_Mode != E_UIMode_Menu || !m_pActiveMenu || !m_pActiveMenu->m_pActiveItem)
		return;

	switch (m_pActiveMenu->m_pActiveItem->m_Action)
	{
	case E_UIMenuAction_CycleCanvas:
		{
			//Cycle between canvases
			int cCanvas = (int)C_LiveryEditor::Instance().m_CurrentCanvas;
			cCanvas++;
			if (cCanvas >= E_Canvas_Max)
				cCanvas = E_Canvas_Front;
			C_LiveryEditor::Instance().m_CurrentCanvas = (E_Canvas)cCanvas;
			C_LiveryEditor::Instance().SetActiveCanvas(C_LiveryEditor::Instance().m_CurrentCanvas);
			C_LiveryEditorUI::Instance().SetViewDefaults();
		}
		break;
	case E_UIMenuAction_AddLayer:
		{
			C_LiveryEditor::Instance().AddLayer();
		}
		break;
	case E_UIMenuAction_RemoveLayer:
		{
			if (C_LiveryEditor::Instance().GetActiveCanvas())
				C_LiveryEditor::Instance().GetActiveCanvas()->RemoveLayer();
		}
		break;
	case E_UIMenuAction_LayerPromote:
		{
			if (C_LiveryEditor::Instance().GetActiveCanvas())
				C_LiveryEditor::Instance().GetActiveCanvas()->PromoteLayer();
		}
		break;
	case E_UIMenuAction_LayerDemote:
		{
			if (C_LiveryEditor::Instance().GetActiveCanvas())
				C_LiveryEditor::Instance().GetActiveCanvas()->DemoteLayer();
		}
		break;
	case E_UIMenuAction_NextTexture:
		{
			if (C_LiveryEditor::Instance().GetResource() && C_LiveryEditor::Instance().GetActiveCanvas() && C_LiveryEditor::Instance().GetActiveCanvas()->GetActiveLayer())
			{
				C_Layer* pLayer = C_LiveryEditor::Instance().GetActiveCanvas()->GetActiveLayer();
				pLayer->m_TextureIndex++;
				int resourceCount = C_LiveryEditor::Instance().GetResource()->ResourceCount();
				if (pLayer->m_TextureIndex > resourceCount)
					pLayer->m_TextureIndex = 0;
				pLayer->m_pLiveryTexture = C_LiveryEditor::Instance().m_pLiveryResource->GetTexture2D(pLayer->m_TextureIndex);
			}
		}
		break;
	case E_UIMenuAction_PreviousTexture:
		{
			if (C_LiveryEditor::Instance().GetResource() && C_LiveryEditor::Instance().GetActiveCanvas() && C_LiveryEditor::Instance().GetActiveCanvas()->GetActiveLayer())
			{
				C_Layer* pLayer = C_LiveryEditor::Instance().GetActiveCanvas()->GetActiveLayer();
				pLayer->m_TextureIndex--;
				int resourceCount = C_LiveryEditor::Instance().GetResource()->ResourceCount();
				if (pLayer->m_TextureIndex < 0)
					pLayer->m_TextureIndex = resourceCount - 1;
				pLayer->m_pLiveryTexture = C_LiveryEditor::Instance().m_pLiveryResource->GetTexture2D(pLayer->m_TextureIndex);
			}
		}
		break;
	case E_UIMenuAction_Save:
		{
			C_LiveryEditor::Instance().Save("dontship/liveries/", "temp_livery.livery");
		}
		break;
	case E_UIMenuAction_Load:
		{
			C_LiveryEditor::Instance().Load("dontship/liveries/", "temp_livery.livery");
		}
		break;
	case E_UIMenuAction_CopyLayer:
		{
			C_LiveryEditor::Instance().CopyLayer();
		}
		break;
	case E_UIMenuAction_PasteLayer:
		{
			C_LiveryEditor::Instance().PasteLayer();
		}
		break;
	case E_UIMenuAction_LayerMove:
		{
			m_Mode = E_UIMode_Edit;
			m_EditMode = E_UIEditMode_EditMove;
		}
		break;
	case E_UIMenuAction_LayerScale:
		{
			m_Mode = E_UIMode_Edit;
			m_EditMode = E_UIEditMode_EditScale;
		}
		break;
	case E_UIMenuAction_LayerRotate:
		{
			m_Mode = E_UIMode_Edit;
			m_EditMode = E_UIEditMode_EditRotate;
		}
	case E_UIMenuAction_ChangeColour:
		{
			m_Mode = E_UIMode_Edit;
			m_EditMode = E_UIEditMode_EditColour;
		}
		break;
	}
}

/*
===============
NAME: UpdateEditMode

NOTE: Updates edit-mode controls
===============
*/
void C_LiveryEditorUI::UpdateEditMode()
{
	if (m_Mode != E_UIMode_Edit)
		return;

	if (!C_LiveryEditor::Instance().GetActiveCanvas())
		return;

	C_Layer* pLayer = C_LiveryEditor::Instance().GetActiveCanvas()->GetActiveLayer();

	if (!pLayer)
		return;

	switch (m_EditMode)
	{
	case E_UIEditMode_EditMove:
		{
			m_CameraZoomActive = false;

			float dx = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeRX);
			float dy = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeRY);

			float slowSpeed = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedSlow);
			float fastSpeed = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedFast);
			
			float scale = 0.01f;

			if (slowSpeed > 0.0f)
				scale = Maths::Lerp(scale, 0.001f, slowSpeed);	
			else if (fastSpeed > 0.0f)
				scale = Maths::Lerp(scale, 0.1f, fastSpeed);

			pLayer->m_Position.x += dx * scale;
			pLayer->m_Position.y -= dy * scale;

			bool loob = true;
		}
		break;
	case E_UIEditMode_EditScale:
		{
			m_CameraZoomActive = false;

			float dx = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeRX);
			float dy = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeRY);

			float slowSpeed = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedSlow);
			float fastSpeed = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedFast);
			
			float scale = 0.01f;

			if (slowSpeed > 0.0f)
				scale = Maths::Lerp(scale, 0.001f, slowSpeed);	
			else if (fastSpeed > 0.0f)
				scale = Maths::Lerp(scale, 0.1f, fastSpeed);

			pLayer->m_Size.x += dx * scale;
			pLayer->m_Size.y += dy * scale;

			bool loob = true;
		}
		break;
	case E_UIEditMode_EditRotate:
		{
			float dx = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeRX);
			float dy = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeRY);

			bool loob = true;
		}
		break;
	case E_UIEditMode_EditColour:
		{
			m_CameraZoomActive = false;
			m_CameraRotateActive = false;

			if (!m_pActiveMenu->m_pActiveItem)
				break;

			float dx = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeLX);
			
			float slowSpeed = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedSlow);
			float fastSpeed = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedFast);

			float scale = 1.0f;

			if (slowSpeed > 0.0f)
				scale = Maths::Lerp(scale, 0.1f, slowSpeed);	
			else if (fastSpeed > 0.0f)
				scale = Maths::Lerp(scale, 5.0f, fastSpeed);

			if (Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_Back) > 0.5f)
			{
				pLayer->m_Colour.x += dx * scale;
				pLayer->m_Colour.x = Maths::Clamp(pLayer->m_Colour.x, 0.0f, 255.0f);
			}
			else if (Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_Proceed) > 0.5f)
			{
				pLayer->m_Colour.y += dx * scale;
				pLayer->m_Colour.y = Maths::Clamp(pLayer->m_Colour.y, 0.0f, 255.0f);
			}
			else if (Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_Blue) > 0.5f)
			{
				pLayer->m_Colour.z += dx * scale;
				pLayer->m_Colour.z = Maths::Clamp(pLayer->m_Colour.z, 0.0f, 255.0f);
			}
			
			if (Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_MenuUp))
			{
				pLayer->m_Colour.w += scale;
				pLayer->m_Colour.w = Maths::Clamp(pLayer->m_Colour.w, 0.0f, 255.0f);
			}
			else if (Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_MenuDown))
			{
				pLayer->m_Colour.w -= scale;
				pLayer->m_Colour.w = Maths::Clamp(pLayer->m_Colour.w, 0.0f, 255.0f);
			}

			bool loob = true;
		}
		break;
	}
}

/*
===============
NAME: SetViewDefaults

NOTE: Sets view defaults
===============
*/
void C_LiveryEditorUI::SetViewDefaults()
{
	if (!m_pCamera)
		return;

	switch (C_LiveryEditor::Instance().m_CurrentCanvas)
	{
	case E_Canvas_Front:
		m_CameraPosition = Maths::HVec3(0.0f, 1.0f, -4.0f);
		m_CameraLookAt = Maths::HVec3(0.0f, 0.0f, 0.0f);
		break;

	case E_Canvas_Rear:
		m_CameraPosition = Maths::HVec3(0.0f, 1.0f, 4.0f);
		m_CameraLookAt = Maths::HVec3(0.0f, 0.0f, 0.0f);
		break;

	case E_Canvas_Left:
		m_CameraPosition = Maths::HVec3(4.0f, 1.0f, 0.0f);
		m_CameraLookAt = Maths::HVec3(0.0f, 0.0f, 0.0f);
		break;

	case E_Canvas_Right:
		m_CameraPosition = Maths::HVec3(-4.0f, 1.0f, 0.0f);
		m_CameraLookAt = Maths::HVec3(0.0f, 0.0f, 0.0f);
		break;

	case E_Canvas_Top:
		m_CameraPosition = Maths::HVec3(0.0f, 6.0f, 3.0f);
		m_CameraLookAt = Maths::HVec3(0.0f, 0.0f, 0.0f);
		break;
	}

	m_pCamera->SetPosition(m_CameraPosition);
	m_pCamera->LookAt(m_CameraLookAt);
}

/*
===============
NAME: RenderMenu

NOTE: Renders menu
===============
*/
void C_LiveryEditorUI::RenderMenu()
{
	if (!m_MenuActive || !m_pActiveMenu)
		return;
	
	float xStart = 50.0f;
	float yStart = 50.0f;
	float xPos = xStart;
	float yPos = yStart;
	float xOffset = 40.0f;
	float yOffset = 40.0f;

	//Main title
	char title[MAX_CHARS];
	_snprintf(title, lengthof(title), " :: %s ::", m_pActiveMenu->m_Title.m_String);
	Display::FontRender()->Draw(title, xPos, yPos, 30, 30, m_pActiveMenu->m_Title.m_Colour);

	xPos += xOffset;
	yPos += yOffset * 2.0f;

	//Menu items first
	for (int iItem = 0; iItem < m_pActiveMenu->m_pItems.Size(); iItem++)
	{
		if (m_pActiveMenu->m_pItems[iItem]->m_Selected)
			Display::FontRender()->Draw("-> ", xPos - xOffset, yPos, 20, 20, Maths::RGBA(1.0f, 1.0f, 1.0f, 1.0f));

		//Item text
		Display::FontRender()->Draw(m_pActiveMenu->m_pItems[iItem]->m_String, xPos, yPos, 20, 20, m_pActiveMenu->m_pItems[iItem]->m_Colour);

		yPos += yOffset;
	}

	//Sub menus next
	for (int iMenu = 0; iMenu < m_pActiveMenu->m_pSubMenus.Size(); iMenu++)
	{
		if (m_pActiveMenu->m_pSubMenus[iMenu]->m_Selected)
			Display::FontRender()->Draw("-> ", xPos - xOffset, yPos, 20, 20, Maths::RGBA(1.0f, 1.0f, 1.0f, 1.0f));

		//Sub menu title
		Display::FontRender()->Draw(m_pActiveMenu->m_pSubMenus[iMenu]->m_Title.m_String, xPos, yPos, 20, 20, m_pActiveMenu->m_pSubMenus[iMenu]->m_Title.m_Colour);

		yPos += yOffset;
	}	
}

/*
===============
NAME: UpdateCamera

NOTE: Updates camera
===============
*/
void C_LiveryEditorUI::UpdateCamera()
{
	if (m_CameraRotateActive)
	{
		float dx = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeLX);
		float dy = Input::GetGameInput(0)->GetAxisSignedValue(E_LiveryAppAxis_ChangeLY);

		float yaw = dx * 0.075f;
		float pitch = dy * 0.075f;

		Maths::HVec3 vYaw(0.0f, 1.0f, 0.0f);
		Maths::HVec3 vPitch(1.0f, 0.0f, 0.0f);

		Maths::C_Quaternion qYaw(vYaw, Maths::HF32(yaw));

		vPitch = vPitch.Transformed(qYaw, Maths::HVec3(0.0f, 0.0f, 0.0f));
		Maths::C_Quaternion qPitch(vPitch, Maths::HF32(pitch));

		m_CameraPosition = m_CameraPosition.Transformed(qYaw, Maths::HVec3(0.0f, 0.0f, 0.0f));
		m_CameraPosition = m_CameraPosition.Transformed(qPitch, Maths::HVec3(0.0f, 0.0f, 0.0f));
	}

	if (m_CameraZoomActive)
	{
		float backward = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedSlow);
		float forward = Input::GetGameInput(0)->GetButtonValue(E_LiveryAppButton_SpeedFast);
	
		if (forward > 0.0f)
		{
			Maths::HVec3 futurePos = m_CameraPosition;
			Maths::HVec3 vVec = futurePos - m_CameraLookAt;
			vVec.Normalise();
			futurePos -= vVec * forward * 0.1f;
			m_CameraPosition = futurePos;
		}
		else if (backward > 0.0f)
		{
			Maths::HVec3 futurePos = m_CameraPosition;
			Maths::HVec3 vVec = futurePos - m_CameraLookAt;
			vVec.Normalise();
			futurePos += vVec * backward * 0.1f;
			m_CameraPosition = futurePos;
		}
	}

	m_pCamera->SetPosition(m_CameraPosition);
	m_pCamera->LookAt(m_CameraLookAt);
}

} //namespace Livery

} //namespace Vehicle
