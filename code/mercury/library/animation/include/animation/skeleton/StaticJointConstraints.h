#ifndef ANIMATION_STATICJOINTCONSTRAINTS_H
#define ANIMATION_STATICJOINTCONSTRAINTS_H

#ifdef _MSC_VER
#pragma once
#endif

/**
	\file StaticJointConstraints.h
	
	@defgroup StaticJointConstraints Static Joint Constraints
	
	Static (stateless) joint constraints. These constraints mimic the behaviour of
	constraints in Maya.
	
	Using constraints avoids the need to store animation data for certain joints,
	but at the cost of additional computational time. Another advantage is that
	constrained joints will behave correctly if a character is procedurally
	animated, for example by using ragdoll or IK.
*/

#include <animation/skeleton/DrivenKeyCurve.h>
#include <animation/common/ArrayProxy.h>

namespace Animation
{
	class C_SkeletonPose;

	/**
		Constrains a joint's orientation to be a weighted sum of the orientations of
		two other joints.
		
		In Maya, this is referred to as an <em>orient constraint<em>.
		
		@ingroup StaticJointConstraints
	*/
	class C_OrientationConstraint
	{
	public:
	
		/// Apply constraint to skeleton pose.
		void Apply( C_SkeletonPose& skeletonPose ) const;
	
		/// Deserialise from a stream.
		template <typename T>
		void Deserialise( T& in );
		
	private:
		enum E_InputJoint
		{
			E_InputJoint_OrientationReference1	= 0,
			E_InputJoint_OrientationReference2,
			E_InputJoint_Count
		};
	
		// 50 bytes.
		MathsStorage::Float4 m_localRotationOffset;
		MathsStorage::Float3 m_minLimits;
		MathsStorage::Float3 m_maxLimits;
		F32 m_blendFraction;
		S16 m_jointIndex;
		S16 m_inputJointIndices[ E_InputJoint_Count ];
	};
	
	#if !PLATFORM_SPU
	template <typename T>
	inline void C_OrientationConstraint::Deserialise( T& in )
	{
		in >> m_localRotationOffset;
		in >> m_minLimits;
		in >> m_maxLimits;
		in >> m_blendFraction;
		in >> m_jointIndex;
		for ( U32 i = 0; i < E_InputJoint_Count; ++i )
		{
			in >> m_inputJointIndices[ i ];
		}
	}
	#endif
	
	/**
		Constrains a joint's position to be a blend of the positions of two
		other joints.
		
		In Maya, this is referred to as a <em>point constraint<em>.
		
		@ingroup StaticJointConstraints
	*/
	class C_PositionConstraint
	{
	public:
	
		void Apply( C_SkeletonPose& skeletonPose ) const;
	
		template <typename T>
		void Deserialise( T& in );
		
	private:
		enum E_InputJoint
		{
			E_InputJoint_Reference1	= 0,
			E_InputJoint_Reference2,
			E_InputJoint_Count
		};
	
		MathsStorage::Float3 m_localReferenceJointOffset1; 	// Offset from origin of first reference joint.
		MathsStorage::Float3 m_localReferenceJointOffset2; 	// Offset from origin of second reference joint.
		MathsStorage::Float3 m_minLimits;
		MathsStorage::Float3 m_maxLimits;
		MathsStorage::Float3 m_offset;	// Model space translation offset applied to target position.

		F32 m_blendFraction;
		S16 m_jointIndex;
		S16 m_inputJointIndices[ E_InputJoint_Count ];
	};
	
	#if !PLATFORM_SPU
	template <typename T>
	inline void C_PositionConstraint::Deserialise( T& in )
	{
		in >> m_localReferenceJointOffset1;
		in >> m_localReferenceJointOffset2;
		in >> m_offset;
		in >> m_minLimits;
		in >> m_maxLimits;
		in >> m_blendFraction;
		in >> m_jointIndex;
		for ( U32 i = 0; i < E_InputJoint_Count; ++i )
		{
			in >> m_inputJointIndices[ i ];
		}

	}
	#endif	
	
	/**
		Aims a joint towards another.
		
		@ingroup StaticJointConstraints
	*/
	class C_AimConstraint
	{
	public:
	
		void Apply( C_SkeletonPose& skeletonPose ) const;

		template <typename T>
		void Deserialise( T& stream );
		
	private:
		
		// 110 bytes.
	
