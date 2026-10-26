#include "stdafx.h"

#include <Ui\Graphics\UIColourClasses.h>
#include <ui/Game/Gadgets/ButtonMessages.h>
#include <ui\Game\UIDataFinder.h>

#include "UIAmaxSlider.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

// XT Definition
XT_TYPE( UI::UIAmaxSlider, "UIAmaxSlider" )
	XT_BASE( UI::UISlider )
	XT_FIELD( m_FontOffset, "font_offset" )
	XT_FIELD( m_FontColourName, "font_colour_name" )
	XT_FIELD( m_ItemStyle, "item_style" )
	XT_FIELD( m_TitleStyle, "title_style" )
	XT_FIELD( m_BackingColourName, "backing_colour_name" )
	XT_FIELD( m_BackingSize, "backing_size" )
	XT_FIELD( m_SymbolColourName, "symbol_colour_name" )
	XT_FIELD( m_SymbolSize, "symbol_size" )
	XT_FIELD( m_PlusIntensity, "plus_intensity" )
	XT_FIELD( m_MinusIntensity, "minus_intensity" )
	XT_FIELD( m_ArrowPadding, "arrow_padding" )
XT_END_TYPE

namespace UI
{

void UIAmaxSlider::Setup( lua_State* pState )
{
	m_pLeftArrow = NULL;
	m_pRightArrow = NULL;

	m_bHasResolvedColours = false;

	m_FontColour.SetByName( m_FontColourName, m_Alpha, m_Intensity );
	m_BackingColour.SetByName( m_BackingColourName, m_Alpha, m_Intensity );
	
	m_ItemStyle.Setup( pState );
	m_TitleStyle.Setup( pState );

	// invalidate it
	m_MaxItemSizeX = -1;

	RemoveSubscriber( E_UiMsg_CtrlLeftStick_X );

	UISlider::Setup( pState );
}

void UIAmaxSlider::PostInit()
{
	m_pLeftArrow = FindChildByName( "left_arrow" );
	m_pRightArrow = FindChildByName( "right_arrow" );

	UpdateArrows();
	ActivateArrows();
}

void UIAmaxSlider::FrameUpdate()
{
	if ( !m_Activated )
		return;

	UISlider::FrameUpdate();

	ActivateArrows();
}

void UIAmaxSlider::UpdateArrows()
{
	if ( m_pLeftArrow && m_pRightArrow )
	{
		const F32 maxSize = GetItemMaxSizeX();
		const F32 maxSizeHalf = maxSize * 0.5f;

		m_pLeftArrow->m_Position.x = -( maxSizeHalf + m_ArrowPadding );
		m_pRightArrow->m_Position.x = ( maxSizeHalf + m_ArrowPadding );
	}
}

void UIAmaxSlider::ActivateArrows()
{
	if ( !m_pLeftArrow || !m_pRightArrow )
		return;

	if ( !m_Selected || m_Items.Size() == 1 )
	{
		// Turn both off
		m_pLeftArrow->Activate( false, false );
		m_pRightArrow->Activate( false, false );
	}
	else
	if ( m_CurrentItem == 0 )
	{
		// Turn right on only
		m_pLeftArrow->Activate( false, false );
		m_pRightArrow->Activate( true, false );
	}
	else
	if ( m_CurrentItem == m_Items.Size() -1 )
	{
		// Turn left on only
		m_pLeftArrow->Activate( true, false );
		m_pRightArrow->Activate( false, false );
	}
	else
	{
		// Turn both on
		m_pLeftArrow->Activate( true, false );
		m_pRightArrow->Activate( true, false );
	}
}

F32	UIAmaxSlider::GetItemMaxSizeX()
{
	F32 maxSizeX = 0.0f;

	for ( S32 i = 0; i < m_Items.Size(); i++ )
	{
		F32 sizeX = gFont.StringLength( GetString( i ), m_Size.y, m_ItemStyle.GetFontStyle( 0 ) );

		if ( sizeX > maxSizeX )
			maxSizeX = sizeX;
	}

	return maxSizeX;
}


bool UIAmaxSlider::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	return UISlider::MessageHandler( pMessage );
}

