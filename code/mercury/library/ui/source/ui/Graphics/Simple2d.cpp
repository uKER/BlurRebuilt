#include "stdafx.h"

#include "ui/Game/UILua.h"
#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/Simple2d.h"
#ifdef PLATFORM_PC
#include <vector>	// [Blur2PC] anchored sprite transforms
#endif
#include "ui/Graphics/UITextureManager.h"
#include "ui/Graphics/Movie.h"
//#include "ui/Graphics/PostProcess.h"
#include "ui/Graphics/Font/UIPolygonFont.h"
#include "ui/Graphics/UIGraphicalInterface.h"
#include "ui/UiInterface.h"

#ifndef CFG_GOLD
#include "ui/Illustrator/UIAdobeScene.h"
// For stats
#include "ui/Graphics/UIParticle.h"
#include "ui/Game/Gadgets/ButtonController.h"
#endif

#include "UI/UIDefines.h"

#if defined(_WIN64) // [Blur2PC][x64] m_UserData and m_Callback are pointer-sized
BIZ_STATIC_ASSERT(sizeof(UI::C_Sprite) == 136);
#else
BIZ_STATIC_ASSERT(sizeof(UI::C_Sprite) == 128); // Must be 128 bytes in size
#endif


#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// Display annotations used by the UI - these probably could / should be move to a display file
XT_TYPE( Display::E_CmpFunc, "DisplayCompareFunction" )
	XT_OPTION( Display::E_CmpFunc_Default, "Default" )
	XT_OPTION( Display::E_CmpFunc_Never, "Never" )
	XT_OPTION( Display::E_CmpFunc_Less, "Less" )
	XT_OPTION( Display::E_CmpFunc_Equal, "Equal" )
	XT_OPTION( Display::E_CmpFunc_LessEqual, "LessEqual" )
	XT_OPTION( Display::E_CmpFunc_Greater, "Greater" )
	XT_OPTION( Display::E_CmpFunc_NotEqual, "NotEqual" )
	XT_OPTION( Display::E_CmpFunc_GreaterEqual, "GreaterEqual" )
	XT_OPTION( Display::E_CmpFunc_Always, "Always" )
XT_END_TYPE

XT_TYPE( Display::E_StencilOp, "DisplayStencilOp" )
	XT_OPTION( Display::E_StencilOp_Keep, "Keep" )
	XT_OPTION( Display::E_StencilOp_Zero, "Zero" )
	XT_OPTION( Display::E_StencilOp_Replace, "Replace" )
	XT_OPTION( Display::E_StencilOp_IncrSat, "IncrSat" )
	XT_OPTION( Display::E_StencilOp_DecrSat, "DecrSat" )
	XT_OPTION( Display::E_StencilOp_Invert, "Invert" )
	XT_OPTION( Display::E_StencilOp_Incr, "Incr" )
	XT_OPTION( Display::E_StencilOp_Decr, "Decr" )
XT_END_TYPE

XT_TYPE( Display::C_StencilState, "DisplayStencilState" )
	XT_FIELD( m_StencilEnable, "Enable" )
	XT_FIELD( m_StencilFunc, "Func" )
	XT_FIELD( m_StencilRef, "Ref" )
	XT_FIELD( m_StencilMask, "Mask" )
	XT_FIELD( m_StencilWriteMask, "WriteMask" )
	XT_FIELD( m_StencilFail, "Fail" )
	XT_FIELD( m_StencilDepthFail, "DepthFail" )
	XT_FIELD( m_StencilDepthPass, "DepthPass" )
XT_END_TYPE

XT_TYPE( Display::E_Bool, "DisplayBool" )
	XT_OPTION( Display::E_Bool_False, "False" )
	XT_OPTION( Display::E_Bool_True, "True" )
XT_END_TYPE

XT_TYPE( Display::E_CullMode, "DisplayCullMode" )
	XT_OPTION( Display::E_CullMode_None, "None" )
	XT_OPTION( Display::E_CullMode_Cw, "CW" )
	XT_OPTION( Display::E_CullMode_Ccw, "CCW" )
XT_END_TYPE

XT_TYPE( UI::E_UiBlendMode, "UIBlendMode" )
	XT_OPTION( UI::E_UiBlendMode_Solid,				"Solid" )
	XT_OPTION( UI::E_UiBlendMode_SolidAlphaTest,	"SolidAlphaTest" )
	XT_OPTION( UI::E_UiBlendMode_SolidAlphaMask,	"SolidAlphaMask" )
	XT_OPTION( UI::E_UiBlendMode_Alpha,				"Alpha" )
	XT_OPTION( UI::E_UiBlendMode_AlphaPreMultiply,	"AlphaPreMultiply" )
	XT_OPTION( UI::E_UiBlendMode_Additive,			"Additive" )
	XT_OPTION( UI::E_UiBlendMode_AlphaAdditive,		"AlphaAdditive" )
	XT_OPTION( UI::E_UiBlendMode_AlphaSubtractive,	"AlphaSubtractive" )
	XT_OPTION( UI::E_UiBlendMode_Subtractive,		"Subtractive" )
	XT_OPTION( UI::E_UiBlendMode_Multiply,			"Multiply" )
	// Extended UI modes
	XT_OPTION( UI::E_UiBlendMode_ExtMultiplyAdd,	"ExtMultiplyAdd" )
XT_END_TYPE

XT_TYPE( UI::C_Simple2D::C_StackedRenderState, "UIStackedRenderState" )
	XT_FIELD( m_ClipEnabled, "clip_enabled" )
	XT_FIELD( m_ClipPos, "clip_pos" )
	XT_FIELD( m_ClipSize, "clip_size" )
	XT_FIELD( m_ColourWriteEnable, "colour_write_enable" )
	XT_FIELD( m_DepthEnabled, "depth_enabled" )
	XT_FIELD( m_DepthWriteEnabled, "depth_write_enabled" )
	XT_FIELD( m_DepthCompareFunc, "depth_compare_function" )
	XT_FIELD( m_CullMode, "cull_mode" )
	XT_FIELD( m_StencilState, "stencil_state" )
XT_END_TYPE

XT_TYPE( Display::E_ColourWriteEnable, "DisplayColourWriteEnable" )
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG( Display::E_ColourWriteEnable_Red, "Red"  )
	XT_FLAG( Display::E_ColourWriteEnable_Green, "Green" )
	XT_FLAG( Display::E_ColourWriteEnable_Blue, "Blue" )
	XT_FLAG( Display::E_ColourWriteEnable_Alpha, "Alpha" )
XT_END_TYPE

Debug::Bool			 g_UiDebugDoRender( "UI.Render", true );
DECLARE_DEBUG_FLOAT( g_UiLoadingTint, "UI.LoadingTexture.Tint", 0.25f, Debug::Range(0.0f,1.0f) );
DECLARE_DEBUG_FLOAT( g_UiLoadingPower, "UI.LoadingTexture.Pow", 1.5f, Debug::Range(0.0f,4.0f) );
DECLARE_DEBUG_FLOAT( g_UiLoadingDarken, "UI.LoadingTexture.Darken", 0.25f, Debug::Range(0.0f,1.0f) );
DECLARE_DEBUG_FLOAT( g_UiLoadingVig, "UI.LoadingTexture.Vig", 5.0f, Debug::Range(0.0f,16.0f) );

#ifndef CFG_GOLD
DEBUG_ENUM(UI::E_Justify)
	DE_OPTION(UI::E_Justify_TopLeft,		"TopLeft")
	DE_OPTION(UI::E_Justify_TopCentre,		"TopCentre")
	DE_OPTION(UI::E_Justify_TopRight,		"TopRight")
	DE_OPTION(UI::E_Justify_MiddleLeft,		"MiddleLeft")
	DE_OPTION(UI::E_Justify_MiddleCentre,	"MiddleCentre")
	DE_OPTION(UI::E_Justify_MiddleRight,	"MiddleRight")
	DE_OPTION(UI::E_Justify_BottomLeft,		"BottomLeft")
	DE_OPTION(UI::E_Justify_BottomCentre,	"BottomCentre")
	DE_OPTION(UI::E_Justify_BottomRight,	"BottomRight")
END_DEBUG_ENUM

Debug::Float				g_UiDebugColourMatrixRotate("UI.ColourMatrix.Rotate", 90.0f, Debug::Range(0.0f, 360.0f));
Debug::Float				g_UiDebugColourMatrixScale("UI.ColourMatrix.Scale", 1.0f, Debug::Range(0.1f, 640.0f));
Debug::Float				g_UiDebugColourMatrixTransX("UI.ColourMatrix.TransX", 0.0f, Debug::Range(-833.0f, 833.0f));
Debug::Float				g_UiDebugColourMatrixTransY("UI.ColourMatrix.TransY", 0.0f, Debug::Range(-480.0f, 480.0f));
Debug::Int					g_UiDebugPolygonShaderGradientParam( "UI.ColourMatrix.PolygonParam", 0, Debug::Range(0,8));
Debug::Bool					g_UiDebugColourMatrixActive("UI.ColourMatrix.Active", false);

Debug::Bool					g_UiDebugShowClipWindowBounds("UI.Render.ShowClipWindowBounds",false);
Debug::Bool					g_UiDebugShowShowStatistics("UI.ShowStatistics", false);
Debug::Bool					g_UiDebugShowTransformUsage("UI.ShowTransformUsage", false );
Debug::Bool					g_UiDebugShowSafeArea("UI.ShowSafeArea", false, Debug::Persist());

Debug::Bool					g_UiDebugGraphicActive("UI.DebugGraphic.Active", false);
Debug::Bool					g_UiDebugGraphicShowName("UI.DebugGraphic.ShowName", true);
Debug::Enum<UI::E_Justify>	g_UiDebugGraphicJustify("UI.DebugGraphic.Justify", UI::E_Justify_MiddleCentre );
Debug::Int					g_UiDebugGraphicTexture("UI.DebugGraphic.TextureIndex", 0, Debug::Range(-10,256));
Debug::Float				g_UiDebugGraphic_px("UI.DebugGraphic.PosX", 320.0f, Debug::Range(-100.0f, 953.0f));
Debug::Float				g_UiDebugGraphic_py("UI.DebugGraphic.PosY", 240.0f, Debug::Range(-100.0f, 580.0f));
Debug::Float				g_UiDebugGraphic_pz("UI.DebugGraphic.PosZ", 0.0f, Debug::Range(-50.0f, 50.0f));
Debug::Float				g_UiDebugGraphic_sx("UI.DebugGraphic.SizeX", 256.0f, Debug::Range(0.0f, 1024.0f));
Debug::Float				g_UiDebugGraphic_sy("UI.DebugGraphic.SizeY", 256.0f, Debug::Range(0.0f, 1024.0f));
Debug::Float				g_UiDebugGraphic_scale("UI.DebugGraphic.Scale", 0.666f, Debug::Range(0.0f, 8.0f));
Debug::Float				g_UiDebugGraphic_tpu("UI.DebugGraphic.TextureU", 0, Debug::Range(0.0f, 1.0f));
Debug::Float				g_UiDebugGraphic_tpv("UI.DebugGraphic.TextureV", 0, Debug::Range(0.0f, 1.0f));
Debug::Float				g_UiDebugGraphic_tsu("UI.DebugGraphic.TextureUSize", 1.0f, Debug::Range(0.0f, 1.0f));
Debug::Float				g_UiDebugGraphic_tsv("UI.DebugGraphic.TextureVSize", 1.0f, Debug::Range(0.0f, 1.0f));
Debug::Colour				g_UiDebugGraphic_Colour("UI.DebugGraphic.Colour", 1.0f, 1.0f, 1.0f, 1.0f);

