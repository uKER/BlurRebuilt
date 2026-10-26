#ifndef UIMINIMAP_H
#define UIMINIMAP_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>

#include "ui/UIGameDefines.h"
#include "UIRaceLine.h"

#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

namespace UI
{

// ----------
// UIRaceLine
// ----------
class UIMiniMap : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_MiniMap, "MINI_MAP" )

public:
	void	Init();
	void	PostInit();
	void	Setup( lua_State* pState );
	void	FrameUpdate();
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	Render();

private:
	const bool IsSilentRunningHidingThisVehicle(Game::C_LevelState& levelState, const S32 vehicleIndex);

public:
	XtF32				m_fVehicleWidth;
	XtF32				m_fVehicleHeight;
	XtF32				m_fClampFraction;
	XtF32				m_fFadeFraction;
	XtF32				m_fMapAlpha;
	XtF32				m_fBattleViewDist;
	XtF32				m_fMinViewDist;
	XtF32				m_fMaxViewDist;

	XtSz8				m_xtColourYou;
	XtSz8				m_xtColourMap;
	XtSz8				m_xtColourTargeted;
	XtSz8				m_xtColourPulseWinner;
	XtSz8				m_xtColourPulseRival;
	XtSz8				m_xtColourDestructionEasy;
	XtSz8				m_xtColourDestructionMedium;
	XtSz8				m_xtColourDestructionHard;
	XtSz8				m_xtColourDestructionVHard;
	XtSz8				m_xtColourFirst;
	XtSz8				m_xtColourSecond;
	XtSz8				m_xtColourThird;
	XtSz8				m_xtColourNone;
	XtSz8				m_xtColourVP0;
	XtSz8				m_xtColourVP1;
	XtSz8				m_xtColourVP2;
	XtSz8				m_xtColourVP3;

public:
	Maths::HVec3		m_MapSize;
	Maths::HVec3		m_MapOffset;
	Maths::HVec3		m_WorldPosMe;
	F32					m_fCurrentViewDist;

private:
	S32					m_sLocalPlayerIndex;
	F32					m_fTopSpeedInMph;
	bool				m_bIsNormalView;
	bool				m_bRouteShapeExists;
	bool				m_bCheckHeight;
	F32					m_fRivalIntensityTimer;

	bool				m_bSilentRunningHidesPlayersOnRadar;

	static const U32	m_MaxViewports = 4;
	
	C_GadgetBase*		m_pDummyScale;
	C_GadgetBase*		m_pDummyTranslate;

	C_UIRaceLineSorter	m_Sorter;

	C_UISpriteColours	m_ColourYou;
	C_UISpriteColours	m_ColourMap;
	C_UISpriteColours	m_ColourTargeted;
	C_UISpriteColours	m_ColourPulseWinner;
	C_UISpriteColours	m_ColourPulseRival;
	// Destruction
	C_UISpriteColours	m_ColourDestructionEasy;
	C_UISpriteColours	m_ColourDestructionMedium;
	C_UISpriteColours	m_ColourDestructionHard;
	C_UISpriteColours	m_ColourDestructionVHard;
	// Positional
	C_UISpriteColours	m_ColourFirst;
	C_UISpriteColours	m_ColourSecond;
	C_UISpriteColours	m_ColourThird;
	C_UISpriteColours	m_ColourNone;
	// SplitScreen viewports
	C_UISpriteColours	m_ColourVP[m_MaxViewports];

	// Render rival backing with intensity as alpha
	C_RenderState		m_RivalBackingRenderState;

	// Helper functions for pulsing the winner and rival cars
	void				SetupRTGadgets();
	bool				GetIsRival(const Game::C_LevelState& levelState, Game::Actors::C_VehicleLogicActor* pLogicActorMe, U8 vehicleIndex ) const;
	bool				GetIsWinner(const Game::C_LevelState& levelState,  U8 vehicleIndex ) const;
	float				CalculateLerpValue();
	void				PulseColour(const bool bIsWinner, const bool bIsRival, const float lerpVal, C_UISpriteColours& colours);

};

};	// namespace UI

#endif /* UIMINIMAP_H */
