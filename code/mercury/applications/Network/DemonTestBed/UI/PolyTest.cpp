#include "Stdafx.h"
#include "PolyTest.h"

#include <ui/UiInterface.h>


Debug::Bool		g_UiPolyTestReset( "UI.PolyTest.Reset", false );
//Debug::Int		g_UiPolyTestRenderCount( "UI.PolyTest.RenderCount", 200, Debug::Range( 0, 200 ) );
Debug::Int		g_UiCurveStep( "UI.PolyTest.CurveStep", 5, Debug::Range( 1, 20 ) );
Debug::String	g_UiFile( "UI.PolyTest.File", "mad.tx2" );
//Debug::Float	g_UiPolyScale( "UI.PolyTest.PolyScale", 0.45f );
Debug::Vector	g_UiOffsetOriginals( "UI.PolyTest.OffsetOriginals", 50.0f, 0.0f, 0.0f );
Debug::Vector	g_UiGapOriginals( "UI.PolyTest.GapOriginals", 250.0f, 0.0f, 0.0f );
Debug::Vector	g_UiOffsetResults( "UI.PolyTest.OffsetResults", 0.0f, 300.0f, 0.0f );
Debug::Vector	g_UiGapResults( "UI.PolyTest.GapResults", 250.0f, 0.0f, 0.0f );