Debug::Bool					g_UiDebugStencilActive("UI.Stencil.Active", false );
Debug::Int					g_UiDebugStencilValue("UI.Stencil.Value", 0, Debug::Range( 0, 128 ) );
Debug::Colour				g_UiDebugStencilColour("UI.Stencil.Colour", 0.2f, 1.0f, 1.0f, 0.5f );

Debug::Bool					g_UiDebugLightEnable( "UI.Light.EnableDebug", false );
Debug::Colour				g_UiDebugLightColourFrom( "UI.Light.ColourFrom", 0.0f, 0.0f, 0.0f, 1.0f );
Debug::Colour				g_UiDebugLightColourTo( "UI.Light.ColourTo", 0.0f, 0.0f, 0.0f, 0.25f );
Debug::Float				g_UiDebugLightFallOff( "UI.Light.FallOff", 60.0f, Debug::Range( 0.0f, 500.0f ) );
Debug::Float				g_UiDebugLightPosZ( "UI.Light.PosZ", -45.0f, Debug::Range( 0.0f, -500.0f ) );

Debug::Bool					g_UiDebugBlendModeEnable("UI.BlendMode.DebugEnable", false);
Debug::Int					g_UiDebugBlendModeSrcColour("UI.BlendMode.SrcColour", (S32)Display::E_Blend_SrcAlpha,		Debug::Range( (S32)Display::E_Blend_Zero, (S32)Display::E_Blend_Unknown ) );
Debug::Int					g_UiDebugBlendModeDstColour("UI.BlendMode.DstColour", (S32)Display::E_Blend_InvSrcAlpha,	Debug::Range( (S32)Display::E_Blend_Zero, (S32)Display::E_Blend_Unknown ) );
Debug::Int					g_UiDebugBlendModeSrcAlpha("UI.BlendMode.SrcAlpha", (S32)Display::E_Blend_SrcAlpha,			Debug::Range( (S32)Display::E_Blend_Zero, (S32)Display::E_Blend_Unknown ) );
Debug::Int					g_UiDebugBlendModeDstAlpha("UI.BlendMode.DstAlpha", (S32)Display::E_Blend_InvSrcAlpha,		Debug::Range( (S32)Display::E_Blend_Zero, (S32)Display::E_Blend_Unknown ) );
#endif

namespace UI
{

// Singleton action
C_Simple2D		gSimple2D;
C_RenderState	C_RenderState::m_sInstance;

#ifndef CFG_GOLD
Maths::HVec3 g_PerkStashOffset;
#endif

// -----------------
// C_UIAutoTransform
// -----------------
C_UIAutoTransform::C_UIAutoTransform( C_GadgetBase* pObject )
{
	if ( !pObject )
		return;
	
	Maths::HVec3 offset( 0.0f, 0.0f, 0.0f );

	m_PrevTi		= gSimple2D.GetAutoTransform();
	m_PrevOffset	= gSimple2D.GetAutoTransformOffset();

	// calculate the transform
	TransformIndex ti;
	pObject->CalcTransformHierarchy( ti );

	if( ti )
	{
		// shift - as offset is stored in the transform matrix
		offset = -Maths::HVec3( pObject->m_Position );
	}
	gSimple2D.SetAutoTransform( ti, offset );
}


C_UIAutoTransform::~C_UIAutoTransform()
{
	gSimple2D.SetAutoTransform( m_PrevTi, m_PrevOffset );
}

#ifdef _UI_AUTO_CLIPWINDOW
// --------------
// C_UIClipWindow
// --------------

C_UIClipWindow::C_UIClipWindow()
	: m_bActive( false ),
	m_Layer( 0 ),
	m_Panel( 0 )
{
	//
}

C_UIClipWindow::C_UIClipWindow( const LayerIndex layer, const PanelIndex panel, const F32 x1, const F32 y1, const F32 x2, const F32 y2 )
	: m_bActive( false ),
	m_Layer( layer ),
	m_Panel( panel )
{
	if ( ( x1 == -1.0f ) || ( y1 == -1.0f ) || ( x2 == -1.0f ) || ( y2 == -1.0f ) )
		return;

	m_bActive = true;

	//if ( gSimple2D.GetAutoTransform() != C_Sprite::m_MatrixIndexUnused )
		//LOGWARN( g_Log, DBSTR "ClipWindow added _after_ auto-transform - it'll probs go screwy" );

	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x1, y1, 0.0f ), x2 - x1, y2 - y1, 0x00000000, m_Panel, E_SpriteType_SetClipWindow, C_RenderState::Instance(), 1 );

#ifndef CFG_GOLD
	if ( g_UiDebugShowClipWindowBounds )
	{
		UI::gSimple2D.AddSprite( layer, Maths::HVec3( x1, y1, 0.0f ), x1 + x2, y1 + y2, 0x55ffffff, m_Panel );
		UI::gSimple2D.AddTexUVs( 0, UI::gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
	}
#endif
}

C_UIClipWindow::~C_UIClipWindow()
{
	if ( !m_bActive )
		return;

	gSimple2D.AddSprite( m_Layer, Maths::HVec3( gSimple2D.ScreenLeft(), 0.0f, 0.0f ), gSimple2D.ScreenWidth(), UI_VSCREEN_SIZEY, 0x00000000, m_Panel, E_SpriteType_SetClipWindow );
}

C_UIClipWindow	C_Simple2D::AddClipWindowAuto( const C_GadgetBaseData* pData )
{
	return C_UIClipWindow( pData->m_Layer, pData->m_Panel, pData->m_ClipPos.x, pData->m_ClipPos.y, pData->m_ClipPos.x + pData->m_ClipSize.x, pData->m_ClipPos.y + pData->m_ClipSize.y );
}


inline C_UIClipWindow C_Simple2D::AddClipWindowAuto( const LayerIndex layer, const F32 x1, const F32 y1, const F32 x2, const F32 y2, const PanelIndex panel )
{
	return C_UIClipWindow( layer, panel, x1, y1, x2, y2 );
}
#endif // _UI_AUTO_CLIPWINDOW

// --------------
// C_UIAutoStencil
// --------------
C_UIAutoStencil::C_UIAutoStencil( const LayerIndex layer, const PanelIndex panel, const Display::C_StencilState& op ) :
	m_Layer( layer ),
	m_Panel( panel )
{
	m_bActive = (op.m_StencilEnable == Display::E_Bool_True);

	if( !m_bActive)
		return;

	gSimple2D.AddSprite( layer, panel, op );
}

C_UIAutoStencil::~C_UIAutoStencil( )
{
	if( !m_bActive )
		return;

	// Maybe this should be static in this class?
	Display::C_StencilState op;
	op.m_StencilEnable		= Display::E_Bool_False;
	op.m_StencilFunc		= Display::E_CmpFunc_Always;
	op.m_StencilRef			= 0xffffffff;
	op.m_StencilMask		= 0xffffffff;
	op.m_StencilWriteMask	= 0xffffffff;
	op.m_StencilFail		= Display::E_StencilOp_Keep;
	op.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	op.m_StencilDepthPass	= Display::E_StencilOp_Keep;

	gSimple2D.AddSprite( m_Layer, m_Panel, op );
}

// Justification
const Maths::HVec3 C_Simple2D::m_JustificationOffsets[E_Justify_Count] = 
{
	Maths::HVec3(0.0f, 0.0f, 0.0f),		// Left
	Maths::HVec3(0.5f, 0.0f, 0.0f),		// Centre
	Maths::HVec3(1.0f, 0.0f, 0.0f),		// Right
	Maths::HVec3(0.0f, 0.5f, 0.0f),		// MiddleLeft
	Maths::HVec3(0.5f, 0.5f, 0.0f),		// MiddleCentre
	Maths::HVec3(1.0f, 0.5f, 0.0f),		// MiddleRight
	Maths::HVec3(0.0f, 1.0f, 0.0f),		// BottomLeft
	Maths::HVec3(0.5f, 1.0f, 0.0f),		// BottomCentre
	Maths::HVec3(1.0f, 1.0f, 0.0f),		// BottomRight
	Maths::HVec3(0.0f, 0.0f, 0.0f),		// None
};
const Maths::HVec3 C_Simple2D::m_JustificationOffsets_Flipped[E_Justify_Count] = 
{
	Maths::HVec3(0.0f, 1.0f, 0.0f),		// Left
	Maths::HVec3(0.5f, 1.0f, 0.0f),		// Centre
	Maths::HVec3(1.0f, 1.0f, 0.0f),		// Right
	Maths::HVec3(0.0f, 0.5f, 0.0f),		// MiddleLeft
	Maths::HVec3(0.5f, 0.5f, 0.0f),		// MiddleCentre
	Maths::HVec3(1.0f, 0.5f, 0.0f),		// MiddleRight
	Maths::HVec3(0.0f, 0.0f, 0.0f),		// BottomLeft
	Maths::HVec3(0.5f, 0.0f, 0.0f),		// BottomCentre
	Maths::HVec3(1.0f, 0.0f, 0.0f),		// BottomRight
	Maths::HVec3(0.0f, 1.0f, 0.0f),		// None
};
// Quad corner order
const Maths::HVec3 C_Simple2D::m_QuadCorners3[4] = 
{
	Maths::HVec3(0.0f, 0.0f, 0.0f),
	Maths::HVec3(1.0f, 0.0f, 0.0f),
	Maths::HVec3(1.0f, 1.0f, 0.0f),
	Maths::HVec3(0.0f, 1.0f, 0.0f),
};
const Maths::HVec3 C_Simple2D::m_QuadCorners3_Flipped[4] = 
{
	Maths::HVec3(0.0f, 1.0f, 0.0f),
	Maths::HVec3(1.0f, 1.0f, 0.0f),
	Maths::HVec3(1.0f, 0.0f, 0.0f),
	Maths::HVec3(0.0f, 0.0f, 0.0f),
};

C_Simple2D::C_Simple2D()
{
	m_pRenderer						= NULL;
	m_StencilRef					= 0;
#ifdef PLATFORM_PC
	m_ScreenWidth					= UI_VSCREEN_SIZEX;	// [Blur2PC] replaced from the back buffer by the UI manager
#endif

	m_sPanelUsage3D = 0;
#ifdef PLATFORM_PC
	m_Blur2PCListGeneration = 0;	// [Blur2PC]
#endif
}

C_Simple2D::~C_Simple2D()
{
	Exit();
}

void C_Simple2D::Exit(void)
{
	if ( m_pRenderer )
	{
		m_pRenderer->Destroy();
		SAFE_DELETE_PTR( m_pRenderer );
	}
	// Clear stack
	while( m_StackedRenderStates.Size() > 0 )
	{
		C_StackedRenderState*p = m_StackedRenderStates.Back();
		m_StackedRenderStates.PopBack();

		m_StackedRenderStatePool.Free( p );
	}
	// These should be handled by 'you'
	/*
	for(S32 i = 0; i < m_AlternateTextures.Size(); i++)
	{
		SAFE_DELETE_PTR( m_AlternateTextures[i] );
	}
	*/
	m_AlternateTextures.Clear();

	SAFE_DELETE_ARY( m_Sprites );
}

void C_Simple2D::ReAllocSprites(const U32 sprite_alloc_cnt)
{
	m_NewMaxSprites = sprite_alloc_cnt;
}

