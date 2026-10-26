#include "stdafx.h"
#include <animation/pose/EventLogger.h>
#include <animation/pose/Pose.h>
#include "pose/Pose.inl"
#include <animation/pose/PoseFields.h>
#include <animation/common/StringId.h>

namespace Animation
{

C_EventLogger::C_EventLogger():
	m_valueBits(0),
	m_changedBits(0),
	m_numEvents(0),
	m_firstUpdate(1)
{
}

#ifndef PLATFORM_SPU
void C_EventLogger::Create( const C_Pose& pose, U32 numEvents, const C_StringId* eventNames, bool resetState )
{
	BIZ_ASSERT( numEvents < 256 );
		
	for ( U32 i = 0; i < numEvents; ++i )
	{
		S32 index = pose.FindCustomScalarByName( eventNames[ i ] );
		
		BIZ_ASSERT( index < 256 && "Event index unexpectedly large" );
		
		if ( index >= 0 )
		{
			m_eventFieldIndices[ i ] = index + 1;
		}
		else
		{
			m_eventFieldIndices[ i ] = 0;
			//LOGWARN( Debug::GetLogger("mercury.animation"), "Event '%s' not found in pose layout\n", eventNames[ i ].GetString() );
		}
	}
	
	m_numEvents = numEvents;
	
	if (resetState)
	{
		m_firstUpdate = 1;
		m_changedBits = 0;
		m_valueBits = 0;
	}
}
#endif

void C_EventLogger::Update( C_Pose& pose )
{
	U32 newValueBits = 0;
	
	for ( U32 i = 0, count = m_numEvents, eventFlag = 1; i < count; ++i, eventFlag <<= 1 )
	{
		const U32 fieldIndex = m_eventFieldIndices[ i ];
		if ( fieldIndex != 0 )
		{
			const F32 value = pose.GetCustomScalar( fieldIndex - 1 );
		
			if ( value > 0.5f )
			{
				newValueBits |= eventFlag;
			}
		}
	}
	
	if ( !m_firstUpdate )
	{
		m_changedBits = newValueBits ^ m_valueBits;
	}
	else
	{
		m_changedBits = 0;
		m_firstUpdate = 0;
	}
	m_valueBits = newValueBits;
}

S32 C_EventLogger::GetEventFieldIndex( U32 eventType ) const
{
	BIZ_ASSERT( eventType < E_MaxCycleEventTypes );
	return m_eventFieldIndices[ eventType ] - 1;
}

} // end Animation
