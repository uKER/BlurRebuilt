#include "SharedActorsPch.h"

#include <shared_actors/actorcreation/GroupActorsCreate.h>

#include <actor_system/ActorSerialisation.h>
#include <misc_shared/context/ContextualStream.h>
#include <system/serialise/btl.h>


XT_TYPE(Actors::C_GroupActorCreateXt, "GroupActor")
	XT_BASE(Actors::C_GameActorCreate)
	XT_FIELD(m_Members, "InitialMembers")
XT_END_TYPE


namespace Actors
{

U32 C_GroupActorCreateXt::GetNumMembers() const
{
	return m_Members.count;
}

Actor::C_ActorHandle C_GroupActorCreateXt::GetMember(const U32 index) const
{
	if (index < (U32)m_Members.count)
	{
		const C_GameActorCreate* pCreationData = m_Members.elements[index];
		
		if (pCreationData != NULL)
		{
			return pCreationData->GetActorHandle();
		}
	}

	return Actor::C_ActorHandle();
}


U32 C_GroupActorCreateRuntime::GetNumMembers() const
{
	return m_Members.Size();
}

Actor::C_ActorHandle C_GroupActorCreateRuntime::GetMember(const U32 index) const
{
	if (index < (U32)m_Members.Size())
	{
		return m_Members[index];
	}

	return Actor::C_ActorHandle();
}

void C_GroupActorCreateRuntime::Serialise(Sys::C_ContextualStream& stream) const
{
	C_GroupActorCreate::Serialise(stream);
	stream << m_Members;
}

void C_GroupActorCreateRuntime::Deserialise(Sys::C_ContextualStream& stream)
{
	C_GroupActorCreate::Deserialise(stream);
	stream >> m_Members;
}

} // namespace Actors