const F32 UIAmaxSlider::RenderAnalogue()
{
	// Backing
	gSimple2D.AddSprite( m_Layer, m_Position, m_BackingSize.x, m_BackingSize.y, m_BackingColour, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( m_Justify );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

	if( m_Items.Size() > 0 )
	{
		// Object
		gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( (F32)m_CurrentItem * m_Size.x / (F32)m_Items.Size(), 0.0f, 0.0f), m_Size.x, m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );
	}

	return m_Size.x;
}

const F32 UIAmaxSlider::RenderVariable()
{
	S32 items = m_Items.Size()-1;
	for( S32 i = 0; i < items; i++ )
	{
		gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( (F32)i * m_Size.x, 0.0f, 0.0f), m_Size.x, m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( m_Justify );
		gSimple2D.AddTexUVs( 0, GetTextureParam( ( i < m_CurrentItem ) ? 0 : 1 ) );
	}

	return m_Size.x * (F32)items;
}

void UIAmaxSlider::RenderVariableText()
{
	if( m_Items.Size() <= 0 )
		return;

	const WCHAR* pString = GetString( m_CurrentItem );
	if( !pString )
		return;

	gFont.DrawText( m_Layer, m_Position, m_Justify, pString, m_SpriteColours, m_Size.y, m_ItemStyle, 0.0f, m_Panel );
}

void UIAmaxSlider::Render()
{
	if(!m_Activated)
		return;	

	// bit weird, but these are set with SetXtVar after cloning and so would need re-resolving
	if ( !m_bHasResolvedColours )
	{
		m_bHasResolvedColours = true;

		m_FontColour.SetByName( m_FontColourName );
		m_BackingColour.SetByName( m_BackingColourName );
	}

	BaseUIVariableSelect::Render();
		
	// auto-transform
	C_UIAutoTransform transform( this );

	// Title
	if ( m_Text.IsLocString() )
		gFont.DrawText( m_Layer, Maths::HVec3(m_Position) + Maths::HVec3(m_FontOffset), m_Justify, m_Text, m_FontColour, m_FontHeight, m_TitleStyle, 0, m_Panel );
	else if( !m_Text.IsValid() && m_VariableName && m_VariableName[0] )
	{
		const WCHAR* pString = C_UIDataFinder::Instance().GetVariableText( m_VariableName );
		gFont.DrawText( m_Layer, Maths::HVec3(m_Position) + Maths::HVec3(m_FontOffset), m_Justify, pString, m_FontColour, m_FontHeight, m_TitleStyle, 0, m_Panel );
	}


	const F32 half_x = (m_SymbolSize.x / 2);

	// - Symbol (Texture 2)
	if( m_TextureParams.count >= 2 )
	{
		C_UISpriteColours symbol_colour;
		symbol_colour.SetByName( m_SymbolColourName, m_Alpha, m_MinusIntensity );
		symbol_colour.Set( symbol_colour, m_AlphaInherit, m_IntensityInherit );

		gSimple2D.AddSprite( m_Layer, m_Position - Maths::HVec3( half_x, 0.0f, 0.0f), m_SymbolSize.x, m_SymbolSize.y, symbol_colour, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );
	}

	// Bars (Textures 0 and 1)
	F32 size_x = 0.0;
	if( m_TextureParams.count >= 1 )
	{
		switch( m_SliderType )
		{
		case E_Type_Analogue:
			size_x = RenderAnalogue();
			break;

		case E_Type_Variable:
			size_x = RenderVariable();
			break;
		}
	}
	else
	{
		// Just the variable text
		switch( m_SliderType )
		{
		case E_Type_Analogue:
			// Not supported
			break;

		case E_Type_Variable:
			RenderVariableText();
			break;
		}
		
	}

	// + Symbol (Texture 3)
	if( m_TextureParams.count >= 3 )
	{
		C_UISpriteColours symbol_colour;
		symbol_colour.SetByName( m_SymbolColourName, m_Alpha, m_PlusIntensity );
		symbol_colour.Set( symbol_colour, m_AlphaInherit, m_IntensityInherit );

		gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( size_x + half_x, 0.0f, 0.0f), m_SymbolSize.x, m_SymbolSize.y, symbol_colour, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 3 ) );
	}

	BaseUIVariableSelect::Render();
}

}; // namespace UI