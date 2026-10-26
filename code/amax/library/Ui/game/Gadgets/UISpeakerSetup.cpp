#include "stdafx.h"

#include "Game/Gadgets/UISpeakerSetup.h"
#include "Ui/SoundEffects.h"
#include <ui/Game/Gadgets/ButtonMessages.h>

// XT Definition
XT_TYPE( UI::UISpeakerSetup, "UISpeakerSetup" )
	XT_BASE( UI::UIStaticText )
	XT_FIELD( m_FontColourName, "font_colour_name" )
	XT_FIELD( m_UnSelectedFontColourName, "unselected_font_colour_name" )
	XT_FIELD( m_UnSelectedColourName, "unselected_colour_name" )
	XT_FIELD( m_fFontHeight, "font_height") 
	XT_FIELD( m_fFontOffsetX, "font_offset_x" )
	XT_FIELD( m_fFontOffsetY, "font_offset_y" )
	XT_FIELD( m_FontJustify, "font_justify" )
	XT_FIELD( m_fGraphicSize, "graphic_size" )
	XT_FIELD( m_fSpeakerClickRadius, "speaker_radius" )
XT_END_TYPE

namespace UI
{

void UISpeakerSetup::Init()
{
	C_GadgetBase::Init();

	m_bHasResolvedColours = false;

	m_fSpeakerClickRadius = 0.0f;

	m_sMouseDraggingSpeaker = -1;
	m_sPrevMouseDraggingSpeaker = -1;

	m_sCurrentSpeaker = 0;
	m_sMouseDraggingSpeaker = -1;
	
	m_fSpeakerAngles[ E_UISpeaker_Left ]		= 240.0f;
	m_fSpeakerAngles[ E_UISpeaker_Right ]		= 300.0f;
	m_fSpeakerAngles[ E_UISpeaker_RearRight ]	= 20.0f;
	m_fSpeakerAngles[ E_UISpeaker_RearLeft ]	= 160.0f;

	AddSubscriber( E_UiMsg_CtrlButton_Up );
	AddSubscriber( E_UiMsg_CtrlButton_Down );
	AddSubscriber( E_UiMsg_CtrlRightStick_Y );

	UIStaticText::Init();
}

void UISpeakerSetup::Setup( lua_State* pState )
{
	m_FontColour.SetByName( m_FontColourName );
	m_UnSelectedFontColour.SetByName( m_UnSelectedFontColourName );
	m_UnSelectedColour.SetByName( m_UnSelectedColourName );

	UIStaticText::Setup( pState );
}

bool UISpeakerSetup::MessageHandler(const Messaging::C_MessageBase *pMessage)
{
	if ( !m_Activated )
		return UIStaticText::MessageHandler( pMessage );

	switch( pMessage->Type() )
	{
		case E_UiMsg_CtrlButton_Down:
			m_sCurrentSpeaker++;
			if ( m_sCurrentSpeaker > 5 ) 
				m_sCurrentSpeaker = 5;

			UiInterface()->PlaySound( E_UiSoundEffect_Toggle );
			return true;

		case E_UiMsg_CtrlButton_Up:
			m_sCurrentSpeaker--;
			if ( m_sCurrentSpeaker < 0 )
				m_sCurrentSpeaker = 0;

			UiInterface()->PlaySound( E_UiSoundEffect_Toggle );
			return true;

		case E_UiMsg_CtrlRightStick_Y:
		{
			const C_UiMessagePadData* pUIMessage = (C_UiMessagePadData*)pMessage;
			const F32 fAmount = pUIMessage->amount / 2.0f;
			
			switch ( m_sCurrentSpeaker )
			{
				case 0:
					m_fSpeakerAngles[ E_UISpeaker_Left ]		= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_Left ]	+ fAmount, 180.0f, 270.0f );
					m_fSpeakerAngles[ E_UISpeaker_Right ]		= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_Right ] - fAmount, 270.0f, 360.0f );
					break;
				case 1:
					m_fSpeakerAngles[ E_UISpeaker_RearLeft ]	= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_RearLeft ]	+ fAmount, 90.0f, 180.0f );
					m_fSpeakerAngles[ E_UISpeaker_RearRight ]	= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_RearRight ] - fAmount, 0.0f, 90.0f );
					break;
				case 2:
					m_fSpeakerAngles[ E_UISpeaker_Left ]		= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_Left ]	+ fAmount, 180.0f, 270.0f );
					break;
				case 3:
					m_fSpeakerAngles[ E_UISpeaker_Right ]		= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_Right ] - fAmount, 270.0f, 360.0f );
					break;
				case 4:
					m_fSpeakerAngles[ E_UISpeaker_RearRight ]	= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_RearRight ] - fAmount, 0.0f, 90.0f );
					break;
				case 5:
					m_fSpeakerAngles[ E_UISpeaker_RearLeft ]	= Maths::Clamped( m_fSpeakerAngles[ E_UISpeaker_RearLeft ]	+ fAmount, 90.0f, 180.0f );
					break;
				default:
					break;
			}
		}
		return true;
	}
