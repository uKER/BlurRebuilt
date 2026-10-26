#include "stdafx.h"

#include "ui/Graphics/Font/FontCache.h"
#include "ui/Graphics/Font/UIPolygonFont.h"
#include "ui/Graphics/Polygons/UIHoleResolver.h"
#include "ui/Graphics/Polygons/UITriangulator.h"

#include "ui/Graphics/Simple2d.h"

#include "ui/UiInterface.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#ifndef CFG_GOLD
extern Debug::Int	g_UiRenderMaxIndexCount;
#endif

Debug::Int		g_UiPolyFontMaxCurveStep( "UI.PolygonFont.MaxCurveStep", 4, Debug::Range( 1, 5 ) );
Debug::Float	g_UiPolyFont3DExtrusionSize( "UI.PolygonFont.3DExtrusionSize", 2.0f, Debug::Range( 0.0f, 20.0f ) );
Debug::Float	g_UiPolyFont3DExtrusionMiddleOffset( "UI.PolygonFont.3DExtrusionMiddleOffset", 1.25f, Debug::Range( 0.0f, 20.0f ) );
Debug::Float	g_UiPolyFontOutlineWidth( "UI.PolygonFont.OutlineSize", 10.0f, Debug::Range( 0.0f, 100.0f ) );
Debug::Int		g_UiPolyFontCurveThreshold4( "UI.PolygonFont.CurveThreshold4", 80, Debug::Range( 1, 500 ) );
Debug::Int		g_UiPolyFontCurveThreshold3( "UI.PolygonFont.CurveThreshold3", 60, Debug::Range( 1, 500 ) );
Debug::Int		g_UiPolyFontCurveThreshold2( "UI.PolygonFont.CurveThreshold2", 40, Debug::Range( 1, 500 ) );
Debug::Int		g_UiPolyFontCurveThreshold1( "UI.PolygonFont.CurveThreshold1", 20, Debug::Range( 1, 500 ) );
Debug::Int		g_UiPolyFontDebugRenderHeight( "UI.PolygonFont.DebugRenderHeight", 185, Debug::Range( 0, 480 ) );
Debug::Float	g_UiPolyFontDebugStartX( "UI.PolygonFont.DebugRenderX", -40.0f, Debug::Range( -850.0f, 850.0f ) );
Debug::Float	g_UiPolyFontDebugStartY( "UI.PolygonFont.DebugRenderY", 20.0f, Debug::Range( -10000.0f, 20.0f ) );
Debug::Bool		g_UiPolyFontDebugRender( "UI.PolygonFont.Render", true );
Debug::Bool		g_UiPolyFontDebugRenderOutline( "UI.PolygonFont.RenderOutline", true );

const float kPolygonHeightHackFactor = 480.0f;

