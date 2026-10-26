#ifndef FONTSTYLE_H
#define FONTSTYLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/graphics/Simple2d.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Localisation/loc.h"
#include "ui/Game/UILua.h"

#include <btl/Containers/containers.h>

#define _FONTSTYLE_BASE( _TYPE, _TYPESTRING ) \
	virtual const E_FontStyleRenderType		Type()			{ return _TYPE; }			\
	static  const E_FontStyleRenderType		TypeID()		{ return _TYPE; }			\
	static  const Util::C_String32			TypeName()		{ return _TYPESTRING; }		\

namespace UI
{

/// @addtogroup FontStyle
/// @{

// ----------------------
// C_FontStyleTextureInfo
// ----------------------
/// a texture for a font style
class C_FontStyleTextureInfo
{
public:
	enum E_FontMappingMode
	{
		E_FontMappingMode_None,				///< texture slot inactive : ignore
		E_FontMappingMode_AsLetter,			///< copy UVs from letter texture
		E_FontMappingMode_ZeroToOne,		///< map [UV] = 0 [UV]size = 1
		E_FontMappingMode_Shifted,			///< as RndShifted without the randomness
		E_FontMappingMode_RndShifted,		///< as 01 but shifted in the U random-ish (based on screen co-ords)
	};

public:
	/// default - none
	C_FontStyleTextureInfo()
		: m_eMode( E_FontMappingMode_None )
	{
		//
	}

public:
	bool					GetUVFromMode( const C_FontSpriteItem& item, F32& _u, F32& _v, F32& _usize, F32& _vsize );	///< @a

public:	
	E_FontMappingMode		m_eMode;
	C_TextureHandleIndex	m_Texture;
};

// -------------------
// C_FontStyleTextures
// -------------------
/// font textures per render pass in the font style
class C_FontStyleTextures
{
public:
	bool				GetFromLua( lua_State* pState );	///< @a

public:
	static const U32	m_uMaxTextures = 2;
	Btl::Vector< C_FontStyleTextureInfo, Btl::VectorParameters_Array< m_uMaxTextures > >	m_Textures;
};

enum E_FontStyleRenderType
{
	E_FontStyleRenderType_Default = 0,		///< @a

	//
	E_FontStyleRenderType_PolygonOutline,	///< @a
	E_FontStyleRenderType_Polygon3D,		///< @a
	E_FontStyleRenderType_PolygonOutline3D,	///< @a
	E_FontStyleRenderType_PolygonGradient,	///< @a
	E_FontStyleRenderType_PolygonBevel3D,	///< @a
	//

	E_FontStyleRenderType_Count,			///< @a
};

enum E_XtFontShader
{
	E_XtFontShader_Null							= E_Effect_Null,						///< @a
	E_XtFontShader_MultiTexture					= E_Effect_MultiTexture,				///< For bitmaps
	E_XtFontShader_IntensityAsAlpha				= E_Effect_IntensityAsAlpha,			///< For bitmaps
	E_XtFontShader_PolygonFont					= E_Effect_PolygonFont,					///< @a
	E_XtFontShader_PolygonFontOutlineFixed		= E_Effect_PolygonFontOutlineFixed,		///< @a
	E_XtFontShader_PolygonFontOutlineFade		= E_Effect_PolygonFontOutlineFade,		///< @a
	E_XtFontShader_PolygonFontOutlineFixedFade	= E_Effect_PolygonFontOutlineFixedFade,	///< @a
	E_XtFontShader_PolygonFontTextured			= E_Effect_PolygonFontTextured,			///< @a
	E_XtFontShader_PolygonFontGradient			= E_Effect_PolygonFontGradient,			///< @a
	E_XtFontShader_PolygonFontGradient_Param	= E_Effect_PolygonFontGradient_Param,	///< @a
	E_XtFontShader_PolygonFontGradient_Radial	= E_Effect_PolygonFontGradient_Radial,	///< @a
	E_XtFontShader_PolygonFontFinned			= E_Effect_PolygonFontFinned,			///< @a
	E_XtFontShader_PolygonFontFinnedOuter		= E_Effect_PolygonFontFinnedOuter,		///< @a
};
// -------------------------
// C_FontStyleBaseRenderInfo
// -------------------------
/// base render pass info for a font style
class C_FontStyleBaseRenderInfo
{
public:
	_FONTSTYLE_BASE( E_FontStyleRenderType_Default, "Default" );

public:
	/// default
	C_FontStyleBaseRenderInfo( const F32 fOffsetX = 0.0f, const F32 fOffsetY = 0.0f, const F32 fOffsetZ = 0.0f, bool bOffsetProportional = false )
		: m_bUseColourOverride( false ),
		m_BlendMode( E_UiBlendMode_Unknown ),
		m_EffectIndex( E_XtFontShader_Null ),
		m_fOffsetX( fOffsetX ), m_fOffsetY( fOffsetY ), m_fOffsetZ( fOffsetZ ), m_bOffsetProportional( bOffsetProportional ),
		m_pPolygonFontOverride( NULL )
	{
		//
	}

