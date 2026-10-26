#ifndef ANIMATION_TOOLS_COMPRESSEDCURVEUTILS_H
#define ANIMATION_TOOLS_COMPRESSEDCURVEUTILS_H

///
/// \file CompressedCurveUtils.h
///
/// Functions for creating compressed curves.
///

namespace AnimationTools
{

class C_Curve;

/// \addtogroup Animation_Tools
//! @{

///
/// Utility functions for creating compressed curves.
///
///
namespace CompressedCurveUtils
{

	/// \name Compressed Curve Creation Functions
	//@{

	/// Build a compressed quaternion curve.
	/// \param durationInFrames total duration in frames, including final frame in looped animations.
	bool BuildCompressedQuaternionCurve(
		const Maths::C_Quaternion* quatKeys,
		const U32* keyTimes,
		U32 numKeys,
		U32 durationInFrames,
		F32 maxRotationErrorInDegrees,
		C_Curve& outCurve
	);	
	
	/// Build a compressed vector3 curve.
	/// \param durationInFrames total duration in frames, including final frame in looped animations.
	bool BuildCompressedVector3Curve(
		const MathsStorage::Float3* vec3Keys,
		const U32* keyTimes,
		U32 numKeys,
		U32 durationInFrames,
		F32 maxError,
		bool useRelativeError,
		const F32* offset,
		const F32* scale,
		C_Curve& outCurve
	);
	
	/// Build a compressed scalar curve in a memory-optimal format.
	bool BuildCompressedScalarCurve(
		const F32* scalarKeys,
		const U32* keyTimes,
		U32 numKeys,
		U32 durationInFrames,
		F32 maxError,
		bool useRelativeError,
		F32 offset,
		F32 scale,
		C_Curve& outCurve
	);

	/// Flags corresponding to a 3D axis.
	/// \see FindSignificantRotationAxes
	enum E_AxisFlag
	{
		E_AxisFlag_X	= 1 << 0,	///< X axis flag.
		E_AxisFlag_Y	= 1 << 1,	///< Y axis flag.
		E_AxisFlag_Z	= 1 << 2	///< Z axis flag.
	};

	/// Find the significant rotation axes in a list of quaternions. Where no
	/// rotation occurs along a particular axis it is possible to store the
	/// quaternion more compactly.
	///
	/// \param quats array of quaternions.
	/// \param numQuats total number of quaternions in array.
	/// \param threshold when a quaternion X, Y or Z component exceeds this threshold
	/// it is considered to be significant.
	/// \return A bit set for each axis where the quaternion component is above
	/// the specified threshold (see #E_AxisFlag).
	U32 FindSignificantRotationAxes(
		const Maths::C_Quaternion* quats,
		U32 numQuats,
		F32 threshold
	);
	
	/// Returns true if two rotations are equal within a given tolerance.
	bool RotationsEqualWithinTolerance(
		const Maths::C_Quaternion& rotationA,
		const Maths::C_Quaternion& rotationB,
		F32 maxRotationErrorInDegrees
	);
		
	/// Returns true if two vectors are equal within a given tolerance.
	bool VectorsEqualWithinTolerance(
		const F32* vectorA,
		const F32* vectorB,
		U32 numDimensions,
		F32 maxError
	);

	//@}

	/// \name Quaternion Compression Functions
	//@{

	/// Quaternion rotation axis components.
	enum E_QuaternionAxis
	{
		E_QuaternionAxis_X	= 0,///< Component of quaternion rotation along X axis.
		E_QuaternionAxis_Y,		///< Component of quaternion rotation along Y axis.
		E_QuaternionAxis_Z		///< Component of quaternion rotation along Z axis.
	};

	/// Compress a quaternion describing a rotation about a single axis into 16 bits.
	void CompressSingleAxisQuaternion16( const Maths::C_Quaternion& quat, E_QuaternionAxis axis, U16& outQuat16 );

	/// Decompress a quaternion compressed using #CompressSingleAxisQuaternion16.
	void DecompressSingleAxisQuaternion16( U16 quat16, E_QuaternionAxis axis, Maths::C_Quaternion& outQuat );

	/// Compress a quaternion into 32-bits.
	void CompressQuaternion32( const Maths::C_Quaternion& quat, U32& outQuat32 );

	/// Decompress a 32-bit quaternion compressed using #CompressQuaternion32.
	void DecompressQuaternion32( U32 quat32, Maths::C_Quaternion& outQuat );

	/// Compress a quaternion into 48-bits (3 U16s).
	void CompressQuaternion48( const Maths::C_Quaternion& quat, U16 outQuat48[3] );

	/// Decompress a 48-bit quaternion compressed using #CompressQuaternion48.
	void DecompressQuaternion48( const U16 quat48[3], Maths::C_Quaternion& outQuat );

	//@}
	
	/// \name Vector3 Compression Functions
	//@{
		
	/// Compress a vector3 into 3 U8s.
	void CompressVector3U8( const F32 vector3[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], U8 outVec3U8[ 3 ] );
	
	/// Decompress a vector3 from 3 U8s.
	void DecompressVector3U8( const U8 vec3U8[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], F32 outVec3[ 3 ] );
	
	/// Compress a vector3 into 3 U16s.
	void CompressVector3U16( const F32 vector3[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], U16 outVec3U16[ 3 ] );

	/// Decompress a vector3 from 3 U16s.
	void DecompressVector3U16( const U16 vec3U16[ 3 ], const F32 offset[ 3 ], const F32 scale[ 3 ], F32 outVec3[ 3 ] );

	
	//@}
	

	/// Search through an array of key times to find index of key frame with
	/// key time less than or equal to \a time.
	/// \param keyTimes Array of key times.
	/// \param numKeys Number of keys.
	/// \param time The search time.
	/// \param outNextKeyIndex Output containing index of first key whose time is greater than \a time.
	/// \param outInterpolationFraction Output containing the fraction to use for
	/// interpolation between the keys at \a outNextKeyIndex - 1 and \a outNextKeyIndex + 1.
	void FindKeyIndexForTime(
		const U32* keyTimes,
		U32 numKeys,
		F32 time,
		U32& outNextKeyIndex,
		F32& outInterpolationFraction
	);
	
	// Empirically measured error constants..
	extern const F32 kMaxSingleAxisQuaternion16ErrorInDegrees;
	extern const F32 kMaxQuaternion32ErrorInDegrees;
	extern const F32 kMaxQuaternion48ErrorInDegrees;
	
}

//! @}

}

#endif
