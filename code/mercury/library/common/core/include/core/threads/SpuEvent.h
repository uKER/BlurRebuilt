#ifndef __MERCURY_SYSTEM_SPU_EVENT_H__
#define __MERCURY_SYSTEM_SPU_EVENT_H__

#if (PLATFORM_SPU + PLATFORM_PS3)

#include <cell/spurs/types.h>

#if PLATFORM_PS3
#include <sys/types.h>
#include <cell/spurs/lv2_event_queue.h>
#else
typedef __CSTD uint32_t sys_event_queue_t;
typedef __CSTD uint32_t sys_event_port_t;
#endif

#if PLATFORM_SPU
#include <sys/return_code.h>
#elif PLATFORM_PS3
#include <errno.h>
#endif

namespace Scheduler
{
extern CellSpurs* jqSpurs;
};

namespace MT
{

/// Same as the standard Event class, except that it allows an SPU to send an event to the PPU
/// (the standard class only works with PPU)
class SpuEvent
{
#if PLATFORM_SPU
private:
	Event();	// can't construct these on the SPU
#endif
public:
#if PLATFORM_PS3
	/// constructor
	SpuEvent(const char *pszName = NULL)
	{
		// Create an event queue
		sys_event_queue_attribute_t	attr;
		sys_event_queue_attribute_initialize(attr);
#ifndef CFG_GOLD
		if (pszName)
			sys_event_queue_attribute_name_set(attr.name, pszName);
#endif
		// create event queue
		int rtn = sys_event_queue_create(&m_iEventQueue, &attr, SYS_EVENT_QUEUE_LOCAL, 2);
		assert(rtn == CELL_OK);

		// Attach the event queue to LV2
		rtn = cellSpursAttachLv2EventQueue(Scheduler::jqSpurs, m_iEventQueue, &m_iPort, 1);
		assert(rtn == CELL_OK);

		// create event port
		rtn = sys_event_port_create(&m_iEventPort, SYS_EVENT_PORT_LOCAL, SYS_EVENT_PORT_NO_NAME);
		assert(rtn == CELL_OK);

		// Attach event port to queue
		rtn = sys_event_port_connect_local(m_iEventPort, m_iEventQueue);
		assert(rtn == CELL_OK);
	}

	/// destructor
	~SpuEvent()
	{
		// Clean up event queue.
		int rtn = cellSpursDetachLv2EventQueue(Scheduler::jqSpurs, m_iPort);
		assert(rtn == CELL_OK);
		rtn = sys_event_port_disconnect(m_iEventPort);
		assert(rtn == CELL_OK);
		rtn = sys_event_queue_destroy(m_iEventQueue, 0);
		assert(rtn == CELL_OK);
		rtn = sys_event_port_destroy(m_iEventPort);
		assert(rtn == CELL_OK);
	}
#endif

	/// signal the event
    void Signal()
	{
#if PLATFORM_SPU
		int rtn = sys_spu_thread_send_event(m_iPort, 0, 0);
		assert(rtn == CELL_OK || rtn == EBUSY);
#else
		int rtn = sys_event_port_send(m_iEventPort, 0, 0, 0);
		assert(rtn == CELL_OK || rtn == EBUSY);
#endif
	}

#if PLATFORM_PS3
	/// wait for the event to be signaled
    bool Wait( const U32 timeMs = WAIT_TIME_INFINITE )
	{
		sys_event_t event;
		int rtn = sys_event_queue_receive(m_iEventQueue, &event, (usecond_t)timeMs*1000);
		return rtn == CELL_OK;
	}
#endif

private:
	U8 m_iPort;							// port for the SPU to signal the event
	sys_event_queue_t	m_iEventQueue;	// the queue for the PPU to receive the event
	sys_event_port_t	m_iEventPort;	// port for the PPU to signal the event
};

} // namespace MT

#else

// on other platforms SpuEvent is the same as Event
#include "core/threads/Event.h"
namespace MT
{
typedef Event SpuEvent;
}

#endif

#endif
