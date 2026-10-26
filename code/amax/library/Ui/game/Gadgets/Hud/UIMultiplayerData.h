#ifndef UIHUDMPDATA_H
#define UIHUDMPDATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Graphics/Font/FontStyle.h>
#include <GameLogic\RaceModes\RaceModeDefines.h>
#include <GameLogic\RaceLogicCore\Data\RLDefinitions.h>

#include "Ui/Game/Multiplayer/MultiplayerDefines.h"

namespace UI
{

// --------------
// C_UIMpSortInfo
// --------------
class C_UIMpSortInfo
{
public:
	C_UIMpSortInfo()	{ Clear(); }

public:
	bool	operator < ( const C_UIMpSortInfo& o ) const;

	void	Clear();

public:
	S32		m_sVehicleIndex;
	S32		m_sSort;
	S32		m_sData;
	bool	m_bActive;
};

// -----------------
// UIMultiplayerData
// -----------------
class UIMultiplayerData : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_MultiplayerData, "MP_DATA" )

public:
	UIMultiplayerData() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:
	void				Init();
	void				Setup( lua_State* pState );
	void				PostInit();
	void				FrameUpdate();
	bool				MessageHandler( const Messaging::C_MessageBase* pMessage );
	void				Render();

	void				ToggleScoreboard();

private:
	void				RenderFullscreen();

	void				RenderMini();

public: // exposed for Xt
	C_FontStylePasses	m_FontStylePasses;
	C_FontStylePasses	m_DataFontStylePasses;
	C_FontStylePasses	m_TitleFontStylePasses;

	F32					m_fIconSize;
	F32					m_fTeamSize;
	XtSz8				m_xtColourFirst;
	XtSz8				m_xtColourOther;
	XtSz8				m_xtColourLast;
	XtSz8				m_xtModeTitleColour;
	XtSz8				m_xtTimerTitleColour;

	F32					m_fGapX;
	F32					m_fTitleSize;
	F32					m_fTitleOffset;
	bool				m_bFullscreen;
	bool				m_bOnlyShowFinishedCars;

	bool				m_bShowFullscreenTitle;
	bool				m_bShowFullscreenTarget;

private:
	S32					m_sLocalPlayerIndex;
	Game::E_RaceMode	m_eRaceMode;

	E_EliminationEvent	m_eEliminationEvent;
	E_EliminationType	m_eEliminationType;

	S32					m_sPositionMe;
	S32					m_sLocalPlayerPreviousScore;

	C_UISpriteColours	m_ColourFirst;
	C_UISpriteColours	m_ColourOther;
	C_UISpriteColours	m_ColourLast;
	C_UISpriteColours	m_ModeTitleColour;
	C_UISpriteColours	m_TimerTitleColour;

	C_WideString32		m_RaceTypeName;

	Btl::Vector< C_UIMpSortInfo, Btl::VectorParameters_Static< MAX_VEHICLES > >		m_SortInfo;
	Btl::Vector< S32, Btl::VectorParameters_Static< MAX_VEHICLES > >				m_RenderIndices;

	F32					m_fPosLength;
	F32					m_fPointsLength;
	F32					m_fRankLength;
	F32					m_fNameLength;
	F32					m_fTotalLength;

	bool				m_bFullscreenDoRender;
	bool				m_bShowDataColumn;

	bool				m_bSystemLink;

	S32					m_sMaxRank;

	C_RenderState		m_RenderStateAbility;

	typedef	Btl::Pair< E_MultiplayerVehicleAbilities, E_MultiplayerAbilityCategory >	AbilityPair;

	Btl::Vector< 
		Btl::Vector< AbilityPair, 
			Btl::VectorParameters_Static< E_MULTIPLAYER_LOADOUT_SLOT_MAX > >, 
		Btl::VectorParameters_Array< MAX_VEHICLES > >								m_VehicleLoadouts;

	Btl::Vector< C_UISpriteColours, 
		Btl::VectorParameters_Array< E_MultiplayerAbilityCategory_Max > >			m_AbilityCategoryColours;
	Btl::Vector< C_UISpriteColours, 
		Btl::VectorParameters_Array< E_MultiplayerAbilityCategory_Max > >			m_AbilityCategoryColoursCurrent;

	C_GadgetBase*		m_pBackground;
};

};	// namespace UI

#endif /* UIHUDMPDATA_H */
