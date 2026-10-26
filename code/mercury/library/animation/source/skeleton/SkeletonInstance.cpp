#include "stdafx.h"
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/skeleton/WindParams.h>
#include <animation/skeleton/DynamicJointConstraints.h>
#include <animation/skeleton/WindGenerator.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>
#include "DynamicJointContext.h"

namespace Animation
{

DECLARE_DEBUG_INT(gDynamicJointUpdateRate, "Animation.Constraints.DynamicJointUpdateRate", 30, Debug::Range( 1, 120 ) );

C_SkeletonInstance::C_SkeletonInstance():
	m_skeleton(NULL),
	m_numDynamicAimJoints(0),
	m_numDynamicSliderJoints(0),
	m_timeLeft(0.f),
	m_dynamicJointsEnabled(true),
	m_hasConstraints(false),
	m_hasDynamicJoints(false)
{
	SetGravity(Maths::MakeVector3(0.f, -9.81f, 0.f));
	m_windTime = 0;
}

#if !defined(PLATFORM_SPU)

void C_SkeletonInstance::Create( const C_Skeleton& skeleton )
{
	m_skeleton = &skeleton;
	
	m_dynamicJointData.Destroy();
	
	const C_SkeletonConstraints& constraints = skeleton.GetConstraints();
	m_numDynamicAimJoints = constraints.GetNumConstraintsByType( E_ConstraintType_DynamicAimJoint );
	m_numDynamicSliderJoints = constraints.GetNumConstraintsByType( E_ConstraintType_DynamicSliderJoint );
	
	// Allocate memory block containing dynamic joint data.
	C_MemoryBlockSizer sizer;
	sizer.Allocate( m_dynamicAimJoints, m_numDynamicAimJoints );
	sizer.Allocate( m_dynamicSliderJoints, m_numDynamicSliderJoints );
	m_dynamicJointData.Create( sizer );	
	
	// Initialise dynamic joints using constant parameters from skeleton.
	constraints.InitialiseDynamicJoints(
		MakeArrayProxy( m_dynamicAimJoints( m_dynamicJointData ), m_numDynamicAimJoints ),
		MakeArrayProxy( m_dynamicSliderJoints( m_dynamicJointData ), m_numDynamicSliderJoints )
	);
	
	SetGravity(Maths::MakeVector3(0.f, -9.81f, 0.f));
	SetDynamicJointsEnabled(true);
	
	if (constraints.GetNumConstraints() > 0)
	{
		m_hasConstraints = true;
	}
	else
	{
		m_hasConstraints = false;
	}

	if (m_numDynamicAimJoints + m_numDynamicSliderJoints > 0)
	{
		m_hasDynamicJoints = true;
	}
	else
	{
		m_hasDynamicJoints = false;
	}
	
	Reset();
}

void C_SkeletonInstance::Destroy()
{
	m_dynamicJointData.Destroy();
	m_skeleton = NULL;
	m_numDynamicAimJoints = 0;
	m_numDynamicSliderJoints = 0;
}

/**
Simple utility class for mapping between skeletons.
*/
class C_SkeletonMapper
{
public:

	C_SkeletonMapper(const C_Skeleton& oldSkeleton, const C_Skeleton& newSkeleton):
		m_oldSkeleton(oldSkeleton),
		m_newSkeleton(newSkeleton)
	{
	}

