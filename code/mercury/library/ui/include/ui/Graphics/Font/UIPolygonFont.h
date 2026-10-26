
#ifndef UIPOLYGONFONT_H
#define UIPOLYGONFONT_H

#ifdef _MSC_VER
#pragma once
#endif

#define POLYGON_FONT_HEIGHT			1

#include "ui/Graphics/Simple2D.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Polygons/UIPolygonClasses.h"
#include <btl/Containers/containers.h>

#if 0//defined( USER_PEREID )
#define _DEBUG_POLYFONT_
#endif

namespace UI
{

class C_FontCache;

/// @addtogroup PolygonFont
/// @{

/// kerning hash function
U32 GetKerningHash( const WCHAR first, const WCHAR second );

// ---------------
// C_UIKerningPair
// ---------------
/// kerning pair for polygon fonts
class C_UIKerningPair
{
public:
	/// default - zero
	C_UIKerningPair()
		: m_First( 0 ),
		m_Second( 0 ),
		m_fKernAmount( 0.0f )
	{
		//
	}

	/// construct each part
	C_UIKerningPair( const WCHAR first, const WCHAR second, const F32 fKernAmount )
		: m_First( first ),
		m_Second( second ),
		m_fKernAmount( fKernAmount )
	{
		//
	}

	~C_UIKerningPair()				{  }

public:
	WCHAR		First() const		{ return m_First; }				///< first character in the pair
	WCHAR		Second() const		{ return m_Second; }			///< second character in the pair
	F32			KernAmount() const	{ return m_fKernAmount; }		///< kern amount in logical space

private:
	WCHAR		m_First;
	WCHAR		m_Second;
	F32			m_fKernAmount;
};

// --------------
// C_GlyphMetrics
// --------------
/// glyph metrics for the polygon fonts
class C_GlyphMetrics
{
public:
	/// default - zero
	C_GlyphMetrics()
		: m_fOffset( 0.0f ),
		m_fWidth( 0.0f ),
		m_fAdvance( 0.0f )
	{
		//
	}

	/// construct each part
	C_GlyphMetrics( const F32 fOffset, const F32 fWidth, const F32 fAdvance )
		: m_fOffset( fOffset ),
		m_fWidth( fWidth ),
		m_fAdvance( fAdvance )
	{
		//
	}

	~C_GlyphMetrics()				{  }

public:
	F32			Offset() const			{ return m_fOffset; }		///< @a
	F32			Width() const			{ return m_fWidth; }		///< @a
	F32			Advance() const			{ return m_fAdvance; }		///< @a

private:
	F32			m_fOffset;
	F32			m_fWidth;
	F32			m_fAdvance;
};

// -----------------
// C_UICharacterData
// -----------------
/// raw data for a polygon font character
class C_UICharacterData
{
public:
	C_UICharacterData();	///< default - zero
	~C_UICharacterData();	///< @a

public:
	void					Cleanup();	///< cleanup resources

	WCHAR					Character() const								{ return m_Character; }		///< @a
	void					Character( const WCHAR c )						{ m_Character = c; }		///< @a

	const C_GlyphMetrics&	GlyphMetrics() const							{ return m_GlyphMetrics; }	///< @a
	void					GlyphMetrics( const C_GlyphMetrics& gm )		{ m_GlyphMetrics = gm; }	///< @a

	void					AllocPolyCount( const U32 uPolyCount );										///< reserve the amount of polys we need
	void					AllocPolygonPointCount( const U32 uPolyIndex, const U32 uPointCount );		///< reserve the amount of points we need for a poly

	void					SetPoint( const U32 uPolyIndex, const U32 uPointIndex, C_UIResultPoint& p );	///< set a point in the given poly at the given index

