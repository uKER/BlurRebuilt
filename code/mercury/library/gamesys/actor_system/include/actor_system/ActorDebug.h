#ifndef ACTORDEBUG_2CD7B28D_2DEA_4B09_ADF9_D50BE57C339F_H
#define ACTORDEBUG_2CD7B28D_2DEA_4B09_ADF9_D50BE57C339F_H

#ifdef _MSC_VER
#pragma once
#endif


#include <actor_system/ActorDeclare.h>
#include <xt/XtGame.h>


namespace Debug
{
	class Logger;
}


namespace Actor
{

/// Interface that games can use to filter debug messages to specific logs.
class I_ActorDebugLogs
{
public:
	virtual ~I_ActorDebugLogs();

	virtual Debug::Logger* GetGeneral() const; ///< Log to use for general debug.
	virtual Debug::Logger* GetLevelDesign(const C_Actor* pActor) const; ///< Log to use for actors in the level design.
	virtual Debug::Logger* GetSorcery(const XtRecord* pRecord) const; ///< Log to use for Sorcery data.
	virtual Debug::Logger* GetAsset(const Sys::C_FilePath& filename) const; ///< Log to use for resource files.

};

extern const I_ActorDebugLogs* GetDebugLogs(); ///< Get the global debug log interface.
extern void SetDebugLogs(const I_ActorDebugLogs* pDebugLogs); ///< Set the global debug log interface.

extern Debug::Logger& GetDebugLogGeneral(); ///< Log to use for general debug.
extern Debug::Logger& GetDebugLogLevelDesign(const C_Actor* pActor = NULL); ///< Log to use for actors in the level design.
extern Debug::Logger& GetDebugLogSorcery(const XtRecord* pRecord = NULL); ///< Log to use for Sorcery data.
extern Debug::Logger& GetDebugLogAsset(const Sys::C_FilePath filename = Sys::C_FilePath()); ///< Log to use for resource files.

/**
@brief Log to use for Sorcery data.
@param T_DataType type of sorcery data to debug.
@param handle handle of sorcery data to debug.
@returns logger to use for this data.
*/
template <typename T_DataType>
Debug::Logger& GetDebugLogSorcery(const XtHandle<T_DataType>& handle) {return GetDebugLogSorcery(handle.Record());}

/**
@brief Log to use for resource files.
@param T_GameResource type that supplies resource filenames.
@param gameResource object that supplies the filename of the resource.
@returns logger to use for this data.
*/
template <typename T_GameResource>
Debug::Logger& GetDebugLogAsset(const T_GameResource& gameResource) {return GetDebugLogAsset(gameResource.GetFileName());}

/**
@brief Log to use for resource files.
@param T_GameResource type that supplies resource filenames.
@param handle handle to object that supplies the filename of the resource..
@returns logger to use for this data.
*/
template <typename T_GameResource>
Debug::Logger& GetDebugLogAsset(const XtHandle<T_GameResource>& handle) {return GetDebugLogAsset(handle ? handle->GetFileName() : Sys::C_FilePath());}

} // namespace Actor


#endif // ACTORDEBUG_2CD7B28D_2DEA_4B09_ADF9_D50BE57C339F_H
