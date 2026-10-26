#ifndef ANIMATION_POSE_INL
#define ANIMATION_POSE_INL

#ifdef _MSC_VER
#pragma once
#endif

//
// Private inline functions.
//

#include <animation/pose/PoseLayout.h>

namespace Animation
{
	inline C_QuaternionField* C_Pose::GetQuaternionFieldElement( E_PoseFieldElement element, U32 fieldIndex )
	{
		return static_cast<C_QuaternionField*>( GetFieldElement( element, fieldIndex ) );
	}
	
	inline C_Vector3Field* C_Pose::GetVector3FieldElement( E_PoseFieldElement element, U32 fieldIndex )
	{
		return static_cast<C_Vector3Field*>( GetFieldElement( element, fieldIndex ) );
	}
	
	inline C_ScalarField* C_Pose::GetScalarFieldElement( E_PoseFieldElement element, U32 fieldIndex )
	{
		return static_cast<C_ScalarField*>( GetFieldElement( element, fieldIndex ) );
	}

	inline void* C_Pose::GetFieldElement( E_PoseFieldElement element, U32 fieldIndex )
	{
		const C_ElementInfo& info = s_elementInfo[ element ];
		const U32 fieldOffset = m_poseLayout.RawPtr()->GetFieldOffset( info.fieldType, fieldIndex ) + info.offset;
		C_RelativePtr<void> ptr( fieldOffset );
		return ptr(m_data);
	}

	inline void* C_Pose::GetPoseData()
	{
		return &m_data[0];
	}
	
}

#endif
