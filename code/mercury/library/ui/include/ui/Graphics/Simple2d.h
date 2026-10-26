#ifndef SIMPLE2D_H
#define SIMPLE2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/Containers/containers.h>
#include <btl/Allocators/pool.h>
#include <Xt/XtGame.h>
#include "ui/UIDefines.h"
#include "ui/Graphics/UITextureManager.h"
#include "ui/Graphics/UIColourClasses.h"
#include "ui/Graphics/UISpriteTextureParam.h"
#include "ui/Graphics/Renderer.h"
#include "ui/UILogger.h"
#include "ui/illustrator/UIAdobeColour.h"

// default panel number macro
#define	 PANEL_NUM( x )		( 1 << x )

namespace UI
{

class C_GadgetBase;
class C_GadgetBaseData;

// Enumerations

/// @addtogroup UI_Types
/// @{

/// Debug colours
enum E_BasicColour
{
	E_BasicColour_White			= 0xffffffff,
	E_BasicColour_Grey			= 0xff777777,
	E_BasicColour_LightGrey		= 0xffcccccc,
	E_BasicColour_Black			= 0xff000000,
	E_BasicColour_Red			= 0xffff0000,
	E_BasicColour_Green			= 0xff00ff00,
	E_BasicColour_Blue			= 0xff0000ff,
	E_BasicColour_Cyan			= 0xff00ffff,
	E_BasicColour_Magenta		= 0xffff00ff,
	E_BasicColour_Yellow		= 0xffffff00,
	E_BasicColour_Orange		= 0xffff9900,
};

enum E_UiBlendMode
{
	// Mapped from Display
	E_UiBlendMode_Solid				= Display::E_BlendMode_Solid,
	E_UiBlendMode_SolidAlphaTest	= Display::E_BlendMode_SolidAlphaTest,
	E_UiBlendMode_SolidAlphaMask	= Display::E_BlendMode_SolidAlphaMask,
	E_UiBlendMode_Alpha				= Display::E_BlendMode_Alpha,
	E_UiBlendMode_AlphaPreMultiply	= Display::E_BlendMode_AlphaPreMultiply,
	E_UiBlendMode_Additive			= Display::E_BlendMode_Additive,
	E_UiBlendMode_AlphaAdditive		= Display::E_BlendMode_AlphaAdditive,
	E_UiBlendMode_AlphaSubtractive	= Display::E_BlendMode_AlphaSubtractive,
	E_UiBlendMode_Subtractive		= Display::E_BlendMode_Subtractive,
	E_UiBlendMode_Multiply			= Display::E_BlendMode_Multiply,
	E_UiBlendMode_AdditiveSmooth	= Display::E_BlendMode_AdditiveSmooth,
	E_UiBlendMode_Count				= Display::E_BlendMode_Count,
	E_UiBlendMode_Default			= Display::E_BlendMode_Default,
	E_UiBlendMode_Unknown			= Display::E_BlendMode_Unknown,
	// Extended Ui modes
	E_UiBlendMode_ExtMultiplyAdd,	// Render target alpha blending colour : adding alpha
};

/// Sprite render types
enum E_SpriteType
{
	E_SpriteType_SetRendersStateStack = 0,	///< @a Push/Pop a new render stack item
	E_SpriteType_SetColourWrite,			///< @a Sets current render stack item's colour write enable
	E_SpriteType_SetClipWindow,				///< @a Sets current render stack item's clip window state
	E_SpriteType_SetDepthBuffer,			///< @a Sets current render stack item's depth buffer state
	E_SpriteType_SetStencilBuffer,			///< @a Sets current render stack item's stencil state
	E_SpriteType_SetCullMode,				///< @a Sets current render stack item's cull mode	
	E_SpriteType_SetAlphaTest,				///< @a
	E_SpriteType_RenderStateCount,			///< @a End of list for render state change sprites

	E_SpriteType_Sprite,					///< @a
	E_SpriteType_TriList,					///< @a
	E_SpriteType_TriStrip,					///< @a
	E_SpriteType_Model,						///< @a

	E_SpriteType_Count						///< End of internal sprite render
};

/// Default layers, can be overridden
enum E_SpriteLayer
{
	E_SpriteLayer_Background = 0,
	E_SpriteLayer_1,
	E_SpriteLayer_2,
	E_SpriteLayer_3,
	E_SpriteLayer_4,
	E_SpriteLayer_Top,

	E_SpriteLayer_Count
};

/// Default sprite panels - can be overridden
enum E_SpritePanel
{
	E_SpritePanel_Main = 0,	///< Main full screen viewport
	E_SpritePanel_3D0,			///< 3D - panel 0
	E_SpritePanel_3D1,			///< 3D - panel 1

	E_SpritePanel_2D1,			///< 2D - and extra panel
	
	E_SpritePanel_RT0,			///< Render target - panel 0
	E_SpritePanel_RT1,			///< Render target - panel 1
	E_SpritePanel_RT2,			///< Render target - panel 2
	
	E_SpritePanel_VP0,			///< Viewport - panel 0
	E_SpritePanel_VP1,			///< Viewport - panel 1
	E_SpritePanel_VP2,			///< Viewport - panel 2
	E_SpritePanel_VP3,			///< Viewport - panel 3

