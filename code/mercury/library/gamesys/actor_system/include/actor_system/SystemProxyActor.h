#ifndef SYSTEMPROXYACTOR_A67E68DA_9E69_4F0C_8B1B_73629E5C894E_H
#define SYSTEMPROXYACTOR_A67E68DA_9E69_4F0C_8B1B_73629E5C894E_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/Actor.h>
#include <actor_system/ActorDeclare.h>
#include <actor_system/ActorHandleListItem.h>
#include <actor_system/SystemProxyGroup.h>


namespace Actor
{

/// Default lifetime policy for proxy actors.
template <class T_System>
struct C_DefaultSystemProxyLifetime
{
	static void				OnFirstProxyConstructed(const C_ActorList* pActorList, T_System* pSystem) {}
	static void				OnLastProxyDestroyed(const C_ActorList* pActorList, T_System* pSystem) {C_SystemProxyGroupBase::Purge();}
};

/// Default serialisation policy for proxy actors.
template <class T_System>
struct C_DefaultSystemProxySerialisation
{
	static void				SerialiseForSave(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, const T_System* pSystem) {}
	static void				DeserialiseForSave(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, T_System* pSystem) {}

	static void				SerialiseForSync(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, const T_System* pSystem) {}
	static void				DeserialiseForSync(Sys::C_ContextualStream& stream, const C_ActorList* pActorList, T_System* pSystem) {}
};

/// Base class for proxy actors.
template <class T_System, class T_SerialisationPolicy = C_DefaultSystemProxySerialisation<T_System>, class T_LifetimePolicy = C_DefaultSystemProxyLifetime<T_System> >
class C_SystemProxyActor : public C_Actor
{
public:
	C_SystemProxyActor();
	virtual ~C_SystemProxyActor();

	T_System*				GetSystem() const; ///< @returns system that we're a proxy for.

	virtual void			SerialiseForSave(Sys::C_ContextualStream& stream) const; // from C_Actor
	virtual void			DeserialiseForSave(Sys::C_ContextualStream& stream); // from C_Actor

	virtual void			SerialiseForSync(Sys::C_ContextualStream& stream) const; // from C_Actor
	virtual void			DeserialiseForSync(Sys::C_ContextualStream& stream); // from C_Actor

protected:
	virtual bool			InitialiseImpl(); // from C_Actor
	virtual void			KillImpl(); // from C_Actor

private:
	typedef C_SystemProxyGroup<T_System> C_ProxyGroup;
	typedef typename C_ProxyGroup::C_Ptr C_ProxyGroupPtr;

	C_ProxyGroupPtr			m_pProxyGroup; ///< the group we belong to
	C_ActorHandleListItem	m_ProxyGroupListItem; ///< our entry in m_pProxyGroup

	virtual T_System*		AcquireSystem() const = 0; ///< @returns system that we will be a proxy for.

	void					AcquireProxyGroup(); ///< binds us to the correct group

};

} // namespace Actor


#include <actor_system/SystemProxyActor.inl>

#endif // SYSTEMPROXYACTOR_A67E68DA_9E69_4F0C_8B1B_73629E5C894E_H
