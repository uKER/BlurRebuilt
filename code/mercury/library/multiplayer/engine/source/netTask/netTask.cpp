
#include "stdafx.h"
#include "./netTask.h"
#include "netDebug/netDebug.h"
#include "netEnums/netEnums.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

netTask::netTask(int id, const char* name)
: m_id(id)
, m_name(name)
, m_state(STATE_NOT_SET)
, m_status(STATUS_NOT_SET)
, dbg_state(STATE_NOT_SET-1)
{}

netTask::~netTask()
{
	RemoveAllSubscribers();
}

void netTask::addTask(netTaskRef task)
{
	logInfo("adding task '%s'", task->name());
	m_subtasks.addTail(task);
}

bool netTask::poll()
{
	if(finished())
		return false;

	if(!m_subtasks.isEmpty())
	{
		netTaskRef task = m_subtasks.getHead();
		
		task->poll();

		if(task->finished())
		{
			m_subtasks.removeHead();
			onSubTaskFinished(task);
		}
		if(!m_subtasks.isEmpty())
			return true;
	}

	switch(m_state)
	{
	default:
		{
			logFatal("unknown state (%d)", m_state);
			break;
		}
	case STATE_FINISHED:
		{
			break;
		}
	case STATE_NOT_SET:
		{
			if(dbg_state != m_state)
				logInfo("initialising.");

			if(!init()) return false;
			if(finished()) return false;
			logInfo("initialised.");
			m_state = STATE_FIRST_UPDATE;
			break;
		}
	case STATE_FIRST_UPDATE:
		{
			if(dbg_state != m_state)
				logInfo("first update.");

			if(!update()) return false;
			if(finished())return false;
			logInfo("updating...");
			m_state = STATE_UPDATING;
			break;
		}
	case STATE_UPDATING:
		{
			if(!update()) return false;
			if(finished()) return false;
			break;
		}
	}
	dbg_state = m_state;
	return true;
}

bool netTask::abort()
{
	return end(STATUS_ABORTED);
}

bool netTask::onSubTaskFinished(netTaskRef task)
{
	if(task->failed())
		return end(task->status());

	return true;
}

bool netTask::end(int status)
{
	if(status != STATUS_OK)
		logError("ended error(%d).", status);
	else
		logInfo("ended ok.");
		
	m_state = STATE_FINISHED;
	m_status = status;
	return true;
}

netTaskRef netTask::currentSubTask()
{
	if(m_subtasks.isEmpty()) return NULL;
	return (m_subtasks.getHead());
}

netTaskRef netTask::findSubTask(int id)
{
	bdLinkedList<netTaskRef>::Position pos = m_subtasks.getHeadPosition();
	while(pos)
	{
		// current subtask is what we are looking for
		netTaskRef task = m_subtasks.forward(pos);
		if(task->id() == id)
			return  task;

		// find sub task matching the id
		netTaskRef subTask = task->findSubTask(id);
		if(subTask.notNull())
			return subTask;
	}
	return NULL;
}

void netTask::logError(const char* format, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, sizeof(buffer), format, args);
	va_end(args);
	LOGERROR(netDebug::g_Log, "<%s> %s", name(), buffer); 
}

void netTask::logWarn(const char* format, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, sizeof(buffer), format, args);
	va_end(args);
	LOGWARN(netDebug::g_Log, "<%s> %s", name(), buffer); 
}

void netTask::logFatal(const char* format, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, sizeof(buffer), format, args);
	va_end(args);
	LOGFATAL(netDebug::g_Log, "<%s> %s", name(), buffer); 
}

void netTask::logInfo(const char* format, ...)
{
	char buffer[512];
	va_list args;
	va_start(args, format);
	vsprintf_s(buffer, sizeof(buffer), format, args);
	va_end(args);
	LOGINFO(netDebug::g_Log, "<%s> %s", name(), buffer); 
}


netState::netState(int id, const char* name)
: netTask(id, name)
{
	m_nextState = NEXTSTATE_NOT_SET;
}

netState::~netState()
{}

bool netState::end(int status, int nextState)
{
	m_nextState = nextState;
	return netTask::end(status);	
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

