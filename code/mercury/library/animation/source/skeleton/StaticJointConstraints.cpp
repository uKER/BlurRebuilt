#include "stdafx.h"
#include <animation/skeleton/StaticJointConstraints.h>
#include <animation/skeleton/ConstraintTypes.h>
#include <animation/common/AnimationMaths.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>

namespace Animation
{

#define X_ROT_LIMIT_ONLY

	Maths::Quaternion Renormalise(const Maths::Quaternion& q)
	{
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		const Maths::Vector3 imag = Maths::Imag(q);
		const Maths::VF32 w = Maths::Sqrt(one - Maths::LengthSquared(imag));
		return Maths::MakeQuaternion(Maths::MakeVec4F32(imag, w));
	}
	
	#define ARCTAN2(y,x)	Maths::VF32( Maths::ArcTan(Maths::HF32(y),Maths::HF32(x)) )
	
	Maths::Quaternion ClampImag(const Maths::Quaternion& q, const Maths::Vector3& minImag, const Maths::Vector3& maxImag)
	{
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		const Maths::Vec4F32 q4 = Maths::GetVec4F32(Maths::Select(Maths::Real(q) < zero, -q, q));
		return Renormalise(Maths::MakeQuaternion(Maths::Clamp(q4, Maths::GetVec4F32(minImag), Maths::GetVec4F32(maxImag))));
	}
	
	Maths::Quaternion MakeQuaternionFromAxisAngleCosine(const Maths::Normal3& axis, const Maths::VF32& cosTheta)
	{
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		const Maths::VF32 half = Maths::Const<Maths::VF32>::Half();
		const Maths::VF32 safeCosTheta(Maths::Clamp(cosTheta, -one, one));
		const Maths::VF32 sinHalfTheta = Maths::Sqrt(half * (one - safeCosTheta));
		const Maths::VF32 cosHalfTheta = Maths::Sqrt(half * (one + safeCosTheta));
		return Maths::MakeQuaternion(Maths::MakeVec4F32(axis * sinHalfTheta, cosHalfTheta));
	}	
	
	
	/*
	Rotation limit about U axis.
	*/
	Maths::Vector3 GetSingleAxisLimitVector(
		const Maths::Normal3& axisU,
		const Maths::Normal3& axisV,
		const Maths::Normal3& rotationAxis,	// Rotation axis in UV plane.
		const Maths::Vector4& sinLimits,
		const Maths::Vector4& cosLimits
	)
	{
		const Maths::Normal3 axisW = Maths::Normalise(axisU ^ axisV);
		
		// cos(theta) of rotation axis in UV plane, where theta is rotation from U axis.
		const Maths::VF32 u = rotationAxis | axisU;
		const Maths::VF32 v = rotationAxis | axisV;
			
		// Limit points.
		const Maths::Vector3 limitNegU = Maths::X(sinLimits) * -axisU + Maths::X(cosLimits) * axisW;
		const Maths::Vector3 limitPosU = Maths::Y(sinLimits) * axisU  + Maths::Y(cosLimits) * axisW;

		const Maths::Vector3 projNegU = C_StereographicProjection::Project(limitNegU, axisW);
		const Maths::Vector3 projPosU = C_StereographicProjection::Project(limitPosU, axisW);
		
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		const Maths::Vector3 projActiveLimit = Maths::Select(u >= zero, projPosU, projNegU);
		
		const Maths::VF32 r = Maths::Length(projActiveLimit) / Maths::Abs(u);
		
		const Maths::Vector3 projRotationAxis = C_StereographicProjection::Project(Maths::MakeVector3(rotationAxis), axisW);
		const Maths::Vector3 projLimit = projRotationAxis * r;
		
		const Maths::Vector3 limitAxis = C_StereographicProjection::Unproject(projLimit, axisW);
		
		return limitAxis;
	}	
	
