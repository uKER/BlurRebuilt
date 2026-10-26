#include "stdafx.h"
#include "ui/Game/UILocaliser.h"
#include "ui/Game/Gadgets/UIStaticText.h"

#include "ui/Game/UILua.h"
#include "ui/Game/UIDataFinder.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Game/Timeline.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Game/UiXt.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UIStaticText, "UIStaticText" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD( m_CalcStringSize,		"calc_string_size" )
	XT_FIELD( m_MaintainWidth,		"maintain_width" )
	XT_FIELD( m_Truncate,			"truncate" )
	XT_FIELD( m_FontStylePasses,	"styles" )
	XT_FIELD( m_VariableName,		"text_string" )
	XT_FIELD( m_TextID,				"text_id" )
	XT_FIELD( m_eFixedSpaceType,	"fixed_space" )
	XT_FIELD( m_PrintDelayTime,		"print_delay_time" )
	XT_FIELD( m_EnableCache,		"enable_cache" )
XT_END_TYPE

Debug::Bool g_UiDebugUseCache("UI.DebugFontCache", true );

namespace UI
{

void UIStaticText::Init( )
{
	C_GadgetBase::Init();

	m_Text.Invalidate();
	m_TextID.id				= 0;

	m_pEffect				= NULL;

	m_CalcStringSize		= false;
	m_PrintText				= false;
	m_PrintDelayTime		= 0.0f;
	m_PrintTimer			= 0.0f;
	m_PrintAmount			= 0;
	m_TruncateAmount		= 0;
	m_Truncate				= false;

	m_eFixedSpaceType		= E_FixedSpaceType_None;
}

UIStaticText::~UIStaticText( )
{
	if( m_EnableCache )
		SAFE_DELETE_PTR( m_pCache );

	SAFE_DELETE_PTR( m_pEffect );
}

void UIStaticText::Setup( lua_State* pState )
{
#ifdef PLATFORM_PC
	m_fMouseOverEffectTime = 0.0f;
	m_MouseOverToColour.Set( 0xffffffff );

	if( pState )
	{
		m_MouseOverToColour.Set( Lua::GetDataMember<const char *>( pState, "mouse_over_to_colour" ) );
		Lua::GetDataMember( pState, "mouse_over_effect_time", m_fMouseOverEffectTime, true );
	}
	SetupMouseBoundingBox();
#endif

	if( m_TextID.id != 0 )
	{
		// Set directly with Xt annotation
		m_Text = m_TextID.id;
	}
	else 
	if( pState )
	{
		if ( Lua::DataMemberType( pState, "text" ) == LUA_TNUMBER )
		{
			// Old style IDS_
			m_Text = Lua::GetDataMember< U32 >( pState, "text" );
		}
		else 
		if( Lua::DataMemberType( pState, "text" ) == LUA_TSTRING )
		{
			// Old style datafinder "string"
			ChangeVariableName( Lua::GetDataMember<const char *>( pState, "text" ) );
		}
	}

	if( m_PrintDelayTime > 0 )
	{
		m_PrintText = true;
		m_PrintTimer = m_PrintDelayTime;
		AddSubscriber( E_UiMsg_FrameUpdate );
	}
	else
	if( m_pEffect )
		AddSubscriber( E_UiMsg_FrameUpdate );

	m_FontStylePasses.Setup( pState );

	m_DrawWidth = ( m_Truncate ) ? 0.0f : m_Size.x;
	m_DrawHeight = m_Size.y;

	m_fOriginalScaleX = m_Scale.x;

	// Modify height so that the text sits within a certain width
	DoMaintainWidth();

	DoCalcStringSize();

	DoTruncateString();

	if( m_EnableCache )
		m_pCache = new C_Cache();

	C_GadgetBase::Setup( pState );
}


void UIStaticText::ChangeSize( const F32 fX, const F32 fY )
{
	m_Size.x = fX;
	m_Size.y = fY;

	m_DrawWidth = ( m_Truncate ) ? 0.0f : m_Size.x;
	m_DrawHeight = m_Size.y;

	// Modify height so that the text sits within a certain width
	DoMaintainWidth();

	DoCalcStringSize();

	DoTruncateString();
}


void UIStaticText::SetupMouseBoundingBox()
{
#ifdef PLATFORM_PC
	// This may have to be done every frame.
	// PR - I've made it recalculate on ChangeText()
	if(m_Mouse.clickable)
	{
		F32 len = 0.0f;
		if(m_Text.IsLocString()) 
		{
			gFont.FixedSpaceNext( m_eFixedSpaceType );
			len = gFont.StringLength( m_Text, m_DrawHeight, m_FontStylePasses.GetFontStyle( 0 ) );
		}
		else 
		if(!m_Text.IsValid())
		{
			gFont.FixedSpaceNext( m_eFixedSpaceType );
			len = gFont.StringLength( m_Text, m_DrawHeight, m_FontStylePasses.GetFontStyle( 0 ) );
		}
		m_Mouse.bb_left		= len * -C_Simple2D::m_JustificationOffsets[m_Justify].X() + m_Position.x;
		m_Mouse.bb_right	= m_Mouse.bb_left + len;
	}
	if( m_Mouse.clickable )
		AddSubscriber( E_UiMsg_FrameUpdate );
#endif
	
}

void UIStaticText::FrameUpdate( )
{
	if ( !m_Activated )
		return;

	if(m_pEffect)
		m_pEffect->Update();

	C_GadgetBase::FrameUpdate();

#ifdef PLATFORM_PC
	if ( !m_Mouse.clickable || m_fMouseOverEffectTime <= 0.0f )
		return;

	F32 fFrameTime = theFETimer.FrameTime();

	if ( m_Activated && m_Mouse.isover && m_fMouseOverTimer != 1.0f )
		m_fMouseOverTimer = Maths::Clamp( m_fMouseOverTimer + ( 1.0f / m_fMouseOverEffectTime ) * fFrameTime, 0.0f, 1.0f );
	else if ( m_Activated && !m_Mouse.isover && m_fMouseOverTimer != 0.0f )
		m_fMouseOverTimer = Maths::Clamp( m_fMouseOverTimer - ( 1.0f / m_fMouseOverEffectTime ) * fFrameTime, 0.0f, 1.0f );
#endif
	// Update the number of characters to display.
	const WCHAR* pString = GetString();

	if( m_PrintText && pString )
	{
		S32 len = (S32)wcslen( pString );
		if( m_PrintAmount < len )
		{
			m_PrintTimer -= ( theFETimer.FrameTime() );

			while( m_PrintTimer <= 0 )
			{			
				m_PrintAmount++;
				if ( m_PrintAmount == len ) // Finished
				{
					m_PrintTimer = 0;

					// Broadcast that we've finished
					_UI_MESSAGE3( E_UiMsg_TextPrintFinished, (F64)m_ID.Number(), (F64)m_ID.Id(), (F64)m_ID.Context() );

				}
				else
					m_PrintTimer += m_PrintDelayTime;

				if( m_pCache )
					m_pCache->Invalidate(); // We could just set 'end' here?
			}
		}
	}
}

void UIStaticText::ResetTextPrint()
{
	m_PrintTimer = m_PrintDelayTime; 
	m_PrintAmount = 0;

	if( m_pCache )
		m_pCache->Invalidate(); // We could just set 'end' here?
}

void UIStaticText::SkipTextPrint()
{
	const WCHAR* pString = GetString();
	if( !pString )
		return;

	S32 len = (S32)wcslen( pString );
	if( m_PrintAmount >= len )
		return;

	m_PrintTimer = 0;
	m_PrintAmount = len;

	_UI_MESSAGE3( E_UiMsg_TextPrintFinished, (F64)m_ID.Number(), (F64)m_ID.Id(), (F64)m_ID.Context() );

	if( m_pCache )
		m_pCache->Invalidate(); // We could just set 'end' here?
}

void UIStaticText::Render()
{
	if ( !m_Activated )
		return;

	// Changed externally?
	if( m_TextID.id != 0 && m_TextID.id != m_Text.Value() )
	{
		m_Text = m_TextID.id;
	}

	if ( m_Text.IsValid() && !m_Text.IsLocString() )
		return;

	if ( !m_Text.IsValid() && ( !m_VariableName || !m_VariableName[ 0 ] ) && !m_pDirectText )
		return;

#ifdef _UI_AUTO_CLIPWINDOW
	// clip windows
	C_UIClipWindow clipW1 = gSimple2D.AddClipWindowAuto( this );
#endif

	if ( ( !m_Text.IsValid() || !m_Text.IsLocString() ) && m_VariableName && m_VariableName[ 0 ] )
		DoMaintainWidth();	// horrible, but DataFinder strings don't get updated otherwise - TODO better

	// auto-transform
	C_UIAutoTransform transform( this );

#ifdef _GADGET_BOUNDS_CULLING
	if( IsCulled() )
		return;
#endif

	C_GadgetBase::Render();

	bool doCache = ( m_pCache && m_pEffect == NULL ); // TODO call m_pEffect in cache func

#if !defined ( CFG_GOLD )
	if( g_UiDebugUseCache == false )
		doCache = false;
#endif

	// Check to see if any effects have occured
	if( doCache )
	{
		if( DirtySet( E_Dirty_Alpha ) )
			m_pCache->SetAlpha( m_SpriteColours );

		if( DirtySet( E_Dirty_Colour ) )
			m_pCache->SetColour( m_SpriteColours );
	}

	if( doCache && m_pCache->IsValid() )
	{
		for ( U32 i = 0; i < m_FontStylePasses.NumFontStyles(); i++ )
			m_FontStylePasses.GetFontStyle( i ).Render( m_pCache->Stack(), m_DrawHeight, m_Panel );

		return;
	}

	if ( m_pEffect )
		gFont.SetPreRenderNext( m_pEffect );

	C_UISpriteColours colour = m_SpriteColours;

#ifdef PLATFORM_PC
	// mouse over colour effect
	if ( m_fMouseOverEffectTime != 0.0f )
	{
		for ( U32 i = 0; i < C_UISpriteColours::m_SpriteColoursSize; i++ )
			colour[ i ].Lerp( colour[ i ], m_MouseOverToColour, m_fMouseOverTimer );
	}
#endif

	if ( m_PrintText )
		gFont.SetDrawCountNext( m_PrintAmount );

	if ( m_Truncate )
		gFont.SetDrawCountNext( m_TruncateAmount );

	// render
	const WCHAR* pString = GetString();
	if( !pString )
		return;

	if( doCache )
		gFont.SetPreRenderNext( m_pCache );

	gFont.FixedSpaceNext( m_eFixedSpaceType );
	gFont.DrawText( m_Layer, m_Position, m_Justify, pString, colour, m_DrawHeight, m_FontStylePasses, m_DrawWidth, m_Panel );
}

const bool C_FontInterfaceParam::Setup( lua_State* pState )
{
#ifdef _UI_REFACTOR_
	if( !pState || Lua::DataMemberType( pState, "shader_interface") != LUA_TTABLE )
		return false;

	lua_pushstring(pState, "shader_interface");
	lua_gettable(pState, -2);
	{
		const F32 shift = 16.0f; // Pack 0-256 into 4bits (256 >> 4)

		U32 i, a, t;
		// Outline
		Lua::GetDataMember( pState, "outline_i", i );
		Lua::GetDataMember( pState, "outline_a", a );
		Lua::GetDataMember( pState, "outline_t", t );

		m_OutlineI		= (U8)(i/shift);
		m_OutlineA		= (U8)(a/shift);
		m_OutlineTex	= (U8)t;
		
		// Piping
		Lua::GetDataMember( pState, "piping_i", i );
		Lua::GetDataMember( pState, "piping_a", a );
		Lua::GetDataMember( pState, "piping_t", t );

		m_PipingI	= (U8)(i/shift);
		m_PipingA	= (U8)(a/shift);
		m_PipingTex	= (U8)t;

		// Centre
		Lua::GetDataMember( pState, "centre_i", i );
		Lua::GetDataMember( pState, "centre_a", a );
		Lua::GetDataMember( pState, "centre_t", t );
		m_CentreI	= (U8)(i/shift);
		m_CentreA	= (U8)(a/shift);
		m_CentreTex	= (U8)t;

		// Flags
		Lua::GetDataMember( pState, "flags", t );
		m_Flags		= (U8)t;
	}
	lua_pop(pState,1);

#endif
	return true;
}

void UIStaticText::ChangeText( const C_TextStringID& number )
{
	m_Text = number;
	m_TextID.id = m_Text.Value();

	if( m_pCache )
		m_pCache->Invalidate();

	SetupMouseBoundingBox( );
	DoMaintainWidth( );
	DoCalcStringSize( );
}

void UIStaticText::ChangeText( const char* string )
{
	m_Text.Invalidate();
	m_TextID.id = 0;
	
	ChangeVariableName( string );

	if( m_pCache )
		m_pCache->Invalidate();

	SetupMouseBoundingBox( );
	DoMaintainWidth( );
	DoCalcStringSize( );
}

void UIStaticText::DoMaintainWidth()
{
	if( !m_MaintainWidth || m_Size.x <= 0.0f )
		return;

	const WCHAR* p = GetString();
	if( !p )
		return;

	gFont.FixedSpaceNext( m_eFixedSpaceType );
	F32 len  = gFont.StringLength( p, m_DrawHeight, m_FontStylePasses.GetFontStyle( 0 ) );
	if( len <= 0 )
		return;

	F32 frac = Maths::Clamp01( m_Size.x / len );

	if( C_UILocaliser::Instance().Language() == E_UILanguage_Japanese )
		m_DrawHeight *= frac; 
	else
		m_Scale.x = m_fOriginalScaleX * frac;

	m_DrawWidth = 0.0f;
}

void UIStaticText::DoCalcStringSize( const WCHAR* pStringOverride, const bool bForceCalculate )
{
	if( !m_CalcStringSize && !bForceCalculate )
		return;

	const WCHAR* p = pStringOverride;
	
	if ( !p )
		p = GetString();

	if( !p )
		return;

	if( m_DrawWidth )
	{
		gFont.FixedSpaceNext( m_eFixedSpaceType );
		m_Size.y = (F32)gFont.StringNumLines( p, m_DrawHeight, m_DrawWidth, m_FontStylePasses.GetFontStyle( 0 ) );
		gFont.FixedSpaceNext( m_eFixedSpaceType );
		m_Size.x = gFont.StringLength( p, m_DrawHeight, m_FontStylePasses.GetFontStyle( 0 ) );
	}
	else
	{
		gFont.FixedSpaceNext( m_eFixedSpaceType );
		m_Size.x = gFont.StringLength( p, m_DrawHeight, m_FontStylePasses.GetFontStyle( 0 ) );
		m_Size.y = 1.0f;
	}

	m_Size.y *= (m_DrawHeight * gFont.GetHeightPercentage( m_FontStylePasses.GetFontStyle( 0 ) ));
}


void UIStaticText::DoTruncateString()
{
	if ( m_Truncate )
	{
		const WCHAR* pString = GetString();

		if( pString )
		{
			S32 len = (S32)wcslen( pString );

			F32 actualLength = gFont.StringLength( pString, m_DrawHeight, m_FontStylePasses.GetFontStyle( 0 ) );
			F32 ratio = m_Size.x / actualLength;

			m_TruncateAmount = (S32)(len * ratio);

			if ( m_TruncateAmount < 0 )
				m_TruncateAmount = 0;
		}
	}
}


void UIStaticText::ChangeVariableName( const char* new_name )
{
	if( m_VariableName )
	{
		m_XtAllocator.Delete( const_cast<char *>(m_VariableName.string) );
	}

	m_VariableName = XtSz8( CopyCreateString( new_name ) );
}

const WCHAR* UIStaticText::GetString()
{
	if ( m_pDirectText )
		return m_pDirectText;

	const WCHAR* pString = NULL;

	if ( m_Text.IsValid() && m_Text.IsLocString() )
		pString = m_Text.String();
	else if ( m_VariableName && m_VariableName[ 0 ] )
	{
		pString = C_UIDataFinder::Instance().GetVariableText( m_VariableName );

		// Eeek This isnt the best way but i ran out of options.
		if ( m_Truncate && m_TruncateAmount )
		{
			static 	C_WideString512	s_DirectText;

			s_DirectText.Format( L"%s", pString );
			s_DirectText.Replace( m_TruncateAmount, L'\0' );
			s_DirectText += L"...\0";

			return s_DirectText.Chars();
		}
	}
	return pString;
}


const F32 UIStaticText::GetStringLength( const bool bUpdateLengths )
{
	if ( bUpdateLengths )
	{
		DoCalcStringSize( NULL, true );
		return m_Size.x;
	}

	const WCHAR* pString = GetString();
	
	gFont.FixedSpaceNext( m_eFixedSpaceType );
	return gFont.StringLength( pString, m_DrawHeight, m_FontStylePasses.GetFontStyle( 0 ) );
}

const F32 UIStaticText::GetStringHeight()
{
	C_FontStyle& fs = m_FontStylePasses.GetFontStyle( 0 );
	const WCHAR* p = GetString();
	if( !p )
		return 0.0f;

	gFont.FixedSpaceNext( m_eFixedSpaceType );
	U32 num_lines = gFont.StringNumLines( p, m_DrawHeight, m_DrawWidth, fs );

	return m_DrawHeight * (F32)num_lines * gFont.GetHeightPercentage( fs );
}

// Stack cache - make a copy so we can avoid calling DrawText each frame
void UIStaticText::C_Cache::ProcessStack( C_FontSpriteStack& stack )
{
	m_Stack = stack;
	m_Valid = true;
}

void UIStaticText::C_Cache::SetAlpha( const C_UISpriteColours& col )
{
	U32 size = m_Stack.Size();

	for( U32 i = 0; i < size; i++ )
	{
		C_FontSpriteItem* __restrict p = &m_Stack[ i ];

		p->m_Col[0].A( col[0].A() );
		p->m_Col[1].A( col[1].A() );
		p->m_Col[2].A( col[2].A() );
		p->m_Col[3].A( col[3].A() );
	}
}

void UIStaticText::C_Cache::SetColour( const C_UISpriteColours& col )
{
	U32 size = m_Stack.Size();

	for( U32 i = 0; i < size; i++ )
	{
		C_FontSpriteItem* __restrict p = &m_Stack[ i ];

		if( p->m_eItemType == C_FontSpriteItem::E_Type_Seq )
			continue;

		p->m_Col = col;
	}
}


}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
