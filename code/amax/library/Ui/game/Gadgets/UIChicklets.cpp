#include "stdafx.h"

#include "UIChicklets.h"
#include "UiHelpers.h"

#include "UIMessages.h"

#include <ui/Graphics/SimpleTimer.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIChicklets, "UIChicklets" )
	XT_FIELD( m_sNumSegments, "num_segments" )
	XT_FIELD( m_fMinValue, "min_value" )
	XT_FIELD( m_fMaxValue, "max_value" )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_xtColourOff, "colour_off" )
	XT_FIELD( m_xtColourOn, "colour_on" )
	XT_FIELD( m_fSegmentGapX, "segment_gap_x" )
	XT_FIELD( m_fTextGapX, "gap_x" )
	XT_FIELD( m_fTextHeight, "text_height" )
	XT_FIELD( m_fEffectMult, "effect_mult" )
	XT_FIELD( m_xtTextId, "title" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

namespace UI
{

// -----------
// UIChicklets
// -----------
void UIChicklets::Init()
{
	C_GadgetBase::Init();

	m_sNumSegments = 0;
	m_fMinValue = 0.0f;
	m_fMaxValue = 0.0f;
	m_fCurrentValue = 0.0f;
	m_fTargetValue = 0.0f;
	m_fSegmentGapX = 0.0f;
	m_fTextGapX = 0.0f;
	m_fTextHeight = 0.0f;
	m_fEffectMult = 1.0f;

	m_bHasResolvedColours = false;

	// message
	AddSubscriber( E_UiMsg_FrameUpdate );
}

void UIChicklets::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve font styles
	m_FontStylePasses.Setup( pState );

	// resolve text
	m_TextId = m_xtTextId.id;
}

void UIChicklets::FrameUpdate()
{
	if ( !m_Activated )
		return;

	C_GadgetBase::FrameUpdate();

	F32 fFrameTime = theFETimer.FrameTime();

	if ( m_fCurrentValue != m_fTargetValue )
	{
		F32 fStep = fFrameTime * m_fEffectMult;

		if ( m_fCurrentValue < m_fTargetValue )
		{
			m_fCurrentValue += fStep;

			if ( m_fCurrentValue > m_fTargetValue )
				m_fCurrentValue = m_fTargetValue;
		}
		else
		{
			m_fCurrentValue -= fStep;

			if ( m_fCurrentValue < m_fTargetValue )
				m_fCurrentValue = m_fTargetValue;
		}
	}
}

void UIChicklets::Render()
{
	if ( !m_Activated )
		return;

	// bit weird, but these are set with SetXtVar after cloning and so would need re-resolving
	if ( !m_bHasResolvedColours )
	{
		m_bHasResolvedColours = true;

		m_ColourOff.SetByName( m_xtColourOff );
		m_ColourOn.SetByName( m_xtColourOn );
	}

	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	F32 fValue = ( ( m_fCurrentValue + m_fMinValue ) / ( m_fMaxValue - m_fMinValue ) ) * m_sNumSegments;

	C_UIColour col;

	for ( S32 i = 0; i < m_sNumSegments; i++ )
	{
		if ( fValue >= ( i + 1 ) )
			col = m_ColourOn[ 0 ];
		else if ( ceilf( fValue ) < ( i + 1 ) )
			col = m_ColourOff[ 0 ];
		else
		{
			// this is always half
			col = m_ColourOn[ 0 ];
			col.R01( col.R01() / 2.0f );
			col.G01( col.G01() / 2.0f );
			col.B01( col.B01() / 2.0f );
		}

		col.A01( col.A01() * m_AlphaInherit );

		F32 fOffsetX = ( m_fTextGapX / 2.0f ) + ( m_Size.x + m_fSegmentGapX ) * i;

		gSimple2D.AddSprite( m_Layer, m_Position + Maths::HVec3( fOffsetX, 0.0f, 0.0f ), m_Size.x, m_Size.y, 
			col, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_MiddleLeft );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	}

	if ( m_TextId.IsValid() && m_TextId.IsLocString() )
	{
		gFont.DrawText( m_Layer, m_Position + Maths::HVec3( -m_fTextGapX / 2.0f, 0.0f, 0.0f ), E_Justify_MiddleRight, m_TextId, 
			m_SpriteColours, m_fTextHeight, m_FontStylePasses, 0.0f, m_Panel );
	}
}

void UIChicklets::SetValue( const F32 fValue )
{
	m_fTargetValue = Maths::Clamp( fValue, m_fMinValue, m_fMaxValue );
}

}; // namespace UI
