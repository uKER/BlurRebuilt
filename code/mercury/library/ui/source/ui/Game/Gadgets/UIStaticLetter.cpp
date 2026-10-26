#include "stdafx.h"
#include "ui/Game/Gadgets/UIStaticLetter.h"

#include <ui/Graphics/Font/Font.h>
#include <ui/Graphics/Font/FontStyle.h>
#include <ui/Graphics/Font/UIBitmapFont.h>
#include <ui/Graphics/Font/UIPolygonFont.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIStaticLetter, "UIStaticLetter" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_TextLetter, "text_letter" )
	XT_FIELD( m_TextId, "text_id" )
XT_END_TYPE

namespace UI
{

void UIStaticLetter::Init()
{
	C_GadgetBase::Init();

	m_TextId.id = 0;
	m_Letter = 0;
}

void UIStaticLetter::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	if ( m_TextId.id != 0 )
	{
		const WCHAR* pString = C_TextStringID( m_TextId.id ).String();

		if ( pString && pString[ 0 ] )
			m_Letter = pString[ 0 ];
	}
	else if ( m_TextLetter.Text() && m_TextLetter.Text()[ 0 ] )
	{
		m_Letter = m_TextLetter.Text()[ 0 ];
	}
	else if( pState )
	{
		if ( Lua::DataMemberType( pState, "text" ) == LUA_TNUMBER )
		{
			m_TextId.id = Lua::GetDataMember<U32>( pState, "text" );
		}
		else if( Lua::DataMemberType( pState, "text" ) == LUA_TSTRING )
		{
			const char* pString = Lua::GetDataMember<const char*>( pState, "text" );

			if ( pString && pString[ 0 ] )
				m_Letter = pString[ 0 ];
		}
	}

	// resolve font styles
	m_FontStylePasses.Setup( pState );
}

void UIStaticLetter::Render()
{
	if ( !m_Activated )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	C_FontStyle& style = m_FontStylePasses.GetFontStyle( 0 );
	I_FontAttributes* pCurrentFont = gFont.GetFont( style.Font() );
	const I_GlyphAttributes* pCurrentGlyph = ( pCurrentFont ? pCurrentFont->LookupGlyph( m_Letter ) : NULL );

	if ( !pCurrentGlyph )
		return;

	// *** TODO - internal font functions for this so we don't have to copy/paste
	// - and for full font style shizz for multi-pass/texture
	// - oh, and EA fonts

	if ( pCurrentGlyph->GetType() == I_GlyphAttributes::E_GlyphType_Bitmap )
	{
		C_BitmapGlyphAttribute* pGlyph = (C_BitmapGlyphAttribute*)pCurrentGlyph;
		C_BitmapFontAttributes* pFont = (C_BitmapFontAttributes*)pCurrentFont;

		F32 px = m_Position.x;		
		F32 py = m_Position.y;
		F32 sx = ( pGlyph->GetWidth() + pFont->GetHrzOffset( true ) + pFont->GetHrzOffset( false ) ) * ( m_Size.y / pFont->GetFontHeight() );		
		F32 sy = m_Size.y;

		F32 tx1 = (F32)pGlyph->GetLeft() / (F32)pFont->m_TextureWidth;
		F32 tx2 = (F32)pGlyph->GetRight() / (F32)pFont->m_TextureWidth;
		F32 ty1 = (F32)pGlyph->GetTop() / (F32)pFont->m_TextureHeight;
		F32 ty2 = (F32)pGlyph->GetBottom() / (F32)pFont->m_TextureHeight;

		C_RenderState renderState = m_RenderState;
		renderState.m_EffectIndex = E_Effect_IntensityAsAlpha;

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( px, py, m_Position.z ), sx, sy, m_SpriteColours, m_Panel, E_SpriteType_Sprite, renderState );
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs( 0, pFont->m_Texture, tx1, ty1, tx2 - tx1, ty2 - ty1 );
	}
	else if ( pCurrentGlyph->GetType() == I_GlyphAttributes::E_GlyphType_Polygon )
	{
		C_UIPolygonFont* pFont = (C_UIPolygonFont*)pCurrentFont;
		C_UICharacter* pGlyph = (C_UICharacter*)pCurrentGlyph;

		INTPTR uUserData = (INTPTR)&pGlyph->GetRenderInfo();	// [Blur2PC][x64]

		F32 px = m_Position.x;
		F32 py = m_Position.y;
		F32 sx = (F32)( pGlyph->GetWidth() + pGlyph->GetAdvance() ) * ( m_Size.y / pFont->GetFontHeight() );
		F32 sy = m_Size.y;

		Maths::HVec3 offset = gSimple2D.GetJustificationOffset( m_Justify, m_Panel );

		px -= offset.X() * sx;
		py -= offset.Y() * sy;

		sx /= POLYGON_FONT_HEIGHT;
		sy /= POLYGON_FONT_HEIGHT;

		sx = sy; /*NOT a typo!*/

		C_RenderState renderState = m_RenderState;
		renderState.m_EffectIndex = E_Effect_PolygonFont;

		gSimple2D.AddSprite( m_Layer, Maths::HVec3( px, py, m_Position.z ), sx, sy, m_SpriteColours, m_Panel, E_SpriteType_TriList, renderState, uUserData );
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )
#endif