	/// colour override
	C_FontStyleBaseRenderInfo( const C_UISpriteColours colourOverride, const F32 fOffsetX = 0.0f, const F32 fOffsetY = 0.0f, const F32 fOffsetZ = 0.0f, bool bOffsetProportional = false )
		: m_ColourOverride( colourOverride ), m_bUseColourOverride( true ),
		m_BlendMode( E_UiBlendMode_Unknown ),
		m_EffectIndex( E_XtFontShader_Null ),
		m_fOffsetX( fOffsetX ), m_fOffsetY( fOffsetY ), m_fOffsetZ( fOffsetZ ), m_bOffsetProportional( bOffsetProportional ),
		m_pPolygonFontOverride( NULL )
	{
		//
	}

	virtual ~C_FontStyleBaseRenderInfo()	{  }

public:
	virtual void			Bind(); ///< Bind the stlye after variables have been grabbed (from Lua or Xt)
	virtual bool			GetFromLua( lua_State* pState );																		///< @a
	virtual void			Render( C_FontSpriteStack& spritestack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel );	///< @a
	
	C_RenderState&			GetRenderState()						{ return m_RenderState; }			///< @a

protected:
	__forceinline	Maths::HVec3		GetOffset( const F32 fFontHeight, const PanelIndex panel );		///< @a
	__forceinline	C_UISpriteColours	GetColourOverride( const C_UISpriteColours& original );			///< @a

	void								AddCharacter( const C_FontSpriteItem& item, C_FontStyle& style, C_RenderState& renderState, const PanelIndex panel );	///< @a

	void								AddShape( const U32 sceneIndex, const char* groupName, const Maths::HVec3& pos, const Maths::HVec3& size, C_RenderState renderState, const LayerIndex layer, const PanelIndex panel );

	/// render 'normal' but modifying the user data using the given polygon font
	void								RenderPolygonFontNewUserData( C_UIPolygonFont* pFont, C_FontSpriteStack& spritestack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel );

public: // Exposed for Xt
	XtSz8					m_FontOverideName;
	XtSz8					m_ColourStyle;
	XtArray<XtSz8>			m_TextureNames;
	F32						m_Alpha;
	F32						m_fOffsetX;
	F32						m_fOffsetY;
	F32						m_fOffsetZ;
	bool					m_bOffsetProportional;
	C_RenderState			m_RenderState;
	E_XtFontShader			m_EffectIndex;
	E_UiBlendMode			m_BlendMode;
	
protected:

	C_FontStyleTextures		m_FontTextures;	// additional texture per render pass

	C_UISpriteColours		m_ColourOverride;
	bool					m_bUseColourOverride;

