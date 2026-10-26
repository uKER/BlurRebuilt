#include "stdafx.h"
#include <GameLogic\Replays\ReplayThreading.h>
#include <GameLogic\Replays\ReplaySaveManager.h>
#include <GameLogic\Replays\ReplayDirector.h>

namespace Game
{

C_ReplayThread::C_ReplayThread() : m_Active(false)
{
}

C_ReplayThread::~C_ReplayThread()
{
	Leave();
}

void C_ReplayThread::Enter()
{
	if (!IsActive())
	{
		m_Active = true;
		Start(0, 0, "Loading next replay scene from file");
	}
}

void C_ReplayThread::Leave()
{
	if(!IsActive())
	{
#if defined( COMPILER_VC )
		Close();
#endif
	}
}

C_ReplayLoadingThread::C_ReplayLoadingThread()
{
}

C_ReplayLoadingThread::~C_ReplayLoadingThread()
{
}

void C_ReplayLoadingThread::ThreadMain()
{
	C_ReplayDirector::Instance().GetSaveManager()->LoadNewReplayScene();

	m_Active = false;

	Leave();
}

C_ReplaySavingThread::C_ReplaySavingThread()
{
}

C_ReplaySavingThread::~C_ReplaySavingThread()
{
}

void C_ReplaySavingThread::Enter(U32 p_NewSceneSaveTime)
{
	if (!IsActive())
	{
		m_Active = true;
		m_NewSceneSaveTime = p_NewSceneSaveTime;
		Start(0, 0, "Loading next replay scene from file");
	}
}

void C_ReplaySavingThread::ThreadMain()
{
	C_ReplayDirector::Instance().GetSaveManager()->WriteNewReplayScene(m_NewSceneSaveTime);

	m_Active = false;

	Leave();
}


C_ReplaySaveAndLoadThread::C_ReplaySaveAndLoadThread()
{
}

C_ReplaySaveAndLoadThread::~C_ReplaySaveAndLoadThread()
{
}

void C_ReplaySaveAndLoadThread::Enter(U32 p_NewSceneSaveTime)
{
	if (!IsActive())
	{
		m_Active = true;
		m_NewSceneSaveTime = p_NewSceneSaveTime;
		Start(0, 0, "Loading next replay scene from file");
	}
}

void C_ReplaySaveAndLoadThread::ThreadMain()
{
	C_ReplayDirector::Instance().GetSaveManager()->WriteNewReplayScene(m_NewSceneSaveTime);

	C_ReplayDirector::Instance().GetSaveManager()->LoadNewReplayScene();

	m_Active = false;

	Leave();
}

} // namespace Game