#include "stdafx.h"

#include "ui/Game/Gadgets/UIGraph.h"
#include "ui/Graphics/Font/Font.h"

#include <algorithm>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIGraph, "UIGraph" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fAxisMinX, "axis_min_x" )
	XT_FIELD( m_fAxisMaxX, "axis_max_x" )
	XT_FIELD( m_fAxisMinY, "axis_min_y" )
	XT_FIELD( m_fAxisMaxY, "axis_max_y" )
	XT_FIELD( m_fAxisSize, "axis_size" )
	XT_FIELD( m_fPointSize, "point_size" )
	XT_FIELD( m_fLineSize, "line_size" )
	XT_FIELD( m_fBarSize, "bar_size" )
	XT_FIELD( m_fAnimTimer, "anim_timer" )
	XT_FIELD( m_bPoints, "points" )
	XT_FIELD( m_bLines, "lines" )
	XT_FIELD( m_bBars, "bars" )
	XT_FIELD( m_sSubDivX, "sub_div_x" )
	XT_FIELD( m_sSubDivY, "sub_div_y" )
	XT_FIELD( m_fDivSizeX, "div_size_x" )
	XT_FIELD( m_fDivSizeY, "div_size_y" )
	XT_FIELD( m_TitleX, "title_x" )
	XT_FIELD( m_TitleY, "title_y" )
	XT_FIELD( m_fFontHeight, "font_height" )
	XT_FIELD( m_FontStylePasses, "styles" )
	XT_FIELD( m_fTitleOffset, "title_offset" )
	XT_FIELD( m_bFlipAxes, "flip_axes" )
XT_END_TYPE

namespace UI
{

// -------
// UIGraph
// -------
void UIGraph::Init()
{
	C_GadgetBase::Init();

	m_fAxisMinX = 0.0f;
	m_fAxisMaxX = 1.0f;
	m_fAxisMinY = 0.0f;
	m_fAxisMaxY = 1.0f;
	m_fAxisSize = 2.0f;
	m_fPointSize = 5.0f;
	m_fLineSize = 3.0f;
	m_fBarSize = 5.0f;
	m_fAnimTimer = -1.0f;
	m_bPoints = true;
	m_bLines = true;
	m_bBars = true;
	m_sSubDivX = 0;
	m_sSubDivY = 0;
	m_fDivSizeX = 2.0f;
	m_fDivSizeY = 4.0f;
	m_TitleX.id = 0;
	m_TitleY.id = 0;
	m_fFontHeight = 20.0f;
	m_bFlipAxes = false;
}

void UIGraph::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	// resolve font styles
	m_FontStylePasses.Setup( pState );

	// bind loc ids
	m_idTitleX = m_TitleX.id;
	m_idTitleY = m_TitleY.id;

	if ( m_bFlipAxes )
	{
		Btl::Swap( m_fAxisMinX, m_fAxisMinY );
		Btl::Swap( m_fAxisMaxX, m_fAxisMaxY );
		Btl::Swap( m_sSubDivX, m_sSubDivY );
		Btl::Swap( m_idTitleX, m_idTitleY );
	}
}

