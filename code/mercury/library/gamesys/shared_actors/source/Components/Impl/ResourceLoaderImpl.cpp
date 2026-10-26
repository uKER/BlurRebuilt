#include "SharedActorsPch.h"

#include <shared_actors/Components/Impl/ResourceLoaderImpl.h>

#include <actor_system/ActorDebug.h>
#include <shared_actors/Components/Messages/ResourceMessages.h>
#include <actor_system/Actor.h>


namespace Components
{

C_ResourceLoaderImpl::C_Item::C_Item() :
	m_Handle(), 
	m_State(E_State_None)
{
	// intentionally blank
}

C_ResourceLoaderImpl::C_Item::~C_Item()
{
	// intentionally blank
}





C_ResourceLoaderImpl::C_ResourceLoaderImpl()
{
	// intentionally blank
}

C_ResourceLoaderImpl::~C_ResourceLoaderImpl()
{
	// intentionally blank
}

void C_ResourceLoaderImpl::ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData)
{
	C_Item* pItem = reinterpret_cast<C_Item*>(pData);
	E_State state = E_State_None;

	switch (result)
	{
	case Resource::E_Notify_NoData:		state = E_State_None;		break;
	case Resource::E_Notify_DataReady:	state = E_State_Loaded;		break;
	case Resource::E_Notify_DataError:	state = E_State_Error;		break;
	}

	SetState(pItem, state);
}

void C_ResourceLoaderImpl::RequestLoad(const U32 index)
{
	const C_ResourceDesc* pDesc = GetDesc(index);
	C_Item* pItem = GetItem(index);

	if ((pDesc != NULL) && (pItem != NULL))
	{
		switch (pItem->m_State)
		{
		case E_State_Loading:
		case E_State_Loaded:
			break;

		default:
			SetState(pItem, E_State_Loading);
			pItem->m_Handle.Load(pDesc->m_Name, pDesc->m_Type, this, pItem);
			break;
		}
	}
}

void C_ResourceLoaderImpl::RequestUnload(const U32 index)
{
	C_Item* pItem = GetItem(index);

	if ((pItem != NULL) && (pItem->m_State != E_State_None))
	{
		SetState(pItem, E_State_Unloading);
		pItem->m_Handle = Resource::C_HandleBase();
	}
}

bool C_ResourceLoaderImpl::PrimeImpl()
{
	bool primed = true;

	const U32 numResources = GetNumResources();
	for (U32 i = 0; i < numResources; ++i)
	{
		const C_ResourceDesc* pDesc = GetDesc(i);
		if ((pDesc != NULL) && pDesc->GetPrime())
		{
			RequestLoad(i);
			primed = false;
		}
	}

	if (!primed)
		primed = PollForPrimed();

	return primed;
}

bool C_ResourceLoaderImpl::PollForPrimed() const
{
	const U32 numResources = GetNumResources();
	for (U32 i = 0; i < numResources; ++i)
	{
		const C_ResourceDesc* pDesc = GetDesc(i);
		if ((pDesc != NULL) && pDesc->GetPrime())
		{
			const C_Item* pItem = GetItem(i);
			if ((pItem != NULL) && (pItem->m_State == E_State_Loading))
				return false;
		}
	}

	return true;
}

Resource::C_ResourceBase* C_ResourceLoaderImpl::GetResourceBase(const U32 index) const
{
	const C_Item* pItem = GetItem(index);
	if ((pItem != NULL) && (pItem->m_State == E_State_Loaded))
		return pItem->m_Handle;

	return NULL;
}

