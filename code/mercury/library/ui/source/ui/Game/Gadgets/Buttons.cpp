#include "stdafx.h"

#include <misc_shared/Messaging/GameMessaging.h>
#include <system/misc/Random.h>	// [Blur2PC] Util::C_Random

#include "ui/UiInterface.h"
#include "ui/Game/Gadgets/ButtonController.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/UiXt.h"
#include "ui/Game/Timeline.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Game/Gadgets/Buttons.h"

#include <algorithm>

Util::C_Random gUiRand;

#ifndef CFG_GOLD
Debug::Bool		g_UiRenderBounds( "UI.RenderBounds", false );
Debug::Colour	g_UiRenderBoundsColour( "UI.RenderBoundsColour", 0.9f, 0.8f, 0.7f, 0.5f );
Debug::Bool		g_UiRenderGadgetNames( "UI.GadgetNameRender", false );
Debug::Float	g_UiRenderGadgetNameSize( "UI.GadgetNameSize", 16.0f, Debug::Range( 4.0f, 50.0f ) );
Debug::Bool		g_UiTimelineStopTime( "UI.Timeline.StopTime", false );
Debug::Float	g_UiTimelineCurrentTime( "UI.Timeline.CurrentTime", 0.0f, Debug::Range(0.0f, 10.0f) );
Debug::Bool		g_UiUseCachedMatrix( "ui.CacheMatrix", true );

Debug::Bool		g_ShowValidTimelineActiveMsg( "UI.Timeline.ShowValidMessages", false );
#endif

#ifdef PLATFORM_PC
Debug::Bool g_UiRenderMouseBoundingBox("UI.RenderMouseBoundingBoxes", false);
Debug::Bool g_UiRenderMouseBoxWhenOver("UI.RenderMouseBoundingBoxOver", true);
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

XT_TYPE( UI::C_GadgetBaseData::C_VectorBase, "UIVectorBase" )
	XT_FIELD( x, "x" )
	XT_FIELD( y, "y" )
	XT_FIELD( z, "z" )
XT_END_TYPE

XT_TYPE( UI::C_GadgetBaseData::C_Vector3Position, "UIVector3Position" )
	XT_FIELD( x, "x" )
	XT_FIELD( y, "y" )
	XT_FIELD( z, "z" )
XT_END_TYPE

XT_TYPE( UI::C_GadgetBaseData::C_Vector3Direction, "UIVector3Direction" )
	XT_FIELD( x, "x" )
	XT_FIELD( y, "y" )
	XT_FIELD( z, "z" )
XT_END_TYPE

XT_TYPE( UI::C_GadgetBaseData::C_Vector3Rotation, "UIVector3Rotation" )
	XT_FIELD( x, "x" )
	XT_FIELD( y, "y" )
	XT_FIELD( z, "z" )
XT_END_TYPE

XT_TYPE( UI::C_GadgetBaseData::C_Vector3Scale, "UIVector3Scale" )
	XT_FIELD( x, "x" )
	XT_FIELD( y, "y" )
	XT_FIELD( z, "z" )
XT_END_TYPE

XT_TYPE( UI::C_GadgetBaseData::C_Vector3Size, "UIVector3Size" )
	XT_FIELD( x, "x" )
	XT_FIELD( y, "y" )
	XT_FIELD( z, "z" )
XT_END_TYPE

XT_TYPE( UI::C_GadgetBase, "UIDummy" )
	XT_BASE( UI::C_GadgetBaseData )
XT_END_TYPE

