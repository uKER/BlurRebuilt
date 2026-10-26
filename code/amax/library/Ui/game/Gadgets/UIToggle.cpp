#include "stdafx.h"
#include "UIToggle.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

// XT Definition
XT_TYPE( UI::UIToggle, "UIToggle" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_TitleID,			"title" )
	XT_FIELD( m_ItemAID,			"item_a" )
	XT_FIELD( m_ItemBID,			"item_b" )
	XT_FIELD( m_TitleOffset,		"title_offset" )
	XT_FIELD( m_TitleStyle,			"title_style" )
	XT_FIELD( m_TitleColourName,	"title_colour_name" )
	XT_FIELD( m_TitleSize,			"title_size" )
	XT_FIELD( m_ItemStyle,			"item_style" )
	XT_FIELD( m_ItemColourName,		"item_colour_name" )
	XT_FIELD( m_ItemSize,			"item_size" )
XT_END_TYPE

namespace UI
{

void UIToggle::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_TitleColour.SetByName( m_TitleColourName, m_Alpha, m_Intensity );
	m_ItemColour.SetByName( m_ItemColourName, m_Alpha, m_Intensity );

	m_TitleStyle.Setup( pState );
	m_ItemStyle.Setup( pState );

	if( m_TitleID.id != 0 )
		m_TitleText = m_TitleID.id;

	if( m_ItemAID.id != 0 )
		m_ItemAText = m_ItemAID.id;

	if( m_ItemBID.id != 0 )
		m_ItemBText = m_ItemBID.id;
}

void UIToggle::Render()
{
	if(!m_Activated)
		return;	

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	// Title
	if ( m_TitleText.IsLocString() )
	{
		gFont.DrawText( m_Layer, Maths::HVec3(m_Position) + Maths::HVec3(m_TitleOffset), m_Justify, m_TitleText, m_TitleColour, m_TitleSize, m_TitleStyle, 0, m_Panel );
	}
	// Item
	if ( m_Toggle ? m_ItemBText.IsLocString() : m_ItemAText.IsLocString() )
	{
		gFont.DrawText( m_Layer, Maths::HVec3(m_Position), m_Justify, m_Toggle ? m_ItemBText : m_ItemAText, m_ItemColour, m_ItemSize, m_ItemStyle, 0, m_Panel );
	}
}

const bool UIToggle::SetCurrentItemByIndex( const S32 index )
{
	m_Toggle = ( index ) ? true : false;
	return true;
}

const bool UIToggle::SetCurrentItemByValue( const F64 value )
{
	m_Toggle = ((S32)value ) ? true : false;
	return true;
}

void UIToggle::ChangeColour( const S32 sArg, const char* pStyle )
{
	if ( sArg == 0 ) // Title
	{
		m_TitleColour.SetByName( pStyle, m_Alpha, m_Intensity );
	}
	else
	if ( sArg == 1 ) // Item
	{
		m_ItemColour.SetByName( pStyle, m_Alpha, m_Intensity );
	}
}

}; // namespace UI