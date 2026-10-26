#ifndef ANIMATION_POSEFIELDMASK_H
#define ANIMATION_POSEFIELDMASK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <animation/pose/PoseFieldTypes.h>

namespace Animation
{
	/**
	Identifies which pose fields are required for evaluation or other operations.
	
	To simplify construction of temporary C_PoseFieldMask objects passed as parameters, each
	Enable and Disable method returns a reference to the current C_PoseFieldMask. For example,
	a temporary C_PoseFieldMask which disables evaluation of the trajectory transform
	and custom scalar fields can be created like this:
	
	@code
	C_PoseFieldMask().DisableTrajectory().DisableCustomScalars();
	@endcode
	*/
	class C_PoseFieldMask
	{
	public:
		friend class C_CommandList;
		friend class C_CommandProcessor;
	
		/// Create a default pose field mask where all fields are enabled.
		C_PoseFieldMask();
		
		/// Enable all pose fields.
		C_PoseFieldMask& EnableAll( bool enable = true );
		
		/// Disable all pose fields. NB. This mask will completely disable evaluation of a cycle.
		C_PoseFieldMask& DisableAll();
		
		/// Enable trajectory and trajectory delta transforms.
		C_PoseFieldMask& EnableTrajectory( bool enable = true );
		
		/// Disable trajectory and trajectory delta transforms.
		C_PoseFieldMask& DisableTrajectory();
		
		/// Enable all custom scalar curves.
		C_PoseFieldMask& EnableCustomScalars( bool enable = true );
		
		/// Disable all custom scalar curves.
		C_PoseFieldMask& DisableCustomScalars();
	
		/// Enable or disable a single field element.
		C_PoseFieldMask& Enable( E_PoseFieldElement elemType, bool enable = true );
		
		/// Disable a single field element.
		C_PoseFieldMask& Disable( E_PoseFieldElement elemType );
		
		/// Enables or disables all the elements in a field.
		C_PoseFieldMask& Enable( E_PoseFieldType fieldType, bool enabled = true );

		/// Disables all the elements in a field.
		C_PoseFieldMask& Disable( E_PoseFieldType fieldType );
		
		/// Returns true if the specified field element is enabled.
		bool IsEnabled( E_PoseFieldElement elemType ) const;

		/// Returns true if all the elements in a specified field are enabled.
		bool IsEnabled( E_PoseFieldType fieldType ) const;

		
	private:
		// Used by C_CommandList and related classes.
		U16 AsU16() const;
		explicit C_PoseFieldMask( U16 mask );

	private:
		U16	m_mask;		// Mask for all field elements.
		
		// Masks for each E_PoseFieldType.
		static const U32 kFieldTypeMasks[Animation::E_PoseFieldType_Count];
	};
}

#include "PoseFieldMask.inl"


#endif
