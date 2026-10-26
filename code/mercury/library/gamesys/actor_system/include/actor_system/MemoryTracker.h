#ifndef MEMORYTRACKER_80BFD2B0_8231_4A6F_9F60_0B13E514E637_H
#define MEMORYTRACKER_80BFD2B0_8231_4A6F_9F60_0B13E514E637_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>

#if ACTOR_SYSTEM_MEMORY_TRACKING

#include <system/memory/MemoryTracker.h>


namespace ActorSys
{

/// Uses auto_ptr<> style sematics.
class C_MemoryTracker
{
public:
	inline C_MemoryTracker() : m_Id(0) {}

	inline C_MemoryTracker(const C_MemoryTracker& copy) : m_Id(copy.m_Id) {copy.m_Id = 0;}

	inline explicit C_MemoryTracker(const Memory::ContextId id) : m_Id(id) {Push();}
	
	inline ~C_MemoryTracker() {Pop();}

	inline C_MemoryTracker& operator = (const C_MemoryTracker& copy)
	{
		if (&copy != this)
		{
			Pop();
			m_Id = copy.m_Id;
			copy.m_Id = 0;
		}
		return *this;
	}

private:
	mutable Memory::ContextId m_Id;

	inline void Push()
	{
		if (m_Id > 0)
		{
			Memory::TrackNodes::Instance().Push(m_Id);
		}
	}

	inline void Pop()
	{
		if (m_Id > 0)
		{
#ifdef CFG_DEBUG
			Memory::TrackNodes::Instance().Pop(m_Id);
#else
			Memory::TrackNodes::Instance().Pop();
#endif
			m_Id = 0;
		}
	}

};

} // namespace ActorSys


#endif // ACTOR_SYSTEM_MEMORY_TRACKING

#endif // MEMORYTRACKER_80BFD2B0_8231_4A6F_9F60_0B13E514E637_H
