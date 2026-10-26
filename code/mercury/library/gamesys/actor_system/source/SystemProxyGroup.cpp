#include "ActorSystemPch.h"
#include <actor_system/SystemProxyGroup.h>

#include <actor_system/ActorList.h>


namespace Actor
{

C_SystemProxyGroupBase::C_WeakList C_SystemProxyGroupBase::s_WeakList;

/**
@param pActorList actor list that owns the proxies in this group.
@param pSystem system to provide proxies for.
*/
C_SystemProxyGroupBase::C_SystemProxyGroupBase(C_ActorList* pActorList, void* pSystem)
	: Sys::RefObj()
	, m_pActorList(pActorList)
	, m_pSystem(pSystem)
	, m_Proxies()
{
	BIZ_ASSERT(pActorList != NULL);
	BIZ_ASSERT(pSystem != NULL);
}

C_SystemProxyGroupBase::~C_SystemProxyGroupBase()
{
}

U32 C_SystemProxyGroupBase::GetNumProxies() const
{
	return m_Proxies.Size();
}

/**
@param pMessage message to send to proxy actors.
*/
void C_SystemProxyGroupBase::SendMessageToProxies(const Messaging::C_MessageBase* pMessage) const
{
	for (C_ActorHandleList::ConstIterator it = m_Proxies.Begin(); it != m_Proxies.End(); ++it)
	{
		m_pActorList->SendMessageToActor(it->GetHandle(), pMessage);
	}
}

/**
@param msgType message to send to proxy actors.
*/
void C_SystemProxyGroupBase::SendMessageToProxies(const Messaging::TMsgType msgType) const
{
	for (C_ActorHandleList::ConstIterator it = m_Proxies.Begin(); it != m_Proxies.End(); ++it)
	{
		m_pActorList->SendMessageToActor(it->GetHandle(), msgType);
	}
}

/**
@param listItem externally owned link to proxy actor.
*/
void C_SystemProxyGroupBase::AddProxy(C_ActorHandleListItem& listItem)
{
	m_Proxies.PushBack(listItem);
}

/**
@param listItem externally owned link to proxy actor.
*/
void C_SystemProxyGroupBase::RemoveProxy(C_ActorHandleListItem& listItem)
{
	m_Proxies.Erase(C_ActorHandleList::Iterator(&listItem));
}

/**
@param listItem externally owned link to proxy actor.
@returns true if this is the first proxy in the group.
*/
bool C_SystemProxyGroupBase::IsFirstProxy(const C_ActorHandleListItem& listItem) const
{
	return C_ActorHandleList::ConstIterator(&listItem) == m_Proxies.Begin();
}

void C_SystemProxyGroupBase::Purge()
{
	C_WeakList::Iterator it = s_WeakList.Begin();

	while (it != s_WeakList.End())
	{
		C_WeakPtr pProxyGroup = *it;

		if (!pProxyGroup)
		{
			it = s_WeakList.Erase(it);
		}
		else
		{
			++it;
		}
	}
}

/**
@param pActorList actor list owning proxies in this group.
@param pSystem system that the group provides proxies to.
@returns NULL if this group does not exist.
*/
C_SystemProxyGroupBase* C_SystemProxyGroupBase::AcquireBase(C_ActorList* pActorList, void* pSystem)
{
	for (C_WeakList::Iterator it = s_WeakList.Begin(); it != s_WeakList.End(); ++it)
	{
		C_WeakPtr pProxyGroup = *it;

		if (pProxyGroup && (pProxyGroup->GetActorList() == pActorList) && (pProxyGroup->GetSystemBase() == pSystem))
		{
			return pProxyGroup.RawPtr();
		}
	}

	return NULL;
}

/**
@param pProxyGroup new group to track references for.
*/
void C_SystemProxyGroupBase::AcquireNew(C_SystemProxyGroupBase* pProxyGroup)
{
	BIZ_ASSERT(pProxyGroup != NULL);
	BIZ_ASSERT(AcquireBase(pProxyGroup->GetActorList(), pProxyGroup->GetSystemBase()) == NULL);

	s_WeakList.PushBack(pProxyGroup);
}

} // namespace Actor
