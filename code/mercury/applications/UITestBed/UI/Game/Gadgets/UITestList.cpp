
#include "stdafx.h"

#include "UITestList.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI 
#pragma optimize("", off) 
#endif

XT_TYPE( UI::UITestList, "UITestList" )
XT_BASE( UI::BaseUIList )
XT_END_TYPE

namespace UI
{


void UITestList::Setup( lua_State* pState )
{
	BaseUIList::Setup( pState );
}


void UITestList::RenderItem( F32 x, F32 y, const U32 item  )
{
	C_WideString32 buff;
	buff.Format( L"Item %d", item );

	gFont.DrawDebugText( m_Layer, Maths::HVec3( x,y,m_Position.z), E_Justify_TopLeft, buff, ( item == m_CurrentItem ) ? 0xff77ff44 : 0xffffffff, m_RowHeight, 0.0f, m_Panel );
}

}; // namespace UI


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI 
#pragma optimize("", on) 
#endif

