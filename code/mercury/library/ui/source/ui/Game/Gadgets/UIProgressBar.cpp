#include "stdafx.h"

#include "Ui/Game/Gadgets/UIProgressBar.h"

// XT Definition
XT_TYPE( UI::UIProgressBar, "UIProgressBar" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_Sections,	"sections" )
	XT_FIELD( m_Spacing,	"spacing" )
	XT_FIELD( m_Padding,	"padding" )
XT_END_TYPE

namespace UI
{

void UIProgressBar::Init()
{
	C_GadgetBase::Init();

	m_Sections = 10;
	m_CurrentPosition = m_Sections;
	
	m_Spacing = 2.0f;
	m_Padding = 2.0f;
}

void UIProgressBar::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// nobody likes small numbers in 3d 0.01 bleh
	if ( Up() < 1.0f )
	{
		m_Spacing /= 10.0f;
		m_Padding /= 10.0f;
	}
	m_CurrentPosition = m_Sections;
}

void UIProgressBar::Render()
{
	if ( !m_Activated )
		return;

	C_GadgetBase::Render();

	C_UIAutoTransform transform( this );

	// Calculate the justification manually - All sprites are drawn top left
	const Maths::HVec3 drawPos = m_Position + ( gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size );

	const F32 sizeX = ( ( m_Size.x + m_Spacing - ( m_Padding * 2.0f ) ) / m_Sections ) - m_Spacing;
	const F32 sizeY = m_Size.y - ( m_Padding * 2.0f );

	// Add the background / frame
	gSimple2D.AddSprite( m_Layer, drawPos, sizeX, m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

	gSimple2D.AddSprite( m_Layer, drawPos + Maths::HVec3( sizeX, 0.0f, 0.0f ), m_Size.x - ( sizeX * 2.0f ), m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );

	gSimple2D.AddSprite( m_Layer, drawPos + Maths::HVec3( m_Size.x - sizeX, 0.0f, 0.0f ), sizeX, m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );

	F32 x = drawPos.X() + m_Padding;
	F32 y = drawPos.Y() + ( m_Padding * Up() );

	// Draw the sections
	for ( S32 i = 0; i < m_CurrentPosition; i++ )
	{
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, drawPos.Z() ), sizeX, sizeY, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) );

		x += sizeX + m_Spacing;
	}

	for ( S32 i = m_CurrentPosition; i < m_Sections; i++ )
	{
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, drawPos.Z() ), sizeX, sizeY, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 4 ) );

		x += sizeX + m_Spacing;
	}
}

}; // namespace UI