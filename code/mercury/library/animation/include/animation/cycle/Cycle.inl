#ifndef ANIMATION_CYCLE_INL
#define ANIMATION_CYCLE_INL

namespace Animation
{

	inline F32 C_Cycle::GetFrameRate() const
	{
		return m_frameRate;
	}

	inline F32 C_Cycle::GetDurationFrames() const
	{
		return m_playbackDuration;
	}

	inline F32 C_Cycle::GetDuration() const
	{
		return m_playbackDuration / m_frameRate;
	}

	inline Maths::Vector3 C_Cycle::GetLinearVelocity() const
	{
		return m_linearVelocity;
	}
	
	inline Maths::Vector3 C_Cycle::GetAngularVelocity() const
	{
		return m_angularVelocity;
	}

	inline const C_JointTransform& C_Cycle::GetFirstTrajectoryTransform() const
	{
		return m_trajectoryFirst;
	}

	inline const C_JointTransform& C_Cycle::GetFinalTrajectoryTransform() const
	{
		return m_trajectoryFinal;
	}
	
	inline const C_JointTransform& C_Cycle::GetReferenceTransform() const
	{
		return m_referenceTransform;
	}
	
	inline const C_StringId& C_Cycle::GetName() const
	{
		return m_name;
	}

	inline F32 C_Cycle::GetFramesFromSeconds( F32 timeInSeconds, E_LoopMode loopMode ) const
	{
		if (m_playbackDuration == 0.f) return 0.f;
	
		const F32 timeInFrames = timeInSeconds * GetFrameRate();
		F32 localTimeInFrames;
		
		switch ( loopMode )
		{
			default:
			case E_LoopMode_Continuous:
			{
				const F32 loopCount = floorf( timeInFrames / m_playbackDuration );
				localTimeInFrames = timeInFrames - m_playbackDuration * loopCount;
				break;
			}
						
			case E_LoopMode_Once:
				localTimeInFrames = Maths::Clamped( timeInFrames, 0.f, m_playbackDuration );
				break;
		}
		
		BIZ_ASSERT( localTimeInFrames < m_numFrames );
		return localTimeInFrames;
	}
	
	inline U32 C_Cycle::GetNumFields() const
	{
		return m_numFields;
	}
	
#ifndef PLATFORM_SPU
	inline const C_StringId& C_Cycle::GetFieldName( U32 fieldIndex ) const
	{
		return m_fieldNames[ fieldIndex ];
	}
	
	inline E_PoseFieldType C_Cycle::GetFieldType( U32 fieldIndex ) const
	{
		return static_cast< E_PoseFieldType >( m_fieldTypes[ fieldIndex ] );
	}
#endif

}

#endif
