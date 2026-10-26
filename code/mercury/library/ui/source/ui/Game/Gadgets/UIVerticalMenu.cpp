#include "stdafx.h"

#include "ui/Game/UILua.h"

#include "ui/Game/Gadgets/UIVerticalMenu.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/SimpleTimer.h"

#if defined ( PLATFORM_PC )
#include "ui/UiInterface.h"
#endif

#define UI_VERTICAL_MENU_WAIT 2.0f

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIVerticalMenu, "UIVerticalMenu" )
	XT_BASE( UI::BaseUIVariableSelect )
	XT_FIELD( m_ScrollSpeed, "scroll_speed" )
	XT_FIELD( m_NumRows, "num_rows" )
	XT_FIELD( m_bQuickNav, "quick_nav" )
XT_END_TYPE

namespace UI
{

void UIVerticalMenu::Init()
{
	BaseUIVariableSelect::Init();

	mPos = mNextPos = 0.0f;
	mScrollOffset = 0.0f;
	mWait = UI_VERTICAL_MENU_WAIT;

	m_Selected = true;

	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );

	m_bQuickNav = false;
}

void UIVerticalMenu::Setup( lua_State* pState )
{
	m_StartItem = 0;
	m_EndItem = m_NumRows;

	// Setup font interface
	C_FontInterfaceParam my_param;
	if( my_param.Setup( pState ) )
		m_RenderState.m_EffectParam = my_param.GetEffectParam();

	BaseUIVariableSelect::Setup( pState );

	if ( m_bQuickNav )
	{
		AddSubscriber( E_UiMsg_CtrlButton_LeftShoulder );
		AddSubscriber( E_UiMsg_CtrlButton_RightShoulder );
	}
}


bool UIVerticalMenu::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if ( !m_Activated || !m_Selected )
		return BaseUIVariableSelect::MessageHandler( pMessage );

	bool ret = false;
	switch( pMessage->Type() )
	{
	case E_UiMsg_CtrlButton_Up:
	case E_UiMsg_CtrlButton_Down:
	case E_UiMsg_CtrlButton_LeftShoulder:
	case E_UiMsg_CtrlButton_RightShoulder:
		if( m_Selected )
		{
			const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;

			if( pUIMessage && pUIMessage->deb && ( m_Pad == -1 || m_Pad == pUIMessage->pad ) )
			{
				if(pMessage->Type() == E_UiMsg_CtrlButton_Up)
				{
					SetPreviousItem();
				}
				else
				if(pMessage->Type() == E_UiMsg_CtrlButton_Down)
				{
					SetNextItem();
				}
				else if ( pMessage->Type() == E_UiMsg_CtrlButton_LeftShoulder )
				{
					for ( S32 i = 0; i < m_NumRows; i++ )
						SetPreviousItem();
				}
				else if ( pMessage->Type() == E_UiMsg_CtrlButton_RightShoulder )
				{
					for ( S32 i = 0; i < m_NumRows; i++ )
						SetNextItem();
				}
			}
		}
		ret = true;
		break;
		
	default:
		ret = BaseUIVariableSelect::MessageHandler( pMessage );
		break;
	}

	return ret;
}


void UIVerticalMenu::Render()
{
//	bool has_to_scroll=true;
	if ( !m_Activated ) 
		return;

	BaseUIVariableSelect::Render();

#ifdef PLATFORM_PC
	S32 current_item = m_CurrentItem;
#endif

#ifdef _UI_AUTO_CLIPWINDOW
	// clip windows
	C_UIClipWindow clipW1 = gSimple2D.AddClipWindowAuto( this );
	C_UIClipWindow clipW2;
	C_UIClipWindow clipW3;
#endif

	// auto-transform
	C_UIAutoTransform transform( this );

	float tempy=m_Position.y;
#if 0
	if(m_CurrentItem > 6)
	{
		tempy-=(m_CurrentItem-6)*(m_Size.y+mEffectData.m_Position.y-6));
	}
#endif
	if ( m_Items.Size() )
	{
//		int shown=0;
#if 0
		for(int a=0;a<m_Items.Size();a++)
#else

		//------------------------------------------------
		// New logic.
		//------------------------------------------------
		// Scrolling Down!
		if ( m_CurrentItem >= m_EndItem )
		{
			m_StartItem = m_CurrentItem - (m_NumRows-1);
			m_EndItem = m_CurrentItem+1;
		}
		// Scrolling Up!
		if( m_CurrentItem < m_StartItem )
		{
			m_StartItem = m_CurrentItem;
			m_EndItem = m_CurrentItem + m_NumRows;
		}

		// Check values are valid.
		if ( m_StartItem < 0 )
			m_StartItem = 0;
		if ( m_EndItem > m_Items.Size() )
			m_EndItem = m_Items.Size();

		
		for( S32 i = m_StartItem; i < m_EndItem; i++)
#endif
		{
			if ( !m_Items[i].m_bVisible )
				continue;

			// Render the menu item.
			bool bSelected = ( m_CurrentItem == i );
			bool bSelectable = m_Items[i].m_bUnlocked;				

			// Change font style.
			C_FontStylePasses& style = ( bSelected ) ? ( bSelectable ? m_StyleSelected : m_StyleSelectedUnselectable ) : ( bSelectable ? m_StyleDeselected : m_StyleDeselectedUnselectable );
			C_UIColour drawColour = m_SpriteColours[0];

			if ( !m_Selected )
				drawColour.A( drawColour.A() >> 1 );

			F32 x = m_Position.x;
			F32 y = tempy;

			gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_Justify, m_Items[i].m_Text, drawColour, m_Size.y, style, 0, m_Panel );
			F32 size_y = m_Size.x;

#ifdef PLATFORM_PC
			if(UiInterface()->UseMouseInput())
			{
				F32 mouse_x, mouse_y;
				UI::UiInterface()->GetMousePos(mouse_x, mouse_y);
				mouse_y *= UI_VSCREEN_SIZEY;

				if (m_Mouse.isover && mouse_y >= tempy && mouse_y < tempy+size_y)
				{
					current_item = i;
				}
			}
#endif
			tempy += size_y * m_Up;
		}
	}

#ifdef PLATFORM_PC
	if(UiInterface()->UseMouseInput() && m_CurrentItem != current_item && m_Items[m_CurrentItem].m_bUnlocked )
	{
		_UI_MESSAGE4( E_UiMsg_VerticalMenuChanged, 0, 0, (F32)m_ID, (F32)m_CurrentItem );
		m_CurrentItem = current_item;
	}
#endif
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