	UIPolygonList&			GetPolygonList()								{ return m_PolygonList; }	///< get the polygon list

private:
	WCHAR					m_Character;
	C_GlyphMetrics			m_GlyphMetrics;
	UIPolygonList			m_PolygonList;
};

// -------------------
// C_UIPolygonFontData
// -------------------
/// the raw data we read in from a .pf file for a polygon font
class C_UIPolygonFontData
{
public:
	C_UIPolygonFontData();		///< default - zero
	~C_UIPolygonFontData();		///< @a

public:
	void						Cleanup();	///< cleanup resources

	/// read in the raw data from a .pf file
	void						ReadFromFile( const char* pFilename );

	/// get the kerning for a character - returns zero if it doesn't exist
	const F32					LookupKerning( const WCHAR first, const WCHAR second ) const;

	const char*					GetFontName() const				{ return m_pFontName; }						///< @a
	const char*					GetFilePath() const				{ return m_pFilePath; }						///< @a

	const U32					GetNumCharacters() const		{ return m_uNumCharacters; }				///< @a

	C_UICharacterData*			GetCharacterData( U32 uIndex )	{ return &m_pCharacterData[ uIndex ]; }		///< @a

private:
	typedef Btl::OpenHashMap< U32, F32 >	KerningHashMap;

	KerningHashMap		m_KerningHash;

	bool				m_bInitialised;

	static const U32	m_uFontNameLength = 256;
	char				m_pFontName[ m_uFontNameLength ];

	static const U32	m_uFilePathLength = 512;
	char				m_pFilePath[ m_uFilePathLength ];

	U32					m_uNumCharacters;
	C_UICharacterData*	m_pCharacterData;

	U32					m_uNumKerningPairs;
	C_UIKerningPair*	m_pKerningPairs;
};

// ------------------------
// C_UIPolygonFontDataStore
// ------------------------
/// data storage class for all polygon font data so we only ever read and store the data once per font
class C_UIPolygonFontDataStore
{
public:
	C_UIPolygonFontDataStore();		///< @a
	~C_UIPolygonFontDataStore();	///< cleanup font data

public:
	/// get the global instance of the C_UIPolygonFontDataStore
	static C_UIPolygonFontDataStore&	Instance()								{ return m_sInstance; }

public:
	/// get the raw font data - if it doesn't already exist then it will be built here
	C_UIPolygonFontData*				GetFontData( const char* pFileName );

private:
	static	C_UIPolygonFontDataStore	m_sInstance;

private:
	static const U32																		m_uMaxPolygonFonts = 8;
	Btl::Vector< C_UIPolygonFontData, Btl::VectorParameters_Static< m_uMaxPolygonFonts > >	m_FontData;
};

// -------------
// C_UICharacter
// -------------
/// a character with a polygon font
class C_UICharacter : public I_GlyphAttributes
{
public:
	C_UICharacter();
	~C_UICharacter();

public:
	void					Cleanup();		///< cleanup resources

	const C_GlyphMetrics&	GlyphMetrics() const							{ return m_pCharacterData->GlyphMetrics(); }	///< @a
	C_UITriListRenderInfo&	GetRenderInfo()									{ return m_RenderInfo; }						///< @a

	/// patch together quadratic slines based on the font descriptions.
	void					EvaluateSplines( UIPolygonList& evaluatedList );
	/// are then any holes? if so - solve them
	void					FindAndResolveHoles( UIPolygonList& polygonList, UIPolygonList& resolvedList );
	/// expand the shapes to get an outline polygon
	void					BuildOutline( UIPolygonList& evaluatedList, UIPolygonList& outlineList, UIPolygonIndexList& indexList );
	/// extrude into 3D
	void					Build3D( UIPolygonList& resolvedList, UIPolygonIndexList& indexList );
	/// extrude into 3D and bevel the front face
	void					BuildBevel3D( UIPolygonList& evaluatedList, UIPolygonList& resolvedList, UIPolygonIndexList& indexList );
	/// software AA through finning
	void					BuildFinned( UIPolygonList& evaluatedList, UIPolygonList& resolvedList, UIPolygonIndexList& indexList );

