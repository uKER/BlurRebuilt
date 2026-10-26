#ifndef MESSAGESUBSCRIBERLIST_E38AB002_5981_4715_A028_CC05073F125F_INL
#define MESSAGESUBSCRIBERLIST_E38AB002_5981_4715_A028_CC05073F125F_INL

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/messaging/MessageSubscriber.h>
#include <misc_shared/messaging/MessageSubscriberList.h>


namespace Messaging
{

/**
@brief Tools for manipulating a double-ended list.

Why am I writing yet another linked-list when we've got Btl? I want to embed
multiple links inside the same object, so that it can exist in multiple lists.
If this code is buggy then Mark will never let me hear the end of it ;-)

T_LinkPolicy provides the following:
	static const C_ListLink<T>& Get(const T&);
	static C_ListLink<T>& Get(T&);

@param T type of object stored in the list.
@param T_LinkPolicy obtains links from objects.
*/
template <typename T, class T_LinkPolicy>
struct C_ListUtils
{
	typedef C_ListLink<T> C_Link;
	typedef C_ListContainer<T> C_List;

	/**
	@param list list to operate on.
	@returns true if the list is empty.
	*/
	static bool IsEmpty(const C_List& list)
	{
		return (list.m_First == NULL);
	}

	/**
	@param list list to operate on.
	@returns the number of objects in the list.
	*/
	static U32 CalcCount(const C_List& list)
	{
		U32 count = 0;

		for (const T* pData = list.m_First; pData != NULL; pData = T_LinkPolicy::Get(*pData).m_Next)
		{
			++count;
		}

		return count;
	}

	/**
	@brief Wipes the list.
	@param list list to operate on.
	*/
	static void Clear(C_List& list)
	{
		list.m_First = NULL;
		list.m_Last = NULL;
	}

	/**
	@brief Adds an object to the end of a lis.
	@param list list to operate on.
	@param data object to add.
	*/
	static void PushBack(C_List& list, T& data)
	{
		C_Link& link = T_LinkPolicy::Get(data);

		link.m_Prev = list.m_Last;
		link.m_Next = NULL;
		
		T*& pPrevNext = (list.m_Last == NULL) ? list.m_First : T_LinkPolicy::Get(*list.m_Last).m_Next;
		pPrevNext = &data;

		list.m_Last = &data;
	}

	/**
	@brief Removes an object from a list.
	@param list list to operate on.
	@param data object in the list.
	*/
	static void Remove(C_List& list, T& data)
	{
		C_Link& link = T_LinkPolicy::Get(data);

		T*& pPrevNext = (link.m_Prev == NULL) ? list.m_First : T_LinkPolicy::Get(*link.m_Prev).m_Next;
		BIZ_ASSERT(pPrevNext == &data);
		pPrevNext = link.m_Next;

		T*& pNextPrev = (link.m_Next == NULL) ? list.m_Last : T_LinkPolicy::Get(*link.m_Next).m_Prev;
		BIZ_ASSERT(pNextPrev == &data);
		pNextPrev = link.m_Prev;
	}
};


struct C_HandlerLinkPolicy
{
	inline static const C_ListLink<C_MessageSubscriber>& Get(const C_MessageSubscriber& data) {return data.m_MessageHandlerLink;}
	inline static C_ListLink<C_MessageSubscriber>& Get(C_MessageSubscriber& data) {return data.m_MessageHandlerLink;}
};

typedef C_ListUtils<C_MessageSubscriber, C_HandlerLinkPolicy> C_HandlerListUtils;


struct C_SystemLinkPolicy
{
	inline static const C_ListLink<C_MessageSubscriber>& Get(const C_MessageSubscriber& data) {return data.m_SubscriberSystemLink;}
	inline static C_ListLink<C_MessageSubscriber>& Get(C_MessageSubscriber& data) {return data.m_SubscriberSystemLink;}
};

typedef C_ListUtils<C_MessageSubscriber, C_SystemLinkPolicy> C_SystemListUtils;

} // namespace Messaging


#endif // MESSAGESUBSCRIBERLIST_E38AB002_5981_4715_A028_CC05073F125F_INL
