#ifndef ANIMATION_EVENTLOGGER_H
#define ANIMATION_EVENTLOGGER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Animation
{
	class C_Pose;
	class C_StringId;

	/**
		Tracks changes to boolean values.
		
		Values are derived from custom scalar curves whose values are assumed to lie
		in the range [0,1].
		
		There is a current limit of 32 event flags per event logger.
	*/
	class C_EventLogger
	{
	public:
		C_EventLogger();
			
		/// Create an event logger. If @a resetState is true then all current event flags are cleared, otherwise they are preserved.
		void Create( const C_Pose& pose, U32 numEvents, const C_StringId* eventNames, bool resetState = true );
		
		/// Get the status of the specified event flag.
		bool GetEventValue( U32 eventType ) const;
		
		/// Has the specified event flag changed since the last update?
		bool HasChanged( U32 eventType ) const;
		
		/// Has the specified event flag changed to true since the last update?
		bool HasChangedToTrue( U32 eventType ) const;
		
		/// Has the specified event flag changed to false since the last update?
		bool HasChangedToFalse( U32 eventType ) const;
		
		/// Update event flags.
		void Update( C_Pose& pose );
		
		/// Get number of events being tracked.
		U32 GetNumEvents() const;
		
		S32 GetEventFieldIndex( U32 eventType ) const;
		
	private:
		enum
		{
			E_MaxCycleEventTypes = 32
		};
		U32	m_valueBits;
		U32 m_changedBits;
		U8 m_numEvents : 7;
		U8 m_firstUpdate : 1;
		U8 m_eventFieldIndices[ E_MaxCycleEventTypes ];
	};
	
	inline bool C_EventLogger::GetEventValue( U32 eventType ) const
	{
		BIZ_ASSERT( eventType < E_MaxCycleEventTypes );
		return 0 != ( ( 1 << eventType ) & m_valueBits );
	}

	inline bool C_EventLogger::HasChanged( U32 eventType ) const
	{
		BIZ_ASSERT( eventType < E_MaxCycleEventTypes );
		return 0 != ( ( 1 << eventType ) & m_changedBits );
	}

	inline bool C_EventLogger::HasChangedToTrue( U32 eventType ) const
	{
		BIZ_ASSERT( eventType < E_MaxCycleEventTypes );
		const U32 eventFlag = 1 << eventType;
		return 0 != ( eventFlag & ( m_valueBits & m_changedBits ) );
	}

	inline bool C_EventLogger::HasChangedToFalse( U32 eventType ) const
	{
		BIZ_ASSERT( eventType < E_MaxCycleEventTypes );
		const U32 eventFlag = 1 << eventType;
		return 0 != ( eventFlag & ( ~m_valueBits & m_changedBits ) );
	}
	
	inline U32 C_EventLogger::GetNumEvents() const
	{
		return m_numEvents;
	}
}

#endif
