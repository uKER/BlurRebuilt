#include "stdafx.h"

#include "UIPostProcessTexture.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIPostProcessTexture, "UIPostProcessTexture" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

void UIPostProcessTexture::Init()
{
	C_GadgetBase::Init();
};


void UIPostProcessTexture::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );
};


void UIPostProcessTexture::Render()
{
	if ( !m_Activated )
		return;

	C_GadgetBase::Render();

	// turn alpha test off.
	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 1 );
	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetAlphaTest, (U32)false );

	// Render the actual texture
	gSimple2D.AddSprite( m_Layer, m_Position, m_Size, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( m_Justify );
	gSimple2D.AddTexUVs( 0, gSimple2D.AlternateTextureFindByName( "RENDER_BUFFER" ) );

	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 0 );
};

}; // namespace UI


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif