#include "stdafx.h"


#include <ui/Game/UILua.h>
#include <ui/Game/UiXt.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Illustrator/UIAdobeColour.h>

#include "ui/Graphics/UIColourClasses.h"


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

// ----------
// C_UIColour
// ----------
C_UIColour::C_UIColour( const C_AdobeColour& col, const F32 alpha )
{
	Set01( alpha, col.m_Red, col.m_Green, col.m_Blue );
}

C_UIColour C_UIColour::operator * ( const C_UIColour& o ) const
{
	return C_UIColour( ( m_A * o.A() ) >> 8, ( m_R * o.R() ) >> 8, ( m_G * o.G() ) >> 8, ( m_B * o.B() ) >> 8 );
}

C_UIColour& C_UIColour::operator *= ( const C_UIColour& o )
{
	*this = *this * o;
	return *this;
}

U32 C_UIColour::operator & ( const C_UIColour& o ) const
{
	C_UIColour temp( *this );
	if ( temp.A() > o.A() ) temp.A( o.A() );
	if ( temp.R() > o.R() ) temp.R( o.R() );
	if ( temp.G() > o.G() ) temp.G( o.G() );
	if ( temp.B() > o.B() ) temp.B( o.B() );
	return temp;
}

/*
bool C_UIColour::Read( Sys::BinaryDeserialiser< Ut::LitpStyle->m_TopLefteEndianToNative > archive, const S32 version )
{
	F32 r, g, b;

	archive >> r;
	archive >> g;
	archive >> b;

	Set01( 1.0f, r, g, b );

	return true;
}
*/

void C_UIColour::Lerp( const C_UIColour& src, const C_UIColour& dst, F32 f )
{
	f = Maths::Clamped( f, 0.0f, 1.0f );

	m_A = (U8)Maths::Lerp( src.A(), dst.A(), f );
	m_R = (U8)Maths::Lerp( src.R(), dst.R(), f );
	m_G = (U8)Maths::Lerp( src.G(), dst.G(), f );
	m_B = (U8)Maths::Lerp( src.B(), dst.B(), f );
}

const Util::C_String C_UIColour::LuaString() const
{
	Util::C_String string;
	string.Printf( "%d %d %d %d", m_A, m_R, m_G, m_B );
	return string;
}

void C_UIColour::SetIntensity( const F32 i )
{
	if( 1.0f == i )
		return;

	if( i <= 1.0f )
	{
		m_R = (U8)( i * (F32)m_R );
		m_G = (U8)( i * (F32)m_G );
		m_B = (U8)( i * (F32)m_B );
	}
	else
	{
		const F32 m = (i-1) * 255.0f ;

		m_R = (U8)Maths::Clamp< F32 >( (m + (F32)m_R), 0, 255.0f );
		m_G = (U8)Maths::Clamp< F32 >( (m + (F32)m_G), 0, 255.0f );
		m_B = (U8)Maths::Clamp< F32 >( (m + (F32)m_B), 0, 255.0f );
	}
}

void C_UIColour::Set( const U32 c )
{
	*this = C_UIColour( c );
}

void C_UIColour::Set( const char* pString )
{
	if ( !pString || ( strlen( pString ) == 0 ) )
	{
		*this = C_UIColour();
		return;
	}

	S32 a = 0, r = 0, g = 0, b = 0;
	sscanf( pString, "%d %d %d %d", &a, &r, &g, &b);

	const S32 min = 0;
	const S32 max = 255;
	Set( (U8)Maths::Clamp( a, min, max ), (U8)Maths::Clamp( r, min, max ), (U8)Maths::Clamp( g, min, max ), (U8)Maths::Clamp( b, min, max ) );
}

// -----------------
// C_UISpriteColours
// -----------------

// Preprend this when a style starting with '_' comes in
Util::C_String64	C_UISpriteColours::m_sStylePrepend;

C_UISpriteColours::C_UISpriteColours()
{
	//
}

