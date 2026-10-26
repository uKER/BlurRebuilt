#ifndef ANIMATION_EXPORTCONSTRAINTS_H
#define ANIMATION_EXPORTCONSTRAINTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <animation/tools/SceneToGameTransform.h>
#include <animation/tools/Status.h>
#include <animation/skeleton/ConstraintTypes.h>
#include <animation/skeleton/DrivenKeyCurve.h>

namespace Animation
{
	class C_JointTransform;
}

namespace Aftk
{
	class C_SceneNode;
}

namespace AnimationTools
{

	class C_AimConstraintHelper;

	class C_ExportConstraint
	{
	public:
		S32			nodeIndex;
		U32			order;				// Evaluation order.
		
		C_ExportConstraint(Animation::E_ConstraintType type): m_type(type) {}
		Animation::E_ConstraintType GetType() const		{ return m_type; }
		
	private:
		Animation::E_ConstraintType	m_type;
	};

	class C_ExportAimConstraint: public C_ExportConstraint
	{
	public:
		//S32							nodeIndex;
		S32							targetNodeIndex;
		MathsStorage::Float3		aimVector;
		MathsStorage::Float3		localTargetOffset;
		MathsStorage::Float3		localUpVector;
		MathsStorage::Float3		referenceUpVector;
		MathsStorage::Float4		rotationOffset;		// Local space rotation offset as quaternion.
		Animation::E_UpVectorType	upVectorType;
		S32							upVectorReferenceNodeIndex;
		MathsStorage::Float3		upVectorReferenceOffset;
		U32							axisFlags;
		MathsStorage::Float3		minLimits;			// sin(1/2 min angle)
		MathsStorage::Float3		maxLimits;			// sin(1/2 max angle)

		C_ExportAimConstraint(): C_ExportConstraint(Animation::E_ConstraintType_Aim) {}
		
		template <typename T>
		void Serialise( T& out ) const
		{
			BIZ_ASSERT( nodeIndex >= 0 );
			
			out << rotationOffset;
			out << localTargetOffset;
			out << aimVector;
			out << localUpVector;			
			out << referenceUpVector;
			out << upVectorReferenceOffset;
			out << minLimits;
			out << maxLimits;
			out << S16(nodeIndex);
			out << S16(targetNodeIndex);
			out << S16(upVectorReferenceNodeIndex);
			out << U8(upVectorType);
			out << U8(axisFlags);
			
		}			
	};

	class C_ExportOrientationConstraint: public C_ExportConstraint
	{
	public:
		//S32						nodeIndex;
		S32						inputNodeIndex1;
		S32						inputNodeIndex2;
		F32						blendFraction;
		MathsStorage::Float3	minLimits;			// sin(1/2 min angle)
		MathsStorage::Float3	maxLimits;			// sin(1/2 max angle)
		MathsStorage::Float4	localRotationOffset;	// Local space offset rotation as a quaternion.

		C_ExportOrientationConstraint(): C_ExportConstraint(Animation::E_ConstraintType_Orientation) {}

		template <typename T>
		void Serialise( T& out ) const
		{
			out << localRotationOffset;
			out << minLimits;
			out << maxLimits;
			out << blendFraction;
			out << S16(nodeIndex);
			out << S16(inputNodeIndex1);
			out << S16(inputNodeIndex2);
		}		
	};
	
	class C_ExportPositionConstraint: public C_ExportConstraint
	{
	public:
		//S32						nodeIndex;
		S32						inputNodeIndex1;
		S32						inputNodeIndex2;
		F32						blendFraction;
		MathsStorage::Float3	inputNodeOffset1;
		MathsStorage::Float3	inputNodeOffset2;
		MathsStorage::Float3	minLimits;
		MathsStorage::Float3	maxLimits;
		MathsStorage::Float3	localOffset;
		
		C_ExportPositionConstraint(): C_ExportConstraint(Animation::E_ConstraintType_Position) {}
		
		template <typename T>
		void Serialise( T& out ) const
		{
			out << inputNodeOffset1;
			out << inputNodeOffset2;
			out << localOffset;
			out << minLimits;
			out << maxLimits;
			out << blendFraction;			
			out << S16(nodeIndex);
			out << S16(inputNodeIndex1);
			out << S16(inputNodeIndex2);
		}		
	};	
	
