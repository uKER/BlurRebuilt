#include "ResourceUtilsPch.h"

#include <resource_utils/ResourceBagUtils.h>

#include <btl/utility/hash.h>


/**
@param string string to generate tag from.
@returns tag generated from string.
*/
ResourceUtils::C_ResourceBagTag MakeResourceBagTag(const char* string)
{
	return static_cast<ResourceUtils::C_ResourceBagTag>(Btl::Hash(string));
}

/**
@param pRecord record to generate tag from.
@returns tag generated from record.
*/
ResourceUtils::C_ResourceBagTag MakeResourceBagTag(const XtRecord* pRecord)
{
	return static_cast<ResourceUtils::C_ResourceBagTag>(pRecord ? pRecord->id : 0);
}
