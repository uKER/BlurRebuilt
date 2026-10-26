#ifndef LOGICALFS_LOG_FS_MANAGER_H
#define LOGICALFS_LOG_FS_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Utility/Singleton.h>
#include <Core/Threads/Mutex.h>
#include <Core/Threads/Misc.h>
#include <Btl/Containers/List.h>
#include <Btl/Containers/OpenHashMap.h>
#include <System/Debug/Log/Logger.h>
#include <System/Debug/Log/Subscribers/LogFileSubscriber.h>

namespace LogicalFs
{

#define LOGFS_PREINIT_MSG_MAX 100

class C_LogFsManager : public Ut::Singleton<C_LogFsManager, Ut::CreateStatic>
{
public:
	C_LogFsManager();

	struct C_LogMessage
	{
		C_LogMessage() {}
		C_LogMessage(const C_LogMessage& rhs)
		{
			*this = rhs;
		}
		const C_LogMessage& operator=(const C_LogMessage& rhs)
		{
			level = rhs.level;
			msg = rhs.msg;
			return *this;
		}

		DebugLevel::Level level;
		Btl::String msg;
	};
	typedef Btl::Vector<C_LogMessage> PreInitLogArray;

	struct C_RegFs
	{
		Btl::String32 name;
		const Sys::I_FileSystem* pFileSystem;

		C_RegFs(const char* name_, const Sys::I_FileSystem* pFileSys)
			: name(name_), pFileSystem(pFileSys){}
	};
	typedef Btl::List<C_RegFs*> RegFsList;

	const RegFsList& GetRegFsList() const;

	void Initialise(const Sys::C_FilePath& outputPath);

private:
	typedef Btl::Vector<Btl::String32, Btl::VectorParameters_Static<20> > EventStack;
	typedef Btl::OpenHashMap<MT::ThreadId, EventStack> PerThreadEventStack;

	C_LogFsManager(const C_LogFsManager& rhs);
	const C_LogFsManager& operator=(const C_LogFsManager rhs);

	void OnLogFsCreated(Sys::I_FileSystem* pFs, const char* name);
	void OnLogFsDestroy(Sys::I_FileSystem* pFs);
	void PushEvent(const char* name);
	void PopEvent();

	void LogMessage(const char* name, DebugLevel::Level level, char* format, ...);
	void LogFileAccess(const char* name, const Sys::C_FilePath& filePath);

	void HandleLogToDiskState();

	EventStack* AcquireThreadContextStack(MT::ThreadId id);

	//Keep a context stack per thread.
	PerThreadEventStack			m_perThreadEventStack;

	mutable MT::Mutex			m_mutex;
	RegFsList					m_regFsList;
	Debug::Logger&				m_logger;
	Debug::Logger&				m_accessLogger;

	Debug::LogFileSubscriber*	m_pLfSubscriber;
	Debug::LogFileSubscriber*	m_pAccessLfSubscriber;
	bool						m_initialised;
	PreInitLogArray				m_preInitMsgArray;
	PreInitLogArray				m_preInitAccessArray;
	bool						m_logToDisk;
	Sys::C_FilePath				m_outputPath;
	bool						m_openingLogFile;

	template<typename FILE_SYSTEM>
	friend class C_LogFs;
	friend class C_LogFsEvent;
};

}

#endif
