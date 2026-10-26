#ifndef RESOURCEMESSAGES_F34FEA6A_E07F_4BC5_B460_1B2BC2D8A724_H
#define RESOURCEMESSAGES_F34FEA6A_E07F_4BC5_B460_1B2BC2D8A724_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/Messaging/MessageValue.h>
#include <misc_shared/uid/FourCC.h>


namespace ActorMsg
{

enum E_ResourceMsg
{
	E_ResourceMsg_HiBits = FOURCC_HI20('AMRM'), // ActorMsg ResourceMsg

	E_ResourceMsg_Load,			///< @see C_ResourceLoadMsg
	E_ResourceMsg_Unload,		///< @see C_ResourceUnloadMsg

	E_ResourceMsg_LoadAll,		///< Request that all resources are loaded
	E_ResourceMsg_UnloadAll,	///< Request that all resources are unloaded

	E_ResourceMsg_Loaded,		///< @see C_ResourceLoadedMsg
	E_ResourceMsg_Unloaded,		///< @see C_ResourceUnloadedMsg
	E_ResourceMsg_Error,		///< @see C_ResourceErrorMsg

	E_ResourceMsg_AllLoaded,	///< All resources have loaded
	E_ResourceMsg_AllUnloaded,	///< All resources have unloaded
	E_ResourceMsg_AllCritical,	///< All critical resources have loaded
	
	E_ResourceMsg_End, 
	E_ResourceMsg_Begin = E_ResourceMsg_HiBits + 1, 
	E_ResourceMsg_Count = E_ResourceMsg_End - E_ResourceMsg_Begin
};

/**
@brief Message sent to request resource load.
GetValue() returns a user-defined index.
*/
typedef Messaging::C_MessageValue<U32, E_ResourceMsg_Load> C_ResourceLoadMsg;

/**
@brief Message sent to request resource unload.
GetValue() returns a user-defined index.
*/
typedef Messaging::C_MessageValue<U32, E_ResourceMsg_Unload> C_ResourceUnloadMsg;

/**
@brief Message sent when resource loads.
GetValue() returns a user-defined index.
*/
typedef Messaging::C_MessageValue<U32, E_ResourceMsg_Loaded> C_ResourceLoadedMsg;

/**
@brief Message sent when resource unloads.
GetValue() returns a user-defined index.
*/
typedef Messaging::C_MessageValue<U32, E_ResourceMsg_Unloaded> C_ResourceUnloadedMsg;

/**
@brief Message sent when resource contains errors.
GetValue() returns a user-defined index.
*/
typedef Messaging::C_MessageValue<U32, E_ResourceMsg_Error> C_ResourceErrorMsg;

} // namespace ActorMsg


#endif // RESOURCEMESSAGES_F34FEA6A_E07F_4BC5_B460_1B2BC2D8A724_H
