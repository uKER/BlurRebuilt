#include "SharedActorsPch.h"

#include <shared_actors/ActorCreation/SplineActorCreate.h>

// Gameplay Editor Create
XT_TYPE(Actors::C_SplineActorCreate, "SplineActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_SplineData, "SplineData")
	XT_FIELD(m_Attacher, "Attacher")	
XT_END_TYPE
