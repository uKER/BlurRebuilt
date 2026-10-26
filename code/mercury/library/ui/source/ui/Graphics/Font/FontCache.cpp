#include "stdafx.h"

#include "ui/Graphics/Font/FontCache.h"
#include "ui/Graphics/Font/UIPolygonFont.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// 128 * 64 * 64 = 512k
namespace UI
{

// Alternate texture
C_FontCacheAlternateTexture::C_FontCacheAlternateTexture( const S32 cell_size, const S32 num_cells )
	: I_Renderer( "UiFontCache" )
	, m_bInitialised( false )
	, m_CellSize( cell_size )
	, m_NumCells( num_cells )
	, m_Name( "font_cache" )
{
	//
}

C_FontCacheAlternateTexture::~C_FontCacheAlternateTexture()
{
	//
}

bool C_FontCacheAlternateTexture::OnCreateVolatile()
{
	return false;
}

void C_FontCacheAlternateTexture::OnDestroyVolatile()
{
}

bool C_FontCacheAlternateTexture::Bind()
{
	if( !m_bInitialised )
		return false;


	return true;
}

void C_FontCacheAlternateTexture::ResolveCell( const S32 index )
{
}

void C_FontCacheAlternateTexture::Unbind()
{
	// Restore
	Display::Device()->SetColourDepthStencilSurface();
	Display::Device()->InvalidateState();
//	Display::Device()->EndScene();
}

// Font cache list singleton
FontCacheList C_FontCache::m_sList;

C_FontCache::C_FontCache( const S32 cell_size, const S32 num_cells, const char* pFontName ) : 
	m_CellSize( cell_size ),
	m_NumCells( num_cells ),
	m_CurrentFrame( 1 )
{
	S32 font = gFont.FindFontByName( pFontName, true );
	if( font == -1 )
	{
		LOGFATAL( g_Log, DBSTR "Failed to find %s font", pFontName );
	}

	m_pPolyFont = (C_UIPolygonFont*)gFont.GetFont( font );
	if( !m_pPolyFont )
	{
		LOGFATAL( g_Log, DBSTR "Failed to get %s font", pFontName );
	}

	m_pCacheTexture = new C_FontCacheAlternateTexture( m_CellSize, m_NumCells );
	if( !m_pCacheTexture )
	{
		LOGFATAL( g_Log, DBSTR "Failed to create font cache texture");
	}

	m_FontCache.Resize( num_cells );

	// Fake bitmap attributes
	m_BitmapFontAttr.m_TextureWidth = m_CellSize*m_NumCells;
	m_BitmapFontAttr.m_TextureHeight = m_CellSize;
	m_BitmapFontAttr.m_FontHeight = m_CellSize;
	m_BitmapFontAttr.m_VrtSpacing = 0;
	m_BitmapFontAttr.m_HrzSpacing = 0;
	m_BitmapFontAttr.m_Name = pFontName;
	m_BitmapFontAttr.m_Texture = gSimple2D.SetAlternateTexture( m_pCacheTexture );
}

C_FontCache::~C_FontCache()
{
	SAFE_DELETE_PTR( m_pCacheTexture );
}

I_GlyphAttributes* C_FontCache::DrawRequest( const F32 height, const WCHAR glyph )
{
	if(glyph == L' ')
		return NULL;

	U64 old_time = 0;
	S32 old_index = -1;

	F32 h = height;
	if( (S32)h > m_CellSize )
		return NULL;
//		h = (F32)m_CellSize;

	// Check if we already exist
	for(S32 i = 0; i < m_NumCells; i++)
	{
		C_FontCacheItem& fc = m_FontCache[ i ];

		if( fc.m_Glyph == glyph && fc.m_Height == h )
		{
			// Update time last drawn
			if( fc.m_FrameUsed )
				fc.m_FrameUsed = m_CurrentFrame;

			return &fc.m_BitmapGlyph;
		}
		// This char is being drawn this frame
		if( fc.m_FrameUsed == 0 )
			continue;

		// This slot is free
		if( fc.m_FrameUsed == 0xffffffff )
		{
			old_index = i;
			break;
		}

		// Old slot - check time
		U64 diff = m_CurrentFrame - fc.m_FrameUsed; // Note doesn't work when wrapping around
		if( diff && diff > old_time )
		{
			old_time = diff;
			old_index = i;
		}
	}

	// Failed to find slot
	if( old_index == -1 )
		return NULL;

	const I_GlyphAttributes* pGlyph = m_pPolyFont->LookupGlyph( glyph, true );
	// Does this glyph actually exist to be cached?
	if( !pGlyph )
		return NULL;

	m_FontCache[ old_index ].m_Glyph		= glyph;
	m_FontCache[ old_index ].m_Height		= h;
	m_FontCache[ old_index ].m_FrameUsed	= 0;

	F32 frac = (h / (F32)m_CellSize);
	
	F32 u1 = (F32)(old_index * m_CellSize);
	F32 v2 = (F32)m_CellSize * frac;
	F32 u2 = u1 + v2;
		
	m_FontCache[ old_index ].m_BitmapGlyph = C_BitmapGlyphAttribute( glyph, pGlyph->GetOffset(), POLYGON_FONT_HEIGHT, pGlyph->GetAdvance() - (POLYGON_FONT_HEIGHT-pGlyph->GetWidth()), (U16)u1, (U16)u2, 0, (U16)v2, &m_BitmapFontAttr );;

	m_FontRequestList.PushBack( &m_FontCache[ old_index ] );

	return &m_FontCache[ old_index ].m_BitmapGlyph;
}

void C_FontCache::Render()
{
	m_CurrentFrame++;

	if( m_FontRequestList.Size() == 0 )
		return;

	if( !m_pCacheTexture->Bind() )
		return;

	const F32 pColour[ 4 ] = { 1.0f, 1.0f, 1.0f, 1.0f };
	const F32 wBB = (F32)(m_CellSize * m_NumCells);
	const F32 hBB = (F32) m_CellSize;

	// backbuffer -> device
	Maths::C_Rect shaderBounds( Maths::HVec2( -1.0f, 1.0f ), Maths::HVec2( 1.0f, -1.0f ) );
	Maths::C_Transform2d shaderTransform = Maths::C_Transform2d( shaderBounds );

	Maths::C_Rect view( Maths::HVec2( 0, 0 ), Maths::HVec2( wBB, hBB ) );
	Maths::C_Transform2d viewInv = Maths::C_Transform2d( view );
	viewInv.Invert();

	Maths::C_Transform2d deviceTransform = shaderTransform * viewInv;

	Maths::HMat43 device( Maths::C_MatrixScale( deviceTransform.Scale().X(), deviceTransform.Scale().Y(), 1.0f ) );
	device.Translation( Maths::HVec3( deviceTransform.Offset().X(), deviceTransform.Offset().Y(), 0.0f ) );
	Maths::HMat44 deviceMatrix( device );
	// done
	Maths::HMat44 trans = deviceMatrix;

	gSimple2D.SetTransformMatrix2D( trans );

	gSimple2D.GetRenderer().Bind();
	gSimple2D.GetRenderer().SetPolygonFontSettings( C_PolygonFontEffectParam( 0 ).GetShaderSettings() );

	// Draw requested letters
	for( S32 x = 0; x < m_NumCells; x++)
	{
		// Previously drawn?
		if( m_FontCache[ x ].m_FrameUsed )
			continue;

		C_UICharacter* pGlyph = (C_UICharacter*)m_pPolyFont->LookupGlyph( m_FontCache[ x ].m_Glyph );
		if(!pGlyph)
			continue;

		if(pGlyph->GetType() != I_GlyphAttributes::E_GlyphType_Polygon)
		{
			LOGWARN( g_Log, DBSTR "Wrong glyph type");
			continue;
		}
		Maths::HMat43 mat;
		const F32 scale = m_FontCache[ x ].m_Height / (F32)POLYGON_FONT_HEIGHT;

		mat.Rotation( Maths::C_MatrixScale(scale,scale,scale));
		mat.Translation( Maths::HVec3( (F32)(m_CellSize * x), 0.0f, 0.0f ) );

		gSimple2D.GetRenderer().SetPolygonFlatSettings( mat, pColour );
		gSimple2D.GetRenderer().SetEffect( E_Effect_PolygonFont, 0, E_TextureAddressMode_Wrap );

		C_UITriListRenderInfo& pRenderInfo = pGlyph->GetRenderInfo();
		gSimple2D.GetRenderer().RenderTriList( pRenderInfo.m_VertexBuffer, pRenderInfo.m_IndexBuffer, pRenderInfo.m_uIndexCount, pRenderInfo.m_uVertexCount );

		m_FontCache[ x ].m_FrameUsed = m_CurrentFrame;

		m_pCacheTexture->ResolveCell( x );
	}
	
	m_pCacheTexture->Unbind();

	// Resolve and restore
	gSimple2D.GetRenderer().Unbind();
	gSimple2D.SetTransformMatrix2D();

	// Clear request list
	while( m_FontRequestList.Size() )
		m_FontRequestList.PopBack();
}

} // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
