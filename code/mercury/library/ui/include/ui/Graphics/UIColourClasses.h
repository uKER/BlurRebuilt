#ifndef UICOLOURCLASSES_H
#define UICOLOURCLASSES_H

#ifdef _MSC_VER
#pragma once
#endif

//#include <ui/Game/UILua.h>

extern "C"
{
	#include <lua.h>	
	#include <lauxlib.h>
	#include <lualib.h>
}

#include <core\containers\String.h>

struct XtRgba;

namespace UI
{

namespace UiXt { class C_ColourStyle; }
class C_AdobeColour;

/// @addtogroup Colours
/// @{

// ----------
// C_UIColour
// ----------
/// A8R8G8B8 colour support
class C_UIColour
{
public:
	/// default - white
	C_UIColour()
		: m_U32( 0xffffffff )
	{
		//
	}

	/// construct from a 32-bit A8R8G8B8 number
	C_UIColour( const U32 c )
		: m_U32( c )
	{
		//
	}

	/// construct using seperate parts
	C_UIColour( const U8 a, const U8 r, const U8 g, const U8 b )
		: m_A( a ), m_R( r ), m_G( g ), m_B( b )
	{
		//
	}

	/// construct using a LUA-style string - ARGB => "255 255 255 255"
	C_UIColour( const char* pLuaColour )
	{
		Set( pLuaColour );
	}

	/// construct using a adobe colour
	C_UIColour( const C_AdobeColour& col, const F32 alpha = 1.0f );

public:
	__forceinline			operator int() const						{ return m_U32; }	///< 32-bit compact conversion

	C_UIColour				operator *	( const C_UIColour& o ) const;		///< component-wise multiply
	C_UIColour&				operator *= ( const C_UIColour& o );			///< component-wise multiply
	U32						operator &	( const C_UIColour& o ) const;		///< component-wise max

	void					Lerp( const C_UIColour& src, const C_UIColour& dst, F32 f );	///< standard lerp - 0.0f <= f <= 1.0f

	const Util::C_String	LuaString() const;	/// LUA-style string ARGB => "255 255 255 255"
	void					GetFromLua( lua_State* pState, const char* pKey = "colour" );	///< get from LUA-style string

	void 					Set( const U8 a, const U8 r, const U8 g, const U8 b )	///< each component
	{
		m_A = a;
		m_R = r;
		m_G = g;
		m_B = b;
	}

	void					Set01( const F32 a, const F32 r, const F32 g, const F32 b )	///< each component 0-1
	{
		m_A = (U8)( Maths::Clamp<float>( a, 0.0f, 1.0f ) * 255.0f );
		m_R = (U8)( Maths::Clamp<float>( r, 0.0f, 1.0f ) * 255.0f );
		m_G = (U8)( Maths::Clamp<float>( g, 0.0f, 1.0f ) * 255.0f );
		m_B = (U8)( Maths::Clamp<float>( b, 0.0f, 1.0f ) * 255.0f );
	}

	void					SetIntensity( const F32 i ); ///< 0 to 1 multiplies 'i' more than 1 adds 'i-1'

	void 					Set( const U32 c );	///< set from ARGB number
	void 					Set( const char* pString );	///< set using LUA-style string

	void					A( const U8 a )					{ m_A = a; }						///< @a
	U8						A()	const						{ return m_A; }						///< @a
	void					A01( const F32 a )				{ m_A = (U8)( a * 255.0f ); }		///< @a
	F32						A01() const						{ return (F32)( m_A / 255.0f ); }	///< @a

	void					R( const U8 r )					{ m_R = r; }						///< @a
	U8						R() const						{ return m_R; }						///< @a
	void					R01( const F32 r )				{ m_R = (U8)( r * 255.0f ); }		///< @a
	F32						R01() const						{ return (F32)( m_R / 255.0f ); }	///< @a

	void					G( const U8 g )					{ m_G = g; }						///< @a
	U8						G() const						{ return m_G; }						///< @a
	void					G01( const F32 g )				{ m_G = (U8)( g * 255.0f ); }		///< @a
	F32						G01() const						{ return (F32)( m_G / 255.0f ); }	///< @a

	void					B( const U8 b )					{ m_B = b; }						///< @a
	U8						B() const						{ return m_B; }						///< @a
	void					B01( const F32 b )				{ m_B = (U8)( b * 255.0f ); }		///< @a
	F32						B01() const						{ return (F32)( m_B / 255.0f );	}	///< @a

	template < class T_Stream >
	void Serialise( T_Stream &stream ) const
	{
		stream	<< A()
				<< R()
				<< G()
				<< B();
	}

