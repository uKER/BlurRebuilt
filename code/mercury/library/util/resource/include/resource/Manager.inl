#ifndef UTIL_RESOURCE_MANAGER_INL
#define UTIL_RESOURCE_MANAGER_INL

#include <system/misc/StringUtil.h>

namespace Resource
{
// C_Manager

template<typename T_Resource>
T_Resource* C_Manager::GetResource( const Sys::C_FilePath& filenameIn )
{
	return static_cast<T_Resource*>( GetResource( filenameIn, T_Resource::StaticGetResourceTypeId() ) );
}


} // namespace Resource

#endif
