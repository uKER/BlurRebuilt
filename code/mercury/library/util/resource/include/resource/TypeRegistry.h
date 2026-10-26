#ifndef UTIL_RESOURCE_TYPE_REGISTRY_H
#define UTIL_RESOURCE_TYPE_REGISTRY_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\Core.h>

#include <btl/containers/vector.h>
#include <btl/Containers/list.h>
#include <system/serialise.h>

//! @addtogroup Resource
//! @{

namespace Resource
{

typedef S32 C_ResourceTypeId;

class C_ResourceBase;

//! C_TypeRegistry. Used to convert between string resource type name and integer resource type id.
//! derived resource types are registered with this class during static initialisaion.
class C_TypeRegistry
{
public:
	static C_TypeRegistry* GetInstance();

	static void DeleteInstance();

	typedef C_ResourceBase* ( *F_Create )(); ///< factory creation function

	// Register the resource type.
	C_ResourceTypeId RegisterResourceType( const Util::C_String& resourceTypeName, const F_Create pCreate );

	// Lookup resource ID from resource name, returns -1 if not registered.
	C_ResourceTypeId GetResourceTypeId( const Util::C_String& resourceTypeName );

	// Lookup resource name from id.
	Util::C_String GetResourceTypeName( const C_ResourceTypeId typeId );

	// Create a resource object
	C_ResourceBase* CreateResource( const C_ResourceTypeId typeId );

	// Destroy a resource object
	void DestroyResource( C_ResourceBase* pResourceBase );

private:
	static C_TypeRegistry*& _GetInstance();

private:
	struct C_TypeInfo;

	Btl::Vector<C_TypeInfo> m_TypeInfoList;

};

} // namespace Resource

//! @}

#endif