	template< class T_Stream >
	void Deserialise( T_Stream &stream )
	{
		U8 r, g, b, a;

		stream	>> a
				>> r
				>> g
				>> b;

		Set( a, r, g, b );
	}

private:
	union
	{
		struct 
		{
			U8 						m_A;
			U8 						m_R;
			U8 						m_G;
			U8 						m_B;
		};
		U32							m_U32;
	};
};

// -----------------
// C_UIColourARGB16
// -----------------

/// A1R5G5B5 colour support
class C_UIColourARGB16
{
public:
	C_UIColourARGB16( const C_UIColour& n )
	{
		U8 a =(n.A() > 128) ? 1 : 0;
		U8 r = n.R() >> 3;
		U8 g = n.G() >> 3;
		U8 b = n.B() >> 3;

		m_Number = (a << 15) | ( r << 10 ) | ( g << 5 ) | b;
	}

	C_UIColourARGB16( const U16 n ) : m_Number( n )
	{

	}

	// Access
	__forceinline			operator U16() const	{ return m_Number; }

	const C_UIColour		Colour() const			{ return C_UIColour( A(), R(), G(), B() ); }

	const U8				A() const				{ return ((m_Number>>15)&0x1)<<8; }
	const U8				R() const				{ return ((m_Number>>10)&0x1f)<<3; }
	const U8				G() const				{ return ((m_Number>> 5)&0x1f)<<3; }
	const U8				B() const				{ return ((m_Number>> 0)&0x1f)<<3; }

private:
	U16						m_Number;
};

// -----------------
// C_UISpriteColours
// -----------------
/// Colour support for the four corners of a quad - uses C_UIColour
class C_UISpriteColours
{
public:
	/// corners of the quad
	enum E_Corner
	{
		E_Corner_TopLeft = 0,	///< @a
		E_Corner_TopRight,		///< @a
		E_Corner_BottomRight,	///< @a
		E_Corner_BottomLeft,	///< @a

		E_Corner_Count,
	};

	/// total number of corners
	static const U32		m_SpriteColoursSize = 4;
	static const U8			m_MinAlphaValue = 0x00;

public:
	C_UISpriteColours();	///< default - zero
	C_UISpriteColours( const C_UIColour& col0, const C_UIColour& col1, const C_UIColour& col2, const C_UIColour& col3 );	///< constuct from four C_UIColour
	C_UISpriteColours( const S32 colAll );	///< ARGB number set to all corners
	C_UISpriteColours( const C_UIColour& colAll );	///< set to all corners using a C_UIColour
	C_UISpriteColours( const C_UIColour* pColArray );	///< set using a C_UIColour array - MUST have four elements

	/// Set to all corners using a XtRgba
	C_UISpriteColours( const XtRgba& colAll );

public:
							/// Set all using an ARGB number
	C_UISpriteColours&		operator = ( const S32 colAll );

							/// Set all using a C_UIColour
	C_UISpriteColours&		operator = ( const C_UIColour& colAll );
		
							/// Set all using a C_UIColour array - MUST have four elements 
	C_UISpriteColours&		operator = ( const C_UIColour* pColArray );

							/// Set to all corners using a XtRgba
	C_UISpriteColours&		operator = ( const XtRgba& colAll );

	C_UIColour&				operator [] ( const U32 index )
	{ 
		assert( index < m_SpriteColoursSize );
		return m_Colours[ index ]; 
	}

	const C_UIColour&		operator [] ( const U32 index ) const
	{
		assert( index < m_SpriteColoursSize );
		return m_Colours[ index ];
	}


	void					Lerp( const C_UISpriteColours& src, const C_UISpriteColours& dst, F32 f );	///< standard lerp using Lerp from C_UIColour for all elements
	void					SetAlpha( const U8 a );	///< set all corner's alpha value
	void					SetRed( const U8 r );	///< set all corner's red value
	void					SetGreen( const U8 g );	///< set all corner's green value
	void					SetBlue( const U8 b );	///< set all corner's blue value
	void					Set( const UiXt::C_ColourStyle& style, const F32 alpha = 1.0f, const F32 intensity = 1.0f ); ///< Set from Xt colour style type
	void					Set( const C_UISpriteColours& colours, const F32 alpha = 1.0f, const F32 intensity = 1.0f ); ///< Set from another colour
	const bool				SetByName( const char *style_name, const F32 alpha = 1.0f, const F32 intensity = 1.0f );	///< Do Xt style lookup
	void					SetIntensity( const F32 i ); ///< 0 to 1 multiplies 'i' more than 1 adds 'i-1'

	static void				SetStyle( const Util::C_String64& in )		{ m_sStylePrepend = in; }

							/// Returns true if any corner has an alpha over the min alpha value.
	const bool				HasAlpha() const
							{
								for ( S32 i = 0; i < m_SpriteColoursSize; i++ )
									if ( m_Colours[i].A() > m_MinAlphaValue )
										return true;

								return false;
							}
private:
	const bool				SetByNameImpl( const char *style_name, const F32 alpha, const F32 intensity, const bool warn );

private:
	static Util::C_String64	m_sStylePrepend;

private:
	C_UIColour				m_Colours[ m_SpriteColoursSize ];
};

/// @}

}; // namespace UI

#endif // UICOLOURCLASSES_H
