#ifndef RESOURCECOMPONENTS_79E53F62_D67A_4965_84E8_46947CAD5970_H
#define RESOURCECOMPONENTS_79E53F62_D67A_4965_84E8_46947CAD5970_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/UID/FourCC.h>


namespace Components
{

/// Component::C_ComponentInterfaceId values for shared resource components
enum E_ResourceIF
{
	E_ResourceIF_HiBits = FOURCC_HI20('SARI'), // Shared Actor Resource Interface

	E_ResourceIF_Loader,	///< @see I_ResourceLoader.

	E_ResourceIF_End, 
	E_ResourceIF_Begin = E_ResourceIF_HiBits + 1, 
	E_ResourceIF_Count = E_ResourceIF_End - E_ResourceIF_Begin
};

/// Component::C_ComponentTypeId values for shared resource components
enum E_Resource
{
	E_Resource_HiBits = FOURCC_HI20('SARC'), // Shared Actor Resource Component

	E_Resource_Item,		///< @see C_ResourceItem.
	E_Resource_List,		///< @see C_ResourceList.
	//E_Resource_Pack,		// TODO: [Oscar] 
	E_Resource_End, 
	E_Resource_Begin = E_Resource_HiBits + 1, 
	E_Resource_Count = E_Resource_End - E_Resource_Begin
};

} // namespace Components


#endif // RESOURCECOMPONENTS_79E53F62_D67A_4965_84E8_46947CAD5970_H