void C_Simple2D::Init( lua_State* pState, const U32 sprite_alloc_cnt, const U32 pre_matrix_cnt, const U32 post_matrix_cnt, const U32 colour_matrix_cnt )
{
	MEMORY_TRACKER_PUSH("ui.simple2d");

//	m_Sprites = new C_Sprite[sprite_alloc_cnt];
	m_Sprites = (C_Sprite *)BizAlignedMalloc( sizeof( C_Sprite ) * sprite_alloc_cnt, 128 );
	m_NewMaxSprites = m_MaxSprites = sprite_alloc_cnt;
	assert(m_Sprites);

	m_PreTransformMatrixPool.Resize( pre_matrix_cnt );
	m_PostTransformMatrixPool.Resize( post_matrix_cnt );
	m_ColourMatrixPool.Resize( colour_matrix_cnt );

	m_WideScreen = UiInterface()->IsWideScreen();

	// Create the default instance
	C_RenderState defaultInstance;
	defaultInstance.Init();
	C_RenderState::Instance( defaultInstance );

	m_SpritesBatched			= false;
	m_SpriteCount				= 0;
	m_PreTransformPoolIndex		= 1;
	m_PostTransformPoolIndex	= 0;
	m_ColourMatrixPoolIndex		= 0;

	m_ePreviousSpriteType=E_SpriteType_Sprite;

	assert(!m_pRenderer);
	m_pRenderer = new C_Renderer();
	assert( m_pRenderer );

	m_pRenderer->Create();

	// These are set to identity to avoid branching in the inner loop of the Render code.
	m_PostTransformMatrixPool[ 0 ].SetIdentity();	// Ident
	m_PreTransformMatrixPool[ 0 ].SetIdentity();	// Ident
	m_PreTransformMatrixPool[ 1 ].SetIdentity();	// Font italics
	m_ColourMatrixPool[ 0 ].SetIdentity();			// Ident

	// Build the corner / justification table
	for(U32 i = 0; i < E_Justify_Count; i++)
		for(U32 j = 0; j < 4; j++)
		{
			m_JustifiedCorner[i][j] = (m_QuadCorners3[j] - m_JustificationOffsets[i]);
			m_JustifiedCorner_Flipped[i][j] = (m_QuadCorners3_Flipped[ j ] - m_JustificationOffsets_Flipped[ i ]);
		}

#ifndef CFG_GOLD
	if( sizeof(C_Sprite) > 128 + (sizeof(void*) - 4) * 2 )	// [Blur2PC][x64] see the static assert
	{
		size_t c_sprite_size = sizeof(C_Sprite);
		LOGWARN( g_Log, "C_Sprite = %d\n", c_sprite_size );
	}
#endif

#if 0
	m_pGradientTexture = new C_GradientAlternateTexture( 128, 64 );
	if(m_pGradientTexture)
	{
		m_GradientTextureParam.m_Position.u = 0.0f;
		m_GradientTextureParam.m_Position.v = 0.0f;
		m_GradientTextureParam.m_Size.u = 1.0f;
		m_GradientTextureParam.m_Size.v = 1.0f;
		m_GradientTextureParam.m_TextureID = SetAlternateTexture( m_pGradientTexture );
#ifdef CFG_DEBUG
		// Test data - todo get this from Illustrator
		{
			C_UIColour colours[ 3 ] = 
			{
				0xff00ff00,
				0xffffff00,
				0xffff2080,
			};
			F32 positions[] = 
			{
				0.0f,
				0.21f,
				1.0f,
			};
			m_pGradientTexture->Build(m_pGradientTexture->PopIndex(), colours, positions, lengthof(positions));
		}
#endif		
	}
	else
	{
		LOGERROR( g_Log, DBSTR "Failed to create C_GradientAlternateTexture");
	}
#endif

	// Set default stacked render state
	m_DefaultStackedRenderState.m_CullMode							= Display::E_CullMode_Ccw;
	m_DefaultStackedRenderState.m_ClipEnabled						= Display::E_Bool_False;
	m_DefaultStackedRenderState.m_ClipPos							= XtVec2( 0, 0 );
	m_DefaultStackedRenderState.m_ClipSize							= XtVec2( 1280.0f, 720.0f );
	m_DefaultStackedRenderState.m_ColourWriteEnable					= Display::E_ColourWriteEnable_All;
	m_DefaultStackedRenderState.m_DepthEnabled						= Display::E_Bool_True;
	m_DefaultStackedRenderState.m_DepthCompareFunc					= Display::E_CmpFunc_LessEqual;
	m_DefaultStackedRenderState.m_DepthWriteEnabled					= Display::E_Bool_True;
	m_DefaultStackedRenderState.m_StencilState.m_StencilEnable		= Display::E_Bool_False;
	m_DefaultStackedRenderState.m_StencilState.m_StencilFunc		= Display::E_CmpFunc_Always;
	m_DefaultStackedRenderState.m_StencilState.m_StencilRef			= 0;
	m_DefaultStackedRenderState.m_StencilState.m_StencilMask		= 0xffffffff;
	m_DefaultStackedRenderState.m_StencilState.m_StencilWriteMask	= 0xffffffff;
	m_DefaultStackedRenderState.m_StencilState.m_StencilFail		= Display::E_StencilOp_Keep;
	m_DefaultStackedRenderState.m_StencilState.m_StencilDepthFail	= Display::E_StencilOp_Keep;
	m_DefaultStackedRenderState.m_StencilState.m_StencilDepthPass	= Display::E_StencilOp_Keep;

	SetDefaultLayersAndPanels( pState );

	// #24240 - Initialise the unitialised variables
	m_UserGamma = 2.2f;

	// Clear light params	
	C_Renderer::C_Light l;
	l.Clear();
	m_pRenderer->SetLight( l );

	MEMORY_TRACKER_POP("ui.simple2d");
}

void C_Simple2D::BuildLuaEnumsForAlternateTextures( lua_State* pState )
{
	if(m_AlternateTextures.Size() == 0)
		return;

	lua_getglobal(pState, LUA_ENUM_TABLE_NAME);
	if(lua_isnil(pState,-1))
	{
		LOGFATAL( g_Log,  "%s table not found", LUA_ENUM_TABLE_NAME );
		lua_pop(pState,1);
		return;
	}

	lua_pushstring(pState, "TextureType");
	lua_newtable(pState);

	for( S32 i = 0; i < m_AlternateTextures.Size(); i++)
	{
		Lua::AddDataMember(pState, m_AlternateTextures[i]->Name(), -(i+1));
	}
	lua_rawset(pState,-3);

	lua_pop(pState,1);
}

void C_Simple2D::SetDefaultLayersAndPanels( lua_State* pState )
{
	const char* pUIDefaultLayerNames[ E_SpriteLayer_Count ] =
	{
		"BACKGROUND",
		"LAYER_1",
		"LAYER_2",
		"LAYER_3",
		"LAYER_4",
		"TOP",
	};
	const char* pUIDefaultPanelNames[ E_SpritePanel_Count ] =
	{
		"MAIN",
		"_3D0",
		"_3D1",
		"_2D1",
		"_RT0",
		"_RT1",
		"_RT2",
		"_VP0",
		"_VP1",
		"_VP2",
		"_VP3",
	};

	SetLayersAndPanels( pState, pUIDefaultLayerNames, lengthof( pUIDefaultLayerNames ), pUIDefaultPanelNames, lengthof( pUIDefaultPanelNames ) );

	// set default 3D panel usage
	AddPanelUsage3D( E_SpritePanel_3D0 );
	AddPanelUsage3D( E_SpritePanel_3D1 );
}

void C_Simple2D::SetLayersAndPanels( lua_State* pState, const char** ppLayerNames, const U8 uLayerCount, const char** ppPanelNames, const U8 uPanelCount )
{
	lua_getglobal( pState, LUA_ENUM_TABLE_NAME );

	if ( lua_isnil( pState, -1 ) )
	{
		LOGFATAL( g_Log,  "%s table not found", LUA_ENUM_TABLE_NAME );
		lua_pop( pState, 1 );
		return;
	}

	// push over any existing table
	lua_pushstring( pState, LUA_LAYER_TABLE_NAME );
	lua_newtable( pState );
		for ( U32 i = 0; i < uLayerCount; i++ )
			Lua::AddDataMember( pState, ppLayerNames[ i ], i );
	lua_rawset( pState, -3 );

	lua_pushstring( pState, LUA_PANEL_TABLE_NAME );
	lua_newtable( pState );
		for ( S32 i = 0; i < uPanelCount; i++ )
			Lua::AddDataMember( pState, ppPanelNames[ i ], i );
	lua_rawset( pState, -3 );

	lua_pop( pState, 1 );

	// reserve the correct amount of space for the layer and panel lists
	Btl::Vector< Btl::Vector< C_Sprite* > > layerList;
	Btl::Vector< C_Sprite* > panelList;

	m_LayerPanelSprites.Destroy();
	m_LayerPanelSprites.Reserve( uLayerCount );

	for ( U32 i = 0; i < uLayerCount; i++ )
	{
		m_LayerPanelSprites.PushBack( layerList );
		m_LayerPanelSprites[ i ].Reserve( uPanelCount + 1 );	// +1 for the default panel

		for ( S32 j = 0; j < ( uPanelCount + 1 ); j++ )
			m_LayerPanelSprites[ i ].PushBack( panelList );
	}

	m_sPanelUsage3D = 0;
	m_uLayerCount = uLayerCount;
	m_uPanelCount = uPanelCount + 1;

	// Clear out light params
	C_Renderer::C_Light l;
	l.Clear();

	m_Lighting.Destroy();
	m_Lighting.Reserve( uPanelCount );

	for( U32 i = 0; i < uPanelCount; i++ )
		m_Lighting.PushBack( l );

	m_PanelTransformMatrix.Resize( uPanelCount );
}

inline void C_Simple2D::AddClipWindow( const LayerIndex layer, const F32 x1, const F32 y1, const F32 x2, const F32 y2, const PanelIndex panel )
{
	if ( x1 == -1.0f || y1 == -1.0f )
		return;

	//if ( m_TransformIndexAuto != C_Sprite::m_MatrixIndexUnused )
	//LOGWARN( g_Log, DBSTR "ClipWindow added _after_ auto-transform - it'll probs go screwy" );

	AddSprite( layer, Maths::HVec3( x1, y1, 0.0f ), x2 - x1, y2 - y1, 0x00000000, panel, E_SpriteType_SetClipWindow, C_RenderState::Instance(), 1 );

#ifndef CFG_GOLD
	if ( g_UiDebugShowClipWindowBounds )
	{
		UI::gSimple2D.AddSprite( ( layer < TopLayer() ) ? layer+1 : TopLayer(), Maths::HVec3( x1, y1, 0.0f ), x2 - x1, y2 - y1, 0x77888888, panel );
		UI::gSimple2D.AddTexUVs( 0, UI::gUITextureManager.GetTextureByName( "null" ), 0.0f, 0.0f, 1.0f, 1.0f );
	}
#endif
}

#ifdef PLATFORM_PC
// [Blur2PC] see C_Simple2D::Blur2PCSetAnchor
namespace
{
	struct S_Blur2PCAnchor
	{
		const C_GadgetBase*	gadget;
		S32					key;
		bool				rigid;
		Maths::HMat43		built;
	};
	struct S_Blur2PCAnchoredEntry
	{
		U32					anchor;
		TransformIndex		ti;
		Maths::HMat43		original;
	};
	std::vector< S_Blur2PCAnchor >			s_Blur2PCAnchors;
	std::vector< S_Blur2PCAnchoredEntry >	s_Blur2PCAnchored;
	U32										s_Blur2PCAnchorGeneration = 0;
	bool									s_Blur2PCAnchoredSaved = false;	// originals captured for this generation
}

