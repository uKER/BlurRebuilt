#include "stdafx.h"

#include "ui/Graphics/Font/UIBitmapFont.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

// Bitmap fonts
C_BitmapFontAttributes::C_BitmapFontAttributes( ) :
	m_Version( 0 ),
	m_Glyphs( NULL ),
	m_Kerning( NULL )
{

}

C_BitmapFontAttributes::~C_BitmapFontAttributes( )
{
	m_GlyphHash.Clear();
	m_KerningHash.Clear();

	SAFE_DELETE_ARY( m_Glyphs );
	SAFE_DELETE_ARY( m_Kerning );
}

void C_BitmapFontAttributes::Read( Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar, const S32 dataVersion )
{
	char texture_name[128];

	// Font name
	ar.Read(texture_name, lengthof(texture_name));
	m_Name = texture_name;
	m_Texture = gUITextureManager.GetTextureByName(texture_name);
	
	// Texture name
	ar.Read(texture_name, lengthof(texture_name));

	ar >> m_Language;
	ar >> m_TextureWidth;
	ar >> m_TextureHeight;
	ar >> m_NumGlyphs;
	ar >> m_FontHeight;

	m_Version = dataVersion;

	if( dataVersion > 1 )
	{
		ar >> m_HrzSpacing;
		ar >> m_VrtSpacing;
	}
	else
	{
		// Deprecated 
		F32 extrude, spacing_size;

		ar >> extrude;
		ar >> spacing_size;

		m_HrzSpacing = 0;
		m_VrtSpacing = 0;
	}

	ar >> m_TopOffset;
	ar >> m_SizeOveride;

	// Load the glyphs
	if (m_NumGlyphs)
	{
		if( dataVersion > 2 )
		{
			struct GLYPH_ATTRIBUTE
			{
				WCHAR character;
				U16 left, top, right, bottom; // Texture coordinates for the image
				F32 wOffset;					// Pixel offset for glyph start
				F32 wWidth;					// Pixel width of the glyph
				F32 wAdvance;					// Pixels to advance after the glyph
			};

			m_Glyphs = new C_BitmapGlyphAttribute[ m_NumGlyphs ];
			GLYPH_ATTRIBUTE* temp_array = new GLYPH_ATTRIBUTE[ m_NumGlyphs ];

			assert(m_Glyphs);
			ar.Read(temp_array,sizeof(struct GLYPH_ATTRIBUTE) * m_NumGlyphs);
			Ut::LittleEndianToNative::EndianSwap(temp_array, "WWWWWxxFFF", m_NumGlyphs);

			for(S32 i = 0; i < m_NumGlyphs; i++)
			{
				m_Glyphs[ i ] = C_BitmapGlyphAttribute( 
					temp_array[ i ].character,
					temp_array[ i ].wOffset,
					temp_array[ i ].wWidth,
					temp_array[ i ].wAdvance,
					(F32)temp_array[ i ].left / (F32)m_TextureWidth,
					(F32)temp_array[ i ].right / (F32)m_TextureWidth,
					(F32)temp_array[ i ].top / (F32)m_TextureHeight,
					(F32)temp_array[ i ].bottom / (F32)m_TextureHeight );
			}

			delete [] temp_array;
		}
		else
		{
			struct OLD_GLYPH_ATTRIBUTE
			{
				WCHAR character;
				F32 left, top, right, bottom;	// Texture coordinates for the image
				S16 wOffset;					// Pixel offset for glyph start
				S16 wWidth;						// Pixel width of the glyph
				S16 wAdvance;					// Pixels to advance after the glyph
			};

			m_Glyphs = new C_BitmapGlyphAttribute[ m_NumGlyphs ];
			OLD_GLYPH_ATTRIBUTE* temp_array = new OLD_GLYPH_ATTRIBUTE[ m_NumGlyphs ];

			assert(m_Glyphs);
			ar.Read(temp_array,sizeof(struct OLD_GLYPH_ATTRIBUTE) * m_NumGlyphs);
			Ut::LittleEndianToNative::EndianSwap(temp_array, "WxxFFFFSSSxx", m_NumGlyphs);

			for(S32 i = 0; i < m_NumGlyphs; i++)
			{
				m_Glyphs[ i ] = C_BitmapGlyphAttribute( 
					temp_array[ i ].character,
					temp_array[ i ].wOffset,
					(F32)(temp_array[ i ].wWidth - (m_HrzSpacing * 2)),
					(F32)(temp_array[ i ].wAdvance - temp_array[ i ].wWidth),
					temp_array[ i ].left,
					temp_array[ i ].right,
					temp_array[ i ].top,
					temp_array[ i ].bottom );
			}

			delete [] temp_array;		

			m_HrzSpacing = 0;
			m_VrtSpacing = 0;
		}
	}
	else
	{
		m_Glyphs = NULL;
	}

	// Load the kerning pairs
	ar >> m_NumKerningPairs;
	if(m_NumKerningPairs)
	{
		m_Kerning = new C_BitmapKernAttribute[m_NumKerningPairs];
		if( dataVersion > 2 )
		{
			ar.Read(m_Kerning,sizeof(C_BitmapKernAttribute) * m_NumKerningPairs);
			Ut::LittleEndianToNative::EndianSwap(m_Kerning, "SSF", m_NumKerningPairs);
		}
		else
		{
			struct OLD_KERNING
			{
				U16		m_First;
				U16		m_Second;
				char	m_Amount;
			};
			// Read in old file data type
			OLD_KERNING* old_kern = new OLD_KERNING[ m_NumKerningPairs ];
			ar.Read(old_kern,sizeof(OLD_KERNING) * m_NumKerningPairs);
			Ut::LittleEndianToNative::EndianSwap(old_kern, "SSxx", m_NumKerningPairs);
			// Convert
			for( S32 i = 0; i < m_NumKerningPairs; i++)
			{
				m_Kerning[ i ].m_First = old_kern[ i ].m_First;
				m_Kerning[ i ].m_Second = old_kern[ i ].m_Second;
				m_Kerning[ i ].m_Amount = old_kern[ i ].m_Amount;
			}
			
			delete old_kern;
		}
	}

	//[NEEDS DEFINE]MEMORY_TRACKER_PUSH("ui.font.hashmap");
	GenerateHashMaps( );
	//[NEEDS DEFINE]MEMORY_TRACKER_POP("ui.font.hashmap");
}

