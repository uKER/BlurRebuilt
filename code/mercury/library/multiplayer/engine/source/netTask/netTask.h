#ifndef NETTASK_H
#define NETTASK_H

#ifdef _MSC_VER
#pragma once
#endif


#include <bdCore/bdReference/bdReference.h>
#include <bdCore/bdReference/bdReferencable.h>
#include <bdCore/bdContainers/bdLinkedList.h>
#include <misc_shared/Messaging/MessageMultiHandler.h>

BD_REFERENCE(netTask);
class netTask: public bdReferencable, public Messaging::C_MessageMultiHandler
{
public:
	netTask(int id, const char* name);
	virtual ~netTask();
	
	void addTask(netTaskRef task);
	bool poll();

	int id() const { return m_id; }
	const char* name() const { return m_name; }

	netTaskRef findSubTask(int id);
	netTaskRef currentSubTask();

	bool MessageHandler(const Messaging::C_MessageBase* pMessage) { return true; }
	
	bool  pending() const { return m_state == STATE_NOT_SET;  }
	bool finished() const { return m_state == STATE_FINISHED; }
	bool  success() const { return m_state == STATE_FINISHED && m_status == STATUS_OK;    }
	bool   failed() const { return m_state == STATE_FINISHED && m_status >= STATUS_ERROR; }
	int	   status() const { return m_status; }
	int	    state() const { return m_state;  }

	virtual bool abort();
	
protected:
	enum
	{
		STATUS_NOT_SET=-1,
		STATUS_OK,
		STATUS_ERROR,
		STATUS_ABORTED,
	};
	enum
	{
		STATE_FINISHED=-2,
		STATE_NOT_SET=-1,
		STATE_FIRST_UPDATE=0,
		STATE_UPDATING,
	};

	virtual bool onSubTaskFinished(netTaskRef task);
	virtual bool update(){ return true; }
	virtual bool init(){ return true; }
	virtual bool end(int status);

	virtual void logError(const char* format, ...);
	virtual void logWarn(const char* format, ...);
	virtual void logFatal(const char* format, ...);
	virtual void logInfo(const char* format, ...);

	bdLinkedList<netTaskRef> m_subtasks;

	int m_state;
	int m_status;
	int dbg_state;
	
	const int m_id;
	const char* m_name;
};

BD_REFERENCE(netState);
class netState: public netTask
{
public:
	netState(int id, const char* name);
	virtual ~netState();
	virtual bool end(int status, int nextState);

	int	nextState() const { return m_nextState; }
	
public:
	enum { NEXTSTATE_NOT_SET = -1 };
	int m_nextState;
};


#endif //NETTASK_H