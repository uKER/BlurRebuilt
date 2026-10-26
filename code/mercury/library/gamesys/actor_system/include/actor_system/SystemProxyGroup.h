#ifndef SYSTEMPROXYGROUP_120DE243_B21F_47C9_A420_F5E10BE89F67_H
#define SYSTEMPROXYGROUP_120DE243_B21F_47C9_A420_F5E10BE89F67_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <actor_system/ActorHandleListItem.h>
#include <btl/containers/list.h>
#include <core/utility/SmartPtr.h>
#include <misc_shared/messaging/MessagingDeclare.h>


namespace Actor
{

/// Reference counted base class for groups of proxy actors.
class C_SystemProxyGroupBase : public Sys::RefObj
{
public:
	C_ActorList*			GetActorList() const {return m_pActorList;} ///< @returns the actor list that the proxies live in.
	U32						GetNumProxies() const; ///< @returns the number of proxy actors in this group.

	void					SendMessageToProxies(const Messaging::C_MessageBase* pMessage) const; ///< Sends a message to all proxy actors in this group.
	void					SendMessageToProxies(const Messaging::TMsgType msgType) const; ///< Sends a message to all proxy actors in this group.

	void					AddProxy(C_ActorHandleListItem& listItem); ///< Adds a proxy actor to this group.
	void					RemoveProxy(C_ActorHandleListItem& listItem); ///< Removes a proxy actor from this group.

	bool					IsFirstProxy(const C_ActorHandleListItem& listItem) const; ///< Checks if this is the first proxy actor in the group.

	static void				Purge(); ///< Discards unreferenced proxy groups.

protected:
	C_SystemProxyGroupBase(C_ActorList* pActorList, void* pSystem);
	virtual ~C_SystemProxyGroupBase();

	void*					GetSystemBase() const {return m_pSystem;} ///< @returns the address of the system object.
	
	static C_SystemProxyGroupBase*	AcquireBase(C_ActorList* pActorList, void* pSystem); ///< Looks for an existing group.
	static void				AcquireNew(C_SystemProxyGroupBase* pProxyGroup); ///< References a new group.

private:
	typedef Sys::WeakPtr<C_SystemProxyGroupBase> C_WeakPtr;
	typedef Btl::List<C_WeakPtr> C_WeakList;

	C_ActorList*			m_pActorList; ///< actor list that owns the proxies in this group
	void*					m_pSystem; ///< system we're providing a proxy for
	C_ActorHandleList		m_Proxies; ///< proxy actors in this group

	static C_WeakList		s_WeakList; ///< list of weak references to all extant proxy groups

	C_SystemProxyGroupBase(const C_SystemProxyGroupBase& copy); // can not copy
	C_SystemProxyGroupBase& operator = (const C_SystemProxyGroupBase& copy); // can not copy

};

/// Group of proxies to the same system.
template <class T_System>
class C_SystemProxyGroup : public C_SystemProxyGroupBase
{
public:
	typedef Sys::RefPtr<C_SystemProxyGroup> C_Ptr;

	T_System*				GetSystem() const {return reinterpret_cast<T_System*>(GetSystemBase());} ///< @returns the system object.

	static C_Ptr			Acquire(C_ActorList* pActorList, T_System* pSystem); ///< Claims a reference to a group of proxies.

private:
	C_SystemProxyGroup(C_ActorList* pActorList, T_System* pSystem) : C_SystemProxyGroupBase(pActorList, pSystem) {}
	virtual ~C_SystemProxyGroup() {}

};

/**
Proxy groups are reference counted, so the return value should be stored in 
a smart pointer. When the group is no longer required that pointer should be
cleared. If the combination of actor list and system object is no longer 
required then C_SystemProxyGroupBase::Purge() should be called after clearing
the smart pointer.

Example:
@code
// in some class definition
C_SystemProxyGroup<C_MySystem>::C_Ptr m_pProxyGroup;

// in some initialisation code
m_pProxyGroup = C_SystemProxyGroup<C_MySystem>::Acquire(GetActorList(), GetMySystem());

// in some shutdown code
m_pProxyGroup = NULL;
C_SystemProxyGroupBase::Purge();
@endcode

@param pActorList actor list owning the proxies, must not be NULL.
@param pSystem system to provide proxies to, must not be NULL.
@returns smart pointer to proxy group.
*/
template <class T_System>
typename C_SystemProxyGroup<T_System>::C_Ptr C_SystemProxyGroup<T_System>::Acquire(C_ActorList* pActorList, T_System* pSystem)
{
	BIZ_ASSERT(pActorList != NULL);
	BIZ_ASSERT(pSystem != NULL);

	C_SystemProxyGroup* pProxyGroup = static_cast<C_SystemProxyGroup*>(AcquireBase(pActorList, pSystem));

	if (pProxyGroup == NULL)
	{
		pProxyGroup = new C_SystemProxyGroup(pActorList, pSystem);
		AcquireNew(pProxyGroup);
	}

	return C_Ptr(pProxyGroup);
}

} // namespace Actor


#endif // SYSTEMPROXYGROUP_120DE243_B21F_47C9_A420_F5E10BE89F67_H
