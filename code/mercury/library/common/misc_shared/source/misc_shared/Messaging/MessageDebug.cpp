#include "MiscSharedPch.h"
#include <misc_shared/messaging/MessageDebug.h>

#include <btl/allocators/pool.h>
#include <core/BizAssert.h>
#include <core/containers/genesis_containers/Sort.h>
#include <misc_shared/uid/TypeTable.h>
#include <system/debug/Variables.h>


namespace MessageDebug_Impl
{

Debug::Logger* g_pDefaultLog = NULL;

#if MESSAGE_DEBUG_TYPE_ENUM

const U32 kMaxMessageDebugTypes = 2048 - 1;
U32 g_NumMessageDebugTypes = 0;

Debug::Option g_MessageDebugTypeOptions[kMaxMessageDebugTypes + 1] = { { -1, 0 } };

#endif // MESSAGE_DEBUG_TYPE_ENUM

#ifndef CFG_GOLD

typedef UID::C_TypeTableHashPolicyHi20<Messaging::TMsgType, 256> C_HashFunctor;
typedef UID::C_TypeTable<Messaging::TMsgType, Messaging::C_MessageDebugInfo, C_HashFunctor> C_HashTable;

C_HashTable g_HashTable;

struct C_TypeInfo
{
	U32			m_Type;
	const char*	m_Name;

	inline bool operator > (const C_TypeInfo& other) const {return m_Type > other.m_Type;}
};

typedef Btl::Vector<C_TypeInfo> C_TypeInfos;

void DebugDumpInfo(void*)
{
	Debug::Logger& log = Messaging::GetDefaultLog();

	LOGINFO(log, "Dumping messaging system debug info for %d message types...", g_HashTable.Size());

	// gather messages and sort by type id
	C_TypeInfos typeInfos;
	typeInfos.Reserve(static_cast<int>(g_HashTable.Size()));

	for (C_HashTable::ConstIterator it = g_HashTable.Begin(); it != g_HashTable.End(); ++it)
	{
		C_TypeInfo typeInfo;
		typeInfo.m_Type = static_cast<U32>(it.GetUID());
		typeInfo.m_Name = it.GetData().m_MsgName;
		typeInfos.PushBack(typeInfo);
	}

	Util::ShellSortArray<C_TypeInfos, C_TypeInfo>(typeInfos, typeInfos.Size());

	// dump sorted messages
	for (C_TypeInfos::ConstIterator it = typeInfos.Begin(); it != typeInfos.End(); ++it)
	{
		LOGINFO(log, "    0x%8x    %s", it->m_Type, it->m_Name);
	}
}

#endif // CFG_GOLD

} // namespace MessageDebug_Impl


#if MESSAGE_DEBUG_TYPE_ENUM
inline bool operator > (const Debug::Option& lhs, const Debug::Option& rhs) {return stricmp(lhs.name, rhs.name) > 0;}
#endif


#ifndef CFG_GOLD
Debug::Function g_DebugDumpMessageDebugInfo("System.Messaging.DebugDumpInfo", MessageDebug_Impl::DebugDumpInfo, NULL);
#endif


