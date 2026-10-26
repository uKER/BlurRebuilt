#include "SharedActorsPch.h"

#include <shared_actors/Components/Creation/AnimationControllerCreate.h>

namespace Components
{

U32	C_AnimationControllerCreator::GetNumSlaves() const
{
	return m_AnimationClip ? m_AnimationClip->m_AnimationObjects.count : 0;
}

Actor::C_ActorHandle C_AnimationControllerCreator::GetSlaveActorHandle(U32 slaveIndex) const
{
	if(m_AnimationClip)
	{
		if (Actors::C_GameActorCreate* a = m_AnimationClip->m_AnimationObjects[slaveIndex].m_Actor)
			return a->GetActorHandle();
	}

	return Actor::C_ActorHandle();
}

U32	C_AnimationControllerDynamicCreator::GetNumSlaves() const
{
	return 0;	// TODO: Implement
}

Actor::C_ActorHandle C_AnimationControllerDynamicCreator::GetSlaveActorHandle(U32 slaveIndex) const
{
	// TODO: Implement
	return Actor::C_ActorHandle();
}

}

XT_TYPE(Components::C_AnimationControllerCreator, "AnimationControllerComponent")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_AnimationClip, "AnimationClip")
XT_END_TYPE
