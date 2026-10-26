#ifndef UISPEAKERSETUP_H
#define UISPEAKERSETUP_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIManager.h"

#include <ui/Game/Gadgets/UIStaticGraphic.h>
#include "ui/UIGameDefines.h"

namespace UI
{

enum E_UISpeaker
{
	E_UISpeaker_Left = 0,
	E_UISpeaker_Right,
	E_UISpeaker_RearRight,
	E_UISpeaker_RearLeft,

	E_UISpeaker_Count,
};

class UISpeakerSetup : public UIStaticText
{
public:
	DECLARE_GADGET( E_UIGameButtonType_SpeakerSetup, "SPEAKER_SETUP" );

	void		Init();
	void		Setup( lua_State* pState );
	bool		MessageHandler(const Messaging::C_MessageBase *pMessage);
	void		FrameUpdate();
	void		Render();

	F32			GetFL() const { return m_fSpeakerAngles[ E_UISpeaker_Left ]; }
	F32			GetFR() const { return m_fSpeakerAngles[ E_UISpeaker_Right ]; }
	F32			GetRR() const { return m_fSpeakerAngles[ E_UISpeaker_RearRight ]; }
	F32			GetRL() const { return m_fSpeakerAngles[ E_UISpeaker_RearLeft ]; }

	void		SetFL( const F32 fFL )	{ m_fSpeakerAngles[ E_UISpeaker_Left ] = fFL; }
	void		SetFR( const F32 fFR )	{ m_fSpeakerAngles[ E_UISpeaker_Right ] = fFR; }
	void		SetRR( const F32 fRR )	{ m_fSpeakerAngles[ E_UISpeaker_RearRight ] = fRR; }
	void		SetRL( const F32 fRL )	{ m_fSpeakerAngles[ E_UISpeaker_RearLeft ] = fRL; }
// Exposed for XT
public:
	XtSz8				m_FontColourName;
	XtSz8				m_UnSelectedFontColourName;
	XtSz8				m_UnSelectedColourName;
	XtI32				m_sController;
	XtF32				m_fGraphicSize;
	XtF32				m_fSpeakerClickRadius;
	XtF32				m_fFontHeight;
	XtF32				m_fFontOffsetX;
	XtF32				m_fFontOffsetY;
	E_Justify			m_FontJustify;
					
private:			
	S32					m_sCurrentSpeaker;
	F32					m_fSpeakerAngles[ E_UISpeaker_Count ];
					
	S32					m_sMouseDraggingSpeaker;
	S32					m_sPrevMouseDraggingSpeaker;

	C_UISpriteColours	m_FontColour;
	C_UISpriteColours	m_UnSelectedFontColour;
	C_UISpriteColours	m_UnSelectedColour;

	bool				m_bHasResolvedColours;
};

}; // namespace UI

#endif // UISPEAKERSETUP_H
