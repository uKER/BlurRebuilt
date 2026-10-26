
#include "stdafx.h"

#include "ui/Illustrator/UIAdobePath.h"
/*
#ifndef CFG_GOLD // for debug rendering
#include "ui/Graphics/Simple2d.h"
#include "ui/Graphics/Font/Font.h"
Debug::Bool g_UiRenderSplineInfo( "UI.Render.RenderSplineInfo", false );
Debug::Float g_UiRenderSplineSpace( "UI.Render.RenderSplineSpace", 0.0f, Debug::Range( 0.0f, 1.0f ) );
#endif
*/

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

/*---------------------------------------------------------------------------------------
	Constants
---------------------------------------------------------------------------------------*/
const F32 kfEpsilon = 1.0e-06f;
const U16 kfMaxIterations = 32;


/*---------------------------------------------------------------------------------------
	C_AdobePath functions
---------------------------------------------------------------------------------------*/
C_AdobePath::C_AdobePath()
	: C_AdobeElement( Ipf::E_ElementType_Path ),
	  m_TotalLength( 0.0f )
{
}

C_AdobePath::~C_AdobePath()
{
}


const void C_AdobePath::AddPoint( const C_AdobePoint& point )
{
	if ( !m_Path.Size() )
		m_LastPoint = point.m_Pos;
	
	// Add new point and calculate its length
	m_Path.PushBack( point );

	// Update the bounding box
	m_Bounds.UpdateExtents( Maths::C_Vertex( point.m_Pos.X(), point.m_Pos.Y(), 0.0f ) );
}

const void C_AdobePath::CalculateLengths()
{
	if ( m_Path.Size() <= 0 )
		return;

	// Calculate arc lengths and total length.
	for ( S32 i = 0; i < m_Path.Size(); i++ )
	{
		m_Path[i].m_Length = GetArcLength( i, 0.0f, 1.0f );
		m_TotalLength += m_Path[i].m_Length;
		m_Path[i].m_DistFromStart = m_TotalLength;
	}

	// Paramatize length.
	for ( S32 i = 0; i < m_Path.Size(); i++ )
	{
		m_Path[i].m_Time = ( m_Path[i].m_DistFromStart / m_TotalLength );
	}
}


const bool C_AdobePath::EvaluateSpline( AdobePointList& pointList, const F32 fCurveStep, const bool bClosed )
{
	S32 size = m_Path.Size();
	S32 count = 0;

	C_AdobePoint* pCurrentPoint = NULL;
	C_AdobePoint* pNextPoint = NULL;

	// Allocate enough space for the existing points and the new points.
	S32 sortedSize = size + ( size * (S32)fCurveStep );
	pointList.Reserve( sortedSize );

	S32 index = 0;
	pCurrentPoint = &m_Path[index];
	pNextPoint = &m_Path[++index];

	F32 fStep = 1.0f / fCurveStep;

	for ( F32 i = 0.0f; i <= 1.0f; i += fStep )
	{
		if ( i > pNextPoint->m_Time )
		{
			pCurrentPoint = pNextPoint;
			pNextPoint = &m_Path[++index];
		}

		// calculate curve point between current and next
		Maths::HVec2 &p1 = pCurrentPoint->m_Pos;
		Maths::HVec2 &p2 = pCurrentPoint->m_Out;
		Maths::HVec2 &p3 = pNextPoint->m_In;
		Maths::HVec2 &p4 = pNextPoint->m_Pos;

		F32 mu = ( i - pCurrentPoint->m_Time ) / ( pNextPoint->m_Time - pCurrentPoint->m_Time );
		F32 mum1 = 1 - mu;
		F32 mum13 = mum1 * mum1 * mum1;
		F32 mu3 = mu * mu * mu;

		Maths::HVec2 pos = (p1*mum13) + (p2*3.0f*mu*mum1*mum1) + (p3*3.0f*mu*mu*mum1) + (p4*mu3);

		C_AdobePoint point( pos.X(), pos.Y() );
		pointList.PushBack( point );
	}

	return true;
}


