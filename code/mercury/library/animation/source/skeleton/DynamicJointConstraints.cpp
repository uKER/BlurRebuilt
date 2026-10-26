#include "stdafx.h"
#include <animation/skeleton/DynamicJointConstraints.h>
#include "DynamicJointContext.h"
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>
#include <animation/skeleton/Skeleton.h>
#include <animation/common/StringId.h>
#include <animation/common/AnimationMaths.h>
#include <animation/common/JointTransform.inl>

//#include <system/debug/I_DebugRender.h>
//#define ANIM_DEBUG_SPRINGS


namespace Animation
{

//
// C_DynamicJointContext
//
C_DynamicJointContext::C_DynamicJointContext(
	const C_JointTransform& inModelWorldTransform
):
	force( Maths::Const<Maths::Vector3>::Zero() ),
	timeLeft( 0.f ),
	fixedTimeStep( 1.f / 30.f ),
	invFixedTimeStep( 30.f ),
	modelWorldTransform(inModelWorldTransform),
	worldModelTransform(inModelWorldTransform.Inverse()),
	enabled(true),
	waterPlaneEnabled(false)
{
	SetGravity(Maths::MakeVector3(0.f, -9.81f, 0.f));
}

bool C_DynamicJointContext::ShouldSimulate() const
{
	return timeLeft >= fixedTimeStep || reset;
}

F32 C_DynamicJointContext::GetInterpolationAmount() const
{
	F32 amount = 1.f;
	if (!reset)
	{
		amount = fmodf( timeLeft, fixedTimeStep ) * invFixedTimeStep;
	}
	return amount;
}

void C_DynamicJointContext::SetFixedTimeStep( F32 timeStep )
{
	fixedTimeStep = timeStep;
	invFixedTimeStep = Maths::Reciprocal( timeStep );
}

void C_DynamicJointContext::SetGravity(const Maths::Vector3& worldGravity)
{
	gravity = worldGravity;
	modelGravity = worldModelTransform.TransformVector(worldGravity);
}

//
// C_DynamicJoint
//

C_DynamicJoint::C_DynamicJoint():
	m_jointIndex(-1),
	m_targetJointIndex(-1),
	m_oneOverMass(1.f),
	m_oneMinusDrag(1.f),
	m_windInfluence(1.f),
	m_buoyancyInfluence(1.f)
{
}

void C_DynamicJoint::SetJointIndex(S32 jointIndex)
{
	m_jointIndex = jointIndex;
}

S32 C_DynamicJoint::GetJointIndex() const
{
	return m_jointIndex;
}
	
void C_DynamicJoint::SetTargetJointIndex(S32 jointIndex)
{
	m_targetJointIndex = jointIndex;
}

S32 C_DynamicJoint::GetTargetJointIndex() const
{
	return m_targetJointIndex;
}

void C_DynamicJoint::SetMass(F32 mass)
{
	if (mass > 0.f)
	{
		m_oneOverMass = 1.f / mass;
	}
	else
	{
		m_oneOverMass = 1.f;
	}
}

F32	C_DynamicJoint::GetMass() const
{
	return 1.f / m_oneOverMass;
}
	
void C_DynamicJoint::SetAirResistance(F32 ar)
{
	m_oneMinusDrag = Maths::Clamped( 1.f - ar, 0.f, 1.f );
}

F32 C_DynamicJoint::GetAirResistance() const
{
	return 1.f - m_oneMinusDrag;
}

void C_DynamicJoint::SetWindInfluence(F32 f)
{
	m_windInfluence = f;
}

F32	C_DynamicJoint::GetWindInfluence() const
{
	return m_windInfluence;
}

void C_DynamicJoint::SetBuoyancyInfluence(F32 b)
{
	m_buoyancyInfluence = b;
}

F32	C_DynamicJoint::GetBuoyancyInfluence() const
{
	return m_buoyancyInfluence;
}


//
// C_DynamicAimJoint
//

#ifndef PLATFORM_SPU

void C_DynamicAimJoint::Initialise( const C_Skeleton& skeleton, const C_StringId& jointName, const C_StringId& targetJointName )
{
	SetJointIndex(skeleton.FindJointByName( jointName ));
	SetTargetJointIndex(skeleton.FindJointByName( targetJointName ));
	
	if ( IsValid() )
	{
		// TODO: initialise from base pose.
		m_targetFollowerPos = Maths::Const< Maths::Point3 >::Zero();
		m_prevTargetFollowerPos = Maths::Const< Maths::Point3 >::Zero();
		
		m_currLocalRotation = Maths::Const<Maths::Quaternion>::Identity();
		m_prevLocalRotation = Maths::Const<Maths::Quaternion>::Identity();

	}
	
	m_velocity = Maths::Const< Maths::Vector3 >::Zero();
	SetMass( 1.f );
	SetAirResistance( 0.f );
	SetAimAxis(Maths::Const<Maths::Normal3>::AxisZ());
	ClearForce();
}

void C_DynamicAimJoint::Initialise( const C_DynamicAimJointParams& params )
{
	SetJointIndex(params.m_jointIndex);
	SetTargetJointIndex(params.m_targetJointIndex);
	
	// TODO: initialise from base pose.
	m_targetFollowerPos = Maths::Const< Maths::Point3 >::Zero();
	m_prevTargetFollowerPos = Maths::Const< Maths::Point3 >::Zero();
		
	m_currLocalRotation = Maths::Const<Maths::Quaternion>::Identity();
	m_prevLocalRotation = Maths::Const<Maths::Quaternion>::Identity();
	
	m_velocity = Maths::Const< Maths::Vector3 >::Zero();
	
	SetLimits( params.m_minHeading, params.m_maxHeading, params.m_minPitch, params.m_maxPitch );
	SetAimAxis(params.m_aimAxis);
	
	SetMass(params.m_mass);
	SetSpringConstants(params.m_pitchSpring, params.m_pitchDamping, params.m_headingSpring, params.m_headingDamping);
	SetAirResistance(params.m_drag);
	SetWindInfluence(params.m_windInfluence);
	SetBuoyancyInfluence(params.m_buoyancyInfluence);
	
	m_targetJointOffset = Maths::MakePoint3(params.m_targetJointOffset);
	m_upVectorJointOffset = Maths::MakePoint3(params.m_upVectorJointOffset);
	m_upVector = params.m_upVector;
	m_upVectorJointIndex = params.m_upVectorJointIndex;
	
	if ( m_upVectorJointIndex < 0 )
	{
		m_upVectorJointIndex = GetJointIndex();
	}
	ClearForce();
}

void C_DynamicAimJoint::SetLimits( F32 minHeading, F32 maxHeading, F32 minPitch, F32 maxPitch )
{
	F32 minAngle = -Maths::Pi;
	F32 maxAngle = +Maths::Pi;
	minHeading = 0.5f * Maths::Clamped( minHeading, minAngle, maxAngle );
	maxHeading = 0.5f * Maths::Clamped( maxHeading, minAngle, maxAngle );
	minPitch = 0.5f * Maths::Clamped( minPitch, minAngle, maxAngle );
	maxPitch = 0.5f * Maths::Clamped( maxPitch, minAngle, maxAngle );
	
	F32 sinMinPitch = sinf(minPitch);
	F32 sinMaxPitch = sinf(maxPitch);
	F32 sinMinHeading = sinf(minHeading);
	F32 sinMaxHeading = sinf(maxHeading);
	
	if (sinMinPitch > sinMaxPitch)
	{
		F32 oldMin = sinMinPitch;
		sinMinPitch = sinMaxPitch;
		sinMaxPitch = oldMin;
	}
	if (sinMinHeading > sinMaxHeading)
	{
		F32 oldMin = sinMinHeading;
		sinMinHeading = sinMaxHeading;
		sinMaxHeading = oldMin;
	}

	m_sinPitchHeadingLimits = Maths::MakeVector4(
		sinMinPitch,
		sinMaxPitch,
		sinMinHeading,
		sinMaxHeading
	);

	BIZ_ASSERT( sinMinHeading <= sinMaxHeading );
	BIZ_ASSERT( sinMinPitch <= sinMaxPitch );
}

void C_DynamicAimJoint::GetLimits( F32* outMinHeading, F32* outMaxHeading, F32* outMinPitch, F32* outMaxPitch) const
{
	if (outMinHeading)
		*outMinHeading = 2.f * asinf(Maths::GetF32(Maths::Z(m_sinPitchHeadingLimits)));
	if (outMaxHeading)
		*outMaxHeading = 2.f * asinf(Maths::GetF32(Maths::W(m_sinPitchHeadingLimits)));
	if (outMinPitch)
		*outMinPitch = 2.f * asinf(Maths::GetF32(Maths::X(m_sinPitchHeadingLimits)));
	if (outMaxPitch)
		*outMaxPitch = 2.f * asinf(Maths::GetF32(Maths::Y(m_sinPitchHeadingLimits)));
}

void C_DynamicAimJoint::SetAimAxis( const Maths::Normal3& axis )
{
	m_aimAxis = axis;
}

void C_DynamicAimJoint::SetSpringConstants(F32 pitchSpring, F32 pitchDamping, F32 headingSpring, F32 headingDamping)
{
	// Damping should be between zero and one.
	m_pitchHeadingSpringDamping = Maths::MakeVector4(
		pitchSpring,
		headingSpring,
		pitchDamping * Maths::Sqrt(pitchSpring),
		headingDamping * Maths::Sqrt(headingSpring)
	);
}

void C_DynamicAimJoint::GetSpringConstants(F32* outPitchSpring, F32* outPitchDamping, F32* outHeadingSpring, F32* outHeadingDamping) const
{
	if (outPitchSpring)
		*outPitchSpring = Maths::GetF32(Maths::X(m_pitchHeadingSpringDamping));
	if (outPitchDamping)
		*outPitchDamping = Maths::GetF32(Maths::Z(m_pitchHeadingSpringDamping) / Maths::Sqrt(Maths::X(m_pitchHeadingSpringDamping)));
	if (outHeadingSpring)
		*outHeadingSpring = Maths::GetF32(Maths::Y(m_pitchHeadingSpringDamping));
	if (outHeadingDamping)
		*outHeadingDamping = Maths::GetF32(Maths::W(m_pitchHeadingSpringDamping) / Maths::Sqrt(Maths::Y(m_pitchHeadingSpringDamping)));
}

#endif

bool C_DynamicAimJoint::IsValid() const
{
	return GetJointIndex() >= 0 && GetTargetJointIndex() >= 0;
}

bool C_DynamicAimJoint::IsInvalid() const
{
	return GetJointIndex() < 0 || GetTargetJointIndex() < 0;
}


DECLARE_DEBUG_BOOL(gSpringJointEnableLimits, "Animation.Dynamics.EnableLimits", true );

#ifdef ANIM_DEBUG_SPRINGS
DECLARE_DEBUG_FLOAT(gSpringJointVelocityDamping, "Animation.Dynamics.VelocityDamping", 1.f, Debug::Range( 0.f, 1.f ) );
DECLARE_DEBUG_FLOAT(gSpringJointPitchSpring, "Animation.Dynamics.DynamicAimJointPitchSpring", 20.f, Debug::Range( 0.f, 800.f ) );
DECLARE_DEBUG_FLOAT(gSpringJointPitchDamping, "Animation.Dynamics.DynamicAimJointPitchDamping", 1.f, Debug::Range( 0.f, 1.f ) );
DECLARE_DEBUG_FLOAT(gSpringJointHeadingSpring, "Animation.Dynamics.DynamicAimJointHeadingSpring", 20.f, Debug::Range( 0.f, 800.f ) );
DECLARE_DEBUG_FLOAT(gSpringJointHeadingDamping, "Animation.Dynamics.DynamicAimJointHeadingDamping", 1.f, Debug::Range( 0.f, 1.f ) );
#endif

#ifdef ANIM_TEST_WATER
DECLARE_DEBUG_FLOAT(gSpringJointBuoyancyForce, "Animation.Dynamics.BuoyancyForce", 1.f, Debug::Range( 0.f, 100.f ) );
DECLARE_DEBUG_FLOAT(gSpringJointWaterResistance, "Animation.Dynamics.WaterResistance", 1.f, Debug::Range( 0.f, 1.f ) );
DECLARE_DEBUG_BOOL(gSpringJointWaterPlaneEnabled, "Animation.Dynamics.WaterPlaneEnabled", false);
DECLARE_DEBUG_FLOAT(gSpringJointWaterPlaneHeight, "Animation.Dynamics.WaterPlaneHeight", 0.f, Debug::Range(0.f, 2.f));
#endif

inline Maths::Quaternion GetLocalRotationFromTargetPoint(
	const Maths::Normal3& localAimAxis,
	const Maths::Point3& targetPoint,
	const C_JointTransform& worldToLocalTransform )
{
	Maths::Point3 localTargetPos = worldToLocalTransform.TransformPoint( targetPoint );
	Maths::Normal3 localTargetDir = Maths::Normalise( Maths::MakeVector3( localTargetPos ) );
	return CalculateShortestRotationQuaternion( localAimAxis, localTargetDir );
}

void C_DynamicAimJoint::ApplyForce(const Maths::Vector3& f)
{
	m_force += f;
}

void C_DynamicAimJoint::ClearForce()
{
	m_force = Maths::Const<Maths::Vector3>::Zero();
}

void C_DynamicAimJoint::SimulateAndApply( C_SkeletonPose& skeletonPose, const C_DynamicJointContext& context )
{
	if ( IsInvalid() || !context.enabled )
	{
		return;
	}
	
	//PERF_EVENT("DynamicAimJoint.Evaluate");
	
	const S32 targetJointIndex = GetTargetJointIndex();
	const Maths::Point3 modelTargetPos = Maths::MakePoint3( skeletonPose.GetModelJointTranslation( targetJointIndex ) );
	
	C_JointTransform jointModelTransform = skeletonPose.GetModelJointTransform( GetJointIndex() );
	jointModelTransform.SetRotation(GetDesiredModelJointRotation( skeletonPose, context ));

	
	// If we update greater than the animation rate then only the first iteration will carry the correct inherited velocity.
	// Therefore, we limit the dynamics to one iteration per Evaluate call.
	if ( context.ShouldSimulate() )
	{
		if (context.reset)
		{
			Maths::Vector3 modelAimVector = Maths::MakeVector3(jointModelTransform.TransformNormal(m_aimAxis));
			m_targetFollowerPos = jointModelTransform.GetPosition() + modelAimVector;
			m_prevTargetFollowerPos = m_targetFollowerPos;
		}
	
		// Simulate the follower particle's position.
		SimulateFollower(context, jointModelTransform);

		// Now calculate the final aimed rotation.
		Maths::Vector3 modelAimVector = m_targetFollowerPos - jointModelTransform.GetPosition();
		Maths::Quaternion finalAimRotation = Animation::CalculateAimRotation(m_aimAxis, m_upVector, modelAimVector, GetModelUpVector(skeletonPose));
		
		m_prevLocalRotation = m_currLocalRotation;
		m_currLocalRotation = finalAimRotation * !skeletonPose.GetModelParentJointRotation(GetJointIndex());
		
		if (context.reset)
		{
			m_prevTargetFollowerPos = m_targetFollowerPos;
		}
	}
	
	ClearForce();

	const Maths::VF32 alpha = Maths::MakeVF32( context.GetInterpolationAmount() );
	const Maths::Quaternion localRotation = Maths::NearestLerpNormalised( m_prevLocalRotation, m_currLocalRotation, alpha );
	skeletonPose.SetLocalJointRotation( GetJointIndex(), localRotation );
}

Maths::Vector3 C_DynamicAimJoint::GetModelUpVector( C_SkeletonPose& skeletonPose ) const
{
	C_JointTransform modelDynamicJointTransform = skeletonPose.GetModelJointTransform(GetJointIndex());
	C_JointTransform modelUpVectorJointTransform = skeletonPose.GetModelJointTransform(m_upVectorJointIndex);
	Maths::Point3 modelUpVectorTarget = modelUpVectorJointTransform.TransformPoint(m_upVectorJointOffset);
	const Maths::Vector3 modelUpVector = modelUpVectorTarget - modelDynamicJointTransform.GetPosition();

	return modelUpVector;
}

Maths::Quaternion C_DynamicAimJoint::GetDesiredModelJointRotation( C_SkeletonPose& skeletonPose, const C_DynamicJointContext& context ) const
{
	const C_JointTransform modelTargetTransform = skeletonPose.GetModelJointTransform(GetTargetJointIndex());
	const Maths::Point3 modelTargetPos = modelTargetTransform.TransformPoint(m_targetJointOffset);
	
	C_JointTransform modelDynamicJointTransform = skeletonPose.GetModelJointTransform(GetJointIndex());
	C_JointTransform modelUpVectorJointTransform = skeletonPose.GetModelJointTransform(m_upVectorJointIndex);
	Maths::Point3 modelUpVectorTarget = modelUpVectorJointTransform.TransformPoint(m_upVectorJointOffset);
	const Maths::Vector3 modelAimVector = modelTargetPos - modelDynamicJointTransform.GetPosition();
	const Maths::Vector3 modelUpVector = modelUpVectorTarget - modelDynamicJointTransform.GetPosition();
	
	return Animation::CalculateAimRotation(m_aimAxis, m_upVector, modelAimVector, modelUpVector);
}

/*
(1) Integrate the follower particle's position.
(2) Calculate a new rotation that aims at the new follower position (which has been updated to incorporate gravity etc)
(3) Apply a spring to this rotation to orient it towards the desired rotation.
(4) Apply limits to the resulting rotation (spherical)
(5) Recalculate follower position using limited rotation
(6) Recalculate rotation from limited follower position

Note that pitch is considered to be rotation about X and heading a rotation about Y.
It assumed that the aim axis is +Z.
*/
void C_DynamicAimJoint::SimulateFollower( const C_DynamicJointContext& context, const C_JointTransform& desiredModelTransform)
{
	// Initialise constants.
	const Maths::VF32 timeStep = Maths::MakeVF32( context.fixedTimeStep );
	const Maths::VF32 timeStepSquared = timeStep * timeStep;
	const Maths::VF32 invTimeStep = Maths::MakeVF32( context.invFixedTimeStep );
	const Maths::VF32 halfTimeStepSquared = Maths::Const< Maths::VF32 >::Half() * timeStep * timeStep;
	const Maths::VF32 invMass = Maths::MakeVF32( m_oneOverMass );
	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		
	const Maths::Normal3 aimAxis = m_aimAxis;
	const Maths::Normal3 localUpVector = m_upVector;
	
	Maths::VF32 oneMinusDrag = Maths::MakeVF32(m_oneMinusDrag);

	#ifdef ANIM_DEBUG_SPRINGS
	oneMinusDrag = Maths::MakeVF32(gSpringJointVelocityDamping.Value());
	SetSpringConstants(gSpringJointPitchSpring, gSpringJointPitchDamping, gSpringJointHeadingSpring, gSpringJointHeadingDamping);
	#endif
	
	Maths::Vector3 force = context.worldModelTransform.TransformVector(m_force);
	Maths::Vector3 gravity = context.modelGravity;
	
	#ifdef ANIM_TEST_WATER
	if (gSpringJointWaterPlaneEnabled) // context.waterPlaneEnabled)
	{
		Maths::Plane worldWaterPlane = Maths::MakePlane(Maths::Const<Maths::Normal3>::AxisY(), Maths::MakePoint3(0.f, (F32)gSpringJointWaterPlaneHeight, 0.f));
	
		Maths::VF32 distAboveWater = Maths::Distance(worldWaterPlane, desiredWorldTransform.GetPosition());
		Maths::VBool isUnderWater = distAboveWater < zero;
		Maths::VF32 buoyancyForce = Maths::MakeVF32(gSpringJointBuoyancyForce);
		Maths::VF32 waterResistance = Maths::MakeVF32(gSpringJointWaterResistance);
		const Maths::VF32 one = Maths::Const<Maths::VF32>::One();
		oneMinusDrag *= Maths::Select(isUnderWater, waterResistance, one);
		force += Maths::Select(isUnderWater, buoyancyForce, zero) * Maths::Normal(worldWaterPlane);
	}
	#endif
			
	Maths::Vector3 accel = gravity + invMass * force;
		
	// Integrate the follower's position.
	{
		Maths::Vector3 v = ( m_targetFollowerPos - m_prevTargetFollowerPos ) * oneMinusDrag;
	
		Maths::Point3 newTargetFollowerPos = m_targetFollowerPos + v + accel * timeStepSquared;
		m_prevTargetFollowerPos = m_targetFollowerPos;
		m_targetFollowerPos = newTargetFollowerPos;
	}
		
	//
	// Use a spring to drive the joint to aim at the target. The code is only concerned
	// with generating corrective "swing" rotations and ignores "twist" which is calculated
	// separately.
	//
	
	const C_JointTransform modelToDesiredTransform = desiredModelTransform.Inverse();
	
	// local rotation to current target follower position.
	const Maths::Point3 localTargetPos = modelToDesiredTransform.TransformPoint(m_targetFollowerPos);
	Maths::Quaternion currRotation = Animation::CalculateShortestRotationQuaternion(aimAxis, Maths::Normalise(Maths::MakeVector3(localTargetPos)));
	
	// rotation from current to desired.
	Maths::Quaternion deltaRotation = !currRotation;
	
	// Map sin(halfTheta) onto -pi +pi range.
	deltaRotation = Maths::Select( Maths::Real( deltaRotation ) < zero, -deltaRotation, deltaRotation );
	
	// Calculate rotational error in axis-angle form, by assuming that sin(theta) is approximately equal to theta.
	// N.B. this will be perpendicular to the aim axis i.e. it will have a zero Z component.
	Maths::Vector3 rotationError = Maths::Imag( deltaRotation ) + Maths::Imag( deltaRotation );
	
	// local rotation to prev target follower position.
	const Maths::Point3 localPrevTargetPos = modelToDesiredTransform.TransformPoint(m_prevTargetFollowerPos);
	Maths::Quaternion prevRotation = Animation::CalculateShortestRotationQuaternion(aimAxis, Maths::Normalise(Maths::MakeVector3(localPrevTargetPos)));
	
	const Maths::Quaternion dq = Maths::NearestDifference(prevRotation, currRotation); // !prevRotation * currRotation;
	
	// Estimate angular velocity.
	const Maths::VF32 two = Maths::Const<Maths::VF32>::Two();
	Maths::Vector3 angularVelocity = Maths::Imag(dq) * two * invTimeStep;
	
	// Calculate rotational spring torque, used to align with target.
	const Maths::Vector4 pitchHeadingSpringDamping = m_pitchHeadingSpringDamping;
	Maths::Vector3 torqueSpring = rotationError * Maths::MakeVector3(Maths::X(pitchHeadingSpringDamping), Maths::Y(pitchHeadingSpringDamping), zero);
	Maths::Vector3 torqueDamping = angularVelocity * Maths::MakeVector3(Maths::Z(pitchHeadingSpringDamping), Maths::W(pitchHeadingSpringDamping), zero);
	Maths::Vector3 torque = torqueSpring - torqueDamping;
	
	// todo: clamp torque.
	torque *= invMass;
	
	// Integrate rotation, applying torque.
	Maths::Quaternion nextRotation = currRotation + dq * Maths::MakeQuaternion( Maths::MakeVec4F32( torque ) ) * halfTimeStepSquared;
	const Maths::Quaternion oldRotation = currRotation; // just for debugging.
	currRotation = Maths::Normalise( nextRotation );
	const Maths::Quaternion nonLimitedRotation = currRotation; // just for debugging.
		
	// Ensure the aim vector lies within limits around X and Y axes, relative to desired
	// aim orientation.
	const Maths::VF32 one = Maths::Const< Maths::VF32 >::One();
	if ( gSpringJointEnableLimits )
	{
		currRotation = Maths::Select( Maths::Real( currRotation ) < zero, -currRotation, currRotation );
		Maths::Vec4F32 r = Maths::GetVec4F32( currRotation );
	
		// Pitch = rotation about X, heading = rotation about Y
		const Maths::Vector4 sinPitchHeadingLimits = m_sinPitchHeadingLimits;
		
		Maths::VF32 x = Maths::Clamp(Maths::X(r), Maths::X(sinPitchHeadingLimits), Maths::Y(sinPitchHeadingLimits));
		Maths::VF32 y = Maths::Clamp(Maths::Y(r), Maths::Z(sinPitchHeadingLimits), Maths::W(sinPitchHeadingLimits));
		Maths::VF32 z = zero;
		Maths::VF32 w = Maths::Sqrt( one - x*x - y*y /* - z*z */ );
		currRotation = Maths::MakeQuaternion( x, y, z, w );;
	}
	
	// Update follower pos using new rotation.
	const Maths::VF32 targetDist = one;
	const Maths::Point3 newLocalTargetFollowerPos = Maths::MakePoint3( targetDist * ( aimAxis * currRotation ) );
	m_targetFollowerPos = desiredModelTransform.TransformPoint( newLocalTargetFollowerPos );
	
	#if 0
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(Maths::HVec3(desiredModelTransform.GetTranslation()), Maths::HVec3(aimAxis * nonLimitedRotation * desiredModelTransform.GetRotation()), 1.f, 0.3f, Maths::RGBA(0x80, 0x00, 0x30, 0xff));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dArrow(Maths::HVec3(desiredModelTransform.GetTranslation()), Maths::HVec3(m_targetFollowerPos - desiredModelTransform.GetPosition()), 1.f, 0.3f, Maths::RGBA(0x80, 0xff, 0x00, 0xff));
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(Maths::HMat43(C_JointTransform(currRotation * desiredModelTransform.GetRotation(), desiredModelTransform.GetTranslation()).AsMat43()), 0.3f);
	
	F32 d = Maths::GetF32(Maths::Length(m_targetFollowerPos - m_prevTargetFollowerPos));
	F32 dt = Maths::GetF32(oldRotation | currRotation);
	if (d > 0.4f)
	{
		volatile bool breakhere = 1;
	}
	#endif
}
	
//
// C_DynamicSliderJoint
//

C_DynamicSliderJoint::C_DynamicSliderJoint()
{
}

#ifndef PLATFORM_SPU
void C_DynamicSliderJoint::Initialise( const C_Skeleton& skeleton, const C_StringId& jointName, const Maths::Normal3& slideAxis )
{
	m_axis = slideAxis;
	
	const Maths::VF32 zeroScalar = Maths::Const<Maths::VF32>::Zero();
	const Maths::Vector3 zeroVec3 = Maths::Const<Maths::Vector3>::Zero();
	const Maths::Point3 zeroPoint3 = Maths::Const<Maths::Point3>::Zero();
	
	ClearForce();
		
	m_position = zeroPoint3;
	m_prevPosition = zeroPoint3;
	
	m_prevLocalPosition = zeroPoint3;
	m_currLocalPosition = zeroPoint3;
	
	m_stiffness = 50.f;
	m_damping = 0.5f;
	
	m_minLimit = -0.125f;
	m_maxLimit = +0.125f;
	
	SetJointIndex(skeleton.FindJointByName( jointName ));
}

void C_DynamicSliderJoint::Initialise( const C_DynamicSliderJointParams& params )
{
	const Maths::VF32 zeroScalar = Maths::Const<Maths::VF32>::Zero();
	const Maths::Vector3 zeroVec3 = Maths::Const<Maths::Vector3>::Zero();
	const Maths::Point3 zeroPoint3 = Maths::Const<Maths::Point3>::Zero();
	
	m_position = zeroPoint3;
	m_prevPosition = zeroPoint3;
	
	m_prevLocalPosition = zeroPoint3;
	m_currLocalPosition = zeroPoint3;
	
	SetSpringConstants(params.m_spring, params.m_damping);
	SetMass(params.m_mass);
	SetAirResistance(params.m_drag);
	SetWindInfluence(params.m_windInfluence);
	SetBuoyancyInfluence(params.m_buoyancyInfluence);
	ClearForce();

	m_minLimit = params.m_minLimit;
	m_maxLimit = params.m_maxLimit;
	SetJointIndex(params.m_jointIndex);
	m_axis = Maths::MakeNormal3(params.m_axis);
}

#endif

/// Set limits.
void C_DynamicSliderJoint::SetLimits( F32 minLimit, F32 maxLimit )
{
	m_minLimit = minLimit;
	m_maxLimit = maxLimit;
}

F32 C_DynamicSliderJoint::GetMinLimit() const
{
	return m_minLimit;
}

F32 C_DynamicSliderJoint::GetMaxLimit() const
{
	return m_maxLimit;
}

void C_DynamicSliderJoint::SetSpringConstants( F32 strength, F32 damping )
{
	m_stiffness = strength;
	m_damping = damping * Maths::Sqrt(strength);
}

F32 C_DynamicSliderJoint::GetSpring() const
{
	return m_stiffness;
}

F32 C_DynamicSliderJoint::GetDamping() const
{
	return m_damping / Maths::Sqrt(m_stiffness);
}

void C_DynamicSliderJoint::ApplyForce(const Maths::Vector3& force)
{
	m_force += force;
}

void C_DynamicSliderJoint::ClearForce()
{
	m_force = Maths::Const<Maths::Vector3>::Zero();
}


#ifdef ANIM_DEBUG_SPRINGS
static Debug::Float gSlideJointSpring( "Animation.Dynamics.SlideJointSpring", 20.f, Debug::Range( 0.f, 1000.f ) );
static Debug::Float gSlideJointDamping( "Animation.Dynamics.SlideJointDamping", 1.f, Debug::Range( 0.f, 1.f ) );
#endif

void C_DynamicSliderJoint::SimulateAndApply( C_SkeletonPose& skeletonPose, const C_DynamicJointContext& context )
{
	if ( IsInvalid() || !context.enabled )
	{
		return;
	}
	
	//PERF_EVENT("DynamicSliderJoint.Evaluate");

	const Maths::VF32 timeStep = Maths::MakeVF32( context.fixedTimeStep );
	const Maths::VF32 invTimeStep = Maths::MakeVF32( context.invFixedTimeStep );

	// If we update greater than the animation rate then only the first iteration will carry the correct inherited velocity.
	// Therefore, we limit the dynamics to one iteration per Evaluate call.
	if ( context.ShouldSimulate() )
	{
		C_JointTransform modelParentJointTransform = skeletonPose.GetModelParentJointTransform( GetJointIndex() );
		C_JointTransform modelJointTransform = skeletonPose.GetLocalJointTransform( GetJointIndex() ) * modelParentJointTransform;
		
		C_JointTransform parentJointTransform = modelParentJointTransform;

		if (context.reset)
		{
			m_currLocalPosition = skeletonPose.GetLocalJointTransform( GetJointIndex() ).GetPosition();
			m_prevLocalPosition = m_currLocalPosition;
			m_position = parentJointTransform.TransformPoint(m_currLocalPosition);
			m_prevPosition = m_position;
		}
		
		const Maths::VF32 invMass = Maths::MakeVF32( m_oneOverMass );
		const Maths::VF32 timeStepSquared = timeStep * timeStep;
		
		#ifdef ANIM_DEBUG_SPRINGS
		const Maths::VF32 velocityDamping = Maths::MakeVF32( gSpringJointVelocityDamping );
		const Maths::VF32 stiffness = Maths::MakeVF32( gSlideJointSpring);
		const Maths::VF32 damping = Maths::MakeVF32( gSlideJointDamping ) * Maths::Sqrt( stiffness );
		#else
		const Maths::VF32 stiffness = Maths::MakeVF32(m_stiffness);
		const Maths::VF32 damping = Maths::MakeVF32(m_damping);
		const Maths::VF32 velocityDamping = Maths::MakeVF32(m_oneMinusDrag);
		#endif
		
		// Calculate a spring force to pull the joint towards its animated position.
		Maths::Point3 desiredJointPos = modelJointTransform.GetPosition();
		Maths::Vector3 posError = desiredJointPos - m_position;
		Maths::Vector3 velError = -( m_position - m_prevPosition ) * invTimeStep;
		Maths::Vector3 springForce = posError * stiffness + velError * damping;
	
		
		Maths::Vector3 gravity = context.modelGravity;
		Maths::Vector3 externalForce = context.worldModelTransform.TransformVector(context.force + m_force);
		
		const Maths::Vector3 accel = context.modelGravity + invMass * (externalForce + springForce);
						
		// Simulate the sliding joint's movement using Verlet integration.
		Maths::Point3 newPos = m_position + ( m_position - m_prevPosition ) * velocityDamping + accel * timeStepSquared;
		m_prevPosition = m_position;
		m_position = newPos;
		
		if (gSpringJointEnableLimits)
		{
			// Constrain the joint's position to lie along the current slide axis within the defined limits.
			const Maths::Quaternion limitFrameRot = parentJointTransform.GetRotation();
			const Maths::Point3 limitFramePos = parentJointTransform.GetPosition();
			
			const Maths::Normal3 modelSlideAxis = m_axis * limitFrameRot;
			Maths::VF32 distAlongAxis = modelSlideAxis | ( m_position - limitFramePos );
			Maths::VF32 clampedDistAlongAxis = Maths::Clamp( distAlongAxis, Maths::MakeVF32( m_minLimit ), Maths::MakeVF32( m_maxLimit ) );
			Maths::Point3 constrainedPos = limitFramePos + ( modelSlideAxis * clampedDistAlongAxis );
			m_position = constrainedPos;
		}
		
		// Calculate local space position.
		m_prevLocalPosition = m_currLocalPosition;
		m_currLocalPosition = parentJointTransform.Inverse().TransformPoint( m_position );
		
		if (context.reset)
		{
			m_prevLocalPosition = m_currLocalPosition;
		}
	}
	else if (context.reset)
	{
		m_currLocalPosition = skeletonPose.GetLocalJointTransform( GetJointIndex() ).GetPosition();
		m_prevLocalPosition = m_currLocalPosition;
	}
	
	// Smoothly blend between updates.
	const Maths::VF32 alpha = Maths::MakeVF32( context.GetInterpolationAmount() );
	const Maths::Point3 jointPos = Maths::Lerp( m_prevLocalPosition, m_currLocalPosition, alpha );
	skeletonPose.SetLocalJointTranslation( GetJointIndex(), Maths::MakeVector3( jointPos ) );
	
	ClearForce();
}

} // namespace Animation
