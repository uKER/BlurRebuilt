
#include "stdafx.h"

#include "ui/Game/Gadgets/UIScrollBar.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", off )
#endif


XT_TYPE( UI::UIScrollBar, "UIScrollBar" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_StyleRef, "style" )
	XT_FIELD( m_ScrollType, "scroll_type" )
	XT_FIELD( m_BoundsMin, "bounds_min" )
	XT_FIELD( m_BoundsMax, "bounds_max" )
	XT_FIELD( m_ScrollArea, "scroll_area" )
	XT_FIELD( m_bAlwaysShow, "always_show" )
XT_END_TYPE

XT_TYPE( UI::C_ScrollBarStyle, "UIScrollBarStyle" )
	XT_FIELD( m_EndSizeY, "endSizeY" )
	XT_FIELD( m_SliderPaddingX, "sliderPaddingX" )
	XT_FIELD( m_SliderPaddingY, "sliderPaddingY" )
	XT_FIELD( m_MinSliderSize, "minSliderSize" )
	XT_FIELD( m_MaxSliderSize, "maxSliderSize" )
	XT_FIELD( m_EndColour, "endColour" )
	XT_FIELD( m_SliderColour, "sliderColour" )
	XT_FIELD( m_BackgroundColour, "backgroundColour" )
	XT_FIELD( m_BorderColour, "borderColour" )
	XT_FIELD( m_BorderSize, "borderSize" )
XT_END_TYPE



/*---------------------------------------------------------------------------------------
	Debug Variables
---------------------------------------------------------------------------------------*/
#ifndef CFG_GOLD
Debug::Bool g_ShowScrollbarData( "UI.windows.RenderScrollBarData", false );
#endif // CFG_GOLD


namespace UI
{

C_ScrollBarStyle C_ScrollBarStyle::m_sInstance;

C_ScrollBarStyle::C_ScrollBarStyle()
:	m_EndSizeY( 10.0f ), 
	m_SliderPaddingX( 2.0f ),
	m_SliderPaddingY( 2.0f ),
	m_MinSliderSize( 0.0f ),
	m_MaxSliderSize( 0.0f ),
	m_EndColour( 0.4f, 0.4f, 0.4f, 1.0f ), 
	m_SliderColour( 0.4f, 0.6f, 0.8f, 1.0f ), 
	m_BackgroundColour( 0.0f, 0.0f, 0.0f, 1.0f ),
	m_BorderColour( 1.0f, 1.0f, 1.0f, 1.0f ),
	m_BorderSize( 1.0f )
{}

void UIScrollBar::Init()
{
	C_GadgetBase::Init();

	m_ScrollArea = 0.0f;	
	m_SliderPosition = 0.0f;

	m_BoundsMin = 50.0f;	
	m_BoundsMax = 100.0f;
	m_bAlwaysShow = true;

	m_ScrollType = E_ScrollBarType_Vertical;
}


void UIScrollBar::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_pStyle = ( m_StyleRef == NULL ) ? &C_ScrollBarStyle::Instance() : m_StyleRef;

	Build();
}


void UIScrollBar::Build( const bool bForceBuild )
{
	// Apply justification to the gadget.
	Maths::HVec3 offset = gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size;

	F32 x = m_Position.x + offset.X();
	F32 y = m_Position.y + offset.Y();

	if ( m_ScrollType == E_ScrollBarType_Vertical )
	{
		if ( bForceBuild || ( m_BoundsMin == 0.0f && m_BoundsMax == 0.0f ) )
		{
			m_BoundsMin = y;
			m_BoundsMax = y + m_Size.y;
		}
	}
	else
	if ( m_ScrollType == E_ScrollBarType_Horizontal )
	{
		if ( bForceBuild || ( m_BoundsMin == 0.0f && m_BoundsMax == 0.0f ) )
		{
			m_BoundsMin = x;
			m_BoundsMax = x + m_Size.x;
		}
	}

	// Add some defaults.
	if ( m_ScrollArea == 0.0f )
		m_ScrollArea = m_BoundsMax - m_BoundsMin;

	// Use some default values so that the scrollbar is visible.
	if ( m_BoundsMax <= m_BoundsMin )
		m_BoundsMax = m_BoundsMin + 1;

	SetScrollArea( m_ScrollArea );
}