void C_Simple2D::Blur2PCSetAnchor( const C_GadgetBase* pAnchor, const S32 key, const bool rigid, const Maths::HMat43& built )
{
	if( s_Blur2PCAnchorGeneration != m_Blur2PCListGeneration )
	{
		s_Blur2PCAnchorGeneration = m_Blur2PCListGeneration;
		s_Blur2PCAnchors.clear();
		s_Blur2PCAnchored.clear();
		s_Blur2PCAnchoredSaved = false;
	}
	for( size_t i = 0; i < s_Blur2PCAnchors.size(); i++ )
	{
		if( s_Blur2PCAnchors[ i ].gadget == pAnchor )
		{
			s_Blur2PCAnchors[ i ].key = key;
			s_Blur2PCAnchors[ i ].rigid = rigid;
			s_Blur2PCAnchors[ i ].built = built;
			return;
		}
	}
	S_Blur2PCAnchor a = { pAnchor, key, rigid, built };
	s_Blur2PCAnchors.push_back( a );
}

void C_Simple2D::Blur2PCNoteTransform( const C_GadgetBase* pGadget, const TransformIndex ti )
{
	if( s_Blur2PCAnchorGeneration != m_Blur2PCListGeneration || s_Blur2PCAnchors.empty() || s_Blur2PCAnchoredSaved )
		return;
	if( ti == 0 || ti >= m_PostTransformMatrixPool.Size() )
		return;
	for( const C_GadgetBase* p = pGadget; p; p = p->Parent() )
	{
		for( size_t i = 0; i < s_Blur2PCAnchors.size(); i++ )
		{
			if( s_Blur2PCAnchors[ i ].gadget == p )
			{
				S_Blur2PCAnchoredEntry e;
				e.anchor = (U32)i;
				e.ti = ti;
				s_Blur2PCAnchored.push_back( e );
				return;
			}
		}
	}
}

void C_Simple2D::Blur2PCPlaceAnchored( T_Blur2PCResolveAnchor resolve )
{
	if( s_Blur2PCAnchorGeneration != m_Blur2PCListGeneration || s_Blur2PCAnchored.empty() )
		return;
	if( !s_Blur2PCAnchoredSaved )
	{	// the lists are complete by the first render: keep the entries as the UI built them
		for( size_t i = 0; i < s_Blur2PCAnchored.size(); i++ )
			s_Blur2PCAnchored[ i ].original = m_PostTransformMatrixPool[ s_Blur2PCAnchored[ i ].ti ];
		s_Blur2PCAnchoredSaved = true;
	}
	std::vector< Maths::HMat43 > delta( s_Blur2PCAnchors.size() );
	std::vector< bool > valid( s_Blur2PCAnchors.size() );
	for( size_t i = 0; i < s_Blur2PCAnchors.size(); i++ )
	{
		Maths::HMat43 render;
		valid[ i ] = resolve( s_Blur2PCAnchors[ i ].key, render );
		if( !valid[ i ] )
			continue;
		const S_Blur2PCAnchor& a = s_Blur2PCAnchors[ i ];
		if( a.rigid )
			delta[ i ] = a.built.InverseRotTrans() * render;
		else
		{
			delta[ i ].SetIdentity();
			delta[ i ].AW( render.AW() - a.built.AW() );
		}
	}
	for( size_t i = 0; i < s_Blur2PCAnchored.size(); i++ )
	{
		const S_Blur2PCAnchoredEntry& e = s_Blur2PCAnchored[ i ];
		if( !valid[ e.anchor ] )
			m_PostTransformMatrixPool[ e.ti ] = e.original;
		else if( s_Blur2PCAnchors[ e.anchor ].rigid )
			m_PostTransformMatrixPool[ e.ti ] = e.original * delta[ e.anchor ];
		else
		{
			Maths::HMat43 m = e.original;
			m.AW( m.AW() + delta[ e.anchor ].AW() );
			m_PostTransformMatrixPool[ e.ti ] = m;
		}
	}
}
#endif

void C_Simple2D::StartFrame()
{
	// Change for buffer change
	if(m_MaxSprites != m_NewMaxSprites)
	{
#ifndef CFG_GOLD
		// Debug clear out
		memset(m_Sprites, 0xbe, sizeof(C_Sprite)*m_MaxSprites);
#endif

		// kill old
		assert(m_Sprites);
		delete [] m_Sprites;

		// Change size
		LOGDEBUG( g_Log, DBSTR "Re-allocating from %d to %d sprites", m_MaxSprites, m_NewMaxSprites);
		m_MaxSprites = m_NewMaxSprites;

		// Create new
		m_Sprites = new C_Sprite[m_MaxSprites];
		assert(m_Sprites);

#ifndef CFG_GOLD
		// Debug clear out
		memset(m_Sprites, 0xdf, sizeof(C_Sprite)*m_MaxSprites);
#endif
	}

	// PR - need to reset post-transform indices // DG - What are you on about?
//	for ( U32 i = 0; i < m_MaxSprites; i++ )
//		m_Sprites[ i ].m_PostTransformIndex = C_Sprite::m_PostMatrixIndexUnused;

	m_TransformIndexAuto		= 0;
	m_TransformAutoOffset		= Maths::HVec3( 0.0f, 0.0f, 0.0f );

	// Reset counters
	m_SpriteCount				= 0;
	m_PreTransformPoolIndex		= 1; // 1 reserved for italics
	m_PostTransformPoolIndex	= 0;
	m_ColourMatrixPoolIndex		= 0;

	for ( U32 i = 0; i < LayerCount(); i++ )
	{
		for ( U32 j = 0; j < PanelCount(); j++ )
		{
			m_LayerPanelSprites[ i ][ j ].Clear();
		}
	}
	m_SpritesBatched = false;
#ifdef PLATFORM_PC
	++m_Blur2PCListGeneration;	// [Blur2PC]
#endif

	// Reset global reference
	m_StencilRef = 0;
}



const C_TextureHandleIndex C_Simple2D::AlternateTextureFindByName( const char *pName, const bool check_valid )
{
	if( pName != NULL && pName[0] != NULL )
	{
		for( S32 i = 0; i < m_AlternateTextures.Size(); i++)
		{
			if( stricmp( m_AlternateTextures[ i ]->Name().Chars(), pName ) == 0 )
			{
				if( m_AlternateTextures[ i ]->Handle() == NULL && check_valid )
					return C_TextureHandleIndex();

				return C_TextureHandleIndex( i, true );
			}
		}
	}

	return C_TextureHandleIndex();
}





void C_Simple2D::BuildSpriteBatches()
{
	if ( m_SpriteCount == 0 || m_SpritesBatched )
		return;

	PERF_EVENT("C_Simple2D.BuildSpriteBatches");

	//assert( m_SpriteCount < m_MaxSprites );
	C_Sprite* pStartSprite	= NULL;
	C_Sprite* pCurrent		= NULL;

	// Loop through layers.
	for ( S32 layer = 0; layer < m_LayerPanelSprites.Size(); layer++ )
	{
		TPanelSpriteList& panelList = m_LayerPanelSprites[layer];

		// Loop through panels.
		for ( S32 panel = 0; panel < panelList.Size(); panel++ )
		{
			TSpriteList& spriteList = panelList[panel];

			// Reset
			U32 batch = true;
			pStartSprite = NULL;

			// Loop through sprites
			for ( S32 sprite = 0; sprite < spriteList.Size(); sprite++ )
			{
				pCurrent = spriteList[sprite];

				if (pCurrent->m_Type != E_SpriteType_Sprite)
				{
					pStartSprite = 0;
					batch = true;
					continue;
				}

				if ( !pStartSprite )
				{
					pStartSprite = pCurrent;
					batch = true;
					continue;
				}

				// Check textures.
				for ( U32 i = 0; i < C_Sprite::m_NumTextureStages; i++ )
				{
					if( pStartSprite->m_Textures[i].m_Texture != pCurrent->m_Textures[i].m_Texture )
					{
						batch = false;
						break;
					}
				}

				// Check render state
				if ( batch )
					batch = (pStartSprite->m_RenderState == pCurrent->m_RenderState);

				// Increase batch count.
				if ( batch && pStartSprite->m_Count < 65535 )
				{
					pStartSprite->m_Count++;
				}
				else
				{
					pStartSprite = pCurrent;
					batch = true;
				}
			}
		}
	}
	m_SpritesBatched = true;
}