	E_SpritePanel_Count
};

// Types
/// Layer index
typedef U32				LayerIndex;
/// Panel index
typedef U32				PanelIndex;
/// Transform stack index (PreTransform, PostTransform and colour)
typedef U32				TransformIndex;
/// Sprite texture stage index : See C_Sprite::m_Textures
typedef	S32				TextureStage;

/// @}

// ---------------------
// C_UICharacterVertex3d
// ---------------------
/// Simple vertex declaration just describing a 3D position
class C_UICharacterVertex3d
{
public:
	/// Default constructor
	C_UICharacterVertex3d() : m_fX( 0.0f ), m_fY( 0.0f ), m_fZ( 0.0f )		{  }
	/// Default deconstructor
	~C_UICharacterVertex3d()												{  }

public:
	/// Set from 2D position
	void	SetPos( const Maths::HVec2& p )						{ m_fX = p.X(); m_fY = p.Y(); m_fZ = 0.0f; }
	/// Set from 3D position
	void	SetPos( const Maths::HVec3& p )						{ m_fX = p.X(); m_fY = p.Y(); m_fZ = p.Z(); }
	/// Set from 2D position
	void	SetPos( const F32 fX, const F32 fY )				{ m_fX = fX; m_fY = fY; m_fZ = 0.0f; }

private:
	F32		m_fX;
	F32		m_fY;
	F32		m_fZ;
};

// ----------------------------
// C_UICharacterOutlineVertex3d
// ----------------------------
/// vertex position and an outline direction ratio vector for polygon fonts
class C_UICharacterOutlineVertex3d
{
public:
	/// Default constructor
	C_UICharacterOutlineVertex3d() 
		: m_X( 0 ), m_Y( 0 ), m_V(0)
	{
		//
	
	}

public:
	/// set with a position and a vector
	void	Set( const Maths::HVec2& p, const Maths::HVec4& v )
			{ 
				m_X = (S32)Maths::Clamp<int>((int)(p.X() * 32767.0f), -32767, 32767);
				m_Y = (S32)Maths::Clamp<int>((int)(p.Y() * 32767.0f), -32767, 32767);

				U32 vx = (U32)Maths::Clamp<int>((int)(127.0f + v.X() * 127.0f), 0, 255);
				U32 vy = (U32)Maths::Clamp<int>((int)(127.0f + v.Y() * 127.0f), 0, 255);
				U32 vz = (U32)v.Z();
				U32 vw = (U32)v.W();

#ifdef PLATFORM_PS3
				m_V = (vx << 24) | (vy << 16) | (vz << 8) | vw;
#else
				m_V = (vw << 24) | (vz << 16) | (vy << 8) | vx;
#endif
			}

private:
	S16		m_X;
	S16		m_Y;
	U32		m_V;
};

// ---------------------
// C_UIStrokeVertex3d
// ---------------------
/// Simple vertex declaration just describing a 3D position
class C_UIStrokeVertex3d
{
public:
	/// Default constructor
	C_UIStrokeVertex3d() 
	:	m_X( 0 ), 
		m_Y( 0 ), 
		m_U( 0 ),
		m_V( 0 )
	{  

	}

	/// Default deconstructor
	~C_UIStrokeVertex3d()												{  }

public:
	/// Set from 2D position
	void	SetPos( const Maths::HVec2& p )
	{
		m_X = (S32)Maths::Clamp<int>((int)(p.X() * 32767.0f), -32767, 32767);
		m_Y = (S32)Maths::Clamp<int>((int)(p.Y() * 32767.0f), -32767, 32767);
	}

	/// Set texture coordinates
	void	SetUV( const Maths::HVec2& p )
	{
		m_U = (S32)Maths::Clamp<int>((int)(p.X() * 32767.0f), -32767, 32767);
		m_V = (S32)Maths::Clamp<int>((int)(p.Y() * 32767.0f), -32767, 32767);
	}

private:
	S16		m_X;
	S16		m_Y;
	S16		m_U;
	S16		m_V;
};


// ---------------------
// C_UITriListRenderInfo
// ---------------------
class C_UITriListRenderInfo
{
public:
	C_UITriListRenderInfo() 
	:	m_uIndexCount( 0 ),
		m_uVertexCount( 0 ),
		m_StaticShapeOffset( 0.0f, 0.0f, 0.0f )
	{
		//
	}

	~C_UITriListRenderInfo()
	{
		Destroy();
	}

	void Destroy()
	{
		Display::Device()->Destroy( &m_VertexBuffer );
		Display::Device()->Destroy( &m_IndexBuffer );
	}

public:
	Display::C_VertexBuffer				m_VertexBuffer;
	Display::C_IndexBuffer				m_IndexBuffer;
	U32									m_uIndexCount;
	U32									m_uVertexCount;

	// this is a temporary solution for static shape offsets when isolating single shapes.
	Maths::HVec3						m_StaticShapeOffset; 
};


// ------------
// C_Vertex3d
// ------------
/// Generic use vertex declaration with 3D position, 32bit RGBA colour and 3 sets of UV coordinates
class C_Vertex3d
{
public:
	/// Set 3D position
	void			SetPos(const Maths::HVec3& vPos)		{ m_X = vPos.X(); m_Y = vPos.Y(); m_Z = vPos.Z(); }
	/// Set 3D position
	void			SetPos(F32 vx, F32 vy, F32 vz)			{ m_X = vx; m_Y = vy; m_Z = vz; }

