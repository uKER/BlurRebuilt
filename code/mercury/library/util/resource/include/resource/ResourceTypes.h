#ifndef UTIL_RESOURCE_RESOURCE_TYPES_H
#define UTIL_RESOURCE_RESOURCE_TYPES_H

#ifdef _MSC_VER
#pragma once
#endif

//! @addtogroup Resource
//! @{ 

namespace Resource
{

//! E_State
enum E_State
{
	E_State_Unknown = 0,
	E_State_Constructed,
	E_State_InLoadQueue,
	E_State_Loading,
	E_State_PostLoading,
	E_State_Ready,
	E_State_Evicting,
	E_State_PreDeleting,
	E_State_Deleted
};

//! E_Notify
enum E_Notify
{
	E_Notify_NoData =0,
	E_Notify_DataReady,
	E_Notify_DataError
};

} // namespace Resource

//! @}

#endif