XT_TYPE( UI::UISafeArea, "UISafeArea" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

XT_TYPE( UI::UIRenderState, "UIRenderState" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_PushState, "push_state" )
	XT_FIELD( m_AlterFlags, "alter_flags" )
	XT_FIELD( m_State, "render_state" )
XT_END_TYPE

XT_TYPE( UI::UIRenderBranch, "UIRenderBranch" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_DoSort, "do_sort" )
XT_END_TYPE

XT_TYPE( UI::UIDummyLerp, "UILerpDummy" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_fLerp, "lerp" )
XT_END_TYPE

#ifdef PLATFORM_PC
XT_TYPE( UI::C_GadgetBaseData::C_MouseData, "UIMouseData" )
	XT_FIELD( clickable, "clickable" )
	XT_FIELD( bb_top, "bb_top" )
	XT_FIELD( bb_bottom, "bb_bottom" )
	XT_FIELD( bb_left, "bb_left" )
	XT_FIELD( bb_right, "bb_right" )
	XT_FIELD( lclick_msg, "lclick_msg" )
	XT_FIELD( rclick_msg, "rclick_msg" )
	XT_FIELD( isover_msg, "isover_msg" )
XT_END_TYPE
#endif

XT_ABSTRACT_TYPE( UI::C_GadgetBaseData, "UIGadgetBaseData" )
	XT_CALLBACK( UI::C_GadgetBaseData::SorceryCallback )
	XT_FIELD( m_Name,				"name" )
	XT_FIELD( m_Activated,			"activated" )
	XT_FIELD( m_Selected,			"selected" )
	XT_FIELD( m_pParentGadget, 		"parent" )
	XT_FIELD( m_Position,			"pos" )
	XT_FIELD( m_Rotation,			"rot" )
	XT_FIELD( m_Scale,				"scale" )
	XT_FIELD( m_Size,				"size" )
	XT_FIELD( m_ColourStyle,		"colour_style" )
	XT_FIELD( m_Alpha,				"alpha" )
	XT_FIELD( m_Intensity,			"intensity" )
	XT_FIELD( m_Justify,			"justify" )
	XT_FIELD( m_ParentAttach,		"parent_attach" )
	XT_FIELD( m_Layer,				"layer" )
	XT_FIELD( m_Panel,				"panel" )
	XT_FIELD( m_BlendMode,			"blend_mode" )
	XT_FIELD( m_EffectIndex,		"effect_index" )
	XT_FIELD( m_TextureAddressMode,	"texture_address" )
	XT_FIELD( m_TextureParams,		"textures" )
	XT_FIELD( m_Timeline,			"time_lines" )
	XT_FIELD( m_Flags,				"flags" )
#ifdef _UI_AUTO_CLIPWINDOW
	XT_FIELD( m_ClipPos,			"clip_pos" )
	XT_FIELD( m_ClipSize,			"clip_size" )
#endif
#ifdef PLATFORM_PC
	XT_FIELD( m_Mouse,				"mouse" )
#endif
XT_END_TYPE

XT_TYPE( UI::C_GadgetBaseData::E_Flag, "UIGadgetFlags" )
	XT_FLAG_STORAGE(XtU32)
	XT_FLAG( UI::C_GadgetBaseData::E_Flag_ExportToLua, "ExportToLua" )
	XT_FLAG( UI::C_GadgetBaseData::E_Flag_InheritAlpha, "InheritAlpha" )
	XT_FLAG( UI::C_GadgetBaseData::E_Flag_InheritIntensity, "InheritIntensity" )
	XT_FLAG( UI::C_GadgetBaseData::E_Flag_InheritRotation, "InheritRotation" )
	XT_FLAG( UI::C_GadgetBaseData::E_Flag_InheritScale, "InheritScale" )
	XT_FLAG( UI::C_GadgetBaseData::E_Flag_InheritTranslation, "InheritTranslation" )
	XT_FLAG( UI::C_GadgetBaseData::E_Flag_BoundsCulling, "BoundsCulling" )
XT_END_TYPE

XT_TYPE( UI::UIRenderState::E_AlterFlags, "UIRenderStateAlterFlags" )
	XT_FLAG_STORAGE(XtU8)
	XT_FLAG( 1 << UI::UIRenderState::E_AlterFlags_CullMode, "CullMode" )
	XT_FLAG( 1 << UI::UIRenderState::E_AlterFlags_ClipWindow, "ClipWindow" )
	XT_FLAG( 1 << UI::UIRenderState::E_AlterFlags_DepthBuffer, "DepthBuffer" )
	XT_FLAG( 1 << UI::UIRenderState::E_AlterFlags_StencilBuffer, "StencilBuffer" )
	XT_FLAG( 1 << UI::UIRenderState::E_AlterFlags_ColourWrite, "ColourWrite" )
XT_END_TYPE

namespace UI
{

// Culling singleton
C_GadgetBaseData::C_Rect C_GadgetBaseData::C_Rect::m_sCullingInstance;

void C_GadgetBaseData::SorceryCallback( XtEventType type, XtRecord* pRecord, C_GadgetBaseData* pBase, XtCallbackContext* pContext )
{
	switch ( type )
	{
	case kXtEtConstruct:
		break;

	case kXtEtDestruct:
		break;

	case kXtEtUser :
		switch( pContext->tag )
		{
		case 'RRef':
			pContext->tag = 0;
			return;

		case UiXt::E_CallbackContext_Setup:
			{
				const char *cb_typenames[] = 
				{
					"alpha",			// E_CallbackContext_UpdateAlpha
					"colour_style",		// E_CallbackContext_UpdateColour
					"intensity",		// E_CallbackContext_UpdateIntensity
				};
				assert( pContext );
				UiXt::C_CallbackAssignContext *p = (UiXt::C_CallbackAssignContext *)pContext;

				for( S32 i = 0; i < lengthof( cb_typenames ); i++)
					if( stricmp( p->m_pLink->m_pComponentName, cb_typenames[ i ] ) == 0 )
					{
						pContext->tag = i;
						return;
					}
			}
			break;

		case E_CallbackContext_UpdateAlpha: 
			pBase->DirtySet( E_Dirty_Alpha, true );
			break;

		case E_CallbackContext_UpdateIntensity:
			pBase->DirtySet( E_Dirty_Colour, true );
			break;

		case E_CallbackContext_UpdateColour:
			pBase->m_InternalFlags &= ~E_InternalFlag_HasColourBinding;
			break;
		}
	}
}

C_GadgetBaseData::C_GadgetBaseData( C_GadgetBase* self ) : 
	m_pParentGadget( NULL ),
	m_ParentLink(self),
	m_Activated( true ),
	m_Selected( true ),
	m_Flags( E_Flag_InheritRotation | E_Flag_InheritScale | E_Flag_InheritTranslation ),
	m_Alpha( 1.0f ),
	m_Intensity( 1.0f ),
	m_SpriteColours( 0xffffffff ),
	m_BlendMode( E_UiBlendMode_Alpha ),
	m_EffectIndex( E_Effect_MultiTexture ),
	m_Justify( E_Justify_TopLeft ),
	m_ParentAttach( E_Justify_None ),
	m_Layer( 0 ),
	m_Panel( 0 ),
#ifdef _UI_AUTO_CLIPWINDOW
	m_ClipPos( -1.0f, -1.0f ),
	m_ClipSize( -1.0f, -1.0f ),
#endif
	m_InternalFlags(0),
	m_AlphaInherit( -1.0f ),
	m_IntensityInherit( -1.0f )
{
	m_RenderState.Init();

	m_Name = XtSz8(NULL);
	m_ColourStyle = XtSz8(NULL);
	Store( Maths::HMat43(), &m_GadgetMatrix );
}

C_GadgetBaseData::~C_GadgetBaseData()
{
	XtUt::Abandon(&m_ParentChildList);
}

const bool C_GadgetBaseData::IsTransformed( ) const
{
	return(m_Scale.x!=1.0f || m_Scale.y!=1.0f || m_Scale.z!=1.0f || m_Rotation.x!=0.0f || m_Rotation.y!=0.0f || m_Rotation.z!=0.0f);
}

Maths::HMat43 C_GadgetBaseData::TransformMatrix( const bool scale, const bool rotate, const bool translate )
{
	Maths::HMat33 ident;
	ident.SetIdentity();

	Maths::HMat33 s( scale ? Maths::HMat33( Maths::C_MatrixScale( m_Scale ) ) : ident );
	Maths::HMat33 r( rotate ? Maths::HMat33( Maths::C_MatrixRotatePHR( Maths::DegToRad( m_Rotation.x ), Maths::DegToRad( m_Rotation.y ), Maths::DegToRad( m_Rotation.z ) ) ): ident );
	Maths::HVec3 t( translate ? m_Position : Maths::HVec3( 0, 0, 0) );

	return Maths::HMat43( s * r, t );
}

char *C_GadgetBaseData::CopyCreateString( const char *string )
{
	if(string == NULL)
		return NULL;

	const size_t len = strlen( string );
	if( len == 0)
		return NULL;

	// Allocate
	char *p = (char *)m_XtAllocator.New( len + 1 );
	if( p == NULL )
		return NULL;

	// Make a copy
	STRCPY_S( p, len + 1, string );

	return p;
}

void C_GadgetBaseData::SetName( const char *name )
{
	if( m_Name )
	{
		m_XtAllocator.Delete( const_cast<char*>( m_Name.Text() ) );
	}

	m_Name = XtSz8( CopyCreateString( name ) );
}

void C_GadgetBaseData::SetParent(C_GadgetBase* parent)
{
	if ((m_pParentGadget = parent) != 0)
		XtUt::PushBack(&parent->m_ParentChildList, &m_ParentLink);
	else
		XtUt::Detach(&m_ParentLink);
}

void C_GadgetBaseData::SetColourStyle( const char *colour_style )
{
	if( m_ColourStyle )
	{
		m_XtAllocator.Delete( const_cast<char*>( m_ColourStyle.Text() ) );
	}

	m_ColourStyle = XtSz8( CopyCreateString( colour_style ) );

	m_InternalFlags &= ~E_InternalFlag_HasColourBinding;
}

void C_GadgetBaseData::SetTextureCapacity( const S32 capacity )
{
	if( m_TextureParams.count == capacity )
		return;

	if( m_TextureParams.elements )
	{
		m_XtAllocator.Delete( m_TextureParams.elements );

		m_TextureParams.elements = NULL;
		m_TextureParams.count = 0;
	}

	if(!capacity )
		return;

	C_TextureParam* pNewMem = (C_TextureParam *)m_XtAllocator.New( XtGetType<C_TextureParam>(), capacity  );
	if(!pNewMem )
		return;

	m_TextureParams.elements = pNewMem;
	m_TextureParams.count = capacity;
}

void C_GadgetBaseData::BindColourStyle( )
{
	if( !m_ResolveColours.SetByName( m_ColourStyle, 1.0f, 1.0f ) )
	{
		m_ResolveColours.Set(C_UIColour(0xFFFFFFFF));
	}

	m_InternalFlags |= E_InternalFlag_HasColourBinding;
}

void C_GadgetBaseData::ResolveColourStyle( const F32 intensity, const F32 alpha )
{
	if ((m_InternalFlags & E_InternalFlag_HasColourBinding) == 0)
		BindColourStyle();

	F32 a = Maths::Clamp< F32 >( alpha, 0.0f, 1.0f );
	F32 i = Maths::Clamp< F32 >( intensity, 0.0f, 2.0f );

	m_SpriteColours.Set(m_ResolveColours, a, i);
}

void C_GadgetBaseData::ResolveTextureParams( )
{
	// Resolve texture name into index
	for(XtI32 a = 0;a < m_TextureParams.count; a++)
	{
		m_TextureParams[ a ].Bind();
	}
}

void C_GadgetBaseData::ReplaceTimeLineLabel( const S32 sIndex, const char* pNewLabel )
{
	if ( ( sIndex >= m_Timeline.count ) || !pNewLabel || !pNewLabel[ 0 ] )
		return;

	m_XtAllocator.Delete( const_cast<char*>( m_Timeline[ sIndex ].m_Name.Text() ) );
	m_Timeline[ sIndex ].m_Name = XtSz8( CopyCreateString( pNewLabel ) );
}

// Gdaget Base
const S32 C_GadgetBase::LuaSerialise( lua_State* pState, const U32 context, UiXt::C_PointerMap& pointerMap )
{ 
	const XtRecord* pRootRecord = C_ScreenControlManager::Instance().Get( m_ID.Context() ).GetRootRecord();
	if( !pRootRecord )
	{
		LOGERROR( g_Log, DBSTR "Root record invalid");
		return 0;
	}
	
	UiXt::MakeLuaFromDefImpl( pState, m_pXtType, (const U8 *)this, pointerMap, pRootRecord );

	return 1;
}

const S32 C_GadgetBase::LuaDeserialise( lua_State* pState, UiXt::C_PointerMap& pointerMap )
{
	const XtRecord* pRootRecord = C_ScreenControlManager::Instance().Get( m_ID.Context() ).GetRootRecord(); // Okay to be NULL
	
	UiXt::GetLuaFromDefImpl( pState, m_pXtType, (U8* )this, m_XtAllocator, pointerMap, pRootRecord );

	return 1;
}

void C_GadgetBase::BindTimeline( )
{
	if( m_Timeline.count == 0 )
		return;

	static Util::C_String id_token( "[id]" );

	Util::C_String id_string;
	id_string.Printf("%d", m_ID.Number() );

	for( XtI32 i = 0; i < m_Timeline.count; i++) 
	{
		m_Timeline[ i ].BindImpl( m_pXtType, (const U8 *)this ); 
		
		// Start and end
		ReplaceToken( id_token, id_string, m_Timeline[ i ].m_OnRestCmd );
		ReplaceToken( id_token, id_string, m_Timeline[ i ].m_OnStartCmd );

		// Event command list
		for( XtI32 j = 0; j < m_Timeline[ i ].m_Events.count; j++ )
			ReplaceToken( id_token, id_string, m_Timeline[ i ].m_Events[ j ].m_Command );
	}

	CheckSleepingTimeline();
}

void C_GadgetBase::ReplaceToken( const Util::C_String& token, const Util::C_String& replacement, XtSz8& sz8 )
{
	if( !sz8 )
		return;

	if( !sz8[ 0 ] )
		return;

	Util::C_String tmp( sz8.Text() );
	if( tmp.ISearch( token ) == -1 )
		return;

	tmp = tmp.IReplace( token, replacement );

	if( !m_XtAllocator.Delete( const_cast<char*>( sz8.Text() ) ) )
	{
		LOGERROR( g_Log, DBSTR "Failed to free sz8 string");
		return;
	}

	sz8 = XtSz8( CopyCreateString( tmp.Chars() ) );
}

void C_GadgetBase::Init()
{
#ifdef PLATFORM_PC
	// Invalidate
	m_Mouse.clickable		= false;
	m_Mouse.isover			= false;
	m_Mouse.wasover			= false;
	m_Mouse.lclick_msg		= -1;
	m_Mouse.rclick_msg		= -1;
	m_Mouse.isover_msg		= -1;
	m_Mouse.bb_top			= FLT_MAX;
	m_Mouse.bb_bottom		= FLT_MAX;
	m_Mouse.bb_left			= FLT_MIN;
	m_Mouse.bb_right		= FLT_MIN;
	m_PrevMousePosX				= 0.0f;
	m_PrevMousePosY				= 0.0f;
#endif
}

void C_GadgetBase::Setup( lua_State *pState )
{
	// GRR! Copy from the sorcery data
	m_RenderState.m_BlendMode			= (U8)m_BlendMode;
	m_RenderState.m_EffectIndex			= (U8)m_EffectIndex;
	m_RenderState.m_EffectAddressMode	= (U8)m_TextureAddressMode;

	// Resolve then see if we have any old style overides
	ResolveColourStyle( m_Intensity, m_Alpha );

	// No parent? attach to screen
	if( m_ParentAttach != E_Justify_None && Parent() == NULL && gSimple2D.IsPanel3D( m_Panel ) == false )
	{
		m_Position.x += ( gSimple2D.GetJustificationOffset( m_ParentAttach, m_Panel ).X() * gSimple2D.ScreenWidth() ) + gSimple2D.ScreenLeft();
		m_Position.y += ( gSimple2D.GetJustificationOffset( m_ParentAttach, m_Panel ).Y() * UI_VSCREEN_SIZEY );
	}

	// Get parent depth safety check
	CheckParentDepth();

	ResolveTextureParams();

	SetDefaultMouseBoundingBox();

#ifdef _GADGET_BOUNDS_CULLING
	// Get the four points of the bounds		
	const Maths::HVec3	p_size(m_Size.x, m_Size.y, 1.0f);	// TODO - z scaling
	for( U32 i = 0; i < 4; i++)
		m_BoundCorners[ i ] = (gSimple2D.GetJustifiedCorner( m_Justify, i, m_Panel ) * p_size);
#endif

	m_Up = gSimple2D.IsPanel3D( m_Panel ) ? -1.0f : 1.0f;
}

const bool C_GadgetBase::CheckParentDepth( )
{
	S32 depth = 0;
	const S32 max_depth = 16;
	for( const C_GadgetBase* p = Parent(); p; p = p->Parent(), depth++ )
		if( depth > max_depth )
		{
			const char *p = m_Name; // GRR Ps3

			LOGWARN( g_Log, "Parent depth of %s exceeded %d : possible cyclic link", ((p && p[0]) ? p : "unknown"), max_depth );
			SetParent(0); // Error : clear
			return false;
		}

	return true;
}

// Called after all pointers have been re-assigned from looking at the sorcery creation data to the new cloned data
void C_GadgetBase::PostInit()
{
	ResolveInheritedColours();
}

void C_GadgetBase::ResolveInheritedColours()
{
	if( !m_ColourStyle || !*m_ColourStyle )
		return;

	F32 alpha = m_Alpha;
	F32 intensity = m_Intensity; 

	if( FlagSet( E_Flag_InheritAlpha ) || FlagSet( E_Flag_InheritIntensity ) )
	{
		for ( const C_GadgetBase* p = Parent(), * t = this; p; t = p, p = p->Parent() )
		{
			// Intensity
			if ( t->FlagSet( E_Flag_InheritIntensity ) )
			{
				if( p->m_Intensity > 1.0f )
					intensity += (p->m_Intensity-1.0f);
				else
				if( p->m_Intensity < 1.0f )
					intensity *= p->m_Intensity;
			}
			// Alpha
			if ( t->FlagSet( E_Flag_InheritAlpha ) )
			{
				alpha *= p->m_Alpha;
			}
		}
	}
	ResolveColourStyle( intensity, alpha );
}

// Check for multiple effects on one variable and to deny sleep
void C_GadgetBase::CheckSleepingTimeline()
{
	for( XtI32 i = 0; i < m_Timeline.count; i++ )
	{
		// Get timeline (A)
		C_Timeline& ta = m_Timeline[ i ];
		if( ta.m_bDenySleep )
			continue;

		for( XtI32 k = 0; k < ta.m_Effects.count; k++ )
		{
			// Get effect (A)
			C_Timeline::C_Effect& ea = ta.m_Effects[ k ];
			if( !ea.m_ComponentName )
				continue;

			// Run through component of effect (A)
			const char* ap = ea.m_ComponentName;
			char a_name[ 256 + 1 ];
			while( *ap )
			{
				// Copy next (A) component name
				S32 a_cnt;
				for( a_cnt = 0; a_cnt < 256 && *ap != ';' && *ap != NULL; a_cnt++, ap++ )
					a_name[ a_cnt ] = *ap;
				a_name[ a_cnt ] = NULL;

				// Skip component seperator
				while( *ap == ';' || *ap == ' ' )
					ap++;

				for( XtI32 j = 0; j < m_Timeline.count && !ta.m_bDenySleep; j++ )
				{
					// Don't compare against myself that would be stupid
					if( j == i )
						continue;

					// Get timeline (B)
					C_Timeline& tb = m_Timeline[ j ];
					for( XtI32 j = 0; j < tb.m_Effects.count; j++ )
					{
						// Get effect (B)
						C_Timeline::C_Effect& eb = tb.m_Effects[ j ];
						if( !eb.m_ComponentName )
							continue;

						// Run through component of effect (B)
						const char* bp = eb.m_ComponentName;
						char b_name[ 256 + 1 ];
						while( *bp )
						{
							// Copy next (B) component name
							S32 b_cnt;
							for( b_cnt = 0; b_cnt < 256 && *bp != ';' && *bp != NULL; b_cnt++, bp++ )
								b_name[ b_cnt ] = *bp;
							b_name[ b_cnt ] = NULL;

							// Skip component seperator
							while( *bp == ';' || *bp == ' ' )
								bp++;

							// Compare components
							if( strcmp( a_name, b_name ) == 0 )
							{
//								if( !ta.m_bDenySleep ) LOGDEBUG( g_Log, "[%s] : Timeline [%s] is denied sleep due to [%s]", m_Name.string ? m_Name.string : "[Unnamed]", (ta.m_Name.string ? ta.m_Name.string : "[Unnamed]"), a_name );
//								if( !tb.m_bDenySleep ) LOGDEBUG( g_Log, "[%s] : Timeline [%s] is denied sleep due to [%s]", m_Name.string ? m_Name.string : "[Unnamed]", (tb.m_Name.string ? tb.m_Name.string : "[Unnamed]"), b_name );
								ta.m_bDenySleep = true;
								tb.m_bDenySleep = true;
								break;
							}
						}
					}
				}
			}
		}
	}
}


void C_GadgetBase::Select( const bool bSelect, const bool bSendTimelineMsg )
{
	if( m_Selected == bSelect )
		return;

	m_Selected = bSelect;

	if ( bSendTimelineMsg )
	{
		SendPrivateTimelineMessaage( bSelect ? "gadget_selected" : "gadget_deselected", true );
		SendPrivateTimelineMessaage( bSelect ? "gadget_deselected" : "gadget_selected", false );
	}
}


void C_GadgetBase::Activate( const bool bActivate, const bool bSendTimelineMsg )
{
	if( m_Activated == bActivate )
		return;

	m_Activated = bActivate; 

	if ( bSendTimelineMsg )
	{
		SendPrivateTimelineMessaage( bActivate ? "gadget_activated" : "gadget_deactivated", true );
		SendPrivateTimelineMessaage( bActivate ? "gadget_deactivated" : "gadget_activated", false );
	}
}


void C_GadgetBase::SendPrivateTimelineMessaage( const char* msg, const bool active, const F32 time, const bool reset )
{
	C_UiTimelineMessage message( msg, active, time, reset );

	MessageHandler(&message);
}

void C_GadgetBase::CalcTransformHierarchy( TransformIndex& ti )
{
	Maths::HMat43 m;

	if ( !CalcTransformHierarchy( m ) )
	{
		ti = 0;
		return;
	}

	ti = gSimple2D.PopPostTransformMatrix();
	Maths::HMat43& mat = gSimple2D.GetPostTransformMatrix( ti );
	mat = m;
#ifdef PLATFORM_PC
	gSimple2D.Blur2PCNoteTransform( this, ti );	// [Blur2PC] sprites that follow a car (C_Simple2D::Blur2PCSetAnchor)
#endif
}

// TODO -> This is really really slow -> prestore m0 and m1 if we end up with many parented objects
bool C_GadgetBase::CalcTransformHierarchy( Maths::HMat43& mat, const bool scale, const bool rotate, const bool translate )
{
	const bool transformed = ( IsTransformed() || Parent() );
	const bool do_all = (scale && rotate && translate);

#if !defined( CFG_GOLD )
	if( g_UiUseCachedMatrix )
#endif
	if( do_all && DirtySet( E_Dirty_Matrix ) == false )
	{
		mat = Matrix();
		return transformed; // Well possibly anyhoo
	}

	if( !transformed )
	{
		mat.SetIdentity();
		mat.Translation( m_Position );

		SetMatrix(mat);
		DirtySet( E_Dirty_Matrix, false );
		return false;
	}

	// my transform
	mat = TransformMatrix( scale, rotate, translate );

	if( Parent() )
	{
		Maths::HMat43 m0; // Prestore this
		if( FlagSet( E_Flag_InheritTranslation ) )
			Parent()->CalcTransformHierarchy( m0 );
		else
			m0.SetIdentity();

		Maths::HVec3 t( mat.Translation() + ( Parent()->m_Size * ( gSimple2D.GetJustificationOffset( m_ParentAttach, m_Panel ) - gSimple2D.GetJustificationOffset( Parent()->m_Justify, Parent()->m_Panel ) ) ) );
		mat.Translation( t * m0 );

		if( FlagSet( E_Flag_InheritScale ) && FlagSet( E_Flag_InheritRotation ) && FlagSet( E_Flag_InheritTranslation ) )
		{
			mat.Rotation( mat.Rotation() * m0.Rotation() );
		}
		else
		{
			Maths::HMat43 m1; // Prestore this
			Parent()->CalcTransformHierarchy( m1, FlagSet( E_Flag_InheritScale ), FlagSet( E_Flag_InheritRotation ), true );
			mat.Rotation( mat.Rotation() * m1.Rotation() );
		}
	}

	// Cache this
	if( !do_all )
		return true;

	SetMatrix(mat);
	DirtySet( E_Dirty_Matrix, false );
	return true;
}

void C_GadgetBase::UpdateEffects( )
{
#ifndef CFG_GOLD
	if( g_UiTimelineStopTime == true )
	{
		for( XtI32 i = 0; i < m_Timeline.count; i++)
		{
			m_Timeline[ i ].Reset( true );
			m_Timeline[ i ].SetTime( g_UiTimelineCurrentTime );
		}
	}
	else
#endif
	{
		// Reset the timeline effects
		for( XtI32 i = 0; i < m_Timeline.count; i++)
			m_Timeline[ i ].Reset( );

		for( XtI32 i = 0; i < m_Timeline.count; i++)
			m_Timeline[ i ].Update( theFETimer.FrameTime() );
	}
}

#ifdef _GADGET_BOUNDS_CULLING
// Note this should only be called during render else the autotransform thing will/could be wrong
void C_GadgetBase::CalcBoundRect( )
{
	// Get current transform
	TransformIndex autoTransformIndex = gSimple2D.GetAutoTransform();

	// Transform the bounds corners
	Maths::HVec3 v[4];
	if( autoTransformIndex )
	{
		for( U32 i = 0; i < 4; i++)
			v[ i ] = m_BoundCorners[ i ] * gSimple2D.GetPostTransformMatrix( autoTransformIndex  );
	}
	else
	{
		for( U32 i = 0; i < 4; i++)
			v[ i ] = (m_BoundCorners[ i ] + m_Position + gSimple2D.GetAutoTransformOffset());
	}
	// Clear and add points
	m_BoundRect = C_Rect();
	for( U32 i = 0; i < 4; i++)
		m_BoundRect.UpdateExtents( v[ i ] );
}
#endif

const bool C_GadgetBaseData::C_Rect::Overlaps( const C_Rect& _in )
{
	return ( _in.right > left && _in.left < right && _in.bottom > top && _in.top < bottom );
}

#ifdef _GADGET_BOUNDS_CULLING
const bool C_GadgetBase::IsCulled( )
{
	if( !FlagSet( E_Flag_BoundsCulling ) )
		return false;

	CalcBoundRect();

	if( m_BoundRect.Overlaps( C_Rect::CullingInstance() ) )
		return false;

	return true;
}
#endif

void C_GadgetBase::Render()
{
#ifndef CFG_GOLD
#ifdef _GADGET_BOUNDS_CULLING
	if( !FlagSet( E_Flag_BoundsCulling ) && g_UiRenderBounds == true )
		CalcBoundRect();
#endif
#endif

	// Off by default as this is pretty slow to because it does a colour lookup each frame (TODO: Prestore)
	bool doa = FlagSet( E_Flag_InheritAlpha );
	bool doi = FlagSet( E_Flag_InheritIntensity );

	F32 alpha		= m_Alpha;
	F32 intensity	= m_Intensity;

	if ( doa || doi )
	{
		for ( const C_GadgetBase* p = Parent(), *t = this; p; t = p, p = p->Parent() )
		{
			// Alpha

			if (doa)
			{
				if (!t->FlagSet( E_Flag_InheritAlpha ))
				{
					if (!doi)
						break;

					doa = false;
				}
				else
					alpha *= p->m_Alpha;
			}

			// Intensity

			if (doi)
			{
				if (!t->FlagSet( E_Flag_InheritIntensity ))
				{
					if (!doa)
						break;

					doi = false;
				}
				else
				{
					if ( p->m_Intensity > 1.0f )
						intensity += ( p->m_Intensity - 1.0f );
					else
					if ( p->m_Intensity < 1.0f )
						intensity *= p->m_Intensity;
				}
			}
		}
	}

	int iNewIntensity	= Maths::Clamp<int>((int)(intensity * 256.0f), 0, 511);
	int iOldIntensity	= Maths::Clamp<int>((int)(m_IntensityInherit * 256.0f), 0, 511);
	int iNewAlpha		= Maths::Clamp<int>((int)(alpha * 255.0f), 0, 255);
	int iOldAlpha		= Maths::Clamp<int>((int)(m_AlphaInherit * 255.0f), 0, 255);

	if ((iNewIntensity != iOldIntensity) || (iNewAlpha != iOldAlpha) || (m_IntensityInherit < 0.0f) || (m_AlphaInherit < 0.0f))
	{
		m_IntensityInherit	= Maths::Clamp<F32>(intensity, 0.0f, 2.0f);
		m_AlphaInherit		= Maths::Clamp<F32>(alpha, 0.0f, 1.0f);

		DirtySet( E_Dirty_Alpha, true );
		DirtySet( E_Dirty_Colour, true );

		ResolveColourStyle( m_IntensityInherit, m_AlphaInherit );
	}

#ifndef CFG_GOLD
#ifdef _GADGET_BOUNDS_CULLING
	if( g_UiRenderBounds == true )
	{
		// Store current auto transform
		TransformIndex autoTransformIndex = gSimple2D.GetAutoTransform();
		gSimple2D.SetAutoTransform( 0 );

		C_UIColour colour( 
			(U8)(g_UiRenderBoundsColour->a*255.0f), 
			(U8)(g_UiRenderBoundsColour->r*255.0f), 
			(U8)(g_UiRenderBoundsColour->g*255.0f),
			(U8)(g_UiRenderBoundsColour->b*255.0f)
		);

		// Add debug graphic
		gSimple2D.AddSprite(gSimple2D.TopLayer(), Maths::HVec3( m_BoundRect.left, m_BoundRect.top, 0.0f) , m_BoundRect.right - m_BoundRect.left, m_BoundRect.bottom - m_BoundRect.top, colour);
		gSimple2D.AddJustification( E_Justify_TopLeft );
		gSimple2D.AddTexUVs(0,0,0,0,1.0f,1.0f);

		// Restore current auto transform
		gSimple2D.SetAutoTransform( autoTransformIndex );
	}
#endif
	if( g_UiRenderGadgetNames == true )
	{
		// stop using auto-transform
		TransformIndex autoTransformIndex = gSimple2D.GetAutoTransform();
		gSimple2D.SetAutoTransform( 0 );

		C_WideString128 wbuffer( ( m_Name && m_Name[0] ) ? m_Name.string : "unknown" );

		C_Vector3Position pos = m_Position;

		if ( autoTransformIndex )
		{
			pos.x = gSimple2D.GetPostTransformMatrix( autoTransformIndex ).AW().X();
			pos.y = gSimple2D.GetPostTransformMatrix( autoTransformIndex ).AW().Y();
			pos.z = gSimple2D.GetPostTransformMatrix( autoTransformIndex ).AW().Z() - 1.0f;
		}

		// Backing
		const F32 len = gFont.StringLength( wbuffer, (F32)g_UiRenderGadgetNameSize ) * 1.1f;
		gSimple2D.AddSprite( gSimple2D.TopLayer(), pos, len, (F32)g_UiRenderGadgetNameSize, 0xffff00ff, m_Panel );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddTexUVs( 0, 0, 0, 0, 1.0f, 1.0f );

		// Name
		pos.z -= 1.0f;
		gFont.DrawDebugText( gSimple2D.TopLayer(), pos, E_Justify_MiddleCentre, wbuffer, 0xffffffff, (F32)g_UiRenderGadgetNameSize, 0, m_Panel );

		// start using auto-transform again
		gSimple2D.SetAutoTransform( autoTransformIndex );
	}
#endif

#ifdef PLATFORM_PC
	if(g_UiRenderMouseBoundingBox && m_Mouse.clickable && m_Activated )
	{
		if(!g_UiRenderMouseBoxWhenOver || m_Mouse.isover || m_Mouse.wasover)
		{
			int button_colour = 0x40ffffff;
			if(m_Mouse.isover && !m_Mouse.wasover) // First frame in AABB?
				button_colour = 0x400000ff;
			else
			if(!m_Mouse.isover && m_Mouse.wasover) // Left the AABB?
				button_colour = 0x40ff0000;

			F32 width = m_Mouse.bb_right - m_Mouse.bb_left;
			F32 height = m_Mouse.bb_bottom - m_Mouse.bb_top;
			F32 x = m_Mouse.bb_left + (C_Simple2D::m_JustificationOffsets[m_Justify].X() * width);
			F32 y = m_Mouse.bb_top + (C_Simple2D::m_JustificationOffsets[m_Justify].Y() * height);

			// stop using auto-transform
			TransformIndex autoTransformIndex = gSimple2D.GetAutoTransform();
			gSimple2D.SetAutoTransform( 0 );

			gSimple2D.AddSprite(gSimple2D.TopLayer(), Maths::HVec3( x, y, m_Position.z ), width, height, button_colour);
			gSimple2D.AddJustification(m_Justify);
			gSimple2D.AddTexUVs(0,0,0,0,1.0f,1.0f);

			TransformIndex ti;
			CalcTransformHierarchy( ti );

			gSimple2D.AddPostTransformIndex( ti );

			// start using auto-transform again
			gSimple2D.SetAutoTransform( autoTransformIndex );
		}
	}
#endif
}

void C_GadgetBase::FrameUpdate()
{
#ifdef PLATFORM_PC
	bool leftMouseDown = UI::UiInterface()->GetLeftMouseDown();
	F32 mousePosX = 0.0f, mousePosY = 0.0f;
	UI::UiInterface()->GetMousePos(mousePosX, mousePosY);

	m_Mouse.wasover = m_Mouse.isover;
	m_Mouse.isover = false;

//	if ( CheckUpdate() )
	{
		if(m_Mouse.clickable)
		{
			F32 x, y;
			if(UI::UiInterface()->GetMousePos(x, y))
			{
				x *= gSimple2D.ScreenWidth();
				x += gSimple2D.ScreenLeft();

				y *= UI_VSCREEN_SIZEY;

				Maths::HMat43 m;
				CalcTransformHierarchy( m );
				Maths::HMat44 mat( m );

				// inverse transform mouse coord into object space
				Maths::HVec3 mouse_pos = Maths::HVec3( x - m_Position.x, y - m_Position.y, 0.0f ) * mat.Inverse();

				x = mouse_pos.X() + m_Position.x;
				y = mouse_pos.Y() + m_Position.y;

				m_Mouse.isover = (x >= m_Mouse.bb_left && x <= m_Mouse.bb_right && y >= m_Mouse.bb_top && y <= m_Mouse.bb_bottom);
			}

			if ( m_Activated )
			{
				if(m_Mouse.isover && !m_Mouse.wasover)
				{
					_UI_MESSAGE4( E_UiMsg_MouseInBox, 0, 0, 0, (F64)m_ID );
				}
				else
				if(!m_Mouse.isover && m_Mouse.wasover)
				{
					_UI_MESSAGE4( E_UiMsg_MouseOutBox, 0, 0, 0, (F64)m_ID );
				}
				else
				if(m_Mouse.isover)
				{
					if(leftMouseDown || mousePosX != m_PrevMousePosX || mousePosY != m_PrevMousePosY)
					{
						_UI_MESSAGE4( E_UiMsg_MouseInBox, 0, 0, 0, (F64)m_ID );

						m_PrevMousePosX = mousePosX;
						m_PrevMousePosY = mousePosY;
					}
				}
			}
#ifdef _FIX_WIN32MSG_
			if ( UiInterface()->UseMouseInput() && m_Mouse.isover && m_Mouse.isover_msg != -1 )
			{
				_UI_MESSAGE4( m_Mouse.isover_msg, 0, 0, 0, (F64)m_ID );
			}
#endif
		}
	}
#endif
}

const S32 C_GadgetBase::FindChildren( ButtonArray* children, const bool all, const bool all_context, const E_UIButtonType type )
{
	if( !all_context )
		return C_ScreenControlManager::Instance().Get( m_ID.Context() ).FindChildren( this, children, all, type );

	S32 cnt = 0;
	for( S32 i = 0; i < C_ScreenControlManager::Instance().Count(); i++ )
		cnt += C_ScreenControlManager::Instance().Get( i ).FindChildren( this, children, all, type );

	return cnt;
}

C_GadgetBase* C_GadgetBase::FindChildByName( const char* pName )
{
	return C_ScreenControlManager::Instance().Get( m_ID.Context() ).FindChildByName( this, pName );
}

C_GadgetBase* C_GadgetBase::ArrayFindByName( ButtonArray& array, const char* pName )
{
	for(S32 i = 0; i < array.Size(); i++)
		if(array[ i ]->m_Name && stricmp( array[ i ]->m_Name, pName ) == 0 )
			return array[ i ];

	return NULL;
}

void C_GadgetBase::SetDefaultMouseBoundingBox()
{
#ifdef PLATFORM_PC
	m_Mouse.bb_left		= m_Size.x * -C_Simple2D::m_JustificationOffsets[m_Justify].X();
	m_Mouse.bb_top		= m_Size.y * -C_Simple2D::m_JustificationOffsets[m_Justify].Y();

	m_Mouse.bb_left		+= m_Position.x;
	m_Mouse.bb_top		+= m_Position.y;

	m_Mouse.bb_right	= m_Mouse.bb_left + m_Size.x;
	m_Mouse.bb_bottom	= m_Mouse.bb_top + m_Size.y;
#endif
}

bool C_GadgetBase::AddSubscriber( Messaging::TMsgType messageType )
{
	bool ret = Messaging::C_MessageHandler::AddSubscriber( messageType );
	SetSubscriberMask( messageType, (1 << m_ID.Context() ) );

	return ret;
}

void C_GadgetBase::ResetDirtyFlags()
{
	// Clear
	m_Flags = (m_Flags & ~E_Dirty_Mask) | E_Dirty_Matrix;
}

bool C_GadgetBase::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
#if !defined(CFG_GOLD) && !defined(CFG_PROFILE)
	C_UILua::C_StackCheck c = g_pUILua->CreateStackCheck();
#endif
	
	switch( pMessage->Type() )
	{
	case E_UiMsg_StartFrame:
		ResetDirtyFlags();
		return true;

	case E_UiMsg_FrameUpdate:
		FrameUpdate();
		return true;

	case E_UiMsg_UpdateEffects:
		UpdateEffects();
		return true;

	case E_UiMsg_Render:
		Render();
		return true;

	case E_UiMsg_BindTimelines:
		BindTimeline();
		return true;

	case E_UiMsg_PostInit:
		PostInit();
		return true;

	case E_UiMsg_EnableControllerInput:
	{
		/* Note : 
		 * Set arg1 (pad) == true	: controller input active 
		 * Set arg1 (pad) == false	: controller input inactive  == false
		 * Set arg2 (deb) == true	: Use counter system,  so if two things can make something inactive it will take two active messages to turn controller input back on
		 * Set arg2 (deb) == false	: Ignore counter system (and reset it) set directly controller input active/inactive.
		 */

		const bool  previous = (m_IgnoreController == 0);
		const C_UiEnableControllerMessage* pUIMessage = Messaging::MessageCast<C_UiEnableControllerMessage>(pMessage);
		if( pUIMessage->m_ResetCount )
		{
			// Set to true if you want to clear the counter
			m_IgnoreController = (pUIMessage->m_Enable == 0);
		}
		else
		{
			// Change counter
			if( pUIMessage->m_Enable )
				m_IgnoreController--;
			else
				m_IgnoreController++;
		}

		if( previous != (m_IgnoreController == 0) )
		{
			const bool enable = (m_IgnoreController == 0);

			for( U32 i = E_UiMsg_CtrlButton_Up; i < E_UiMsg_CtrlEnd; i++ )
				EnableSubscriber( i, enable );
		}
	}
	break;

	case E_UiMsg_SetTimelineActive:
		{
			const C_UiTimelineMessage* p = Messaging::MessageCast<C_UiTimelineMessage>(pMessage);

			for ( S32 j = 0; j < m_Timeline.count; j++ )
			{
				// Timeline doesn't have a name
				if ( !m_Timeline[ j ].m_Name.string )
					continue;

				// Timeline name doesn't match message name
				if ( stricmp( p->m_Name, m_Timeline[ j ].m_Name ) )
					continue;

#ifndef CFG_GOLD
				if( g_ShowValidTimelineActiveMsg )
					LOGDEBUG( g_Log, "Timeline found : [%s] - Name: %s - Set: %s - Reset: %s - Time: %.2f", 
						m_Name.Text() ? m_Name.Text() : "<unknown>", 
						p->m_Name[ 0 ] ? p->m_Name : "<unknown>", 
						p->m_Set ? "true" : "false", 
						p->m_Reset ? "true" : "false", 
						p->m_fTime );
#endif

				// Valid
				m_Timeline[ j ].Active( p->m_Set );
				if ( p->m_Reset )
					m_Timeline[ j ].Reset( true );

				if( p->m_fTime != -1.0f )
					m_Timeline[ j ].SetTime( p->m_fTime );
			}
		}
		return true;

	case E_UiMsg_Exit:
		Exit();
		return true;
	}

#ifdef PLATFORM_PC
	if(m_Activated && m_Mouse.clickable && m_Mouse.isover)
	{
		const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*) pMessage;
		
		if ( pUIMessage )
		{
			// left mouse button
			if ( pUIMessage->Type() == E_UiMsg_CtrlMouse_DebOff && pUIMessage->pad == 1 )
			{
				if ( m_Mouse.lclick_msg == -1 )
				{
					_UI_MESSAGE3( E_UiMsg_CtrlMouse_Click, 1, 0, (F64)m_ID );
				}
#ifdef _FIX_WIN32MSG_
				else
				{
					_UI_MESSAGE3( m_Mouse.lclick_msg, 1, 0, (F64)m_ID );
				}
#endif
			}
			// right mouse button
			else if ( pUIMessage->Type() == E_UiMsg_CtrlMouse_DebOff && pUIMessage->pad == 2 )
			{
				if ( m_Mouse.rclick_msg == -1 )
				{
					_UI_MESSAGE3( E_UiMsg_CtrlMouse_Click, 2, 0, (F64)m_ID );
				}
#ifdef _FIX_WIN32MSG_
				else
				{
					_UI_MESSAGE3( m_Mouse.rclick_msg, 2, 0, (F64)m_ID );
				}
#endif
			}
			// middle mouse button
			else if ( pUIMessage->Type() == E_UiMsg_CtrlMouse_DebOff && pUIMessage->pad == 3 )
			{
				_UI_MESSAGE3( E_UiMsg_CtrlMouse_Click, 3, 0, (F64)m_ID );
			}			
		}
	}
#endif

	return false;
}