	/// Set 32bit RGBA colour
	void			SetColour(const Maths::RGBA& rgba)
					{
						m_Colour = rgba;
					};
	/// Set 32bit RGBA colour
	void			SetColour(const Maths::HVec4& colour)
					{
						Maths::Store(colour, (MathsStorage::Float4 *) &m_Colour);
					};

	void			SetUV0(const Maths::HVec2& vUV)		{ Maths::Store(vUV, (MathsStorage::Float2 *) &m_U0); }		///< Set UV coordinate set 0
	void			SetUV1(const Maths::HVec2& vUV)		{ Maths::Store(vUV, (MathsStorage::Float2 *) &m_U1); }		///< Set UV coordinate set 1
	void			SetUV2(const Maths::HVec2& vUV)		{ Maths::Store(vUV, (MathsStorage::Float2 *) &m_U2); }		///< Set UV coordinate set 2

public:
	F32				m_X, m_Y, m_Z;
	U32				m_Colour;
	F32				m_U0, m_V0;
	F32				m_U1, m_V1;
	F32				m_U2, m_V2;
};

// -----------------
// C_UIAutoTransform
// -----------------
/// class the sets up Simple2D automatically for transform hierarchy stuffage and reset on deconstruction
class C_UIAutoTransform
{
public:
	/// @a
	C_UIAutoTransform( C_GadgetBase* pObject );	

	/// @a
	~C_UIAutoTransform();

private:
	TransformIndex	m_PrevTi;
	Maths::HVec3	m_PrevOffset;
};

// #define _UI_AUTO_CLIPWINDOW
#ifdef _UI_AUTO_CLIPWINDOW
// --------------
// C_UIClipWindow
// --------------
/// Clip window object - usally created by a gadget before rendering of sprites, - automatically clears the clip window when the object goes out of scope.
class C_UIClipWindow
{
public:
	C_UIClipWindow();
	/// Constructor : Activates the clip window for this panel / layer - automatically clears the clip window when the object goes out of scope.
	C_UIClipWindow( const LayerIndex layer, const PanelIndex panel, const F32 x1, const F32 y1, const F32 x2, const F32 y2 );
	/// Deconstructor : Clears clip window (if active)
	~C_UIClipWindow();

private:
	bool			m_bActive;
	LayerIndex		m_Layer;
	PanelIndex		m_Panel;	
};
#endif

// --------------
// C_UIAutoStencil
// --------------
/// Stencil state object - usally created by a gadget before rendering of sprites, - automatically clear the stencil state when the object goes out of scope.
class C_UIAutoStencil
{
public:
	C_UIAutoStencil( const LayerIndex layer, const PanelIndex panel, const Display::C_StencilState& op );
	~C_UIAutoStencil( );
private:
	bool			m_bActive;
	LayerIndex		m_Layer;
	PanelIndex		m_Panel;	
};

/// Render state object held by each sprite, sprites get batched together when their renderstates and texture stages and the same.
class C_RenderState
{
public:
	C_RenderState()			{ Init(); }

public:
	/// Initialise with the default render state
	void Init( )
	{
		m_BlendMode		= E_UiBlendMode_Alpha;
		m_EffectIndex	= E_Effect_MultiTexture;
		m_EffectParam	= 0;
		m_EffectParam2	= 0;
	}

	const bool				operator==( const C_RenderState& rhs ) const
	{
		return ( rhs.m_BlendMode == m_BlendMode && rhs.m_EffectIndex == m_EffectIndex && rhs.m_EffectParam == m_EffectParam && rhs.m_EffectParam2 == m_EffectParam2 );
	}

public:
	static const U16		m_StencilUnused = 0x7;

	/// Get a copy of the default render state
	static const C_RenderState&	Instance( )											{ return m_sInstance; }
	/// Setup the default render state, should only be called by the UIManager
	static void					Instance( const C_RenderState& defaultInstance )	{ m_sInstance = defaultInstance; }
	
public:
	U8						m_BlendMode;
	U8						m_EffectIndex		: 6;
	U8						m_EffectAddressMode : 2;
	U32						m_EffectParam;
	U32						m_EffectParam2;

private:
	static C_RenderState	m_sInstance;
};

/// Sprite quad texture parameter
class C_Simple2DTexture
{
public:
	C_TextureHandleIndex	m_Texture;

	F32						m_pu, m_pv;
	F32						m_su, m_sv;
};

/// Generic sprite object, everything rendered by the UI is a sprite of some kind, extended types use m_UserData
class C_Sprite
{
public:
	Display::C_StencilState StencilState( ) const
	{
		Display::C_StencilState stencilState;

		// Pack stencil ops into colours
		*((U32*)&stencilState.m_StencilFunc)		= *((U32*)&m_Pos.x);
		*((U32*)&stencilState.m_StencilFail)		= *((U32*)&m_Pos.y);
		*((U32*)&stencilState.m_StencilDepthFail)	= *((U32*)&m_Pos.z);
		stencilState.m_StencilDepthPass				= (Display::E_StencilOp)(int)m_Colours[0];
		stencilState.m_StencilRef					= (U32)(int)m_Colours[1];
		stencilState.m_StencilMask					= (U32)(int)m_Colours[2];
		stencilState.m_StencilWriteMask				= (U32)(int)m_Colours[3];
		stencilState.m_StencilEnable				= (Display::E_Bool)m_UserData;

		return stencilState;
	}

