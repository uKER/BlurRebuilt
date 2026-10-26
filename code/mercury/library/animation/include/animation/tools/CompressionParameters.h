#ifndef ANIMATION_TOOLS_COMPRESSIONPARAMETERS_H
#define ANIMATION_TOOLS_COMPRESSIONPARAMETERS_H

#include <btl/Containers/vector.h>

namespace AnimationTools
{

/// @addtogrop Animation_Tools
//! @{

///
/// Compression parameters for controlling key reduction and quantisation.
///
// TODO: add custom curves (scalars), event curves etc.
class C_CompressionParameters
{
public:
	/// Create parameters for the specified number of joints.
	C_CompressionParameters( U32 numJoints );

	/// Set global single axis rotation threshold.
	/// If the absolute value of a quaternion's X, Y or Z component stays below
	/// this value then it is assumed that it there is no rotation about the
	/// corresponding axis.
	void SetGlobalSingleAxisRotationThreshold( F32 singleAxisRotationThreshold );

	/// Set maximum rotation error (in degrees) for all joints.
	void SetGlobalMaxRotationError( F32 maxRotationErrorInDegrees );

	/// Set maximum translation error for all joints.
	void SetGlobalMaxTranslationError( F32 maxTranslationError );

	/// Set maximum rotation error (in degrees) of an individual joint.
	void SetJointMaxRotationError( U32 jointIndex, F32 maxRotationErrorInDegrees );

	/// Set maximum translation error of an individual joint.
	void SetJointMaxTranslationError( U32 jointIndex, F32 maxTranslationError );

	/// Get maximum rotation error (in degrees) for an individual joint.
	F32 GetJointMaxRotationError( U32 jointIndex ) const;

	/// Get maximum translation error for an individual joint.
	F32 GetJointMaxTranslationError( U32 jointIndex ) const;

	/// Get single axis rotation threshold for an individual joint.
	F32 GetSingleAxisRotationThreshold( U32 jointIndex ) const;
	
	/// Gets maximum scale error of an individual joint.
	F32 GetJointMaxScaleError( U32 jointIndex ) const;
	
	/// Sets maximum scale error of an individual joint.
	void SetJointMaxScaleError( U32 jointIndex, F32 maxError );
	
private:
	struct JointParameters
	{
		F32 m_maxRotationErrorInDegrees;
		F32 m_maxTranslationError;
		F32 m_maxScaleError;
	};

	F32 m_singleAxisRotationThreshold;
	Btl::Vector< JointParameters > m_jointParams;
};

//! @}

}

#endif