	void					Triangulate( UIPolygonList& resolvedList, UIPolygonIndexList& indexList );	///< @a

	/// build a vertex and index buffer for the character
	void					BuildVertexAndIndexBuffer( UIPolygonList& resolvedList, UIPolygonIndexList& indexList );

	// interface with font renderer
	const WCHAR				GetCharacter() const	{ return m_pCharacterData->Character(); }			///< @a
	const F32				GetOffset() const		{ return GlyphMetrics().Offset(); }					///< @a
	const F32				GetWidth() const		{ return GlyphMetrics().Width(); }					///< @a
	const F32				GetAdvance() const		{ return GlyphMetrics().Advance(); }				///< @a
	const E_GlyphType		GetType() const			{ return E_GlyphType_Polygon; }						///< @a

#ifndef CFG_GOLD
	void					DebugRender( const F32 x, const F32 y, const F32 fFontHeight, const C_UIColour& colour, const bool bOutline );	///< @a
#endif

	/// set the raw character data from the C_UIPolygonFontDataStore
	void					CharacterData( C_UICharacterData* pData )		{ m_pCharacterData = pData; }

private:
	Maths::HVec2			GetOutlinePoint( C_UIResultPolygon& poly, S32 sIndex );		///< @a

private:
	C_UICharacterData*		m_pCharacterData;
	C_UITriListRenderInfo	m_RenderInfo;
};

// ---------------
// C_UIPolygonFont
// ---------------
/// the polygon font
class C_UIPolygonFont : public I_FontAttributes
{
public:
	C_UIPolygonFont();
	virtual ~C_UIPolygonFont();

public:
	void					Cleanup();	///< cleanup resources

	bool					Initialised()							{ return m_bInitialised; }		///< @a						
	const char*				GetFontName()							{ return m_pFontName; }			///< @a

	/// Bind texture cache
	void					EnableTextureCache();
						
#ifndef CFG_GOLD		
	void					DebugRender( const C_UIColour& colour, const bool bOutline = false );	///< @a
#endif

	/// retrieve (or build) from the C_UIPolygonFontDataStore
	void					BuildFontData( const char* pFilename );
	/// create the font from the given file
	virtual void			Build( const char* pFilename );
	/// return a glyph def if the character exists within this font
	const I_GlyphAttributes*	LookupGlyph( const WCHAR glyph, const bool bNullIfNotFound = false, const F32 drawHeight = 0.0f ) const;
	/// get the kerning for a character - returns zero if it doesn't exist
	const F32				LookupKerning( const WCHAR first, const WCHAR second ) const;

	// interface with I_FontAttributes

	/// this shouldn't be called - does nothing
	void					Read( Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar, const S32 dataVersion  )	{  }

	const F32				GetFontHeight() const			{ return POLYGON_FONT_HEIGHT; }									///< @a
	const F32				GetHrzOffset( bool left ) const	{ return (F32)0.0f; }											///< @a
	const F32				GetVrtOffset( bool top ) const	{ return (F32)0.0f; }											///< @a
	const U16				GetNumGlyphs() const			{ return m_pFontData->GetNumCharacters(); }						///< @a
	const Util::C_String256	GetName() const					{ return Util::C_String256( m_pFontName ); }					///< @a
	const I_GlyphAttributes*	GetDefaultGlyph() const			{ return m_pDefaultGlyph; }										///< @a
	const F32				GetSizeOveride() const 			{ return 0; }													///< @a
	const S32				GetVersion() const				{ return 0; }													///< @a

	/// returns address to the C_UITriListRenderInfo for the given character
	/// @remarks returns 0 if character doesn't exist in this font
	INTPTR					GetUserData( const WCHAR c );	// [Blur2PC][x64] a pointer

protected:
	void					GenerateHashMap();			///< generate the glyph hash map

protected:
	typedef Btl::OpenHashMap< U32, C_UICharacter* >	GlyphHashMap;

	bool					m_bInitialised;

