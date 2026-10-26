#ifndef UIRACELINESPLITSCREEN_H
#define UIRACELINESPLITSCREEN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>

#include "ui/UIGameDefines.h"

#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

namespace UI
{

// -------------------------------
// C_UIRaceLineSortInfoSplitscreen
// -------------------------------
class C_UIRaceLineSortInfoSplitscreen
{
public:
	C_UIRaceLineSortInfoSplitscreen()
		: m_fValue01( 0.0f )
		, m_sVehicleIndex( -1 )
		, m_bActive( true )
		, m_bPlayer( false )
	{
		//
	}

public:
	bool operator < ( const C_UIRaceLineSortInfoSplitscreen& o ) const;

public:
	F32				m_fValue01;
	S32				m_sVehicleIndex;
	bool			m_bActive;
	bool			m_bPlayer;
};

// ---------------------
// UIRaceLineSplitscreen
// ---------------------
class UIRaceLineSplitscreen : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_RaceLineSplitscreen, "RACE_LINE_SPLITSCREEN" )

public:
	void	Init();
	void	Setup( lua_State* pState );
	void	Render();

private:
	void	GatherSortInfo();

public: // exposed for Xt
	F32					m_fVehicleWidth;
	F32					m_fVehicleHeight;
	F32					m_fIconWidthStart;
	F32					m_fIconHeightStart;
	F32					m_fIconWidthEnd;
	F32					m_fIconHeightEnd;

	XtSz8				m_xtColourYou0;
	XtSz8				m_xtColourYou1;
	XtSz8				m_xtColourYou2;
	XtSz8				m_xtColourYou3;
	XtSz8				m_xtColourOther;
	XtSz8				m_xtColourStart;
	XtSz8				m_xtColourEnd;
	
	F32					m_fPlayerOffsetY;

private:
	C_UISpriteColours	m_ColourYou0;
	C_UISpriteColours	m_ColourYou1;
	C_UISpriteColours	m_ColourYou2;
	C_UISpriteColours	m_ColourYou3;
	C_UISpriteColours	m_ColourOther;
	C_UISpriteColours	m_ColourStart;
	C_UISpriteColours	m_ColourEnd;

	Btl::Vector< C_UIRaceLineSortInfoSplitscreen >	m_SortInfo;

	S32					m_sNumVehicles;
	F32					m_fMaxLogic01;

	bool				m_HideBecauseRaceModeIsNonLapBased;
};

};	// namespace UI

#endif /* UIRACELINESPLITSCREEN_H */