C_UISpriteColours::C_UISpriteColours( const C_UIColour& col0, const C_UIColour& col1, const C_UIColour& col2, const C_UIColour& col3 )
{
	m_Colours[ 0 ]  = col0;
	m_Colours[ 1 ]  = col1;
	m_Colours[ 2 ]  = col2;
	m_Colours[ 3 ]  = col3;
}

C_UISpriteColours::C_UISpriteColours( const S32 colAll )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ] = colAll;
}

C_UISpriteColours::C_UISpriteColours( const C_UIColour& colAll )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ] = colAll;
}

C_UISpriteColours::C_UISpriteColours( const C_UIColour* pColArray )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ] = pColArray[ i ];
}

C_UISpriteColours::C_UISpriteColours( const XtRgba& colAll )
{
	for( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].Set01( colAll.a, colAll.r, colAll.g, colAll.b );
}

C_UISpriteColours& C_UISpriteColours::operator = ( const S32 colAll )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ] = colAll;

	return *this;
}

C_UISpriteColours& C_UISpriteColours::operator = ( const C_UIColour& colAll )
{
#if 0
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ] = colAll;
#else
		m_Colours[ 0 ] = colAll;
		m_Colours[ 1 ] = colAll;
		m_Colours[ 2 ] = colAll;
		m_Colours[ 3 ] = colAll;
#endif

	return *this;
}

C_UISpriteColours& C_UISpriteColours::operator = ( const C_UIColour* pColArray )
{
	for( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ] = pColArray[ i ];

	return *this;
}

C_UISpriteColours& C_UISpriteColours::operator = ( const XtRgba& colAll )
{
	for( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].Set01( colAll.a, colAll.r, colAll.g, colAll.b );

	return *this;
}

void C_UISpriteColours::Lerp( const C_UISpriteColours& src, const C_UISpriteColours& dst, F32 f )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].Lerp( src[ i ], dst[ i ], f );
}

void C_UISpriteColours::SetAlpha( const U8 a )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].A( a );
}

void C_UISpriteColours::SetRed( const U8 r )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].R( r );
}

void C_UISpriteColours::SetGreen( const U8 g )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].G( g );
}

void C_UISpriteColours::SetBlue( const U8 b )
{
	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].B( b );
}

void C_UISpriteColours::SetIntensity( const F32 fi )
{
	if( 1.0f == fi )
		return;

	for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		m_Colours[ i ].SetIntensity( fi );
}

void C_UISpriteColours::Set( const UiXt::C_ColourStyle& style, const F32 alpha, const F32 intensity )
{
#if 1
	BIZ_ASSERT(m_SpriteColoursSize == 4 && alpha >= 0.0f && alpha <= 1.0f && intensity >= 0.0f);

	U8 ialpha = (U8)( alpha * 255.0f );

	C_UIColour* __restrict dest = m_Colours;

	if ( intensity <= 1.0f )
	{
		float i = intensity * 255.0f;

		dest[0].Set(
			ialpha,
			(U8)( style.m_TopLeft.r * i ),
			(U8)( style.m_TopLeft.g * i ),
			(U8)( style.m_TopLeft.b * i ) );

		dest[1].Set(
			ialpha,
			(U8)( style.m_TopRight.r * i ),
			(U8)( style.m_TopRight.g * i ),
			(U8)( style.m_TopRight.b * i ) );

		dest[2].Set(
			ialpha,
			(U8)( style.m_BottomRight.r * i ),
			(U8)( style.m_BottomRight.g * i ),
			(U8)( style.m_BottomRight.b * i ) );

		dest[3].Set(
			ialpha,
			(U8)( style.m_BottomLeft.r * i ),
			(U8)( style.m_BottomLeft.g * i ),
			(U8)( style.m_BottomLeft.b * i ) );
	}
	else
	{
		float m = intensity - 1.0f;

		dest[0].Set(
			ialpha,
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_TopLeft.r + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_TopLeft.g + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_TopLeft.b + m ) ), 255 ) ) );

		dest[1].Set(
			ialpha,
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_TopRight.r + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_TopRight.g + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_TopRight.b + m ) ), 255 ) ) );

		dest[2].Set(
			ialpha,
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_BottomRight.r + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_BottomRight.g + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_BottomRight.b + m ) ), 255 ) ) );

		dest[3].Set(
			ialpha,
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_BottomLeft.r + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_BottomLeft.g + m ) ), 255 ) ),
			(U8)( Maths::Min<S32>( (S32)( 255.0f * ( style.m_BottomLeft.b + m ) ), 255 ) ) );
	}
