#ifndef UIAIHUD_H
#define UIAIHUD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Game/Gadgets/UIStaticText.h>

#include "ui\UIGameDefines.h"
#include "GameLogic\RaceLogicCore\Data\RLDefinitions.h"

namespace UI
{

enum E_AiHud
{
	E_AiHud_Parent = 0,
	E_AiHud_Name,
	E_AiHud_Position,
	E_AiHud_Perks,
	E_AiHud_Damage,
	E_AiHud_Targeting,
	E_AiHud_TargetingLeft,
	E_AiHud_TargetingRight,
	E_AiHud_WinnerIcon,
	E_AiHud_RivalIcon,
	E_AiHud_RivalArrow,
	E_AiHud_WinnerPos,
	E_AiHud_TeamIcon,

	E_AiHud_Count,
};

// ------------
// C_AiSortInfo
// ------------
class C_AiSortInfo
{
public:
	C_AiSortInfo()
		: m_fDistanceSq( 0.0f )
		, m_sVehicleIndex( -1 )
	{
		//
	}

public:
	bool operator < ( const C_AiSortInfo& o ) const;

public:
	F32		m_fDistanceSq;
	S32		m_sVehicleIndex;
};

// -------
// UIAiHud
// -------
class UIAiHud : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_AiHud, "AI_HUD" )

public:
	UIAiHud() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:
	void	Init();
	void	Setup( lua_State* pState );
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	Render();

	void	Resolve( C_GadgetBase* pParentClone );

#if PLATFORM_PC
	// [Blur2PC] render interpolation: moves every sprite built from a car's update pose (AI badges, the
	// countdown's UICarNode; see C_Simple2D::Blur2PCSetAnchor) to the car's render pose; called per
	// rendered frame from C_UiManager::PreRender
	static void	Blur2PCPlaceCarSprites();
#endif

private:
	bool	IsBossCar( const S32 sVehicleIndex );

public: // exposed for Xt
	F32		m_fFadeNearStart;
	F32		m_fFadeNearEnd;
	F32		m_fFadeStart;
	F32		m_fFadeEnd;
	bool	m_bDisable;
	S32		m_sMaxDrawCount;
	F32		m_fScaleStart;
	F32		m_fScaleMultAddPerMetre;
	F32		m_fScaleMax;
	
private:
	bool	m_bShowNames;
	bool	m_bShowPerks;
	S32		m_DoSort;
	S32		m_sLocalPlayerIndex;

	F32		m_fParentScaleStart;

	struct C_Vehicle
	{
		C_Vehicle()
			: m_TeamColourSet( false )
		{}

		bool					m_TeamColourSet;
		U8						m_Alpha;
		UIStaticText::C_Cache	m_TextCache;
	};

	typedef Btl::Vector< C_GadgetBase*, Btl::VectorParameters_Array< E_AiHud_Count > >			AiGadgets;

	Btl::Vector< AiGadgets >												m_Objects;
	Btl::Vector< C_AiSortInfo >												m_SortedVehicles;
	Btl::Vector< C_Vehicle, Btl::VectorParameters_Array< MAX_VEHICLES > >	m_Vehicle;

	Btl::Vector< S32, Btl::VectorParameters_Array< MAX_VEHICLES > >			m_BossVehicles;
};

};	// namespace UI

#endif /* UIRIVALHUD_H */