#ifndef CFG_GOLD
void C_Simple2D::AddDebugHelpers()
{
	if(g_UiDebugShowSafeArea)
	{
		const F32 safe_area_size		= 0.15f;					// 15%
		const F32 safe_area_half_size	= safe_area_size*0.5f;		// 7.5%
		const F32 safe_area_width		= 1.0f-safe_area_size;		// 85%

		// Add 4:3 safe area
		AddSprite(gSimple2D.TopLayer(),
				Maths::HVec3( (UI_VSCREEN_SIZEX * safe_area_half_size), (UI_VSCREEN_SIZEY * safe_area_half_size), 0.0f ),
				UI_VSCREEN_SIZEX * safe_area_width, UI_VSCREEN_SIZEY * safe_area_width, 0x20ffff00);
		AddTexUVs(0,gUITextureManager.GetTextureByName("null"),0,0,1,1);

		// Add widescreen area
		if(m_WideScreen)
		{
			AddSprite(gSimple2D.TopLayer(),
					Maths::HVec3( ScreenLeft() + (ScreenWidth() * safe_area_half_size), (UI_VSCREEN_SIZEY * safe_area_half_size), 0.0f ),
					ScreenWidth() * safe_area_width, UI_VSCREEN_SIZEY * safe_area_width, 0x20ff0000);
			AddTexUVs(0,gUITextureManager.GetTextureByName("null"),0,0,1,1);
		}
	}

	if(g_UiDebugGraphicActive)
	{
		C_UIColour colour( 
			(U8)(g_UiDebugGraphic_Colour->a*255.0f), 
			(U8)(g_UiDebugGraphic_Colour->r*255.0f), 
			(U8)(g_UiDebugGraphic_Colour->g*255.0f),
			(U8)(g_UiDebugGraphic_Colour->b*255.0f)
		);

		// Add debug graphic
		AddSprite(gSimple2D.TopLayer(), Maths::HVec3( (F32)g_UiDebugGraphic_px, (F32)g_UiDebugGraphic_py, (F32)g_UiDebugGraphic_pz ), g_UiDebugGraphic_sx*g_UiDebugGraphic_scale, g_UiDebugGraphic_sy*g_UiDebugGraphic_scale, colour);
		AddJustification(g_UiDebugGraphicJustify);
		C_WideString32 textureName;
		if( g_UiDebugGraphicTexture >= 0 )
		{
			AddTexUVs(0,gUITextureManager.GetTextureByIndex( (U32)g_UiDebugGraphicTexture ),g_UiDebugGraphic_tpu,g_UiDebugGraphic_tpv,g_UiDebugGraphic_tsu,g_UiDebugGraphic_tsv);
			textureName.Format( L"%S", gUITextureManager.GetTextureNameByIndex( (U32)g_UiDebugGraphicTexture ) );
		}
		else
		{
			C_TextureHandleIndex handle = (S32)g_UiDebugGraphicTexture;
			if( handle.Index() < m_AlternateTextures.Size() )
			{
				if( m_AlternateTextures[ handle.Index() ]->Handle() )
				{
					AddTexUVs(0, handle, g_UiDebugGraphic_tpu,g_UiDebugGraphic_tpv,g_UiDebugGraphic_tsu,g_UiDebugGraphic_tsv);
					const char *name = m_AlternateTextures[ handle.Index() ]->Name();
					if( name )
						textureName.Format( L"%S", name );
				}
				else
				{
					AddTexUVs(0, 0, g_UiDebugGraphic_tpu,g_UiDebugGraphic_tpv,g_UiDebugGraphic_tsu,g_UiDebugGraphic_tsv);
					const char *name = m_AlternateTextures[ handle.Index() ]->Name();
					if( name )
						textureName.Format( L"<%S> - FREE", name );
				}
			}
			else
			{
				// Out of range
				AddTexUVs(0, 0, g_UiDebugGraphic_tpu,g_UiDebugGraphic_tpv,g_UiDebugGraphic_tsu,g_UiDebugGraphic_tsv);
				textureName = "<unknown>";
			}
		}

		if(g_UiDebugGraphicShowName && textureName.Chars() )
		{
			// Add texture name.
			const Maths::HVec3 drawSize( g_UiDebugGraphic_sx * g_UiDebugGraphic_scale, g_UiDebugGraphic_sy * g_UiDebugGraphic_scale, 0.0f );
			const Maths::HVec3 offset = GetJustifiedCorner( g_UiDebugGraphicJustify, 0, false ) * drawSize;
			gFont.DrawDebugText( gSimple2D.TopLayer(), Maths::HVec3( (F32)g_UiDebugGraphic_px + ( g_UiDebugGraphic_sx * 0.5f )*g_UiDebugGraphic_scale, (F32)g_UiDebugGraphic_py + (g_UiDebugGraphic_sy*g_UiDebugGraphic_scale), (F32)g_UiDebugGraphic_pz ) + offset, E_Justify_TopCentre, textureName.Chars(), 0xffffffff, 12.0f );
		}
	}

	if( g_UiDebugStencilActive )
	{
		C_UIColour colour( 
			(U8)(g_UiDebugStencilColour->a*255.0f), 
			(U8)(g_UiDebugStencilColour->r*255.0f), 
			(U8)(g_UiDebugStencilColour->g*255.0f),
			(U8)(g_UiDebugStencilColour->b*255.0f)
		);

		Display::C_StencilState op;

		op = m_DefaultStackedRenderState.m_StencilState;

		op.m_StencilEnable		= Display::E_Bool_True;
		op.m_StencilFunc		= Display::E_CmpFunc_Equal;
		op.m_StencilRef			= g_UiDebugStencilValue;

		C_UIAutoStencil banana( gSimple2D.TopLayer(), 0, op );

		AddSprite( TopLayer(), Maths::HVec3( ScreenLeft(), 0.0f, 0.0f ), ScreenWidth(), UI_VSCREEN_SIZEY, colour);
		AddTexUVs(0,gUITextureManager.GetTextureByName("null"),0,0,1,1);
	}

	if ( g_UiDebugShowShowStatistics )
	{
		C_WideString64 text;

		// Draw gadget count.
		text.Format( L"Gadgets : %d", gButtons.NumberOfGadgets() );
		gFont.DrawDebugText( gSimple2D.TopLayer(), Maths::HVec3( 0.0f, 32.0f, 0.0f ), E_Justify_TopLeft, text, 0xff00ff80, 16.0f, 0.0f );

		// Draw sprite
		text.Format( L"Sprites : %d / %d", m_SpriteCount, m_MaxSprites );
		gFont.DrawDebugText( gSimple2D.TopLayer(), Maths::HVec3( 0.0f, 48.0f, 0.0f ), E_Justify_TopLeft, text, 0xff00ff80, 16.0f, 0.0f );

		// Draw particles
		text.Format( L"Particles : %d / %d", gUIParticleMan.AliveParticles(), gUIParticleMan.NumParticles() );
		gFont.DrawDebugText( gSimple2D.TopLayer(), Maths::HVec3( 0.0f, 64.0f, 0.0f ), E_Justify_TopLeft, text, 0xff00ff80, 16.0f, 0.0f );
	}

	if( g_UiDebugShowTransformUsage )
	{
		static U16 pre_max = 0, post_max = 0;
		if( pre_max < m_PreTransformPoolIndex )
			pre_max = m_PreTransformPoolIndex;
		if( post_max < m_PostTransformPoolIndex )
			post_max = m_PostTransformPoolIndex;

		C_WideString32 blah;
		blah.Format( L"Pre = %d:%d / Post = %d:%d", m_PreTransformPoolIndex,pre_max,  m_PostTransformPoolIndex,post_max );
		gFont.DrawDebugText( gSimple2D.TopLayer(), Maths::HVec3( 32.0f, 65.0f, 0.0f ), E_Justify_MiddleCentre, blah, 0xff00ff80, 16.0f, 0.0f );
	}
}
#endif

// -------------
// C_RenderState
// -------------

#ifndef CFG_GOLD
Debug::Bool g_ShowLayerBackground("UI.Layers.Background",true);
Debug::Bool g_ShowLayerLayer1("UI.Layers.Layer1",true);
Debug::Bool g_ShowLayerLayer2("UI.Layers.Layer2",true);
Debug::Bool g_ShowLayerLayer3("UI.Layers.Layer3",true);
Debug::Bool g_ShowLayerLayer4("UI.Layers.Layer4",true);
Debug::Bool g_ShowLayerTop("UI.Layers.Top",true);
#endif


C_Renderer::C_Light& C_Simple2D::GetLight( const PanelIndex panelIndex )
{
	return m_Lighting[ panelIndex ];
}

void C_Simple2D::SetLight( const PanelIndex panelIndex )
{
#ifndef CFG_GOLD
	if( g_UiDebugLightEnable )
	{
		C_Renderer::C_Light	l;

		l.m_Pos			= Maths::HVec4( 0.0f, 0.0f, (F32)g_UiDebugLightPosZ, 0.0f );
		l.m_Dir			= Maths::HVec4( 0.0f, 0.0f, 1.0f, 0.0f );
		l.m_ColourFrom	= Maths::HVec4( (F32)g_UiDebugLightColourFrom->r, (F32)g_UiDebugLightColourFrom->g, (F32)g_UiDebugLightColourFrom->b, (F32)g_UiDebugLightColourFrom->a );
		l.m_ColourTo	= Maths::HVec4( (F32)g_UiDebugLightColourTo->r, (F32)g_UiDebugLightColourTo->g, (F32)g_UiDebugLightColourTo->b, (F32)g_UiDebugLightColourTo->a );
		l.m_FallOff		= Maths::HVec4( (F32)g_UiDebugLightFallOff, (F32)g_UiDebugLightFallOff, (F32)g_UiDebugLightFallOff, 0.0f );

		m_pRenderer->SetLight( l );

		return;
	}
#endif
	m_pRenderer->SetLight( m_Lighting[ panelIndex ] );
}

void C_Simple2D::PreRender(  )
{
	// texture addressing
	m_pRenderer->Bind();

	// stencil states
	SetCurrentStackedRenderState( );

	// fill mode
	Display::Device()->SetFillMode( Display::E_FillMode_Solid );

	// common UI shader constant
	SetTransformMatrixShaderConstant();
}

void C_Simple2D::PostRender( )
{
	m_pRenderer->Unbind();

	// stencil states
	Display::Device()->SetStencilEnable( Display::E_Bool_False );

	// culling
	Display::Device()->SetCullMode( Display::E_CullMode_Default );

	// z-test
	Display::Device()->SetDepthEnable( Display::E_Bool_Default );
	Display::Device()->SetDepthWriteEnable( Display::E_Bool_Default );
	Display::Device()->SetDepthFunc( Display::E_CmpFunc_Default );

	// scissor test
	Display::Device()->SetScissorTest( Display::E_Bool_False );
}

bool C_Simple2D::ShouldRender( const PanelIndex panel )
{
	for ( LayerIndex i = 0; i < LayerCount(); i++ )
		if ( !m_LayerPanelSprites[ i ][ panel ].Empty() )
			return true;

	return false;
}

void C_Simple2D::RenderPanel( const PanelIndex uPanel )
{
#ifndef CFG_GOLD
	const bool debugShowLayer[ E_SpriteLayer_Count ] = 
	{
		g_ShowLayerBackground,		// E_SPRITE_LAYER_BACKGROUND
		g_ShowLayerLayer1,			// E_SPRITE_LAYER_1
		g_ShowLayerLayer2,			// E_SPRITE_LAYER_2
		g_ShowLayerLayer3,			// E_SPRITE_LAYER_3
		g_ShowLayerLayer4,			// E_SPRITE_LAYER_4
		g_ShowLayerTop,				// E_SPRITE_LAYER_TOP
	};
#endif

	if ( !m_SpritesBatched )
		LOGWARN( g_Log, DBSTR "Sprites have not been batched and could cause performance slowdown" );

	if( g_UiDebugDoRender == false )
		return;

	SetLight( uPanel );

	for( U32 i = 0; i < m_uLayerCount; i++ )
	{
#ifndef CFG_GOLD
		if ( ( i < E_SpriteLayer_Count ) && !debugShowLayer[ i ] )
			continue;
#endif
		if( m_LayerPanelSprites[ i ][ uPanel ].Empty() )
			continue;

		Render( i, uPanel );
	}
}