	/// Map joint index from old skeleton to new skeleton.
	S32 MapJointIndex(S32 oldJointIndex) const
	{
		C_StringId oldJointName = m_oldSkeleton.GetJointName(oldJointIndex);
		S32 newJointIndex = m_newSkeleton.FindJointByName(oldJointName);
		return newJointIndex;
	}

private:
	const C_Skeleton&	m_oldSkeleton;
	const C_Skeleton&	m_newSkeleton;
};

void C_SkeletonInstance::Remap( const C_Skeleton& newSkeleton )
{
	if (m_skeleton == &newSkeleton) return;
	if (m_skeleton == NULL) return;
	
	const C_Skeleton& oldSkeleton = *m_skeleton;
	
	C_SkeletonMapper mapper(oldSkeleton, newSkeleton);
	
	// Remap dynamic aim joints.
	C_DynamicAimJoint* dynamicAimJoints = m_dynamicAimJoints(m_dynamicJointData);
	for (U32 i = 0, count = m_numDynamicAimJoints; i < count; ++i)
	{
		S32 newJointIndex = mapper.MapJointIndex(dynamicAimJoints[i].GetJointIndex());
		dynamicAimJoints[i].SetJointIndex(newJointIndex);

		S32 newTargetJointIndex = mapper.MapJointIndex(dynamicAimJoints[i].GetTargetJointIndex());
		dynamicAimJoints[i].SetJointIndex(newTargetJointIndex);
	}		

	// Remap dynamic slider joints.
	C_DynamicSliderJoint* dynamicSliderJoints = m_dynamicSliderJoints(m_dynamicJointData);
	for (U32 i = 0, count = m_numDynamicSliderJoints; i < count; ++i)
	{
		S32 newJointIndex = mapper.MapJointIndex(dynamicSliderJoints[i].GetJointIndex());
		dynamicSliderJoints[i].SetJointIndex(newJointIndex);
	}
	
	// Switch to new skeleton.
	m_skeleton = &newSkeleton;	
}


#endif

void C_SkeletonInstance::Reset()
{
	m_reset = true;
	m_timeLeft = 0.f;
	m_windTime = 0;
}

void C_SkeletonInstance::SpuFetchAsync(U32 dmaTag)
{
#ifdef PLATFORM_SPU
	m_dynamicJointData.SpuFetchAsync(dmaTag);
#endif
}

void C_SkeletonInstance::SpuFetch()
{
	SpuFetchAsync(0);
	Spu::Wait(0);
}

void C_SkeletonInstance::SpuStore()
{
#ifdef PLATFORM_SPU
	// DMA back updated dynamic joint data.
	m_dynamicJointData.SpuStore();
#endif
}

void C_SkeletonInstance::ApplyJointConstraints( C_Pose& pose, const C_JointTransform& modelWorldTransform, const C_Skeleton* skeleton )
{
	Spu::MemBaseMarker memBaseMarker;
	
	if (skeleton == NULL)
	{
		if (m_skeleton == NULL)
		{
			return;
		}
		skeleton = m_skeleton;
	}

	if (skeleton->GetNumJoints() == 0)
	{
		return;
	}
	
	// Update dynamic joint context.
	m_fixedTimeStep =  1.f / gDynamicJointUpdateRate;
	
	C_DynamicJointContext dynamicJointContext( modelWorldTransform );
	
	dynamicJointContext.SetFixedTimeStep( m_fixedTimeStep );
	dynamicJointContext.timeLeft= m_timeLeft;
	dynamicJointContext.SetGravity(m_gravity);
	dynamicJointContext.enabled = m_dynamicJointsEnabled;
	dynamicJointContext.reset = m_reset;
	m_reset = false;

	// Apply dynamic joints without root motion.
	C_JointTransform savedRootTransform = pose.GetJointTransform(0);
	pose.GetJointTransform(0).SetIdentity();

	// Update and apply constraints.
	C_SkeletonPose skeletonPose( *skeleton, pose );
	skeleton->GetConstraints().ApplyConstraints(
		skeletonPose,
		MakeArrayProxy( m_dynamicAimJoints(m_dynamicJointData), m_numDynamicAimJoints ),
		MakeArrayProxy( m_dynamicSliderJoints(m_dynamicJointData), m_numDynamicSliderJoints ),
		dynamicJointContext
	);

	// Eat up any remaining time as dynamic joints work best when updated no more frequently than the animation.
	m_timeLeft = fmodf( m_timeLeft, m_fixedTimeStep );

	// Restore root transform.
	pose.GetJointTransform(0) = savedRootTransform;
}

void C_SkeletonInstance::UpdateDynamicJoints( F32 timeStep )
{
	m_timeLeft += Maths::Abs(timeStep);
}

Maths::Vector3 C_SkeletonInstance::GetGravity() const
{
	return m_gravity;
}

void C_SkeletonInstance::SetGravity(const Maths::Vector3& g)
{
	m_gravity = g;
}

void C_SkeletonInstance::SetDynamicJointsEnabled(bool enabled)
{
	m_dynamicJointsEnabled = enabled;
}

bool C_SkeletonInstance::AreDynamicJointsEnabled() const
{
	return m_dynamicJointsEnabled;
}

U32 C_SkeletonInstance::GetNumDynamicJoints() const
{
	return m_numDynamicAimJoints + m_numDynamicSliderJoints;
}

void C_SkeletonInstance::ApplyForceToDynamicJoint(S32 instance, const Maths::Vector3& force)
{
	if (instance < 0) return;
	
	if (instance < m_numDynamicAimJoints)
	{
		S32 aimJointIndex = instance;
		m_dynamicAimJoints(m_dynamicJointData)[aimJointIndex].ApplyForce(force);
	}
	else
	{
		S32 sliderJointIndex = instance - m_numDynamicAimJoints;
		if (sliderJointIndex < m_numDynamicSliderJoints)
		{
			m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex].ApplyForce(force);
		}
	}
}

