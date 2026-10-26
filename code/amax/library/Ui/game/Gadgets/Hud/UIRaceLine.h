#ifndef UIRACELINE_H
#define UIRACELINE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>

#include "ui/UIGameDefines.h"

#include "UIPerks.h" // C_UIPerkInfo
#include "Game/GameActors/VehicleLogic/VehicleLogicActor.h"

namespace UI
{

// --------------------
// C_UIRaceLineSortInfo
// --------------------
class C_UIRaceLineSortInfo
{
public:
	C_UIRaceLineSortInfo()
		: m_fValue01( 0.0f )
		, m_pPerkInfo( NULL )
		, m_sVehicleIndex( -1 )
		, m_bActive( true )
		, m_bTracking( false )
	{
		//
	}

public:
	bool operator < ( const C_UIRaceLineSortInfo& o ) const;

public:
	F32				m_fValue01;
	C_UIPerkInfo*	m_pPerkInfo;
	S32				m_sVehicleIndex;
	bool			m_bActive;
	bool			m_bTracking;
};

typedef Btl::Vector< C_UIRaceLineSortInfo >	C_UIRaceLineSortArray;

// ----------------------
// C_UIRaceLineEffectInfo
// ----------------------
class C_UIRaceLineEffectInfo
{
public:
	C_UIRaceLineEffectInfo()
		: m_bActive( false )
		, m_fTime01( 0.0f )
		, m_ePerkType( E_PICKUP_TYPE_NONE )
		
	{
		//
	}

public:
	bool			m_bActive;
	F32				m_fTime01;
	E_PICKUP_TYPE	m_ePerkType;
};

class C_UIRaceLineSorter
{
public:
	C_UIRaceLineSorter()	: m_sNumVehicles( 0 ) { }

	void					Init( const S32 sNumVehicles );
	C_UIRaceLineSortArray&	GatherSortInfo( const S32 sVehicleIndexMe );
	
private:
	C_UIRaceLineSortArray	m_SortInfo;
	S32						m_sNumVehicles;
};

// ----------
// UIRaceLine
// ----------
class UIRaceLine : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_RaceLine, "RACE_LINE" )

public:
	UIRaceLine() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:
	void	Init();
	void	Setup( lua_State* pState );
	void	FrameUpdate();
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	Render();

private:
	void			ProcessHitByPerk( const Messaging::C_MessageBase* pMessage );

	void			RenderLine();

public: // exposed for Xt
	F32					m_fVehicleWidth;
	F32					m_fVehicleHeight;
	F32					m_fDotSize;
	F32					m_fAlphaFadeSize;
	XtSz8				m_xtColourYou;
	XtSz8				m_xtColourOther;
	XtSz8				m_xtColourMine;
	F32					m_fScaleMin;
	XtSz8				m_xtColourName;
	F32					m_fNameTextHeight;
	F32					m_fNameGapMult;
	C_FontStylePasses	m_FontStylePasses;
	F32					m_fSpinTime;
	F32					m_fViewDistance;
	bool				m_bUseCameraNotCar;
	F32					m_fMapSize;
	F32					m_fMapScale;
	XtSz8				m_xtColourRadarFill;
	XtSz8				m_xtColourRadarOuter;
	F32					m_fTargetingSize;
	F32					m_fTargetingGapMult;
	XtSz8				m_xtColourTargeted;
	
private:
	S32					m_sLocalPlayerIndex;

	C_UISpriteColours	m_ColourYou;
	C_UISpriteColours	m_ColourOther;
	C_UISpriteColours	m_ColourName;
	C_UISpriteColours	m_ColourMine;

	C_UISpriteColours	m_ColourRadarFill;
	C_UISpriteColours	m_ColourRadarOuter;	

	C_UISpriteColours	m_ColourTargeted;	

	Btl::Vector< C_UIRaceLineEffectInfo >	m_EffectInfo;

	C_UIRaceLineSorter	m_Sorter;

	F32					m_fTrackLength;
	F32					m_fLogicCap01;

	Btl::Vector< bool, Btl::VectorParameters_Array< MAX_VEHICLES > >	m_VehicleInRange;
};

};	// namespace UI

#endif /* UIRACELINE_H */
