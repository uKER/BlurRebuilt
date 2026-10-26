#ifndef PICKUPPOSITIONACTORDECOY_H
#define PICKUPPOSITIONACTORDECOY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic\ActorStudioActors\PickupPositionActor.h>

namespace Actors
{

class PickupPositionDecoyCreate : public PickupPositionCreate
{
public:
	static Actor::C_ActorTypeId					ClassType()				{return Game::E_ActorTypeId_PickupPositionDecoy; }
	const Actor::C_ActorTypeId					GetType() const			{return Game::E_ActorTypeId_PickupPositionDecoy;}
};

class PickupPositionActorDecoy : public PickupPositionActor
{
public:
	static const Actor::C_ActorTypeId			TypeId()				{ return Game::E_ActorTypeId_PickupPositionDecoy; }
	static const Actor::C_ActorTypeName			TypeName()				{ return "PickupPositionDecoy"; }

protected:
	void										Update(const float frameTime);
	bool										CheckPickup(VehiclePickupPosition& vehiclePosition);
	void										GrantPickup(VehiclePickupPosition& vehiclePosition);
	bool										SetRandomPickupType(bool checkAgainstNext = false, bool forceChange = false);
	void										SetupVisualPickupIcon();
};

};

#endif //PICKUPPOSITIONACTORDECOY_H