void C_SkeletonInstance::ApplyWindForceToDynamicJoint(S32 instance, const Maths::Vector3& force)
{
	if (instance < 0) return;
	
	if (instance < m_numDynamicAimJoints)
	{
		S32 aimJointIndex = instance;
		C_DynamicAimJoint& aimJoint = m_dynamicAimJoints(m_dynamicJointData)[aimJointIndex];
		aimJoint.ApplyForce(force * Maths::MakeVF32(aimJoint.GetWindInfluence()));
	}
	else
	{
		S32 sliderJointIndex = instance - m_numDynamicAimJoints;
		if (sliderJointIndex < m_numDynamicSliderJoints)
		{
			C_DynamicSliderJoint& sliderJoint = m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex];
			sliderJoint.ApplyForce(force * Maths::MakeVF32(sliderJoint.GetWindInfluence()));
		}
	}
}

void C_SkeletonInstance::ApplyWindForces(const C_WindParams& windParams, F32 timeStep)
{
	C_WindGenerator windGenerator;
	windGenerator.SetTime(m_windTime);
	windGenerator.Update(windParams, timeStep);

	U32 seed = 0;
	for (U32 i = 0, count = m_numDynamicAimJoints; i < count; ++i, ++seed)
	{
		C_DynamicAimJoint& aimJoint = m_dynamicAimJoints(m_dynamicJointData)[i];
		Maths::Vector3 force = windGenerator.GenerateWindForce(seed);
		aimJoint.ApplyForce(force * Maths::MakeVF32(aimJoint.GetWindInfluence()));
	}
	for (U32 i = 0, count = m_numDynamicSliderJoints; i < count; ++i, ++seed)
	{
		C_DynamicSliderJoint& sliderJoint = m_dynamicSliderJoints(m_dynamicJointData)[i];
		Maths::Vector3 force = windGenerator.GenerateWindForce(seed);
		sliderJoint.ApplyForce(force * Maths::MakeVF32(sliderJoint.GetWindInfluence()));
	}
	m_windTime = windGenerator.GetTime();
}

E_DynamicJointType C_SkeletonInstance::GetDynamicJointType(U32 instance) const
{
	if (instance < m_numDynamicAimJoints)
	{
		return E_DynamicJointType_Aim;
	}
	else if (instance < (static_cast<U32>(m_numDynamicAimJoints) + m_numDynamicSliderJoints))
	{
		return E_DynamicJointType_Slider;
	}
	return E_DynamicJointType_Invalid;
}