const I_GlyphAttributes *C_BitmapFontAttributes::LookupGlyph( const WCHAR glyph, const bool bNullIfNotFound, const F32 drawHeight ) const
{
	GlyphHashMap::ConstIterator itGlyph = m_GlyphHash.Find( glyph );

	if ( itGlyph == m_GlyphHash.End() )
	{
		if(bNullIfNotFound)
			return NULL;
		else if(m_Language > 0)	//not English
			return NULL;
		else
			return &m_Glyphs[0];
	}

	return &itGlyph->GetValue();
}

void C_BitmapFontAttributes::GenerateHashMaps( )
{
	// Glyph map
	m_GlyphHash.Clear();
	for (S32 glyphIndex = 0; glyphIndex < m_NumGlyphs; ++glyphIndex)
	{
		const C_BitmapGlyphAttribute& glyph = m_Glyphs[glyphIndex];
		m_GlyphHash[ glyph.GetCharacter() ] = glyph;
	}

	// Kerning pairs map
	m_KerningHash.Clear();
	for (S32 pairIndex = 0; pairIndex < m_NumKerningPairs; ++pairIndex)
	{
		const C_BitmapKernAttribute& kerning = m_Kerning[ pairIndex ];

		U32 hash_key = ((U32)(kerning.m_First) << 16) | (U32)(kerning.m_Second);
		m_KerningHash[hash_key] = (F32)kerning.m_Amount;
	}
}

const F32 C_BitmapFontAttributes::LookupKerning(const WCHAR first, const WCHAR second) const
{
	U32 hash_key = ((U32)(first) << 16) | (U32)(second);

	KerningHashMap::ConstIterator itKerning = m_KerningHash.Find( hash_key );

	return ( itKerning == m_KerningHash.End() ) ? 0.0f : itKerning->GetValue();
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