	C_UIPolygonFont*		m_pPolygonFontOverride;
};

// -------------------------
// C_FontStylePolygonOutline
// -------------------------
/// polygon font outline style
class C_FontStylePolygonOutline : public C_FontStyleBaseRenderInfo
{
public:
	_FONTSTYLE_BASE( E_FontStyleRenderType_PolygonOutline, "PolygonOutline" );

public:
	/// default
	C_FontStylePolygonOutline()	
		: C_FontStyleBaseRenderInfo(),
		m_fOutlineAmount( 0.0f )
	{
		//
	}

public:
	virtual void	Bind(); ///< Bind the stlye after variables have been grabbed (from Lua or Xt)
	virtual bool	GetFromLua( lua_State* pState );																			///< @a
	virtual void	Render( C_FontSpriteStack& spritestack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel );		///< @a

public: // Exposed for Xt
	F32							m_fOutlineAmount;
};

// --------------------
// C_FontStylePolygon3D
// --------------------
/// polygon font 3d style
class C_FontStylePolygon3D : public C_FontStyleBaseRenderInfo
{
public:
	_FONTSTYLE_BASE( E_FontStyleRenderType_Polygon3D, "Polygon3D" );

public:
	/// default
	C_FontStylePolygon3D()	
		: C_FontStyleBaseRenderInfo(),
		m_fExtrudeAmount( 0.0f )
	{
		//
	}

public:
	virtual void	Bind(); ///< Bind the stlye after variables have been grabbed (from Lua or Xt)
	virtual bool	GetFromLua( lua_State* pState );																			///< @a
	virtual void	Render( C_FontSpriteStack& spritestack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel );		///< @a

public: // Exposed for Xt
	F32							m_fExtrudeAmount;
};

// ---------------------------
// C_FontStylePolygonOutline3D
// ---------------------------
/// polygon font outline 3d style
class C_FontStylePolygonOutline3D : public C_FontStyleBaseRenderInfo
{
public:
	_FONTSTYLE_BASE( E_FontStyleRenderType_PolygonOutline3D, "PolygonOutline3D" );

public:
	/// default
	C_FontStylePolygonOutline3D()	
		: C_FontStyleBaseRenderInfo(),
		m_fOutlineAmount( 0.0f ), m_fExtrudeAmount( 0.0f )
	{
		//
	}

public:
	virtual void	Bind(); ///< Bind the stlye after variables have been grabbed (from Lua or Xt)
	virtual bool	GetFromLua( lua_State* pState );																			///< @a
	virtual void	Render( C_FontSpriteStack& spritestack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel );		///< @a

public: // Exposed for Xt
	F32							m_fOutlineAmount;
	F32							m_fExtrudeAmount;
};

// --------------------------
// C_FontStylePolygonGradient
// --------------------------
/// polygon gradient font style
class C_FontStylePolygonGradient : public C_FontStyleBaseRenderInfo
{
public:
	_FONTSTYLE_BASE( E_FontStyleRenderType_PolygonGradient, "PolygonGradient" );

public:
	/// default
	C_FontStylePolygonGradient()	
		: C_FontStyleBaseRenderInfo(),
		m_uGradientIndex( 0 ), m_fGradientAngle( 0.0f )
	{
		//
	}

public:
	virtual bool	GetFromLua( lua_State* pState );																			///< @a
	virtual void	Render( C_FontSpriteStack& spritestack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel );		///< @a

public: // exposed for Xt
	U32				m_uGradientIndex;
	F32				m_fGradientAngle;
};

// -------------------------
// C_FontStylePolygonBevel3D
// -------------------------
/// polygon font beveled 3d style
class C_FontStylePolygonBevel3D : public C_FontStyleBaseRenderInfo
{
public:
	_FONTSTYLE_BASE( E_FontStyleRenderType_PolygonBevel3D, "PolygonBevel3D" );

public:
	/// default
	C_FontStylePolygonBevel3D()	
		: C_FontStyleBaseRenderInfo(),
		m_fOutlineAmount( 0.0f ), m_fExtrudeAmount( 0.0f ), m_fExtrudeAmountBevel( 0.0f )
	{
		//
	}

public:
	virtual void	Bind();	///< bind the style after variables have been grabbed (from Lua or Xt)
	virtual bool	GetFromLua( lua_State* pState );																			///< @a
	virtual void	Render( C_FontSpriteStack& spritestack, C_FontStyle& style, const F32 fFontHeight, const PanelIndex panel );		///< @a

public: // exposed for Xt
	F32							m_fOutlineAmount;
	F32							m_fExtrudeAmount;
	F32							m_fExtrudeAmountBevel;
};

// -----------
// C_FontStyle
// -----------
/// font style that can store multiple plug-in render passes
class C_FontStyle
{
public:
	C_FontStyle();
	C_FontStyle( const C_FontStyle& o );
	~C_FontStyle();

public:
	static C_FontStyle&	Instance()		{ return m_sInstance; }		///< @a
	static void			SorceryCallback( XtEventType type, XtRecord* pRecord, C_FontStyle* pFontStyle, XtCallbackContext* pContext );

public:
	void				XtManaged( )		{ m_ManagedByXt = true; } ///< Memory managed by Xt
	void				Cleanup();	///< @a

