#include <misc_shared/context/ContextualStream.h>


namespace Actor
{

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::C_SystemProxyActor()
	: C_Actor()
	, m_pProxyGroup()
	, m_ProxyGroupListItem()
{
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::~C_SystemProxyActor()
{
	BIZ_ASSERT(!m_pProxyGroup);
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
void C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::AcquireProxyGroup()
{
	BIZ_ASSERT(!m_pProxyGroup);

	C_ActorList* pActorList = GetActorList();
	T_System* pSystem = AcquireSystem();

	if ((pActorList != NULL) && (pSystem != NULL))
	{
		m_pProxyGroup = C_ProxyGroup::Acquire(pActorList, pSystem);

		if (m_pProxyGroup->GetNumProxies() == 0)
		{
			T_LifetimePolicy::OnFirstProxyConstructed(pActorList, pSystem);
		}

		m_ProxyGroupListItem.SetHandle(GetHandle());
		m_pProxyGroup->AddProxy(m_ProxyGroupListItem);
	}
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
bool C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::InitialiseImpl()
{
	BIZ_ASSERT(!m_pProxyGroup);

	AcquireProxyGroup();

	return m_pProxyGroup;
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
void C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::KillImpl()
{
	if (m_pProxyGroup)
	{
		m_pProxyGroup->RemoveProxy(m_ProxyGroupListItem);

		if (m_pProxyGroup->GetNumProxies() == 0)
		{
			T_LifetimePolicy::OnLastProxyDestroyed(m_pProxyGroup->GetActorList(), m_pProxyGroup->GetSystem());
		}

		m_pProxyGroup = NULL;
	}
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
T_System* C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::GetSystem() const
{
	return m_pProxyGroup ? m_pProxyGroup->GetSystem() : NULL;
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
void C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	BIZ_ASSERT(m_pProxyGroup);

	if (m_pProxyGroup->IsFirstProxy(m_ProxyGroupListItem))
	{
		stream.WriteAs<bool>(true);
		T_SerialisationPolicy::SerialiseForSave(stream, m_pProxyGroup->GetActorList(), m_pProxyGroup->GetSystem());
	}
	else
	{
		stream.WriteAs<bool>(false);
	}
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
void C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	AcquireProxyGroup();

	if (stream.ReadAs<bool>())
	{
		T_SerialisationPolicy::DeserialiseForSave(stream, m_pProxyGroup->GetActorList(), m_pProxyGroup->GetSystem());
	}
}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
void C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	BIZ_ASSERT(m_pProxyGroup);

	if (m_pProxyGroup->IsFirstProxy(m_ProxyGroupListItem))
	{
		stream.WriteAs<bool>(true);
		T_SerialisationPolicy::SerialiseForSync(stream, m_pProxyGroup->GetActorList(), m_pProxyGroup->GetSystem());
	}
	else
	{
		stream.WriteAs<bool>(false);
	}

}

template <class T_System, class T_SerialisationPolicy, class T_LifetimePolicy>
void C_SystemProxyActor<T_System, T_SerialisationPolicy, T_LifetimePolicy>::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	BIZ_ASSERT(m_pProxyGroup);

	if (stream.ReadAs<bool>())
	{
		T_SerialisationPolicy::DeserialiseForSync(stream, m_pProxyGroup->GetActorList(), m_pProxyGroup->GetSystem());
	}
}

} // namespace Actor
