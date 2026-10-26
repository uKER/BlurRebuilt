#include "stdafx.h"

#include <core/utility/endian.h>

#include "ui/UiInterface.h"
#include "ui/Game/UILua.h"
#include "ui/Game/Gadgets/ButtonMessages.h"
#include "ui/Graphics/Simple2d.h"
#include "ui/Graphics/simpleTimer.h"

#include "ui/Game/Gadgets/UILegal.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

// XT Definition
XT_TYPE( UI::UILegal, "UILegal" )
	XT_BASE( UI::C_GadgetBase )
	XT_FIELD(m_ColourStyleName, "colour_style_name" )
	XT_FIELD(m_Style, "font_style" )
	XT_FIELD(m_Height, "heights" )
	XT_FIELD(m_PreCr, "pre_cr" )
	XT_FIELD(m_PostCr, "post_cr" )
	XT_FIELD(m_StartPos, "start_pos" )
	XT_FIELD(m_ScrollSpeed, "scroll_speed")
	XT_FIELD(m_FadeLength, "fade_length")
	XT_FIELD(m_FadeBottom, "fade_bottom")
XT_END_TYPE

namespace UI
{

// Legal text singleton
UILegal::C_Text	UILegal::C_Text::m_sInstance;

// Legal display gadget
void UILegal::C_Text::Load( const char *filename )
{
	MEMORY_TRACKER_PUSH("ui.legal");

	assert(!m_Line);

	Sys::I_FileSystem *fs = Sys::GetDefaultFileSystem();
	Sys::I_File* fp = fs->Open(filename);	

	// Ooooh lets check for a null file pointer first ;)
	if (!fp)
	{
		LOGWARN( g_Log,  "UILegal::C_Text::Load failed %s", filename ? filename : "(filename was null)" );
		MEMORY_TRACKER_POP("ui.legal");
		return;
	}

	// Ooooh lets make a big hole in memory
	U32 length	= (U32)fp->Length();
	U32 count	= length / sizeof(U16);

	U16* text_block = new U16[ count ];
	assert(text_block);

	fp->Read(text_block, length);

	delete fp;

	// Endian swap 
	U32 lines = 0;
	// [Blur2PC] MSVC 14.51 hits an internal compiler error on the no-op swap call that follows; the guard is a compile-time constant
	if (Ut::NativeToLittleEndian::swap) for(U32 i = 0; i < count; i++)
		Ut::NativeToLittleEndian().EndianSwap(text_block[i]);


	// ---- End of text block process

	// Count the lines
	for(U32 i = 0; i < count; i++)
		if(text_block[i] == 0x000d)
		{
			lines++;
		}
	assert(lines);

	m_Line = new C_Line[lines+1];
	assert(m_Line);
	memset(m_Line, 0, sizeof(C_Line) * (lines+1));

	lines = 0;
	for(U32 i = 1; i < count; i++)// 1 to Clear the unicode header
	{
		C_Line::E_Type type = C_Line::E_Type_Text;
		bool ids = true;
		switch(text_block[i])
		{
		case L'G':
			type = C_Line::E_Type_Graphics;
			break;
		case L'M':
			type = C_Line::E_Type_MainHeading;
			break;
		case L'm':
			type = C_Line::E_Type_Heading1;
			break;
		case L'H':
			type = C_Line::E_Type_Heading2;
			break;
		case L'T':
			type = C_Line::E_Type_Text;
			ids = false;
			break;

		case 0x00a: // LF
		case 0x00d: // CR
			continue;

		default:
			LOGWARN(g_Log, DBSTR "text type not found");
		}
		i++; // Clear letter
		i++; // Clear space

		// Count number of chars in this line
		U32 chars = 0;
		for(U32 j = i; j < count && text_block[j] != 0x000d; j++)
			chars++;
		assert(chars);

		if(ids)
		{
			char buffer[128];
			WCHAR wbuffer[128];

			assert(chars<128);

			// Make char version for Loc lookup
			memcpy(wbuffer, text_block + i, sizeof(WCHAR)*chars);
			wbuffer[chars] = 0;
			_snprintf(buffer, 128, "CRE_%S", wbuffer);
			
			// Do lookup
			m_Line[lines].m_StringID = buffer;

			if(m_Line[lines].m_StringID.IsLocString())
				m_Line[lines].m_Chars = NULL;
			else
				ids = false; // Just print the string
		}
		// No lookup - just print the string
		if(!ids)
		{
			m_Line[lines].m_Chars = new WCHAR[chars+1];
			memcpy(m_Line[lines].m_Chars, text_block + i, sizeof(WCHAR)*chars);
			m_Line[lines].m_Chars[chars] = 0;
		}
		m_Line[lines].m_Type = type;

		i+=chars;	// Move along
		i++;		// Clear cr
		lines++;	// Next line
	}
	m_LineCount = lines;
	m_Built = true;

	delete [] text_block;

	MEMORY_TRACKER_POP("ui.legal");
}

void UILegal::Init()
{
	C_GadgetBase::Init();

	// messages
	AddSubscriber( E_UiMsg_FrameUpdate );
	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlLeftStick_Y );
	AddSubscriber( E_UiMsg_CtrlMouse_Pressed );
}

