// PolygonFont.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "PolygonFont.h"

// ***
int main(int argc, char* argv[])
{
	// read in glyphs from EuroGlyphs.ini
	S32 pGlyphCount[ MAX_GLYPH_COUNT ];
	memset( pGlyphCount, 0, sizeof( S32 ) * MAX_GLYPH_COUNT );

	const char* pGlyphFileName = argv[ 1 ];

	FILE* pGlyphFile;
	S32 sError = fopen_s( &pGlyphFile, pGlyphFileName, "rb" );

	if ( sError )
	{
		printf( "*** FAILED TO READ FILE %s***\n", pGlyphFileName );
		return 0;
	}

	// skip "IDS_GLYPHS = "
	S32 sNumToSkip = strlen( "IDS_GLYPHS = " );

	while ( sNumToSkip-- >= 0 )
		fgetwc( pGlyphFile );

	// read the characters - counting dupes
	for ( WCHAR c = fgetwc( pGlyphFile ); c != MAX_GLYPH_COUNT; c = fgetwc( pGlyphFile ) )
		pGlyphCount[ c ]++;

	fclose( pGlyphFile );

	// count how many we've read
	S32 sGlyphCount = 0;

	for ( S32 i = 0; i < MAX_GLYPH_COUNT; i++ )
	{
		if ( pGlyphCount[ i ] > 0 )
			sGlyphCount++;
	}

	// create the font
	HDC hDC = CreateCompatibleDC( NULL );

	char* pFontName = argv[ 2 ];

	WCHAR pFont[ 256 ];
	swprintf_s( pFont, 256, L"%S", pFontName );
	wprintf_s( L"FONT - %s\n", pFont );

	//S32 sHeight = 0;
	//S32 sHeight = -FONT_HEIGHT;
	//S32 sHeight = FONT_HEIGHT * 72 / 90;
	S32 sHeight = -MulDiv( FONT_HEIGHT, GetDeviceCaps( hDC, LOGPIXELSY ), 72 );

	HFONT hFont = CreateFont( sHeight, 0, 0, 0, 
							  FW_NORMAL, FALSE, FALSE, FALSE, 
							  DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, 
							  PROOF_QUALITY, VARIABLE_PITCH, 
							  pFont );

	assert( hFont != NULL );
	SelectObject( hDC, hFont );

	SIZE size;
	GetTextExtentPoint32( hDC, L"A", 1, &size );
	//sHeight = size.cy;

	TEXTMETRIC tm;
	GetTextMetrics( hDC, &tm );

	// glyph transform matrix
	MAT2 matrix;
	memset( &matrix, 0, sizeof( MAT2 ) );
	matrix.eM11.value = matrix.eM22.value = 1;	// identity

	// glyph format
	U32 uFormat = GGO_NATIVE;
	//U32 uFormat = GGO_NATIVE | GGO_BEZIER;

	// list of characters
	Container< C_Letter > characterSet;

	// per glyph - we make a copy
	GLYPHMETRICS glyphMetrics;
	ABCFLOAT abc;

	S32 sMinY = 0;
	S32 sMaxY = 0;

	// process the characters
	for ( S32 i = 0; i < MAX_GLYPH_COUNT; i++ )
	{
		if ( !pGlyphCount[ i ] )
			continue;

		// check that this character exists in this font
		DWORD bufferSize = GetGlyphOutline( hDC, i, uFormat, &glyphMetrics, 0, NULL, &matrix );

		if ( bufferSize == GDI_ERROR )
		{
			printf( "ERROR - processing glyph 0x%x\n", i );
			continue;
		}
			
		// alloc buffer needed for the character info
		U8* pBuffer = new U8[ bufferSize ];
		assert( pBuffer );

		// now get the info
		DWORD size = GetGlyphOutline( hDC, i, uFormat, &glyphMetrics, bufferSize, pBuffer, &matrix );
		assert( size == bufferSize );

		// range in y
		if ( (S32)glyphMetrics.gmptGlyphOrigin.y < sMinY )
			sMinY = glyphMetrics.gmptGlyphOrigin.y;
		if ( (S32)( glyphMetrics.gmptGlyphOrigin.y + glyphMetrics.gmBlackBoxY ) > sMaxY )
			sMaxY = glyphMetrics.gmptGlyphOrigin.y + glyphMetrics.gmBlackBoxY;

		// the letter
		//printf( "NEW LETTER - 0x%x\n", i );
		characterSet.push_back( C_Letter() );

		C_Letter& letter = characterSet.back();
		letter.m_Character = (WCHAR)i;

		// get abc widths
		if ( GetCharABCWidthsFloatW( hDC, i, i, &abc ) == 0 )
			abc.abcfA = abc.abcfC = abc.abcfB = 0.0f;

		letter.m_GlyphMetrics = C_GlyphMetrics( abc );

		// process the character info
		U8* pHeader = pBuffer;
	
		while ( (U8*)pHeader < (U8*)( pBuffer + bufferSize ) )
		{
			// new polygon
			//printf( "\nNew Polygon\n" );
			Container< C_LetterPoint > p;
			letter.m_PolyList.push_back( p );

			Container< C_LetterPoint >& poly = letter.m_PolyList.back();

			TTPOLYGONHEADER* pPoly = (TTPOLYGONHEADER*)pHeader;

			if ( pPoly->dwType == TT_POLYGON_TYPE )
			{
				// first point
				poly.push_back( C_LetterPoint( pPoly->pfxStart ) );
				//printf( "\t\tx:%d,%d\ty:%d,%d\n", pPoly->pfxStart.x.fract, pPoly->pfxStart.x.value, pPoly->pfxStart.y.fract, pPoly->pfxStart.y.value );

				// move passed the header
				pHeader += sizeof( TTPOLYGONHEADER );

				// the curves
				while ( pHeader < (U8*)( (U8*)pPoly + pPoly->cb ) )
				{
					TTPOLYCURVE* pCurve = (TTPOLYCURVE*)pHeader;

					bool bCurve = false;

					if ( pCurve->wType == TT_PRIM_LINE )
					{
						//printf( "\tTT_PRIM_LINE %d\n", pCurve->cpfx );
					}
					else if ( pCurve->wType == TT_PRIM_QSPLINE )
					{
						bCurve = true;
						//printf( "\tTT_PRIM_QSPLINE %d\n", pCurve->cpfx );
					}
					else if ( pCurve->wType == TT_PRIM_CSPLINE )
					{
						bCurve = true;
						//printf( "\tTT_PRIM_CSPLINE %d\n", pCurve->cpfx );
					}

					C_LetterPoint point;
					C_LetterPoint* pFirstPoint = NULL;

					S32 sStartPoint = 0;

					if ( bCurve )
					{
						point = C_LetterPoint( pCurve->apfx[ 0 ] );
						point.m_uNumCurvePoints = 1;	// new curve
						poly.push_back( point );
						pFirstPoint = &poly.back();
						sStartPoint = 1;
					}

					// read point info
					for ( S32 i = sStartPoint; i < pCurve->cpfx; i++ )
					{
						poly.push_back( C_LetterPoint( pCurve->apfx[ i ] ) );
						//printf( "\t\tx:%d,%d\ty:%d,%d\n", pCurve->apfx[ i ].x.fract, pCurve->apfx[ i ].x.value, pCurve->apfx[ i ].y.fract, pCurve->apfx[ i ].y.value );

						if ( pFirstPoint )
							pFirstPoint->m_uNumCurvePoints++;
					}

					// next curve
					pHeader += ( ( sizeof( WORD ) * 2 ) + ( sizeof( POINTFX ) * pCurve->cpfx ) );
				}
			}
		}

		delete [] pBuffer;
	}
	
	// read in kerning pairs for the font
	KERNINGPAIR* pKerningPairs = NULL;
	DWORD numKerningPairs = GetKerningPairs( hDC, 0, NULL );

	Container< C_KerningPair > kerningPairs;

	if ( numKerningPairs != 0 )
	{
		pKerningPairs = new KERNINGPAIR[ numKerningPairs ];

		// now get the info
		DWORD num = GetKerningPairs( hDC, numKerningPairs, pKerningPairs );
		assert( num == numKerningPairs );

		for ( DWORD i = 0; i < numKerningPairs; i++ )
			kerningPairs.push_back( C_KerningPair( pKerningPairs[ i ] ) );

		delete [] pKerningPairs;
	}

	// finished with the font
	DeleteObject( hFont );
    DeleteDC( hDC );

	// ***

	//printf( "sMinY - %d\n", sMinY );
	//printf( "sMaxY - %d\n", sMaxY );

	//F32 fScale = FONT_HEIGHT / (F32)( sMaxY - sMinY );

	// range in y
	//S32 sShiftY = -sMinY; //0;//( FONT_HEIGHT - ( sMaxY - sMinY ) ) / 2;

	// calculate max font bounds
	F32 fMinX = 1000.0f;
	F32 fMaxX = -1000.0f;
	F32 fMinY = 1000.0f;
	F32 fMaxY = -1000.0f;

	for ( Container< C_Letter >::iterator it0 = characterSet.begin(); it0 != characterSet.end(); ++it0 )
	{
		F32 fMinXl = 1000.0f;
		F32 fMaxXl = -1000.0f;
		F32 fMinYl = 1000.0f;
		F32 fMaxYl = -1000.0f;

		for ( Container< Container< C_LetterPoint > >::iterator it1 = it0->m_PolyList.begin(); it1 != it0->m_PolyList.end(); ++it1 )
		{
			for ( Container< C_LetterPoint >::iterator it2 = it1->begin(); it2 != it1->end(); ++it2 )
			{
				if ( it2->m_PointX < fMinX )
					fMinX = it2->m_PointX;
				if ( it2->m_PointX > fMaxX )
					fMaxX = it2->m_PointX;

				if ( it2->m_PointY < fMinY )
					fMinY = it2->m_PointY;
				if ( it2->m_PointY > fMaxY )
					fMaxY = it2->m_PointY;

				// letter local
				if ( it2->m_PointX < fMinXl )
					fMinXl = it2->m_PointX;
				if ( it2->m_PointX > fMaxXl )
					fMaxXl = it2->m_PointX;

				if ( it2->m_PointY < fMinYl )
					fMinYl = it2->m_PointY;
				if ( it2->m_PointY > fMaxYl )
					fMaxYl = it2->m_PointY;
			}
		}

		it0->m_fOriginX = fMinXl + ( fMaxXl - fMinXl ) * 0.5f;
		it0->m_fOriginY = fMinYl + ( fMaxYl - fMinYl ) * 0.5f;
		
		// now shift each letter to start at 0 in x
		for ( Container< Container< C_LetterPoint > >::iterator it1 = it0->m_PolyList.begin(); it1 != it0->m_PolyList.end(); ++it1 )
		{
			for ( Container< C_LetterPoint >::iterator it2 = it1->begin(); it2 != it1->end(); ++it2 )
			{
				it2->m_PointX -= fMinXl;
				assert( it2->m_PointX >= 0.0f );
			}
		}
	}

	//printf( "fMinX - %.2f\n", fMinX );
	//printf( "fMaxX - %.2f\n", fMaxX );
	//printf( "fMinY - %.2f\n", fMinY );
	//printf( "fMaxY - %.2f\n", fMaxY );

	F32 fScale = FONT_HEIGHT / ( fMaxY - fMinY );

	F32 fScaleExtra = ( fMaxY - fMinY ) / (F32)size.cy;
	F32 fEffectiveScale = fScale * fScaleExtra;

	F32 fShiftY = ( -fMinY * fScale ) + ( -fMinY * fScale * ( 1.0f - fScaleExtra ) );

	// each character
	for ( Container< C_Letter >::iterator it0 = characterSet.begin(); it0 != characterSet.end(); ++it0 )
	{
		it0->m_GlyphMetrics.m_fOffset *= fEffectiveScale;
		it0->m_GlyphMetrics.m_fWidth *= fEffectiveScale;
		it0->m_GlyphMetrics.m_fAdvance *= fEffectiveScale;

		// now change them

		for ( Container< Container< C_LetterPoint > >::iterator it1 = it0->m_PolyList.begin(); it1 != it0->m_PolyList.end(); ++it1 )
		{
			for ( Container< C_LetterPoint >::iterator it2 = it1->begin(); it2 != it1->end(); ++it2 )
			{
				// pivot at centre
				//it2->m_PointX -= it0->m_fOriginX;
				//it2->m_PointY -= it0->m_fOriginY;

				// scale
				it2->m_PointX *= fEffectiveScale;
				it2->m_PointY *= fEffectiveScale;
				
				// offset back
				//it2->m_PointX += it0->m_fOriginX;
				//it2->m_PointY += it0->m_fOriginY;

				// shift
				it2->m_PointY += fShiftY;
			}
		}
	}

	// kerning pairs
	for ( Container< C_KerningPair >::iterator it = kerningPairs.begin(); it != kerningPairs.end(); ++it )
		it->m_fKernAmount *= fEffectiveScale;

	// create the output file
	char* pOutputFolder = argv[ 3 ];

	if ( pOutputFolder[ strlen( pOutputFolder ) - 1 ] == '\"' )
		pOutputFolder[ strlen( pOutputFolder ) - 1 ] = '\0';

	// replace spaces with underscore in filename
	for ( U32 i = 0; i < wcslen( pFont ); i++ )
	{
		if ( pFont[ i ] == L' ' )
			pFont[ i ] = L'_';
	}
		
	char pFilename[ 256 ];
	sprintf_s( pFilename, 256, "%s\\%S.pf", pOutputFolder, pFont );

	FILE* pFile;
	sError = fopen_s( &pFile, pFilename, "wb" );

	if ( sError )
		printf( "*** FAILED TO OPEN FILE %s***\n", pFilename );
	else
	{
		// replace spaces with underscore in font name
		for ( U32 i = 0; i < strlen( pFontName ); i++ )
		{
			if ( pFontName[ i ] == ' ' )
				pFontName[ i ] = '_';
		}

		// font name
		fwrite( pFontName, sizeof( char ), 256, pFile );

		// number of characters
		U32 uNumCharacters = (U32)characterSet.size();
		fwrite( &uNumCharacters, sizeof( U32 ), 1, pFile );
		
		// each character
		for ( Container< C_Letter >::iterator it0 = characterSet.begin(); it0 != characterSet.end(); ++it0 )
		{
			// the character
			fwrite( &it0->m_Character, sizeof( WCHAR ), 1, pFile );

			// glyph metrics
			fwrite( &it0->m_GlyphMetrics.m_fOffset, sizeof( F32 ), 1, pFile );
			fwrite( &it0->m_GlyphMetrics.m_fWidth, sizeof( F32 ), 1, pFile );
			fwrite( &it0->m_GlyphMetrics.m_fAdvance, sizeof( F32 ), 1, pFile );

			//fwrite( &it0->m_GlyphMetrics, sizeof( C_GlyphMetrics ), 1, pFile );

			// number of polygons
			U32 uNumPolys = (U32)it0->m_PolyList.size();
			fwrite( &uNumPolys, sizeof( U32 ), 1, pFile );

			// each poly
			for ( Container< Container< C_LetterPoint > >::iterator it1 = it0->m_PolyList.begin(); it1 != it0->m_PolyList.end(); ++it1 )
			{
				// number of points
				U32 uNumPoints = (U32)it1->size();
				fwrite( &uNumPoints, sizeof( uNumPoints ), 1, pFile );

				// each point
				for ( Container< C_LetterPoint >::iterator it2 = it1->begin(); it2 != it1->end(); ++it2 )
				{
					fwrite( &it2->m_PointX, sizeof( F32 ), 1, pFile );
					fwrite( &it2->m_PointY, sizeof( F32 ), 1, pFile );
					fwrite( &it2->m_uNumCurvePoints, sizeof( U8 ), 1, pFile );

					//fwrite( &(*it2), sizeof( C_LetterPoint ), 1, pFile );
				}
			}
		}

		// number of kerning pairs
		S32 uNumKerningPairs = (U32)kerningPairs.size();
		fwrite( &uNumKerningPairs, sizeof( U32 ), 1, pFile );

		// each kerning pair
		for ( Container< C_KerningPair >::iterator it = kerningPairs.begin(); it != kerningPairs.end(); ++it )
		{
			fwrite( &it->m_uFirst, sizeof( WCHAR ), 1, pFile );
			fwrite( &it->m_uSecond, sizeof( WCHAR ), 1, pFile );
			fwrite( &it->m_fKernAmount, sizeof( F32 ), 1, pFile );

			//fwrite( &(*it), sizeof( C_KerningPair ), 1, pFile );
		}

		fclose( pFile );

		printf( "\n*** EXPORT COMPLETE ***\n\n" );
	}

	system( "pause" );
	return 0;
}