#if 0 // TODO
#ifdef PLATFORM_PC
	// Handle mouse
	if(mData.mouse_isover || m_sPrevMouseDraggingSpeaker != -1)
	{
		F32 mouse_x;
		F32 mouse_y;

		UI::UiInterface()->GetMousePos(mouse_x, mouse_y);
		mouse_x *= gSimple2D.ScreenWidth();
		mouse_x += gSimple2D.ScreenLeft();
		mouse_y *= UI_VSCREEN_SIZEY;

		if(m_sPrevMouseDraggingSpeaker != -1) // Currently dragging one around
		{
			// Still held?
			if(strcmp(pMessage->Type,"BUTTON_LMOUSE")==0)
			{
				m_sMouseDraggingSpeaker = m_sPrevMouseDraggingSpeaker;

				F32 x = (mData.drawx + mData.drawsizex / 2) - mouse_x;
				F32 y = (mData.drawy + mData.drawsizey / 2) - mouse_y;

				// Get current mouse angle
				F32 dist = sqrtf((x*x) + (y*y));
				if(dist > 0.0f)
				{
					// Sheesh!
					F32 angle = acosf(-y / dist);
					if(x < 0)
						angle = 180.0f2 - angle;

					angle += 90.0f;
					switch ( m_sCurrentSpeaker )
					{
						case 2:
							m_fSpeakerAngles[ E_UISpeaker_Left ]		= Maths::Clamped( angle, 180.0f, 180.0f + ( 180.0f / 2 ) );
							break;
						case 3:
							m_fSpeakerAngles[ E_UISpeaker_Right ]		= Maths::Clamped( angle, 180.0f + ( 180.0f / 2 ), 180.0f * 2 );
							break;
						case 4:
							while(angle >= 180.0f)
								angle -= 180.0f2;
							m_fSpeakerAngles[ E_UISpeaker_RearRight ]	= Maths::Clamped( angle, (F32)0, 180.0f / 2 );
							break;
						case 5:
							while(angle >= 180.0f2)
								angle -= 180.0f2;
							m_fSpeakerAngles[ E_UISpeaker_RearLeft ]	= Maths::Clamped( angle, 180.0f / 2, 180.0f );
							break;
						default:
							break;
					}
				}
			}
		}
		else
		{
			if(strcmp(pMessage->Type,"BUTTON_LMOUSE_DEB")==0 && mData.mouse_isover)
			{
				// Find out if we're near any speaker?
				F32 nearest_dist = m_fSpeakerClickRadius;
				S32 current_clicked = -1;
				for ( S32 i = 0; i < E_UISpeaker_Count; i++ )
				{
					F32 x = mData.drawx + mData.drawsizex / 2;
					F32 y = mData.drawy + mData.drawsizey / 2;
					F32 a = Maths::DegToRad( m_fSpeakerAngles[ i ] );

					x += Maths::Cos( a ) * ( mData.drawsizex / 2 );
					y += Maths::Sin( a ) * ( mData.drawsizey / 2 );

					x -= mouse_x;
					y -= mouse_y;

					F32 dist = sqrtf((x*x)+(y*y));
					if(dist < nearest_dist)
					{
						nearest_dist = dist;
						current_clicked = i;
					}
				}
				m_sMouseDraggingSpeaker = current_clicked;
				m_sCurrentSpeaker = (current_clicked == -1) ? 0 : current_clicked + 2;
			}
		}
	}
#endif
#endif

	return UIStaticText::MessageHandler( pMessage );
}

void UISpeakerSetup::FrameUpdate()
{
	if ( !m_Activated )
		return;

	m_sPrevMouseDraggingSpeaker = m_sMouseDraggingSpeaker;
	m_sMouseDraggingSpeaker = -1;

	UIStaticText::FrameUpdate();
}