void UILegal::Setup( lua_State *pState )
{
	C_GadgetBase::Setup( pState );

	m_DisplayStyle = E_DisplayStyle_Scroller;
	m_AutoScroll = E_ScrollSpeed_Off; // off

	// Validate sorcery array sizes
	if( m_ColourStyleName.count < C_Text::C_Line::E_Type_Count || 
		m_Style.count < C_Text::C_Line::E_Type_Count ||
		m_Height.count < C_Text::C_Line::E_Type_Count ||
		m_PreCr.count < C_Text::C_Line::E_Type_Count ||
		m_PostCr.count < C_Text::C_Line::E_Type_Count )
	{
		LOGWARN( g_Log, DBSTR "Sorcery array not big enough");
		m_Activated = false;
	}

	// Resolve colour styles and setup font styles
	for( int i = 0; i < C_Text::C_Line::E_Type_Count; i++ )
	{
		m_Colour[ i ].SetByName( m_ColourStyleName[ i ], m_Alpha, m_Intensity );
		m_Style[ i ].Setup( pState );
	}

	m_OriginalScrollSpeed=m_ScrollSpeed;
	// Reset
	m_Offset = m_StartPos;

	const U32 lc = UILegal::C_Text::Instance().LineCount();
	for(U32 i = 0; i < lc; i++)
	{
		U32 lines = 0;
		C_TextStringID id = UILegal::C_Text::Instance().Line(i).m_StringID;

		C_FontStyle& style = (m_Style[ UILegal::C_Text::Instance().Line(i).m_Type ].NumFontStyles() > 0) ? m_Style[ UILegal::C_Text::Instance().Line(i).m_Type ].GetFontStyle( 0 ) : C_FontStyle::Instance();

		F32 height = m_Height[UILegal::C_Text::Instance().Line(i).m_Type];

		gFont.StartFrame();

		if ( !id.IsLocString() )
			lines = gFont.StringNumLines( UILegal::C_Text::Instance().Line( i ).m_Chars, height, m_Size.x, style );
		else
			lines = gFont.StringNumLines( id, height, m_Size.x, style );
		
		mNumLines.PushBack(lines);
	}
	
	C_GadgetBase::Setup( pState );
}

bool UILegal::MessageHandler( const Messaging::C_MessageBase* pMessage )
{
	if ( !m_Activated )
		return C_GadgetBase::MessageHandler( pMessage );

	if ( !pMessage )
		return false;

	const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;

	switch ( pUIMessage->Type() )
	{
		case E_UiMsg_CtrlButton_Up:
			// toggle auto-scroll
			if( m_AutoScroll < E_ScrollSpeed_Off )
				m_AutoScroll = E_ScrollSpeed_Off;
			else if( m_AutoScroll < E_ScrollSpeed_UFast )
				m_AutoScroll++;
			break;

		case E_UiMsg_CtrlButton_Down:
			// toggle auto-scroll
			if( m_AutoScroll > E_ScrollSpeed_Off )
				m_AutoScroll = E_ScrollSpeed_Off;
			else if( m_AutoScroll > E_ScrollSpeed_DFast )
				m_AutoScroll--;
			break;

		case E_UiMsg_CtrlLeftStick_Y:
			// scroll
			m_ScrollSpeed = m_OriginalScrollSpeed + ( pUIMessage->amount * -1024.0f );
			m_AutoScroll = E_ScrollSpeed_Off;
			break;

		case E_UiMsg_CtrlMouse_Pressed:
		{
			if ( pUIMessage->pad == 1 ) // mouse button 1
			{
				// scroll up
				m_ScrollSpeed = m_OriginalScrollSpeed + 1024.f;
				m_AutoScroll = E_ScrollSpeed_Off;
			}
			else if ( pUIMessage->pad == 2 ) // mouse button 2
			{
				// scroll down
				m_ScrollSpeed = m_OriginalScrollSpeed - 1024.f;
				m_AutoScroll = E_ScrollSpeed_Off;
			}
			break;
		}

		default:
			return C_GadgetBase::MessageHandler( pMessage );
	}

	return true;
}

