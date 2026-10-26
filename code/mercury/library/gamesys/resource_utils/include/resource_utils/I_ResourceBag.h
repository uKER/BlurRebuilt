#ifndef GAMESYS_I_RESOURCE_BAG_H
#define GAMESYS_I_RESOURCE_BAG_H

#ifdef _MSC_VER
#pragma once
#endif

#include <resource/Resource.h>
#include <resource_utils/ResourceUtilsDeclare.h>

namespace ResourceUtils
{

/// Reference-counted container for resource handles.
class I_ResourceBag
{
public:
	virtual ~I_ResourceBag();

	virtual void AddResource(const C_ResourceBagTag tag, const Sys::C_FilePath& fileName, const Resource::C_ResourceTypeId typeId) = 0; ///< Bags a resource.

	/// Gets a bagged resource.
	template <class T_Resource>
	T_Resource* GetResource(const C_ResourceBagTag tag) const;

	virtual bool IsWaitingForLoad() const = 0; ///< Checks if resources are still loading.

#ifndef CFG_GOLD
	virtual void DebugOutputBagContents(void) const;
#endif


private:
	virtual Resource::C_ResourceBase* GetResourceBase(const C_ResourceBagTag tag, const Resource::C_ResourceTypeId typeId) const = 0; ///< Gets a bagged resource.
};

/**
@brief Extracts a resource from the bag.
@param tag unique resource identifier.
@returns NULL if the resource isn't loaded or isn't in the bag.
*/
template <class T_Resource>
T_Resource* I_ResourceBag::GetResource(const C_ResourceBagTag tag) const
{
	T_Resource* pResource = NULL;
	{
		Resource::C_ResourceBase* pBase = GetResourceBase(tag, Resource::C_Resource<T_Resource>::StaticGetResourceTypeId());

		if (pBase != NULL)
		{
			pResource = static_cast<T_Resource*>(pBase);
		}
	}

	return pResource;
}

} // namespace ResourceUtils

#endif // GAMESYS_I_RESOURCE_BAG_H