void UISpeakerSetup::Render()
{
	if ( !m_Activated )
		return;

	
	// bit weird, but these are set with SetXtVar after cloning and so would need re-resolving
	if ( !m_bHasResolvedColours )
	{
		m_bHasResolvedColours = true;

		m_FontColour.SetByName( m_FontColourName );
		m_UnSelectedFontColour.SetByName( m_UnSelectedFontColourName );
		m_UnSelectedColour.SetByName( m_UnSelectedColourName );
	}
	
#ifdef _UI_AUTO_CLIPWINDOW
	// clip window
	C_UIClipWindow clipW = gSimple2D.AddClipWindowAuto( this );
#endif

	// auto-transform
	C_UIAutoTransform transform( this );

	//centre speaker
	Maths::HVec3 cp( m_Position.x, m_Position.y - ( m_Size.y * Up() * 0.5f), 0.0f );
	Maths::HVec3 fo_x( m_fFontOffsetX, 0.0f, 0.0f );
	Maths::HVec3 fo_y( 0.0f, m_fFontOffsetY, 0.0f );
	gSimple2D.AddSprite( m_Layer, cp, m_fGraphicSize, m_fGraphicSize, m_UnSelectedColour, m_Panel, E_SpriteType_Sprite, m_RenderState );
	gSimple2D.AddTexUVs( 0, GetTextureParam( 0 ) );
	gSimple2D.AddJustification( E_Justify_MiddleCentre );
	
	C_TextStringID id( "CMN_OPT_AUDIO_SPEAKER_CENTRE" );
	gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
	gFont.DrawText( m_Layer, cp + fo_y*1.5, m_FontJustify, id, m_UnSelectedFontColour, m_fFontHeight, m_FontStylePasses, 0, m_Panel );
	
	// now draw the speakers
	F32 x = 0.0f;
	F32 y = 0.0f;
	F32 z = m_Position.z;
	
	const WCHAR* pStrings[ E_UISpeaker_Count ] =
	{
		C_TextStringID( "CMN_OPT_AUDIO_SPEAKER_FRONT_LEFT" ).String(),
		C_TextStringID( "CMN_OPT_AUDIO_SPEAKER_FRONT_RIGHT" ).String(),
		C_TextStringID( "CMN_OPT_AUDIO_SPEAKER_REAR_RIGHT" ).String(),
		C_TextStringID( "CMN_OPT_AUDIO_SPEAKER_REAR_LEFT" ).String(),
	};

	for ( S32 i = 0; i < E_UISpeaker_Count; i++ )
	{
		x = m_Position.x;
		y = m_Position.y;

		F32 a = Maths::DegToRad( m_fSpeakerAngles[ i ] );

		x += Maths::Cos( a ) * (m_Size.x * 0.5f);
		y += Maths::Sin( a ) * (m_Size.y * 0.5f) * Up();

		Maths::HVec3 pos( x, y, z );
		bool selected = ( ( i < 2 && m_sCurrentSpeaker == 0 ) || ( i > 1 && m_sCurrentSpeaker == 1 ) || ( i == m_sCurrentSpeaker - 2 ) );
		C_UISpriteColours& c = selected ? m_SpriteColours : m_UnSelectedColour;
		
		gSimple2D.AddSprite( m_Layer, pos, m_fGraphicSize, m_fGraphicSize, c, m_Panel, E_SpriteType_Sprite, m_RenderState );
		gSimple2D.AddTexUVs( 0, GetTextureParam( (i == E_UISpeaker_Left || i == E_UISpeaker_Right) ? 1 : 2 ) );
		gSimple2D.AddJustification( E_Justify_MiddleCentre );
		gSimple2D.AddRotation( 0, 0, (m_fSpeakerAngles[ i ] + 90.0f) * Up() );

		C_WideString32	temp_str;
		if ( i < 2 )
			temp_str.Format( pStrings[ i ], Maths::Abs( m_fSpeakerAngles[ i ] - 270.0f ) );
		else if ( i == 2 )
			temp_str.Format( pStrings[ i ], Maths::Abs( m_fSpeakerAngles[ i ] + 90.0f ) );
		else if ( i == 3 )
			temp_str.Format( pStrings[ i ], Maths::Abs( 270.0f - m_fSpeakerAngles[ i ] ) );
		
		gFont.FixedSpaceNext( E_FixedSpaceType_Numbers );
		gFont.DrawText( m_Layer, pos + (i<2 ? fo_y : fo_y*-1) + ( i==1 || i==2 ? fo_x : fo_x*-1), m_FontJustify, temp_str.Chars(), selected ? m_FontColour : m_UnSelectedFontColour, m_fFontHeight, m_FontStylePasses, 0, m_Panel );
	}

	C_GadgetBase::Render();
}

}; // namespace UI