	void StencilState( const Display::C_StencilState& stencilState )
	{
		// Unpack stencil ops from colours
		*((U32*)&m_Pos.x)	= *((U32*)&stencilState.m_StencilFunc);
		*((U32*)&m_Pos.y)	= *((U32*)&stencilState.m_StencilFail);
		*((U32*)&m_Pos.z)	= *((U32*)&stencilState.m_StencilDepthFail);
		m_Colours[0]		= C_UIColour( stencilState.m_StencilDepthPass );
		m_Colours[1]		= C_UIColour( stencilState.m_StencilRef );
		m_Colours[2]		= C_UIColour( stencilState.m_StencilMask );
		m_Colours[3]		= C_UIColour( stencilState.m_StencilWriteMask );
		m_UserData			= stencilState.m_StencilEnable;
	}

	static const U32		m_NumTextureStages = 3;

	MathsStorage::Float3	m_Pos;								// 12
	MathsStorage::Float3	m_Size;								// 12
	C_Simple2DTexture		m_Textures[m_NumTextureStages];		// 60(4)
	C_UISpriteColours		m_Colours;							// 16
	C_RenderState			m_RenderState;						// 12
	U16						m_PreTransformIndex;				// 1
	U16						m_PostTransformIndex;				// 1

	U16						m_Justify	: 4;					// 2
	U16						m_Layer		: 4;
	U16						m_Panel		: 4;					
	U16						m_Type		: 4;

	U16						m_Count;							// 2
	INTPTR					m_UserData;							// 4 [Blur2PC][x64] carries pointers; 8 on x64

	void					(*m_Callback)(const C_Sprite*);
};

/// Singleton class to managing and rendering the sprite list
class C_Simple2D
{
public:
	/// Interface class for alternative textures which are not contained within the UITextureManager such as movies and render targets.
	class I_AlternateTexture
	{
	public:
		virtual ~I_AlternateTexture( ) { }

		/// Return handle to the texture data
		virtual UITextureHandle				Handle( ) = 0;
		/// Name of the alternate texture
		virtual const Util::C_String32&		Name( ) = 0;
	};

	/// Gradient texture used, 1D gradient strips are wrote horizontally into a texture for use with Adobe polygon objects
	class C_GradientAlternateTexture : public I_AlternateTexture
	{
	public:
		/// Construct with texture width and height
		C_GradientAlternateTexture( const U32 width, const U32 height );
		/// Deconstructor
		virtual ~C_GradientAlternateTexture( );

		// Alternate texture Interface
		UITextureHandle			Handle( )	{ return &m_Texture; }
		Util::C_String32&		Name( )		{ return m_Name; }
		const U32				Height( )	{ return m_Height; }

		/// Build a gradient line on index line given an array of colours and 'stop' positions, of colour_count length
		void		Build( const U32 index, const C_UIColour* pColours, F32* pPositions, const U32 colour_count );
		/// Build a gradient line on index line with a C_AdobeGradient object
		void		Build( const U32 index, const C_AdobeGradient& gradient );
		/// Grab a new index and increase the counter
		const U8	PopIndex( )				{ assert( m_Index < m_Height ); return m_Index++; }
		/// Clear the index counter
		void		ResetIndex( )			{ m_Index = 0; }
	private:
		U8									m_Index;
		Util::C_String32					m_Name;
		U32									m_Width;
		U32									m_Height;
		Display::C_Texture2D				m_Texture;
	};

	/// Alternate texture setup for render targets
	class C_RenderTargetAlternateTexture : public C_Simple2D::I_AlternateTexture, public Display::I_Renderer
	{
	public:
		C_RenderTargetAlternateTexture( const S32 sIndex );
		C_RenderTargetAlternateTexture( const S32 sIndex, const U32 uWidth, const U32 uHeight );
		~C_RenderTargetAlternateTexture();

	public:
		bool							OnCreateVolatile();
		void							OnDestroyVolatile();

	public:	
		UITextureHandle					Handle()	{ return m_bInitialised ? &m_RenderTexture.GetTexture() : NULL; }
		const Util::C_String32&			Name()		{ return m_Name; }

		bool							BeginScene();
		void							EndScene();

	private:
		S32								m_sIndex;
		U32								m_Width;
		U32								m_Height;
		Util::C_String32				m_Name;

		bool							m_bInitialised;
		Display::C_RenderTexture2D		m_RenderTexture;
	};
	// Current render state, for renderstate stack
	class C_StackedRenderState
	{
	public:
		Display::E_Bool					m_AlphaTestEnabled;
		// Clip
		Display::E_Bool					m_ClipEnabled;
		XtVec2							m_ClipPos;
		XtVec2							m_ClipSize;
		// Z Buffer
		Display::E_Bool					m_DepthEnabled;
		Display::E_Bool					m_DepthWriteEnabled;
		Display::E_CmpFunc				m_DepthCompareFunc;
		// Cull					
		Display::E_CullMode				m_CullMode;
		// Stencil buffer
		Display::C_StencilState			m_StencilState;
		// Colour
		XtFlags<XtU32,Display::E_ColourWriteEnable>	m_ColourWriteEnable;
	public:
		struct C_DepthState
		{
			U32	enabled		: 1;
			U32	write		: 1;
			U32 function	: 30;
		};
	};

public:
	// Typedef sprite list.
	typedef Btl::Vector< C_Sprite* >			TSpriteList;
	typedef Btl::Vector< TSpriteList >			TPanelSpriteList;
	typedef Btl::Vector< TPanelSpriteList >		TLayerPanelSpriteList;