const bool C_AdobePath::EvaluateSplineFixedSpacing( AdobePointList& pointList, const F32 fSpacing )
{
	TCurvePointList list;
	S32 size = m_Path.Size();

	// add first point
	S32 firstIndex = 0;

	C_AdobePoint newPoint( m_Path[firstIndex].m_Pos.X(), m_Path[firstIndex].m_Pos.Y() );
	pointList.PushBack( newPoint );

	// Add all but the last point.
	for ( S32 i = 0; i < size - 1; i++ )
		SubDivideArc( list, m_Path[i].m_Pos, m_Path[i].m_Out, m_Path[i+1].m_In, m_Path[i+1].m_Pos, fSpacing );


	// Copy points over.
	for ( TCurvePointList::Iterator iter = list.Begin(); iter != list.End(); ++iter )
	{
		C_AdobePoint newPoint1( (*iter).X(), (*iter).Y() );
		pointList.PushBack( newPoint1 );
	}

	// add last point
	S32 lastIndex = size - 1;
	C_AdobePoint newPoint2( m_Path[lastIndex].m_Pos.X(), m_Path[lastIndex].m_Pos.Y() );
	pointList.PushBack( newPoint2 );

	return true;
}


const void C_AdobePath::SubDivideArc( TCurvePointList& list, const Maths::HVec2 p0, const Maths::HVec2 p1, const Maths::HVec2 p2, const Maths::HVec2 p3, const F32& fSpacing )
{
	// Calculate control points for subdivision
	Maths::HVec2 l1 = Maths::HVec2( p0 + p1 ) * 0.5f;
	Maths::HVec2 h = Maths::HVec2( p1 + p2 ) * 0.5f;
	Maths::HVec2 l2 = Maths::HVec2( l1 + h ) * 0.5f;

	Maths::HVec2 r2 = Maths::HVec2( p2 + p3 ) * 0.5f;
	Maths::HVec2 r1 = Maths::HVec2( h + r2 ) * 0.5f;
	Maths::HVec2 mid = Maths::HVec2( l2 + r1 ) * 0.5f;

	if ( Maths::HVec2( p0 - p3 ).Length() < fSpacing )
	{
		list.PushBack( mid );
		return;
	}
	else
	{
		SubDivideArc( list, p0, l1, l2, mid, fSpacing );
		SubDivideArc( list, mid, r1, r2, p3, fSpacing );
	}

	/*
	// Add first point.
	list.PushBack( p0 );

	if ( Maths::HVec2( p0 - p3 ).Length() < fSpacing )
		return;

	// Calculate control points for subdivision
	Maths::HVec2 l1 = Maths::HVec2( p0 + p1 ) * 0.5f;
	Maths::HVec2 h = Maths::HVec2( p1 + p2 ) * 0.5f;
	Maths::HVec2 l2 = Maths::HVec2( l1 + h ) * 0.5f;

	Maths::HVec2 r2 = Maths::HVec2( p2 + p3 ) * 0.5f;
	Maths::HVec2 r1 = Maths::HVec2( h + r2 ) * 0.5f;
	Maths::HVec2 mid = Maths::HVec2( l2 + r1 ) * 0.5f;

	SubDivideArc( list, p0, l1, l2, mid, fSpacing );
	SubDivideArc( list, mid, r1, r2, p3, fSpacing );
	*/
}


Maths::HVec2 C_AdobePath::SubDividePoints( const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& p3,
										Maths::HVec2& l1, Maths::HVec2& l2, Maths::HVec2& r1, Maths::HVec2& r2 )
{
	Maths::HVec2 h = Maths::HVec2( p1 + p2 ) * 0.5f;

	l1 = Maths::HVec2( p0 + p1 ) * 0.5f;
	r2 = Maths::HVec2( p2 + p3 ) * 0.5f;

	l2 = Maths::HVec2( l1 + h ) * 0.5f;
	r1 = Maths::HVec2( h + r2 ) * 0.5f;

	return ( Maths::HVec2( l2 + r1 ) * 0.5f );
}