	enum E_TransformChannel
	{
		E_TransformChannel_Rotation	= 0,
		E_TransformChannel_Translation
	};

	
	
	class C_ExportDrivenKeyConstraint: public C_ExportConstraint
	{
	public:
		U32		drivenKeyIndices[ 3 ];

		C_ExportDrivenKeyConstraint(E_TransformChannel channel):
			C_ExportConstraint(channel == E_TransformChannel_Rotation ? Animation::E_ConstraintType_DrivenKeyRotation : Animation::E_ConstraintType_DrivenKeyTranslation)
		{}
	
		
		template <typename T>
		void Serialise( T& out ) const
		{
			out << S16( nodeIndex );
			out << U8( drivenKeyIndices[ 0 ] );
			out << U8( drivenKeyIndices[ 1 ] );
			out << U8( drivenKeyIndices[ 2 ] );
		}
	};
	
	class C_ExportDrivenKey
	{
	public:
		S32							inputNodeIndex;
		Animation::E_InputType 		inputType;
		F32							inputScaleFactor;
		Animation::C_DrivenKeyCurve	curve;
	
		template <typename T>
		void Serialise( T& out ) const
		{
			out << U16( inputType );
			out << S16( inputNodeIndex );
			out << inputScaleFactor;
			out << curve;
		}
	};
	
	class C_ExportDynamicAimJoint: public C_ExportConstraint
	{
	public:
		MathsStorage::Float3	aimAxis;
		MathsStorage::Float3	upVectorNodeOffset;
		MathsStorage::Float3	upVector;
		MathsStorage::Float3	targetNodeOffset;
		S32						targetNodeIndex;
		S32						upVectorNodeIndex;
		F32 					minHeading;
		F32 					maxHeading;
		F32 					minPitch;
		F32 					maxPitch;
		F32						pitchSpring;
		F32						pitchDamping;
		F32						headingSpring;
		F32						headingDamping;
		F32						mass;
		F32						drag;
		F32						windInfluence;
		F32						buoyancyInfluence;
		
		
		C_ExportDynamicAimJoint():
			C_ExportConstraint(Animation::E_ConstraintType_DynamicAimJoint)
		{}

		
		template <typename T>
		void Serialise( T& out ) const
		{
			out << aimAxis;

			out << minHeading;
			out << maxHeading;
			out << minPitch;
			out << maxPitch;
			
			out << S16( nodeIndex );
			out << S16( targetNodeIndex );

			out << S16(upVectorNodeIndex);
			out << targetNodeOffset;
			out << upVectorNodeOffset;
			out << upVector;
			
			out << pitchSpring;
			out << pitchDamping;
			out << headingSpring;
			out << headingDamping;
			out << mass;
			out << drag;
			
			out << windInfluence;
			out << buoyancyInfluence;
		}	
	};
	
	class C_ExportDynamicSliderJoint: public C_ExportConstraint
	{
	public:
		F32						minLimit;
		F32						maxLimit;
		F32						spring;
		F32						damping;
		F32						mass;
		F32						drag;
		F32						windInfluence;
		F32						buoyancyInfluence;
		MathsStorage::Float3	axis;
	
		C_ExportDynamicSliderJoint():
			C_ExportConstraint(Animation::E_ConstraintType_DynamicSliderJoint)
		{}

	
		template <typename T>
		void Serialise( T& out ) const
		{
			out << axis;
			out << minLimit;
			out << maxLimit;

			out << spring;
			out << damping;
			out << mass;
			out << drag;
			out << windInfluence;
			out << buoyancyInfluence;
			
			out << S16( nodeIndex );
		}	
	};
	
	// pointer/reference/entry?
	class C_ExportConstraintIndex
	{
	public:
	
		C_ExportConstraintIndex():
			numInputs(0)
		{
		}
	
		static const U32 kMaxInputs = 4;
		
		
		Animation::E_ConstraintType	type;
		U32							index;
		
		// Intermediate data for sorting constraints.
		S32					nodeIndex;
		U32					numInputs;
		S32					inputNodeIndices[ kMaxInputs ];
		S32					next;		// Index of next index with same node index.
		
