#include "stdafx.h"

#include "ui/Game/Gadgets/UIKeyboard.h"

#include "ui/Game/UILua.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Graphics/SimpleTimer.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

XT_TYPE( UI::UIKeyboard, "UIKeyboard" )
	XT_BASE( UI::C_GadgetBase )
XT_END_TYPE

/// *** TODO - make these definable by the game?
static const S32 s_sNumRows = 4;
static const S32 s_sNumCols = 11;
static const S32 s_Keyboard[ s_sNumRows ][ s_sNumCols ] =
{
	{ 'a', 'b', 'c', 'd', 'e', 'f', 'g',   0, '7', '8', '9', },
	{ 'h', 'i', 'j', 'k', 'l', 'm', 'n',   0, '4', '5', '6', },
	{ 'o', 'p', 'q', 'r', 's', 't', 'u',   0, '1', '2', '3', },
	{ 'v', 'w', 'x', 'y', 'z', '_', '-',   0, '0',   0, '.', },
};
static const S32 s_KeyboardCaps[ s_sNumRows ][ s_sNumCols ] =
{
	{ 'A', 'B', 'C', 'D', 'E', 'F', 'G',  -1,  -1,  -1,  -1, },
	{ 'H', 'I', 'J', 'K', 'L', 'M', 'N',  -1,  -1,  -1,  -1, },
	{ 'O', 'P', 'Q', 'R', 'S', 'T', 'U',  -1,  -1,  -1,  -1, },
	{ 'V', 'W', 'X', 'Y', 'Z',  -1,  -1,  -1,  -1,  -1,  -1, },
};
//

// *** TODO - put these in Sorcery
static const F32 fGap = 2.0f;
static UI::C_UIColour colBackground = 0xff808080;
static UI::C_UIColour colText = 0xff000000;
static UI::C_UIColour colSelected = 0xffffffff;
static UI::C_UIColour colCursor = 0x80808080;
static UI::C_UIColour colHighlight = 0x40808080;
static const F32 fCursorWidth = 2.0f;
static const F32 fTextGapMult = 0.5f;
//

namespace UI
{

// ------------
// C_UIKeyboard
// ------------
C_UIKeyboard C_UIKeyboard::m_sInstance;

void C_UIKeyboard::Initialise( const S32 sContext, const char* pScriptName )
{
	m_sContext = sContext;
	STRCPY( m_pScriptName, m_uMaxScriptNameLength, pScriptName );
	m_bInitialised = true;
}

void C_UIKeyboard::Start( const S32 sMaxCharacters, const WCHAR* pString, const bool bHighlight )
{
	if ( !m_bInitialised )
	{
		_UI_MESSAGE( E_UiMsg_KeyboardCancelled );
		return;
	}

	if ( m_bDisplayed )
		return;

	// store current context
	m_sRestoreContext = C_ScreenControlManager::Instance().Context();

	// activate
	C_ScreenControlManager::Instance().ResetContext( true, m_sContext );

	C_ScreenControlManager::Instance().Get( m_sContext ).SetNextScreen( m_pScriptName );
	C_ScreenControlManager::Instance().Get( m_sContext ).Init();

	m_bDisplayed = true;

	if ( ( sMaxCharacters > 0 ) && ( sMaxCharacters < (S32)m_String.Size() ) )
		m_sMaxCharacters = sMaxCharacters;
	else
		m_sMaxCharacters = m_uMaxCharacterDefault;

	m_String.Clear();

	if ( pString )
		m_String = pString;

	m_bHighlight = bHighlight;
}

void C_UIKeyboard::Cancel( const C_WideString256& string )
{
	if ( !m_bInitialised || !m_bDisplayed )
		return;

	C_ScreenControlManager::Instance().Get( m_sContext ).End();
	C_ScreenControlManager::Instance().ResetContext( true, m_sRestoreContext );

	m_bDisplayed = false;

	m_String = string;

	if ( m_String.Length() > 0 )
	{
		_UI_MESSAGE( E_UiMsg_KeyboardFinished );
	}
	else
	{
		_UI_MESSAGE( E_UiMsg_KeyboardCancelled );
	}
}

// ----------
// UIKeyboard
// ----------
void UIKeyboard::Init()
{
	C_GadgetBase::Init();

	m_sRow = 0;
	m_sCol = 0;
	m_sPosition = 0;
	m_bCaps = false;

	// messages
	AddSubscriber( E_UiMsg_MenuNext );
	AddSubscriber( E_UiMsg_CtrlButton_X );
	AddSubscriber( E_UiMsg_CtrlButton_Y );
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlButton_Left );
	AddSubscriber( E_UiMsg_CtrlButton_Right );
	AddSubscriber( E_UiMsg_CtrlButton_LeftShoulder );
	AddSubscriber( E_UiMsg_CtrlButton_RightShoulder );
	AddSubscriber( E_UiMsg_CtrlButton_LeftThumb );
	AddSubscriber( E_UiMsg_MenuBack );
	AddSubscriber( E_UiMsg_CtrlButton_Start );