void UILegal::FrameUpdate()
{
	C_GadgetBase::FrameUpdate();

	switch( m_AutoScroll )
	{
	case E_ScrollSpeed_DFast : // down
		m_ScrollSpeed = m_OriginalScrollSpeed + 1024.f;
		break;

	case E_ScrollSpeed_DSlow : // down
		m_ScrollSpeed = m_OriginalScrollSpeed + 512.f;
		break;

	case E_ScrollSpeed_USlow : // up
		m_ScrollSpeed = m_OriginalScrollSpeed - 512.f;
		break;

	case E_ScrollSpeed_UFast : // up
		m_ScrollSpeed = m_OriginalScrollSpeed - 1024.f;
		break;
	}

	// Do the scroll
	m_Offset += (m_ScrollSpeed*theFETimer.FrameTime());
	if( m_Offset < m_StartPos )
		m_Offset = m_StartPos;

	m_ScrollSpeed=m_OriginalScrollSpeed;
}

void UILegal::Render( )
{
	C_GadgetBase::Render();

	if(!m_Activated)
		return;

	if(!UILegal::C_Text::Instance().Built())
	{
		_UI_MESSAGE( E_UiMsg_LegalFinished );
		return;
	}

	// auto-transform
	C_UIAutoTransform transform( this );

	if(m_DisplayStyle == E_DisplayStyle_Scroller)
		RenderScroller();
}

void UILegal::RenderScroller( )
{
	Maths::HVec3 pos(0.0f,m_Offset*Up(),0.0f);

	const U32 lc = UILegal::C_Text::Instance().LineCount();
	const F32 fade_distance = m_Height[C_Text::C_Line::E_Type_MainHeading];
	for(U32 i = 0; i < lc; i++)
	{
		int num_lines=1;
		const C_Text::C_Line::E_Type type = UILegal::C_Text::Instance().Line(i).m_Type;

		C_RenderState* render_state = &m_RenderState;
		C_UISpriteColours colour = m_Colour[type];
		F32 height = m_Height[type];
		F32 alpha;
		U8 col;

		pos += Maths::HVec3(0.0f,m_PreCr[type],0.0f);

		F32 y;

		if(pos.Y() < m_FadeBottom * 0.5f )
		{
			y = pos.Y();
			{
				alpha = y / m_FadeLength;
				alpha = (alpha<0.0f) ? 0.0f : (alpha>1.0f) ? 1.0f : alpha;
				col = (U8)(alpha*255.0f);
			}
			if( gSimple2D.IsPanel3D( m_Panel ) )
			{
				colour[2].A( col );
				colour[3].A( col );
			}
			else
			{
				colour[0].A( col );
				colour[1].A( col );
			}

			y = pos.Y() + height;
			{
				alpha = y / m_FadeLength;
				alpha = (alpha<0.0f) ? 0.0f : (alpha>1.0f) ? 1.0f : alpha;
			}
			if( gSimple2D.IsPanel3D( m_Panel ) )
			{
				colour[0].A( col );
				colour[1].A( col );
			}
			else
			{
				colour[2].A( col );
				colour[3].A( col );
			}
		}
		else
		{
			y = pos.Y() - (m_FadeBottom - m_FadeLength);
			{
				alpha = y / m_FadeLength;
				alpha = (alpha<0.0f) ? 0.0f : (alpha>1.0f) ? 1.0f : alpha;
				col = 255 - (U8)(alpha*255.0f);
			}
			if( gSimple2D.IsPanel3D( m_Panel ) )
			{
				colour[2].A( col );
				colour[3].A( col );
			}
			else
			{
				colour[0].A( col );
				colour[1].A( col );
			}

			y = pos.Y() - (m_FadeBottom - m_FadeLength);
			{
				alpha = y / m_FadeLength;
				alpha = (alpha<0.0f) ? 0.0f : (alpha>1.0f) ? 1.0f : alpha;
			}
			if( gSimple2D.IsPanel3D( m_Panel ) )
			{
				colour[0].A( col );
				colour[1].A( col );
			}
			else
			{
				colour[2].A( col );
				colour[3].A( col );
			}
		}

		if(colour[3].A() ||colour[0].A() )
		{
			C_TextStringID id = UILegal::C_Text::Instance().Line(i).m_StringID;

			C_FontStylePasses& style = m_Style[ type ];

			const WCHAR* pString = NULL;

			if ( !id.IsLocString() )
				pString = UILegal::C_Text::Instance().Line( i ).m_Chars;
			else
				pString = id.String();

			gFont.DrawText( m_Layer, Maths::HVec3( m_Position.x, m_Position.y - pos.Y(), m_Position.z ), m_Justify, pString, colour, height, style, m_Size.x, m_Panel );
		}

		pos += Maths::HVec3( 0.0f, height, 0.0f ) * mNumLines[i];
		pos += Maths::HVec3(0.0f, m_PostCr[type], 0.0f);
	}

	// Reset at the end
	if(pos.Y() + fade_distance < 0.0f )
	{
		m_Offset = m_StartPos;
		_UI_MESSAGE( E_UiMsg_LegalFinished );
	}
}

void UILegal::Exit()
{
	mNumLines.Destroy();
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
