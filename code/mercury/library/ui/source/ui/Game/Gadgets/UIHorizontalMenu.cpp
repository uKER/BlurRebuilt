#include "stdafx.h"

#include "ui/Game/UILua.h"

#include "ui/Game/Gadgets/UIHorizontalMenu.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#include "ui/Graphics/SimpleTimer.h"
#include "ui/Graphics/Font/Font.h"

#if defined ( PLATFORM_PC )
#include "ui/UiInterface.h"
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif


XT_TYPE( UI::UIHorizontalMenu, "UIHorizontalMenu" )
	XT_BASE( UI::BaseUIVariableSelect )
	XT_FIELD( m_bDoScroll, "do_scroll" )
XT_END_TYPE


namespace UI
{

void UIHorizontalMenu::Init()
{
	BaseUIVariableSelect::Init();

	m_Pos = 0.f;
	m_NextPos = 0.f;
	m_bDoScroll = true;

	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
}


void UIHorizontalMenu::AddItem( const WCHAR* pText, U32 value )
{
	BaseUIVariableSelect::AddItem( pText, value );

	F32 string_size = gFont.StringLength( GetString( m_Items.Size() - 1 ), m_Size.y, m_StyleDeselected.GetFontStyle( 0 ) );

	m_StringSize.PushBack( string_size );
}


bool UIHorizontalMenu::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if ( !m_Activated || !m_Selected )
		return BaseUIVariableSelect::MessageHandler( pMessage );

	bool ret = false;
	switch( pMessage->Type() )
	{
	case E_UiMsg_CtrlButton_Left:
	case E_UiMsg_CtrlButton_Right:
		if( m_Selected )
		{
			const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;

			if( pUIMessage && pUIMessage->deb && ( m_Pad == -1 || m_Pad == pUIMessage->pad ) )
			{
				if(pMessage->Type() == E_UiMsg_CtrlButton_Right)
					SetNextItem();
				else
				if(pMessage->Type() == E_UiMsg_CtrlButton_Left)
					SetPreviousItem();
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


void UIHorizontalMenu::Render()
{
	if ( !m_Activated )
		return;

	BaseUIVariableSelect::Render();

	F32 barX = 0.0f;
	F32 len = 0.0f;

#ifdef _UI_AUTO_CLIPWINDOW
	// clip window
	C_UIClipWindow clipW = gSimple2D.AddClipWindowAuto( this );
#endif

	// auto-transform
	C_UIAutoTransform transform( this );

	for ( S32 i = 0; i < m_CurrentItem; i++ )
	{
		if ( m_Items[i].m_bVisible )
			len += m_StringSize[i] + (m_Size.x);
	}

	m_NextPos = len;
	m_Pos += ( m_NextPos - m_Pos ) * ( theFETimer.FrameTime() * 4 );
	barX =- m_Pos;

	if ( !m_bDoScroll )
		barX = 0.0f;

	for ( S32 i = 0; i < m_Items.Size(); i++ )
	{
		if ( !m_Items[i].m_bVisible )
			continue;

		int tempx = (int)(barX + m_Position.x + m_Position.x);
		C_UIColour col = m_SpriteColours[0];
		bool bSelectable = m_Items[ i ].m_bUnlocked;

		C_FontStylePasses& style = ( m_CurrentItem == i ) ? ( bSelectable ? m_StyleSelected : m_StyleSelectedUnselectable ) : ( bSelectable ? m_StyleDeselected : m_StyleDeselectedUnselectable );

		F32 x = barX + m_Position.x + m_Position.x;
		F32 y = m_Position.y + m_Position.y;

		if ( !m_Selected )
			col.A( col.A() >> 1 );

		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_Justify, m_Items[i].m_Text, col, m_Size.y, style, 0, m_Panel );

		barX += m_StringSize[i];

		if( i !=( m_Items.Size() - 1 ) )
			barX += m_Size.x;
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