	/*
	Apply a single axis rotation limit.
	*/
	Maths::Quaternion ApplyEllipticalRotationLimits(
		const Maths::Quaternion& rotation,
		const Maths::Normal3& axisU,			// limited axis.
		const Maths::Normal3& axisV,			
		const Maths::Vector4& sinLimits,
		const Maths::Vector4& cosLimits
	)
	{
		const Maths::Normal3 coneAxis = Maths::Normalise(axisU ^ axisV);
		
		// Convert parent's cone axis into local space.
		Maths::Normal3 parentConeAxis = coneAxis * rotation;
		Maths::VF32 cosineSwingAngle = coneAxis | parentConeAxis;
		Maths::Normal3 swingAxis = Maths::Normalise(coneAxis ^ parentConeAxis);
		Maths::Quaternion swing = MakeQuaternionFromAxisAngleCosine(swingAxis, cosineSwingAngle);
		Maths::Quaternion twist = rotation * !swing;
		
		Maths::Vector3 limitVector = GetSingleAxisLimitVector(axisU, axisV, swingAxis, sinLimits, cosLimits);
		
		Maths::VF32 cosineMaxSwingAngle = limitVector | coneAxis;
		
		if ( cosineSwingAngle < cosineMaxSwingAngle )
		{
			// Swung out too far, recalculate the swing rotation.
			swing = MakeQuaternionFromAxisAngleCosine(swingAxis, cosineMaxSwingAngle);
		}
		
		return swing; // twist * swing
	}
	
	//Debug::Bool gEnableRotationLimits("Animation.EnableRotationLimits", true);

	const Maths::Quaternion ApplyRotationLimits(
		const Maths::Quaternion& rotation,
		const Maths::Vector3& minLimits,
		const Maths::Vector3& maxLimits
	)
	{
		const Maths::VF32 zero  = Maths::Const<Maths::VF32>::Zero();
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();

		Maths::Normal3 zAxis = Maths::Const<Maths::Normal3>::AxisZ();
		Maths::Normal3 rotatedZAxis = zAxis * rotation;

		Maths::Quaternion shortestRot = Animation::CalculateShortestRotationQuaternion(zAxis, rotatedZAxis);
		Maths::Quaternion q = Maths::Select(Maths::Real( shortestRot) < zero, -shortestRot, shortestRot);

		Maths::Quaternion twist = rotation * !shortestRot;

		const Maths::Vec4F32 q4 = Maths::GetVec4F32(q);

		const Maths::Vec4F32 clampedQAsVec4 = Maths::Clamp( q4, Maths::GetVec4F32( minLimits ), Maths::GetVec4F32( maxLimits ) );
		const Maths::VF32 w = Maths::Sqrt( one - Maths::Dot3( clampedQAsVec4, clampedQAsVec4 ) );
		const Maths::Quaternion result = Maths::MakeQuaternion( Maths::MakeVec4F32( Maths::MakeVector3( clampedQAsVec4 ), w ) );

		return twist * result;
	}
	
	const Maths::Vector3 LoadVector3( const MathsStorage::Float3* src )
	{
		return Maths::Load<Maths::Vector3, F32>( &src->x );
	}

	const Maths::Point3 LoadPoint3( const MathsStorage::Float3* src )
	{
		return Maths::MakePoint3( Maths::Load<Maths::Vector3, F32>( &src->x ) );
	}
	
	const Maths::Normal3 LoadNormal3( const MathsStorage::Float3* src )
	{
		return Maths::MakeNormal3( Maths::Load<Maths::Vector3, F32>( &src->x ) );
	}
	