void UIGraph::Render()
{
	if ( !m_Activated )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	C_GadgetBase::Render();

	// x sub-div
	for ( S32 i = 0; i < m_sSubDivX; i++ )
	{
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x + i * ( m_Size.x / ( m_sSubDivX - 1 ) ), m_Position.y + m_Size.y * Up(), m_Position.z ), 
			m_fDivSizeX, m_fDivSizeY, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
		gSimple2D.AddJustification( E_Justify_TopCentre );
	}

	// x axis
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x - m_fAxisSize, m_Position.y + m_Size.y * Up(), m_Position.z ), 
		m_Size.x + m_fAxisSize, m_fAxisSize, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_TopLeft );

	// x title
	if ( m_idTitleX.IsValid() && m_idTitleX.IsLocString() )
		gFont.DrawText( m_Layer, Maths::HVec3( m_Position.x + ( m_Size.x / 2.0f ), m_Position.y + ( m_Size.y + m_fDivSizeY + m_fTitleOffset ) * Up(), m_Position.z ),
			E_Justify_TopCentre, m_idTitleX.String(), m_SpriteColours, m_fFontHeight, m_FontStylePasses, 0.0f, m_Panel );

	// y sub-div
	for ( S32 i = 0; i < m_sSubDivY; i++ )
	{
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x, m_Position.y + ( m_Size.y - i * ( m_Size.y / ( m_sSubDivY - 1 ) ) ) * Up(), m_Position.z ), 
			m_fDivSizeY, m_fDivSizeX, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
		gSimple2D.AddJustification( E_Justify_MiddleRight );
	}

	// y axis
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( m_Position.x, m_Position.y + m_Size.y * Up(), m_Position.z ), 
		m_fAxisSize, m_Size.y, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_BottomRight );

	// y title
	if ( m_idTitleY.IsValid() && m_idTitleY.IsLocString() )
	{
		TransformIndex ti = gSimple2D.PopPostTransformMatrix();
		Maths::HMat43& mat = gSimple2D.GetPostTransformMatrix( ti );

		F32 x = m_Position.x - m_fDivSizeY - ( m_fFontHeight / 2.0f ) - m_fTitleOffset;
		F32 y = m_Position.y + ( m_Size.y / 2.0f ) * Up();

		Maths::HVec3 rotPivot( x, y, m_Position.z );
		mat.Translation( mat.Translation() - rotPivot );
		mat = mat * Maths::HMat43( Maths::C_MatrixRotateRPH( -Maths::PiBy2, 0.0f, 0.0f ) );
		mat.Translation( mat.Translation() + rotPivot );

		gFont.PostTransformMatrixNext( ti );
		gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ),
			E_Justify_MiddleCentre, m_idTitleY.String(), m_SpriteColours, m_fFontHeight, m_FontStylePasses, 0.0f, m_Panel );
	}

	for ( S32 i = 0; i < m_Data.Size(); i++ )
	{
		F32 x01 = ( m_Data[ i ].first - m_fAxisMinX ) / ( m_fAxisMaxX - m_fAxisMinX );
		F32 y01 = ( m_Data[ i ].second - m_fAxisMinY ) / ( m_fAxisMaxY - m_fAxisMinY );

		F32 x = m_Position.x + x01 * m_Size.x;
		F32 y = m_Position.y + ( m_Size.y - y01 * m_Size.y ) * Up();

		if ( ( m_fAnimTimer == -1.0f ) || ( x01 <= m_fAnimTimer ) )
		{
			if ( m_bBars )
			{
				F32 xx01 = x01 + ( m_fBarSize / m_Size.x );

				F32 fMult01 = 1.0f;

				if ( ( m_fAnimTimer != -1.0f ) && ( m_fAnimTimer < xx01 ) )
					fMult01 = ( ( m_fAnimTimer - x01 ) / ( xx01 - x01 ) );

				gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, m_Position.y + m_Size.y * Up(), m_Position.z ), 
					m_fBarSize, Maths::Abs( y01 ) * m_Size.y * fMult01, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
				gSimple2D.AddJustification( ( y01 > 0.0f ) ? E_Justify_BottomCentre : E_Justify_TopCentre );
			}

			if ( m_bLines && ( i != ( m_Data.Size() - 1 ) ) )
			{
				F32 xx01 = ( m_Data[ i + 1 ].first - m_fAxisMinX ) / ( m_fAxisMaxX - m_fAxisMinX );
				F32 yy01 = ( m_Data[ i + 1 ].second - m_fAxisMinY ) / ( m_fAxisMaxY - m_fAxisMinY );

				F32 xx = m_Position.x + xx01 * m_Size.x;
				F32 yy = m_Position.y + ( m_Size.y - yy01 * m_Size.y ) * Up();

				F32 fMult01 = 1.0f;

				if ( ( m_fAnimTimer != -1.0f ) && ( m_fAnimTimer < xx01 ) )
					fMult01 = ( ( m_fAnimTimer - x01 ) / ( xx01 - x01 ) );

				Maths::HVec2 diff( Maths::HVec2( xx, yy ) - Maths::HVec2( x, y ) );
				F32 fAngle = atan2f( diff.Y(), diff.X() );

				gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), 
					diff.Length() * fMult01, m_fLineSize, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
				gSimple2D.AddJustification( E_Justify_MiddleLeft );
				gSimple2D.AddRotation( 0.0f, 0.0f, Maths::RadToDeg( fAngle ) );
			}

			if ( m_bPoints )
			{
				gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), 
					m_fPointSize, m_fPointSize, m_SpriteColours, m_Panel, E_SpriteType_Sprite, m_RenderState );
				gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
				gSimple2D.AddJustification( E_Justify_MiddleCentre );
			}
		}
	}
}

void UIGraph::AddData( const F32 x, const F32 y )
{
	m_Data.PushBack( m_bFlipAxes ? DataPair( y, x ) : DataPair( x, y ) );
	std::sort( m_Data.Begin(), m_Data.End() );
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
