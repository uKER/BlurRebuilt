#ifndef ADDITIONALCOMPONENTS_H
#define ADDITIONALCOMPONENTS_H

#include <misc_shared/Messaging/MessageBase.h>

#include <shared_actors/XtActorCreation.h>
#include <shared_actors/XtSharedLevelActorParams.h>

#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Impl/MovementComponents.h>

namespace Actors
{
} // namespace Actors

namespace Components
{

enum E_AdditionalComponent
{
	E_AdditionalComponent_HiBits = FOURCC_HI20('AMAC'),		// Amax Actor Components

	// Movement
};

} // namespace Components

#endif		// ADDITIONALCOMPONENTS_H
