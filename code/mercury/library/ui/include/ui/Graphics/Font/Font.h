#ifndef FONT_H
#define FONT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/containers/openhashtable.h>
#include <ui/graphics/simple2d.h>
#include <ui/Graphics/Localisation/loc.h>
#include <ui/Game/UILua.h>

#include <ui/Util/WideString.h>

#include <btl/Containers/containers.h>

#define UI_FONT_REFACTOR_TODO

namespace UI
{

// forward decl
class C_UIPolygonFont;
class C_FontStyle;
class C_FontStylePasses;

/// @addtogroup UI_Types
/// @{
	
enum E_FontFlag
{
	E_FontFlag_Normal = 0,			///< @a
	E_FontFlag_NormalNoWrap,		///< @a
	E_FontFlag_CheckingLength,		///< @a
	E_FontFlag_CheckingLines,		///< @a

	E_FontFlag_Count
};

enum E_FixedSpaceType
{
	E_FixedSpaceType_None = 0,		///< @a
	E_FixedSpaceType_Numbers,		///< @a
	E_FixedSpaceType_Letters,		///< @a
	E_FixedSpaceType_All,			///< @a

	E_FixedSpaceType_Count
};

/// @}

// ----------------
// I_GlyphAttributes
// ----------------
/// Glyph interface base class
class I_GlyphAttributes
{
public:
	enum E_GlyphType
	{
		E_GlyphType_None = 0,	///< @a
		E_GlyphType_Bitmap,		///< @a
		E_GlyphType_Polygon,		///< @a
		E_GlyphType_Shape,		///< @a
	};

public:
	virtual ~I_GlyphAttributes()								{  };

public:
	virtual const E_GlyphType	GetType() const = 0;
	virtual const WCHAR			GetCharacter() const = 0;		///< Get the wide character for this glyph
	virtual const F32			GetOffset() const = 0;			///< Amount to advance before drawing glyph
	virtual const F32			GetWidth() const = 0;			///< Actual draw width
	virtual const F32			GetAdvance() const = 0;			///< Amount to advance after drawing glyph
};

// ----------------
// I_FontAttributes
// ----------------
/// Font interface base class
class I_FontAttributes
{
public:
	virtual ~I_FontAttributes( )	{  }

	virtual void						Read( Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar, const S32 dataVersion = 0 ) = 0;
	virtual const I_GlyphAttributes*	LookupGlyph( const WCHAR glyph, const bool bNullIfNotFound = false, const F32 drawHeight = 0.0f ) const = 0;
	virtual const F32					LookupKerning( const WCHAR first, const WCHAR second ) const = 0;
	virtual const F32					GetFontHeight() const = 0;
	virtual const F32					GetHrzOffset( bool left ) const = 0;
	virtual const F32					GetVrtOffset( bool top ) const = 0;
	virtual const U16					GetNumGlyphs() const = 0;
	virtual const Util::C_String256		GetName() const = 0;
	virtual const I_GlyphAttributes*	GetDefaultGlyph() const = 0;
	virtual const F32					GetSizeOveride() const = 0;
	virtual const S32					GetVersion() const = 0;
};

// --------------
// C_FontSequence
// --------------
/// Font sequnces so we can use an escape characters in strings to display non-standard characters such as the 'B' controller button or an infinity symbol
class C_FontSequence
{
public:
	bool                    m_InputGlyph; // [Blur2PC] eligible for the active input atlas
	C_UIColour				m_Colour;
	C_TextureHandleIndex	m_Texture;
	C_WideString32			m_Sequence;
	F32						m_U; 
	F32						m_V;
	F32						m_SizeU;
	F32						m_SizeV;
	F32						m_Aspect;
	F32						m_LeadingX;
	F32						m_TrailingX;
	F32						m_ScaleX;
	F32						m_ScaleY;
	F32						m_OffsetY;
	bool					m_DropShadow		: 1;
	bool					m_AllowItalics		: 1;
	U32						m_SceneIndex;		/// Used with shapes. Scene containing the shape.
	char					m_GroupName[16];	/// Group that we want to pull out.
};

// ----------------
// C_FontSpriteItem
// ----------------
/// Font item to be held in list before a string is rendered, used to justify string and also be externally manipulated (see C_FontPreRender)
class C_FontSpriteItem
{
public:
	enum E_Type
	{
		E_Type_Normal,		/// <@
		E_Type_Seq,			/// <@
		E_Type_EAsian,		/// <@
	};

public:
	C_FontSpriteItem()					{  }
	C_FontSpriteItem(const C_FontSpriteItem& copy);
	C_FontSpriteItem& operator = (const C_FontSpriteItem& copy);

public:
	/// use to set a bitmap font item
	__forceinline	void	Set( const I_GlyphAttributes::E_GlyphType eFontType, const C_TextureHandleIndex texture, const bool bLineBreak, 
								 const E_Type eItemType, const Maths::HVec3& pos, const F32 sx, const F32 sy, 
								 const C_UISpriteColours& col, 
								 const F32 tx1, const F32 ty1, const F32 txS, const F32 tyS, 
								 const bool bDropShadow, const bool bItalics, 
								 const LayerIndex layer, WCHAR character,
								 const F32 fActualWidth );