S32 C_SkeletonInstance::FindDynamicJoint(S32 skeletonJointIndex) const
{
	if (skeletonJointIndex <= 0) return -1;

	for (U32 i = 0, count = m_numDynamicAimJoints; i < count; ++i)
	{
		if (m_dynamicAimJoints(m_dynamicJointData)[i].GetJointIndex() == skeletonJointIndex)
		{
			return i;
		}
	}
	for (U32 i = 0, count = m_numDynamicSliderJoints; i < count; ++i)
	{
		if (m_dynamicSliderJoints(m_dynamicJointData)[i].GetJointIndex() == skeletonJointIndex)
		{
			return i + m_numDynamicAimJoints;
		}
	}
	
	return -1;
}

S32 C_SkeletonInstance::GetDynamicJointSkeletonJointIndex(U32 instance) const
{
	if (instance < m_numDynamicAimJoints)
	{
		return m_dynamicAimJoints(m_dynamicJointData)[instance].GetJointIndex();
	}
	else if (instance < (static_cast<U32>(m_numDynamicAimJoints) + m_numDynamicSliderJoints))
	{
		return m_dynamicSliderJoints(m_dynamicJointData)[instance - m_numDynamicAimJoints].GetJointIndex();
	}
	return -1;
}

//
// Common dynamic joint settings.
//

#if !defined(PLATFORM_SPU)

void C_SkeletonInstance::SetDynamicJointMass(U32 instance, F32 mass )
{
	C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		j->SetMass(mass);
	}
}

F32 C_SkeletonInstance::GetDynamicJointMass(U32 instance) const
{
	F32 mass = 0.f;
	const C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		mass = j->GetMass();
	}
	return mass;
}

void C_SkeletonInstance::SetDynamicJointAirResistance(U32 instance, F32 airResistance )
{
	C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		j->SetAirResistance(airResistance);
	}
}

F32 C_SkeletonInstance::GetDynamicJointAirResistance(U32 instance) const
{
	F32 a = 0.f;
	const C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		a = j->GetAirResistance();
	}
	return a;
}

void C_SkeletonInstance::SetDynamicJointWindInfluence(U32 instance, F32 influence)
{
	C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		j->SetWindInfluence(influence);
	}
}

F32 C_SkeletonInstance::GetDynamicJointWindInfluence(U32 instance) const
{
	F32 a = 0.f;
	const C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		a = j->GetWindInfluence();
	}
	return a;	
}

void C_SkeletonInstance::SetDynamicJointBuoyancyInfluence(U32 instance, F32 influence)
{
	C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		j->SetBuoyancyInfluence(influence);
	}
}

F32 C_SkeletonInstance::GetDynamicJointBuoyancyInfluence(U32 instance) const
{
	F32 a = 0.f;
	const C_DynamicJoint* j = GetDynamicJoint(instance);
	if (j != NULL)
	{
		a = j->GetBuoyancyInfluence();
	}
	return a;	
}


//
// Dynamic slider joint settings.
//

void C_SkeletonInstance::SetDynamicSliderJointSpringConstants(U32 instance, F32 spring, F32 damping)
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Slider) return;
	U32 sliderJointIndex = instance - m_numDynamicAimJoints;
	m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex].SetSpringConstants(spring, damping);
}

void C_SkeletonInstance::GetDynamicSliderJointSpringConstants(U32 instance, F32* outSpring, F32* outDamping) const
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Slider)
	{
		if (outSpring) *outSpring = 0.f;
		if (outDamping) *outDamping = 0.f;
		return;
	}
	
	U32 sliderJointIndex = instance - m_numDynamicAimJoints;
	
	if (outSpring)
		*outSpring = m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex].GetSpring();

	if (outDamping)
		*outDamping = m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex].GetDamping();
}

