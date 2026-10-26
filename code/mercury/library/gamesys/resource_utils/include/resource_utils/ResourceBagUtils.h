#ifndef GAMESYS_RESOURCEBAGUTILS_H
#define GAMESYS_RESOURCEBAGUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <xt/XtGame.h>

#include "resource_utils/I_ResourceBag.h"
#include "resource_utils/ResourceUtilsDeclare.h"


/// Makes a tag from a string
ResourceUtils::C_ResourceBagTag MakeResourceBagTag(const char* string);

/// Makes a tag from an Xt record
ResourceUtils::C_ResourceBagTag MakeResourceBagTag(const XtRecord* pRecord);

/**
@brief Makes a tag from an Xt handle.
@param handle handle to generate tag.
@returns tag generated from handle.
*/
template <class T_DataType>
ResourceUtils::C_ResourceBagTag MakeResourceBagTag(const XtHandle<T_DataType>& handle)
{
	return MakeResourceBagTag(handle.Record());
}

/**
@brief Helper for adding a resource from a GameResource handle.
@param bag bag to add to.
@param tag tag to identify resource.
@param gameResource resource to load.
*/
template <class T_GameResource>
void AddToResourceBag(ResourceUtils::I_ResourceBag& bag, const ResourceUtils::C_ResourceBagTag tag, const XtHandle<T_GameResource>& gameResource)
{
	if (gameResource)
		bag.AddResource(tag, gameResource->GetFileName(), T_GameResource::GetResourceTypeId());
}


#endif // GAMESYS_RESOURCEBAGUTILS_H
