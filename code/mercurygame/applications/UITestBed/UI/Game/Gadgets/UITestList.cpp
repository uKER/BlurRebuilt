
#include "stdafx.h"
#include "UITestList.h"

#include "ui/Graphics/Font/Font.h"

XT_TYPE( UI::UITestList, "UITestList" )
	XT_BASE( UI::BaseUIList )
XT_END_TYPE

namespace UI
{

void UITestList::Init()
{
	BaseUIList::Init();

	m_NumberOfItems = 12;
}


void UITestList::RenderItem( const F32 fX, const F32 fY, const U32 uItem, const Maths::HVec3& vRelativePos )
{
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( fX, fY, 0.0f ), MathsStorage::Float3( m_RowHeight, m_RowHeight, 0.0f ), ( uItem != m_CurrentItem ) ? E_BasicColour_Orange : 0xff00ff80 );
	gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
	
#ifndef CFG_GOLD
	C_WideString64 text;
	text.Format( L"Item : %d", uItem );
	gFont.DrawDebugText( m_Layer, Maths::HVec3( fX + m_RowHeight + 2.0f, fY, 0.0f ), E_Justify_TopLeft, text, ( uItem != m_CurrentItem ) ? E_BasicColour_White : 0xff00ff80, m_RowHeight, 0.0f );
#endif
}

}; // namespace UI