void UIScrollBar::SetScrollArea( const F32 size )
{
	F32 maxSize = ( ( m_ScrollType == E_ScrollBarType_Vertical ) ? m_Size.y : m_Size.x ) - ( m_pStyle->m_EndSizeY * 2.0f ) - ( m_pStyle->m_BorderSize * 2.0f );

	if ( size > ( m_BoundsMax - m_BoundsMin ) )
	{
		m_ScrollBarSize = maxSize / ( size / ( m_BoundsMax - m_BoundsMin ) );

		// Cap slider size to min and max.
		if ( m_pStyle->m_MinSliderSize > 0.0f && m_ScrollBarSize < m_pStyle->m_MinSliderSize )
			m_ScrollBarSize = m_pStyle->m_MinSliderSize;

		if ( m_pStyle->m_MaxSliderSize > 0.0f && m_ScrollBarSize > m_pStyle->m_MaxSliderSize )
			m_ScrollBarSize = m_pStyle->m_MaxSliderSize;
	}
	else
		m_ScrollBarSize = maxSize;
	m_ScrollArea = size;
}


void UIScrollBar::Render()
{
	if ( !m_Activated )
		return;

	// Render??
	if ( !CanScroll() && !m_bAlwaysShow )
		return;

	C_UIAutoTransform transform( this );

	// Apply justification to the gadget.
	Maths::HVec3 offset = gSimple2D.GetJustifiedCorner( m_Justify, 0, m_Panel ) * m_Size;

	F32 x = m_Position.x + offset.X();
	F32 y = m_Position.y + offset.Y();
	F32 height = m_Size.y - ( m_pStyle->m_BorderSize * 2.0f );
	F32 width = m_Size.x - ( m_pStyle->m_BorderSize * 2.0f );

	if ( m_pStyle->m_BorderSize > 0.0f )
	{
		RenderBorder( x, y );

		// Add border size on.
		x += m_pStyle->m_BorderSize;
		y += m_pStyle->m_BorderSize * Up();
	}

	if ( m_ScrollType == E_ScrollBarType_Vertical )
	{
		// Debug render the scroll area.
		#ifndef CFG_GOLD
		if ( g_ShowScrollbarData )
		{
			gSimple2D.AddSprite( m_Layer, Maths::HVec3( x - 20.0f, m_BoundsMin, m_Position.z ), 15.0f, m_BoundsMax - m_BoundsMin, 0x77ffffff, m_Panel );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );

			gSimple2D.AddSprite( m_Layer, Maths::HVec3( x - 40.0f, m_BoundsMin, m_Position.z ), 15.0f, m_ScrollArea, 0x77ff4444, m_Panel );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
		}
		#endif // CFG_GOLD

		// Scale the slider accordinly.
		F32 top = y + m_pStyle->m_EndSizeY;
		F32 bottom = top + height - m_ScrollBarSize - ( m_pStyle->m_EndSizeY * 2 );
		F32 sliderPos = y + ( ( m_pStyle->m_EndSizeY + ( ( bottom - top ) * m_SliderPosition ) ) * Up() );

		/*-------------------------------------------------------------------------------------
			Render top piece.
		-------------------------------------------------------------------------------------*/
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), width, m_pStyle->m_EndSizeY, m_pStyle->m_EndColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );

		y += m_pStyle->m_EndSizeY * Up();

		/*-------------------------------------------------------------------------------------
			Render the background.
		-------------------------------------------------------------------------------------*/
		F32 middleSize = height - ( m_pStyle->m_EndSizeY * 2.0f );
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), width, middleSize, m_pStyle->m_BackgroundColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

		y += middleSize * Up();

		/*-------------------------------------------------------------------------------------
			Render bottom piece.
		-------------------------------------------------------------------------------------*/
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), width, m_pStyle->m_EndSizeY, m_pStyle->m_EndColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );

		y += m_pStyle->m_EndSizeY * Up();

		/*-------------------------------------------------------------------------------------
			Render the slider
		-------------------------------------------------------------------------------------*/
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x + m_pStyle->m_SliderPaddingX, sliderPos + ( m_pStyle->m_SliderPaddingY * Up() ), m_Position.z ), width - ( m_pStyle->m_SliderPaddingX * 2.0f ), m_ScrollBarSize - ( m_pStyle->m_SliderPaddingY * 2.0f ), m_pStyle->m_SliderColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );
	}
	else
	if ( m_ScrollType == E_ScrollBarType_Horizontal )
	{
		// Debug render the scroll area.
		#ifndef CFG_GOLD
		if ( g_ShowScrollbarData )
		{
			gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_BoundsMin, y - 5.0f, m_Position.z ), m_BoundsMax - m_BoundsMin, 15.0f, 0x77ffffff, m_Panel );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );

			gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_BoundsMin, y - 20.0f, m_Position.z ), m_ScrollArea, 15.0f, 0x77ff4444, m_Panel );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
		}
		#endif // CFG_GOLD

		// Scale the slider accordinly.
		F32 left = x + m_pStyle->m_EndSizeY;
		F32 right = left + width - m_ScrollBarSize - ( m_pStyle->m_EndSizeY * 2 );
		F32 sliderPos = x + m_pStyle->m_EndSizeY + ( ( right - left ) * m_SliderPosition );

		/*-------------------------------------------------------------------------------------
			Render top piece.
		-------------------------------------------------------------------------------------*/
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_pStyle->m_EndSizeY, height, m_pStyle->m_EndColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );

		x += m_pStyle->m_EndSizeY;

		/*-------------------------------------------------------------------------------------
			Render the background.
		-------------------------------------------------------------------------------------*/
		F32 middleSize = width - ( m_pStyle->m_EndSizeY * 2.0f );
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), middleSize, height, m_pStyle->m_BackgroundColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

		x += middleSize;

		/*-------------------------------------------------------------------------------------
			Render bottom piece.
		-------------------------------------------------------------------------------------*/
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_pStyle->m_EndSizeY, height, m_pStyle->m_EndColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 1 ) );

		x += m_pStyle->m_EndSizeY;

		/*-------------------------------------------------------------------------------------
			Render the slider
		-------------------------------------------------------------------------------------*/
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( sliderPos + m_pStyle->m_SliderPaddingX, y + m_pStyle->m_SliderPaddingY, m_Position.z ), m_ScrollBarSize - ( m_pStyle->m_SliderPaddingY * 2.0f ), height - ( m_pStyle->m_SliderPaddingY * 2.0f ), m_pStyle->m_SliderColour, m_Panel );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 2 ) );
	}
}


void UIScrollBar::RenderBorder( const F32& x, const F32& y )
{
	F32 middleSize = m_Size.x - ( m_pStyle->m_EndSizeY * 2.0f );
	F32 sideHeight = m_Size.y - ( m_pStyle->m_BorderSize * 2.0f );

	// Top
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), m_Size.x, m_pStyle->m_BorderSize, m_pStyle->m_BorderColour, m_Panel );
	gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );

	// Bottom
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y + (( m_Size.y - m_pStyle->m_BorderSize ) * Up()), m_Position.z ), m_Size.x, m_pStyle->m_BorderSize, m_pStyle->m_BorderColour, m_Panel );
	gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
	
	// Left
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y + m_pStyle->m_BorderSize * Up(), m_Position.z ), m_pStyle->m_BorderSize, sideHeight, m_pStyle->m_BorderColour, m_Panel );
	gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
	
	// Right
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x + m_Size.x - m_pStyle->m_BorderSize, y + m_pStyle->m_BorderSize * Up(), m_Position.z ), m_pStyle->m_BorderSize, sideHeight, m_pStyle->m_BorderColour, m_Panel );
	gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ) );
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize( "", on )
#endif
