#ifndef UILOADOUTCONTOLLER_H
#define UILOADOUTCONTOLLER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/Gadgets/Buttons.h>
#include "ui/UIGameDefines.h"
#include "Ui/game/Multiplayer/MultiplayerDefines.h"

namespace UI
{

// --------
// UILoadoutContoller
// --------
class UILoadoutController : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_LoadoutController, "LoadoutController" )

public:
	void	Init();
	void	PostInit();
	bool	MessageHandler( const Messaging::C_MessageBase* pMessage );
	void	FrameUpdate();

private:
	void	SetupGadgets( S32 sPlayerVehicleIndex );

public:
	XtArray<XtSz8>			m_AbilityNames;
	F32						m_fDeactiveWaitTime;

private:
	S32						m_sVehicleIndex;

	Btl::Vector< bool, Btl::VectorParameters_Array< E_MultiplayerAbilityCategory_Max > >			m_AbilitySlotActive;
	Btl::Vector< F32,  Btl::VectorParameters_Array< E_MultiplayerAbilityCategory_Max > >			m_AbilitySlotDeactiveTimer;
	Btl::Vector< C_GadgetBase*, Btl::VectorParameters_Array< E_MultiplayerAbilityCategory_Max > >	m_pGadget;

	//for the laser sight mod dot on the mirror
	C_GadgetBase*			m_LaserSightDot;
};

}	// namespace UI

#endif /* UILOADOUTCONTOLLER_H */
