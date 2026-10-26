#ifndef GAME_COMPONENTS_H
#define GAME_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageBase.h>
#include <misc_shared/UID/FourCC.h>
#include <actor_system/ActorComponent.h>
#include <shared_actors/Components/SharedComponents.h>

class I_GameActorComponent : public Actor::I_ActorComponent
{
public:
// 	C_GameActorList *	GetActorList(void) const {return (C_GameActorList *)I_ActorComponent::GetActorList();}
// 	C_GameWorld *		GetGameWorld(void) const {return GetActorList()->GetGameWorld();}
	virtual bool		OnMessage(const Messaging::C_MessageBase *pMessage ) {return I_ActorComponent::OnMessage(pMessage);}	// component message handler

};

namespace Components
{

// enum for game component interfaces
enum E_GameComponentIF
{
	// 	E_IF_All = -1,
	// 	E_IF_None = 0,
	E_GameComponentIF_HiBits = FOURCC_HI20('MGCI'), // Mercury Game Component Interface

	// Now in Mercury:
	//E_IF_Render,

};

// enum for our game components
enum E_GameActorComponent
{
	E_GameActorComponent_HiBits = FOURCC_HI20('MGAC'), // Mercury Game Actor Component

	// Render
	// Now in Mercury:
	//E_SingleElementRenderModel,
	//E_MultiElementRenderModel,

	// Physics
	E_ModelPhysicsBody,

	E_MovementDualSpline
};

void RegisterGameComponents(void);

}

#endif