		MathsStorage::Float4 m_localRotationOffset;	// Local space rotation offset as a quaternion.
		MathsStorage::Float3 m_localAimTargetOffset;			// Local-space offset applied to position of aim target joint.
		MathsStorage::Float3 m_localAimVector;				// Local aim vector of driven joint.
		MathsStorage::Float3 m_localUpVector;					// Local up vector of driven joint. 
		MathsStorage::Float3 m_referenceUpVector;				// Up vector in model space or reference joint space.
		MathsStorage::Float3 m_localReferenceJointOffset; 	// Offset from origin of first reference joint.
		MathsStorage::Float3 m_minLimits;
		MathsStorage::Float3 m_maxLimits;

		S16 m_jointIndex;
		S16 m_aimTargetJointIndex;
		S16 m_upVectorJointIndex;
		U8 m_upVectorType;
		U8 m_axisFlags;
	};
	
	#if !PLATFORM_SPU
	template <typename T>
	inline void C_AimConstraint::Deserialise( T& in )
	{
		in >> m_localRotationOffset;
		in >> m_localAimTargetOffset;
		in >> m_localAimVector;
		in >> m_localUpVector;
		in >> m_referenceUpVector;
		in >> m_localReferenceJointOffset;
		in >> m_minLimits;
		in >> m_maxLimits;
		in >> m_jointIndex;
		in >> m_aimTargetJointIndex;
		in >> m_upVectorJointIndex;
		in >> m_upVectorType;
		in >> m_axisFlags;
		
		BIZ_ASSERT( m_jointIndex >=0 );
	}
	#endif	
	
	class C_DrivenKey;
	
	/**
		Base functionality for driven key constraints with 3 inputs and outputs.
		
		@ingroup StaticJointConstraints
		
		Each constraint channel takes an input from another joint (e.g. a Euler rotation channel),
		applies a curve, and applies the result to a single channel of the driven joint. For example,
		the rotation of one joint can be made to control the translation of another.
		
		A large number of expensive quaterion to Euler conversions are required, so this type of
		constraint should be used in moderation.
	*/
	class C_DrivenKeyConstraint
	{
	public:
		C_DrivenKeyConstraint() {}
		~C_DrivenKeyConstraint() {}		// Not virtual, but that's OK for the time being.
	
	protected:
		void EvaluateDrivenKey(
			const C_SkeletonPose& skeletonPose,
			const C_ArrayProxy<const C_DrivenKey>& drivenKeys,
			const F32 inXyz[ 3 ],
			F32 outXyz[ 3 ] ) const;
		
	public:
		/// Deserialise.
		template <typename T>
		void Deserialise( T& in );
		
	protected:

		// 6 bytes total.
	
		S16 m_jointIndex;					// 2
		U8 m_drivenKeyIndices[ 3 ];			// One-based: zero indicates no influence.
		U8 m_padding;
	};

	template <typename T>
	void C_DrivenKeyConstraint::Deserialise( T& in )
	{
		in >> m_jointIndex;
		in >> m_drivenKeyIndices[ 0 ];
		in >> m_drivenKeyIndices[ 1 ];
		in >> m_drivenKeyIndices[ 2 ];
	}

	
	class C_DrivenKey
	{
	public:
		template <typename T>
		void Deserialise( T& in );
		
		U16					inputType;
		S16					inputJointIndex;
		F32					scaleFactor;
		C_DrivenKeyCurve	curve;
	};
	
	template <typename T>
	void C_DrivenKey::Deserialise( T& in )
	{
		in >> inputType;
		in >> inputJointIndex;
		in >> scaleFactor;
		in >> curve;
	}
	
		
	/**
		A driven key constraint affecting the Euler XYZ rotation channels of a joint.
		
		@ingroup StaticJointConstraints
	*/
	class C_DrivenKeyRotationConstraint: public C_DrivenKeyConstraint
	{
	public:
		void Apply( C_SkeletonPose& skeletonPose, const C_ArrayProxy<const C_DrivenKey>& drivenKeys ) const;
	};
	
	/**
		A driven key constraint affecting the XYZ translation channels of a joint.
		
		@ingroup StaticJointConstraints
	*/
	class C_DrivenKeyTranslationConstraint: public C_DrivenKeyConstraint
	{
	public:
		void Apply( C_SkeletonPose& skeletonPose, const C_ArrayProxy<const C_DrivenKey>& drivenKeys ) const;
	};

}

#endif
