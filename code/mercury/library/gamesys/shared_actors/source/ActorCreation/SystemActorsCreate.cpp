#include "SharedActorsPch.h"

#include <shared_actors/ActorCreation/SystemActorsCreate.h>

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

XT_TYPE(Actors::C_NullActorCreate, "NullActor")
XT_BASE(Actors::C_PosActorCreate)
XT_END_TYPE

XT_TYPE(Actors::C_ProxyActorCreate, "ProxyActor")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_InitialActor, "InitialActor")
XT_END_TYPE

XT_TYPE(Actors::C_NodeActorCreate, "NodeActor")
XT_BASE(Actors::C_PosActorCreate)
XT_FIELD(m_pSorceryMovementComponentCreate, "MovementComponent")
XT_END_TYPE

namespace Actors
{
void C_NodeActorCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_PosActorCreate::DoSorceryCallback(pCallbackFn);

	if (m_pSorceryMovementComponentCreate != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pSorceryMovementComponentCreate);
}

}

