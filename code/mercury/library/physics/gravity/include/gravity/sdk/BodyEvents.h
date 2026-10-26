#ifndef BODY_EVENTS_H
#define BODY_EVENTS_H

#include "gravity/SharedPhysics/MemoryAllocator.h"

namespace Gravity
{
	class C_Body;

	class C_BodyEvent
	{
	public:
		enum E_BodyEventType
		{
			E_WakeUp,
			E_GoToSleep,
			E_RemoveConstraint,
			E_EnterWater,
			E_ExitWater
		};

		C_BodyEvent()
		{
		}

		void SetBody(C_Body* pBody)
		{
			m_pBody = pBody;
		}

		void SetEventType(E_BodyEventType ev)
		{
			m_EventType = ev;
		}

		C_Body* GetBody() const
		{
			return m_pBody;
		}

		void SetConstraintUserData(void* pConstraint)
		{
			m_pConstraint = pConstraint;
		}

		void* GetConstraintUserData() const
		{
			return m_pConstraint;
		}

		const E_BodyEventType GetEventType() const
		{
			return m_EventType;
		}

	private:
		union
		{
			C_Body* m_pBody;
			void* m_pConstraint;
		};
		E_BodyEventType m_EventType;
		//C_TriggerBox* m_pTriggerEvent;
	};

	class C_EventStream
	{
		SharedPhysics::MemoryAllocator<C_BodyEvent> m_Events;
		SharedPhysics::C_MemoryIterator<C_BodyEvent> m_EventIterator;
		U32 m_NumEvents;
	public:
		C_EventStream() : m_EventIterator(&m_Events)
		{
		}

		void Reset()
		{
			m_EventIterator.Reset();
			m_NumEvents = 0;
		}

		C_BodyEvent* CreateEvent()
		{
			m_NumEvents++;
			return m_EventIterator.Create();
		}

		SharedPhysics::C_MemoryIterator<C_BodyEvent> GetIterator()
		{
			return SharedPhysics::C_MemoryIterator<C_BodyEvent>(&m_Events);
		}

		const U32 NumEvents() const
		{
			return m_NumEvents;
		}
	};

}

#endif