// Helper gadgets
void UISafeArea::Setup( lua_State* pState )
{
	F32 saa = 0.075f; // default is 7.5%		// MS TCR says 90%, Sony TRC says 85% - so we'll go with the smallest area
	if( pState )
	{
		lua_getglobal( pState, LUA_GLOBAL_TABLE_NAME );
		if ( lua_type( pState, -1 ) == LUA_TTABLE )
		{
			Lua::GetDataMember( pState, "safe_area_amount", saa, true );
		}
		
		lua_pop( pState, 1 ); // Pop off global name
	}

	F32 x_amount = (saa * gSimple2D.ScreenWidth());
	F32 y_amount = (saa * UI_VSCREEN_SIZEY);

	m_Size.x = gSimple2D.ScreenWidth() - ( x_amount * 2.0f );
	m_Size.y = UI_VSCREEN_SIZEY - ( y_amount * 2.0f );
	m_Position.x = gSimple2D.ScreenLeft() + x_amount;
	m_Position.y = y_amount;

	// Remove the default subscribers
	RemoveSubscriber( E_UiMsg_PostInit );
	RemoveSubscriber( E_UiMsg_StartFrame );
	RemoveSubscriber( E_UiMsg_FrameUpdate );
	RemoveSubscriber( E_UiMsg_UpdateEffects );
	RemoveSubscriber( E_UiMsg_Render );
	RemoveSubscriber( E_UiMsg_Exit );
	RemoveSubscriber( E_UiMsg_EnableControllerInput );

	DirtySet( E_Dirty_Matrix, true );

	C_GadgetBase::Setup( pState );
}