void C_Simple2D::Render( const LayerIndex uLayer, const PanelIndex uPanel )
{
#ifdef PLATFORM_PC
	const U32 vertexCount = 6;
	const U32 quadVertexMapping[6] = {0,1,2,0,2,3};
#else
	const U32 vertexCount = 4;
	const U32 quadVertexMapping[4] = {0,1,2,3};
#endif

	const Maths::HVec2 sQuadTexCorners[ 4 ] = 
	{
		Maths::HVec2(0.0f, 0.0f),
		Maths::HVec2(1.0f, 0.0f),
		Maths::HVec2(1.0f, 1.0f),
		Maths::HVec2(0.0f, 1.0f),
	};
	Maths::HVec2 uv_pos[C_Sprite::m_NumTextureStages];
	Maths::HVec2 uv_size[C_Sprite::m_NumTextureStages];

	S32 currentRunCount = 0;

	// grrr set blend mode sets AlphaTestEnable to true.
	bool setAlphaTestEnable = false;
	Display::E_Bool alphaTestEnable = Display::E_Bool_False;
	// Now for our overides > E_BlendMode_Unknown
	U8 currentBlendMode = (U8)E_UiBlendMode_Unknown;

	UITextureHandle	texturePtr[C_Sprite::m_NumTextureStages] = { NULL, NULL, NULL };
	C_Sprite *pSprite = m_Sprites;
	U8*  __restrict pVtx = NULL;// = (C_Vertex3d*)lock.m_pData;

	U32 stages = 0;

	TSpriteList& panelLayerSprites = m_LayerPanelSprites[ uLayer ][ uPanel ];

	Maths::HMat43 zeroMatrixZ;
	zeroMatrixZ.SetIdentity();
	if( m_bUsing2D )
		zeroMatrixZ.AZ( Maths::HVec3( 0.0f, 0.0f, 0.0f ) );

	for ( S32 iSpriteNo = 0; iSpriteNo < panelLayerSprites.Size(); iSpriteNo++ )
	{
		C_Sprite* pSprite = panelLayerSprites[ iSpriteNo ];

		if ( !pSprite )	// this should never happen
			continue;

		// Render mode change
		if( pSprite->m_Type < E_SpriteType_RenderStateCount )
		{
			// Get current renderstate for altering
			C_StackedRenderState& set = CurrentStackedRenderState();
			
			switch( pSprite->m_Type )
			{
			// Colour write enable
			case E_SpriteType_SetColourWrite : 
				set.m_ColourWriteEnable = (Display::E_ColourWriteEnable)pSprite->m_UserData;
				SetColourWriteEnable( set );
				break;

			// clip window
			case E_SpriteType_SetClipWindow: 
				if( pSprite->m_UserData == 1 )
				{
					set.m_ClipEnabled = Display::E_Bool_True;
					set.m_ClipPos.vx = pSprite->m_Pos.x;
					set.m_ClipPos.vy = pSprite->m_Pos.y;
					set.m_ClipSize.vx = pSprite->m_Size.x;
					set.m_ClipSize.vy = pSprite->m_Size.y;
				}
				else
				{
					set.m_ClipEnabled = Display::E_Bool_False;
				}
				SetClipWindow( set );
				break;

			// Stencil buffer
			case E_SpriteType_SetStencilBuffer:
				set.m_StencilState = pSprite->StencilState();
				SetStencilBufferState( set );
				break;

			// Cull mode
			case E_SpriteType_SetCullMode :
				set.m_CullMode = (Display::E_CullMode)pSprite->m_UserData;
				SetCullMode( set );
				break;

			// Alpha test
			case E_SpriteType_SetAlphaTest :
				set.m_AlphaTestEnabled = (Display::E_Bool)pSprite->m_UserData;

				alphaTestEnable = set.m_AlphaTestEnabled;
				setAlphaTestEnable = true;
				break;

			// Depth buffer
			case E_SpriteType_SetDepthBuffer : 
				{
					C_StackedRenderState::C_DepthState depth = *((C_StackedRenderState::C_DepthState*)&pSprite->m_UserData);

					set.m_DepthEnabled		= (depth.enabled == 1) ? Display::E_Bool_True : Display::E_Bool_False;
					set.m_DepthWriteEnabled = (depth.write == 1) ? Display::E_Bool_True : Display::E_Bool_False;
					set.m_DepthCompareFunc	= (Display::E_CmpFunc)depth.function;

					SetDepthBufferState( set );
				}
				break;
			// Stack
			case E_SpriteType_SetRendersStateStack:
				if(pSprite->m_UserData == 1)
				{
					C_StackedRenderState* p = m_StackedRenderStatePool.Allocate();
					if( p )
					{
						// Make a copy of the current state
						*p = CurrentStackedRenderState();

						m_StackedRenderStates.PushBack( p );
					}
				}
				else
				{
					if(m_StackedRenderStates.Size() > 0 )
					{
						C_StackedRenderState* p = m_StackedRenderStates.Back();
						m_StackedRenderStates.PopBack();

						m_StackedRenderStatePool.Free( p );

						// Restore renderstates
						SetCurrentStackedRenderState();
					}
				}
				break;

			// Unknown
			default:
				LOGWARN( g_Log, DBSTR "Unknown sprite type %d", pSprite->m_Type );
				break;
			}

			// Onto next sprite
			continue;
		}

		// Draw normally
		if(currentRunCount == 0)
		{
#ifndef CFG_GOLD
			if( g_UiDebugBlendModeEnable )
			{
					Display::Device()->SetBlendMode( Display::E_BlendMode_Unknown );
					Display::Device()->SetAlphaTestEnable(Display::E_Bool_True);
					Display::Device()->SetBlendEnable(Display::E_Bool_True);

					Display::Device()->SetBlend(	(Display::E_Blend)(S32)g_UiDebugBlendModeSrcColour, 
													(Display::E_Blend)(S32)g_UiDebugBlendModeDstColour, 
													Display::E_BlendOp_Add,
													(Display::E_Blend)(S32)g_UiDebugBlendModeSrcAlpha,
													(Display::E_Blend)(S32)g_UiDebugBlendModeDstAlpha,
													Display::E_BlendOp_Add );
			}
			else
#endif
			{
				// Blend mode
				const U8 bm = pSprite->m_RenderState.m_BlendMode;
				if( currentBlendMode != bm )
				{
					if( bm < Display::E_BlendMode_Count )
					{
						// Valid *Display* library blend mode
						Display::Device()->SetBlendMode( (Display::E_BlendMode)bm );
					}
					else
					{
						// Extended UI blend mode
						Display::Device()->SetBlendMode( Display::E_BlendMode_Unknown );
						Display::Device()->SetAlphaTestEnable(Display::E_Bool_True);
						Display::Device()->SetBlendEnable(Display::E_Bool_True);

						// switch( bm ) // Not needed we've only one overide in Amax
						// {
						// case E_UiBlendMode_ExtAlphaAdd:
						Display::Device()->SetBlend(Display::E_Blend_SrcAlpha, 
													Display::E_Blend_InvSrcAlpha, 
													Display::E_BlendOp_Add,
													Display::E_Blend_One,
													Display::E_Blend_InvSrcAlpha,
													Display::E_BlendOp_Add );
						// break;
						// }
					}

					currentBlendMode = bm;
				}
			}

			if ( setAlphaTestEnable )
			{
				SetAlphaTestEnable( alphaTestEnable );
				setAlphaTestEnable = false;
			}
			
			const E_Effect eEffect = (E_Effect)pSprite->m_RenderState.m_EffectIndex;

			// texture stages
			stages = 0;
			for(U32 b=0;b<C_Sprite::m_NumTextureStages;b++)
			{
				if( !pSprite->m_Textures[b].m_Texture.Active())
					break;

				const C_TextureHandleIndex& texture_idx = pSprite->m_Textures[b].m_Texture;

				// Get texture
				texturePtr[b] = texture_idx.Alternative() ? m_AlternateTextures[ texture_idx.Index() ]->Handle( ) : gUITextureManager.GetTexturePtr( texture_idx.Index() );
				if( !texturePtr[b] )
					break;

				// This stage is valid increment
				stages++;
			}
			for(U32 i = 0; i < stages; i++)
				Display::Device()->SetTexture(i, texturePtr[i]);

			if ( pSprite->m_Type == E_SpriteType_TriList || pSprite->m_Type == E_SpriteType_TriStrip )
			{
				RenderTriangles( pSprite );
				continue;
			}
			else
			if( pSprite->m_Type != E_SpriteType_Sprite && UiInterface()->ExternalRender( pSprite ) )
				continue;
	
			// Set shader data
			switch( eEffect )
			{
			case E_Effect_IntensityLerp:
			case E_Effect_AM_DamageBar:
			case E_Effect_AM_Perks:
				m_pRenderer->SetLerpColourParam( pSprite->m_RenderState.m_EffectParam );
				m_pRenderer->SetSettings( *(F32*)&pSprite->m_RenderState.m_EffectParam2 );
				break;
			case E_Effect_IntensityCutout:
				m_pRenderer->SetSettings( *(F32*)&pSprite->m_RenderState.m_EffectParam );
				break;
			case E_Effect_UserGammaCorrect:
			case E_Effect_UserGammaCorrect2_IAA:
				m_pRenderer->SetSettings( m_UserGamma, 0.0f, 0.0f, 0.0f );
				break;
			case E_Effect_AM_Loading:
				{
					m_pRenderer->SetSettings( (F32)g_UiLoadingTint, (F32)g_UiLoadingPower, (F32)g_UiLoadingDarken, (F32)g_UiLoadingVig );
				}
				break;
			}

			// Set shader
			const U32 stagesCount = m_pRenderer->SetEffect( eEffect, stages, (E_TextureAddressMode) pSprite->m_RenderState.m_EffectAddressMode );
			if( stagesCount != stages )
			{
				LOGWARN( g_Log, DBSTR "%d stages set should be %d", stages, stagesCount );
				stages = stagesCount;
			}

			// Setup verts for drawing
			currentRunCount = pSprite->m_Count;
			if(currentRunCount <= 0)
			{
				LOGFATAL(g_Log, "SIMPLE2D: currentRunCount %i is not greater than zero...see DavidG or LloydW asap!\n", currentRunCount);
			}
			
			#ifndef CFG_GOLD
			if(pVtx)
			{
				LOGFATAL(g_Log, "SIMPLE2D: Vertex is not NULL\n");
			}
			#endif

			#ifdef PLATFORM_PC
			pVtx = (U8*)m_pRenderer->BeginVertices(Display::E_PrimitiveType_TriangleList, currentRunCount * vertexCount, m_pRenderer->GetVertexStride() ); // This must not fail

			#else
			pVtx = (U8*)m_pRenderer->BeginVertices(Display::E_PrimitiveType_QuadList, currentRunCount * vertexCount, m_pRenderer->GetVertexStride() ); // This must not fail
			#endif
			
			#ifndef CFG_GOLD
			if(!pVtx)
			{
				LOGFATAL(g_Log, "SIMPLE2D: Vertex is NULL\n");
			}
			#endif
		}
		
		if(!pVtx)
		{
			LOGFATAL(g_Log, "SIMPLE2D: Failed to render %i vertices of total size %i...tell LloydW asap!\n", currentRunCount * vertexCount, currentRunCount * vertexCount * m_pRenderer->GetVertexStride() );
		}

		// Get quad vertex positions
		Maths::HVec3*	pJustifiedCorner = IsPanel3D( uPanel ) ? m_JustifiedCorner_Flipped[ pSprite->m_Justify ] : m_JustifiedCorner[ pSprite->m_Justify ];

		Maths::HVec3 p_size = Maths::Load<Maths::HVec3, MathsStorage::Float3>(&pSprite->m_Size);
		Maths::HVec3 mrPossy = Maths::Load<Maths::HVec3, MathsStorage::Float3>(&pSprite->m_Pos);

		Maths::HMat43 mrTranny = Maths::HMat43(Maths::C_MatrixScale(p_size)) * m_PreTransformMatrixPool[ pSprite->m_PreTransformIndex ];
		mrTranny.Translation(mrTranny.Translation() + mrPossy);

#ifndef CFG_GOLD
		// If there is a Photo Mode capture, move the Perk Stash graphics with the car:
		if (uPanel == E_SpritePanel_VP2)
			mrTranny.Translation(mrTranny.Translation() + g_PerkStashOffset);
#endif

		mrTranny = mrTranny * m_PostTransformMatrixPool[ pSprite->m_PostTransformIndex ];

		for(U32 i = 0; i < C_Sprite::m_NumTextureStages; i++)
		{
			uv_pos[i]	= Maths::HVec2(pSprite->m_Textures[i].m_pu,pSprite->m_Textures[i].m_pv);
			uv_size[i]	= Maths::HVec2(pSprite->m_Textures[i].m_su,pSprite->m_Textures[i].m_sv);
		}

		U32 stride = m_pRenderer->GetVertexStride();
		if( stages == 1 )
		{
#ifdef PLATFORM_PC
			for(U32 vtIndex = 0; vtIndex < vertexCount; vtIndex++, pVtx += stride )
			{
				C_Vertex3d* __restrict pVertex = (C_Vertex3d*)pVtx;
				// Pos
				U32 i = quadVertexMapping[vtIndex];

				Maths::Store( pJustifiedCorner[i] * mrTranny, (MathsStorage::Float3 *) &pVertex->m_X );

				Maths::HVec2 vUV0( uv_pos[0] + ( uv_size[0] * sQuadTexCorners[ i ] ) );

				// Colour
				pVertex->m_Colour = Maths::RGBA(pSprite->m_Colours[i].R(), pSprite->m_Colours[i].G(), pSprite->m_Colours[i].B(), pSprite->m_Colours[i].A()).Colour();

				// UV
				Maths::Store( vUV0, (MathsStorage::Float2 *) &pVertex->m_U0 );
			}
#else
			DCBT(0,&pSprite->m_Colours);

			C_Vertex3d* __restrict pVertex0 = (C_Vertex3d*)(pVtx         );
			C_Vertex3d* __restrict pVertex1 = (C_Vertex3d*)(pVtx+stride  );
			C_Vertex3d* __restrict pVertex2 = (C_Vertex3d*)(pVtx+stride*2);
			C_Vertex3d* __restrict pVertex3 = (C_Vertex3d*)(pVtx+stride*3);
			// Pos
			Maths::Store( pJustifiedCorner[0] * mrTranny, (MathsStorage::Float3 *) &pVertex0->m_X );
			Maths::Store( pJustifiedCorner[1] * mrTranny, (MathsStorage::Float3 *) &pVertex1->m_X );
			Maths::Store( pJustifiedCorner[2] * mrTranny, (MathsStorage::Float3 *) &pVertex2->m_X );
			Maths::Store( pJustifiedCorner[3] * mrTranny, (MathsStorage::Float3 *) &pVertex3->m_X );

			Maths::HVec2 vUV0( uv_pos[0] + ( uv_size[0] * sQuadTexCorners[ 0 ] ) );
			Maths::HVec2 vUV1( uv_pos[0] + ( uv_size[0] * sQuadTexCorners[ 1 ] ) );
			Maths::HVec2 vUV2( uv_pos[0] + ( uv_size[0] * sQuadTexCorners[ 2 ] ) );
			Maths::HVec2 vUV3( uv_pos[0] + ( uv_size[0] * sQuadTexCorners[ 3 ] ) );

			// Colour
			pVertex0->m_Colour = ((U32*)&pSprite->m_Colours)[0];
			pVertex1->m_Colour = ((U32*)&pSprite->m_Colours)[1];
			pVertex2->m_Colour = ((U32*)&pSprite->m_Colours)[2];
			pVertex3->m_Colour = ((U32*)&pSprite->m_Colours)[3];

			// UV
			Maths::Store( vUV0, (MathsStorage::Float2 *) &pVertex0->m_U0 );
			Maths::Store( vUV1, (MathsStorage::Float2 *) &pVertex1->m_U0 );
			Maths::Store( vUV2, (MathsStorage::Float2 *) &pVertex2->m_U0 );
			Maths::Store( vUV3, (MathsStorage::Float2 *) &pVertex3->m_U0 );

			pVtx += stride*4;
#endif
		}
		else
		if( stages == 2 )
		{
			for(U32 vtIndex = 0; vtIndex < vertexCount; vtIndex++, pVtx += stride )
			{
				C_Vertex3d* __restrict pVertex = (C_Vertex3d*)pVtx;
				// Pos
				U32 i = quadVertexMapping[vtIndex];

				Maths::Store( pJustifiedCorner[i] * mrTranny, (MathsStorage::Float3 *) &pVertex->m_X );

				Maths::HVec2 vUV0( uv_pos[0] + ( uv_size[0] * sQuadTexCorners[ i ] ) );
				Maths::HVec2 vUV1( uv_pos[1] + ( uv_size[1] * sQuadTexCorners[ i ] ) );

				// Colour
				pVertex->m_Colour = Maths::RGBA(pSprite->m_Colours[i].R(), pSprite->m_Colours[i].G(), pSprite->m_Colours[i].B(), pSprite->m_Colours[i].A()).Colour();

				// UV
				Maths::Store( vUV0, (MathsStorage::Float2 *) &pVertex->m_U0 );
				Maths::Store( vUV1, (MathsStorage::Float2 *) &pVertex->m_U1 );
			}
		}
		else
		{
			for(U32 vtIndex = 0; vtIndex < vertexCount; vtIndex++, pVtx += stride )
			{
				C_Vertex3d* __restrict pVertex = (C_Vertex3d*)pVtx;
				// Pos
				U32 i = quadVertexMapping[vtIndex];

				Maths::Store( pJustifiedCorner[i] * mrTranny, (MathsStorage::Float3 *) &pVertex->m_X );

				Maths::HVec2 vUV0( uv_pos[0] + ( uv_size[0] * sQuadTexCorners[ i ] ) );
				Maths::HVec2 vUV1( uv_pos[1] + ( uv_size[1] * sQuadTexCorners[ i ] ) );
				Maths::HVec2 vUV2( uv_pos[2] + ( uv_size[2] * sQuadTexCorners[ i ] ) );

				// Colour
				pVertex->m_Colour = Maths::RGBA(pSprite->m_Colours[i].R(), pSprite->m_Colours[i].G(), pSprite->m_Colours[i].B(), pSprite->m_Colours[i].A()).Colour();

				// UV
				Maths::Store( vUV0, (MathsStorage::Float2 *) &pVertex->m_U0 );
				if( stages != 0 ) // [LloydW] This writes over the end of the vertex buffer if stages is 0
				{
					Maths::Store( vUV1, (MathsStorage::Float2 *) &pVertex->m_U1 );
					Maths::Store( vUV2, (MathsStorage::Float2 *) &pVertex->m_U2 );
				}
			}
		}
		if (--currentRunCount == 0)
		{
			m_pRenderer->EndVertices((void*)pVtx);
			pVtx = NULL;
		}
	}
	
	if(currentRunCount != 0)
	{
		LOGFATAL(g_Log, "SIMPLE2D: currentRunCount is %i it should be zero...see DavidG or LloydW asap!\n", currentRunCount);
	}
	if(pVtx)
	{
		LOGFATAL(g_Log, "SIMPLE2D: pVertex is not NULL...see LloydW asap!\n", currentRunCount);
	}
	if(m_pRenderer->BeginVerticesLocked())
	{
		LOGFATAL(g_Log, "SIMPLE2D: Begin vertices is locked...see LloydW asap!\n");
	}

	// Clean up renderstate stack
	if(m_StackedRenderStates.Size() > 0 )
	{
		// Clear stack
		while( m_StackedRenderStates.Size() > 0 )
		{
			C_StackedRenderState*p = m_StackedRenderStates.Back();
			m_StackedRenderStates.PopBack();

			m_StackedRenderStatePool.Free( p );
		}
		SetCurrentStackedRenderState( );
	}
}