Maths::HVec2 C_AdobePath::SubDividePoints( const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& p3 )
{
	Maths::HVec2 h = Maths::HVec2( p1 + p2 ) * 0.5f;

	Maths::HVec2 l1 = Maths::HVec2( p0 + p1 ) * 0.5f;
	Maths::HVec2 r2 = Maths::HVec2( p2 + p3 ) * 0.5f;

	Maths::HVec2 l2 = Maths::HVec2( l1 + h ) * 0.5f;
	Maths::HVec2 r1 = Maths::HVec2( h + r2 ) * 0.5f;

	return ( Maths::HVec2( l2 + r1 ) * 0.5f );
}


Maths::HVec2 C_AdobePath::MidPoint( Maths::HVec2& p1, Maths::HVec2& p2 )
{
	return Maths::HVec2( ( p1 + p2 ) * 0.5f );
}


const void C_AdobePath::RemovePoints()
{
	m_Path.Clear();
	m_Bounds = Maths::C_AxisBox();
	m_LastPoint = Maths::HVec3();

	m_TotalLength = 0.0f;
}


const F32 C_AdobePath::GetArcLength( const U32 index, F32 min, F32 max )
{
    assert( index >= 0 && index < (U32)m_Path.Size() );

	min = Maths::Clamp( min, 0.0f, 1.0f );
	max = Maths::Clamp( max, 0.0f, 1.0f );

	if ( m_Path.Size() < 2 || index == 0 )
		return min;

	if ( index == (U32)m_Path.Size() - 1 )
		return max;

	Maths::HVec2 &p0 = m_Path[index].m_Pos;
	Maths::HVec2 &p1 = m_Path[index].m_Out;
	Maths::HVec2 &p2 = m_Path[index-1].m_In;
	Maths::HVec2 &p3 = m_Path[index-1].m_Pos;

    // get control points for subcurve from 0.0 to max (de Casteljau's method)
	F32 minusMax = ( 1.0f - max );
	Maths::HVec2 l1 = ( p0 * minusMax ) + ( p1 * max );
	Maths::HVec2 h = ( p1 * minusMax ) + ( p2 * max );
	Maths::HVec2 l2 = ( l1 * minusMax ) + ( h * max );
	Maths::HVec2 l3 = ( l2 * minusMax ) + ( ( ( h * minusMax ) + ( ( p2 * minusMax + p3 * max ) * max ) ) * max );

	F32 minusMin = ( 1.0f - min );
	h = l1 * minusMin + l2 * minusMin;
	Maths::HVec2 r3 = l3;
	Maths::HVec2 r2 = l2 * minusMin + l3 * min;
	Maths::HVec2 r1 = h * minusMin + r2 * min;
	Maths::HVec2 r0 = ( ( ( ( p0 * minusMin + l1 * min ) * minusMin ) + h * min ) * minusMin ) + r1 * min;

	return SubDivideLength( r0, r1, r2, r3 );
}


const F32 C_AdobePath::SubDivideLength( const Maths::HVec2& p0, const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& p3 )
{
	Maths::HVec2 d0( p0 - p3 );
	Maths::HVec2 d1( p0 - p1 );
	Maths::HVec2 d2( p1 - p2 );
	Maths::HVec2 d3( p2 - p3 );

	// check to see if basically straight
	F32 minLength = d0.Length();
	F32 maxLength = d1.Length() + d2.Length() + d3.Length();
	F32 diff = minLength - maxLength;

	if ( diff * diff < 1.0e-3f )
		return 0.5f * ( minLength + maxLength );

    // otherwise get control points for subdivision
	Maths::HVec2 l1 = ( p0 + p1 ) * 0.5f;
	Maths::HVec2 h = ( p1 + p2 ) * 0.5f;
	Maths::HVec2 l2 = ( l1 + h ) * 0.5f;
	Maths::HVec2 r2 = ( p2 + p3 ) * 0.5f;
	Maths::HVec2 r1 = ( h + r2 ) * 0.5f;
	Maths::HVec2 mid = ( l2 + r1 ) * 0.5f;

    // subdivide
    return SubDivideLength( p0, l1, l2, mid ) + SubDivideLength( mid, r1, r2, p3 );
}


