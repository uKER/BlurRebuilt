#ifndef ANIMATION_SKELETONCONSTRAINTSDESERIALISE_INL
#define ANIMATION_SKELETONCONSTRAINTSDESERIALISE_INL

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_SPU

namespace Maths
{
	// Temporary deserialise functions.
	template <typename T>
	inline void Deserialise( T& in, Maths::Vector3& v )
	{
		Maths::Deserialise( in, &v );
	}
	
	template <typename T>
	inline void Deserialise( T& in, Maths::Normal3& v )
	{
		Maths::Deserialise( in, &v );
	}	
}


#include <btl/utility/types.h>
#include <animation/common/MemoryBlock.h>
#include <animation/skeleton/StaticJointConstraints.h>
#include <animation/skeleton/DynamicJointConstraints.h>
#include <system/memory/MemoryTracker.h>

namespace Animation
{

	#define kSkeletonConstraintsMemorySubTag	"constraints"
	
	template <typename T>
	inline void C_SkeletonConstraints::Deserialise( T& in )
	{
		MEMORY_TRACKER_PUSH_SUB( kSkeletonConstraintsMemorySubTag );
		
		// Free existing constraints.
		Clear();
		
		U16 version;
		
		in >> version;
		
		BIZ_ASSERT( version >= 1 );
		
		// Read number of constraints and driven keys.
		in >> m_numConstraints;
		in >> m_numDrivenKeys;
		
		
		// Read number of constraints of each type.
		for ( U32 i = 0; i < E_ConstraintType_Count; ++i )
		{
			in >> m_numConstraintsOfType[ i ];
		}
		
		C_MemoryBlockSizer sizer;
		
		sizer.Allocate( m_constraints, m_numConstraints );
		sizer.Allocate( m_aimConstraints, m_numConstraintsOfType[ E_ConstraintType_Aim ] );
		sizer.Allocate( m_orientationConstraints, m_numConstraintsOfType[ E_ConstraintType_Orientation ] );
		sizer.Allocate( m_positionConstraints, m_numConstraintsOfType[ E_ConstraintType_Position ] );
		sizer.Allocate( m_drivenKeys, m_numDrivenKeys );
		sizer.Allocate( m_drivenKeyRotationConstraints, m_numConstraintsOfType[ E_ConstraintType_DrivenKeyRotation ] );
		sizer.Allocate( m_drivenKeyTranslationConstraints, m_numConstraintsOfType[ E_ConstraintType_DrivenKeyTranslation ] );
		
		sizer.Allocate( m_dynamicAimJointParams, m_numConstraintsOfType[ E_ConstraintType_DynamicAimJoint ] );
		sizer.Allocate( m_dynamicSliderJointParams, m_numConstraintsOfType[ E_ConstraintType_DynamicSliderJoint ] );
		
		m_constraintData.Create( sizer );
		
		// Read sorted constraint array, <type, relative index> for each constraint.
		for ( U32 i = 0; i < m_numConstraints; ++i )
		{
			U8 type;
			in >> type;
			U8 index;
			in >> index;
			m_constraints(m_constraintData)[ i ].type = type;
			m_constraints(m_constraintData)[ i ].index = index;
		}
		
		// Deserialise driven keys.
		for ( U32 i = 0; i < m_numDrivenKeys; ++i )
		{
			in >> m_drivenKeys(m_constraintData)[ i ];
		}
		
		// Deserialise constraints by type.
		
		for ( U32 i = 0; i < m_numConstraintsOfType[ E_ConstraintType_Aim ]; ++i )
		{
			in >> m_aimConstraints(m_constraintData)[ i ];
		}
		
		for ( U32 i = 0; i < m_numConstraintsOfType[ E_ConstraintType_Orientation ]; ++i )
		{
			in >> m_orientationConstraints(m_constraintData)[ i ];
		}

		for ( U32 i = 0; i < m_numConstraintsOfType[ E_ConstraintType_Position ]; ++i )
		{
			in >> m_positionConstraints(m_constraintData)[ i ];
		}

		for ( U32 i = 0; i < m_numConstraintsOfType[ E_ConstraintType_DrivenKeyRotation ]; ++i )
		{
			in >> m_drivenKeyRotationConstraints(m_constraintData)[ i ];
		}
		
		for ( U32 i = 0; i < m_numConstraintsOfType[ E_ConstraintType_DrivenKeyTranslation ]; ++i )
		{
			in >> m_drivenKeyTranslationConstraints(m_constraintData)[ i ];
		}
		
		// (custom joint data would go here).

		for ( U32 i = 0; i < m_numConstraintsOfType[ E_ConstraintType_DynamicAimJoint ]; ++i )
		{
			m_dynamicAimJointParams(m_constraintData)[i].Deserialise(in, version);
		}

		for ( U32 i = 0; i < m_numConstraintsOfType[ E_ConstraintType_DynamicSliderJoint ]; ++i )
		{
			m_dynamicSliderJointParams(m_constraintData)[ i ].Deserialise(in, version);
		}
		
		// Done.
		MEMORY_TRACKER_POP_SUB( kSkeletonConstraintsMemorySubTag );
	}
	
	#undef kSkeletonConstraintsMemorySubTag
}
#endif // PLATFORM_SPU


#endif