// Change render state gadget
void UIRenderState::Init()
{
	C_GadgetBase::Init();

	// Clear messages (apart from render)
//	RemoveSubscriber( E_UiMsg_UpdateEffects );
	RemoveSubscriber( E_UiMsg_StartFrame );
	RemoveSubscriber( E_UiMsg_Exit );
	RemoveSubscriber( E_UiMsg_EnableControllerInput );

	m_AlterFlags = 0;
}

void UIRenderState::Render()
{
	if( !m_Activated )
		return;

	// Pop current state
	if( !m_PushState )
	{
		gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 0 );
		return;
	}

	// Push on a new state
	if( !m_AlterFlags )
		return;

	gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetRendersStateStack, 1 );

	// Clip winddow (change if it was on or is on)
	if( m_AlterFlags & (1<<E_AlterFlags_ClipWindow) )
	{
		if( m_State.m_ClipEnabled == Display::E_Bool_True )
		{
			F32 x1 = m_State.m_ClipPos.vx; 
			F32 x2 = m_State.m_ClipPos.vx + m_State.m_ClipSize.vx;
			F32 y1 = m_State.m_ClipPos.vy;
			F32 y2 = m_State.m_ClipPos.vy + m_State.m_ClipSize.vy;
			gSimple2D.AddSprite( m_Layer, Maths::HVec3( x1, y1, 0.0f ), x2 - x1, y2 - y1, m_SpriteColours, m_Panel, E_SpriteType_SetClipWindow, C_RenderState::Instance(), 1 );
		}
		else
		{
			gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetClipWindow, 0 );
		}
	}

	// Depth buffer
	if( m_AlterFlags & (1<<E_AlterFlags_DepthBuffer) )
	{
		C_Simple2D::C_StackedRenderState::C_DepthState depth;

		depth.function	= m_State.m_DepthCompareFunc;
		depth.write		= m_State.m_DepthWriteEnabled;
		depth.enabled	= m_State.m_DepthEnabled;

		gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetDepthBuffer, *((S32*)&depth) );
	}
	// Stencil
	if( m_AlterFlags & (1<<E_AlterFlags_StencilBuffer) )
	{
		gSimple2D.AddSprite( m_Layer, m_Panel, m_State.m_StencilState );
	}
	// Colour write enable?
	if( m_AlterFlags & (1<<E_AlterFlags_ColourWrite) )
	{
		gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetColourWrite, (U32)m_State.m_ColourWriteEnable );
	}
	// Cull mode
	if( m_AlterFlags & (1<<E_AlterFlags_CullMode) )
	{
		gSimple2D.AddSprite( m_Layer, m_Panel, E_SpriteType_SetCullMode, (U32)m_State.m_CullMode );
	}
}

