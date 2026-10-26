#ifndef MESSAGEDEBUG_DC482830_3F0B_4650_AF9C_6107BD70C326_H
#define MESSAGEDEBUG_DC482830_3F0B_4650_AF9C_6107BD70C326_H

#ifdef _MSC_VER
#pragma once
#endif


#include <misc_shared/messaging/MessagingDeclare.h>
#include <system/debug/log/DebugLog.h>


#ifndef CFG_GOLD
 #define MESSAGE_DEBUG_TYPE_ENUM 1
#else
 #define MESSAGE_DEBUG_TYPE_ENUM 0
#endif


namespace Messaging
{

extern void SetDefaultLog(Debug::Logger* pLog); ///<@ param pLog default log for messaging system.
extern Debug::Logger& GetDefaultLog(); ///< @returns default log for messaging system.

#ifndef CFG_GOLD

/// Debug info for a message type.
struct C_MessageDebugInfo
{
	const char*	m_MsgName;	///< weak ref to permanent storage, human-readable message type name
};

extern U32 GetNumDebugInfoRegistered(); ///< @returns the number of message types with debug info registered.
extern void UnregisterDebugInfo(); ///< Clears all debug info.
extern void RegisterDebugInfo(const TMsgType msgType, const char* name); ///< Registers extra type information for debugging.
extern const C_MessageDebugInfo* FindDebugInfo(const TMsgType msgType); ///< Finds the debug info for this message type.
extern void MissingDebugInfo(const TMsgType msgType); ///< Informs about message types that are missing debug info.

extern void RegisterMessageDebugInfo(const char* const* pMessageNames, const U32 messageCount, const U32 messageBegin); /// Registers a contiguous sequence of messages from an array of message names

#endif // CFG_GOLD

extern const char* GetUnregisteredTypeName(); ///< @returns name to use for unregistered message types.
extern const char* FindDebugName(const TMsgType msgType); ///< Find the debug name for this message type.


#if MESSAGE_DEBUG_TYPE_ENUM

class C_DebugMessageType;

C_DebugMessageType* CreateDebugMessageType(const char* name);
void DestroyDebugMessageType(C_DebugMessageType* pVar);

TMsgType GetDebugMessageType(const C_DebugMessageType& var);
void SetDebugMessageType(C_DebugMessageType& var, const TMsgType msgType);

#endif // MESSAGE_DEBUG_TYPE_ENUM

} // namespace Messaging


/**
@define REGISTER_MESSAGE_DEBUG_INFO
@breif Registers debug info for a range of message types.
@param x_NameArray an array of type 'const char*' containing the names of each message type.
@param x_EnumCount the number of message types with contiguous values.
@param x_EnumStart the value of the first message type.
*/
#ifndef CFG_GOLD
#define REGISTER_MESSAGE_DEBUG_INFO(x_NameArray, x_EnumCount, x_EnumStart) \
	StaticAssert(lengthof(x_NameArray) == x_EnumCount, THE_NUMBER_OF_MESSAGE_NAMES_IN_THE_ARRAY_DOES_NOT_MATCH_THE_NUMBER_OF_MESSAGE_TYPES_IN_THE_ENUM); \
	Messaging::RegisterMessageDebugInfo(x_NameArray, x_EnumCount, x_EnumStart);
#else
#define REGISTER_MESSAGE_DEBUG_INFO(x_NameArray, x_EnumCount, x_EnumStart) \
	StaticAssert(lengthof(x_NameArray) == x_EnumCount, THE_NUMBER_OF_MESSAGE_NAMES_IN_THE_ARRAY_DOES_NOT_MATCH_THE_NUMBER_OF_MESSAGE_TYPES_IN_THE_ENUM);
#endif


#endif // MESSAGEDEBUG_DC482830_3F0B_4650_AF9C_6107BD70C326_H
