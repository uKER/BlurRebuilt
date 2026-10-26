#ifndef GAMESYSTEMS_H
#define GAMESYSTEMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <framework/application/CoreSystems.h>

namespace LogicalFs
{
	class C_OverlayFs;
	class C_MemoryFs;
}

namespace Game
{

class C_SchedulerConfig;

//! C_GameSystems
class C_GameSystems : public Framework::C_CoreSystems
{
public:
	//! Constructor.
	C_GameSystems();

	//! Destructor.
	virtual ~C_GameSystems();

	//! Returns a pointer to the core systems instance.
	static C_GameSystems& Instance();

	//! Initialises the core sytems.
	virtual HRESULT Initialise(const Framework::C_CoreSystemsParams& params);

	//! Shutsdown the core sytems.
	virtual void Shutdown();

	//! Updates the core sytems.
	virtual void Update();

	void CreateFsLayers();

#ifdef PLATFORM_PS3
	void RemountPakLayers();
#endif

	void MountPaksByLanguageCode(const char* pLanguageCode);

	LogicalFs::C_MemoryFs* GetMemoryFs();

	C_SchedulerConfig& GetSchedulerConfig();

	void UnloadGamedataRampak();

private:
	C_SchedulerConfig*				m_pSchedulerConfig;
	LogicalFs::C_OverlayFs*			m_pFileSystem;
	LogicalFs::C_MemoryFs*			m_pMemoryFileSystem;
	static C_GameSystems*			s_pInstance;
};

} // namespace Game

#endif // GAMESYSTEMS_H
