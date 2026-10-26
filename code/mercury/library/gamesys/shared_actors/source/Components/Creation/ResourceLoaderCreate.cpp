#include "SharedActorsPch.h"

#include <shared_actors/Components/Creation/ResourceLoaderCreate.h>
#include <shared_actors/Components/Interfaces/I_ResourceLoader.h>


namespace Components
{

C_ResourceDesc::C_ResourceDesc() :
	m_Name(), 
	m_Type(-1), 
	m_Flags(0)
{
	// intentionally blank
}




C_ResourceItemCreate::C_ResourceItemCreate() :
	Component::I_ComponentCreator(), 
	m_Desc(), 
	m_ResourceNotificationFlags(kDefaultResourceNotificationFlags)
{
	// intentionally blank
}

C_ResourceItemCreate::~C_ResourceItemCreate()
{
	// intentionally blank
}





C_ResourceListCreate::C_ResourceListCreate() :
	Component::I_ComponentCreator(), 
	m_List(), 
	m_ResourceNotificationFlags(kDefaultResourceNotificationFlags)
{
	// intentionally blank
}

C_ResourceListCreate::~C_ResourceListCreate()
{
	// intentionally blank
}

/**
@param resourceName name of resource to load.
@param typeId type of resource to load.
@param resourceFlags bitfield of E_ResourceFlag.
@returns index of the resource.
*/
U32 C_ResourceListCreate::AddItem(const Util::C_String& resourceName, const Resource::C_ResourceTypeId typeId, const U32 resourceFlags)
{
	const U32 index = m_List.Size();
	
	m_List.Resize(index + 1);
	C_ResourceDesc& item = m_List.Back();
	item.m_Name = resourceName;
	item.m_Type = typeId;
	item.m_Flags = resourceFlags;

	return index;
}

/**
@param index index of resource
@param resourceName name of resource to load.
@param typeId type of resource to load.
@param resourceFlags bitfield of E_ResourceFlag.
*/
void C_ResourceListCreate::SetItem(const U32 index, const Util::C_String& resourceName, const Resource::C_ResourceTypeId typeId, const U32 resourceFlags)
{
	if (index >= static_cast<U32>(m_List.Size()))
	{
		m_List.Resize(index + 1);
	}

	C_ResourceDesc& item = m_List[index];
	item.m_Name = resourceName;
	item.m_Type = typeId;
	item.m_Flags = resourceFlags;
}

} // namespace Components