namespace Messaging
{

void SetDefaultLog(Debug::Logger* pLog)
{
	MessageDebug_Impl::g_pDefaultLog = pLog;
}

Debug::Logger& GetDefaultLog()
{
	return (MessageDebug_Impl::g_pDefaultLog != NULL) ? *MessageDebug_Impl::g_pDefaultLog : Debug::GetRootLogger();
}

#ifndef CFG_GOLD

U32 GetNumDebugInfoRegistered()
{
	return static_cast<U32>(MessageDebug_Impl::g_HashTable.Size());
}

void UnregisterDebugInfo()
{
	MessageDebug_Impl::g_HashTable.Clear();
}

/**
@param msgType message type to find.
@returns NULL if the debug info has not been registered.
*/
const C_MessageDebugInfo* FindDebugInfo(const TMsgType msgType)
{
	using namespace MessageDebug_Impl;

	const C_HashTable::ConstIterator it = g_HashTable.Find(msgType);

	return (it != g_HashTable.End()) ? &it.GetData() : NULL;
}

/**
The game should register all the message types that it will use.
@param msgType message type to register.
@param name name of message type.
*/
void RegisterDebugInfo(const TMsgType msgType, const char* name)
{
	BIZ_ASSERT(name != NULL);
	BIZ_ASSERT(strlen(name) > 0);

	using namespace MessageDebug_Impl;

	const C_HashTable::InsertionResult result = g_HashTable.Insert(msgType);

	if (result.first)
	{
		result.second.GetData().m_MsgName = name;
	}
	else
	{
		LOGERROR(GetDefaultLog(), DBSTR " message type %d already registered as '%s'!", msgType, result.second.GetData().m_MsgName);
	}
}

void RegisterMessageDebugInfo(const char* const* pMessageNames, const U32 messageCount, const U32 messageBegin)
{
	const U32 preCount = GetNumDebugInfoRegistered();

	for (U32 i = 0; i < messageCount; ++i)
	{
		RegisterDebugInfo(static_cast<TMsgType>(messageBegin + i), pMessageNames[i]);
	}

	const U32 postCount = GetNumDebugInfoRegistered();
	BIZ_ASSERT((postCount - preCount) == messageCount);
}

#endif // CFG_GOLD

const char* GetUnregisteredTypeName()
{
	return "UNREGISTERED MESSAGE TYPE";
}

/**
@param msgType message type to find.
@returns a special string is the type has no debug info registered.
*/
const char* FindDebugName(const TMsgType msgType)
{
	using namespace MessageDebug_Impl;

#ifndef CFG_GOLD
	const C_MessageDebugInfo* pInfo = FindDebugInfo(msgType);

	if (pInfo != NULL)
	{
		return pInfo->m_MsgName;
	}
#endif // CFG_SPEED

	return GetUnregisteredTypeName();
}

void MissingDebugInfo(const TMsgType msgType)
{
	// maintain buckets of LRU message types
	static const U32 kNumBuckets = 8;
	static const U32 kBucketSize = 8;

	static TMsgType lruBuckets [kNumBuckets] [kBucketSize] = {0};
	static U8 lruEvict [kNumBuckets] = {0}; // index of next item to evict from each LRU bucket

	// get bucket index for this message type
	// we're just using the lo bits here, if kNumBuckets is large then most of them won't be used!
	const U32 bucketIndex = (U32)msgType % kNumBuckets;
	
	// check if message type is in the bucket
	// if kBucketSize is large then this could get expensive
	for (U32 i = 0; i < kBucketSize; ++i)
	{
		if (lruBuckets[bucketIndex][i] == msgType)
		{
			return;
		}
	}

	// replace LRU item in bucket
	lruBuckets[bucketIndex][lruEvict[bucketIndex]] = msgType;
	lruEvict[bucketIndex] = (U8)((U32)(lruEvict[bucketIndex] + 1) % kBucketSize);

	// warn about missing info
	LOGWARN(GetDefaultLog(), DBSTR " unregistered message type 0x%08x", msgType);
}


#if MESSAGE_DEBUG_TYPE_ENUM

class C_DebugMessageType : public Debug::EnumBase
{
public:
	C_DebugMessageType(const char* name)
		: Debug::EnumBase(name, MessageDebug_Impl::g_MessageDebugTypeOptions)
	{
		//SetDynamic();
	}

	virtual void SetRawValue(int rawValue) {m_RawValue = static_cast<TMsgType>(rawValue);}
	virtual int RawValue() const {return static_cast<int>(m_RawValue);}
	virtual int RawDefault() const {return -1;}

	void SetMessageType(const TMsgType msgType) {m_RawValue = msgType;}
	TMsgType GetMessageType() const {return m_RawValue;}

private:
	TMsgType m_RawValue;

};

C_DebugMessageType* CreateDebugMessageType(const char* name)
{
	using namespace MessageDebug_Impl;

	// ensure global table is up-to-date
	if ((g_NumMessageDebugTypes < kMaxMessageDebugTypes) && (g_NumMessageDebugTypes != g_HashTable.Size()))
	{
		if (kMaxMessageDebugTypes < g_HashTable.Size())
		{
			LOGWARN(GetDefaultLog(), DBSTR " there are too many message types (%d/%d) to list all in the debug enum variable drop-down", g_HashTable.Size(), kMaxMessageDebugTypes);
		}

		// populate data table
		{
			U32 numMessageDebugTypes = 0;
			C_HashTable::ConstIterator it = g_HashTable.Begin();

			while ((numMessageDebugTypes < kMaxMessageDebugTypes) && (it != g_HashTable.End()))
			{
				Debug::Option& dest = g_MessageDebugTypeOptions[numMessageDebugTypes];

				dest.value = static_cast<int>(it.GetUID());
				dest.name = it.GetData().m_MsgName;

				++numMessageDebugTypes;
				++it;
			}

			g_NumMessageDebugTypes = numMessageDebugTypes;
		}

		// sort alphabetically
		{
			Debug::Option* pOptions = &g_MessageDebugTypeOptions[0];
			Util::ShellSortArray<Debug::Option*, Debug::Option>(pOptions, g_NumMessageDebugTypes);
		}
	}

	return new C_DebugMessageType(name);
}

void DestroyDebugMessageType(C_DebugMessageType* pVar)
{
	delete pVar;
}

TMsgType GetDebugMessageType(const C_DebugMessageType& var)
{
	return var.GetMessageType();
}

void SetDebugMessageType(C_DebugMessageType& var, const TMsgType msgType)
{
	var.SetMessageType(msgType);
}

#endif // MESSAGE_DEBUG_TYPE_ENUM


} // namespace Messaging