const Maths::HVec3 C_AdobePath::GetPointAt( const F32 distance, const Maths::HVec2& offset )
{
	if ( !m_Path.Size() )
		return Maths::HVec3( 0.0f, 0.0f, 0.0f );

	C_AdobePoint* pPoint1 = NULL;
	C_AdobePoint* pPoint2 = NULL;

	for ( S32 i = 0; i < m_Path.Size() - 1; i++ )
	{
		if ( distance <= m_Path[i+1].m_Time )
		{
			pPoint1 = &m_Path[i];
			pPoint2 = &m_Path[i + 1];
			break;
		}
	}
	// Can't succeed without this.
	if( !pPoint1 && !pPoint2 )
		return Maths::HVec3( 0.0f, 0.0f, 0.0f );

	// calculate curve point between current and next
	Maths::HVec2 &p1 = pPoint1->m_Pos;
	Maths::HVec2 &p2 = pPoint1->m_Out;
	Maths::HVec2 &p3 = pPoint2->m_In;
	Maths::HVec2 &p4 = pPoint2->m_Pos;

	// Get the current point.
	F32 mu = 1.0f - ( ( pPoint2->m_Time - distance ) / ( pPoint2->m_Time - pPoint1->m_Time ) ) ;
	F32 mum1 = 1 - mu;
	F32 mum13 = mum1 * mum1 * mum1;
	F32 mu3 = mu * mu * mu;

	return Maths::HVec3( ( (p1 - offset )*mum13) + ( (p2 - offset )*3.0f*mu*mum1*mum1) + ((p3 - offset )*3.0f*mu*mu*mum1) + ((p4 - offset )*mu3) );
}


Maths::HVec3 C_AdobePath::GetPositionAt( const F32 distance, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const bool bOffset )
{
	F32 fDistance = Maths::Clamp( distance, 0.0f, 1.0f );

	// Remove file offset.
	Maths::HVec2 offset( 0.0f, 0.0f );
	if ( bOffset )
		offset = Maths::HVec2( m_Path[0].m_Pos.X(), m_Path[0].m_Pos.Y() );

	Maths::HVec3 pos = GetPointAt( fDistance, offset );

	return Maths::HVec3( drawPos.X() + ( pos.X() * drawSizeX ), drawPos.Y() + ( pos.Y() * drawSizeY ), drawPos.Z() );
}

const F32 C_AdobePath::GetRotationAt( const F32 distance, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const bool bOffset )
{
	if ( !m_Path.Size() )
		return 0.0f;

	F32 fDistance1 = Maths::Clamp( distance, 0.0f, 1.0f );
	F32 fDistance2 = Maths::Clamp( distance + ( 0.05f / C_AdobePath::m_SplineCurveStep ), 0.0f, 1.0f );

	// Remove file offset.
	Maths::HVec2 offset( 0.0f, 0.0f );
	if ( bOffset )
		offset = Maths::HVec2( m_Path[0].m_Pos.X(), m_Path[0].m_Pos.Y() );

	Maths::HVec3 pos1 = GetPointAt( fDistance1, offset );
	Maths::HVec3 pos2 = GetPointAt( fDistance2, offset );

	// Facing vector.
	Maths::HVec3 dir = ( pos2 - pos1 ) ^ Maths::HVec3( 0.0f, 0.0f, -1.0f );
	dir.Normalise();

	F32 fDirAngle = -atan2f( dir.X(), dir.Y() );

	return Maths::RadToDeg( fDirAngle );
}

