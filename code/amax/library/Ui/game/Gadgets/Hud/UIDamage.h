#ifndef UIHUDDAMAGE_H
#define UIHUDDAMAGE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>

#include "ui/UIGameDefines.h"
#include "gamelogic\racelogiccore\data\RLEnums.h"

#include "System/misc/Random.h"

namespace UI
{

class UIStaticGraphic;
class UIStaticText;

// -------------------
// C_UiDamageSlotParam
// -------------------
class C_UiDamageSlotParam
{
public:
	F32		m_fScale;
	F32		m_fAlpha;
};

// --------
// UIDamage
// --------
class UIDamage : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Damage, "DAMAGE" )

public:
	UIDamage() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:
	void	Init();
	void	Setup( lua_State* pState );
	void	PostInit();
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	FrameUpdate();
	void	Render();

	void	SetVehicleIndexOverride( const S32 sVehicleIndexOverride )		{ m_sVehicleIndexOverride = sVehicleIndexOverride; }

private:
	void	DoDamagedText();
	void	DoMirrorSmash();

public: // exposed for Xt
	XtSz8								m_xtColourHigh;
	XtSz8								m_xtColourMedium;
	XtSz8								m_xtColourLow;
	XtSz8								m_xtColourEmpty;
	F32									m_fColourEmptyAlpha;
	F32									m_fGapMult;
	XtArray< C_UiDamageSlotParam >		m_Slot;
	S32									m_sVehicleIndexOverride;
	bool								m_bBoss;
	bool								m_bAlwaysDoEffect;
	
private:
	S32									m_sLocalPlayerIndex;

	E_HEALTH_LEVEL						m_eHealthLevel;
	E_HEALTH_LEVEL						m_eHealthLevelPrev;
	S32									m_sCurrentHealth;

	bool								m_bDamageEffectStarted;
	bool								m_bDamageWarning;

	C_UISpriteColours					m_ColourHigh;
	C_UISpriteColours					m_ColourMedium;
	C_UISpriteColours					m_ColourLow;
	C_UISpriteColours					m_ColourEmpty;

	F32									m_fSegmentSizeX;
	F32									m_PositionOriginalX;

	S32									m_sMaxSegments;
	UIStaticText*						m_pHealthText;
	UIStaticGraphic*					m_pMirrorL;
	UIStaticGraphic*					m_pMirrorR;

	C_RenderState						m_RenderStateScale;

	// sp destruction
	bool								m_bIsDestruction;
	U32									m_uAmaxVehicleIdEasy;
	U32									m_uAmaxVehicleIdMedium;
	U32									m_uAmaxVehicleIdHard;
	U32									m_uAmaxVehicleIdVHard;
};

};	// namespace UI

#endif /* UIHUDDAMAGE_H */