	/// use to set a polygon font item
	__forceinline	void	Set( const I_GlyphAttributes::E_GlyphType eFontType, const INTPTR uUserData, const bool bLineBreak, 
								 const E_Type eItemType, const Maths::HVec3& pos, const F32 sx,const F32 sy, 
								 const C_UISpriteColours& col, 
								 const bool bDropShadow, const bool bItalics, 
								 const LayerIndex layer, WCHAR character,
								 const F32 fActualWidth );

public:
	WCHAR							m_Character;
	I_GlyphAttributes::E_GlyphType	m_eFontType;

	C_TextureHandleIndex			m_Texture;
	bool                            m_InputGlyph; // [Blur2PC] retained in cached text
	INTPTR							m_UserData;		// [Blur2PC][x64] carries pointers

	C_UISpriteColours				m_Col;

	E_Type			m_eItemType;
	LayerIndex		m_Layer;

	Maths::HVec3	m_Pos;
	F32 			m_Sx;
	F32 			m_Sy;

	F32				m_fActualWidth;
	
	F32 			m_T0_px;
	F32 			m_T0_py;
	F32 			m_T0_sx;
	F32 			m_T0_sy;

	F32 			m_T1_px;
	F32 			m_T1_sx;
	F32 			m_T1_py;
	F32 			m_T1_sy;

	F32				m_T2_px;
	F32				m_T2_sx;

	U8 				m_DropShadow	: 1;
	U8 				m_Italics		: 1;
	U8				m_bLineBreak	: 1;
};
/// Doubly linked list of sprite items
// typedef Btl::List< C_FontSpriteItem* >		SpriteList;
class C_FontSpriteStack
{
public:
							C_FontSpriteStack( )				{ Reset();	}

	void					DrawStart( const U32 start );
	void					DrawEnd( const U32 end );
	const U32				DrawStart( ) const					{ return m_DrawStart; }
	const U32				DrawEnd( ) const					{ return m_DrawEnd;	}

	void					Reset( );

	void					Resize( const U32 capacity );
	const U32				Size( ) const;

	C_FontSpriteItem*		PopNext( );

	C_FontSpriteItem&		operator []( const U32 index )		{ return m_Array[ index ]; }
	C_FontSpriteStack&		operator=( C_FontSpriteStack& _in );

private:
	Btl::Vector< C_FontSpriteItem >		m_Array;
	U16									m_DrawStart;
	U16									m_DrawEnd;
};

// ---------------
// C_FontPreRender
// ---------------
/// Pre render callback function so gadgets can 
class C_FontPreRender
{
public:
	virtual ~C_FontPreRender() {}
	virtual void ProcessStack( C_FontSpriteStack& stack ) = 0;		///< Overide this to get the callback to modify the sprite list before it's rendered
};

// ------
// C_Font
// ------
/// Global singleton for drawing text
class C_Font
{
public:
	// [Blur2PC] Select at draw time so cached text also follows physical input.
	void SetInputGlyphTexture( const C_TextureHandleIndex texture ) { m_InputGlyphTexture = texture; }
	C_TextureHandleIndex InputGlyphTexture() const { return m_InputGlyphTexture; }

private:
	C_TextureHandleIndex m_InputGlyphTexture;

public:
	C_Font();
	~C_Font();

public:
	static const F32	s_fUseDefaultHeightPercentage;
	static const U32	m_MaxTextures = 2;				// Number of 'extra' texture stages to try and load
	static const WCHAR	s_NullCharacter = 0xfffe;

public:
	void 	Destroy();															///< Clean up allocated memory
	
	void 	StartFrame();														///< Reset counts for this frame
	void 	Initialise( const char* pFilename = NULL );							///< Initialise, load bitmap font data file if filename passed

