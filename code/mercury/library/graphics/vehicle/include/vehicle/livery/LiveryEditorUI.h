/////////////////////////////////////////////////////////////
// C_LiveryEditorUI
/////////////////////////////////////////////////////////////

#ifndef __LIVERY_EDITOR_UI_H__
#define __LIVERY_EDITOR_UI_H__

#include <core/containers/vector.h>

#include "Framework/Application/UserControls.h"

namespace Cam2 { class C_Camera; }

namespace Vehicle
{

namespace Livery
{

enum E_LiveryAppButton
{
	E_LiveryAppButton_Toggle = Framework::E_Button_Count,
	E_LiveryAppButton_MenuDown,
	E_LiveryAppButton_MenuUp,
	E_LiveryAppButton_MenuBack,
	E_LiveryAppButton_MenuNext,
	E_LiveryAppButton_Back,
	E_LiveryAppButton_Proceed,
	E_LiveryAppButton_Blue,
	E_LiveryAppButton_SpeedSlow,
	E_LiveryAppButton_SpeedFast,
	E_LiveryAppButton_NextLayer,
	E_LiveryAppButton_PrevLayer,

	E_LiveryAppButton_Count,
};//WTF
//!!!
enum E_LiveryAppAxis
{
	E_LiveryAppAxis_ChangeLX = Framework::E_Axis_Count,
	E_LiveryAppAxis_ChangeLY,
	E_LiveryAppAxis_ChangeRX,
	E_LiveryAppAxis_ChangeRY,

	E_LiveryAppAxis_Count,
};

enum E_UIMode
{
	E_UIMode_None,

	E_UIMode_Fly,
	E_UIMode_Menu,
	E_UIMode_Edit,
};

enum E_UIEditMode
{
	E_UIEditMode_None,

	//Edit modes
	E_UIEditMode_EditMove,
	E_UIEditMode_EditScale,
	E_UIEditMode_EditRotate,
	E_UIEditMode_EditColour,
};

enum E_UIMenuAction
{
	E_UIMenuAction_None,
	
	E_UIMenuAction_CycleCanvas,
	E_UIMenuAction_AddLayer,
	E_UIMenuAction_RemoveLayer,
	E_UIMenuAction_ChangeTexture,
	E_UIMenuAction_ChangeColour,
	E_UIMenuAction_SetColourRed,
	E_UIMenuAction_SetColourGreen,
	E_UIMenuAction_SetColourBlue,
	E_UIMenuAction_SetColourAlpha,
	E_UIMenuAction_NextTexture,
	E_UIMenuAction_PreviousTexture,
	E_UIMenuAction_LayerMove,
	E_UIMenuAction_LayerScale,
	E_UIMenuAction_LayerRotate,
	E_UIMenuAction_LayerPromote,
	E_UIMenuAction_LayerDemote,
	E_UIMenuAction_Save,
	E_UIMenuAction_Load,
	E_UIMenuAction_CopyLayer,
	E_UIMenuAction_PasteLayer,
};

class C_MenuItem
{
public:

	C_MenuItem()
	{
		m_Colour			= Maths::RGBA(0.0f, 0.0f, 0.0f, 1.0f);

		m_Selected			= false;
	}

public:

	char						m_String[MAX_CHARS];
	
	E_UIMenuAction				m_Action;

	Maths::RGBA					m_Colour;

	bool						m_Selected;
};

class C_Menu
{
public:

	C_Menu()
	{
		m_pActiveItem		= NULL;
		m_pActiveSubMenu	= NULL;
		m_pParent			= NULL;

		m_Selected			= false;

		m_Index				= 0;
	}

	void Reset()
	{
		for (int iItem = 0; iItem < m_pItems.Size(); iItem++)
			m_pItems[iItem]->m_Selected = false;
		for (int iMenu = 0; iMenu < m_pSubMenus.Size(); iMenu++)
			m_pSubMenus[iMenu]->m_Selected = false;

		if (m_pItems.Size() != 0)
		{
			m_Index							= 0;
			m_pActiveItem					= m_pItems[0];
			m_pActiveSubMenu				= NULL;
			m_pActiveItem->m_Selected		= true;
		}
		else if (m_pSubMenus.Size() != 0)
		{
			m_Index							= 0;
			m_pActiveItem					= NULL;
			m_pActiveSubMenu				= m_pSubMenus[0];
			m_pActiveSubMenu->m_Selected	= true;
		}
	}

public:

	C_MenuItem					m_Title;

	Ut::Vector<C_Menu*>			m_pSubMenus;

	Ut::Vector<C_MenuItem*>		m_pItems;	
	
	C_MenuItem*					m_pActiveItem;
	C_Menu*						m_pActiveSubMenu;
	C_Menu*						m_pParent;

	int							m_Index;

	bool						m_Selected;
};


/* 
===============================================================
					class C_LiveryEditor

NOTES: 
===============================================================
*/
class C_LiveryEditorUI
{
public:

	static C_LiveryEditorUI& Instance() {return s_instance;}

	C_LiveryEditorUI();

	~C_LiveryEditorUI();

	//! Initialise
	void Initialise();

	//! Shutdown
	void Shutdown();

	//! Update
	void Update();

	//! Updates camera/edit controls
	void UpdateControls();

	//! Updates menu-mode controls
	void ProcessAction();

	//! Updates edit-mode controls
	void UpdateEditMode();

	//! Sets view defaults
	void SetViewDefaults();

	//! Renders menu
	void RenderMenu();

	//! Updates camera
	void UpdateCamera();

public:
	
	static C_LiveryEditorUI		s_instance;

	Cam2::C_Camera*			m_pCamera;

	E_UIMode					m_Mode;
	E_UIEditMode				m_EditMode;

	bool						m_MenuActive;
	bool						m_CameraRotateActive;
	bool						m_CameraZoomActive;

	Ut::Vector<C_Menu*>			m_pMenus;

	C_Menu*						m_pActiveMenu;

	Maths::HVec3				m_CameraPosition;
	Maths::HVec3				m_CameraLookAt;
};

} //namespace Livery

} //namespace Vehicle

#endif