	//typedef Btl::Vector< Btl::Vector< Btl::Vector< C_Sprite* > > >	TSpriteList;

	// Public member functions
	C_Simple2D();
	~C_Simple2D();
	/// Initialise 
	void		Init( lua_State* pState, const U32 sprite_alloc_cnt = 2048, const U32 pre_matrix_cnt = 200, const U32 post_matrix_cnt = 1000, const U32 colour_matrix_cnt = 1 );
	/// Clean up memory (called by destructor)
	void		Exit( );
	/// Re allocate the sprite array - gets deffered until next time StartFrame() is called
	void		ReAllocSprites(const U32 sprite_alloc_cnt);

	void		StartFrame( );
#ifdef PLATFORM_PC
	/// [Blur2PC] counts StartFrame calls: the sprite lists are rebuilt once per game update, and with the
	/// decoupled PC loop several frames can render one set of lists
	U32			Blur2PCListGeneration( ) const { return m_Blur2PCListGeneration; }

	/// [Blur2PC] render interpolation for sprites that follow a car. The UI builds its sprite lists once per
	/// game update, from the car's update pose, while the car is drawn blended between its last two updates.
	/// A gadget registered as an anchor for the current lists (after StartFrame) marks every post-transform
	/// entry popped by it or its descendants (C_GadgetBase::CalcTransformHierarchy); every rendered frame,
	/// Blur2PCPlaceAnchored moves those entries by the car's move from `built` to its render pose. rigid:
	/// the whole move; otherwise only the translation (sprites that face the camera).
	void		Blur2PCSetAnchor( const C_GadgetBase* pAnchor, const S32 key, const bool rigid, const Maths::HMat43& built );
	void		Blur2PCNoteTransform( const C_GadgetBase* pGadget, const TransformIndex ti );
	typedef bool (*T_Blur2PCResolveAnchor)( const S32 key, Maths::HMat43& renderMatrix );
	void		Blur2PCPlaceAnchored( T_Blur2PCResolveAnchor resolve );
#endif
	/// Sets render states and binds the renderer
	void		PreRender( );
	/// Cleans up render states and un-binds the renderer
	void		PostRender( );

	/// Ui pixel shader lighting
	C_Renderer::C_Light&	GetLight( const PanelIndex panelIndex );
	void					SetLight( const PanelIndex panelIndex );

	/// User gamma correction
	const F32	GetGammaCorrection() const				{ return m_UserGamma; }

	void		SetGammaCorrection( const F32 gamma )	{ m_UserGamma = gamma; }

	///	Batch sprites ready for rendering.
	void		BuildSpriteBatches();

#ifndef CFG_GOLD
	/// Add helpers which are switched on by debug variables : Includes the Debug graphic and Safe area
	void		AddDebugHelpers( );
#endif
	/// Calls SetLayersAndPanels with the default setup, see E_SpriteLayer and E_SpritePanel
	void		SetDefaultLayersAndPanels( lua_State* pState );
	/// Setup non-default layer and panel list
	/// @remarks - remember to call AddPanelUsage3D for your 3D panels after this
	void		SetLayersAndPanels( lua_State* pState, const char** ppLayerNames, const U8 uLayerCount, const char** ppPanelNames, const U8 sPanelCount );

	bool				IsPanel3D( const PanelIndex panel )									{ return ( ( panel != 0 ) && ( PANEL_NUM( panel ) & m_sPanelUsage3D ) ); } ///< @a
	void				AddPanelUsage3D( const S8 sPanelUsage3D )							{ m_sPanelUsage3D |= PANEL_NUM( sPanelUsage3D ); }	///< @a

	const LayerIndex	LayerCount()														{ return m_uLayerCount; }		///< @a
	const LayerIndex	TopLayer()															{ return m_uLayerCount - 1; }	///< @a
	const PanelIndex	PanelCount()														{ return m_uPanelCount; }		///< @a

	void				SetClipSize2D( const F32 fClipWidth, const F32 fClipHeight );										///<Set the size of the back buffer
	F32					GetClipWidth() const												{ return m_fClipWidth; }		///< @a
	F32					GetClipHeight() const												{ return m_fClipHeight; }		///< @a
	
	const Maths::HMat44&	GetTransformMatrix() const										{ return m_TransformMatrix; }		///< @a
	const Maths::HMat44&	GetTransformMatrix2D() const									{ return m_TransformMatrix2D; }		///< @a

