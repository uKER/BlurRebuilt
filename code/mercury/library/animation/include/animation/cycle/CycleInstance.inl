#ifndef ANIMATION_CYCLEINSTANCE_INL
#define ANIMATION_CYCLEINSTANCE_INL

#include <animation/common/JointTransform.inl>

namespace Animation
{

#ifndef PLATFORM_SPU
	inline C_CycleInstance::C_CycleInstance():
		m_cycle( 0 ),
		m_cycleMapping( 0 ),
		m_mappingSize( 0 ),
		m_duration( 0.f )
	{
	}
#endif
	
	inline bool C_CycleInstance::IsValid() const
	{
#ifdef PLATFORM_SPU
		return m_cycle.EA != 0;
#else
		return m_cycle != 0;
#endif
	}
		
	inline bool C_CycleInstance::IsBound() const
	{
#ifdef PLATFORM_SPU
		return m_cycleMapping.EA != 0;
#else
		return m_cycleMapping != 0;
#endif
	}
	
	inline F32 C_CycleInstance::GetDuration() const
	{
		return m_duration;
	}
	
	inline const C_CycleMapping* C_CycleInstance::GetCycleMapping() const
	{
		return m_cycleMapping;
	}
	
	inline C_JointTransform C_CycleInstance::GetPreviousTrajectoryTransform() const
	{
		Maths::Vec4F32 q = Maths::Load<Maths::Vec4F32, MathsStorage::Float4>(&m_prevTrajectoryRotation);
		Maths::Vec4F32 t = Maths::Load<Maths::Vec4F32, MathsStorage::Float3>(&m_prevTrajectoryTranslation);
		
		return C_JointTransform( Maths::MakeQuaternion(q), Maths::MakeVector3(t) );
	}
	
	inline void C_CycleInstance::SetPreviousTrajectoryTransform( const C_JointTransform& transform )
	{
		Maths::Vec4F32 q = Maths::GetVec4F32(transform.GetRotation());
		Maths::Vec4F32 t = Maths::GetVec4F32(transform.GetTranslation());
		Maths::Store(q, &m_prevTrajectoryRotation);
		Maths::Store(t, &m_prevTrajectoryTranslation);
	}
	
	inline U32 C_CycleInstance::GetPreviousUpdateCounter() const
	{
		return m_prevUpdateCounter;
	}

	inline void C_CycleInstance::SetPreviousUpdateCounter( U32 currentCounter )
	{
		m_prevUpdateCounter = currentCounter;
	}
	
	inline F32 C_CycleInstance::GetPreviousTime() const
	{
		return m_prevTime;
	}
	
	inline F32 C_CycleInstance::GetPreviousTimeInFrames() const
	{
		return m_prevTimeInFrames;
	}
	
	inline void C_CycleInstance::SetPreviousTime( F32 time, F32 timeInFrames )
	{
		m_prevTime = time;
		m_prevTimeInFrames = timeInFrames;
	}
	
		
}

#endif
