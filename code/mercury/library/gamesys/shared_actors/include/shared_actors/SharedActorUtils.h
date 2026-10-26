#ifndef _SHAREDACTORUTILS_H
#define _SHAREDACTORUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <misc_shared/Context/ContextualStream.h>


namespace SharedActorUtils
{
class C_ActorLimitList
{
public:
	C_ActorLimitList(U32 maxEntries, Actor::C_ActorList *pActorList);
	~C_ActorLimitList();
	bool	AddActor(Actor::C_ActorHandle newActor);
private:
	Actor::C_ActorList *		m_pActorList;
	U32							m_MaxEntries;
	U32							m_CurrentSlot;
	Actor::C_ActorHandle *		m_ActorHandleList;
};

extern void* HandleToUserData(const Actor::C_ActorHandle& actorHandle);
extern Actor::C_ActorHandle HandleFromUserData(const void* pUserData);


template <class T>
inline void SerialiseVector(const Btl::Vector<T>& vec, Sys::C_ContextualStream& stream)
{
	const U32 count = vec.Size();
	stream << count;
	for (U32 i=0 ; i<count ; i++)
		stream << vec[i];
}

template <class T>
inline void DeserialiseVector(Btl::Vector<T>& vec, Sys::C_ContextualStream& stream)
{
	U32 count = 0;
	stream >> count;
	vec.Reserve(count);
	for (U32 i=0 ; i<count ; i++)
	{
		T item;
		stream >> item;
		vec.PushBack(item);
	}
}


} // namespace SharedActorUtils


#endif // _SHAREDACTORUTILS_H
