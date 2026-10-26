#include "ActorSystemPch.h"
#include <actor_system/ActorDebug.h>

#include <system/debug/log/GlobalInterface.h>


namespace ActorDebug_Impl
{

const Actor::I_ActorDebugLogs* g_pDebugLogs = NULL; ///< global debug logs.

/**
@brief Makes sure we have a valid log.
@param pLog log we'd like to use.
@returns log we can use.
*/
inline Debug::Logger& EnsureLog(Debug::Logger* pLog)
{
	return (pLog != NULL) ? *pLog : Debug::GetRootLogger();
}

}


namespace Actor
{

I_ActorDebugLogs::~I_ActorDebugLogs()
{
}

/**
This is used for general system debug, usually programmatic things not
connected to specific data.
@returns log to use for general debug, may be NULL.
*/
Debug::Logger* I_ActorDebugLogs::GetGeneral() const
{
	return NULL;
}

/**
This is used for level design debug such as reporting invalid data in actor
studio levels. Games might choose to filter on the actor type or actor name
for example.
@param pActor actor to debug, may be NULL.
@returns log to use for this actor, may be NULL.
*/
Debug::Logger* I_ActorDebugLogs::GetLevelDesign(const C_Actor* pActor) const
{
	return GetGeneral();
}

/**
This is used for general sorcery data not specifically connected to actor
studio. Games might choose to filter on the XtType or XtPack for example.
@param pRecord data to debug, may be NULL.
@returns log to use for this data, may be NULL.
*/
Debug::Logger* I_ActorDebugLogs::GetSorcery(const XtRecord* pRecord) const
{
	return GetGeneral();
}

/**
This is used for assets loaded from disk. Games might choose to filter on the
extension or by matching paths for example.
@param filename resource to debug, may be empty.
@returns log to use for this resource, may be NULL.
*/
Debug::Logger* I_ActorDebugLogs::GetAsset(const Sys::C_FilePath& filename) const
{
	return GetGeneral();
}




/**
@returns global debug logs interface, may be NULL.
*/
const I_ActorDebugLogs* GetDebugLogs()
{
	return ActorDebug_Impl::g_pDebugLogs;
}

/**
@param pDebugLogs interface to use for global debug logs, may be NULL.
*/
void SetDebugLogs(const I_ActorDebugLogs* pDebugLogs)
{
	ActorDebug_Impl::g_pDebugLogs = pDebugLogs;
}




/**
@returns logger to use for general debug.
*/
Debug::Logger& GetDebugLogGeneral()
{
	using namespace ActorDebug_Impl;
	return EnsureLog((g_pDebugLogs != NULL) ? g_pDebugLogs->GetGeneral() : NULL);
}

/**
@param pActor actor to debug, may be NULL.
@returns logger to use for debug on this actor.
*/
Debug::Logger& GetDebugLogLevelDesign(const C_Actor* pActor)
{
	using namespace ActorDebug_Impl;
	return EnsureLog((g_pDebugLogs != NULL) ? g_pDebugLogs->GetLevelDesign(pActor) : NULL);
}

/**
@param pRecord data to debug, may be NULL.
@returns logger to use for debug on this data.
*/
Debug::Logger& GetDebugLogSorcery(const XtRecord* pRecord)
{
	using namespace ActorDebug_Impl;
	return EnsureLog((g_pDebugLogs != NULL) ? g_pDebugLogs->GetSorcery(pRecord) : NULL);
}

/**
@param filename filename of the resource, may be empty string.
@returns logger to use for debug on this resource.
*/
Debug::Logger& GetDebugLogAsset(const Sys::C_FilePath filename)
{
	using namespace ActorDebug_Impl;
	return EnsureLog((g_pDebugLogs != NULL) ? g_pDebugLogs->GetAsset(filename) : NULL);
}

} // namespace Actor
