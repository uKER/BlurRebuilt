#ifndef REPLAY_THREADING_H
#define REPLAY_THREADING_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Threads/Thread.h>

namespace Game
{
	class C_ReplaySaveManager;
}

namespace Game
{

/* 
===============================================================
					class C_ReplayThread

NOTES: Pure virtual class for threading replay functionality.
===============================================================
*/
class C_ReplayThread : public MT::Thread
{
public:

	C_ReplayThread();
	virtual ~C_ReplayThread();

	virtual void								Enter(void);
	void										Leave();
	bool										IsActive(void) const {return m_Active;};
	virtual void								ThreadMain() = 0;

protected:

	bool										m_Active;
};

/* 
===============================================================
					class C_ReplayLoadingThread

NOTES: Thread that loads the next replay scene from file
===============================================================
*/
class C_ReplayLoadingThread : public C_ReplayThread
{
public:
	C_ReplayLoadingThread();
	~C_ReplayLoadingThread();

private:
	void										ThreadMain();
};

/* 
===============================================================
					class C_ReplaySavingThread

NOTES: Thread that saves the last replay scene to file
===============================================================
*/
class C_ReplaySavingThread : public C_ReplayThread
{
public:
	C_ReplaySavingThread();
	~C_ReplaySavingThread();

	void										Enter(U32 p_NewSceneSaveTime);

private:
	void										ThreadMain();
	
	U32											m_NewSceneSaveTime;
};

/* 
===============================================================
				class C_ReplaySaveAndLoadThread

NOTES: Thread that saves the replay data, then loads the next scene
		Used for editing replay files (for cutscenes)
===============================================================
*/
class C_ReplaySaveAndLoadThread : public C_ReplayThread
{
public:
	C_ReplaySaveAndLoadThread();
	~C_ReplaySaveAndLoadThread();

	void										Enter(U32 p_NewSceneSaveTime);

private:
	void										ThreadMain();
	
	U32											m_NewSceneSaveTime;
};

} // namespace Game


#endif // REPLAY_THREADING_H