// Mini gadget which branches batch rendering of all children
void UIRenderBranch::PostInit()
{
	C_GadgetBase::PostInit();

	// The branch could have had its scan called in lua. So we dont want duplicates.
	if ( !m_BranchChildren.Size() )
	{
		Scan();
	}
}

void UIRenderBranch::DirtyMatrix()
{
	DirtySet( E_Dirty_Matrix, true );
	for( S32 i = 0; i < m_BranchChildren.Size(); i++ )
		m_BranchChildren[ i ]->DirtySet( E_Dirty_Matrix, true );
}

static bool MrOperator( const C_GadgetBase* lhs, const C_GadgetBase* rhs )
{ 
	return ( lhs->m_ID.Number() < rhs->m_ID.Number() ); 
}

void UIRenderBranch::Scan()
{
	m_BranchChildren.Clear();
	FindChildren( &m_BranchChildren, true, true );

	// remove children who are already children of other render branches
	for ( ButtonArray::Iterator it = m_BranchChildren.Begin(); it != m_BranchChildren.End(); )
	{
		C_GadgetBase* p = (*it)->Parent();

		while ( p && ( ( p->GetType() != E_UIButtonType_RenderBranch ) && ( p->GetType() != E_UIButtonType_Node ) && ( p->GetType() != E_UIButtonType_NodeList ) ) )
			p = p->Parent();

		if ( !p || ( p == this ) )
			++it;
		else
			it = m_BranchChildren.Erase( it );
	}

	for( S32 i = 0; i < m_BranchChildren.Size(); i++ )
	{
		m_BranchChildren[ i ]->RemoveSubscriber( E_UiMsg_Render );
		m_BranchChildren[ i ]->RemoveSubscriber( E_UiMsg_UpdateEffects );
		m_BranchChildren[ i ]->RemoveSubscriber( E_UiMsg_StartFrame );
	}

	if( m_DoSort )
		std::sort( m_BranchChildren.Begin(), m_BranchChildren.End(), MrOperator );
}