	void 	CreatePolygonFont( const char* pFilename, const bool b2D, const bool bOutline2D, const bool b3D, const bool bOutline3D, const bool bBevel3D, const bool bFinned2D );	///< load, build and add a polygon font files
	void 	AddPolygonFont( C_UIPolygonFont* pPolygonFont );																								///< add in the given polygon font
	
	void 	InitialiseCommonAdditionalFont( const char* pFilename );			///< (Old) Additional font : used for large EA bitmap font sheets
	void 	InitialiseUIAdditionalFont( const char* pFilename );				///< (Old) Additional font : used for large EA bitmap font sheets
	void 	DestroyUIAdditionalFont();											///< (Old) Additional font : used for large EA bitmap font sheets
	void 	DestroyCommonAdditionalFont();										///< (Old) Additional font : used for large EA bitmap font sheets
	
	void 	InitialiseSequences( lua_State* pState );							///< Initialise the font sequences from LUA (global LUA_FONTSEQ_TABLE_NAME position)
	void 	CreateLuaEnums( lua_State* pState );								///< Add fonts to LUA (global LUA_FONT_TABLE_NAME position)

	// single fontstyle
	void	DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const WCHAR* pString, const C_UISpriteColours& colours, const F32 fHeight, C_FontStyle& style, const F32 fWrapSize = 0.0f, const PanelIndex panel = 0 );	///< @a
	void	DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStyle& style, const F32 fWrapSize = 0.0f, const PanelIndex panel = 0 );		///< @a
	void	DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStyle& style, const F32 fWrapSize = 0.0f, const PanelIndex panel = 0 );		///< @a
	// multi-pass font style
	void	DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const WCHAR* pString, const C_UISpriteColours& colours, const F32 fHeight, C_FontStylePasses& styles, const F32 fWrapSize = 0.0f, const PanelIndex panel = 0 );	///< @a
	void	DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStylePasses& styles, const F32 fWrapSize = 0.0f, const PanelIndex panel = 0 );		///< @a
	void	DrawText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const C_TextStringID& id, const C_UISpriteColours& colours, const F32 fHeight, C_FontStylePasses& styles, const F32 fWrapSize = 0.0f, const PanelIndex panel = 0 );		///< @a

#ifndef CFG_GOLD
	/// draw a debug string using the default font style
	void	DrawDebugText( const LayerIndex layer, const Maths::HVec3& pos, const E_Justify eJustify, const WCHAR* pString, const C_UIColour& colour, const F32 fHeight, const F32 fWrapSize = 0.0f, const PanelIndex panel = 0 );
#endif

	F32		StringLength( const WCHAR* pString, const F32 fHeight, C_FontStyle& style, const bool bInternal = false );	///< @a
	F32		StringLength( const WCHAR* pString, const F32 fHeight, const bool bInternal = false );						///< debug text - uses default font style
	F32		StringLength( C_TextStringID& id, const F32 fHeight, C_FontStyle& style, const bool bInternal = false );	///< @a

	U32		StringNumLines( const WCHAR* pString, const F32 fHeight, const F32 fWrapSize, C_FontStyle& style );		///< @a
	U32		StringNumLines( const WCHAR* pString, const F32 fHeight, const F32 fWrapSize );							///< debug text - uses default font style
	U32		StringNumLines( C_TextStringID& id, const F32 fHeight, const F32 fWrapSize, C_FontStyle& style );		///< @a

	F32						GetHeightPercentage( C_FontStyle& style );
	I_FontAttributes*		GetFont( const U32 index )																{ assert( index < m_uNumFonts ); return m_pFonts[ index ]; }	///< @a
	S32						FindFontByName( const char* pFontName, const bool bZeroIfNotFound = true );				///< Returns index of font

	U8		GetItalicsMatrixIndex()											{ return m_ItalicsMatrixIndex; }				///< Reserved pre-transform matrix index (see Simple2D)
	U32		GetNumLinesDrawn()												{ return m_uNumLinesDrawn + 1; }				///< Get number of lines the string took to draw (useful only if wrapped)

#ifndef UI_FONT_REFACTOR_TODO
	void 	DoClippingNext(const F32 startX = gSimple2D.ScreenLeft(), const F32 endX = gSimple2D.ScreenRight(), const F32 startY = 0.0f, const F32 endY = UI_VSCREEN_SIZEY)	{ mClipStartX = startX; mClipEndX = endX; mClipStartY = startY; mClipEndY = endY; }
