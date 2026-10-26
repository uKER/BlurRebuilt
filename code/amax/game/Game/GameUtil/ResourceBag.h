#ifndef GAME_RESOURCEBAG_H
#define GAME_RESOURCEBAG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/list.h>
#include <btl/containers/map.h>
#include <btl/containers/vector.h>
#include <resource/Resource.h>
#include <resource_utils/I_ResourceBag.h>

#include "game/GameUtil/ResourceBagDeclare.h"

class C_ResourceBag;

class C_ResourceBag : public Sys::RefObj, public ResourceUtils::I_ResourceBag, public Resource::C_Listener
{
public:
	static C_ResourceBagHandle Create(); ///< Create a new bag.
	
	void AddResource(const ResourceUtils::C_ResourceBagTag tag, const Sys::C_FilePath& fileName, const Resource::C_ResourceTypeId typeId) OVERRIDE;
	bool IsWaitingForLoad() const OVERRIDE;
	void ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData) OVERRIDE;

#ifndef CFG_GOLD
	void DebugOutputBagContents(void) const OVERRIDE;
#endif

	void AddListener(C_ResourceBagListener* pListener); ///< Add a listener.
	void RemoveListener(C_ResourceBagListener* pListener); ///< Remove a listener.

	void LockListeners(); ///< Blocks notifications to listeners.
	void UnlockListeners(); ///< Unblocks notifications to listeners.
#ifndef CFG_GOLD
	void ReloadResource(const ResourceUtils::C_ResourceBagTag tag);
#endif

private:
	typedef Btl::Pair<Resource::E_Notify, Resource::C_HandleBase> C_Item;
	typedef Btl::Map<ResourceUtils::C_ResourceBagTag, C_Item> C_Resources; ///< all resources in the bag.
	typedef Btl::List<C_ResourceBagListener*> C_Listeners; ///< all listeners to this bag.
	typedef Btl::Vector<Resource::C_HandleBase> C_OldResources; ///< all listeners to this bag.

	C_Resources		m_Resources;
	C_Listeners		m_Listeners; ///< listeners to this bag
	U32				m_ListenerLockCount; ///< delay informing until locks are released
#ifndef CFG_GOLD
	C_OldResources	m_OldResources;
#endif

	C_ResourceBag();
	virtual ~C_ResourceBag();

	C_ResourceBag(const C_ResourceBag&); // can't be copied
	C_ResourceBag& operator = (const C_ResourceBag&); // can't be copied

	Resource::C_ResourceBase* GetResourceBase(const ResourceUtils::C_ResourceBagTag tag, const Resource::C_ResourceTypeId typeId) const OVERRIDE;

	void NotifyListenersIfComplete() const; ///< If no resources are loading then notify all listeners.
};


#endif // GAME_RESOURCEBAG_H
