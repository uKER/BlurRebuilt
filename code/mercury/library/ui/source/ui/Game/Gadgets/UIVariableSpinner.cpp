#include "stdafx.h"

#include "ui/Game/Gadgets/UIVariableSpinner.h"
#include "ui/Game/UILua.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/UiInterface.h"
#include "ui/Game/StringStore.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::UIVariableSpinner, "UIVariableSpinner" )
	XT_BASE( UI::BaseUIVariableSelect )
	XT_FIELD( m_TextID, "text_id" )
	XT_FIELD( m_DataJustify, "data_justify" )
	XT_FIELD( m_bAddTitleLength, "add_title_length" )
XT_END_TYPE

namespace UI
{

void UIVariableSpinner::Init()
{
	BaseUIVariableSelect::Init();

	m_Text.Invalidate();
	m_TextID.id = 0;

	m_bAddTitleLength = false;
	m_fTitleLength = 0.0f;
	m_DataJustify = E_Justify_TopLeft;

	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
	AddSubscriber( E_UiMsg_CtrlLeftStick_X );
	AddSubscriber( E_UiMsg_CtrlMouse_Pressed );
}

void UIVariableSpinner::Setup( lua_State* pState )
{
	BaseUIVariableSelect::Setup( pState );

	if( m_TextID.id != 0 )
	{
		// set directly with Xt annotation
		m_Text = m_TextID.id;
	}
	else if ( pState )
	{
		U32 text;
		Lua::GetDataMember( pState, "text", text );
		SetText( text );

		m_HeaderStoreString.Clear();
		if(!m_Text.IsLocString())
		{
			m_HeaderStoreString.Format(C_StringStore::Instance().Get(text));
		}
	}

	// Setup font interface
	C_FontInterfaceParam interfaceParam;
	if( interfaceParam.Setup( pState ) )
		m_RenderState.m_EffectParam = interfaceParam.GetEffectParam();

#ifdef PLATFORM_PC
	// override mouse box?
	if( pState )
	{
		S32 sMouseOverWidth = -1;
		if( Lua::DataMemberType( pState, "mouse_over_width" ) == LUA_TNUMBER )
		{
			Lua::GetDataMember( pState, "mouse_over_width", sMouseOverWidth );
			m_Mouse.bb_right = m_Position.x + sMouseOverWidth;
		}
	}
#endif

	// calculate title length
	if ( m_bAddTitleLength )
	{
		if ( m_HeaderStoreString.Length() )
			m_fTitleLength = gFont.StringLength( m_HeaderStoreString, m_Size.y, m_StyleSelected.GetFontStyle( 0 ) );
		else
			m_fTitleLength = gFont.StringLength( m_Text, m_Size.y, m_StyleSelected.GetFontStyle( 0 ) );

	}
}


bool UIVariableSpinner::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if ( !m_Activated || !m_Selected )
		return BaseUIVariableSelect::MessageHandler( pMessage );

	bool ret = false;
	switch( pMessage->Type() )
	{
	case E_UiMsg_CtrlButton_Left:
	case E_UiMsg_CtrlButton_Right:
	case E_UiMsg_CtrlMouse_Pressed:
		{
			const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;
			if( pUIMessage && pUIMessage->deb&& ( m_Pad == -1 || m_Pad == pUIMessage->pad ) )
			{
				if ( pMessage->Type() == E_UiMsg_CtrlButton_Left || ( pMessage->Type() == E_UiMsg_CtrlMouse_Pressed && pUIMessage->pad == 1 ) )
				{
					SetPreviousItem();
				}
				else
				if ( pMessage->Type() == E_UiMsg_CtrlButton_Right || ( pMessage->Type() == E_UiMsg_CtrlMouse_Pressed && pUIMessage->pad == 2 ) )
				{
					SetNextItem();
				}
			}
		}
		ret = true;
		break;

		/*
	case E_UiMsg_CtrlLeftStick_X:
		{
			const C_UiMessageBase* pUIMessage = Messaging::MessageCast_Reckless<C_UiMessageBase>(pMessage);
			if( pUIMessage && ( m_Pad == -1 || m_Pad == pUIMessage->pad ) )
			{
				if( mType == E_SpinnerType_Step )
				{
					mCurrentSlider += ( pUIMessage->amount * mStep );
					mCurrentSlider = Maths::Clamped< F32 >( mCurrentSlider, mMin, mMax );
				}
			}
		}
		*/
	default:
		ret = BaseUIVariableSelect::MessageHandler( pMessage );
		break;
	}

	return ret;
}

void UIVariableSpinner::Render()
{
	if ( !m_Activated )
		return;

	if( m_SpriteColours[0].A() == 0 )
		return;

	if ( m_TextID.id != 0 )
		m_Text = m_TextID.id;

	// auto-transform
	C_UIAutoTransform transform( this );
	Maths::HVec3 ox( m_Size.x, 0.0f, 0.0f );

	if ( !UI::gUIScreenControl.InvalidStrings() )
	{
		// draw the title text
		C_FontStylePasses& titleStyle = ( m_Selected && m_CurrentItem < m_Items.Size() && m_Items[ m_CurrentItem ].m_bUnlocked ) ? m_StyleSelected : m_StyleSelectedUnselectable;

		if(m_HeaderStoreString.Length())
		{
			gFont.DrawText( m_Layer, m_Position, m_Justify, m_HeaderStoreString, m_SpriteColours, m_Size.y, titleStyle, 0, m_Panel );
		}
		else
		{
			gFont.DrawText( m_Layer, m_Position, m_Justify, m_Text, m_SpriteColours, m_Size.y, titleStyle, 0, m_Panel );
		}
		
		// draw the data text
		C_FontStylePasses& dataStyle = ( m_Selected && m_CurrentItem < m_Items.Size() && m_Items[ m_CurrentItem ].m_bUnlocked ) ? m_StyleDeselected : m_StyleDeselectedUnselectable;

		if ( m_bAddTitleLength )
			ox.X( ox.X() + m_fTitleLength );

		if( (m_CurrentItem >= 0) && (m_CurrentItem < m_Items.Size()) )
		{
			// display the changeable text
			const WCHAR* pString = GetString( m_CurrentItem );
			gFont.DrawText( m_Layer, m_Position + ox, m_DataJustify, pString, m_SpriteColours, m_Size.y, dataStyle, 0, m_Panel );
		}
		else
		{
			C_TextStringID id( "CMN_NA" );
			gFont.DrawText( m_Layer, m_Position + ox, m_DataJustify, id, m_SpriteColours, m_Size.y, dataStyle, 0, m_Panel );
		}
	}
/*
	else if(mType == E_SpinnerType_Step)
	{
		// draw the title text
		C_FontStylePasses& titleStyle = m_Selected ? m_StyleSelected : m_StyleSelectedUnselectable;
		gFont.DrawText( m_Layer, m_Position, m_Justify, m_Text, m_SpriteColours, m_Size.y, titleStyle, 0, m_Panel );
		
		// draw the data text
		C_FontStylePasses& dataStyle = m_Selected ? m_StyleDeselected : m_StyleDeselectedUnselectable;

		C_WideString512 buffer;
//		buffer.Format( L"%d", (S32)mCurrentSlider );

		gFont.DrawText( m_Layer, Maths::HVec3( m_Position.x + m_Size.x, m_Position.y, m_Position.z ), m_DataJustify, buffer, m_SpriteColours, m_Size.y, dataStyle, 0, m_Panel );
	}
*/
	BaseUIVariableSelect::Render();
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