#else
	const XtRgb* cols[ m_SpriteColoursSize ] = 
	{
		&style.m_TopLeft,
		&style.m_TopRight,
		&style.m_BottomRight,
		&style.m_BottomLeft,
	};

	C_UISpriteColours colours;

	for( U32 i = 0; i < m_SpriteColoursSize; i++ )
		colours[ i ].Set01( 1.0f, cols[ i ]->r, cols[ i ]->g, cols[ i ]->b );

	Set( colours, alpha, intensity );
#endif
}

void C_UISpriteColours::Set( const C_UISpriteColours& colours, const F32 falpha, const F32 fintensity )
{
#if 1
	int alpha = Maths::Clamp((int)(falpha * 256.0f), 0, 255);

	const C_UIColour* __restrict source = colours.m_Colours;
	C_UIColour* __restrict dest = m_Colours;

	if ( fintensity <= 1.0f )
	{
		int intensity = Maths::Clamp((int)(fintensity * 256.0f), 0, 255);

		U8 a0 = ((int)source[0].A() * alpha) / 255;
		U8 r0 = ((int)source[0].R() * intensity) / 255;
		U8 g0 = ((int)source[0].G() * intensity) / 255;
		U8 b0 = ((int)source[0].B() * intensity) / 255;

		U8 a1 = ((int)source[1].A() * alpha) / 255;
		U8 r1 = ((int)source[1].R() * intensity) / 255;
		U8 g1 = ((int)source[1].G() * intensity) / 255;
		U8 b1 = ((int)source[1].B() * intensity) / 255;

		U8 a2 = ((int)source[2].A() * alpha) / 255;
		U8 r2 = ((int)source[2].R() * intensity) / 255;
		U8 g2 = ((int)source[2].G() * intensity) / 255;
		U8 b2 = ((int)source[2].B() * intensity) / 255;

		U8 a3 = ((int)source[3].A() * alpha) / 255;
		U8 r3 = ((int)source[3].R() * intensity) / 255;
		U8 g3 = ((int)source[3].G() * intensity) / 255;
		U8 b3 = ((int)source[3].B() * intensity) / 255;

		dest[0].Set( a0, r0, g0, b0 );
		dest[1].Set( a1, r1, g1, b1 );
		dest[2].Set( a2, r2, g2, b2 );
		dest[3].Set( a3, r3, g3, b3 );
	}
	else
	{
		int m = Maths::Clamp((int)( (fintensity - 1.0f) * 256.0f ), 0, 255);

		U8 a0 = ((int)source[0].A() * alpha) / 255;
		U8 r0 = Maths::Min<S32>( (int)source[0].R() + m, 255 );
		U8 g0 = Maths::Min<S32>( (int)source[0].G() + m, 255 );
		U8 b0 = Maths::Min<S32>( (int)source[0].B() + m, 255 );

		U8 a1 = ((int)source[1].A() * alpha) / 255;
		U8 r1 = Maths::Min<S32>( (int)source[1].R() + m, 255 );
		U8 g1 = Maths::Min<S32>( (int)source[1].G() + m, 255 );
		U8 b1 = Maths::Min<S32>( (int)source[1].B() + m, 255 );

		U8 a2 = ((int)source[2].A() * alpha) / 255;
		U8 r2 = Maths::Min<S32>( (int)source[2].R() + m, 255 );
		U8 g2 = Maths::Min<S32>( (int)source[2].G() + m, 255 );
		U8 b2 = Maths::Min<S32>( (int)source[2].B() + m, 255 );

		U8 a3 = ((int)source[3].A() * alpha) / 255;
		U8 r3 = Maths::Min<S32>( (int)source[3].R() + m, 255 );
		U8 g3 = Maths::Min<S32>( (int)source[3].G() + m, 255 );
		U8 b3 = Maths::Min<S32>( (int)source[3].B() + m, 255 );

		dest[0].Set( a0, r0, g0, b0 );
		dest[1].Set( a1, r1, g1, b1 );
		dest[2].Set( a2, r2, g2, b2 );
		dest[3].Set( a3, r3, g3, b3 );
	}
#else
	if ( intensity <= 1.0f )
	{
		for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		{
			const F32 a = Maths::Clamp< F32 >( colours[ i ].A01() * alpha,	   0.0f, 1.0f );
			const F32 r = Maths::Clamp< F32 >( colours[ i ].R01() * intensity, 0.0f, 1.0f );
			const F32 g = Maths::Clamp< F32 >( colours[ i ].G01() * intensity, 0.0f, 1.0f );
			const F32 b = Maths::Clamp< F32 >( colours[ i ].B01() * intensity, 0.0f, 1.0f );

			m_Colours[ i ].Set01( a, r, g, b );
		}
	}
	else
	{
		const F32 m = ( intensity - 1 );

		for ( U32 i = 0; i < m_SpriteColoursSize; i++ )
		{
			const F32 a = Maths::Clamp< F32 >( colours[ i ].A01() * alpha,	0.0f, 1.0f );
			const F32 r = Maths::Clamp< F32 >( colours[ i ].R01() + m,		0.0f, 1.0f );
			const F32 g = Maths::Clamp< F32 >( colours[ i ].G01() + m, 		0.0f, 1.0f );
			const F32 b = Maths::Clamp< F32 >( colours[ i ].B01() + m, 		0.0f, 1.0f );

			m_Colours[ i ].Set01( a, r, g, b );
		}
	}
#endif
}

