
#ifndef UI_CARNODE_H
#define UI_CARNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui\UIGameDefines.h"
#include <ui/Game/Gadgets/Buttons.h>	// [Blur2PC] C_GadgetBase

#include <Game\GameActors\Vehicles\VehicleActor.h>

namespace UI
{

class UICarNode : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_CarNode, "UICarNode" )

	void			Init();
	bool			MessageHandler( const Messaging::C_MessageBase* pMessage );
	void			FrameUpdate();

private:
	S32									m_sLocalPlayerIndex;
	Game::Actors::C_VehicleLogicActor*	m_pLogicActor;
	Game::Actors::C_VehicleActor*		m_pVehicleActor;
};

}; // namespace UI

#endif // UI_CARNODE_H