namespace UI 
{

// ----------
// C_PolyTest
// ----------
C_PolyTest::C_PolyTest()
{
	//
}

void C_PolyTest::Init()
{

#if defined (USER_PREID)
	/*
	for ( S32 i = 0; i < m_Input.Size(); i++ )
		m_Result[ 0 ].PushBack( m_Input[ i ] );

	Triangulate( m_Result[ 0 ], m_ResultTriList[ 0 ] );
	*/

	// create the polys from a file
	bool bReadSuccess = false;

	Sys::I_File* pFile = UiInterface()->GetFileSys()->Open( Util::C_String( "GAME:\\User\\Ui\\Data\\" ) + g_UiFile );

	if ( pFile )
	{
		Sys::BinaryDeserialiser< Ut::LittleEndianToNative > archive( *pFile );

		S32 sNumShapes;
		archive >> sNumShapes;

		S32 sTotalPoints = 0;

		for ( S32 j = 0; j < sNumShapes; j++ )
		{
			// create new poly
			C_UIClipPolygon* pPoly = new C_UIClipPolygon;
			assert( pPoly );
			m_Input.PushBack( pPoly );

			S16 sNumPoints;
			archive >> sNumPoints;

			sTotalPoints += sNumPoints;

			S32 sTemp;
			archive >> sTemp; //m_Closed;
			archive >> sTemp; //m_FillPaint;

			F32 fTemp;
			archive >> fTemp; //r;
			archive >> fTemp; //g;
			archive >> fTemp; //b;

			archive >> sTemp; //m_StrokePaint;
			archive >> sTemp; //m_OddEven;

			if ( sNumPoints > 1 )
			{
				for( S32 k = 0; k < sNumPoints; k++ )
				{
					F32 x, y;

					archive >> x;
					archive >> y;
					Maths::HVec2 point( x, y );

					archive >> x;
					archive >> y;
					Maths::HVec2 in( x, y );
					
					archive >> x;
					archive >> y;
					Maths::HVec2 out( x, y );

					S32 sIsCorner;
					archive >> sIsCorner;

					m_Input[ m_Input.Size() - 1 ]->AddVertex( point );
				}
			}

			m_Input[ m_Input.Size() - 1 ]->CreateInitialNextAndBackLinks();
		}

		delete pFile;

		bReadSuccess = true;
	}

	assert( bReadSuccess );

	/*
	if ( !bReadSuccess )
	{
		// safe cleanup incase we've partially read in something
		for ( S32 i = 0; i < m_Input.Size(); i++ )
			SAFE_DELETE_PTR( m_Input[ i ] );

		m_Input.Destroy();
		
		// create the test polys
		C_Polygon* pPoly = new C_Polygon;
		assert( pPoly );
		m_Input.PushBack( pPoly );

		pPoly->AddVertex( 100, 100 );
		pPoly->AddVertex( 100, 300 );
		pPoly->AddVertex( 300, 300 );
		pPoly->AddVertex( 300, 100 );
		
		//pPoly->AddVertex( 320+90, 240-10 );
		//pPoly->AddVertex( 320+50, 240-50 );
		//pPoly->AddVertex( 320, 240+50 );
		//pPoly->AddVertex( 320-150, 240-150 );
		//pPoly->AddVertex( 320-50, 240+100 );
		pPoly->CreateInitialNextAndBackLinks();

		pPoly = new C_Polygon;
		assert( pPoly );
		m_Input.PushBack( pPoly );

		pPoly->AddVertex( 100, 500 );
		pPoly->AddVertex( 100, 700 );
		pPoly->AddVertex( 300, 700 );
		pPoly->AddVertex( 300, 500 );		
		

		pPoly->CreateInitialNextAndBackLinks();

		pPoly = new C_Polygon;
		assert( pPoly );
		m_Input.PushBack( pPoly );

		pPoly->AddVertex( 0, 800 );
		pPoly->AddVertex( 0, 0 );
		pPoly->AddVertex( 400, 0 );
		pPoly->AddVertex( 400, 800 );

		//pPoly->AddVertex( 320-100, 240-100 );
		//pPoly->AddVertex( 320+100, 240-100 );
		//pPoly->AddVertex( 320+100, 240 );
		//pPoly->AddVertex( 320-100, 240 );
		pPoly->CreateInitialNextAndBackLinks();
	}
	*/

	assert( m_Input.Size() > 1 );	// for now

	// evaluate curves

	/*
	F32 fStep = 1.0f / (F32)g_UiCurveStep;

	for ( S32 i = 0; i < m_Input.Size(); i++ )
	{
		Btl::Vector< C_PolyVertex* >& vertices = m_Input[ i ]->GetVertices();
		S32 sSize = vertices.Size();
		S32 sCount = 0;

		for ( Btl::Vector< C_PolyVertex* >::Iterator it = vertices.Begin(); it != vertices.End(); )
		{
			C_PolyVertex* pNextVertex = ( ( it + 1 ) == vertices.End() ) ? (*vertices.Begin()) : (*( it + 1 ));
			sCount++;

			if ( (*it)->m_sIsCorner )
				++it;
			else
			{
				// calculate curve point between it and it + 1
				Maths::HVec2& p1 = (*it)->m_Pos;
				Maths::HVec2& p2 = (*it)->m_Out;
				Maths::HVec2& p3 = ( ( it + 1 ) == vertices.End() ) ? (*vertices.Begin())->m_In : (*( it + 1 ))->m_In;
				Maths::HVec2& p4 = ( ( it + 1 ) == vertices.End() ) ? (*vertices.Begin())->m_Pos : (*( it + 1 ))->m_Pos;

				Btl::Vector< C_PolyVertex* >::Iterator itInsert = ( ( it + 1 ) == vertices.End() ) ? vertices.Begin() : ( it + 1 );

				C_PolyVertex* pVertex = NULL;

				for ( F32 a = ( 0.0f + fStep ); a < 1.0f; a += fStep )
				{
					F32 mu = a;
					F32 mum1 = 1 - mu;
					F32 mum13 = mum1 * mum1 * mum1;
					F32 mu3 = mu * mu * mu;
					
					Maths::HVec2 pos = (p1*mum13) + (p2*3.0f*mu*mum1*mum1) + (p3*3.0f*mu*mu*mum1) + (p4*mu3);

					pVertex = new C_PolyVertex( pos.X(), pos.Y() );
					assert( pVertex );
					vertices.InsertBefore( pVertex, itInsert );

					// recalculate the iterator
					for ( Btl::Vector< C_PolyVertex* >::Iterator it2 = vertices.Begin(); it2 != vertices.End(); ++it2 )
					{
						if ( *it2 == pVertex )
						{
							itInsert = it2 + 1;
							break;
						}
					}
				}

				// recalculate the iterator - AGAIN!
				for ( Btl::Vector< C_PolyVertex* >::Iterator it2 = vertices.Begin(); it2 != vertices.End(); ++it2 )
				{
					if ( *it2 == pNextVertex )
					{
						it = it2;
						break;
					}
				}
			}

			// nasty
			if ( sCount == sSize )
				break;
		}

		// link in the new vertices
		m_Input[ i ]->CreateInitialNextAndBackLinks();
	}
	*/

	// NOW DO STUFF

	for ( S32 i = 0; i < E_UIPolyClip_Count; i++ )
	{
		ResetInput();

		UI::ClipOperation( *m_Input[ 0 ], *m_Input[ 1 ], m_Result[ i ], (E_UIPolyClip)i );
	}

	// we now have the clipped poly(s)!

	/*
	// Triangulate
	for ( S32 i = 0; i < E_PolyClip_Count; i++ )
		Triangulate( m_Result[ i ], m_ResultTriList[ i ] );
	*/
#endif
}

C_PolyTest::~C_PolyTest()
{
	Cleanup();
}

void C_PolyTest::Cleanup()
{
	// cleanup input polys
	for ( S32 i = 0; i < m_Input.Size(); i++ )
		SAFE_DELETE_PTR( m_Input[ i ] );

	m_Input.Destroy();

	// cleanup result polys
	for ( S32 i = 0; i < E_UIPolyClip_Count; i++ )
	{
		for ( S32 j = 0; j < m_Result[ i ].Size(); j++ )
			SAFE_DELETE_PTR( m_Result[ i ][ j ] );

		m_Result[ i ].Destroy();
	}

#ifdef _ADOBE_TEST_
	m_RenderInfo.Destroy();
#endif
}

void C_PolyTest::Recalculate()
{
	Cleanup();
	Init();
}

void C_PolyTest::Update()
{
	if ( g_UiPolyTestReset )
	{
#ifdef PLATFORM_XENON
		Display::Device()->BlockUntilIdle();
#endif
		Recalculate();
		g_UiPolyTestReset = false;
	}
}

void C_PolyTest::Render()
{
	Maths::RGBA c1( 255, 0, 0 );
	Maths::RGBA c2( 0, 0, 255 );
	Maths::RGBA c3( 0, 255, 0 );
	Maths::RGBA c4( 255, 255, 255 );
	Maths::RGBA c5( 255, 255, 0 );
	Maths::RGBA c6( 0, 255, 0 );
	Maths::RGBA c7( 255, 0, 0 );

	Maths::HVec2 offset( g_UiOffsetOriginals.Value().vx, g_UiOffsetOriginals.Value().vy );

	for ( S32 i = 0; i < m_Input.Size(); i++ )
		m_Input[ i ]->DebugRender( ( i & 1 ) ? c2 : c1, offset );

	offset += Maths::HVec2( g_UiGapOriginals.Value().vx, g_UiGapOriginals.Value().vy );

	//for ( S32 i = 0; i < m_Input.Size(); i++ )
		//m_Input[ i ]->RenderIntersections( ( i & 1 ) ? c2 : c1, c3, offset );

	offset += Maths::HVec2( g_UiGapOriginals.Value().vx, g_UiGapOriginals.Value().vy );

	//for ( S32 i = 0; i < m_Input.Size(); i++ )
		//m_Input[ i ]->RenderCorners( ( i & 1 ) ? c2 : c1, c4, offset );

	offset = Maths::HVec2( g_UiOffsetResults.Value().vx, g_UiOffsetResults.Value().vy );

	for ( S32 i = 0; i < E_UIPolyClip_Count; i++ )
	{
		// draw each of the result polys
		for ( S32 j = 0; j < m_Result[ i ].Size(); j++ )
			m_Result[ i ][ j ]->DebugRender( c5, offset, true );

		offset += Maths::HVec2( g_UiGapResults.Value().vx, g_UiGapResults.Value().vy );
	}

	/*
	offset = Maths::HVec2( g_UiOffsetResults.Value().vx, g_UiOffsetResults.Value().vy + ( g_UiOffsetResults.Value().vy - g_UiOffsetOriginals.Value().vy ) );

	for ( S32 i = 0; i < E_PolyClip_Count; i++ )
	{
		// draw the triangulated results
		for ( S32 j = 0; j < m_ResultTriList[ i ].Size(); j++ )
		{
			// each tri list
			assert( ( m_ResultTriList[ i ][ j ].m_Vertices.Size() % 3 ) == 0 );

			for ( S32 k = 0; k < m_ResultTriList[ i ][ j ].m_Vertices.Size(); k += 3 )
			{
				S32 i1 = m_ResultTriList[ i ][ j ].m_Vertices[ k ];
				S32 i2 = m_ResultTriList[ i ][ j ].m_Vertices[ k + 1 ];
				S32 i3 = m_ResultTriList[ i ][ j ].m_Vertices[ k + 2 ];

				C_Polygon& poly = (*m_Result[ i ][ j ]);

				Maths::RGBA c = m_ResultTriList[ i ][ j ].m_bOk ? c6 : c7;
				// i1 => i2
				Display::BasicRender()->DrawColoured2DLine( poly[ i1 ]->m_Pos * g_UiPolyScale + offset, poly[ i2 ]->m_Pos * g_UiPolyScale + offset, c );
				// i2 => i3
				Display::BasicRender()->DrawColoured2DLine( poly[ i2 ]->m_Pos * g_UiPolyScale + offset, poly[ i3 ]->m_Pos * g_UiPolyScale + offset, c );
				// i3 => i
				Display::BasicRender()->DrawColoured2DLine( poly[ i3 ]->m_Pos * g_UiPolyScale + offset, poly[ i1 ]->m_Pos * g_UiPolyScale + offset, c );
			}
		}

		offset += Maths::HVec2( g_UiGapResults.Value().vx, g_UiGapResults.Value().vy );
	}
	*/
}

void C_PolyTest::ResetInput()
{
	for ( S32 i = 0; i < m_Input.Size(); i++ )
		m_Input[ i ]->Reset();
}

};	// namespace UI