// ------------------------------
// C_GradientAlternateTexture
// ------------------------------
C_Simple2D::C_GradientAlternateTexture::C_GradientAlternateTexture( const U32 width, const U32 height ) :
	m_Index( 0 ),
	m_Name( "GRADIENT" ),
	m_Width( width ),
	m_Height( height )

{
	RESOURCE_MEMORY_ID("UI");

	if (!Display::Device()->CreateTexture2D(width, height, 1, Display::E_Format_Linear, Display::E_Format_A8R8G8B8, &m_Texture))
	{
		LOGERROR( UI::g_Log, DBSTR "Failed to create gradient texture");
		return;
	}
#if 1
	// Debug clear the data out with test colour
	Display::C_LockedRect rect = m_Texture.Lock(0);
	if(rect.pBits)
	{
		// Test pink & yellow gradient 
		U32 *pData = reinterpret_cast<U32*>(rect.pBits);
		for(U32 y = 0; y < m_Height; y++)
		{
			U32 *pData = reinterpret_cast<U32*>(rect.pBits);
			for(U32 x = 0; x < m_Width; x++)
			{
				pData[x] = 0xffff00ff; //0xffff0000 | (((x*4)&0xff)<<8) | ((y*4)&0xff);
			}
			rect.pBits += rect.pitch;
		}
		m_Texture.Unlock(0);
	}
#endif

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(&m_Texture, "_GradientAlternateTexture");
	#endif
}

C_Simple2D::C_GradientAlternateTexture::~C_GradientAlternateTexture( )
{
	if(m_Texture.IsCreated())
		Display::Device()->DestroyTexture2D( &m_Texture );
}

void C_Simple2D::C_GradientAlternateTexture::Build( const U32 index, const C_AdobeGradient& gradient )
{
	if( index > m_Height )
	{
		LOGERROR( g_Log, DBSTR "index out of range");
		return;
	}
	if( m_Texture.IsCreated() == false )
	{
		LOGERROR( g_Log, DBSTR "texture hasn't been created");
		return;
	}

	Display::C_LockedRect rect = m_Texture.Lock(0);
	if(rect.pBits)
	{
		rect.pBits += rect.pitch * index;
		U32 *pData = reinterpret_cast<U32*>(rect.pBits);

		if(gradient.Stop(0).m_Ramp != 0.0f)
		{
			// Fill 0 -> first stop with first colour
			const U32 start = 0;
			const U32 end = (U32)floorf(gradient.Stop(0).m_Ramp * (F32)m_Width);
			for(U32 j = start; j < end; j++)
			{
				C_UIColour* p = (C_UIColour*)(&pData[j]);
				*p = gradient.Stop(0).m_Colour;
			}
		}
		if(gradient.Stop(gradient.StopCount()-1).m_Ramp != 1.0f)
		{
			// Fill last colour stop -> 1.0 with final colour
			const U32 start = (U32)floorf(gradient.Stop(gradient.StopCount()-1).m_Ramp * (F32)m_Width);
			const U32 end = m_Width;
			for(U32 j = start; j < end; j++)
			{
				C_UIColour* p = (C_UIColour*)(&pData[j]);
				*p = gradient.Stop(gradient.StopCount()-1).m_Colour;
			}
		}

		for(S32 i = 0; i < gradient.StopCount()-1; i++)
		{
			for(S32 j = 0; j < 2; j++)
			{
				F32 f_start = gradient.Stop(i).m_Ramp * (F32)m_Width;
				F32 f_end = gradient.Stop(i+1).m_Ramp * (F32)m_Width;
				F32 f_len = f_end - f_start;

				C_UIColour fromColour;
				C_UIColour toColour;

				// Do mid point
				if(j==0)
				{
					// start to mid
					f_end = f_start + (f_len * gradient.Stop(i).m_Mid);

					fromColour = gradient.Stop(i).m_Colour;
					toColour.Lerp( gradient.Stop(i).m_Colour, gradient.Stop(i+1).m_Colour, 0.5f );
				}
				else
				{
					// mid - end
					f_start = f_start + (f_len * gradient.Stop(i).m_Mid);

					fromColour.Lerp( gradient.Stop(i).m_Colour, gradient.Stop(i+1).m_Colour, 0.5f );
					toColour = gradient.Stop(i+1).m_Colour; 
				}

				const U32 start = (U32)floorf(f_start);
				const U32 end = (U32)floorf(f_end);

				if(start == end)
				{
					LOGWARN( g_Log, DBSTR "Points %2.2f and %2.2f are the same at this resolution : current range is %2.2f", gradient.Stop(i).m_Ramp, gradient.Stop(i+1).m_Ramp, 1.0f/(F32)m_Width );
					C_UIColour* p = (C_UIColour*)(&pData[start]);
					*p = gradient.Stop(i).m_Colour;
				}
				else
				{
					for(U32 j = start; j < end; j++)
					{
						C_UIColour* p = (C_UIColour*)(&pData[j]);
						p->Lerp( fromColour, toColour, (F32)(j-start) / (F32)(end-start));
					}
				}
			}
		}
		m_Texture.Unlock(0);
	}
}

void C_Simple2D::C_GradientAlternateTexture::Build( const U32 index, const C_UIColour* pColours, F32* pPositions, const U32 colour_count )
{
	if( index > m_Height )
	{
		LOGERROR( g_Log, DBSTR "index out of range");
		return;
	}
	if( m_Texture.IsCreated() == false )
	{
		LOGERROR( g_Log, DBSTR "texture hasn't been created");
		return;
	}

	Display::C_LockedRect rect = m_Texture.Lock(0);
	if(rect.pBits)
	{
		rect.pBits += rect.pitch * index;

		U32 *pData = reinterpret_cast<U32*>(rect.pBits);

		if(pPositions[0] != 0.0f)
		{
			// Fill 0 -> first stop with first colour
			const U32 start = 0;
			const U32 end = (U32)floorf(pPositions[0] * (F32)m_Width);
			for(U32 j = start; j < end; j++)
			{
				C_UIColour* p = (C_UIColour*)(&pData[j]);
				*p = pColours[0];
			}
		}
		if(pPositions[colour_count-1] != 1.0f)
		{
			// Fill last colour stop -> 1.0 with final colour
			const U32 start = (U32)floorf(pPositions[colour_count-1] * (F32)m_Width);
			const U32 end = m_Width;
			for(U32 j = start; j < end; j++)
			{
				C_UIColour* p = (C_UIColour*)(&pData[j]);
				*p = pColours[colour_count-1];
			}
		}

		// Run through the stops
		for(U32 i = 0; i < colour_count-1; i++)
		{
			const U32 start = (U32)floorf(pPositions[i] * (F32)m_Width);
			const U32 end = (U32)floorf(pPositions[i+1] * (F32)m_Width);

			if(start == end)
			{
				LOGWARN( g_Log, DBSTR "Points %2.2f and %2.2f are the same at this resolution : current range is %2.2f", pPositions[i], pPositions[i+1], 1.0f/(F32)m_Width );
				C_UIColour* p = (C_UIColour*)(&pData[start]);
				*p = pColours[i];
			}
			else
			{
				for(U32 j = start; j < end; j++)
				{
					C_UIColour* p = (C_UIColour*)(&pData[j]);
					p->Lerp( pColours[i], pColours[i+1], (F32)(j-start) / (F32)(end-start));
				}
			}
		}
		m_Texture.Unlock(0);
	}
}

