#ifndef FRAMEWORK_CORE_SYSTEMS_H
#define FRAMEWORK_CORE_SYSTEMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System/Serialise/I_FileSystem.h>

#include "Framework/Application/ApplicationTypes.h"

#if defined(USER_LLWRIGHT)
//#define GAMESYS_USE_DEBUGLOG
#endif

namespace LogicalFs
{
	class C_MemoryFs;
}

namespace Resource
{
	class C_Manager;
}

namespace Debug
{
	class LogFileSubscriber;
}

#ifdef PLATFORM_PS3
namespace PlatformPS3
{
	extern char g_ps3_titleId[];
}
#endif

namespace Framework
{

//! C_CoreSystemsParams. Params used to initialise the core sytems.


class C_CoreSystemsParams
{
public:

	//! Constructor.
	C_CoreSystemsParams();

	//! Destructor.
	~C_CoreSystemsParams();

public:
	Util::C_String					m_Name;
	Sys::C_FilePath					m_WorkingDir;
};

//! C_CoreSystems. Sets up debug loggers, starts artery, updates memory tracker etc.

class C_CoreSystems
{
public:

	//! Constructor.
	C_CoreSystems();

	//! Destructor.
	virtual ~C_CoreSystems();

	//! Has the core sytems been initialised?
	const bool IsInitialised() const;

	//! Returns the name.
	const Util::C_String& GetName() const;

	//! Has the network already been initialised by artery.
	bool NetworkInitialised(void) const;

	//! Initialises the core sytems.
	virtual HRESULT Initialise(const C_CoreSystemsParams& params);

	//! Shutsdown the core sytems.
	virtual void Shutdown();

	//! Updates the core sytems.
	virtual void Update();

	//! Returns a pointer to the core sytems instance.
	static C_CoreSystems& Instance()
	{
		BIZ_ASSERT(s_pInstance);
		return *s_pInstance;
	}

protected:
		//! Startsup artery.
	HRESULT StartupArtery(const C_CoreSystemsParams& params);

	//! Shutsdown artery.
	void ShutdownArtery();

protected:
	void SetFileSystems(Sys::I_FileSystem* pFileSystem, LogicalFs::C_MemoryFs* pMemoryFs);

	bool							m_Initialised;
	bool							m_ArteryStartupInitialisedNetwork;
	Util::C_String					m_Name;
	#ifdef GAMESYS_USE_DEBUGLOG
	Debug::LogFileSubscriber*		m_pLogFileSubscriber;
	#endif
	Resource::C_Manager*			m_pResourceManager;
	static C_CoreSystems*			s_pInstance;

private:
	Sys::I_FileSystem*				m_pFileSystem;
	LogicalFs::C_MemoryFs*			m_pMemoryFs;
};

}; // namespace Framework

#endif // FRAMEWORK_CORE_SYSTEMS_H
