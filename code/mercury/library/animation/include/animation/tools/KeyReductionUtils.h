#ifndef ANIMATION_TOOLS_KEYREDUCTIONUTILS_H
#define ANIMATION_TOOLS_KEYREDUCTIONUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

///
/// \file KeyReductionUtils.h
///
/// Functions for creating key-reduced curves.
///

#include <btl/Containers/vector.h>
#include <btl/Containers/rlebitvector.h>
#include <btl/Containers/bitvector.h>

namespace Maths
{
	class C_Quaternion;
}

namespace MathsStorage
{
	struct Float3;
}

namespace AnimationTools
{
	/// @addtogroup Animation_Tools
	//! @{

	///
	/// \brief Key reduction utility functions.
	///
	/// Key reduction is the process of reducing a sequence of keyframes, usually
	/// one per frame, by removing redundant keys that can be acceptably approximated
	/// using interpolation. The functions here assume linear interpolation with
	/// normalised linear interpolation for quaternions.
	///
	///
	namespace KeyReductionUtils
	{
		typedef Btl::Vector<Maths::C_Quaternion> QuaternionVector;	///< Quaternion dynamic vector.
		typedef Btl::Vector<MathsStorage::Float3> Float3Vector;		///< Float3 dynamic vector.
		typedef Btl::Vector<F32> FloatVector;						///< Float dynamic dynamic vector.
		typedef Btl::Vector<U32>	KeyIndexVector;					///< Key frame index dynamic vector.
		typedef Btl::BitVector	FlagVector;							///< Flags per frame vector.
		
		/// Creates a key-reduced quaternion curve from a regularly sampled list of keyframes.
		/// \returns the number of keys in the reduced curve.
		/// \param quaternionKeys pointer to the start of the keyframe array.
		/// \param numFrames number of key frames.
		/// \param maxRotationErrorInDegrees maximum permissable error between the reduced curve and input keyframes.
		/// \param outQuaternionKeys dynamic vector to be filled with the reduced set of quaternion keys.
		/// \param outKeyIndices dynamic vector to be filled with the reduced set of keyframe indices.
		S32 GetKeyReducedQuaternionCurve(
			const Maths::C_Quaternion* quaternionKeys,
			const FlagVector& discontinuityFlags,			
			S32 numFrames,
			F32 maxRotationErrorInDegrees,
			QuaternionVector& outQuaternionKeys,
			KeyIndexVector& outKeyIndices
		);

		/// Creates a key-reduced 3-d vector curve from a regularly sampled list of keyframes.
		/// \returns number of keys in the reduced curve.
		/// \param vector3Keys pointer to the start of the keyframe array.
		/// \param numFrames number of key frames.
		/// \param maxError maximum permissable error (3-d distance) between the reduced curve and input keyframes.
		/// \param outVector3Keys dynamic vector to be filled with the reduced set of quaternion keys.
		/// \param outKeyIndices dynamic vector to be filled with the reduced set of keyframe indices.
		S32 GetKeyReducedVector3Curve(
			const MathsStorage::Float3* vector3Keys,
			const FlagVector& discontinuityFlags,
			S32 numFrames,
			F32 maxError,
			Float3Vector& outVector3Keys,
			KeyIndexVector& outKeyIndices
		);

		/// Creates a key-reduced scalar curve from a regularly sampled list of keyframes.
		/// \returns number of keys in the reduced curve.
		/// \param scalarKeys pointer to the start of the keyframe array.
		/// \param numFrames number of key frames.
		/// \param maxError maximum permissable error between the reduced curve and the input keyframes.
		/// \param outScalarKeys dynamic vector to be filled with the reduced set of keys.
		/// \param outKeyIndices dynamic vector to be filled with the reduced set of keyframe indices.
		S32 GetKeyReducedScalarCurve(
			const F32* scalarKeys,
			const FlagVector& discontinuityFlags,
			S32 numFrames,
			F32 maxError,
			FloatVector& outScalarKeys,
			KeyIndexVector& outKeyIndices
		);

	}
	
	//! @}
}



#endif
