
#include "Stdafx.h"

#include "ui/Game/Gadgets/UIStaticGraphic.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Game/UILua.h"
#include "ui/Game/UiXt.h"
#include "ui/Game/Gadgets/ButtonMessages.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIStaticGraphic, "UIStaticGraphic" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

// Static graphic
void UIStaticGraphic::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

#ifdef PLATFORM_PC
	m_fMouseOverEffectTime = 0.0f;
	m_MouseOverToColour.Set( 0xffffffff );

	if( pState )
	{
		if ( Lua::DataMemberType( pState, "mouse_over_to_colour" ) == LUA_TSTRING )
			m_MouseOverToColour.Set( Lua::GetDataMember<const char *>( pState, "mouse_over_to_colour" ) );

		if ( Lua::DataMemberType( pState, "mouse_over_effect_time" ) == LUA_TNUMBER )
			Lua::GetDataMember( pState, "mouse_over_effect_time", m_fMouseOverEffectTime );
	}
	if ( m_Mouse.clickable && m_fMouseOverEffectTime > 0.0f )
		AddSubscriber( E_UiMsg_FrameUpdate );
#endif
}

void UIStaticGraphic::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();

#ifdef PLATFORM_PC
	if ( !m_Mouse.clickable || m_fMouseOverEffectTime <= 0.0f )
		return;

	F32 fFrameTime = theFETimer.FrameTime();

	if ( m_Activated && m_Mouse.isover && m_fMouseOverTimer != 1.0f )
		m_fMouseOverTimer = Maths::Clamp( m_fMouseOverTimer + ( 1.0f / m_fMouseOverEffectTime ) * fFrameTime, 0.0f, 1.0f );
	else if ( m_Activated && !m_Mouse.isover && m_fMouseOverTimer != 0.0f )
		m_fMouseOverTimer = Maths::Clamp( m_fMouseOverTimer - ( 1.0f / m_fMouseOverEffectTime ) * fFrameTime, 0.0f, 1.0f );
#endif
}


void UIStaticGraphic::Render( )
{
	if(!m_Activated)
		return;

#ifdef _UI_AUTO_CLIPWINDOW
	// clip window
	C_UIClipWindow clipW = gSimple2D.AddClipWindowAuto( this );
#endif

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	if( m_RenderState.m_BlendMode == Display::E_BlendMode_Alpha && (m_Alpha < 0.01f || (m_SpriteColours[0].A() == 0 && m_SpriteColours[1].A() == 0 && m_SpriteColours[2].A() == 0 && m_SpriteColours[3].A() == 0)) )
		return;

	C_UISpriteColours colour = m_SpriteColours;

#ifdef PLATFORM_PC
	// mouse over colour effect
	if ( m_fMouseOverEffectTime != 0.0f )
	{
		for(U32 i = 0; i < C_UISpriteColours::m_SpriteColoursSize; i++)
			colour[i].Lerp( colour[i], m_MouseOverToColour, m_fMouseOverTimer );
	}
#endif

#ifdef _GADGET_BOUNDS_CULLING
	if( IsCulled() )
		return;
#endif

	gSimple2D.AddSprite(m_Layer, m_Position, m_Size.x, m_Size.y, colour, m_Panel, E_SpriteType_Sprite, m_RenderState);
	gSimple2D.AddJustification( m_Justify );

	for(U32 a = 0;a < C_Sprite::m_NumTextureStages && a < (U32)m_TextureParams.count; a++) // Note : num texture stages in Simple2D not number of texture slots in mData.textures[]
	{
		gSimple2D.AddTexUVs(a, m_TextureParams.elements[ a ] );
	}
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