		void AddInput( S32 nodeIndex )
		{
			if ( nodeIndex >= 0 )
			{
				inputNodeIndices[ numInputs++ ] = nodeIndex;
			}
		}
		
		template <typename T>
		void Serialise( T& out ) const
		{
			out << U8( type );
			out << U8( index );
		}
	};
	
	
	class C_AimConstraintHelper;
	class C_OrientationConstraintHelper;
	class C_PositionConstraintHelper;
	class C_DrivenKeyHelper;
	class C_Scene;
	
	class C_ExportConstraints
	{
	public:
	
		void Clear();
	
		bool ImportConstraintsFromAftkSceneNode(
			const Aftk::C_SceneNode& sceneNode,
			const C_Scene& scene
		);
		
		C_Status Sort( const C_Scene& scene );
		
		template <typename T>
		void Serialise( T& out );
		
		U32 GetNumAimConstraints() const								{ return m_aimConstraints.Size(); }
		const C_ExportAimConstraint& GetAimConstraint(U32 index) const	{ return m_aimConstraints[index]; }
		
		U32 GetNumOrientationConstraints() const										{ return m_orientationConstraints.Size(); }
		const C_ExportOrientationConstraint& GetOrientationConstraint(U32 index) const	{ return m_orientationConstraints[index]; }
		
		U32 GetNumPositionConstraints() const										{ return m_positionConstraints.Size(); }
		const C_ExportPositionConstraint& GetPositionConstraint(U32 index) const	{ return m_positionConstraints[index]; }

		U32 GetNumDrivenKeyRotationConstraints() const										{ return m_drivenKeyRotationConstraints.Size(); }
		const C_ExportDrivenKeyConstraint& GetDrivenKeyRotationConstraint(U32 index) const	{ return m_drivenKeyRotationConstraints[index]; }

		U32 GetNumDrivenKeyTranslationConstraints() const										{ return m_drivenKeyTranslationConstraints.Size(); }
		const C_ExportDrivenKeyConstraint& GetDrivenKeyTranslationConstraint(U32 index) const  { return m_drivenKeyTranslationConstraints[index]; }

		U32 GetNumDrivenKeys() const								{ return m_drivenKeys.Size(); }
		const C_ExportDrivenKey& GetDrivenKey(U32 index) const		{ return m_drivenKeys[index]; }
		
		U32 GetNumDynamicAimJoints() const											{ return m_dynamicAimJoints.Size(); }
		const C_ExportDynamicAimJoint& GetDynamicAimJoint(U32 index) const			{ return m_dynamicAimJoints[index]; }
		U32 GetNumDynamicSliderJoints() const										{ return m_dynamicSliderJoints.Size(); }
		const C_ExportDynamicSliderJoint& GetDynamicSliderJoint(U32 index) const	{ return m_dynamicSliderJoints[index]; }
		
		U32 GetNumConstraints() const												{ return m_evalSequence.Size(); }
		Animation::E_ConstraintType GetConstraintType(U32 evalIndex) const			{ return m_evalSequence[evalIndex].type; }
		U32 GetConstraintIndex(U32 evalIndex) const			{ return m_evalSequence[evalIndex].index; }
		
		const C_ExportConstraint* GetConstraint(U32 evalIndex) const;
		
	private:
		bool ImportAimConstraint(
			const C_AimConstraintHelper& aimConstraintHelper
		);

		bool ImportOrientationConstraint(
			const C_OrientationConstraintHelper& orientationConstraintHelper
		);
		
		bool ImportPositionConstraint(
			const C_PositionConstraintHelper& positionConstraintHelper
		);
		
		bool ImportDrivenKeyConstraint(
			const C_DrivenKeyHelper& drivenKeyHelper,
			E_TransformChannel transformChannel
		);
		
		bool ImportDynamicAimJoint( const Aftk::C_SceneNode& sceneNode, const C_AimConstraintHelper& aimConstraintHelper );
		bool ImportDynamicSliderJoint( const Aftk::C_SceneNode& sceneNode );
		
		void ConvertEulerRotationToQuaternion( const MathsStorage::Float3& eulerRotation, Aftk::E_RotationOrder rotationOrder, MathsStorage::Float4& outQuaternion ) const;
		void ConvertToGameEulerAngles( const MathsStorage::Float3& eulerXyz, Aftk::E_RotationOrder rotationOrder, MathsStorage::Float3& outGameEulerXyz );

