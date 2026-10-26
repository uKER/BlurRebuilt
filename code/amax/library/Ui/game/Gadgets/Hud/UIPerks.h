#ifndef UIHUDPERKS_H
#define UIHUDPERKS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>
#include <ui/Graphics/Font/FontStyle.h>

#include "GameLogic/RaceLogicCore/Data/RLEnums.h"
#include "GameLogic/Structures/VehiclePickupInfo.h"

namespace UI
{
// ------------
// C_UIPerkInfo
// ------------
class C_UIPerkInfo
{
public:
	C_UIPerkInfo() : m_sTextureIndex( 0 )	{  }

public:
	void	Resolve();

public:	// exposed for Xt
	S32			m_sTextureIndex;
	XtSz8		m_xtColourFill;
	XtSz8		m_xtColourGlow;
	XtSz8		m_xtColourText;
	XtLocId		m_LocIdName;
	XtLocId		m_PastLocIdName;

	C_UISpriteColours	m_ColourFill;
	C_UISpriteColours	m_ColourGlow;
	C_UISpriteColours	m_ColourText;
};

struct C_PerkDisplayParameters
{
	C_UIPerkInfo		m_PerkInfoNitrous;
	C_UIPerkInfo		m_PerkInfoShock;
	C_UIPerkInfo		m_PerkInfoShunt;
	C_UIPerkInfo		m_PerkInfoBarge;
	C_UIPerkInfo		m_PerkInfoBomb;
	C_UIPerkInfo		m_PerkInfoBlindfire3;
	C_UIPerkInfo		m_PerkInfoBlindfire2;
	C_UIPerkInfo		m_PerkInfoBlindfire1;
	C_UIPerkInfo		m_PerkInfoRepair;
	C_UIPerkInfo		m_PerkInfoShield;
	C_UIPerkInfo		m_PerkInfoSuperShield;
	C_UIPerkInfo		m_PerkInfoEmpty;
	C_UIPerkInfo		m_PerkInfoExtraBlindfire4;
	C_UIPerkInfo		m_PerkInfoExtraBlindfire3;
	C_UIPerkInfo		m_PerkInfoExtraBlindfire2;
	C_UIPerkInfo		m_PerkInfoExtraBlindfire1;

	C_UIPerkInfo*		GetPerkInfo( const E_PICKUP_TYPE ePickupType, const S32 sAmount = 0, const bool bHasExtraBoltMod = false );
};

// -------
// UIPerks
// -------
class UIPerks : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Perks, "PERKS" )

public:
	UIPerks() : C_GadgetBase(), m_sLocalPlayerIndex( -1 ) {  }

public:
	void				Init();
	void				Setup( lua_State* pState );
	bool				MessageHandler( const Messaging::C_MessageBase* pMessage );
	void				Render();

	void				SetVehicleIndexOverride( const S32 sVehicleIndexOverride )		{ m_sVehicleIndexOverride = sVehicleIndexOverride; }

private:
	S32					m_sLocalPlayerIndex;
	S32					m_sVehicleIndexOverride;

	C_RenderState		m_RenderStatePickupNone;
};

};	// namespace UI

#endif /* UIHUDPERKS_H */
