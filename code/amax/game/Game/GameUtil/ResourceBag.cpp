#include "stdafx.h"

#include "ResourceBag.h"
#include "ResourceBagListener.h"

C_ResourceBag::C_ResourceBag()
	: m_Resources()
	, m_Listeners()
	, m_ListenerLockCount(0)
{
}

C_ResourceBag::~C_ResourceBag()
{
	BIZ_ASSERT(m_ListenerLockCount == 0); // this could signify badness if our listener is still expecting us to tell it something

	while (!m_Listeners.Empty())
	{
		RemoveListener(m_Listeners.Front());
	}
}

C_ResourceBagHandle C_ResourceBag::Create()
{
	return C_ResourceBagHandle(new C_ResourceBag);
}

void C_ResourceBag::AddListener(C_ResourceBagListener* pListener)
{
	BIZ_ASSERT(pListener != NULL);

#ifndef CFG_SPEED
	for (C_Listeners::ConstIterator it = m_Listeners.Begin(); it != m_Listeners.End(); ++it)
	{
		BIZ_ASSERT(*it != pListener);
	}

	for (C_ResourceBagListener::C_Listenees::ConstIterator it = pListener->m_Listenees.Begin(); it != pListener->m_Listenees.End(); ++it)
	{
		BIZ_ASSERT(*it != this);
	}
#endif

	m_Listeners.PushBack(pListener);
	pListener->m_Listenees.PushBack(this);
}

void C_ResourceBag::RemoveListener(C_ResourceBagListener* pListener)
{
	BIZ_ASSERT(pListener != NULL);

	for (C_ResourceBagListener::C_Listenees::Iterator it = pListener->m_Listenees.Begin(); it != pListener->m_Listenees.End(); ++it)
	{
		if (*it == this)
		{
			pListener->m_Listenees.Erase(it);
			break;
		}
	}

	for (C_Listeners::Iterator it = m_Listeners.Begin(); it != m_Listeners.End(); ++it)
	{
		if (*it == pListener)
		{
			m_Listeners.Erase(it);
			break;
		}
	}
}

void C_ResourceBag::LockListeners()
{
	++m_ListenerLockCount;
}

void C_ResourceBag::UnlockListeners()
{
	BIZ_ASSERT(m_ListenerLockCount > 0);
	--m_ListenerLockCount;
	NotifyListenersIfComplete();
}

void C_ResourceBag::AddResource(const ResourceUtils::C_ResourceBagTag tag, const Sys::C_FilePath& fileName, const Resource::C_ResourceTypeId typeId)
{
	if (m_Resources.Find(tag) == m_Resources.End())
	{
		C_Item& item = m_Resources[tag];
		item.first = (Resource::E_Notify)-1;
		item.second.Load(fileName, typeId, this, (void*)tag);
	}
}

bool C_ResourceBag::IsWaitingForLoad() const
{
	for (C_Resources::ConstIterator it = m_Resources.Begin(); it != m_Resources.End(); ++it)
	{
		if (it->GetValue().first == (Resource::E_Notify)-1)
		{
			return true;
		}
	}

	return false;
}

#ifndef CFG_GOLD
void C_ResourceBag::DebugOutputBagContents(void) const
{
	LOGINFO(Debug::GetRootLogger(), "C_ResourceBag::DebugOutputBagContents:");
	for (C_Resources::ConstIterator it = m_Resources.Begin(); it != m_Resources.End(); ++it)
	{
		Resource::C_ResourceBase* pResource = it->GetValue().second;
		LOGINFO(Debug::GetRootLogger(), "%s %d", pResource->GetResourceFilename().Chars(), it->GetValue().first);
	}
}
#endif

void C_ResourceBag::ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
{
	C_Resources::Iterator it = m_Resources.Find((ResourceUtils::C_ResourceBagTag)(INTPTR)pData);	// [Blur2PC][x64] tag carried in the pointer

	if (it != m_Resources.End())
	{
		it->GetValue().first = result;
	}

	NotifyListenersIfComplete();
}

Resource::C_ResourceBase* C_ResourceBag::GetResourceBase(const ResourceUtils::C_ResourceBagTag tag, const Resource::C_ResourceTypeId typeId) const
{
	C_Resources::ConstIterator it = m_Resources.Find(tag);

	if (it != m_Resources.End())
	{
		if (it->GetValue().first == Resource::E_Notify_DataReady)
		{
			return it->GetValue().second;
		}
	}

	return false;
}


void C_ResourceBag::NotifyListenersIfComplete() const
{
	if ((m_ListenerLockCount == 0) && !IsWaitingForLoad())
	{
		for (C_Listeners::ConstIterator it = m_Listeners.Begin(); it != m_Listeners.End(); ++it)
		{
			(*it)->OnResourceBagLoaded(*this);
		}
	}
}

#ifndef CFG_GOLD
void C_ResourceBag::ReloadResource( const ResourceUtils::C_ResourceBagTag tag )
{
	C_Resources::Iterator it = m_Resources.Find(tag);

	if (it != m_Resources.End())
	{
		Resource::C_ResourceBase* pResourceBase = (Resource::C_ResourceBase*)it->GetValue().second;
		pResourceBase->SetDataOld(true);

		//Mantain old resource
		m_OldResources.PushBack(it->GetValue().second);

		//Add new resource
		it->GetValue().first = Resource::E_Notify_NoData;
		it->GetValue().second.Load(pResourceBase->GetResourceFilename(), pResourceBase->GetResourceTypeId(), this, (void*)tag);

		
		//Wait
		if (WaitForResourceData((Resource::C_ResourceBase*)it->GetValue().second))
		{
			it->GetValue().first = Resource::E_Notify_DataReady;
			LOGINFO(Debug::GetRootLogger(), "%s resource reloaded", pResourceBase->GetResourceFilename().Chars());
		}
		else
		{
			it->GetValue().first = Resource::E_Notify_DataError;
			LOGWARN(Debug::GetRootLogger(), "error reloading %s resource", pResourceBase->GetResourceFilename().Chars());
		}
	}
	else
	{
		LOGWARN(Debug::GetRootLogger(), "resource is not in the resource bag");
	}
}
#endif