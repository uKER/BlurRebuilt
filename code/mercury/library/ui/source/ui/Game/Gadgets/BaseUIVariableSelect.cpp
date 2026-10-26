#include "stdafx.h"

#include "ui/UIDefines.h"
#include "ui/UiInterface.h"

#include "ui/Game/UIScreenControl.h"
#include "ui/Game/UIDataFinder.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#include "ui/Graphics/UITextureManager.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Localisation/Loc.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::BaseUIVariableSelect, "UIVariableSelect" )
	XT_BASE( UI::C_GadgetBase )
	//XT_FIELD( m_Pad, "controller" )
	XT_FIELD( m_SorceryItems, "items" )
	XT_FIELD( m_StyleSelected, "style_selected" )
	XT_FIELD( m_StyleDeselected, "style_deselected" )
	XT_FIELD( m_StyleSelectedUnselectable, "style_selected_unselectable" )
	XT_FIELD( m_StyleDeselectedUnselectable, "style_deselected_unselectable" )
	XT_FIELD( m_bLoopList, "loop_list" )
	XT_FIELD( m_bHighlightLocked, "highlight_locked" )
XT_END_TYPE

XT_TYPE( UI::BaseUIVariableSelect::C_Item, "UIVariableSelectItem" )
	XT_FIELD( m_NameId, "nameId" )
	XT_FIELD( m_NameString, "nameString" )
	XT_FIELD( m_HelpId, "helpId" )
	XT_FIELD( m_Value, "value" )
	XT_FIELD( m_OnNextCmd, "on_next" )
XT_END_TYPE

namespace UI
{

void BaseUIVariableSelect::Init()
{
	C_GadgetBase::Init();

	m_CurrentItem		= 0;
	m_StartItem			= 0;
	m_EndItem			= 0;
	m_bLoopList			= false;
	m_Selected			= true;
	m_Pad				= -1;

	// Message subscribers
	AddSubscriber( E_UiMsg_MenuNext );
}

void BaseUIVariableSelect::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_StyleSelected.Setup( pState, "style_selected" );
	m_StyleDeselected.Setup( pState, "style_deselected" );
	m_StyleSelectedUnselectable.Setup( pState, "style_selected_unselectable" );
	m_StyleDeselectedUnselectable.Setup( pState, "style_deselected_unselectable" );

	if( pState )
	{
		if ( Lua::DataMemberType( pState, "controller" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "controller", m_Pad );
	}

	// Add sorcery items
	for( XtI32 i = 0; i < m_SorceryItems.count; i++)
	{
		if ( m_SorceryItems[ i ].m_NameString && m_SorceryItems[ i ].m_NameString[ 0 ] )	// datafinder lookup
			AddItem( C_UIDataFinder::Instance().GetVariableText( m_SorceryItems[ i ].m_NameString ), m_SorceryItems[ i ].m_Value );
		else
			AddItem( C_TextStringID( m_SorceryItems[ i ].m_NameId.id ).String(), m_SorceryItems[ i ].m_Value );		

		// Add help text
		if ( m_SorceryItems[ i ].m_HelpString && m_SorceryItems[ i ].m_HelpString[ 0 ] )
			AddItemHelp( C_UIDataFinder::Instance().GetVariableText( m_SorceryItems[ i ].m_HelpString ));
		else
			AddItemHelp( C_TextStringID( m_SorceryItems[ i ].m_HelpId.id ).String() );	
	}
}

bool BaseUIVariableSelect::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	bool ret = false;

	switch( pMessage->Type() )
	{
		case E_UiMsg_MenuNext:
		if( m_Selected )
		{
			const C_UiMessagePad<E_UiMsg_MenuNext>* pUIMessage = Messaging::MessageCast< C_UiMessagePad<E_UiMsg_MenuNext> >( pMessage );
			if( pUIMessage && pUIMessage->deb && ( m_Pad == -1 || m_Pad == pUIMessage->pad ) )
			{
				if( m_CurrentItem < m_SorceryItems.count && m_SorceryItems[ m_CurrentItem ].m_OnNextCmd && m_SorceryItems[ m_CurrentItem ].m_OnNextCmd[0] )
				{
					lua_State *pState = g_pUILua->GetState();
					int status=luaL_dostring( pState, m_SorceryItems[ m_CurrentItem ].m_OnNextCmd );
					if( status != 0 )
					{
						LOGWARN( g_Log, DBSTR "%s", lua_tostring( pState, -1 ) );
						lua_pop( pState, 1); /* remove error message and _ALERT */
					}
				}
			}
		}
		ret = true;

		default:
			ret = C_GadgetBase::MessageHandler( pMessage );
			break;
	}

	return ret;
}

void BaseUIVariableSelect::Render()
{
	if( !gUIScreenControl.ScreenTransitionActive() )
	{
		for( S32 i = 0; i < m_Items.Size(); i++)
		{
			if( i == m_CurrentItem ) 
			{
				m_Items[i].m_Timer.m_TimerDirc = true;
				m_Items[i].m_Timer.m_DoWhat = UITimer::E_Type_Stop;
			}
			else
			{
				m_Items[i].m_Timer.m_TimerDirc = false;
				m_Items[i].m_Timer.m_DoWhat = UITimer::E_Type_Stop;
			}
			m_Items[i].m_Timer.Update();
		}
	}
	C_GadgetBase::Render();
}


