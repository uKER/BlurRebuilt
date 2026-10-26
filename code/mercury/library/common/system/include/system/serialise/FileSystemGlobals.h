#ifndef __SYSTEM_SERIALISE_FILESYSTEMGLOBALS_H__
#define __SYSTEM_SERIALISE_FILESYSTEMGLOBALS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <system\debug\log\DebugLevel.h>
#include <Btl/Utility/String.h>

namespace Debug
{
class Logger;
}

#ifndef CFG_GOLD
#define ENABLE_FS_LOGWRAPPER
//#define ENABLE_FS_SPAM
#endif

namespace Sys
{
	class I_FileSystem;

	typedef void (*FileRwErrorCallback)(const char* fileName, bool writing, bool isCritical, void* pUserData);
	typedef Btl::String (*FileReadErrorStringCallback)();

	/// Get the default file system. Initialised by C_CoreSystems and its derivatives.
	I_FileSystem* GetDefaultFileSystem();

	/// Set the default file system.
	void SetDefaultFileSystem(I_FileSystem* pFileSystem);

	Btl::String GetFileReadErrorString();

	void SetFileRwErrorCallback(FileRwErrorCallback cb);

	void SetFileReadErrorStringCallback(FileReadErrorStringCallback errStrCb);

	void FileRwError(const char* fileName, bool writing, bool isCritical, void* pUserData = 0);

	void FileOpenError(const char* fileName, bool writing, bool isCritical);

	void FsLog(Debug::Logger& logger, ::DebugLevel::Level level, bool isSpam, const char* text, ...);

#if defined(ENABLE_FS_LOGWRAPPER)
	#define FSINFO(logger, txt, ...) Sys::FsLog(logger, ::DebugLevel::Info, false, txt, __VA_ARGS__)
	#define FSWARN(logger, txt, ...) Sys::FsLog(logger, ::DebugLevel::Warn, false, txt, __VA_ARGS__)
	#define FSERROR(logger, txt, ...) Sys::FsLog(logger, ::DebugLevel::Error, false, txt, __VA_ARGS__)
	#define FSASSERT(logger, txt, ...) Sys::FsLog(logger, ::DebugLevel::Fatal, false, txt, __VA_ARGS__)
#else
	#define FSINFO(logger, txt, ...)	{}
	#define FSWARN(logger, txt, ...)	{}
	#define FSERROR(logger, txt, ...)	{}
	#define FSASSERT(logger, txt, ...)	{}
#endif

#if defined(ENABLE_FS_LOGWRAPPER) && defined(ENABLE_FS_SPAM)
	#define FSINFOSPAM(logger, txt, ...) Sys::FsLog(logger, ::DebugLevel::Info, true, txt, __VA_ARGS__)
	#define FSWARNSPAM(logger, txt, ...) Sys::FsLog(logger, ::DebugLevel::Warn, true, txt, __VA_ARGS__)
	#define FSERRORSPAM(logger, txt, ...) Sys::FsLog(logger, ::DebugLevel::Error, true, txt, __VA_ARGS__)
#else
	#define FSINFOSPAM(logger, txt, ...)	{}
	#define FSWARNSPAM(logger, txt, ...)	{}
	#define FSERRORSPAM(logger, txt, ...)	{}
#endif
}

#endif