namespace UI
{

// kerning hash function
U32 GetKerningHash( const WCHAR first, const WCHAR second )
{
	return ( (U32)( first << 16 ) | (U32)( second ) );
}

#ifdef _DEBUG_POLYFONT_
C_UIPolygonFontTest gUIPolygonFont;
#endif

// global instance of the polygon font data store
C_UIPolygonFontDataStore C_UIPolygonFontDataStore::m_sInstance;

// -----------------
// C_UICharacterData
// -----------------
C_UICharacterData::C_UICharacterData()
	: m_Character( 0 ),
	m_GlyphMetrics()
{
	//
}

C_UICharacterData::~C_UICharacterData()
{
	Cleanup();
}

void C_UICharacterData::Cleanup()
{
	m_PolygonList.Clear();
}

void C_UICharacterData::AllocPolyCount( const U32 uPolyCount )
{
	m_PolygonList.Reserve( uPolyCount );

	for ( U32 i = 0; i < uPolyCount; i++ )
		m_PolygonList.PushBack( C_UIResultPolygon() );
}

void C_UICharacterData::AllocPolygonPointCount( const U32 uPolyIndex, const U32 uPointCount )
{
	m_PolygonList[ uPolyIndex ].GetPointList().Reserve( uPointCount );

	for ( U32 i = 0; i < uPointCount; i++ )
		m_PolygonList[ uPolyIndex ].AddPoint( C_UIResultPoint() );
}

void C_UICharacterData::SetPoint( const U32 uPolyIndex, const U32 uPointIndex, C_UIResultPoint& p )
{
	m_PolygonList[ uPolyIndex ].SetPoint( uPointIndex, p );
}


// -------------------
// C_UIPolygonFontData
// -------------------
C_UIPolygonFontData::C_UIPolygonFontData()
	: m_bInitialised( false ),
	m_uNumCharacters( 0 ),
	m_pCharacterData( NULL ),
	m_uNumKerningPairs( 0 ),
	m_pKerningPairs( NULL )
{
	m_pFontName[ 0 ] = '\0';
	m_pFilePath[ 0 ] = '\0';
}

C_UIPolygonFontData::~C_UIPolygonFontData()
{
	Cleanup();
}

void C_UIPolygonFontData::Cleanup()
{
	m_bInitialised = false;

	for ( U32 i = 0; i < m_uNumCharacters; i++ )
		m_pCharacterData[ i ].Cleanup();

	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH( "ui.font.poly.chars" );
	SAFE_DELETE_ARY( m_pCharacterData );
	//[NEEDS DEFINE]MEMORY_TRACKER_POP( "ui.font.poly.chars" );

	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH( "ui.font.poly.kern" );
	SAFE_DELETE_ARY( m_pKerningPairs );	
	//[NEEDS DEFINE]MEMORY_TRACKER_POP( "ui.font.poly.kern" );
	
	m_uNumCharacters = 0;
	m_uNumKerningPairs = 0;

	m_KerningHash.Clear();
}

void C_UIPolygonFontData::ReadFromFile( const char* pFilename )
{
	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH( "ui.font.poly" );

	STRCPY( m_pFilePath, m_uFilePathLength, pFilename );

	// open file
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open( pFilename );

	if ( !pFile )
	{
		//[NEEDS DEFINE]MEMORY_TRACKER_POP( "ui.font.poly" );
		return;
	}

	Sys::BinaryDeserialiser< Ut::LittleEndianToNative > archive( *pFile );

	// font name
	archive.Read( m_pFontName, sizeof( char ) * m_uFontNameLength );

	// number of characters
	archive >> m_uNumCharacters;

	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH( "ui.font.poly.chars" );
	m_pCharacterData = new C_UICharacterData[ m_uNumCharacters ];
	assert( m_pCharacterData );
	//[NEEDS DEFINE]MEMORY_TRACKER_POP( "ui.font.poly.chars" );

	// each character
	for ( U32 i = 0; i < m_uNumCharacters; i++ )
	{
		// the character
		U16 uChar;
		archive >> uChar;
		m_pCharacterData[ i ].Character( uChar );

		// glyph metrics
		F32 f1, f2, f3;
		archive >> f1;
		archive >> f2;
		archive >> f3;
		m_pCharacterData[ i ].GlyphMetrics( C_GlyphMetrics( f1 / kPolygonHeightHackFactor, f2 / kPolygonHeightHackFactor, f3 / kPolygonHeightHackFactor ) );

		// number of polygons
		U32 uNumPolys;
		archive >> uNumPolys;
		m_pCharacterData[ i ].AllocPolyCount( uNumPolys );

		// each poly
		for ( U32 j = 0; j < uNumPolys; j++ )
		{
			// number of points
			U32 uNumPoints;
			archive >> uNumPoints;
			m_pCharacterData[ i ].AllocPolygonPointCount( j, uNumPoints );

			// each point
			for ( U32 k = 0; k < uNumPoints; k++ )
			{
				F32 x, y;
				U8 curves;
				archive >> x;
				archive >> y;
				archive >> curves;
				C_UIResultPoint p( x, kPolygonHeightHackFactor - y, curves );
				m_pCharacterData[ i ].SetPoint( j, k, p );
			}
		}
	}

	// number of kerning pairs
	archive >> m_uNumKerningPairs;

	if ( m_uNumKerningPairs )
	{
		//[NEEDS DEFINE]MEMORY_TRACKER_PUSH( "ui.font.poly.kern" );
		m_pKerningPairs = new C_UIKerningPair[ m_uNumKerningPairs ];
		//[NEEDS DEFINE]MEMORY_TRACKER_POP( "ui.font.poly.kern" );
		assert( m_pKerningPairs );

		// each kerning pair
		for ( U32 i = 0; i < m_uNumKerningPairs; i++ )
		{
			U16 f, s;
			F32	kern;
			archive >> f;
			archive >> s;
			archive >> kern;

			C_UIKerningPair kp( f, s, kern / kPolygonHeightHackFactor );
			m_pKerningPairs[ i ] = kp;
		}
	}
	else
		m_pKerningPairs = NULL;

	SAFE_DELETE_PTR( pFile );
	//[NEEDS DEFINE]MEMORY_TRACKER_POP( "ui.font.poly" );

	// build the kerning hash map
	m_KerningHash.Clear();

	for ( U32 i = 0; i < m_uNumKerningPairs; i++ )
	{
		C_UIKerningPair& kerningPair = m_pKerningPairs[ i ];
		m_KerningHash[ GetKerningHash( kerningPair.First(), kerningPair.Second() ) ] = kerningPair.KernAmount();
	}

	m_bInitialised = true;
}

const F32 C_UIPolygonFontData::LookupKerning( const WCHAR first, const WCHAR second ) const
{
	KerningHashMap::ConstIterator itKerning = m_KerningHash.Find( GetKerningHash( first, second ) );

	return ( itKerning == m_KerningHash.End() ) ? 0.0f : itKerning->GetValue();
}

// ------------------------
// C_UIPolygonFontDataStore
// ------------------------
C_UIPolygonFontDataStore::C_UIPolygonFontDataStore()
{
	//
}

C_UIPolygonFontDataStore::~C_UIPolygonFontDataStore()
{
	m_FontData.Clear();
}

C_UIPolygonFontData* C_UIPolygonFontDataStore::GetFontData( const char* pFileName )
{
	// check that we haven't already build the font
	for ( S32 i = 0; i < m_FontData.Size(); i++ )
	{
		if ( !strcmp( pFileName, m_FontData[ i ].GetFilePath() ) )
		{
			// we've already built it - return a pointer to the data
			return &m_FontData[ i ];
		}
	}

	// else we need to build the font data
	m_FontData.PushBack( C_UIPolygonFontData() );
	C_UIPolygonFontData& fontData = m_FontData.Back();
	fontData.ReadFromFile( pFileName );

	return &m_FontData[ m_FontData.Size() - 1 ];
}

// -------------
// C_UICharacter
// -------------
C_UICharacter::C_UICharacter()
{
	//
}

C_UICharacter::~C_UICharacter()
{
	Cleanup();
}

void C_UICharacter::Cleanup()
{
	m_RenderInfo.Destroy();
}

void C_UICharacter::EvaluateSplines( UIPolygonList& evaluatedList )
{
	Btl::Vector< Maths::HVec2 > controlPoints;

	UIPolygonList& polygonList = m_pCharacterData->GetPolygonList();

	for ( S32 i = 0; i < polygonList.Size(); i++ )
	{
		// new polygon
		evaluatedList.PushBack( C_UIResultPolygon() );

		// each point
		for ( U32 j = 0; j < polygonList[ i ].NumPoints(); j++ )
		{
			U8 uNumCurvePoints = polygonList[ i ][ j ].NumCurvePoints();

			//evaluatedList[ i ].AddPoint( polygonList[ i ][ j ].GetPoint() );
			//continue;

			if ( uNumCurvePoints == 0 )
				evaluatedList[ i ].AddPoint( polygonList[ i ][ j ].GetPoint() );
			else
			{
				// build a list of control points
				controlPoints.Clear();

				// first is always the last point on the previous curve
				controlPoints.PushBack( ( j == 0 ) ? polygonList[ i ][ polygonList[ i ].NumPoints() - 1 ].GetPoint() : polygonList[ i ][ j - 1 ].GetPoint() );

				// subdivide the rest
				U32 uSubdivideCount = uNumCurvePoints - 2;	// as last two are both in
				U32 uCurrentCount = 0;

				while ( uCurrentCount < uSubdivideCount )
				{
					controlPoints.PushBack( polygonList[ i ][ j + uCurrentCount ].GetPoint() );
					controlPoints.PushBack( ( polygonList[ i ][ j + uCurrentCount ].GetPoint() + polygonList[ i ][ j + 1 + uCurrentCount ].GetPoint() ) / 2.0f );
					uCurrentCount++;
				}

				// last two always go in
				controlPoints.PushBack( polygonList[ i ][ j + uNumCurvePoints - 2 ].GetPoint() );
				controlPoints.PushBack( polygonList[ i ][ j + uNumCurvePoints - 1 ].GetPoint() );

				j += ( uNumCurvePoints - 1 );

				// now evaluate the list
				U32 uNumCurves = ( ( controlPoints.Size() - 3 ) / 2 ) + 1;

				for ( U32 k = 0; k < uNumCurves; k++ )
				{
					U32 sStart = k * 2;

					// quadratic bezier
					Maths::HVec2& p1 = controlPoints[ sStart ];
					Maths::HVec2& p2 = controlPoints[ sStart + 1 ];
					Maths::HVec2& p3 = controlPoints[ sStart + 2 ];

					S32 sCurveStep = g_UiPolyFontMaxCurveStep;

					if ( sCurveStep < 1 ) sCurveStep = 1;
					else if ( sCurveStep > 5 ) sCurveStep = 5;

					// adaptive depending on the length of the curve
					F32 fLength = ( p2 - p1 ).Length() + ( p3 - p2 ).Length();

					// curve reduction thresholds
					if ( fLength < g_UiPolyFontCurveThreshold1 )
						sCurveStep = Maths::Min<S32>( 1, sCurveStep );
					else if ( fLength < g_UiPolyFontCurveThreshold2 )
						sCurveStep = Maths::Min<S32>( 2, sCurveStep );
					else if ( fLength < g_UiPolyFontCurveThreshold3 )
						sCurveStep = Maths::Min<S32>( 3, sCurveStep );
					else if ( fLength < g_UiPolyFontCurveThreshold4 )
						sCurveStep = Maths::Min<S32>( 4, sCurveStep );

					F32 fStep = 1.0f / sCurveStep;

					for ( F32 t = fStep; t <= 1.0f; t += fStep )
					{
						F32 tm1 = 1.0f - t;
						Maths::HVec2 p = ( ( p1*tm1*tm1 ) + ( p2*2.0f*t*tm1 ) + ( p3*t*t ) );
						evaluatedList[ i ].AddPoint( p );
					}
				}
			}
		}
	}

	// some things screw up cos the TTF's sometimes have two or more points in the same place (for some reason) - so remove them!
	for ( S32 i = 0; i < evaluatedList.Size(); i++ )
	{
		ResultPointList& list = evaluatedList[ i ].GetPointList();

		for ( ResultPointList::Iterator it = list.Begin(); it != list.End(); )
		{
			Maths::HVec2 p1 = it->GetPoint();
			Maths::HVec2 p2 = ( ( it + 1 ) == list.End() ) ? list.Begin()->GetPoint() : ( it + 1 )->GetPoint();

			if ( ( p1.X() == p2.X() ) && ( p1.Y() == p2.Y() ) )
				it = list.Erase( it );
			else
				++it;
		}
	}
}

void C_UICharacter::FindAndResolveHoles( UIPolygonList& polygonList, UIPolygonList& resolvedList )
{
	UI::FindAndResolveHoles( polygonList, resolvedList );
}

Maths::HVec2 C_UICharacter::GetOutlinePoint( C_UIResultPolygon& poly, S32 sIndex )
{
	return poly.GetOutlinePoint( sIndex );
}

void C_UICharacter::BuildOutline( UIPolygonList& evaluatedList, UIPolygonList& outlineList, UIPolygonIndexList& indexList )
{
	for ( S32 i = 0; i < evaluatedList.Size(); i++ )
	{
		C_UIResultPolygon outline;

		for ( U32 j = 0; j < evaluatedList[ i ].NumPoints(); j++ )
		{
			Maths::HVec2 p2 = evaluatedList[ i ][ j ].GetPoint();
			Maths::HVec2 p = GetOutlinePoint( evaluatedList[ i ], j );

			// new outline uses a length ratio, then we control the size of the outline in the vertex shader
			Maths::HVec2 ratio = ( p - p2 );

			outline.AddPoint( p2, Maths::HVec2( 0.0f, 0.0f ) );
			outline.AddPoint( p2, Maths::HVec2( ratio.X(), ratio.Y() ) );
		}

		// done
		outlineList.PushBack( outline );

		// index list
		indexList.PushBack( C_ResultIndexList() );
		C_ResultIndexList& indices = indexList.Back();

		for ( U32 j = 0; j < outline.NumPoints(); j++ )
		{
			indices.AddIndex( j );
			indices.AddIndex( ( j + 1 ) % outline.NumPoints() );
			indices.AddIndex( ( j + 2 ) % outline.NumPoints() );
		}
	}
}

void C_UICharacter::Build3D( UIPolygonList& resolvedList, UIPolygonIndexList& indexList )
{
	S32 sResolvedListSize = resolvedList.Size();

	// front face vertex list
	for ( S32 j = 0; j < sResolvedListSize; j++ )
	{
		for ( U32 k = 0; k < resolvedList[ j ].NumPoints(); k++ )
			resolvedList[ j ][ k ].VZ( -0.5f );	// extrude
	}

	// back face vertex list
	for ( S32 j = 0; j < sResolvedListSize; j++ )
		resolvedList.PushBack( resolvedList[ j ] );

	for ( S32 j = sResolvedListSize; j < resolvedList.Size() ; j++ )
	{
		for ( U32 k = 0; k < resolvedList[ j ].NumPoints(); k++ )
			resolvedList[ j ][ k ].VZ( 0.5f );	// extrude
	}

	// back face index list
	S32 sIndexListSize = indexList.Size();

	for ( S32 j = 0; j < sIndexListSize; j++ )
		indexList.PushBack( indexList[ j ] );

	// bridge the two polygons
	S32 sIndexOffset = 0;

	for ( S32 j = 0; j < sResolvedListSize; j++ )
		sIndexOffset += resolvedList[ j ].NumPoints();

	S32 sIndexCount = 0;

	for ( S32 j = 0; j < sResolvedListSize; j++ )
	{
		C_ResultIndexList list;
		indexList.PushBack( list );
		C_ResultIndexList& indices = indexList.Back();

		S32 sStartIndex = sIndexCount;

		for ( U32 k = 0; k < resolvedList[ j ].NumPoints(); k++ )
		{
			// two triangles
			if ( k != ( resolvedList[ j ].NumPoints() - 1 ) )
			{
				indices.AddIndex( sIndexCount );
				indices.AddIndex( sIndexCount + sIndexOffset );
				indices.AddIndex( sIndexCount + sIndexOffset + 1 );

				indices.AddIndex( sIndexCount );
				indices.AddIndex( sIndexCount + 1 );
				indices.AddIndex( sIndexCount + sIndexOffset + 1 );
			}
			else
			{
				indices.AddIndex( sIndexCount );
				indices.AddIndex( sIndexCount + sIndexOffset );
				indices.AddIndex( sStartIndex + sIndexOffset );

				indices.AddIndex( sIndexCount );
				indices.AddIndex( sStartIndex );
				indices.AddIndex( sStartIndex + sIndexOffset );
			}

			sIndexCount++;
		}
	}
}

void C_UICharacter::BuildBevel3D( UIPolygonList& evaluatedList, UIPolygonList& resolvedList, UIPolygonIndexList& indexList )
{
	// calculate and store outline normals in the original list
	for ( S32 i = 0; i < evaluatedList.Size(); i++ )
	{
		for ( U32 j = 0; j < evaluatedList[ i ].NumPoints(); j++ )
		{
			Maths::HVec2 p2 = evaluatedList[ i ][ j ].GetPoint();
			Maths::HVec2 p = GetOutlinePoint( evaluatedList[ i ], j );

			// new outline uses a length ratio, then we control the size of the outline in the vertex shader
			Maths::HVec2 ratio = ( p - p2 );

			evaluatedList[ i ][ j ].VX( ratio.X() );
			evaluatedList[ i ][ j ].VY( ratio.Y() );
		}
	}

	// now find the corresponding vertex in the resolved and triangulated list
	for ( S32 i = 0; i < resolvedList.Size(); i++ )
	{
		for ( U32 j = 0; j < resolvedList[ i ].NumPoints(); j++ )
		{
			bool bFound = false;
			Maths::HVec2 v;

			for ( S32 x = 0; x < evaluatedList.Size(); x++ )
			{
				for ( U32 y = 0; y < evaluatedList[ x ].NumPoints(); y++ )
				{
					if ( resolvedList[ i ][ j ] == evaluatedList[ x ][ y ] )
					{
						// copy over the normal
						resolvedList[ i ][ j ].VX( evaluatedList[ x ][ y ].VX() );
						resolvedList[ i ][ j ].VY( evaluatedList[ x ][ y ].VY() );

						bFound = true;
						break;
					}
				}

				if ( bFound )
					break;
			}

			// and extrude
			resolvedList[ i ][ j ].VZ( -0.5f );

			assert( bFound );
		}
	}

	S32 sResolvedListSize = resolvedList.Size();

	// front face vertex list
	for ( S32 i = 0; i < sResolvedListSize; i++ )
	{
		resolvedList.PushBack( resolvedList[ i ] );

		C_UIResultPolygon& poly = resolvedList.Back();

		// zero the vector
		for ( U32 j = 0; j < poly.NumPoints(); j++ )
		{
			poly[ j ].VX( 0.0f );
			poly[ j ].VY( 0.0f );
			poly[ j ].VZ( -1.0f ); // converts to 3d + bevel extrusion in the vertex shader
		}
	}

	S32 sIndexListSize = indexList.Size();

	// front face index list
	for ( S32 i = 0; i < sIndexListSize; i++ )
		indexList.PushBack( indexList[ i ] );

	// back face vertex list
	for ( S32 i = 0; i < sResolvedListSize; i++ )
	{
		resolvedList.PushBack( resolvedList[ i ] );

		C_UIResultPolygon& poly = resolvedList.Back();

		for ( U32 j = 0; j < poly.NumPoints(); j++ )
			poly[ j ].VZ( 0.5f );	// extrude
	}

	// back face index list
	for ( S32 i = 0; i < sIndexListSize; i++ )
		indexList.PushBack( indexList[ i ] );

	// remove indices form the first set - the front non-beveled face
	for ( S32 i = 0; i < sIndexListSize; i++ )
		indexList[ i ] = C_ResultIndexList();

	S32 sIndexOffset = 0;

	// now bridge stuff - front face to beveled front face and front face to back face
	for ( S32 i = 0; i < 2; i++ )
	{
		for ( S32 j = 0; j < sResolvedListSize; j++ )
			sIndexOffset += resolvedList[ j ].NumPoints();		

		S32 sIndexCount = 0;

		for ( S32 j = 0; j < sResolvedListSize; j++ )
		{
			C_ResultIndexList list;
			indexList.PushBack( list );
			C_ResultIndexList& indices = indexList.Back();

			S32 sStartIndex = sIndexCount;

			for ( U32 k = 0; k < resolvedList[ j ].NumPoints(); k++ )
			{
				// two triangles
				if ( k != ( resolvedList[ j ].NumPoints() - 1 ) )
				{
					indices.AddIndex( sIndexCount );
					indices.AddIndex( sIndexCount + sIndexOffset );
					indices.AddIndex( sIndexCount + sIndexOffset + 1 );

					indices.AddIndex( sIndexCount );
					indices.AddIndex( sIndexCount + 1 );
					indices.AddIndex( sIndexCount + sIndexOffset + 1 );
				}
				else
				{
					indices.AddIndex( sIndexCount );
					indices.AddIndex( sIndexCount + sIndexOffset );
					indices.AddIndex( sStartIndex + sIndexOffset );

					indices.AddIndex( sIndexCount );
					indices.AddIndex( sStartIndex );
					indices.AddIndex( sStartIndex + sIndexOffset );
				}

				sIndexCount++;
			}
		}
	}
}

void C_UICharacter::BuildFinned( UIPolygonList& evaluatedList, UIPolygonList& resolvedList, UIPolygonIndexList& indexList )
{
	// the hole-resolver should make sure we're winded correctly

	// calculate and store inwards-normals
	for ( S32 i = 0; i < evaluatedList.Size(); i++ )
	{
		for ( U32 j = 0; j < evaluatedList[ i ].NumPoints(); j++ )
		{
			Maths::HVec2 p2 = evaluatedList[ i ][ j ].GetPoint();
			Maths::HVec2 p = evaluatedList[ i ].GetOutlinePoint( j );

			// new outline uses a length ratio, then we control the size of the outline in the vertex shader
			Maths::HVec2 ratio = ( p - p2 );

			evaluatedList[ i ][ j ].VX( -ratio.X() * 0.5f );
			evaluatedList[ i ][ j ].VY( -ratio.Y() * 0.5f );
			evaluatedList[ i ][ j ].VZ( 1.0f );
			evaluatedList[ i ][ j ].VW( 0.0f );
		}
	}
	
	// now find the corresponding vertex in the resolved and triangulated list
	for ( S32 i = 0; i < resolvedList.Size(); i++ )
	{
		for ( U32 j = 0; j < resolvedList[ i ].NumPoints(); j++ )
		{
			bool bFound = false;
			Maths::HVec2 v;

			for ( S32 x = 0; x < evaluatedList.Size(); x++ )
			{
				for ( U32 y = 0; y < evaluatedList[ x ].NumPoints(); y++ )
				{
					if ( resolvedList[ i ][ j ] == evaluatedList[ x ][ y ] )
					{
						// copy over the normal
						resolvedList[ i ][ j ].VX( evaluatedList[ x ][ y ].VX() );
						resolvedList[ i ][ j ].VY( evaluatedList[ x ][ y ].VY() );
						resolvedList[ i ][ j ].VZ( evaluatedList[ x ][ y ].VZ() );
						resolvedList[ i ][ j ].VW( evaluatedList[ x ][ y ].VW() );

						bFound = true;
						break;
					}
				}

				if ( bFound )
					break;
			}

			BIZ_ASSERT( bFound );
		}
	}

	// build the outlines
	for ( S32 i = 0; i < evaluatedList.Size(); i++ )
	{
		resolvedList.PushBack( UI::C_UIResultPolygon() );
		UI::C_UIResultPolygon& outline = resolvedList.Back();

		for ( U32 j = 0; j < evaluatedList[ i ].NumPoints(); j++ )
		{
			UI::C_UIResultPoint p1( evaluatedList[ i ][ j ].GetPoint(), -evaluatedList[ i ][ j ].GetVector4() );
			UI::C_UIResultPoint p2( evaluatedList[ i ][ j ].GetPoint(),  evaluatedList[ i ][ j ].GetVector4() );

			p1.VZ( 0.0f );
			p1.VW( 2.0f );

			p2.VZ( 1.0f );
			p2.VW( 0.0f );

			outline.AddPoint( p1 );
			outline.AddPoint( p2 );
		}

		indexList.PushBack( UI::C_ResultIndexList() );
		UI::C_ResultIndexList& outlineIndexList = indexList.Back();

		for ( U32 j = 0; j < outline.NumPoints(); j++ )
		{
			if ( j & 1 )
			{
				outlineIndexList.AddIndex( j );
				outlineIndexList.AddIndex( ( j + 1 ) % outline.NumPoints() );
				outlineIndexList.AddIndex( ( j + 2 ) % outline.NumPoints() );
			}
			else
			{
				outlineIndexList.AddIndex( j );
				outlineIndexList.AddIndex( ( j + 2 ) % outline.NumPoints() );
				outlineIndexList.AddIndex( ( j + 1 ) % outline.NumPoints() );
			}
		}
	}
}

void C_UICharacter::Triangulate( UIPolygonList& resolvedList, UIPolygonIndexList& indexList )
{
	UI::Triangulate( resolvedList, indexList );
}

void C_UICharacter::BuildVertexAndIndexBuffer( UIPolygonList& resolvedList, UIPolygonIndexList& indexList )
{
	if ( m_pCharacterData->Character() == L' ' )
		return;

	if ( resolvedList.Size() == 0 )
		return;

	// vertex buffer
	U32 uVertexCount = 0;
	U32 uIndexCountCorrect = 0;

	for ( S32 i = 0; i < resolvedList.Size(); i++ )
	{
		uVertexCount += resolvedList[ i ].NumPoints();
		uIndexCountCorrect += ( resolvedList[ i ].NumPoints() - 2 ) * 3;
	}

	if ( Display::Device()->CreateVertexBuffer( sizeof( C_UICharacterOutlineVertex3d ) * uVertexCount, 0, &m_RenderInfo.m_VertexBuffer ) )
	{
		C_UICharacterOutlineVertex3d* pVertex = (C_UICharacterOutlineVertex3d*)m_RenderInfo.m_VertexBuffer.Lock();

		for ( S32 i = 0; i < resolvedList.Size(); i++ )
		{
			for ( U32 j = 0; j < resolvedList[ i ].NumPoints(); j++ )
			{
				pVertex->Set( resolvedList[ i ][ j ].GetPoint() * Maths::HVec2(1.0f / kPolygonHeightHackFactor, 1.0f / kPolygonHeightHackFactor), resolvedList[ i ][ j ].GetVector4() );
				pVertex++;
			}
		}

		m_RenderInfo.m_VertexBuffer.Unlock();
	}

	m_RenderInfo.m_uVertexCount = uVertexCount;

	// index buffer
	U32 uIndexCount = 0;

	for ( S32 i = 0; i < indexList.Size(); i++ )
		uIndexCount += indexList[ i ].NumIndices();

	//assert( uIndexCount == uIndexCountCorrect );

	if ( uIndexCount == 0 )
		return;

	if ( Display::Device()->CreateIndexBuffer( sizeof( U16 ) * uIndexCount, 0, Display::E_Format_Index16, &m_RenderInfo.m_IndexBuffer ) )
	{
		U16* pIndex = (U16*)m_RenderInfo.m_IndexBuffer.Lock();

		U32 uStartOffset = 0;

		for ( S32 i = 0; i < resolvedList.Size(); i++ )
		{
			for ( S32 j = 0; j < indexList[ i ].NumIndices(); j++ )
				*pIndex++ = indexList[ i ][ j ] + uStartOffset;

			uStartOffset += resolvedList[ i ].NumPoints();
		}

		// add in any extra indices that we've pre-calculated

		for ( S32 i = resolvedList.Size(); i < indexList.Size(); i++ )
		{
			for ( S32 j = 0; j < indexList[ i ].NumIndices(); j++ )
				*pIndex++ = indexList[ i ][ j ];
		}

		m_RenderInfo.m_IndexBuffer.Unlock();
	}

	m_RenderInfo.m_uIndexCount = uIndexCount;

	
}

#ifndef CFG_GOLD
void C_UICharacter::DebugRender( const F32 x, const F32 y, const F32 fFontHeight, const C_UIColour& colour, const bool bOutline )
{
	INTPTR uUserData = ( m_pCharacterData->Character() == L' ' ) ? 0 : (INTPTR)&m_RenderInfo;	// [Blur2PC][x64]

	C_RenderState renderState = C_RenderState::Instance();

	static E_Effect s_Effect = E_Effect_PolygonFontFinned;
	renderState.m_EffectIndex = s_Effect;

	C_PolygonFontEffectParam effectParam;
	effectParam.SetOutlineWidth( g_UiPolyFontOutlineWidth );
	effectParam.SetExtrusion( g_UiPolyFont3DExtrusionSize * ( bOutline ? 1.0f : g_UiPolyFont3DExtrusionMiddleOffset ) );

	renderState.m_EffectParam = effectParam;

	F32 fScale = fFontHeight / POLYGON_FONT_HEIGHT;

	gSimple2D.AddSprite( 0, Maths::HVec3( x, y, 0.0f ), fScale, fScale, colour, 0, E_SpriteType_TriList, renderState, uUserData );
	//gSimple2D.AddSprite( 0, Maths::HVec3( x / 4.0f, y / 4.0f, 0.0f ), fScale / 4.0f, fScale / 4.0f, colour, 1, E_SpriteType_TriList, renderState, uUserData );	
}
#endif

// ---------------
// C_UIPolygonFont
// ---------------
C_UIPolygonFont::C_UIPolygonFont()
	: m_bInitialised( false ),
	m_pCharacters( NULL ),
	m_pTextureCache( NULL ),
	m_pDefaultGlyph( NULL )
{
	//
}

C_UIPolygonFont::~C_UIPolygonFont()
{
	Cleanup();
}

void C_UIPolygonFont::EnableTextureCache( )
{
	if( m_pTextureCache != NULL )
	{
		LOGERROR( g_Log, DBSTR " Texture cache already enabled for %s", m_pFontName );
		return;
	}

	m_pTextureCache = new C_FontCache( 64, 32, m_pFontName );
	if( m_pTextureCache )
		C_FontCache::List().PushBack( m_pTextureCache );
}

void C_UIPolygonFont::Cleanup()
{
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
		m_pCharacters[ i ].Cleanup();

	SAFE_DELETE_ARY( m_pCharacters );
	if( m_pTextureCache )
	{
		SAFE_DELETE_PTR( m_pTextureCache );
	}

	m_bInitialised = false;

	m_GlyphHash.Clear();
}

void C_UIPolygonFont::BuildFontData( const char* pFilename )
{
	// get the font data
	m_pFontData = C_UIPolygonFontDataStore::Instance().GetFontData( pFilename );
	assert( m_pFontData );
	if( m_pFontData->GetNumCharacters() == 0 )
		return;
	
	// create the character info
	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH( "ui.font.poly.chars" );
	m_pCharacters = new C_UICharacter[ GetNumGlyphs() ];
	assert( m_pCharacters );
	//[NEEDS DEFINE]MEMORY_TRACKER_POP( "ui.font.poly.chars" );

	// set the character data
	for ( U32 i = 0; i < GetNumGlyphs(); i++ )
		m_pCharacters[ i ].CharacterData( m_pFontData->GetCharacterData( i ) );

	GenerateHashMap();

	m_pDefaultGlyph = (C_UICharacter*)LookupGlyph( L'?', false );
	assert( m_pDefaultGlyph );
}

void C_UIPolygonFont::Build( const char* pFilename )
{
	RESOURCE_MEMORY_ID("UI");

	BuildFontData( pFilename );

	// copy over the fontname
	STRCPY( m_pFontName, m_uFontNameLength, m_pFontData->GetFontName() );

	// temp buffers to do the processing
	UIPolygonList evaluatedList;
	evaluatedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonList resolvedList;
	resolvedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonIndexList indexList;
	indexList.Reserve( MAX_NUM_POLYGONS );

	// process each character
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
	{
		evaluatedList.Clear();
		resolvedList.Clear();
		indexList.Clear();

		// the character
		m_pCharacters[ i ].EvaluateSplines( evaluatedList );
		m_pCharacters[ i ].FindAndResolveHoles( evaluatedList, resolvedList );

		m_pCharacters[ i ].Triangulate( resolvedList, indexList );
		m_pCharacters[ i ].BuildVertexAndIndexBuffer( resolvedList, indexList );
	}

	m_bInitialised = true;
}

void C_UIPolygonFont::GenerateHashMap( )
{
	// Glyph map
	m_GlyphHash.Clear();
	for (U32 glyphIndex = 0; glyphIndex < m_pFontData->GetNumCharacters(); ++glyphIndex)
	{
		C_UICharacter* glyph = &m_pCharacters[glyphIndex];
		m_GlyphHash[ glyph->GetCharacter() ] = glyph;
	}
}

INTPTR C_UIPolygonFont::GetUserData( const WCHAR c )
{
	if ( c == L' ' )
		return 0;	// space is never rendered - but will have spacing info in the glyphmetrics

	C_UICharacter* pCharacter = (C_UICharacter*)LookupGlyph( c, true );

	if ( pCharacter )
		return (INTPTR)&pCharacter->GetRenderInfo();

	return 0;
}

const I_GlyphAttributes*	C_UIPolygonFont::LookupGlyph( const WCHAR glyph, const bool bNullIfNotFound, const F32 drawHeight ) const
{
	if( m_pTextureCache && drawHeight > 0.0f )
	{
		I_GlyphAttributes*p = m_pTextureCache->DrawRequest( drawHeight, glyph );
		if( p )
			return p;
	}

	GlyphHashMap::ConstIterator itGlyph = m_GlyphHash.Find( glyph );

	if ( itGlyph == m_GlyphHash.End() )
		return ( bNullIfNotFound ) ? NULL : GetDefaultGlyph( );

	return itGlyph->GetValue();
}

const F32 C_UIPolygonFont::LookupKerning( const WCHAR first, const WCHAR second ) const 
{
	return m_pFontData->LookupKerning( first, second );
}

#ifndef CFG_GOLD
void C_UIPolygonFont::DebugRender( const C_UIColour& colour, const bool bOutline )
{
	Display::Device()->SetCullMode( Display::E_CullMode_None );

	F32 x = g_UiPolyFontDebugStartX;
	F32 y = g_UiPolyFontDebugStartY;
	F32 fFontHeight = (F32)g_UiPolyFontDebugRenderHeight;

	F32 fScale = (F32)g_UiPolyFontDebugRenderHeight / POLYGON_FONT_HEIGHT;
		
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
	{
		if ( x > ( g_UiPolyFontDebugStartX + 700.0f ) )
		{
			x = g_UiPolyFontDebugStartX;
			y += fFontHeight * 0.8f;
		}

		m_pCharacters[ i ].DebugRender( x, y, fFontHeight, colour, bOutline );
		x += ( m_pCharacters[ i ].GlyphMetrics().Width() + m_pCharacters[ i ].GlyphMetrics().Advance() ) * fScale;
	}

	Display::Device()->SetCullMode( Display::E_CullMode_Default );
}
#endif

// ----------------------
// C_UIPolygonFontOutline
// ----------------------
void C_UIPolygonFontOutline::Build( const char* pFilename )
{
	BuildFontData( pFilename );

	// copy over the fontname
	STRCPY( m_pFontName, m_uFontNameLength, m_pFontData->GetFontName() );
	STRCAT( m_pFontName, m_uFontNameLength, "_outline" );

	// temp buffers to do the processing
	UIPolygonList evaluatedList;
	evaluatedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonList outlineList;
	outlineList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonIndexList indexList;
	indexList.Reserve( MAX_NUM_POLYGONS );

	// process each character
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
	{
		evaluatedList.Clear();
		outlineList.Clear();
		indexList.Clear();

		// the character
		m_pCharacters[ i ].EvaluateSplines( evaluatedList );
		m_pCharacters[ i ].FindAndResolveHoles( evaluatedList, outlineList );

		// now that the holes are resolved, all outer polys will be CCW and holes CW.
		outlineList.Clear();

		// the outline bit
		m_pCharacters[ i ].BuildOutline( evaluatedList, outlineList, indexList );

		// done
		m_pCharacters[ i ].BuildVertexAndIndexBuffer( outlineList, indexList );
	}

	m_bInitialised = true;
}

// -----------------
// C_UIPolygonFont3D
// -----------------
void C_UIPolygonFont3D::Build( const char* pFilename )
{
	BuildFontData( pFilename );

	// copy over the fontname
	STRCPY( m_pFontName, m_uFontNameLength, m_pFontData->GetFontName() );
	STRCAT( m_pFontName, m_uFontNameLength, "_3d" );

	// temp buffers to do the processing
	UIPolygonList evaluatedList;
	evaluatedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonList resolvedList;
	resolvedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonIndexList indexList;
	indexList.Reserve( MAX_NUM_POLYGONS );

	// process each character
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
	{
		evaluatedList.Clear();
		resolvedList.Clear();
		indexList.Clear();

		// the character
		m_pCharacters[ i ].EvaluateSplines( evaluatedList );
		m_pCharacters[ i ].FindAndResolveHoles( evaluatedList, resolvedList );
		m_pCharacters[ i ].Triangulate( resolvedList, indexList );
		
		// the 3D bit
		m_pCharacters[ i ].Build3D( resolvedList, indexList );

		// done
		m_pCharacters[ i ].BuildVertexAndIndexBuffer( resolvedList, indexList );
	}

	m_bInitialised = true;
}

// ------------------------
// C_UIPolygonFontOutline3D
// ------------------------
void C_UIPolygonFontOutline3D::Build( const char* pFilename )
{
	BuildFontData( pFilename );

	// copy over the fontname
	STRCPY( m_pFontName, m_uFontNameLength, m_pFontData->GetFontName() );
	STRCAT( m_pFontName, m_uFontNameLength, "_outline_3d" );

	// temp buffers to do the processing
	UIPolygonList evaluatedList;
	evaluatedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonList outlineList;
	outlineList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonIndexList indexList;
	indexList.Reserve( MAX_NUM_POLYGONS );

	// process each character
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
	{
		evaluatedList.Clear();
		outlineList.Clear();
		indexList.Clear();

		// the character
		m_pCharacters[ i ].EvaluateSplines( evaluatedList );
		m_pCharacters[ i ].FindAndResolveHoles( evaluatedList, outlineList );

		// now that the holes are resolved, all outer polys will be CCW and holes CW.
		outlineList.Clear();

		// the outline bit
		m_pCharacters[ i ].BuildOutline( evaluatedList, outlineList, indexList );

		// the 3d bit
		m_pCharacters[ i ].Build3D( outlineList, indexList );

		// done
		m_pCharacters[ i ].BuildVertexAndIndexBuffer( outlineList, indexList );
	}

	m_bInitialised = true;
}

// ----------------------
// C_UIPolygonFontBevel3D
// ----------------------
void C_UIPolygonFontBevel3D::Build( const char* pFilename )
{
	BuildFontData( pFilename );

	// copy over the fontname
	STRCPY( m_pFontName, m_uFontNameLength, m_pFontData->GetFontName() );
	STRCAT( m_pFontName, m_uFontNameLength, "_bevel" );

	// temp buffers to do the processing
	UIPolygonList evaluatedList;
	evaluatedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonList resolvedList;
	resolvedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonIndexList indexList;
	indexList.Reserve( MAX_NUM_POLYGONS );

	// process each character
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
	{
		evaluatedList.Clear();
		resolvedList.Clear();
		indexList.Clear();

		// the character
		m_pCharacters[ i ].EvaluateSplines( evaluatedList );
		m_pCharacters[ i ].FindAndResolveHoles( evaluatedList, resolvedList );

		m_pCharacters[ i ].Triangulate( resolvedList, indexList );
		
		// the 3D bevel bit
		m_pCharacters[ i ].BuildBevel3D( evaluatedList, resolvedList, indexList );

		// done
		m_pCharacters[ i ].BuildVertexAndIndexBuffer( resolvedList, indexList );
	}

	m_bInitialised = true;
}

// ---------------------
// C_UIPolygonFontFinned
// ---------------------
void C_UIPolygonFontFinned::Build( const char* pFilename )
{
	BuildFontData( pFilename );

	// copy over the fontname
	STRCPY( m_pFontName, m_uFontNameLength, m_pFontData->GetFontName() );
	STRCAT( m_pFontName, m_uFontNameLength, "_finned" );

	// temp buffers to do the processing
	UIPolygonList evaluatedList;
	evaluatedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonList resolvedList;
	resolvedList.Reserve( MAX_NUM_POLYGONS );

	UIPolygonIndexList indexList;
	indexList.Reserve( MAX_NUM_POLYGONS );

	// process each character
	for ( U32 i = 0; i < m_pFontData->GetNumCharacters(); i++ )
	{
		evaluatedList.Clear();
		resolvedList.Clear();
		indexList.Clear();

		// the character
		m_pCharacters[ i ].EvaluateSplines( evaluatedList );
		m_pCharacters[ i ].FindAndResolveHoles( evaluatedList, resolvedList );

		m_pCharacters[ i ].Triangulate( resolvedList, indexList );

		// the finning bit
		m_pCharacters[ i ].BuildFinned( evaluatedList, resolvedList, indexList );

		// done
		m_pCharacters[ i ].BuildVertexAndIndexBuffer( resolvedList, indexList );
	}

	m_bInitialised = true;
}

#ifdef _DEBUG_POLYFONT_
// -------------------
// C_UIPolygonFontTest
// -------------------
C_UIPolygonFontTest::C_UIPolygonFontTest()
{
	for ( U32 i = 0; i < m_uNumFonts; i++ )
		m_Fonts[ i ] = NULL;

	m_uNumFonts = 0;
}

C_UIPolygonFontTest::~C_UIPolygonFontTest()
{
	//
}

void C_UIPolygonFontTest::Init()
{
	//
}

void C_UIPolygonFontTest::DestroyAll()
{
	for ( U32 i = 0; i < m_uNumFonts; i++ )
		SAFE_DELETE_PTR( m_Fonts[ i ] );

	m_uNumFonts = 0;
}

void C_UIPolygonFontTest::BuildFont( const char* pFilePath, const bool bOutlineFont )
{
	if ( m_uNumFonts < m_uMaxPolygonFonts )
	{
		if ( m_Fonts[ m_uNumFonts ] && m_Fonts[ m_uNumFonts ]->Initialised() )
		{
			assert( false && "We already have a font built here - !! check this - it should NEVER happen !! - but I'll free it anyway" );
			m_Fonts[ m_uNumFonts ]->Cleanup();
			m_uNumFonts--;
		}

		if ( bOutlineFont )
		{
			//m_Fonts[ m_uNumFonts ] = new C_UIPolygonFontOutline3D;
			//m_Fonts[ m_uNumFonts ] = new C_UIPolygonFontOutline;
		}
		else
		{
			//m_Fonts[ m_uNumFonts ] = new C_UIPolygonFontBevel3D;
			//m_Fonts[ m_uNumFonts ] = new C_UIPolygonFont3D;
			//m_Fonts[ m_uNumFonts ] = new C_UIPolygonFont;
			m_Fonts[ m_uNumFonts ] = new C_UIPolygonFontFinned;
		}

		if ( m_Fonts[ m_uNumFonts ] )
			m_Fonts[ m_uNumFonts++ ]->Build( pFilePath );
	}
}

const char*	C_UIPolygonFontTest::GetFontName( const U32 uSlot )
{
	if ( ( uSlot < m_uMaxPolygonFonts ) && m_Fonts[ uSlot ] && m_Fonts[ uSlot ]->Initialised() )
		return m_Fonts[ uSlot ]->GetFontName();

	return NULL;
}

INTPTR C_UIPolygonFontTest::GetUserData( const U32 uFont, const WCHAR c )
{
	if ( ( uFont < m_uMaxPolygonFonts ) && m_Fonts[ uFont ] && m_Fonts[ uFont ]->Initialised() )
		return m_Fonts[ uFont ]->GetUserData( c );

	return 0;	// will cast to a NULL pointer
}

#ifndef CFG_GOLD
void C_UIPolygonFontTest::DebugRender( const S32 sSlot )
{
	static C_UIColour colours[ m_uMaxPolygonFonts ] =
	{
		0xffafafaf,
		0xffff0000,
	};

	if ( sSlot == -1 )
	{
		if (  g_UiPolyFontDebugRenderOutline && m_Fonts[ 0 ] && m_Fonts[ 0 ]->Initialised() )
			m_Fonts[ 0 ]->DebugRender( colours[ 0 ], true );

		if (  g_UiPolyFontDebugRender && m_Fonts[ 1 ] && m_Fonts[ 1 ]->Initialised() )
			m_Fonts[ 1 ]->DebugRender( colours[ 1 ] );
	}
	else if ( ( sSlot < m_uMaxPolygonFonts ) && m_Fonts[ sSlot ] && m_Fonts[ sSlot ]->Initialised() )
		m_Fonts[ sSlot ]->DebugRender( colours[ sSlot ] );
}
#endif
#endif /* _DEBUG_POLYFONT_ */

// ------------------------
// C_PolygonFontEffectParam
// ------------------------
Maths::HVec4 C_PolygonFontEffectParam::GetShaderSettings()
{
	Maths::HVec4 v;

	v.X( POLYGON_FONT_HEIGHT );
	v.Y( m_Components.outline + ( m_Components.outlineF / 4.0f ) );
	v.Z( m_Components.extrusion + ( m_Components.extrusionF / 16.0f ) );
	v.W( m_Components.extrusionBevel + ( m_Components.extrusionBevelF / 16.0f ) );

	return v;
}

void C_PolygonFontEffectParam::SetOutlineWidth( const F32 fOutlineWidth )
{
	m_Components.outline = (U32)fOutlineWidth;
	m_Components.outlineF = (U32)( ( fOutlineWidth - (U32)fOutlineWidth ) * 4.0f );
}

void C_PolygonFontEffectParam::SetExtrusion( const F32 fExtrusion )
{
	m_Components.extrusion = (U32)fExtrusion;
	m_Components.extrusionF = (U32)( ( fExtrusion - (U32)fExtrusion ) * 16.0f );
}

void C_PolygonFontEffectParam::SetExtrusionBevel( const F32 fExtrusionBevel )
{
	m_Components.extrusionBevel = (U32)fExtrusionBevel;
	m_Components.extrusionBevelF = (U32)( ( fExtrusionBevel - (U32)fExtrusionBevel ) * 16.0f );
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
