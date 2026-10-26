#ifndef ANIMATION_POSEFIELDMASK_INL
#define ANIMATION_POSEFIELDMASK_INL

namespace Animation
{

	inline C_PoseFieldMask::C_PoseFieldMask():
		m_mask((U16)~0)
	{
	}
		
	inline C_PoseFieldMask& C_PoseFieldMask::EnableAll( bool enable )
	{
		m_mask = enable ? ~0 : 0;
		return *this;
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::DisableAll()
	{
		EnableAll( false );
		return *this;
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::EnableTrajectory( bool enable )
	{
		const U32 kTrajectoryFlags =
			  (1 << E_PoseFieldElement_TrajectoryTransformRotation)
			| (1 << E_PoseFieldElement_TrajectoryTransformTranslation)
			| (1 << E_PoseFieldElement_TrajectoryDeltaTransformRotation)
			| (1 << E_PoseFieldElement_TrajectoryDeltaTransformTranslation);
			
		if ( enable )
			m_mask |= kTrajectoryFlags;
		else
			m_mask &= ~kTrajectoryFlags;
			
		return *this;
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::DisableTrajectory()
	{
		return EnableTrajectory( false );
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::EnableCustomScalars( bool enable )
	{
		const U32 kCustomScalarFlags =
			( 1 << E_PoseFieldElement_CustomScalar);
		if ( enable )
			m_mask |= kCustomScalarFlags;
		else
			m_mask &= ~kCustomScalarFlags;
		
		return *this;
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::DisableCustomScalars()
	{
		return EnableCustomScalars( false );
	}

	inline C_PoseFieldMask& C_PoseFieldMask::Enable( E_PoseFieldElement elemType, bool enable )
	{
		U32 flag = 1 << elemType;
		if ( enable )
			m_mask |= flag;
		else
			m_mask &= ~flag;
		return *this;
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::Disable( E_PoseFieldElement elemType )
	{
		return Enable( elemType, false );
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::Enable( E_PoseFieldType fieldType, bool enable )
	{
		U32 flags = kFieldTypeMasks[fieldType];
		if ( enable )
			m_mask |= flags;
		else
			m_mask &= ~flags;
		return *this;
	}
	
	inline C_PoseFieldMask& C_PoseFieldMask::Disable( E_PoseFieldType fieldType )
	{
		m_mask &= ~kFieldTypeMasks[fieldType];
		return *this;
	}	
	
	inline bool C_PoseFieldMask::IsEnabled( E_PoseFieldElement elemType ) const
	{
		return 0 != (m_mask & (1 << elemType));
	}
	
	inline bool C_PoseFieldMask::IsEnabled(Animation::E_PoseFieldType fieldType) const
	{
		U32 fieldTypeMask = kFieldTypeMasks[fieldType];
		return fieldTypeMask == (m_mask & fieldTypeMask);
	}
	
	//
	// Internal functions.
	//

	inline U16 C_PoseFieldMask::AsU16() const
	{
		return m_mask;
	}
	
	inline C_PoseFieldMask::C_PoseFieldMask( U16 mask ):
		m_mask(mask)
	{
	}

}

#endif