void BaseUIVariableSelect::AddItem( const WCHAR* pText, U32 value )
{
	C_VariableItem item;

	// Copy the text string.
	item.m_Text			= C_WideString128( pText );
	item.m_Value		= value;
	item.m_bUnlocked	= true;
	item.m_bVisible		= true;
	item.m_HelpText		= L"";

	m_Items.PushBack( item );
}


void BaseUIVariableSelect::AddItemHelp( const WCHAR* pText )
{
	// Add help text to the last item added.
	m_Items[m_Items.Size()-1].m_HelpText = C_WideString64( pText );
}


bool BaseUIVariableSelect::SetNextItem()
{
	if ( m_Items.Size() <= 0 )
		return false;

	S32 oldSelection = m_CurrentItem;

	// Check that the item is valid.
	S32 newSelection = m_CurrentItem;

	// Stop infinate loops.
	S32 maxLoop = ( m_bLoopList ) ? m_Items.Size(): m_Items.Size() - m_CurrentItem; 

	for ( S32 count = 0; count < maxLoop; count++ )
	{
		newSelection++; // onto next item.

		if ( newSelection >= m_Items.Size() )
		{
			if ( m_bLoopList )
				newSelection = 0;
			else
				newSelection = m_Items.Size() - 1;
		}

		// Check its valid
		if ( m_Items[newSelection].m_bVisible )
		{
			if ( m_Items[newSelection].m_bUnlocked )
			{
				m_CurrentItem = newSelection;
				break;
			}
			else
			if ( !m_Items[newSelection].m_bUnlocked && m_bHighlightLocked )
			{
				m_CurrentItem = newSelection;
				break;
			}
		}
	}

	// Was a new selection found?
	if ( m_CurrentItem != oldSelection )
	{
		UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
		return true;
	}
	return false;
}


bool BaseUIVariableSelect::SetPreviousItem()
{
	if ( m_Items.Size() <= 0 )
		return false;

	S32 oldSelection = m_CurrentItem;

	// Check that the item is valid.
	S32 newSelection = m_CurrentItem;

	// Stop infinate loops.
	S32 maxLoop = ( m_bLoopList ) ? m_Items.Size() : m_CurrentItem; 

	for ( S32 count = 0; count < maxLoop; count++ )
	{
		newSelection--; // onto next item.

		if ( newSelection < 0 )
		{
			if ( m_bLoopList )
				newSelection = m_Items.Size() - 1;
			else
				newSelection = 0;
		}

		// Check its valid
		if ( m_Items[newSelection].m_bVisible )
		{
			if ( m_Items[newSelection].m_bUnlocked )
			{
				m_CurrentItem = newSelection;
				break;
			}
			else
			if ( !m_Items[newSelection].m_bUnlocked && m_bHighlightLocked )
			{
				m_CurrentItem = newSelection;
				break;
			}
		}
	}

	// Was a new selection found?
	if ( m_CurrentItem != oldSelection )
	{
		UiInterface()->PlaySound( UiInterface()->GetSoundIndex( E_BaseSoundEffect_Toggle ) );
		return true;
	}
	return false;
}


void BaseUIVariableSelect::SetItemUnlocked( S32 value, bool unlocked )
{
	for ( S32 i = 0; i < m_Items.Size(); i++ )
	{
		if ( m_Items[ i ].m_Value == value )
		{
			m_Items[ i ].m_bUnlocked = unlocked;
			break;
		}
	}
	ResolveList();
}


void BaseUIVariableSelect::SetItemVisible( S32 value, bool visible )
{
	for ( S32 i = 0; i < m_Items.Size(); i++ )
	{
		if ( m_Items[ i ].m_Value == value )
		{
			m_Items[ i ].m_bVisible = visible;
			break;
		}
	}
	ResolveList();
}

const bool BaseUIVariableSelect::SetCurrentItemByIndex( const S32 index )
{
	if( index < 0 || index >= m_Items.Size() )
		return false;

	m_CurrentItem = index;
	ResolveList();

	return true;

}

const bool BaseUIVariableSelect::SetCurrentItemByValue( const F64 value )
{
	bool found = false;
	U32 ival = (U32)value;
	for ( S32 i = 0; i < m_Items.Size() && !found; i++ )
	{
		if( m_Items[i].m_Value == ival ) 
		{
			m_CurrentItem = i;
			found = true;
			break;
		}
	}
	if( found )
	{
		ResolveList();
		return true;
	}

	return false;
}


void BaseUIVariableSelect::ResolveList()
{
	if ( m_CurrentItem < 0 || m_CurrentItem >= m_Items.Size() )
		return;

	if ( ( !m_bHighlightLocked && !m_Items[ m_CurrentItem ].m_bUnlocked ) || !m_Items[ m_CurrentItem ].m_bVisible )
	{
		S32 sOldItem = m_CurrentItem;

		SetNextItem();

		if ( m_CurrentItem == sOldItem )
			SetPreviousItem();

		// all closed - eek
		//assert( m_CurrentItem != sOldItem );
	}
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