void C_SkeletonInstance::SetDynamicSliderJointLimits(U32 instance, F32 minLimit, F32 maxLimit)
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Slider) return;
	U32 sliderJointIndex = instance - m_numDynamicAimJoints;
	m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex].SetLimits(minLimit, maxLimit);
}

void C_SkeletonInstance::GetDynamicSliderJointLimits(U32 instance, F32* outMin, F32* outMax) const
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Slider) return;
	U32 sliderJointIndex = instance - m_numDynamicAimJoints;
	if (outMin)
		*outMin = m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex].GetMinLimit();
	if (outMax)
		*outMax = m_dynamicSliderJoints(m_dynamicJointData)[sliderJointIndex].GetMaxLimit();
}

//
// Dynamic aim joint settings.
//

void C_SkeletonInstance::SetDynamicAimJointTarget( U32 instance, S32 targetJointIndex )
{
	if (GetDynamicJointType(instance) == E_DynamicJointType_Aim)
	{
		m_dynamicAimJoints(m_dynamicJointData)[instance].SetTargetJointIndex(targetJointIndex);
	}
}

void C_SkeletonInstance::GetDynamicAimJointSpringConstants(U32 instance, F32* outPitchSpring, F32* outPitchDamping, F32* outHeadingSpring, F32* outHeadingDamping) const
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Aim) return;
	m_dynamicAimJoints(m_dynamicJointData)[instance].GetSpringConstants(outPitchSpring, outPitchDamping, outHeadingSpring, outHeadingDamping);
}

void C_SkeletonInstance::SetDynamicAimJointSpringConstants(U32 instance, F32 pitchSpring, F32 pitchDamping, F32 headingSpring, F32 headingDamping)
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Aim) return;
	m_dynamicAimJoints(m_dynamicJointData)[instance].SetSpringConstants(pitchSpring, pitchDamping, headingSpring, headingDamping);
}

void C_SkeletonInstance::GetDynamicAimJointLimits(U32 instance, F32* outPitchMin, F32* outPitchMax, F32* outHeadingMin, F32* outHeadingMax) const
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Aim) return;
	m_dynamicAimJoints(m_dynamicJointData)[instance].GetLimits(outHeadingMin, outHeadingMax, outPitchMin, outPitchMax);
}

void C_SkeletonInstance::SetDynamicAimJointLimits(U32 instance, F32 pitchMin, F32 pitchMax, F32 headingMin, F32 headingMax)
{
	if (GetDynamicJointType(instance) != E_DynamicJointType_Aim) return;
	m_dynamicAimJoints(m_dynamicJointData)[instance].SetLimits(headingMin, headingMax, pitchMin, pitchMax);
}

S32 C_SkeletonInstance::GetDynamicAimJointTargetJointIndex(U32 instance) const
{
	S32 index = -1;
	if (GetDynamicJointType(instance) == E_DynamicJointType_Aim)
	{
		index = m_dynamicAimJoints(m_dynamicJointData)[instance].GetTargetJointIndex();
	}
	return index;
}


#endif

C_DynamicJoint*	C_SkeletonInstance::GetDynamicJoint(U32 instance)
{
	C_DynamicJoint* j = NULL;
	if (instance < m_numDynamicAimJoints)
	{
		j = &m_dynamicAimJoints(m_dynamicJointData)[instance];
	}
	else if (instance < GetNumDynamicJoints())
	{
		j = &m_dynamicSliderJoints(m_dynamicJointData)[instance - m_numDynamicAimJoints];
	}
	return j;
}

const C_DynamicJoint* C_SkeletonInstance::GetDynamicJoint(U32 instance) const
{
	const C_DynamicJoint* j = NULL;
	if (instance < m_numDynamicAimJoints)
	{
		j = &m_dynamicAimJoints(m_dynamicJointData)[instance];
	}
	else if (instance < GetNumDynamicJoints())
	{
		j = &m_dynamicSliderJoints(m_dynamicJointData)[instance - m_numDynamicAimJoints];
	}
	return j;
}


} // namespace Animation