	C_UIPolygonFontData*	m_pFontData;
	C_UICharacter*			m_pCharacters;
	C_UICharacter*			m_pDefaultGlyph;

	GlyphHashMap			m_GlyphHash;

	static const U32		m_uFontNameLength = 256;
	char					m_pFontName[ m_uFontNameLength ];

	C_FontCache *			m_pTextureCache;
};

// ----------------------
// C_UIPolygonFontOutline
// ----------------------
/// the outline for a polygon font
class C_UIPolygonFontOutline : public C_UIPolygonFont
{
public:
	/// create the font from the given file
	void			Build( const char* pFilename );
};

// -----------------
// C_UIPolygonFont3D
// -----------------
/// a 3d polygon font
class C_UIPolygonFont3D : public C_UIPolygonFont
{
public:
	/// create the font from the given file
	void			Build( const char* pFilename );
};

// ------------------------
// C_UIPolygonFontOutline3D
// ------------------------
/// the outline for a 3d polygon font
class C_UIPolygonFontOutline3D : public C_UIPolygonFont
{
public:
	/// create the font from the given file
	void			Build( const char* pFilename );
};

// ----------------------
// C_UIPolygonFontBevel3D
// ----------------------
/// a beveled 3d polygon font
class C_UIPolygonFontBevel3D : public C_UIPolygonFont
{
public:
	/// create the font from the given file
	void			Build( const char* pFilename );
};

// ---------------------
// C_UIPolygonFontFinned
// ---------------------
/// a 2d finned polygon font
class C_UIPolygonFontFinned : public C_UIPolygonFont
{
public:
	/// create the font from the given file
	void			Build( const char* pFilename );
};

#ifdef _DEBUG_POLYFONT_
// -------------------
// C_UIPolygonFontTest
// -------------------
class C_UIPolygonFontTest
{
public:
	C_UIPolygonFontTest();
	~C_UIPolygonFontTest();

public:
	void			Init();
	void			DestroyAll();

	void			BuildFont( const char* pFilePath, const bool bOutlineFont = false );

	U32				NumFonts()											{ return m_uNumFonts; }
	const char*		GetFontName( const U32 uSlot );

	INTPTR			GetUserData( const U32 uFont, const WCHAR c );

#ifndef CFG_GOLD
	void			DebugRender( const S32 sSlot = -1 );
#endif

private:
	U32									m_uNumFonts;

	static const U32					m_uMaxPolygonFonts = 2;
	C_UIPolygonFont*					m_Fonts[ m_uMaxPolygonFonts ];
};
#endif

// ------------------------
// C_PolygonFontEffectParam
// ------------------------
/// U32 effect param containing the necessary things for the shaders
class C_PolygonFontEffectParam
{
public:
			operator U32() const				{ return param; }		///< @a
	void	operator = ( const U32 c )			{ param = c; }			///< @a

public:
	/// default
	C_PolygonFontEffectParam()
	{
		param = 0;
	}

	/// construct from another
	C_PolygonFontEffectParam( const U32 c )
		: param( c )
	{
		//
	}

public:
	void			SetOutlineWidth( const F32 fOutlineWidth );			///< outline width
	void			SetExtrusion( const F32 fExtrusion );				///< 3d extrusion
	void			SetExtrusionBevel( const F32 fExtrusionBevel );		///< 3d bevel extrusion
	
	Maths::HVec4	GetShaderSettings();								///< build a vector to pass down to the shaders

public:
	union
	{
		U32	param;

		struct
		{
			U32	outline			: 7;
			U32	outlineF		: 3;
			U32	extrusion		: 6;
			U32	extrusionF		: 5;
			U32	extrusionBevel	: 6;
			U32	extrusionBevelF	: 5;
		} m_Components;
	};
};

/// @}

#ifdef _DEBUG_POLYFONT_
extern C_UIPolygonFontTest gUIPolygonFont;
#endif // _DEBUG_POLYFONT_

};	// namespace UI

#endif	/* UIPOLYGONFONT_H */