// ------------------------------
// C_RenderTargetAlternateTexture
// ------------------------------
C_Simple2D::C_RenderTargetAlternateTexture::C_RenderTargetAlternateTexture( const S32 sIndex ) :
		I_Renderer( "UiRenderTarget" ),
		m_sIndex( sIndex )
{
	m_Name.Printf( "RENDER_TARGET%d", sIndex );

	m_Width		= (U32)gSimple2D.GetClipHeight();
	m_Height	= (U32)gSimple2D.GetClipHeight();
}

C_Simple2D::C_RenderTargetAlternateTexture::C_RenderTargetAlternateTexture( const S32 sIndex, const U32 uWidth, const U32 uHeight ) : 
		m_Width( uWidth ),
		m_Height( uHeight ),
		I_Renderer( "UiRenderTarget" ),
		m_sIndex( sIndex ),
		m_bInitialised( false )
{
	m_Name.Printf( "RENDER_TARGET%d", sIndex );
}

C_Simple2D::C_RenderTargetAlternateTexture::~C_RenderTargetAlternateTexture()
{
	//
}

bool C_Simple2D::C_RenderTargetAlternateTexture::OnCreateVolatile()
{
	// gSimple2D.GetClipHeight()

	// fullscreen texture - maybe we should have a parameter for this
	m_bInitialised = Display::Device()->CreateRenderTexture2D( m_Width, m_Height, 1, Display::E_Usage_Colour, Display::E_Format_A8R8G8B8, Display::E_Multisample_2, NULL, &m_RenderTexture );
	#if TRACK_DISPLAY2_RESOURCES
	if(m_bInitialised)
		Display2::Device()->SetRenderTexture2DName(&m_RenderTexture, "_Simple2DRT");
	#endif
	return m_bInitialised;
}

void C_Simple2D::C_RenderTargetAlternateTexture::OnDestroyVolatile()
{
	Display::Device()->Destroy( &m_RenderTexture );
	m_bInitialised = false;
}

bool C_Simple2D::C_RenderTargetAlternateTexture::BeginScene()
{
	if ( !m_bInitialised )
		return false;

//	Display::Device()->BeginScene();
	Display::Device()->SetColourDepthStencilSurface( &m_RenderTexture.GetSurface(), NULL );
	Display::Device()->Clear( Display::E_Clear_Colour, Maths::RGBA( 0, 0, 0, 0 ) );

	Display::C_Viewport viewport;
	viewport.m_X = 0;
	viewport.m_Y = 0;
	viewport.m_W = m_Width;
	viewport.m_H = m_Height;

	Display::Device()->SetViewport( viewport );

	return true;
}

void C_Simple2D::C_RenderTargetAlternateTexture::EndScene()
{
	if ( !m_bInitialised )
		return;

	Display::Device()->SetColourDepthStencilSurface();
	Display::Device()->InvalidateState();
//	Display::Device()->EndScene();
}

void C_Simple2D::RenderTriangles( const C_Sprite* pSprite )
{
	const E_Effect eEffect = (E_Effect)pSprite->m_RenderState.m_EffectIndex;
	if ( ! (m_pRenderer->ShaderUsage( eEffect ) & E_ShaderUsageFlags_Polygon) )
		return;

	Maths::HMat43 mat( Maths::C_MatrixScale( Maths::HVec3( pSprite->m_Size.x, pSprite->m_Size.y, pSprite->m_Size.z ) ) );
	mat = mat * m_PreTransformMatrixPool[ pSprite->m_PreTransformIndex ];
	mat.Translation( Maths::HVec3( &pSprite->m_Pos ) );

	mat = mat * m_PostTransformMatrixPool[ pSprite->m_PostTransformIndex ];// * zeroMatrixZ;

	// user data is an address to the render info
	C_UITriListRenderInfo* pRenderInfo = (C_UITriListRenderInfo*)pSprite->m_UserData;

	if ( !pRenderInfo )
		return;

	if ( m_pRenderer->ShaderUsage( eEffect ) & E_ShaderUsageFlags_PolygonFont )
	{
		m_pRenderer->SetPolygonFontSettings( C_PolygonFontEffectParam( pSprite->m_RenderState.m_EffectParam ).GetShaderSettings() );
	}

	if ( m_pRenderer->ShaderUsage( eEffect ) & E_ShaderUsageFlags_PolygonColour )
	{
		F32 pColour[ 4 ] =
		{
			pSprite->m_Colours[ 0 ].R01(),
			pSprite->m_Colours[ 0 ].G01(),
			pSprite->m_Colours[ 0 ].B01(),
			pSprite->m_Colours[ 0 ].A01(),
		};

		m_pRenderer->SetPolygonFlatSettings( mat, pColour );

		if (   pSprite->m_RenderState.m_EffectIndex == E_Effect_PolygonStrokeAlpha 
			|| pSprite->m_RenderState.m_EffectIndex == E_Effect_PolygonStrokeIntensityAsAlpha 
			|| pSprite->m_RenderState.m_EffectIndex == E_Effect_PolygonTrail 
			|| pSprite->m_RenderState.m_EffectIndex == E_Effect_AM_AudioEqAlpha 
			|| pSprite->m_RenderState.m_EffectIndex == E_Effect_AM_AudioEqIntensityAsAlpha )
		{
			m_pRenderer->SetTextureVectorSettings( pSprite->m_Textures[0].m_pu, pSprite->m_Textures[0].m_pv, pSprite->m_Textures[0].m_su, pSprite->m_Textures[0].m_sv );
		}
	}

	if( m_pRenderer->ShaderUsage( eEffect ) & E_ShaderUsageFlags_MultiplyBounds )
	{
		m_pRenderer->SetPolygonMultiplySettings( GetColourTransformMatrix( (pSprite->m_RenderState.m_EffectParam>>16)&0xff ) );
	}

	if ( m_pRenderer->ShaderUsage( eEffect ) & E_ShaderUsageFlags_GradientBounds )
	{
		Maths::HMat43 col_mat;
		F32 pColour[ C_Renderer::m_NumGradientColours ][ 4 ];
		for(U32 i = 0; i < C_Renderer::m_NumGradientColours; i++) // Note is the same as 4 corners at the moment
		{
			pColour[ i ][ 0 ] = pSprite->m_Colours[ i ].R01();
			pColour[ i ][ 1 ] = pSprite->m_Colours[ i ].G01();
			pColour[ i ][ 2 ] = pSprite->m_Colours[ i ].B01();
			pColour[ i ][ 3 ] = pSprite->m_Colours[ i ].A01();
		}
		F32 pColourParam[4] = 
		{
			0.0f,
			0.0f,
			0.0f,
			0.0f,
		};
#ifndef CFG_GOLD
		if(g_UiDebugColourMatrixActive)
		{
			// Built the colour matrix
			Maths::HMat43 col_mat;
			
			if(m_pGradientTexture)
			{
				pColourParam[0] = ((F32)g_UiDebugPolygonShaderGradientParam+0.5f) / m_pGradientTexture->Height(); // Gradient index
			}
			col_mat.Translation( Maths::HVec3( (F32)g_UiDebugColourMatrixTransX, (F32)g_UiDebugColourMatrixTransY, 0.0f ) );
			col_mat = col_mat * Maths::HMat43(Maths::C_MatrixScale(Maths::HVec3( (F32)2.0f / g_UiDebugColourMatrixScale, (F32)2.0f / g_UiDebugColourMatrixScale, 0.0f ) ));
			col_mat = col_mat * Maths::HMat43(Maths::C_MatrixRotateRPH(Maths::DegToRad( (F32)g_UiDebugColourMatrixRotate ), 0.0f, 0.0f));

			m_pRenderer->SetPolygonGradientSettings( mat, col_mat, &pColour[ 0 ] [ 0 ], pColourParam );
		}
		else
#endif
		{
			if(m_pGradientTexture)
			{
				U8 param = ((pSprite->m_RenderState.m_EffectParam>>8)&0xff);
				pColourParam[0] = ((F32)param+0.5f) / m_pGradientTexture->Height(); // Gradient index
			}

			Maths::HMat43& col_mat = GetColourTransformMatrix( pSprite->m_RenderState.m_EffectParam&0xff );

			col_mat = col_mat;// * zeroMatrixZ;

			m_pRenderer->SetPolygonGradientSettings( mat, col_mat, &pColour[ 0 ][ 0 ], pColourParam );
		}
	}
	if ( pSprite->m_RenderState.m_EffectIndex == E_Effect_PolygonTrail )
	{
		U32 t =  pSprite->m_Colours[1].A()| (pSprite->m_Colours[1].R()<<8) | (pSprite->m_Colours[1].G()<<16) | (pSprite->m_Colours[1].B()<<24);
		F32 time = *((F32*)&t);

		U32 i =  pSprite->m_Colours[2].A()| (pSprite->m_Colours[2].R()<<8) | (pSprite->m_Colours[2].G()<<16) | (pSprite->m_Colours[2].B()<<24);
		F32 intensity = *((F32*)&i);

		U32 f =  pSprite->m_Colours[3].A()| (pSprite->m_Colours[3].R()<<8) | (pSprite->m_Colours[3].G()<<16) | (pSprite->m_Colours[3].B()<<24);
		F32 frequency = *((F32*)&f);

		m_pRenderer->SetPolygonTrailSettings( time, intensity, frequency );
	}
	if ( pSprite->m_RenderState.m_EffectIndex == E_Effect_AM_AudioEqAlpha || pSprite->m_RenderState.m_EffectIndex == E_Effect_AM_AudioEqIntensityAsAlpha )
	{
		m_pRenderer->SetAudioEqSettings( pSprite->m_Textures[1].m_pu, pSprite->m_Textures[1].m_pv, pSprite->m_Textures[1].m_su, pSprite->m_Textures[1].m_sv );
	}

	// Sets the current vertex and index shaders. Also sets the consts.
	m_pRenderer->SetEffect( eEffect, 0, (E_TextureAddressMode) pSprite->m_RenderState.m_EffectAddressMode );

	if ( pSprite->m_Type == E_SpriteType_TriList )
	{
		m_pRenderer->RenderTriList( pRenderInfo->m_VertexBuffer, pRenderInfo->m_IndexBuffer, pRenderInfo->m_uIndexCount, pRenderInfo->m_uVertexCount );
	}
	else
	if ( pSprite->m_Type == E_SpriteType_TriStrip )
	{
		S32 vertexStart = (pSprite->m_RenderState.m_EffectParam & 0xffff);
		S32 vertexCount = (pSprite->m_RenderState.m_EffectParam >> 16) & 0xffff;

		m_pRenderer->RenderTriStrip( pRenderInfo->m_VertexBuffer, vertexCount - vertexStart, vertexStart );
	}
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
