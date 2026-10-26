#include "stdafx.h"
#include <animation/cycle/CycleInstance.h>
#include <animation/cycle/Cycle.h>
#include <animation/pose/Pose.h>
//#include <animation/common/JointTransform.h>
//#include <animation/common/JointTransform.inl>
#include "CycleMapping.h"

namespace Animation
{

//BIZ_STATIC_ASSERT(sizeof(C_CycleInstance) == 64);

const U32 kInitialUpdateCounter = 0xdeadbeef;

#ifndef PLATFORM_SPU

C_CycleInstance::C_CycleInstance( const C_Cycle* cycle, const C_Pose* pose ):
	m_cycle( cycle ),
	m_cycleMapping( 0 ),
	m_duration( 0.f ),
	m_prevTime( 0.f ),
	m_prevUpdateCounter( kInitialUpdateCounter )
{
	if ( cycle )
	{
		m_duration = cycle->GetDuration();
		if (pose)
		{
			BindToPose(*pose);
		}
	}
}

C_CycleInstance::C_CycleInstance( const C_Cycle* cycle ):
	m_cycle( cycle ),
	m_cycleMapping( 0 ),
	m_duration( 0.f ),
	m_prevTime( 0.f ),
	m_prevUpdateCounter( kInitialUpdateCounter )
{
	if ( cycle )
	{
		m_duration = cycle->GetDuration();
	}
}

C_CycleInstance::C_CycleInstance(const C_CycleInstance& other):
	m_cycle(other.m_cycle),
	m_cycleMapping(other.m_cycleMapping),
	m_prevTrajectoryRotation(other.m_prevTrajectoryRotation),
	m_prevTrajectoryTranslation(other.m_prevTrajectoryTranslation),
	m_duration(other.m_duration),
	m_mappingSize(other.m_mappingSize),
	m_prevTime(other.m_prevTime),
	m_prevTimeInFrames(other.m_prevTimeInFrames),
	m_prevUpdateCounter(other.m_prevUpdateCounter)
{
	if (m_cycleMapping)
	{
		m_cycleMapping->AddRef();
	}
}

void C_CycleInstance::operator=(const C_CycleInstance& rhs)
{
	if (rhs.m_cycleMapping)
	{
		rhs.m_cycleMapping->AddRef();
	}
	ReleaseCycleMapping();

	m_cycle = rhs.m_cycle;
	m_cycleMapping = rhs.m_cycleMapping;
	m_prevTrajectoryRotation = rhs.m_prevTrajectoryRotation;
	m_prevTrajectoryTranslation = rhs.m_prevTrajectoryTranslation;
	m_duration = rhs.m_duration;
	m_mappingSize = rhs.m_mappingSize;
	m_prevTime = rhs.m_prevTime;
	m_prevTimeInFrames = rhs.m_prevTimeInFrames;
	m_prevUpdateCounter = rhs.m_prevUpdateCounter;
}

C_CycleInstance::~C_CycleInstance()
{
	ReleaseCycleMapping();
}
	
void C_CycleInstance::BindToPose( const C_Pose& pose )
{
	const Animation::C_CycleMapping* newMapping = 0;

	if ( m_cycle )
	{
		newMapping = m_cycle.RawPtr()->GetMapping( *pose.GetPoseLayout() );
	}

	ReleaseCycleMapping();

	m_cycleMapping = newMapping;
	if (newMapping)
	{
		m_mappingSize = newMapping->GetSize();
	}
}

void C_CycleInstance::Clear()
{
	m_cycle = NULL;
	ReleaseCycleMapping();
	m_duration = 0.f;
	m_prevTime = 0.f;
	m_prevUpdateCounter = kInitialUpdateCounter;
}

void C_CycleInstance::Create( const C_Cycle* cycle, const C_Pose* pose )
{
	m_cycle = cycle;
	m_prevTime = 0.f;
	m_prevUpdateCounter = kInitialUpdateCounter;
	m_duration = 0.f;
	
	if (cycle != NULL)
	{
		m_duration = cycle->GetDuration();
		
		// Check to see if we're binding to the same animation again.
		bool alreadyBound = false;
		if (m_cycleMapping && pose && cycle)
		{
			if (m_cycleMapping->GetCycle() == cycle &&
				m_cycleMapping->GetPoseLayout() == pose->GetPoseLayout())
			{
				alreadyBound = true;
			}
		}		
		
		if (!alreadyBound)
		{
			ReleaseCycleMapping();
			if (pose)
			{
				BindToPose(*pose);
			}
		}
	}
	else
	{
		ReleaseCycleMapping();
	}
}

#endif

C_StringId C_CycleInstance::GetCycleName() const
{
	return m_cycle ? m_cycle.RawPtr()->GetName() : C_StringId();
}

F32 C_CycleInstance::GetFrameRate() const
{
	return m_cycle ? m_cycle->GetFrameRate() : 0.f;
}

U32 C_CycleInstance::GetCycleMemoryUsage() const
{
	return m_cycle ? m_cycle->GetMemoryUsage() : 0;
}

Maths::Vector3 C_CycleInstance::GetLinearVelocity() const
{
	return m_cycle ? m_cycle.RawPtr()->GetLinearVelocity() : Maths::Const<Maths::Vector3>::Zero();
}

Maths::Vector3 C_CycleInstance::GetAngularVelocity() const
{
	return m_cycle ? m_cycle.RawPtr()->GetAngularVelocity() : Maths::Const<Maths::Vector3>::Zero();
}

C_JointTransform C_CycleInstance::GetFirstTrajectoryTransform() const
{
	return m_cycle ? m_cycle.RawPtr()->GetFirstTrajectoryTransform() : C_JointTransform(C_JointTransform::IDENTITY);
}

C_JointTransform C_CycleInstance::GetFinalTrajectoryTransform() const
{
	return m_cycle ? m_cycle.RawPtr()->GetFinalTrajectoryTransform() : C_JointTransform(C_JointTransform::IDENTITY);
}

C_JointTransform C_CycleInstance::GetReferenceTransform() const
{
	return m_cycle ? m_cycle.RawPtr()->GetReferenceTransform() : C_JointTransform(C_JointTransform::IDENTITY);
}

/// Fetch cycle and cycle mapping into local memory.
void C_CycleInstance::SpuFetchCycleAndMapping()
{
	Spu::FetchAlloc<const C_Cycle>( m_cycle, sizeof( C_Cycle ) );
	Spu::FetchAlloc<const C_CycleMapping>( m_cycleMapping, m_mappingSize );
}

#ifndef PLATFORM_SPU

void C_CycleInstance::ReleaseCycleMapping()
{
	if (m_cycleMapping)
	{
		m_cycleMapping->DecRef();
		m_cycleMapping = 0;
	}
	
	m_mappingSize = 0;
}

#endif

} // namespace Animation
