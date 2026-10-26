#ifndef ANIMATION_POSEFIELDTYPES_H
#define ANIMATION_POSEFIELDTYPES_H

#ifdef _MSC_VER
#pragma once
#endif

/**
\file PoseFieldTypes.h

Defines enums for pose data fields.
*/
namespace Animation
{
	/**
	Types of data fields that can be present in a pose.
	
	\warning Reordering this enum can break the cycle file format! Appending is generally OK.
	*/
	enum E_PoseFieldType
	{
		E_PoseFieldType_JointTransform		= 0,			///< Skeleton joint transforms.
		E_PoseFieldType_TrajectoryDeltaTransform,			///< Skeleton trajectory joint delta transform.
		E_PoseFieldType_CustomVector3,						///< Custom 3-d vectors.
		E_PoseFieldType_CustomScalar,						///< Custom scalars.
		E_PoseFieldType_TrajectoryTransform,				///< Skeleton trajectory joint transform.
		E_PoseFieldType_CustomVector4,						///< Custom 4-d vectors.
		
		E_PoseFieldType_Count								///< Total number of pose field types.
	};
	
	/**
	Individual elements of pose fields.
	
	Custom 4D vectors are currently stored as two separate elements to leverage existing code.
	This wastes around 12 bytes of storage for each 4D vector field in a pose. Gains made in reduced
	code size and improved key frame compression.
	
	@warning Reordering this enum will break the cycle file format! Appending is generally OK.
	*/
	enum E_PoseFieldElement
	{
		E_PoseFieldElement_JointTransformRotation			= 0,	///< Joint transform rotation quaternion.
		E_PoseFieldElement_JointTransformTranslation,				///< Joint transform translation 3-d vector.
		E_PoseFieldElement_JointTransformScale,						///< Joint transform scale 3-d vector.
				
		E_PoseFieldElement_CustomScalar,							///< Custom scalar value.
		E_PoseFieldElement_CustomVector3,							///< Custom 3-d vector.
	
		E_PoseFieldElement_TrajectoryDeltaTransformRotation,		///< Skeleton trajectory joint delta transform rotation quaternion.
		E_PoseFieldElement_TrajectoryDeltaTransformTranslation,		///< Skeleton trajectory joint delta transform translation 3-d vector.
		//E_PoseFieldElement_TrajectoryDeltaTransformScale,			///< Skeleton trajectory joint delta transform scale 3-d vector.		
		
		E_PoseFieldElement_TrajectoryTransformRotation,				///< Skeleton trajectory joint transform rotation quaternion.
		E_PoseFieldElement_TrajectoryTransformTranslation,			///< Skeleton trajectory joint transform translation 3-d vector.
		//E_PoseFieldElement_TrajectoryTransformScale,				///< Skeleton trajectory joint transform rotation quaternion.		
		
		E_PoseFieldElement_CustomVector4XYZ,						///< Custom 4-d vector, XYZ part.
		E_PoseFieldElement_CustomVector4W,							///< Custom 4-d vector, W part.
		
		E_PoseFieldElement_Count									///< Total number of pose field element types.
	};
	
}

#endif