	void				InitDefaultStyle( lua_State* pState );	///< attempt to grap a default style from Lua (global LUA_FONTSTYLE_TABLE_NAME position)

	void				Bind(); ///< Bind the stlye after variables have been grabbed (from Lua or Xt)
	bool				GetFromLua( lua_State* pState, const char* pTableName = NULL );		///< @a

//	void						AddRenderPassInfo( C_FontStyleBaseRenderInfo* pRenderInfo )			{ m_RenderPassInfo.PushBack( pRenderInfo ); }		///< @a
	U32							GetRenderPassCount() const											{ return (U32)m_RenderPassInfo.count; }				///< @a
	C_FontStyleBaseRenderInfo&	GetRenderPassInfo( const S32 uIndex ) const							{ return *m_RenderPassInfo[ uIndex ]; }				///< @a

	void				Render( C_FontSpriteStack& spritestack, const F32 fFontHeight, const PanelIndex panel );	///< @a

	U32					Font() const						{ return m_uFont; }						///< @a
	void				Font( const U32 uFont )				{ m_uFont = uFont; }					///< @a

	F32					Tracking() const					{ return m_fTrackingOverride; }			///< @a
	void				Tracking( const F32 fTracking )		{ m_fTrackingOverride = fTracking; }	///< @a

	F32					HeightPercentage() const			{ return m_fHeightPercentage; }			///< @a
	void				HeightPercentage( const F32 fHP )	{ m_fHeightPercentage = fHP; }			///< @a

	bool				Italics() const						{ return m_bItalics; }					///< @a
	void				Italics( const bool bItalics )		{ m_bItalics = bItalics; }				///< @a

	bool				PixelBoundaryX() const				{ return m_bPixelBoundaryX; }			///< @a
	void				PixelBoundaryX( const bool bDoIt ) 	{ m_bPixelBoundaryX = bDoIt; }			///< @a

	bool				PixelBoundaryY() const				{ return m_bPixelBoundaryY; }			///< @a
	void				PixelBoundaryY( const bool bDoIt ) 	{ m_bPixelBoundaryY = bDoIt; }			///< @a

public:
	C_FontStyle&		operator = ( const C_FontStyle& o );	///< @a

public: // Exposed for Xt
	XtSz8				m_Name;
	XtSz8				m_FontName;
	U32					m_uFont;

	F32					m_fTrackingOverride;
	F32					m_fHeightPercentage;

	bool				m_bItalics;

	bool				m_bPixelBoundaryX;
	bool				m_bPixelBoundaryY;