#endif

	void 	IgnoreEscapeColoursNext()										{ m_bIgnoreEscapeColours = true; }			///< @a
	void 	NoReverseTextureLookupNext()									{ m_bUseReverseTextureLook = false; }		///< (Old)

	void 	SetDrawCountNext( const S32 sDrawCount )						{ m_sDrawCount = sDrawCount; }				///< @a
	void 	SetPreRenderNext( C_FontPreRender* pPreRender )					{ m_pPreRender = pPreRender; }				///< @a
	void	FixedSpaceNext( const E_FixedSpaceType eFixedSpaceType )		{ m_eFixedSpaceType = eFixedSpaceType; }	///< @a

	// transform matrix functions
	void 			PreTransformMatrixNext( const TransformIndex matrix_index )		{ m_PreTransformIndex = matrix_index; }		///< @a
	void 			PostTransformMatrixNext( const TransformIndex matrix_index )	{ m_PostTransformIndex = matrix_index; }	///< @a
	void			ClearVars();										///< clear ready for the next work to process

	TransformIndex 	GetPreTransformMatrix() const									{ return m_PreTransformIndex; }		///< @a
	TransformIndex 	GetPostTransformMatrix() const									{ return m_PostTransformIndex; }	///< @a
	TransformIndex 	GetItalicsMatrix() const										{ return m_ItalicIndex; }			///< @a

	E_Justify		GetCurrentJustify() const										{ return m_eCurrentJustify; }		///< @a

private:
	void						ClearLists();										///< clear the render lists
	void						StoreVars( I_FontAttributes* pCurrentFont, const F32 fHeight );

	F32							ProcessText( const WCHAR* pStringText, const I_FontAttributes* pFont, F32 fHeight, const F32 fWrapSize, C_FontStyle& style, const E_FontFlag eFontRenderFlag = E_FontFlag_Normal ); ///< @a

	const I_FontAttributes *	GetFontAttributes( WCHAR glyph, bool* ea_glyph );	///< (Old) Additional font : Lookup which font a 'glyph' comes from 

	bool						CouldWrap( WCHAR letter ) const					{ return ( ( letter == 0x0020 ) | ( letter > 0x3000 ) ); }			///< @a

	bool						ShouldWrap( const F32 x, const WCHAR* pStringText, const F32 fWrapSize, const F32 fHeight, C_FontStyle& style );	///< @a

	F32							GetWrapShiftY( const F32 fFontHeight, const F32 fHeightPercentage, const PanelIndex panel )
								{
									return ( fFontHeight * ( gSimple2D.IsPanel3D( panel ) ? -fHeightPercentage : fHeightPercentage ) );
								}

	S32							ExtractSequenceEscapeNumber( const WCHAR* pSeq );
	F32							ExtractSequenceEscapeNumberF( const WCHAR* pSeq );

private:
	static const WCHAR			jpn_wordwrap[];

	// fonts
	U32							m_uNumFonts;
	I_FontAttributes*			m_pCommonAdditionalFont;
	I_FontAttributes*			m_pUIAdditionalFont;

	static const U32			m_uMaxFonts = 64;
	I_FontAttributes*			m_pFonts[ m_uMaxFonts ];

	const I_FontAttributes*		m_pCurrentFont;


	static const S32			m_MaxFontSpriteItems = 580;
	
	// italic support
	static const TransformIndex	m_ItalicsMatrixIndex = 1;										// reserved from Simple2D
	TransformIndex				m_ItalicIndex;													// actual - will be different to the above if we want italics _and_ a pre-transform

	// font sequences
	U32							m_uNumFontSequences;
	C_FontSequence*				m_pFontSequences;

	// transforms
	TransformIndex				m_PreTransformIndex;
	TransformIndex				m_PostTransformIndex;

	Maths::HVec3				m_StartPos;
	
	F32							m_fCurrentHeight;
	F32							m_fDrawHeight;
							
	LayerIndex					m_CurrentLayer;
	PanelIndex					m_CurrentPanel;

	C_UISpriteColours			m_CurrentColour;

	E_Justify					m_eCurrentJustify;

	E_FixedSpaceType			m_eFixedSpaceType;

	bool						m_bIgnoreEscapeColours;

	bool						m_bWordStart;
	bool						m_bSentenceStart;

	bool						m_bUseReverseTextureLook;

	S32							m_sDrawCount;

	U32							m_uNumLinesDrawn;

	F32							m_fDefaultHeightPercentage;

	F32							mClipStartX;
	F32							mClipEndX;
	F32							mClipStartY;
	F32							mClipEndY;

	C_FontSpriteStack			m_SpriteStack;
	
	C_FontPreRender*			m_pPreRender;

	bool						m_bInitialised;

	bool						m_bDoNotClearVars;
};

extern C_Font gFont;

}; // namespace UI

#endif