/*
#ifndef CFG_GOLD
void C_AdobePath::RenderSplineInfo( const F32 distance, const U8 layer, const Maths::HVec3& drawPos, const F32 drawSizeX, const F32 drawSizeY, const U8 panel, const bool bOffset )
{
	if ( !m_Path.Size() )
		return;

	F32 fDistance = Maths::Clamp( distance, 0.0f, 1.0f );

	// Remove file offset.
	Maths::HVec2 offset( 0.0f, 0.0f );
	if ( bOffset )
		offset = Maths::HVec2( m_Path[0].m_Pos.X(), m_Path[0].m_Pos.Y() );

	// Render the spline control points.
	F32 pathX = 0.0f;
	F32 pathY = 0.0f;
	Maths::HVec3 pathPos( 0.0f, 0.0f, 0.0f );

	C_AdobePoint* pCurrentPoint = NULL;
	C_AdobePoint* pNextPoint = NULL;

	// Iterate through all of the points on this polygon.
	for ( S32 i = 0; i < m_Path.Size(); i++ )
	{
		// Get the current and next points.
		pCurrentPoint = &m_Path[i];

		if ( i < m_Path.Size()-1 )
		{
			pNextPoint = &m_Path[i+1];

			// calculate curve point between current and next
			Maths::HVec2 &p1 = pCurrentPoint->m_Pos;
			Maths::HVec2 &p2 = pCurrentPoint->m_Out;
			Maths::HVec2 &p3 = pNextPoint->m_In;
			Maths::HVec2 &p4 = pNextPoint->m_Pos;

			F32 fStep = 1.0f / (F32)C_AdobePath::m_SplineCurveStep;
		
			Maths::HVec3 lastPos( pCurrentPoint->m_Pos );

			for ( F32 a = ( 0.0f + fStep ); a < 1.0f; a += fStep )
			{
				F32 mu = a;
				F32 mum1 = 1 - mu;
				F32 mum13 = mum1 * mum1 * mum1;
				F32 mu3 = mu * mu * mu;

				Maths::HVec3 pos( ( (p1 - offset )*mum13) + ( (p2 - offset )*3.0f*mu*mum1*mum1) + ((p3 - offset )*3.0f*mu*mu*mum1) + ((p4 - offset )*mu3) );

				if ( Maths::Distance( lastPos, pos ) > g_UiRenderSplineSpace )
				{
					lastPos = pos;

					pathPos = Maths::HVec3( drawPos.X() + ( pos.X() * drawSizeX ), drawPos.Y() + ( pos.Y() * drawSizeY ), drawPos.Z() );

					gSimple2D.AddSprite( layer+1, pathPos, 1.0f, 1.0f, 0xffffffff, panel );
					gSimple2D.AddJustification( E_Justify_MiddleCentre );
					gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
				}
			}
		}

		pathX = ( ( m_Path[i].m_Pos.X() - offset.X() ) * drawSizeX );
		pathY = ( ( m_Path[i].m_Pos.Y() - offset.Y() )  * drawSizeY );
		pathPos = Maths::HVec3( drawPos.X() + pathX, drawPos.Y() + pathY, drawPos.Z() );
		gSimple2D.AddSprite( layer, pathPos, 4.0f, 4.0f, 0xffff0000, panel );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );

		C_WideString16 buff;
		buff.Format( L"%.2f - %.2f" , m_Path[i].m_Time, m_Path[i].m_Length );
		gFont.DrawDebugText( layer, pathPos + Maths::HVec3( 0.0f, 12.0f, 0.0f ), E_Justify_MiddleCentre, buff, 0xffffffff, 8.0f, 0.0f, panel );

		pathX = ( ( m_Path[i].m_In.X() - offset.X() )  * drawSizeX );
		pathY = ( ( m_Path[i].m_In.Y() - offset.Y() )  * drawSizeY );
		pathPos = Maths::HVec3( drawPos.X() + pathX, drawPos.Y() + pathY, drawPos.Z() );
		gSimple2D.AddSprite( layer, pathPos, 2.0f, 2.0f, 0xff0000ff, panel );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );

		pathX = ( ( m_Path[i].m_Out.X() - offset.X() )  * drawSizeX );
		pathY = ( ( m_Path[i].m_Out.Y() - offset.Y() )  * drawSizeY );
		pathPos = Maths::HVec3( drawPos.X() + pathX, drawPos.Y() + pathY, drawPos.Z() );
		gSimple2D.AddSprite( layer, pathPos, 2.0f, 2.0f, 0xff00ff00, panel );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
	}

	//---------------------------------
	// Draw the rotation information.
	//---------------------------------

	F32 fDistance1 = Maths::Clamp( distance, 0.0f, 1.0f );
	F32 fDistance2 = Maths::Clamp( distance + ( 0.05f / C_AdobePath::m_SplineCurveStep ), 0.0f, 1.0f );

	Maths::HVec3 pos1 = GetPointAt( fDistance1, offset );
	Maths::HVec3 pos2 = GetPointAt( fDistance2, offset );

	// Facing vector.
	//Maths::HVec3 dir = ( pos2 - pos1 ) ^ Maths::HVec3( 0.0f, 0.0f, ( pos2.X() > pos1.X() ) ? -1.0f : 1.0f );
	Maths::HVec3 dir = ( pos2 - pos1 ) ^ Maths::HVec3( 0.0f, 0.0f, -1.0f );
	dir.Normalise();

	F32 fDirAngle = -atan2f( dir.X(), dir.Y() );

	if ( g_UiRenderSplineInfo )
	{
		// Debug draw the normal and spline information
		Maths::HVec3 pathPos = Maths::HVec3( drawPos.X() + ( pos1.X() * drawSizeX ), drawPos.Y() + ( pos1.Y() * drawSizeY ), drawPos.Z() );

		gSimple2D.AddSprite( layer, pathPos, 4.0f, 4.0f, 0xff00ffff, panel );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
		gSimple2D.AddRotation( 0.0f, 0.0f, Maths::RadToDeg( fDirAngle ) );

		pathPos = Maths::HVec3( drawPos.X() + ( pos2.X() * drawSizeX ), drawPos.Y() + ( pos2.Y() * drawSizeY ), drawPos.Z() );

		gSimple2D.AddSprite( layer, pathPos, 4.0f, 4.0f, 0xffffff00, panel );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
		gSimple2D.AddRotation( 0.0f, 0.0f, Maths::RadToDeg( fDirAngle ) );

		for ( S32 i = 0; i < 10; i++ )
		{
			pathPos = Maths::HVec3( drawPos.X() + ( pos1.X() * drawSizeX ), drawPos.Y() + ( pos1.Y() * drawSizeY ), drawPos.Z() );
			gSimple2D.AddSprite( layer, pathPos + ( dir * ( 4.0f * i ) ), 1.0f, 1.0f, 0xffffff00, panel );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
		}
		for ( S32 i = 0; i < 10; i++ )
		{
			pathPos = Maths::HVec3( drawPos.X() + ( pos1.X() * drawSizeX ), drawPos.Y() + ( pos1.Y() * drawSizeY ), drawPos.Z() );
			gSimple2D.AddSprite( layer, pathPos + ( Maths::HVec3( 0.0f, 1.0f, 0.0f ) * ( 4.0f * i ) ), 1.0f, 1.0f, 0xffff0000, panel );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );
			gSimple2D.AddTexUVs( 0, gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
		}

		C_WideString16 buff;
		buff.Format( L"%.2f", Maths::RadToDeg( fDirAngle ) );
		gFont.DrawDebugText( layer, pathPos + Maths::HVec3( 0.0f, 15.0f, 0.0f ), E_Justify_MiddleCentre, buff, 0xffffffff, 12.0f, 0.0f, panel );
	}
}
#endif // CFG_GOLD
*/

}; // namespace UI


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
