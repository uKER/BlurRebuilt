#ifndef UITARGETING_H
#define UITARGETING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>

#include "GameLogic/RaceLogicCore/Data/RLDefinitions.h"

namespace UI
{

class UIStaticText;
class C_UiScoringEventPerk;

// -----------
// UITargeting
// -----------
class UITargeting : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Targeting, "TARGETING" )

public:
	UITargeting() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }
	
	void	Init();
	void	Setup( lua_State* pState );
	void	PostInit();
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	FrameUpdate();
	void	Render();

	void	SetVehicleIndexOverride( const S32 sVehicleIndexOverride )		{ m_sVehicleIndexOverride = sVehicleIndexOverride; }

	void	ResetTargets();
	bool	PreviouslyTargetted()						{ return m_PrevTargetted; }

public: // exposed for Xt
	XtSz8				m_xtColourTargeted;
	F32					m_fColourLerp;
	F32					m_fScaleMult;
	F32					m_fTargetedIntensityAdd;

private:
	void				ProcessHitByPerk( const C_UiScoringEventPerk* pMessage );
	void				SetupGadgets();

private:
	S32					m_sLocalPlayerIndex;
	S32					m_sVehicleIndexOverride;
	bool				m_PrevTargetted;
	UIStaticText*		m_pPerkedText;
	F32					m_fMessageTimer;


	Btl::Vector< bool, Btl::VectorParameters_Array< MAX_VEHICLES > >	m_VehicleInRange;		// AI

	Btl::Vector< bool, Btl::VectorParameters_Array< 4 > >				m_SegmentActive;
	Btl::Vector< C_GadgetBase*, Btl::VectorParameters_Array< 4 > >		m_Gadgets;

	bool				m_bPlayerInRangeLocked;
	bool				m_bEffectsStartedLocked;

	Btl::Vector< bool, Btl::VectorParameters_Array< 4 > >				m_PlayerInRange;		// player's HUD
	Btl::Vector< bool, Btl::VectorParameters_Array< 4 > >				m_EffectsStartedInRange;

	C_UISpriteColours	m_ColourTargeted;
};

};	// namespace UI

#endif /* UITARGETING_H */