const bool C_UISpriteColours::SetByName( const char *style_name, const F32 alpha, const F32 intensity )
{
	// Check for preprended version?
	if( style_name && style_name[0] == '_' && m_sStylePrepend.Chars() )
	{
		Util::C_String128 tmp( m_sStylePrepend );
		tmp += style_name;

		if( SetByNameImpl( tmp, alpha, intensity, false ) )
			return true;

		// Attempt to find the default one
		return SetByNameImpl( style_name+1, alpha, intensity, true );
	}

	// Normal mode
	return SetByNameImpl( style_name, alpha, intensity, true );
}

const bool C_UISpriteColours::SetByNameImpl( const char *style_name, const F32 alpha, const F32 intensity, const bool warn )
{
	if( !style_name || !style_name[0] )
		return false;

	if(style_name[0] == '!')
	{
		char buffer[ 256 ];
		strcpy_s( buffer, lengthof( buffer ), style_name );

		const char *tok = ";";
		char *p = strtok( buffer + 1, tok );

		if( p )
		{
			// Base
			C_UIColour col( p );
			col.A( (U8) ((F32)col.A() * alpha) );
			col.SetIntensity( intensity );
			*this = col;

			// Set Corners?
			S32 i = 1;
			for(p = strtok( NULL, tok ); p && i < m_SpriteColoursSize; p = strtok( NULL, tok ), i++)
			{
				m_Colours[ i ] = p;
				m_Colours[ i ].A( (U8) ((F32)m_Colours[ i ].A() * alpha) );
			}
		}

		return true;
	}

	const UiXt::C_ColourStyle* pStyle = NULL;

#if 0
	// Check local colour styles
	const UiXt::C_Screen *pScreen = UiXt::C_ScreenList::Instance().FindByName( gUIScreenControl.GetCurrentScreen() );
	if( pScreen )
	{				
		pStyle = pScreen->FindColourStyleByName( style_name );
	}
#endif

	// Check global colour styles
	if( !pStyle )
	{
		pStyle = UiXt::C_ColourStyleList::Instance().FindByName( style_name );
	}
	
	// Not found
	if( !pStyle )
	{
		if( warn )
			LOGWARN( g_Log, DBSTR "Colour style %s not found\n", style_name );

		return false;
	}

	Set( *pStyle, alpha, intensity);

	return true;
}

}; // namespace UI
