#ifndef POLYGONFONT_H
#define POLYGONFONT_H

#include <list>

#define MAX_GLYPH_COUNT		65535
#define FONT_HEIGHT			480

#define Container			std::list

typedef struct C_GlyphMetrics
{
	C_GlyphMetrics()
		: m_fOffset( 0 ),
		m_fWidth( 0 ),
		m_fAdvance( 0 )
	{
		//
	}

	C_GlyphMetrics( ABCFLOAT& abc )
	{
		m_fOffset = abc.abcfA;
		m_fWidth = abc.abcfB;
		m_fAdvance = abc.abcfC;
	}

	F32		m_fOffset;
	F32		m_fWidth;
	F32		m_fAdvance;
} C_GlyphMetrics;

typedef struct C_KerningPair
{
	C_KerningPair()
	{
		//
	}

	C_KerningPair( KERNINGPAIR& kp )
	{
		m_uFirst = kp.wFirst;
		m_uSecond = kp.wSecond;
		m_fKernAmount = (F32)kp.iKernAmount;
	}

	WCHAR		m_uFirst;
	WCHAR		m_uSecond;
	F32			m_fKernAmount;
} C_KerningPair;

typedef struct C_LetterPoint
{
	C_LetterPoint()
	{
		m_PointX = 0.0f;
		m_PointY = 0.0f;
		m_uNumCurvePoints = 0;
	}

	C_LetterPoint( POINTFX& point )
	{
		// convert from fixed to float
		m_PointX = ( point.x.value + ( point.x.fract / 65536.0f ) );
		m_PointY = ( point.y.value + ( point.y.fract / 65536.0f ) );
		m_uNumCurvePoints = 0;
	}

	F32				m_PointX;
	F32				m_PointY;
	U8				m_uNumCurvePoints;
} C_LetterPoint;

typedef struct C_Letter
{
	C_Letter()
	{
		//
	}

	F32 m_fOriginX;
	F32 m_fOriginY;
	WCHAR										m_Character;
	C_GlyphMetrics								m_GlyphMetrics;
	Container< Container< C_LetterPoint > >		m_PolyList;
} C_Letter;

#endif /* POLYGONFONT_H */
