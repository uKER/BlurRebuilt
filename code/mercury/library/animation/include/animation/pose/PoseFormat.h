#ifndef ANIMATION_POSEFORMAT_H
#define ANIMATION_POSEFORMAT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <animation/common/StringId.h>
#include "PoseFieldTypes.h"

namespace Animation
{
	class C_Skeleton;
	class C_StringId;
	class C_Cycle;
	class C_PoseFieldMask;
	class C_Pose;

	/**
	
	Defines the data fields to be created inside a C_Pose. All fields of the same type must
	have a unique name.
	
	Example usage:
	
	@code
	// Add a skeleton and some custom scalars to a pose format.
	C_PoseFormat myPoseFormat;
	myPoseFormat.SetSkeleton( *mySkeleton );
	U32 index1 = myPoseFormat.AddCustomScalarField( "game_root.LEFT_FOOTSTEP_FLAG" );
	U32 index2 = myPoseFormat.AddCustomScalarField( "game_root.damage" );
	
	// Create a pose to hold this data.
	C_Pose myPose;
	myPose.Create( myPoseFormat );
	
	// ... evaluate cycles, do IK etc.
	
	// Query damage level.
	F32 damage = myPose.GetCustomScalar( index2 );
	@endcode
	
	To construct a pose that contains all the fields stored in a particular cycle:
	@code
	C_PoseFormat poseFormat;
	poseFormat.AddCycleFields( cycle );
	@endcode
	
	*/
	class C_PoseFormat
	{
	public:
		C_PoseFormat();
		
		/// Create a C_PoseFormat containing all the fields required by @a skeleton.
		explicit C_PoseFormat( const C_Skeleton* skeleton );
		
		/// Create a C_PoseFormat containing all the fields in @a cycle.
		explicit C_PoseFormat( const C_Cycle* cycle );
	
		//
		// Construction.
		//
		
		/// Clear out all fields.
		void Clear();
	
		/// Add all fields required for @a skeleton.
		void SetSkeleton( const C_Skeleton* skeleton );
		
		/// Add a custom scalar field.
		U32 AddCustomScalarField( const CHAR8* name );
		
		/// Add a custom 3D vector field.
		U32 AddCustomVector3Field( const CHAR8* name );
		
		/// Add a custom 4D vector field.
		U32 AddCustomVector4Field( const CHAR8* name );
		
		/// Add all fields stored in @a cycle.
		void AddCycleFields( const C_Cycle* cycle );
		
		/// Adds all cycle fields whose type is enabled in @a poseFieldMask.
		void AddCycleFields( const C_Cycle* cycle, const C_PoseFieldMask& poseFieldMask );
		
		/// Adds all pose fields whose type is enabled in @a poseFieldMask.
		void AddPoseFields( const C_Pose* pose, const C_PoseFieldMask& poseFieldMask );

		/// Adds all pose fields whose type is enabled in @a poseFieldMask and have the specified namespace prefix.
		/// The namespace prefix will be removed
		void AddPoseFieldsWithNamespace( const C_Pose* pose, const C_PoseFieldMask& poseFieldMask, const CHAR8* namespacePrefix );
		
		/// Adds a trajectory transform field.
		void AddTrajectoryTransformField();

		/// Adds a trajectory delta transform field.
		void AddTrajectoryDeltaTransformField();
		
		//
		// Info.
		//
		
		/// Returns total fields of all types.
		U32 GetTotalFields() const;
		
		/// Returns number of fields of type @a fieldType.
		U32 GetFieldCount( E_PoseFieldType fieldType ) const;
		
		/// Returns name of a specific field.
		const C_StringId& GetFieldName( E_PoseFieldType fieldType, U32 index ) const;
		
		/// Returns size of field in bytes.
		U32 GetFieldSize( E_PoseFieldType fieldType ) const;
		
		/// Add a field of arbitrary type.
		U32 AddField( E_PoseFieldType fieldType, const C_StringId& name );
		
		/// Returns index of field with matching name or -1 if none found.
		S32 FindFieldByName( E_PoseFieldType fieldType, const CHAR8* name ) const;

	private:
		U32 AddUniqueField(E_PoseFieldType fieldType, const C_StringId& name, S32 searchCount);
		
		typedef Btl::Vector<Animation::C_StringId>	FieldNameVector;
	
		const C_Skeleton*		m_skeleton;
		U32						m_totalFields;
		FieldNameVector			m_fieldNames[ E_PoseFieldType_Count ];
	};

}

#endif