	void					SetTransformMatrix2D();																													///<Default back to 2D world
	void					SetTransformMatrix2D( const Maths::HMat44& transformMatrix );																			///< @a
	void					SetTransformMatrix3D( const Maths::HMat44& transformMatrix, const Maths::HVec3& viewPosition = Maths::HVec3( 0.0f, 0.0f, 0.0f ) );		///< @a
	void					SetTransformMatrixShaderConstant();																										///< @a
	void					SetTransformMatrixByPanel( const PanelIndex panel, const Maths::HMat44& transformMatrix )		{ m_PanelTransformMatrix[ panel ] = transformMatrix; }
	Maths::HMat44			GetTransformMatrixByPanel( const PanelIndex panel )												{ return m_PanelTransformMatrix[ panel ]; }

	void					CalcTransform2D( const F32* transformUI, const F32 fClipWidth, const F32 fClipHeight, const F32 fNear, const F32 fFar, Maths::HMat44& dest );
				
	bool					IsUsing2D()															{ return m_bUsing2D; }			///< @a

	/// Deprecated
	void				AddWideScreenBorders( );

	
	const F32			ToScreenX( const F32 x );																			///<Convert 0-1 to 0-640 (or the widescreen equiv
	const F32			ToScreenY( const F32 y );																			///< Convert 0-1 to 0-640 (or the widescreen equiv

	const bool			WideScreen()														{ return m_WideScreen; }
	void				WideScreen( const bool bWideScreen )								{ m_WideScreen = bWideScreen; }

#ifdef PLATFORM_PC
	// [Blur2PC] retail Blur PC: the 2D screen is 480 high and as wide as the back buffer's aspect ratio makes it
	// (width * 480 / height), centred on the 640-wide design area (retail C_Simple2D +0x14, set by the UI
	// manager at start-up and after a graphics options change)
	const F32			ScreenWidth( )														{ return m_ScreenWidth; }
	const F32			ScreenLeft( )														{ return (UI_VSCREEN_SIZEX - m_ScreenWidth) * 0.5f; }
	void				SetScreenWidth( const F32 fScreenWidth )							{ m_ScreenWidth = fScreenWidth; }
#else
	/// Get 2D screen width (different when in widescreen)
	const F32			ScreenWidth( )														{ return m_WideScreen ? UI_VSCREEN_SIZEX_WIDE : UI_VSCREEN_SIZEX; }
	/// Get 2D left screen coordinate (not 0 in widescreen)
	const F32			ScreenLeft( )														{ return m_WideScreen ? (UI_VSCREEN_SIZEX - ScreenWidth()) * 0.5f : 0.0f; }
#endif
	/// Get 2D right screen coordinate 
	const F32			ScreenRight( )														{ return ScreenLeft() + ScreenWidth(); }
#ifdef _UI_AUTO_CLIPWINDOW
	/// Setup a panel & layer's clip window - returns an object that automatically resets clip window when it goes out of scope.
	C_UIClipWindow		AddClipWindowAuto( const LayerIndex layer, const F32 x1, const F32 y1, const F32 x2, const F32 y2, const PanelIndex panel = 0 );
	/// Setup a auto clip window with gadget data
	C_UIClipWindow		AddClipWindowAuto( const C_GadgetBaseData* pData );
#endif
	/// Manually set and panel & layer's clip window
	void				AddClipWindow( const LayerIndex layer, const F32 x1, const F32 y1, const F32 x2, const F32 y2, const PanelIndex panel = 0 );
	/// Manually reset and panel & layer's clip window
	void				ResetClipWindow( const LayerIndex layer, const PanelIndex panel = 0 );

	/// Add a sprite to be rendered, Use Addxxxx or CurrentSprite() functions to alter it's defaults
	void				AddSprite( LayerIndex uLayer, const Maths::HVec3& pos, const MathsStorage::Float3& size, const C_UISpriteColours& colours, PanelIndex sPanel = 0, const E_SpriteType type = E_SpriteType_Sprite, const C_RenderState& renderState = C_RenderState::Instance(), const INTPTR uUserData = 0 );
	/// Add a sprite to be rendered, Use Addxxxx or CurrentSprite() functions to alter it's defaults
	void				AddSprite( LayerIndex uLayer, const Maths::HVec3& pos, const F32 fSizeX, const F32 fSizeY, const C_UISpriteColours& colours, PanelIndex sPanel = 0, const E_SpriteType type = E_SpriteType_Sprite, const C_RenderState& renderState = C_RenderState::Instance(), const INTPTR uUserData = 0 );
	/// Add a state control sprite
	void				AddSprite( LayerIndex uLayer, PanelIndex sPanel, const E_SpriteType type, const INTPTR userData = 0 );
	/// Add a stencil control sprite
	void				AddSprite( LayerIndex uLayer, PanelIndex sPanel, const Display::C_StencilState& state );
	/// Get the current sprite in the stack (valid only after AddSprite is called)
	C_Sprite* __restrict	CurrentSprite( )													{ assert(m_SpriteCount); return &m_Sprites[m_SpriteCount-1]; }

