#ifndef UIFONTCACHE_H
#define UIFONTCACHE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>

#include "ui/Graphics/Font/UIBitmapFont.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Simple2D.h"

namespace UI
{

class C_FontCacheAlternateTexture : public C_Simple2D::I_AlternateTexture, public Display::I_Renderer
{
public:
	C_FontCacheAlternateTexture( const S32 cell_size, const S32 num_cells );
	~C_FontCacheAlternateTexture();

public:	
	UITextureHandle					Handle()	{ return m_bInitialised ? &m_RenderTexture.GetTexture() : NULL; }
	const Util::C_String32&			Name()		{ return m_Name; }

public:
	bool							OnCreateVolatile();
	void							OnDestroyVolatile();

public:
	void							ResolveCell( const S32 index );
	void							ResolveAll();
	bool							Bind();
	void							Unbind();

private:
	S32								m_CellSize;
	S32								m_NumCells;
	Display::C_RenderTexture2D		m_RenderTexture;
	Util::C_String32				m_Name;
	bool							m_bInitialised;
};

class C_FontCache;
typedef Btl::List<C_FontCache*> FontCacheList;

class C_FontCache : public Btl::ListElementSingle< C_FontCache >
{
private:
	class C_FontCacheItem
	{
	public:
		C_FontCacheItem( const F32 height, const WCHAR glyph ) : 
			m_Height( height ),
			m_Glyph( glyph ),
			m_FrameUsed( 0 )
		{

		}

		C_FontCacheItem() : 
			m_FrameUsed( 0xffffffff )
		{

		}

		C_FontCacheItem( const C_FontCacheItem& fc )
		{
			*this = fc;
		}

	public:
		WCHAR					m_Glyph;
		U64						m_FrameUsed;
		F32						m_Height;
		C_BitmapGlyphAttribute	m_BitmapGlyph;
	};

public:
	C_FontCache( const S32 cell_size, const S32 num_cells, const char* pFontName );
	~C_FontCache();

	void				Render();
	I_GlyphAttributes*	DrawRequest( const F32 height, const WCHAR glyph );

public:
	static FontCacheList&	List()		{ return m_sList; }

private:
	static FontCacheList			m_sList;

	S32								m_CellSize;
	S32								m_NumCells;
	const C_UIPolygonFont*			m_pPolyFont;


	U64								m_CurrentFrame;
	Btl::Vector<C_FontCacheItem>	m_FontCache;
	Btl::Vector<C_FontCacheItem*>	m_FontRequestList;
	C_FontCacheAlternateTexture*	m_pCacheTexture;
	C_BitmapFontAttributes			m_BitmapFontAttr;
};

} // namespace UI

#endif
