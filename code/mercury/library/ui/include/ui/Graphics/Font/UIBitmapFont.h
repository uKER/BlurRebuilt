#ifndef UIBITMAPFONT_H
#define UIBITMAPFONT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Graphics/Font/Font.h"

namespace UI
{

class C_BitmapFontAttributes;

/// Kerning entry attributes
class C_BitmapKernAttribute
{
public:
	U16		m_First;
	U16		m_Second;
	F32		m_Amount;
};

/// Bitmap glyph
class C_BitmapGlyphAttribute : public I_GlyphAttributes
{
public:
	C_BitmapGlyphAttribute( WCHAR character, F32 offset, F32 width, F32 advance, F32 left, F32 right, F32 top, F32 bottom, C_BitmapFontAttributes* font = NULL ) :
		m_Character( character ),
		m_Offset( offset ),
		m_Width( width ),
		m_Advance( advance ),
		m_Left( left ),
		m_Top( top ),
		m_Right( right ),
		m_Bottom( bottom ),
		m_pFont( font )
	{
	}

	C_BitmapGlyphAttribute( ) : 
		m_Character( 0xffff )
	{
	}

	const E_GlyphType	GetType() const			{ return E_GlyphType_Bitmap; }
	const WCHAR			GetCharacter() const	{ return m_Character; }
	const F32			GetOffset() const		{ return m_Offset; }
	const F32			GetWidth() const		{ return m_Width; }
	const F32			GetAdvance() const		{ return m_Advance; }
					
	const F32			GetLeft() const			{ return m_Left; }
	const F32			GetRight() const		{ return m_Right; }
	const F32			GetTop() const			{ return m_Top; }
	const F32			GetBottom() const		{ return m_Bottom; }

	C_BitmapFontAttributes* GetFont() const		{ return m_pFont; }

private:
	WCHAR	m_Character;
	F32		m_Offset;
	F32		m_Width;
	F32		m_Advance;
    F32		m_Left;
	F32		m_Top;
	F32		m_Right;
	F32		m_Bottom;

	C_BitmapFontAttributes*	m_pFont;
};

/// Bitmap font attribtutes
class C_BitmapFontAttributes : public I_FontAttributes
{
public:
	C_BitmapFontAttributes( );
	~C_BitmapFontAttributes( );

	// Derived functions
	void							Read( Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar, const S32 dataVersion = 0 );
	const I_GlyphAttributes*		LookupGlyph( const WCHAR glyph, const bool bNullIfNotFound = false, const F32 drawHeight = 0.0f ) const;
	const F32						LookupKerning( const WCHAR first, const WCHAR second ) const;
	const F32						GetFontHeight( ) const			{ return (F32)m_FontHeight + m_VrtSpacing + m_VrtSpacing; }
	const F32						GetHrzOffset( bool left ) const	{ return (F32)m_HrzSpacing; }
	const F32						GetVrtOffset( bool top ) const	{ return (F32)m_VrtSpacing; }
	const U16						GetNumGlyphs( ) const			{ return (U16)m_NumGlyphs; }
	const Util::C_String256			GetName( ) const				{ return m_Name; }
	const I_GlyphAttributes*		GetDefaultGlyph( ) const		{ return (I_GlyphAttributes*)&m_Glyphs[0]; }
	const F32						GetSizeOveride( ) const			{ return m_SizeOveride; }
	const S32						GetVersion( ) const				{ return m_Version; }

private:
	void							GenerateHashMaps( );

public:
	Util::C_String256	m_Name;

	S32					m_Language;
	S32					m_TextureWidth;
	S32					m_TextureHeight;
	S32					m_NumGlyphs;
	S32					m_NumKerningPairs;
	S32					m_FontHeight;
	S32					m_VrtSpacing;
	S32					m_HrzSpacing;
			
	F32					m_TopOffset;
	F32					m_SizeOveride;

	C_TextureHandleIndex	m_Texture;

private :
	S32							m_Version;

	typedef Btl::OpenHashMap< U32, C_BitmapGlyphAttribute >		GlyphHashMap;
	typedef Btl::OpenHashMap< U32, F32 >						KerningHashMap;

	C_BitmapGlyphAttribute *	m_Glyphs;
	C_BitmapKernAttribute *		m_Kerning;
	GlyphHashMap				m_GlyphHash;
	KerningHashMap				m_KerningHash;
};

} // namespace UI

#endif