	/// CurrentSprite: Add texture parameters : C_TextureParam for a given stage
	void				AddTexUVs( const TextureStage stage, const C_TextureParam& texture );
	/// CurrentSprite: Manually add texture parameters for a given stage
	void				AddTexUVs( const TextureStage stage, const C_TextureHandleIndex texture, const F32 u = 0.0f, const F32 v = 0.0f, const F32 sizeu = 1.0f, const F32 sizev = 1.0f );
	/// CurrentSprite: Rotate (Eular)
	void				AddRotation( const F32 degX, const F32 degY, const F32 degZ );
	/// CurrentSprite: Scale
	void				AddScale( const F32 x, const F32 y, const F32 z = 1.0f );
	/// CurrentSprite: Set the pre-transform  pivot
	void				AddPivot( const F32 x, const F32 y, const F32 z = 0.0f );
	/// CurrentSprite: Set the pre-transform pivot
	void				AddPivot( const Maths::HVec3& pos );
	/// CurrentSprite: Translate the pre-transform pivot
	void				AddOffsetPivot( const F32 x, const F32 y, const F32 z = 0.0f );
	/// CurrentSprite: Inverted translate the pre-transform pivot
	void				AddOffsetPivotNeg( const F32 x, const F32 y, const F32 z = 0.0f );
	/// CurrentSprite: Justify the current sprite's draw position (2D)
	void				AddJustification( const E_Justify justify );
	/// CurrentSprite: Set the pre-transform matrix index
	void				AddPreTransformIndex( const TransformIndex index )									{ assert(m_SpriteCount); CurrentSprite( )->m_PreTransformIndex = index; }
	/// CurrentSprite: Set the post-transform matrix index
	void				AddPostTransformIndex( const TransformIndex index )									{ assert(m_SpriteCount); CurrentSprite( )->m_PostTransformIndex = index; }
	/// setup auto-transform hierarchy for parents - reset on deconstruction
	void				SetAutoTransform( const TransformIndex ti, const Maths::HVec3& transformAutoOffset = Maths::HVec3( 0.0f, 0.0f, 0.0f ) )		{ m_TransformIndexAuto = ti; m_TransformAutoOffset = transformAutoOffset; }
	TransformIndex		GetAutoTransform()																											{ return m_TransformIndexAuto; }	///< @a
	const Maths::HVec3&	GetAutoTransformOffset( ) const														{ return m_TransformAutoOffset; }

	/// does this panel have any sprites to render
	bool				ShouldRender( const PanelIndex panel );
	/// Render an entire panel
	void				RenderPanel( const PanelIndex uPanel );
	/// Render the current layer & panel - this needs to be for every panel and every layer in that panel.
	void				Render( const LayerIndex sLayer, const PanelIndex sPanel );
	/// Internal rendering of TriList or TriString
	void				RenderTriangles( const C_Sprite* pSprite );
	/// Pop a pre-transform index off the pre-transform matrix list
	const TransformIndex				PopPreTransformMatrix( );
	/// Pop a post-transform index off the post-transform matrix list
	const TransformIndex				PopPostTransformMatrix( );
	/// Pop a colour-transform index off the colour matrix list
	const TransformIndex				PopColourTransformMatrix( );
	/// Access
	Maths::HMat43&						PreTransformMatrixPool( U32 index )									{ return m_PreTransformMatrixPool[ index ]; }
	/// Access
	Maths::HMat43&						PostTransformMatrixPool( U32 index )								{ return m_PostTransformMatrixPool[ index ]; }
	/// Get a pre-transform matrix a given index position
	Maths::HMat43&						GetPreTransformMatrix( const TransformIndex matrix_index, const bool reserved = false );
	/// Get a post-transform matrix a given index position
	Maths::HMat43&						GetPostTransformMatrix( const TransformIndex matrix_index, const bool reserved = false );
	/// Get a colour-transform matrix a given index position
	Maths::HMat43&						GetColourTransformMatrix( const TransformIndex matrix_index );
	/// Get offset for a given justification and corner
	inline const Maths::HVec3&			GetJustifiedCorner( const E_Justify type, const U32 corner, const PanelIndex panel );
	/// Get offset for a given justification and corner
	inline const Maths::HVec3&			GetJustifiedCorner( const E_Justify type, const U32 corner, const bool flipped );
	/// Get offset for a given justification type and panel (y is flipped in 3D)
	inline const Maths::HVec3			GetJustificationOffset( const E_Justify type, const PanelIndex panel );
	/// Add an alternate texture (movie, render target etc)
	const C_TextureHandleIndex			SetAlternateTexture( I_AlternateTexture* alternate_texture )	{ m_AlternateTextures.PushBack( alternate_texture ); return C_TextureHandleIndex( ((S16)m_AlternateTextures.Size()-1), true ); }
	/// Find an alternative texture by name
	const C_TextureHandleIndex			AlternateTextureFindByName( const char *name, const bool check_valid =false );
	/// Call once all calls to SetAlternateTexture() have been made to build the UIEnums.TextureType table 
	void								BuildLuaEnumsForAlternateTextures( lua_State* pState );
	/// Align x & y coordinates to screen pixel boundary (only in 2D)
	void								PixelBoundary( const bool doX, const bool doY, F32& x, F32& y, F32& sizeX, F32& sizeY );
	/// Accessor : Get the UI renderer object
	C_Renderer&							GetRenderer( )													{ assert(m_pRenderer); return *m_pRenderer; }
	/// Accessor : Get the gradient alternate texture object (C_TextureParam)
	C_GradientAlternateTexture&			GetGradient( )													{ assert(m_pGradientTexture); return *m_pGradientTexture; }
	/// Description of gradient texture for use with AddTexUVs if 
	const C_TextureParam&				GetGradientTextureParam( )										{ return m_GradientTextureParam; }
	/// Get the current stacked renderstate
	C_StackedRenderState&				CurrentStackedRenderState( )									{ return m_StackedRenderStates.Size() == 0 ? m_DefaultStackedRenderState : *m_StackedRenderStates.Back(); }
	/// Set the default renderstate for the start of each panel / layer
	void								SetDefaultStackedRenderState(const C_StackedRenderState& state );
	// Set the default ZBuffer state
	void								SetZEnable( const bool enable );
	