		/// Calculates an offset transform from an Aftk scene node to the nearest parent one in the animation scene.
		Animation::C_JointTransform GetOffsetTransform(const Aftk::C_SceneNode* aftkSceneNode, S32* outAnimSceneNodeIndex=0) const;
		
		/// Gets the relative translation of sceneNode in the space of animation node 'nodeIndex'.
		void GetNodeOffset(MathsStorage::Float3* outOffset, const Aftk::C_SceneNode* sceneNode, S32 nodeIndex);

		Btl::Vector< C_ExportAimConstraint > m_aimConstraints;
		Btl::Vector< C_ExportOrientationConstraint> m_orientationConstraints;
		Btl::Vector< C_ExportPositionConstraint> m_positionConstraints;
		Btl::Vector< C_ExportDrivenKeyConstraint > m_drivenKeyRotationConstraints;
		Btl::Vector< C_ExportDrivenKeyConstraint > m_drivenKeyTranslationConstraints;
		Btl::Vector< C_ExportDrivenKey > m_drivenKeys;
		Btl::Vector< C_ExportDynamicAimJoint > m_dynamicAimJoints;
		Btl::Vector< C_ExportDynamicSliderJoint > m_dynamicSliderJoints;
		
		Btl::Vector< C_ExportConstraintIndex > m_evalSequence;
		
		C_SceneToGameTransform m_sceneToGameTransform;
		const C_Scene* m_scene;
	};
	
	template <typename S, typename T>
	void SerialiseArray( S& out, const T& v )
	{
		for ( S32 i = 0; i < v.Size(); ++i )
		{
			out << v[ i ];
		}
	}

	template <typename T>
	inline void C_ExportConstraints::Serialise( T& out )
	{
	#ifdef NIGHTWING_BRANCH_COMPATIBLE
		static const U32 kVersion = 2;
	#else
		static const U32 kVersion = 4;
	#endif
		out << U16( kVersion );
		
		// Count total number of constraints.
		U32 totalConstraints = 0;
		totalConstraints += m_aimConstraints.Size();
		totalConstraints += m_orientationConstraints.Size();
		totalConstraints += m_positionConstraints.Size();
		totalConstraints += m_drivenKeyRotationConstraints.Size();
		totalConstraints += m_drivenKeyTranslationConstraints.Size();
		totalConstraints += 0; // custom constraints.
		totalConstraints += m_dynamicAimJoints.Size();
		totalConstraints += m_dynamicSliderJoints.Size();
		
		BIZ_ASSERT( totalConstraints == m_evalSequence.Size() );
		
		U32 totalDrivenKeys = m_drivenKeys.Size();
		
		out << U16(totalConstraints);
		out << U16(totalDrivenKeys);
		
		// Serialise number of constraints in E_ConstraintType order.
		out << U8( m_aimConstraints.Size() );
		out << U8( m_orientationConstraints.Size() );
		out << U8( m_positionConstraints.Size() );
		out << U8( m_drivenKeyRotationConstraints.Size() );
		out << U8( m_drivenKeyTranslationConstraints.Size() );
		out << U8( 0 ); // custom constraints.
		out << U8( m_dynamicAimJoints.Size() );
		out << U8( m_dynamicSliderJoints.Size() );
		
		// Write sorted constraint array. (U8 type, U8 relativeIndex)
		for ( U32 i = 0; i < totalConstraints; ++i )
		{
			out << m_evalSequence[ i ];
		}
		
		// Write driven keys.
		for ( U32 i = 0; i < totalDrivenKeys; ++i )
		{
			out << m_drivenKeys[ i ];
		}
		
		// Write constraints by type.
		SerialiseArray( out, m_aimConstraints );
		SerialiseArray( out, m_orientationConstraints );
		SerialiseArray( out, m_positionConstraints );
		SerialiseArray( out, m_drivenKeyRotationConstraints );
		SerialiseArray( out, m_drivenKeyTranslationConstraints );
		SerialiseArray( out, m_dynamicAimJoints );
		SerialiseArray( out, m_dynamicSliderJoints );
	}



}

#endif
