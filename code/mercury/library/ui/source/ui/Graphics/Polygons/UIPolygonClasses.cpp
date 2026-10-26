#include "stdafx.h"

#include "ui/Graphics/Polygons/UIPolygonClasses.h"
#include "ui/Illustrator/UIAdobePolygon.h"
#include "ui/Illustrator/UIAdobePoint.h"

Debug::Int		g_UiPolyTestRenderCount( "UI.PolyTest.RenderCount", 200, Debug::Range( 0, 200 ) );
Debug::Float	g_UiPolyScale( "UI.PolyTest.PolyScale", 0.45f );

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

Debug::Bool		g_UiPolyFontOutlineFix( "UI.PolygonFont.OutlineFix", true );
Debug::Float	g_UiPolyFontOutlineMaxMult( "UI.PolygonFont.OutlineMaxMult", 3.0f, Debug::Range( 1.0f, 5.0f ) );

namespace UI
{

// -----------------
// C_UIResultPolygon
// -----------------
C_UIResultPolygon::C_UIResultPolygon( C_UIClipPolygon& p )
	: m_bHoleProcessed( false )
{
	if ( !p.NumVertices() )
		return;

	C_UIClipVertex* pVertex = p[ 0 ];

	do 
	{
		AddPoint( pVertex->m_Pos );
		pVertex = pVertex->m_pNext;
	} 
	while( pVertex != p[ 0 ] );

	//for ( S32 i = 0; i < p.NumVertices(); i++ )
		//AddPoint( p[ i ]->m_Pos );
}


bool C_UIResultPolygon::CW()
{
	Maths::HF32 fSignedArea = 0.0f;

	for ( S32 i = 0; i < m_PointList.Size(); i++ )
        fSignedArea += ( m_PointList[ i ].GetPoint() ^ m_PointList[ ( i + 1 ) % m_PointList.Size() ].GetPoint() );

	F32 r( fSignedArea );
	return ( ( r < 0.0f && r > -0.01f ) || ( r >= 0.0f ) );
}

void C_UIResultPolygon::ReverseWindingOrder()
{
	// start vertex stays the same
	for ( S32 i = 0; i < (S32)floor( ( m_PointList.Size() - 1 ) / 2.0f ); i++ )
	{
		C_UIResultPoint temp = m_PointList[ i + 1 ];	// offset for start vertex
		m_PointList[ i + 1 ] = m_PointList[ m_PointList.Size() - 1 - i ];
		m_PointList[ m_PointList.Size() - 1 - i ] = temp;
	}
}

Maths::HVec2 C_UIResultPolygon::GetOutlinePoint( S32 sIndex )
{
	Maths::HVec2 p1 = ( sIndex == 0 ) ? m_PointList[ NumPoints() - 1 ].GetPoint(): m_PointList[ sIndex - 1 ].GetPoint();
	Maths::HVec2 p2 = m_PointList[ sIndex ].GetPoint();
	Maths::HVec2 p3 = m_PointList[ ( sIndex + 1 ) % NumPoints() ].GetPoint();

	// outline point is the intersection of the two extruded incident edges of this vertex

	Maths::HVec3 n1 = Maths::HVec3( p2 - p1 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
	n1.Normalise();
	Maths::HVec3 n2 = Maths::HVec3( p3 - p2 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
	n2.Normalise();

	Maths::HVec2 i1 = Maths::HVec2( Maths::HVec3( p1 ) + n1 );
	Maths::HVec2 i2 = Maths::HVec2( Maths::HVec3( p2 ) + n1 );
	Maths::HVec2 j1 = Maths::HVec2( Maths::HVec3( p2 ) + n2 );
	Maths::HVec2 j2 = Maths::HVec2( Maths::HVec3( p3 ) + n2 );

	F32 a = 0.0f;
	F32 b = 0.0f;

	bool bIntersect = UI::LineIntersect( i1, i2, j1, j2, a, b, false );

	Maths::HVec2 p( 0.0f, 0.0f );

	if ( bIntersect )
	{
		p = i1 + ( i2 - i1 ) * a; //p = j1 + ( j2 - j1 ) * b;

		if ( g_UiPolyFontOutlineFix )
		{
			// cap distance
			if ( ( p2 - p ).Length() > g_UiPolyFontOutlineMaxMult )
			{
				Maths::HVec2 n = ( p - p2 );
				n.Normalise();

				p = p2 + ( n * g_UiPolyFontOutlineMaxMult );
			}
		}
	}
	else
	{
		// use the approximate normal - it's not perfect but does the job in this situation
		Maths::HVec3 d1 = Maths::HVec3( p2 - p1 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
		d1.Normalise();
		Maths::HVec3 d2 = Maths::HVec3( p3 - p2 ) ^ Maths::HVec3( 0.0f, 0.0f, 1.0f );
		d2.Normalise();

		Maths::HVec3 ex = ( d1 + d2 ) * 0.5f;
		ex.Normalise();

		p = p2 + Maths::HVec2( ex );
	}

	return p;
}

// --------------
// C_UIClipVertex
// --------------
C_UIClipVertex::C_UIClipVertex()
{
	m_Pos.X( 0.0f );
	m_Pos.Y( 0.0f );
	m_pNext = m_pPrev = m_pNeighbour = NULL;
	m_bIntersect = false;
	m_fAlpha = 0.0f;
	m_bEntryStatus = false;
	m_bProcessed = false;
}

C_UIClipVertex::C_UIClipVertex( const Maths::HVec2& pos )
{
	m_Pos = pos;
	m_pNext = m_pPrev = m_pNeighbour = NULL;
	m_bIntersect = false;
	m_fAlpha = 0.0f;
	m_bEntryStatus = false;
	m_bProcessed = false;
}

C_UIClipVertex::C_UIClipVertex( const F32 x, const F32 y )
{
	m_Pos.X( x );
	m_Pos.Y( y );
	m_pNext = m_pPrev = m_pNeighbour = NULL;
	m_bIntersect = false;
	m_fAlpha = 0.0f;
	m_bEntryStatus = false;
	m_bProcessed = false;
}

C_UIClipVertex::~C_UIClipVertex()
{
	//
}

void C_UIClipVertex::Reset()
{
	m_pNext = m_pPrev = m_pNeighbour = NULL;
	m_bIntersect = false;
	m_fAlpha = 0.0f;
	m_bEntryStatus = false;
	m_bProcessed = false;
}

bool C_UIClipVertex::operator == ( const C_UIClipVertex& o )
{
	return ( ( m_Pos.X() == o.m_Pos.X() ) && ( m_Pos.Y() == o.m_Pos.Y() ) );
}

bool C_UIClipVertex::operator != ( const C_UIClipVertex& o )
{
	return ( ( m_Pos.X() != o.m_Pos.X() ) || ( m_Pos.Y() != o.m_Pos.Y() ) );
}

// ---------------
// C_UIClipPolygon
// ---------------
C_UIClipPolygon::C_UIClipPolygon( const bool bCleanup )
	: m_bCleanup( bCleanup )
{
	//
}

C_UIClipPolygon::C_UIClipPolygon( C_UIResultPolygon& p )
	: m_bCleanup( true )
{
	for ( U32 i = 0; i < p.NumPoints(); i++ )
		AddVertex( p[ i ].GetPoint() );

	CreateInitialNextAndBackLinks();
}


C_UIClipPolygon::~C_UIClipPolygon()
{
	if ( m_bCleanup )
	{
		// cleanup
		for ( S32 i = 0; i < m_Vertices.Size(); i++ )
			SAFE_DELETE_PTR( m_Vertices[ i ] );

		m_Vertices.Destroy();
	}

	// else we've just used this polygon instance as a storage class
}

void C_UIClipPolygon::Reset()
{
	// reset original vertices
	for ( S32 i = 0; i < m_sNumOriginalVertices; i++ )
		m_Vertices[ i ]->Reset();

	// now free all of the intersect ones
	if ( m_Vertices.Size() > m_sNumOriginalVertices )
	{
		S32 sCount = 0;
		for ( Btl::Vector< C_UIClipVertex* >::Iterator it = m_Vertices.Begin(); it != m_Vertices.End(); )
		{
			if ( sCount >= m_sNumOriginalVertices )
			{
				SAFE_DELETE_PTR( *it );
				it = m_Vertices.Erase( it );
			}
			else
				++it;

			sCount++;
		}

		m_Vertices.Reserve( m_sNumOriginalVertices );
	}

	// and re-create the next/back links
	CreateInitialNextAndBackLinks();
}

void C_UIClipPolygon::ResetProcessed()
{
	for ( S32 i = 0; i < m_Vertices.Size(); i++ )
		if ( m_Vertices[ i ] )
			m_Vertices[ i ]->m_bProcessed = false;
}

void C_UIClipPolygon::AddVertex( const F32 x, const F32 y )
{
	C_UIClipVertex* pVertex = new C_UIClipVertex( x, y );
	assert( pVertex );

	m_Vertices.PushBack( pVertex );
}

void C_UIClipPolygon::AddVertex( const Maths::HVec2& pos )
{
	AddVertex( pos.X(), pos.Y() );
}

void C_UIClipPolygon::AddVertex( C_UIClipVertex* pVertex )
{
	assert( pVertex );
	m_Vertices.PushBack( pVertex );
}

void C_UIClipPolygon::CreateInitialNextAndBackLinks()
{
	m_sNumOriginalVertices = m_Vertices.Size();

	if ( m_sNumOriginalVertices < 2 )
		return;

	for ( S32 i = 0; i < m_Vertices.Size(); i++ )
	{
		if ( i == 0 )
		{
			m_Vertices[ i ]->m_pPrev = m_Vertices[ m_Vertices.Size() - 1 ];
			m_Vertices[ i ]->m_pNext = m_Vertices[ i + 1 ];
		}
		else if ( i == m_Vertices.Size() - 1 )
		{
			m_Vertices[ i ]->m_pPrev = m_Vertices[ i - 1 ];
			m_Vertices[ i ]->m_pNext = m_Vertices[ 0 ];
		}
		else
		{
			m_Vertices[ i ]->m_pPrev = m_Vertices[ i - 1 ];
			m_Vertices[ i ]->m_pNext = m_Vertices[ i + 1 ];
		}
	}
}

void C_UIClipPolygon::AddAndSortInto( C_UIClipVertex* pAddVertex, C_UIClipVertex* pStart, C_UIClipVertex* pEnd )
{
	AddVertex( pAddVertex );

	C_UIClipVertex* pCurrent = pStart;

	while ( ( pCurrent != pEnd ) && ( pCurrent->m_fAlpha < pAddVertex->m_fAlpha ) )
		pCurrent = pCurrent->m_pNext;

	// insert here
	pAddVertex->m_pNext = pCurrent;
	pAddVertex->m_pPrev = pCurrent->m_pPrev;
	pAddVertex->m_pPrev->m_pNext = pAddVertex;
	pAddVertex->m_pNext->m_pPrev = pAddVertex;
}
/*
void C_UIClipPolygon::DebugRender( const Maths::RGBA c1, const Maths::HVec2 offset, const bool bGradient )
{
	Maths::RGBA c2 = c1;
	Maths::RGBA c3 = c1;
	c3.A( c3.A() - (U8)( 255.0f / ( m_Vertices.Size() + 1 ) ) );
	
	if ( m_sNumOriginalVertices < 2 )
		return;

	S32 sCount = 0;
	for( C_UIClipVertex* p = m_Vertices[ 0 ]; ( p->m_pNext != m_Vertices[ 0 ] ) && ( sCount < g_UiPolyTestRenderCount ); p = p->m_pNext )
	{
		Display::BasicRender()->DrawColoured2DLine( p->m_Pos * g_UiPolyScale + offset, p->m_pNext->m_Pos * g_UiPolyScale + offset, 
			bGradient ? c2 : c1, bGradient ? c3 : c1 );

		sCount++;

		if ( bGradient )
		{
			c2.A( c2.A() - (U8)( 255.0f / ( m_Vertices.Size() + 1 ) ) );		
			c3.A( c3.A() - (U8)( 255.0f / ( m_Vertices.Size() + 1 ) ) );		
		}
	}

	if ( sCount < g_UiPolyTestRenderCount )
	{
		// link first and last
		Display::BasicRender()->DrawColoured2DLine( m_Vertices[ 0 ]->m_pPrev->m_Pos * g_UiPolyScale + offset, m_Vertices[ 0 ]->m_Pos * g_UiPolyScale + offset, 
			bGradient ? c2 : c1, bGradient ? c3 : c1 );
	}
}
*/
C_UIClipVertex* C_UIClipPolygon::operator [] ( S32 sIndex )
{
	assert( sIndex >= 0 && sIndex < m_Vertices.Size() );
	return m_Vertices[ sIndex ];
}

C_UIClipVertex*	C_UIClipPolygon::GetVertex( S32 sIndex )
{
	assert( sIndex >= 0 && sIndex < m_Vertices.Size() );
	return m_Vertices[ sIndex ];
}


// ------------------------------------------------

bool LineIntersect( const Maths::HVec2& p1, const Maths::HVec2& p2, const Maths::HVec2& q1, const Maths::HVec2& q2, F32& a, F32& b, const bool bSegmentTest )
{
	Maths::HVec2 q2q1 = q2 - q1;
	Maths::HVec2 p2p1 = p2 - p1;

	F32 fDenominator = ( q2q1.Y() * p2p1.X() ) - ( q2q1.X() * p2p1.Y() );

	if ( Maths::Abs( fDenominator ) < 0.01f )	// Hmmmm ...
		return false;		// almost/are parallel

	// else they intersect

	Maths::HVec2 p1q1 = p1 - q1;

	F32 fNumeratorA = ( q2q1.X() * p1q1.Y() ) - ( q2q1.Y() * p1q1.X() );
	F32 fNumeratorB = ( p2p1.X() * p1q1.Y() ) - ( p2p1.Y() * p1q1.X() );

	a = fNumeratorA / fDenominator;
	b = fNumeratorB / fDenominator;

	return !bSegmentTest || !( a < 0.0f || a > 1.0f || b < 0.0f || b > 1.0f );
}

bool IsVertexInsidePoly( const Maths::HVec2& p1, C_UIResultPolygon& poly )
{
	// even odd rule
	// -- count intersections with ray extending from start vertex with the other polys edges
	// -- odd == inside else outside.

	Maths::HVec2 p2 = p1 + Maths::HVec2( 100000.0f, 999.0f );	// BODGE-NESS!!!

	F32 a, b;

	S32 sCount = 0;

	for ( U32 i = 0; i < poly.NumPoints(); i++ )
		if ( UI::LineIntersect( p1, p2, poly[ i ].GetPoint(), poly[ ( i + 1 ) % poly.NumPoints() ].GetPoint(), a, b ) )
			sCount++;

	if ( sCount & 1 )
		return true;

	return false;
}

bool IsVertexInsidePoly( const C_UIClipVertex* pVertex, C_UIClipPolygon& poly )
{
	// build a C_UIResultPolygon
	C_UIResultPolygon resultPoly;

	for ( C_UIClipVertex* p = poly[ 0 ]; p->m_pNext != poly[ 0 ]; p = p->m_pNext )
		resultPoly.AddPoint( p->m_Pos );

	// last point
	resultPoly.AddPoint( poly[ 0 ]->m_pPrev->m_Pos );

	return UI::IsVertexInsidePoly( pVertex->m_Pos, resultPoly );
}

};	// namespace UI
