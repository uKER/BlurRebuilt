#include "SharedActorsPch.h"

#include <actor_system/ActorList.h>
#include <actor_system/ActorMessages.h>
#include <shared_actors/SharedActorUtils.h>


namespace SharedActorUtils
{

C_ActorLimitList::C_ActorLimitList(U32 maxEntries, Actor::C_ActorList *pActorList):
				m_pActorList(pActorList),m_MaxEntries(maxEntries),m_CurrentSlot(0) 
{
	m_ActorHandleList = new Actor::C_ActorHandle[m_MaxEntries];
}

C_ActorLimitList::~C_ActorLimitList()
{
	SAFE_DELETE_ARY(m_ActorHandleList);
}


bool	C_ActorLimitList::AddActor(Actor::C_ActorHandle newActor)
{
	BIZ_ASSERT(m_pActorList);

	// Kill Old Actor
	m_pActorList->SendMessageToActor(m_ActorHandleList[m_CurrentSlot], Actor::E_ActorMsg_Kill);

	m_ActorHandleList[m_CurrentSlot] = newActor;
	m_CurrentSlot++;
	if(m_CurrentSlot == m_MaxEntries)
		m_CurrentSlot = 0;
	return true;
}
	
void* HandleToUserData(const Actor::C_ActorHandle& actorHandle)
{
#if defined(_WIN64) // [Blur2PC][x64] The 32-bit handle travels in the low half of the pointer-sized user data.
	StaticAssert(sizeof(Actor::C_ActorHandle) == sizeof(U32), we_want_to_reinterpret_these_bits);
	return (void*)(INTPTR)*reinterpret_cast<const U32*>(&actorHandle);
#else
	StaticAssert(sizeof(Actor::C_ActorHandle) == sizeof(void*), we_want_to_reinterpret_these_bits);
	return *reinterpret_cast<void* const*>(&actorHandle);
#endif
}

Actor::C_ActorHandle HandleFromUserData(const void* pUserData)
{
#if defined(_WIN64) // [Blur2PC][x64] See HandleToUserData.
	StaticAssert(sizeof(Actor::C_ActorHandle) == sizeof(U32), we_want_to_reinterpret_these_bits);
	const U32 bits = (U32)(INTPTR)pUserData;
	return *reinterpret_cast<const Actor::C_ActorHandle*>(&bits);
#else
	StaticAssert(sizeof(Actor::C_ActorHandle) == sizeof(void*), we_want_to_reinterpret_these_bits);
	return *reinterpret_cast<const Actor::C_ActorHandle*>(&pUserData);
#endif
}


} // namespace SharedActorUtils