	// render pass info
//	Btl::Vector< C_FontStyleBaseRenderInfo* >	m_RenderPassInfo;
	XtArray<C_FontStyleBaseRenderInfo* >		m_RenderPassInfo;

private:
	static C_FontStyle								m_sInstance;

private:
	bool				m_ManagedByXt;
};

/// type for a pointer to a Button Creator function
typedef C_FontStyleBaseRenderInfo* (*CreateFontStyleFunctionPtr)( C_FontStyleBaseRenderInfo* pCopy );

/// template to create a font style of registered type
template< class T >C_FontStyleBaseRenderInfo* CreateNewFontStyle( C_FontStyleBaseRenderInfo* pCopy )
{
	T* pNew = new T;
	assert( pNew );

	// copy over the derived type
	if ( pCopy )
		*pNew = *(T*)pCopy;

	return pNew;
}

// ----------------------
// C_FontStyleCreatorItem
// ----------------------
/// container for a font style creation
class C_FontStyleCreatorItem
{
public:
	/// default
	C_FontStyleCreatorItem() : m_pCreator( NULL )		{  }

public:
	CreateFontStyleFunctionPtr		m_pCreator;
	Util::C_String32				m_Name;
};

// ------------------
// C_FontStyleCreator
// ------------------
/// global instance for creating font style
class C_FontStyleCreator
{
public:
	static C_FontStyleCreator&	Instance()		{ return m_sInstance; }		///< @a

public:
	/// register font styles here
	void						Initialise()
								{
									Register< C_FontStyleBaseRenderInfo >();
									Register< C_FontStylePolygonOutline >();
									Register< C_FontStylePolygon3D >();
									Register< C_FontStylePolygonOutline3D >();
									Register< C_FontStylePolygonGradient >();
									Register< C_FontStylePolygonBevel3D >();
								}

	void						CreateLuaEnums( lua_State* pState );

	C_FontStyleBaseRenderInfo*	CreateFontStyle( const E_FontStyleRenderType eType, C_FontStyleBaseRenderInfo* pCopy = NULL )	///< @a
	{
		if ( !m_Creators[ eType ].m_pCreator )
		{
			LOGFATAL( g_Log, DBSTR "%d has not been registered", eType );
			return NULL;
		}

		return m_Creators[ eType ].m_pCreator( pCopy );
	}

private:
	template< class T > void	Register()	///< register a font style
    {
		E_FontStyleRenderType eType = T::TypeID();

		if ( !m_Creators[ eType ].m_pCreator )
		{
			m_Creators[ eType ].m_pCreator	= (CreateFontStyleFunctionPtr)CreateNewFontStyle< T >;
			m_Creators[ eType ].m_Name = T::TypeName();
		}
    }

private:
	static C_FontStyleCreator	m_sInstance;

private:
	Btl::Vector< C_FontStyleCreatorItem, Btl::VectorParameters_Array< E_FontStyleRenderType_Count > >	m_Creators;
};

// -----------------
// C_FontStylePasses
// -----------------
/// a class to hold and render multiple font styles
class C_FontStylePasses
{
public:
	C_FontStylePasses();
	~C_FontStylePasses();	///< cleanup
	
public:
	/// either resolves the style names from Sorcery or attempt to grab the font styles from Lua
	void				Setup( lua_State* pState = NULL, const char* pLuaName = "styles" );

	U32					NumFontStyles() const;					///< @a
	C_FontStyle&		GetFontStyle( const U32 uIndex );		///< @a

public:	// exposed for Xt
	XtSz8	m_StyleName;

private:
	bool								m_bUseXt;			// lookup from Sorcery

	Btl::Vector< C_FontStyle* >			m_XtFontStyles;		// if we're just referencing from Sorcery - just store pointers to avoid allocations
	Btl::Vector< C_FontStyle >			m_FontStyles;		// else we allocate if we're grabbing fully from Lua
};

/// @}

}; // namespace UI

#endif	// FONTSTYLE_H