	const Maths::Quaternion LoadQuaternion( const MathsStorage::Float4* src)
	{
		return Maths::MakeQuaternion( Maths::Load<Maths::Vec4F32, F32>( &src->x ) );
	}


//
// C_OrientationConstraint
//

void C_OrientationConstraint::Apply( C_SkeletonPose& skeletonPose ) const
{
	S32 j1 = m_inputJointIndices[E_InputJoint_OrientationReference1];
	S32 j2 = m_inputJointIndices[E_InputJoint_OrientationReference2];
	
	const Maths::Quaternion refJoint1Rotation = skeletonPose.GetModelJointRotation(j1);
	Maths::Quaternion drivenJointRotation = refJoint1Rotation;
	
	if (j1 != j2)
	{
		// Calculate world space rotation.
		const Maths::Quaternion refJoint2Rotation = skeletonPose.GetModelJointRotation(j2);
		const Maths::VF32 blendFrac = Maths::MakeVF32(m_blendFraction);
		const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		drivenJointRotation = Maths::Select( blendFrac > zero, Maths::NearestSlerp( refJoint1Rotation, refJoint2Rotation, blendFrac ), refJoint1Rotation );
	}
		
	// Convert into local space and apply limits.
	Maths::Quaternion localOffset = LoadQuaternion( &m_localRotationOffset );
	Maths::Quaternion newLocalRot = localOffset * drivenJointRotation * !skeletonPose.GetModelParentJointRotation( m_jointIndex );
	
	Maths::Quaternion newLocalLimitedRot = ApplyRotationLimits( newLocalRot, LoadVector3(&m_minLimits), LoadVector3(&m_maxLimits) );
	
	//skeletonPose.SetModelJointRotation( m_jointIndex, drivenJointRotation );
	skeletonPose.SetLocalJointRotation( m_jointIndex, newLocalLimitedRot );
}

//
// C_PositionConstraint
//

void C_PositionConstraint::Apply( C_SkeletonPose& skeletonPose ) const
{
	S32 j1 = m_inputJointIndices[E_InputJoint_Reference1];
	S32 j2 = m_inputJointIndices[E_InputJoint_Reference2];
	
	const C_JointTransform jointModelTransform1 = skeletonPose.GetModelJointTransform(j1);
	C_JointTransform jointModelTransform2 = jointModelTransform1;
	
	if (j1 != j2)
	{
		jointModelTransform2 = skeletonPose.GetModelJointTransform(j2);
	}

	const Maths::Point3 refJoint1Pos = jointModelTransform1.TransformPoint(LoadPoint3(&m_localReferenceJointOffset1));
	const Maths::Point3 refJoint2Pos = jointModelTransform2.TransformPoint(LoadPoint3(&m_localReferenceJointOffset2));
	Maths::Point3 newDrivenJointPos = Maths::Lerp( refJoint1Pos, refJoint2Pos, Maths::MakeVF32(m_blendFraction) );
	
	// Convert into local space and apply limits.
	const C_JointTransform modelParentTransform = skeletonPose.GetModelParentJointTransform( m_jointIndex ).Inverse();
	const Maths::Point3 localPos = modelParentTransform.TransformPoint( newDrivenJointPos ) + LoadVector3(&m_offset);
	
	const Maths::Vec4F32 localPosLimited = Maths::Clamp(
		Maths::MakeVec4F32( localPos ),
		Maths::MakeVec4F32( LoadVector3(&m_minLimits) ),
		Maths::MakeVec4F32( LoadVector3(&m_maxLimits) ) );
		
	skeletonPose.SetLocalJointTranslation( m_jointIndex, Maths::MakeVector3( localPosLimited ) );
}

//
// C_AimConstraint
//

void C_AimConstraint::Apply( C_SkeletonPose& skeletonPose ) const
{
#define BETTER

#ifdef BETTER	
	const C_JointTransform modelParentJointTransform = skeletonPose.GetModelParentJointTransform( m_jointIndex );
	const C_JointTransform modelJointTransform = skeletonPose.GetLocalJointTransform( m_jointIndex ) * modelParentJointTransform;

	Maths::Point3 drivenJointPos = modelJointTransform.GetPosition(); // skeletonPose.GetModelJointTranslation( m_jointIndex ) );
#else
	Maths::Point3 drivenJointPos = Maths::MakePoint3( skeletonPose.GetModelJointTranslation( m_jointIndex ) );
#endif
	 
	// Calculate aim target position.
	const C_JointTransform aimTargetModelTransform = skeletonPose.GetModelJointTransform( m_aimTargetJointIndex );
	const Maths::Point3 modelAimTargetPos = aimTargetModelTransform.TransformPoint( LoadPoint3(&m_localAimTargetOffset) );
	const Maths::Normal3 modelAimVector = Maths::Normalise( modelAimTargetPos - drivenJointPos );

	// Find rotation required to aim driven joint's local aim vector at target.
	const Maths::Normal3 localAimVector = LoadNormal3(&m_localAimVector);
	const Maths::Normal3 localUpVector = LoadNormal3(&m_localUpVector);
	const Maths::Quaternion aimRotation = CalculateShortestRotationQuaternion( localAimVector, modelAimVector );
	const Maths::Normal3 upVectorAfterAimRotation = localUpVector * aimRotation;
	
	// Derive model space target up vector according to the up vector type.
	Maths::Vector3 modelTargetUpVector;
	C_JointTransform refJointModelTransform;
	
	if (m_upVectorJointIndex >= 0)
	{
		refJointModelTransform = skeletonPose.GetModelJointTransform( m_upVectorJointIndex );
	}
	else
	{
		refJointModelTransform.SetIdentity();
	}
	
#ifdef BETTER
	
	if ( m_upVectorType == E_UpVectorType_AimAtReferenceJoint )
	{
		modelTargetUpVector = refJointModelTransform.TransformPoint( LoadPoint3(&m_localReferenceJointOffset) ) - drivenJointPos;
	}
	else if ( m_upVectorType == E_UpVectorType_ReferenceJointSpace )
	{
		// Transform reference joint space vector into model space.
		modelTargetUpVector = refJointModelTransform.TransformVector( LoadVector3(&m_referenceUpVector) );
	}
	else
	{
		// E_UpVectorType_ModelSpace

		// Transform by modelWorldMatrix to make it truly world space?
		modelTargetUpVector = LoadVector3(&m_referenceUpVector);
	}
	

	// Subtract any part of up vector parallel to model aim vector.
	//modelTargetUpVector = modelTargetUpVector - modelAimVector * ( modelTargetUpVector | modelAimVector );
	modelTargetUpVector = 
		Maths::NegMulSub(
			Maths::MakeVector3( modelAimVector ),
			modelTargetUpVector | modelAimVector,
			modelTargetUpVector
		);

	Maths::VF32 modelTargetUpVectorLength2 = Maths::LengthSquared( modelTargetUpVector );
	Maths::VF32 invLength = Maths::RcpSqrt( modelTargetUpVectorLength2 );	

	Maths::Normal3 modelTargetUpNormal =
		Maths::Select(
			modelTargetUpVectorLength2 != Maths::Const< Maths::VF32 >::Zero(),
			Maths::MakeNormal3( modelTargetUpVector * invLength ),
			upVectorAfterAimRotation
		);
		
#else
	const S32 upVectorReferenceJointIndex = m_upVectorJointIndex;
	switch ( m_upVectorType )
	{
		default:
		case E_UpVectorType_ModelSpace:
			// Could transform by modelWorldMatrix...
			modelTargetUpVector = LoadVector3(&m_referenceUpVector);
			break;
		
		case E_UpVectorType_ReferenceJointSpace:
		{
			// Transform reference joint space vector into model space.
			//const C_JointTransform refJointModelTransform = skeletonPose.GetModelJointTransform( upVectorReferenceJointIndex );
			modelTargetUpVector = refJointModelTransform.TransformVector( modelTargetUpVector );
			break;
		}
			
		case E_UpVectorType_AimAtReferenceJoint:
		{
			//const C_JointTransform refJointModelTransform = skeletonPose.GetModelJointTransform( upVectorReferenceJointIndex );
			modelTargetUpVector = refJointModelTransform.TransformPoint(LoadPoint3(&m_localReferenceJointOffset)) - drivenJointPos;
			break;
		}
	}


	// Subtract any part of up vector parallel to model aim vector.
	modelTargetUpVector = modelTargetUpVector - modelAimVector * ( modelTargetUpVector | modelAimVector );
		
	Maths::Normal3 modelTargetUpNormal =
			Maths::Select(
			Maths::LengthSquared( modelTargetUpVector ) != Maths::Const< Maths::VF32 >::Zero(),
				Maths::Normalise( modelTargetUpVector ),
			upVectorAfterAimRotation
			);
	
#endif			

	// Find twist rotation about modelAimVector that aligns localUpVector with modelTargetUpVector.
	Maths::Quaternion twistRotation = CalculateShortestRotationAboutAxis( upVectorAfterAimRotation, modelTargetUpNormal, modelAimVector );
	
	// Combine two rotations to produce final rotation.
	Maths::Quaternion finalAimRotation = aimRotation * twistRotation;
	
	// Convert into local space.
#ifdef BETTER
	Maths::Quaternion newLocalRotation = finalAimRotation * !modelParentJointTransform.GetRotation();
#else
	Maths::Quaternion newLocalRotation = finalAimRotation * !skeletonPose.GetModelParentJointRotation( m_jointIndex );
#endif
	
	if ( m_axisFlags != E_AxisFlag_XYZ )
	{
		Maths::Quaternion currentRotation = skeletonPose.GetLocalJointRotation( m_jointIndex );

		Maths::Vector3 currentEulerXyz;
		QuaternionToEuler( currentRotation, currentEulerXyz );
		Maths::Vector3 currentAimEulerXyz;
		QuaternionToEuler( newLocalRotation, currentAimEulerXyz );
				
		Maths::Vector3 newEulerXyz = currentEulerXyz;

		if ( m_axisFlags & E_AxisFlag_X ) Maths::SetX( &newEulerXyz, Maths::X( currentAimEulerXyz ) );
		if ( m_axisFlags & E_AxisFlag_Y ) Maths::SetY( &newEulerXyz, Maths::Y( currentAimEulerXyz ) );
		if ( m_axisFlags & E_AxisFlag_Z ) Maths::SetZ( &newEulerXyz, Maths::Z( currentAimEulerXyz ) );

		QuaternionFromEuler( newLocalRotation, newEulerXyz );
	}
	
	// Apply rotation offset, then limits.
	const Maths::Quaternion localOffset = LoadQuaternion( &m_localRotationOffset );
	newLocalRotation = localOffset * newLocalRotation;
	
	const Maths::Quaternion newLocalLimitedRotation = ApplyRotationLimits( newLocalRotation, LoadVector3(&m_minLimits), LoadVector3(&m_maxLimits) );
	skeletonPose.SetLocalJointRotation( m_jointIndex, newLocalLimitedRotation );
}



//
// C_DrivenKeyConstraint
//

void C_DrivenKeyConstraint::EvaluateDrivenKey(
	const C_SkeletonPose& skeletonPose,
	const C_ArrayProxy<const C_DrivenKey>& drivenKeys,
	const F32 inXyz[ 3 ],
	F32 outXyz[ 3 ]
) const
{
	Maths::Vector3 prevInputJointEulerXyz = Maths::Const<Maths::Vector3>::Zero();
	S32 prevInputJointIndex = -1;
	
	for ( U32 axisIndex = 0; axisIndex < 3; ++axisIndex )
	{
		// Skip processing if axis is not controlled by a driven key.
		if ( m_drivenKeyIndices[ axisIndex ] == 0 )
		{
			outXyz[ axisIndex ] = inXyz[ axisIndex ];
			continue;
		}
		
		const C_DrivenKey& drivenKey = drivenKeys[ m_drivenKeyIndices[ axisIndex ] - 1 ];
		
		F32 x;
		const S32 inputJointIndex = drivenKey.inputJointIndex;
		const E_InputType inputType = static_cast< E_InputType >( drivenKey.inputType );
		
		if ( inputType >= E_InputType_RotationX && inputType <= E_InputType_RotationZ )
		{
			Maths::Vector3 eulerXyz = prevInputJointEulerXyz;
			
			// Re-use euler decomposition if same input joint used for multiple axes.
			if ( inputJointIndex != prevInputJointIndex )
			{
				Maths::Quaternion localInputRotation = skeletonPose.GetLocalJointRotation( inputJointIndex );
				QuaternionToEuler( localInputRotation, eulerXyz );
			}
			
			Maths::StorageVector3 storedEulerXyz;
			Maths::Store( eulerXyz, &storedEulerXyz );
			
			x = storedEulerXyz.m_Data.v[ inputType ];
			
			prevInputJointEulerXyz = eulerXyz;
			prevInputJointIndex = inputJointIndex;
		}
		else
		{
			BIZ_ASSERT( inputType >= E_InputType_TranslationX && inputType <= E_InputType_TranslationZ );
			
			Maths::Vector3 localInputTranslation = skeletonPose.GetLocalJointTranslation( inputJointIndex );
			
			Maths::StorageVector3 storedLocalInputTranslation;
			Maths::Store( localInputTranslation, &storedLocalInputTranslation );
			
			x = storedLocalInputTranslation.m_Data.v[ inputType ];
		}
		
		x *= drivenKey.scaleFactor;
		
		outXyz[ axisIndex ] = drivenKey.curve.Evaluate( x );
	}
}



//
// C_DrivenKeyRotationConstraint
//

void C_DrivenKeyRotationConstraint::Apply( C_SkeletonPose& skeletonPose, const C_ArrayProxy<const C_DrivenKey>& drivenKeys ) const
{
	//PERF_EVENT( "DrivenKeyRotationConstraint.Apply" );
	Maths::Quaternion currentLocalRotation = skeletonPose.GetLocalJointRotation( m_jointIndex );
	Maths::Vector3 currentLocalEulerRotation;

	const bool notAllAnglesReplaced = ( m_drivenKeyIndices[ 0 ] == 0 || m_drivenKeyIndices[ 1 ] == 0 || m_drivenKeyIndices[ 2 ] == 0 );
	
	if ( notAllAnglesReplaced )
	{
		QuaternionToEuler( currentLocalRotation, currentLocalEulerRotation );
	}
	
	Maths::StorageVector3 currentLocalEulerXyz;
	Maths::Store( currentLocalEulerRotation, &currentLocalEulerXyz );
	
	Maths::StorageVector3 newLocalEulerXyz;
	
	EvaluateDrivenKey( skeletonPose, drivenKeys, currentLocalEulerXyz.m_Data.v, newLocalEulerXyz.m_Data.v );
	
	Maths::Quaternion newLocalRotation;
	QuaternionFromEuler( newLocalRotation, Maths::Load( &newLocalEulerXyz ) );
	
	skeletonPose.SetLocalJointRotation( m_jointIndex, newLocalRotation );

}

//
// C_DrivenKeyTranslationConstraint
//

void C_DrivenKeyTranslationConstraint::Apply( C_SkeletonPose& skeletonPose, const C_ArrayProxy<const C_DrivenKey>& drivenKeys ) const
{
	//PERF_EVENT( "DriveKeyTranslationConstraint.Apply" );
	Maths::StorageVector3 currentTranslation;
	Maths::Store( skeletonPose.GetLocalJointTranslation( m_jointIndex ), &currentTranslation );
	
	Maths::StorageVector3 newTranslation;
	
	EvaluateDrivenKey( skeletonPose, drivenKeys, currentTranslation.m_Data.v, newTranslation.m_Data.v );
	
	Maths::Vector3 newLocalTranslation = Maths::Load( &newTranslation );
	
	skeletonPose.SetLocalJointTranslation( m_jointIndex, newLocalTranslation );
}

} // namespace Animation

