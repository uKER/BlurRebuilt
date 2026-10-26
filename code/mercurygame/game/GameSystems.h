#ifndef GAMESYSTEMS_H
#define GAMESYSTEMS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <framework/application/CoreSystems.h>

namespace LogicalFs
{
	class C_OverlayFs;
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
	static C_GameSystems* Instance();

	//! Initialises the core sytems.
	virtual HRESULT Initialise(const Framework::C_CoreSystemsParams& params);

	//! Shutsdown the core sytems.
	virtual void Shutdown();

	//! Updates the core sytems.
	virtual void Update();

	C_SchedulerConfig& GetSchedulerConfig();

private:
	C_SchedulerConfig*				m_pSchedulerConfig;
	LogicalFs::C_OverlayFs*			m_pFileSystem;
	static C_GameSystems*			s_pInstance;
};

} // namespace Game

#endif // GAMESYSTEMS_H