bool UIRenderBranch::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch( pMessage->Type() )
	{
	case E_UiMsg_StartFrame:
	case E_UiMsg_Render:
	case E_UiMsg_UpdateEffects:
		for( S32 i = 0; i < m_BranchChildren.Size(); i++ )
		{
			m_BranchChildren[ i ]->MessageHandler( pMessage );
		}
		break;
	}

	return C_GadgetBase::MessageHandler( pMessage );
}

// UIDummyLerp
void UIDummyLerp::Init()
{
	AddSubscriber( E_UiMsg_FrameUpdate );
	RemoveSubscriber( E_UiMsg_StartFrame );		// Called manually
	RemoveSubscriber( E_UiMsg_UpdateEffects );	// Called manually in frame updaet
	RemoveSubscriber( E_UiMsg_Render );			// Not required

	C_GadgetBase::Init();
}

void UIDummyLerp::CopyMatrix( const Maths::HMat43& src )
{
	m_StoreMat = src;
}

void UIDummyLerp::FrameUpdate()
{
	if( !m_Activated )
		return;

	C_GadgetBase::FrameUpdate();

	// Calc my transform each frame
	ResetDirtyFlags();
	UpdateEffects();

	Maths::HMat43 mat = Matrix();

	CalcTransformHierarchy( mat );

	Maths::C_MatrixSRT src_srt( m_StoreMat.GetSRT() );
	Maths::C_MatrixSRT dst_srt( mat.GetSRT() );

	Maths::C_Quaternion src_quat( m_StoreMat.Rotation().NormalisedZY() );
	Maths::C_Quaternion dst_quat( mat.Rotation().NormalisedZY() );

	Maths::HVec3 src_pos( m_StoreMat.AW() );
	Maths::HVec3 dst_pos( mat.AW() );

	Maths::HVec3 src_scale( src_srt.Scale() );
	Maths::HVec3 dst_scale( dst_srt.Scale() );

	Maths::HMat33 scale_mat( Maths::C_MatrixScale( src_scale.Lerp( dst_scale, m_fLerp ) ) );

	mat.Rotation( src_quat.ShortestLerp( dst_quat, m_fLerp ) );
	mat.Rotation( scale_mat * mat.Rotation() );
	mat.Translation( src_pos.Lerp( dst_pos, m_fLerp ) );

	SetMatrix(mat);
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