	w[ 0 ] = 0;
	w[ 1 ] = L'\0';

	m_bHighlight = C_UIKeyboard::Instance().GetHighlight();
	m_String = C_UIKeyboard::Instance().GetString();

	if ( m_bHighlight )
		m_sPosition = (S32)m_String.Length();
}

void UIKeyboard::Setup( lua_State* pState )
{
	C_GadgetBase::Setup( pState );

	m_fCharacterSize = m_Size.y;
}

bool UIKeyboard::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	switch ( pMessage->Type() )
	{
		case E_UiMsg_CtrlButton_Up:
		{
			for ( S32 i = ( m_sRow - 1 ); i >= 0; i-- )
			{
				if ( GetCharacter( i, m_sCol ) != 0 )
				{
					m_sRow = i;
					break;
				}
			}
			break;
		}

		case E_UiMsg_CtrlButton_Down:
		{
			for ( S32 i = ( m_sRow + 1 ); i < s_sNumRows; i++ )
			{
				if ( GetCharacter( i, m_sCol ) != 0 )
				{
					m_sRow = i;
					break;
				}
			}
			break;
		}

		case E_UiMsg_CtrlButton_Left:
		{
			for ( S32 i = ( m_sCol - 1 ); i >= 0; i-- )
			{
				if ( GetCharacter( m_sRow, i ) != 0 )
				{
					m_sCol = i;
					break;
				}
			}
			break;
		}

		case E_UiMsg_CtrlButton_Right:
		{
			for ( S32 i = ( m_sCol + 1 ); i < s_sNumCols; i++ )
			{
				if ( GetCharacter( m_sRow, i ) != 0 )
				{
					m_sCol = i;
					break;
				}
			}
			break;
		}

		case E_UiMsg_MenuNext:	// add
		{
			if ( m_bHighlight )
			{
				m_bHighlight = false;
				m_String.Clear();
				m_sPosition = 0;
			}

			if ( m_String.Length() == C_UIKeyboard::Instance().GetMaxCharacters() )
				break;

			w[ 0 ] = GetCharacter( m_sRow, m_sCol );
			m_String.Insert( w, m_sPosition++ );

			break;
		}

		case E_UiMsg_CtrlButton_X:	// delete
		{
			if ( m_bHighlight )
			{
				m_bHighlight = false;
				m_String.Clear();
				m_sPosition = 0;
			}

			if ( m_sPosition == 0 )
				break;

			C_WideString256 t = m_String.SubString( m_sPosition, m_String.Length() );
			m_String.Replace( m_sPosition - 1, L'\0' );
			m_String.Add( t );

			m_sPosition--;

			break;
		}

		case E_UiMsg_CtrlButton_Y:	// space
		{
			if ( m_bHighlight )
			{
				m_bHighlight = false;
				m_String.Clear();
				m_sPosition = 0;
			}

			if ( m_String.Length() == C_UIKeyboard::Instance().GetMaxCharacters() )
				break;

			w[ 0 ] = L' ';
			m_String.Insert( w, m_sPosition++ );

			break;
		}

		case E_UiMsg_CtrlButton_LeftShoulder:	// cursor back
		{
			if ( m_sPosition > 0 )
				m_sPosition--;

			if ( m_bHighlight )
			{
				m_bHighlight = false;
				m_sPosition = 0;
			}

			break;
		}

		case E_UiMsg_CtrlButton_RightShoulder:	// cursor forward
		{
			if ( m_sPosition < (S32)m_String.Length() )
				m_sPosition++;

			if ( m_bHighlight )
			{
				m_bHighlight = false;
				m_sPosition = (S32)m_String.Length();
			}

			break;
		}

		case E_UiMsg_CtrlButton_LeftThumb:	// caps
		{
			m_bCaps = !m_bCaps;
			break;
		}

		case E_UiMsg_MenuBack:
			C_UIKeyboard::Instance().Cancel();
			break;

		case E_UiMsg_CtrlButton_Start:
			C_UIKeyboard::Instance().Cancel( m_String );
			break;

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UIKeyboard::Render()
{
	if ( !m_Activated )
		return;

	// auto-transform
	C_UIAutoTransform transform( this );

	m_Size.x  = ( m_fCharacterSize * s_sNumCols ) + ( fGap * s_sNumCols );
	m_Size.y  = ( m_fCharacterSize * s_sNumRows ) + ( fGap * s_sNumRows );
	m_Size.y += ( m_fCharacterSize * ( fTextGapMult + 1.0f ) );

	F32 x = m_Position.x;
	F32 y = m_Position.y;

	// justify
	Maths::HVec3 offset = gSimple2D.GetJustificationOffset( m_Justify, m_Panel );

	x -= offset.X() * m_Size.x;
	y -= offset.Y() * m_Size.y;

	//*** TODO - get draw text to return the length? - it's seems silly to calculate this twice anyway
	gFont.SetDrawCountNext( m_sPosition );
	F32 fLength = gFont.StringLength( m_String.Chars(), m_fCharacterSize, C_FontStyle::Instance() );

	if ( m_bHighlight )
	{
		gSimple2D.AddSprite( m_Layer, Maths::HVec3( x, y, m_Position.z ), fLength, m_fCharacterSize, 
			colHighlight, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddJustification( E_Justify_TopLeft );
		gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	}

	// text
	gFont.DrawText( m_Layer, Maths::HVec3( x, y, m_Position.z ), E_Justify_TopLeft, m_String.Chars(), 
		colSelected, m_fCharacterSize, C_FontStyle::Instance(), 0.0f, m_Panel );

	// position cursor
	gSimple2D.AddSprite( m_Layer, Maths::HVec3( x + fLength, y, m_Position.z ), fCursorWidth, m_fCharacterSize, 
		colCursor, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddJustification( E_Justify_TopCentre );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );

	y += ( m_fCharacterSize * ( fTextGapMult + 1.0f ) );

	// keyboard
	for ( S32 row = 0; row < s_sNumRows; row++ )
	{
		for ( S32 col = 0; col < s_sNumCols; col++ )
		{
			if ( !GetCharacter( row, col ) )
				continue;

			Maths::HVec3 pos( x + ( m_fCharacterSize * col ) + ( fGap * col ) + m_fCharacterSize * 0.5f, 
				y + ( m_fCharacterSize * row ) + ( fGap * row ) + m_fCharacterSize * 0.5f, 
				m_Position.z );

			gSimple2D.AddSprite( m_Layer, pos, m_fCharacterSize, m_fCharacterSize, 
				( ( m_sRow == row ) && ( m_sCol == col ) ) ? colSelected : colBackground, m_Panel, E_SpriteType_Sprite, m_RenderState );
			gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
			gSimple2D.AddJustification( E_Justify_MiddleCentre );

			w[ 0 ] = GetCharacter( row, col );

			gFont.FixedSpaceNext( E_FixedSpaceType_All );
			gFont.DrawText( m_Layer, pos, E_Justify_MiddleCentre, w, colText, m_fCharacterSize, C_FontStyle::Instance(), 0.0f, m_Panel );
		}
	}

	C_GadgetBase::Render();
}

WCHAR UIKeyboard::GetCharacter( const S32 sRow, const S32 sCol )
{
	if ( m_bCaps && ( s_KeyboardCaps[ sRow ][ sCol ] != -1 ) )
		return s_KeyboardCaps[ sRow ][ sCol ];

	return s_Keyboard[ sRow ][ sCol ];
}

};	// namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