	F32									Convert01DepthToUI( const F32 fDepth01 )						{ return ( fDepth01 * ( m_fFarZ - m_fNearZ ) ); }

	U32									CurrentStencilRef()												{ U32 ref = m_StencilRef; m_StencilRef++; return ref; }

	void								ResetStencilRef()												{ m_StencilRef = 0; }

	void								SetCurrentStackedRenderState( );

	TLayerPanelSpriteList&				GetSpriteList()
										{
											return m_LayerPanelSprites;
										}

private:
	void								SetAlphaTestEnable( const Display::E_Bool bEnable ); 
	void								SetCullMode( const C_StackedRenderState& set );
	void								SetClipWindow( const C_StackedRenderState& set );
	void								SetColourWriteEnable( const C_StackedRenderState& set );
	void								SetDepthBufferState( const C_StackedRenderState& set );
	void								SetStencilBufferState( const C_StackedRenderState& set );

										/// Clamps layer and panel between the min and max layer/panel range.
	void								ValidatePanelAndLayer( LayerIndex& uLayer, PanelIndex& uPanel );

public:
	static const Maths::HVec3					m_JustificationOffsets[E_Justify_Count];
	static const Maths::HVec3					m_JustificationOffsets_Flipped[E_Justify_Count];
	static const Maths::HVec3					m_QuadCorners3[4];
	static const Maths::HVec3					m_QuadCorners3_Flipped[4];

private:
	static const U32							m_PreMatrixPoolSize		= 128;	// * 16
	static const U32							m_PostMatrixPoolSize	= 4000;	// * 16
	static const U32							m_ColourMatrixPoolSize	= 4;	// * 16
	static const U32							m_MaxAlternateTextures	= 64;

	TLayerPanelSpriteList						m_LayerPanelSprites;

	C_Renderer*									m_pRenderer;
	bool										m_WideScreen;
#ifdef PLATFORM_PC
	F32											m_ScreenWidth;		// [Blur2PC] see ScreenWidth()
#endif
	U32											m_StencilRef;
	E_SpriteType								m_ePreviousSpriteType;

	// clip dimensions
	F32											m_fClipWidth;
	F32											m_fClipHeight;
	F32											m_fNearZ;
	F32											m_fFarZ;

	// transform matrix
	Maths::HMat44								m_TransformMatrix;
	Maths::HMat44								m_TransformMatrix2D;			// default - set when calling SetClipSize2D()

	Maths::HVec4								m_ViewPosition;

	TransformIndex								m_TransformIndexAuto;
	Maths::HVec3								m_TransformAutoOffset;

	bool										m_bUsing2D;

	// Sprites
	C_Sprite*									m_Sprites;
	U32											m_MaxSprites;
	U32											m_NewMaxSprites;
	U32											m_SpriteCount;
	bool										m_SpritesBatched;
#ifdef PLATFORM_PC
	U32											m_Blur2PCListGeneration;	// [Blur2PC] see Blur2PCListGeneration
#endif

	Btl::Vector< C_StackedRenderState *>		m_StackedRenderStates;
	Btl::DynamicPool< C_StackedRenderState >	m_StackedRenderStatePool;
	C_StackedRenderState						m_DefaultStackedRenderState;

	// custom panel and layer count
	U8											m_uLayerCount;
	U8											m_uPanelCount;
	S32											m_sPanelUsage3D;	// which panels are the 3D ones - bitwise

	Maths::HVec3								m_JustifiedCorner[E_Justify_Count][4];
	Maths::HVec3								m_JustifiedCorner_Flipped[E_Justify_Count][4];


	TransformIndex								m_PreTransformPoolIndex;
	TransformIndex								m_PostTransformPoolIndex;
	TransformIndex								m_ColourMatrixPoolIndex;

	C_TextureParam								m_GradientTextureParam;
	C_GradientAlternateTexture*					m_pGradientTexture;

	Btl::Vector< C_Renderer::C_Light >			m_Lighting;
	F32											m_UserGamma;

	Btl::Vector< Maths::HMat43 >				m_PreTransformMatrixPool;
	Btl::Vector< Maths::HMat43 >				m_PostTransformMatrixPool;
	Btl::Vector< Maths::HMat43 >				m_ColourMatrixPool;
	Btl::Vector< Maths::HMat44 >				m_PanelTransformMatrix;

	Btl::Vector< I_AlternateTexture*, Btl::VectorParameters_Static< m_MaxAlternateTextures > >	m_AlternateTextures;
};

/// The global singleton instace
extern C_Simple2D		gSimple2D;

};

// Include inline implementations.
#include "Simple2d.inl"

#endif // SIMPLE2D_H