void C_ResourceLoaderImpl::SetState(C_Item* pItem, const E_State state)
{
	if ((pItem != NULL) && (pItem->m_State != state))
	{
		const U32 index = GetIndexOf(pItem);
		if (index < GetNumResources())
		{
			pItem->m_State = state;

			if (GetLifetimeState() == ActorSys::E_LifetimeState_Priming)
			{
				// check if resources are primed
				if (PollForPrimed())
					SetPrimed();
			}
			else
			{
				const U32 resourceNotificationFlags = GetResourceNotificationFlags();

				// send message for this resource
				switch (state)
				{
				case E_State_Unloading:	
					if (resourceNotificationFlags & E_ResourceNotificationFlag_UnloadItem)
					{
						const ActorMsg::C_ResourceUnloadedMsg message(index);
						SendMessageToActor(&message);	
					}
					break;

				case E_State_Loaded:
					if (resourceNotificationFlags & E_ResourceNotificationFlag_LoadItem)
					{
						const ActorMsg::C_ResourceLoadedMsg message(index);
						SendMessageToActor(&message);
					}
					break;

				case E_State_Error:	
					if (resourceNotificationFlags & E_ResourceNotificationFlag_ErrorItem)
					{
						const ActorMsg::C_ResourceErrorMsg message(index);
						SendMessageToActor(&message);
					}
					else
					{
						LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), DBSTR "Failed to load resource %s for actor %s", GetDesc(index)->m_Name.Chars(), GetActor()->GetActorName());
					}
					break;

				default:
					break;
				}

				// send message for all resources
				{
					bool anyLoaded = false;
					bool anyUnloaded = false;
					bool anyCritical = false;
					bool allLoaded = true;
					bool allUnloaded = true;
					bool allCritical = true;

					const U32 numResources = GetNumResources();
					for (U32 i = 0; i < numResources; ++i)
					{
						const C_ResourceDesc* pDesc = GetDesc(i);
						if (pDesc != NULL)
						{
							const C_Item* pItem = GetItem(i);
							if (pItem != NULL)
							{
								switch (pItem->m_State)
								{
								case E_State_Loading:
									allLoaded = false;
									allUnloaded = false;
									break;

								case E_State_Loaded:
									anyLoaded = true;
									allUnloaded = false;
									break;

								case E_State_None:
								case E_State_Unloading:
								case E_State_Error:
								default:
									anyUnloaded = true;
									allLoaded = false;
									break;
								}

								if (pDesc->GetCritical())
								{
									if (pItem->m_State == E_State_Loaded)
										anyCritical = true;
									else
										allCritical = false;
								}
							}
						}
					}

					if (anyLoaded && allLoaded && (resourceNotificationFlags & E_ResourceNotificationFlag_LoadAll))
						SendMessageToActor(ActorMsg::E_ResourceMsg_AllLoaded);

					if (anyUnloaded && allUnloaded && (resourceNotificationFlags & E_ResourceNotificationFlag_UnloadAll))
						SendMessageToActor(ActorMsg::E_ResourceMsg_AllUnloaded);

					if (anyCritical && allCritical && (resourceNotificationFlags & E_ResourceNotificationFlag_CriticalAll))
						SendMessageToActor(ActorMsg::E_ResourceMsg_AllCritical);
				}
			}
		}
	}
}







C_ResourceItem::C_ResourceItem() :
	C_ResourceLoaderImpl(), 
	m_Item()
{
	// intentionally blank
}

C_ResourceItem::~C_ResourceItem()
{
	// intentionally blank
}

U32 C_ResourceItem::GetNumResources() const
{
	return 1;
}

const C_ResourceDesc* C_ResourceItem::GetDesc(const U32 index) const
{
	if (index == 0)
		return &GetCreationData()->m_Desc;

	return NULL;
}

C_ResourceLoaderImpl::C_Item* C_ResourceItem::GetItem(const U32 index)
{
	if (index == 0)
		return &m_Item;

	return NULL;
}

U32 C_ResourceItem::GetIndexOf(const C_Item* pItem) const
{
	if (pItem == &m_Item)
		return 0;

	return 1;
}

U32 C_ResourceItem::GetResourceNotificationFlags() const
{
	return GetCreationData()->m_ResourceNotificationFlags;
}





C_ResourceList::C_ResourceList() :
	C_ResourceLoaderImpl(), 
	m_Items()
{
	// intentionally blank
}

C_ResourceList::~C_ResourceList()
{
	// intentionally blank
}

U32 C_ResourceList::GetNumResources() const
{
	return GetCreationData()->m_List.Size();
}

bool C_ResourceList::PrimeImpl()
{
	m_Items.Resize(GetCreationData()->m_List.Size());
	return C_ResourceLoaderImpl::PrimeImpl();
}

const C_ResourceDesc* C_ResourceList::GetDesc(const U32 index) const
{
	const C_ResourceListCreate* pCreator = GetCreationData();

	if (index < (U32)pCreator->m_List.Size())
		return &pCreator->m_List[index];

	return NULL;
}

C_ResourceLoaderImpl::C_Item* C_ResourceList::GetItem(const U32 index)
{
	if (index < (U32)m_Items.Size())
		return &m_Items[index];

	return NULL;
}

U32 C_ResourceList::GetIndexOf(const C_Item* pItem) const
{
	if (m_Items.Size() > 0)
	{
		const U32 index = pItem - &m_Items.Front();
		if (index < (U32)m_Items.Size())
			return index;
	}

	return m_Items.Size();
}

U32 C_ResourceList::GetResourceNotificationFlags() const
{
	return GetCreationData()->m_ResourceNotificationFlags;
}

} // namespace Components
