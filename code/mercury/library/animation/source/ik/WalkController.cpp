#include "stdafx.h"
#include <animation/ik/WalkController.h>
#include <animation/ik/IkChain.h>
#include <animation/cycle/Cycle.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/pose/SkeletonPose.inl>
#include <animation/skeleton/Skeleton.h>
#include <animation/common/AnimationMaths.h>
#include <core/maths/hvec/new/IncludeGeometry.h>
#include <core/maths/hvec/new/IncludeGeometry.inl>

#ifndef CFG_GOLD
#include <system/debug/I_DebugRender.h>
#endif

namespace Animation
{

//
// C_StepInfo
//

//
// C_GaitInfo
//

C_GaitInfo::C_GaitInfo()
{
}

void C_GaitInfo::CreateFromCycle(const S16* footJointIndices, U32 numLegs, const C_Skeleton& skeleton, const C_Cycle& cycle)
{
	// Distance above ground plane, below it is assumed the foot is on the floor.
	const Maths::VF32 threshold = Maths::MakeVF32(0.005f);
	const Maths::VF32 footThickness = Maths::MakeVF32(0.0f);
	
	Maths::Plane groundPlane = Maths::MakePlane(Maths::Const<Maths::Normal3>::AxisY(), Maths::Const<Maths::Point3>::Zero());
	
	Animation::C_PoseFormat poseFormat(&skeleton);
	Animation::C_Pose pose(poseFormat);
	Animation::C_CycleInstance cycleInstance(&cycle, &pose);
	Animation::C_SkeletonPose skeletonPose(skeleton, pose);
	
	bool footDown[kMaxWalkControllerLegs];
	F32 supportStart[kMaxWalkControllerLegs];
	
	m_numLegs = numLegs;
	for (U32 i = 0; i < numLegs; ++i)
	{
		footDown[i] = false;
		supportStart[i] = 0.f;
	}
	
	// Set gait period.
	m_gaitPeriod = cycle.GetDuration();
		
	U32 numFrames = static_cast<U32>(cycleInstance.GetDuration() * cycleInstance.GetFrameRate());
	F32 timeScale = 1.f / cycleInstance.GetFrameRate();
	
	for (U32 frame = 0; frame < numFrames; ++frame)
	{
		F32 t = frame * timeScale;
		F32 phase = t / m_gaitPeriod;
		
		pose.Clear();
		
		cycleInstance.GetCycle()->Evaluate(
			Animation::E_BlendMode_Accumulate,
			t,
			cycleInstance,
			0,
			1.f,
			pose
		);
		
		pose.Normalise(&skeleton);
		
		for (U32 legIndex = 0; legIndex < numLegs; ++legIndex)
		{
			if (footJointIndices[legIndex] < 0) continue;
			
			Maths::Point3 footPos = Maths::MakePoint3(skeletonPose.GetModelJointTranslation(footJointIndices[legIndex]));
			
			if (Maths::Distance(groundPlane, footPos) - footThickness < threshold)
			{
				if (frame > 0)
				{
					if (!footDown[legIndex])
					{
						// This is the time at which the support phase begins.
						supportStart[legIndex] = phase;
					}
				}
				footDown[legIndex] = true;
			}
			else
			{
				if (footDown[legIndex])
				{
					// Foot no longer down - start of step phase and end of the support phase.
					m_stepInfo[legIndex].m_stepTrigger = phase;
				}
				
				footDown[legIndex] = false;
			}
		}
	}
	
	for (U32 legIndex = 0; legIndex < numLegs; ++legIndex)
	{
		F32 supportTime = 0.f;
		F32 stepStart = m_stepInfo[legIndex].m_stepTrigger;
		F32 stepEnd = supportStart[legIndex];
		
		F32 dutyFactor = stepEnd - stepStart;
		
		if (dutyFactor < 0.f)
		{
			dutyFactor += 1.f;
		}
		m_stepInfo[legIndex].m_dutyFactor = dutyFactor;
	}
}

void C_GaitInfo::Clear()
{
}
	
void C_GaitInfo::BlendInto(C_GaitInfo& dest, Animation::E_BlendMode blendMode, F32 weight) const
{
}

void C_GaitInfo::Normalise()
{
}

F32 C_GaitInfo::GetPeriod() const
{
	return m_gaitPeriod;
}

//
// C_StepController
//

// needs


C_StepController::C_StepController():
	m_stepInfo(NULL),
	m_stepping(false),
	m_currentFootPosValid(false)
{
	Reset();
}

void C_StepController::Reset()
{
	m_stepping = false;
	m_currentFootPosValid = false;
	m_onGround = false;
	m_groundNormal = Maths::Const<Maths::Normal3>::AxisY();
	//m_groundPlane = Maths::MakePlane(Maths::Const<Maths::Normal3>::AxisY(), Maths::Const<Maths::Point3>::Zero());
	m_heightDelta = 0.f;
	m_groundHeight = 0.f;	
	m_prevGroundHeight = 0.f;	
	m_stepPhase = 0.f;
}

// Relative to hip.
void C_StepController::SetCurrentFootPosition(const Maths::Point3& footPos)
{
	m_currentFootPos = footPos;
	m_currentFootPosValid = true;
}

Debug::Float gDebugStepThreshold("Animation.WalkController.StepThreshold", 0.1f, Debug::Range(0.f, 0.5f));

void C_StepController::Update(
	F32 lastGaitPhase,
	F32 gaitPhase,
	F32 gaitPeriod,
	F32 smoothing,
	const Maths::Vector3& rootVelocity,
	const Maths::Mat43& modelWorldMatrix,
	bool onGround,
	I_GroundCollision* groundCollision
)
{
	const Maths::Normal3 upDir = Maths::Const<Maths::Normal3>::AxisY();	// negative gravity direction
	
	F32 rootGroundHeight = Maths::GetF32(Maths::RowW(modelWorldMatrix) | upDir);
	F32 newGroundHeight = rootGroundHeight;

	if (m_stepInfo != NULL)
	{
		//
		// Use gait information to predict future footstep position.
		//
	
		if (m_stepping)
		{
			F32 deltaPhase = gaitPhase - lastGaitPhase;
			if (deltaPhase < 0.f)
			{
				deltaPhase += 1.f;
			}
			BIZ_ASSERT(deltaPhase >= 0.f);
			m_stepPhase += deltaPhase;
			
			// Finished step?
			if (m_stepPhase > (1.f - m_stepInfo->m_dutyFactor))
			{
				m_stepping = false;
				m_prevGroundHeight = m_groundHeight;
			}
		}
		else
		{			
			// Just passed step trigger?
			if (lastGaitPhase < m_stepInfo->m_stepTrigger && gaitPhase > m_stepInfo->m_stepTrigger)
			{
				m_stepping = true;
				m_stepPhase = (gaitPhase - m_stepInfo->m_stepTrigger); // relative to gait phase
				BIZ_ASSERT(m_stepPhase >= 0.f);
			}
		}
		
		//
		// TODO: prediction broken, revisit in future.
		//
		if (m_stepping)
		{
			// Use approximate current foot position translated forwards by (1 - transferPhase) * 0.5 of root displacement.
			F32 transferPhase = m_stepPhase / (1.f - m_stepInfo->m_dutyFactor);
			F32 newFutureGroundHeight = rootGroundHeight;
			Maths::Point3 currentWorldFootPos = m_currentFootPos * modelWorldMatrix;
			Maths::Point3 futureFootPos = currentWorldFootPos;
			Maths::Vector3 v = rootVelocity;
			if (m_currentFootPosValid)
			{
				Maths::VF32 k = Maths::MakeVF32((1.f - transferPhase) * 0.75f);
				futureFootPos = currentWorldFootPos + k * v;
			}
			m_futureFootPos = futureFootPos;
			
			if (onGround)
			{
				Maths::Point3 traceStart = futureFootPos;
				Maths::VF32 traceRadius = Maths::MakeVF32(1.f);
			
				if (groundCollision->TraceRay(traceStart + upDir * traceRadius, traceStart - upDir * traceRadius, &m_groundContactPoint, &m_groundContactNormal))
				{
					newFutureGroundHeight = Maths::GetF32(m_groundContactPoint | upDir);
					m_futureFootPos = m_groundContactPoint;
				}
				else
				{
					onGround = false;
				}
			}
			
			// Smooth future ground height position.
			m_futureGroundHeight = m_futureGroundHeight + (newFutureGroundHeight - m_futureGroundHeight) * smoothing;
						
			// Current height is interpolated between future and previous step ground height.
			// Without smoothing?
			newGroundHeight = m_prevGroundHeight + (m_futureGroundHeight - m_prevGroundHeight) * transferPhase;
			smoothing = 1.f;
			m_groundHeight = newGroundHeight;
			m_heightDelta = m_groundHeight - rootGroundHeight;
			return;
		}
		else
		{
			// Keep ground height.
			newGroundHeight = m_prevGroundHeight;
		}
	}
	else
	{
		//
		// No gait information, so adapt foot height from the current pose state.
		//
		Maths::Point3 traceStart = Maths::MakePoint3(Maths::RowW(modelWorldMatrix));
		Maths::VF32 traceRadius = Maths::MakeVF32(1.f);
		
		if (m_currentFootPosValid)
		{
			traceStart = m_currentFootPos * modelWorldMatrix;
		}
		
		if (onGround)
		{
			if (groundCollision->TraceRay(traceStart + upDir * traceRadius, traceStart - upDir * traceRadius, &m_groundContactPoint, &m_groundContactNormal))
			{
				newGroundHeight = Maths::GetF32(m_groundContactPoint | upDir);
			}
			else
			{
				onGround = false;
			}
		}
	}
	
	if (onGround)
	{
		if (!m_onGround)
		{
			m_groundHeight = newGroundHeight;
		}
		
		// If foot is on the ground then blend its height towards the correct ground height.
		// Using absolute ground heights prevents the overshoot that can occur when smoothing deltas.
		// However, whilst walking up a slope feet will sink into the ground.
		// And whilst walking down a slope, feet will float above the ground.
		// Deltas work better on slopes due to movement of the character root acting as a trend line.
		
		// FIXME: how would step up animations work? foot would end up being lifted even further...
		// Need to disable IK when foot is off the ground, unless it's intersecting with ground.
		
		F32 lift = newGroundHeight - m_groundHeight;
		
		// Instantly adapt foot to correct height if it needs to be lifted clear by a small amount.
		// For larger lifts (e.g. climbing stairs), minimising popping is more important than avoiding intersection.
		if (lift >= 0.f && lift < gDebugStepThreshold)
		{
			m_groundHeight = newGroundHeight;
		}

		m_groundHeight = m_groundHeight + (newGroundHeight - m_groundHeight) * smoothing;
		m_heightDelta = (m_groundHeight - rootGroundHeight);
		
		// Blend ground normal towards contact normal.
		Maths::Vector3 smoothNormal = Maths::Lerp(Maths::MakeVector3(m_groundNormal), m_groundContactNormal, Maths::MakeVF32(smoothing));
		m_groundNormal = Maths::Normalise(smoothNormal);
	}
	else
	{
		// If foot not in contact with ground then use a smoothed height delta to stop it lagging behind the character.
		F32 newHeightDelta = newGroundHeight - rootGroundHeight;
		m_heightDelta = m_heightDelta + (newHeightDelta - m_heightDelta) * smoothing;
		
		// Blend ground normal towards "up" direction.
		Maths::Vector3 smoothNormal = Maths::Lerp(Maths::MakeVector3(m_groundNormal), Maths::MakeVector3(upDir), Maths::MakeVF32(smoothing));
		m_groundNormal = Maths::Normalise(smoothNormal);
	}
	
	m_onGround = onGround;
}

F32 C_StepController::GetHeightDelta() const
{
	return m_heightDelta;
}

Maths::Normal3 C_StepController::GetGroundNormal() const
{
	return m_groundNormal;
}

F32 C_StepController::GetGroundHeight() const
{
	return m_groundHeight;
}

#ifndef CFG_GOLD
void C_StepController::DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& modelWorldMatrix) const
{
	F32 size = 0.04f;

	debugRender->Draw3dPlaneCross(
		Maths::HVec3(m_groundContactPoint),
		Maths::HVec3(m_groundContactNormal),
		size,
		Maths::RGBA(0xff, 0xff, 0x00)
	);

	debugRender->Draw3dCircle(
		Maths::HVec3(m_futureFootPos),
		Maths::HVec3(m_groundContactNormal),
		0.1f,
		m_stepping ? Maths::RGBA(0xff, 0xff, 0x00) : Maths::RGBA(0xff, 0x40, 0x40)
	);
}
#endif

//
// C_WalkController
// 

Debug::Bool gDebugEnableHipLowering("Animation.WalkController.EnableHipLowering", true);
Debug::Bool gDebugEnableFootstepPrediction("Animation.WalkController.EnableFootstepPrediction", false);
Debug::Float gDebugGroundHeightSmoothing("Animation.WalkController.GroundHeightSmoothing", 0.1f, Debug::Range(0.f, 1.f));
Debug::Float gDebugHipHeightSmoothing("Animation.WalkController.HipHeightSmoothing", 0.1f, Debug::Range(0.f, 1.f));

C_WalkController::C_WalkController():
	m_numLegs(0),
	m_hipHeightSmoothing(0.5f),
	m_groundHeightSmoothing(0.6f),
	m_hipJointIndex(-1),
	m_hipHeightDelta(0.f),
	m_gaitPhase(0.f)
{
}

void C_WalkController::Clear()
{
	m_numLegs = 0;
	m_minLegLength = FLT_MAX;
	Reset();
}

bool C_WalkController::AddLeg(C_IkChain& ikChain, U32 ikTargetIndex, S32 footJointIndex, F32 footToGroundHeight)
{
	if (m_numLegs >= kMaxWalkControllerLegs)
	{
		return false;
	}
	U32 legIndex = m_numLegs;
	
	m_legIkTargetIndices[legIndex] = ikTargetIndex;
	m_legIkChains[legIndex] = &ikChain;	// should controller own IK chains or store pointers?
	m_footJointIndices[legIndex] = footJointIndex;
	m_footToGroundHeights[legIndex] = footToGroundHeight;
	
	m_minLegLength = Maths::Min(ikChain.GetTotalLength(), m_minLegLength);
	
	++m_numLegs;
	
	return true;
}

U32 C_WalkController::GetNumLegs() const
{
	return m_numLegs;
}

void C_WalkController::Reset()
{
	m_hipHeightDelta = 0.f;
	m_gaitPhase = 0.f;
	for (U32 i = 0; i < m_numLegs; ++i)
	{
		m_stepControllers[i].Reset();
	}
}

void C_WalkController::UpdateFootPositions(
	const C_Pose& pose,
	const C_Skeleton* skeleton
)
{
	if (skeleton != NULL)
	{
		C_SkeletonPose skeletonPose(*skeleton, const_cast<C_Pose&>(pose));
		
		for (U32 i = 0; i < m_numLegs; ++i)
		{
			Maths::Point3 pos = Maths::MakePoint3(skeletonPose.GetModelJointTranslation(m_footJointIndices[i]));
			m_stepControllers[i].SetCurrentFootPosition(pos);
		}
	}
	
}

// SmoothTime is approximate time it will take to reach 1/e of value.
F32 GetTimeAdjustedSmoothing(F32 smoothTime, F32 deltaTime)
{
	F32 invSmoothTime = Maths::Reciprocal(smoothTime);
	F32 x = -deltaTime * invSmoothTime;
	F32 oneMinusExpX = -x - x*x * 0.5f + x*x*x * 0.1666666667f;
	return Maths::Select_GT(smoothTime, 0.f, oneMinusExpX, 1.f);
}

void C_WalkController::Update(
	F32 deltaTime,
	const Maths::Vector3& rootVelocity,
	const Maths::Mat43& modelWorldMatrix,
	const C_GaitInfo* gaitInfo,
	F32 speed,
	I_GroundCollision* groundCollision,
	bool onGround,
	F32 ikWeight
)
{
	if (!gDebugEnableFootstepPrediction)
	{
		gaitInfo = NULL;
	}

	F32 lastGaitPhase = m_gaitPhase;
	F32 gaitPeriod = 0.f;
	if (gaitInfo)
	{
		gaitPeriod = gaitInfo->GetPeriod();
		m_gaitPhase = fmodf(m_gaitPhase + (deltaTime * speed / gaitPeriod), 1.f);
	}
	
	m_groundHeightSmoothing = gDebugGroundHeightSmoothing;
	m_hipHeightSmoothing = gDebugHipHeightSmoothing;	
		
	F32 groundHeightSmoothing = GetTimeAdjustedSmoothing(m_groundHeightSmoothing, deltaTime);
	F32 hipHeightSmoothing = GetTimeAdjustedSmoothing(m_hipHeightSmoothing, deltaTime);
		
	F32 lowestHeightDelta = FLT_MAX;
	for (U32 i = 0; i < m_numLegs; ++i)
	{
		if (gaitInfo)
		{
			m_stepControllers[i].m_stepInfo = gaitInfo->GetStepInfo(i);
		}
		else
		{
			m_stepControllers[i].m_stepInfo = NULL;
		}
	
		// Update step controllers which will estimate the ground height and normal beneath each foot.
		m_stepControllers[i].Update(
			lastGaitPhase,
			m_gaitPhase,
			gaitPeriod,
			groundHeightSmoothing,
			rootVelocity,
			modelWorldMatrix,
			onGround,
			groundCollision);
	
		// Find lowest ground height adjustment.
		F32 localHeightDelta = m_stepControllers[i].GetHeightDelta();
		lowestHeightDelta = Maths::Min(lowestHeightDelta, localHeightDelta);
	}
	
	// Adjust hips down.
	m_allowHipMovement = gDebugEnableHipLowering;
	if (m_allowHipMovement && onGround)
	{
		m_hipHeightDelta = m_hipHeightDelta + (lowestHeightDelta - m_hipHeightDelta) * hipHeightSmoothing;
	}
	else
	{
		m_hipHeightDelta = 0.f;
	}
	
	// Restrict maximum hip delta.
	F32 maxHipHeightDelta = 0.25f * m_minLegLength;
	m_hipHeightDelta = Maths::Clamped(m_hipHeightDelta, -maxHipHeightDelta, maxHipHeightDelta);
		
	const Maths::Normal3 upDir = Maths::Const<Maths::Normal3>::AxisY();
	m_hipOffset = upDir * Maths::MakeVF32(m_hipHeightDelta);
	
	Maths::Mat43 worldToModelMatrix = Maths::Inverse(modelWorldMatrix);

	// Set IK targets for legs.
	for (U32 i = 0; i < m_numLegs; ++i)
	{
		F32 localHeightDelta = m_stepControllers[i].GetHeightDelta() - m_hipHeightDelta;
		
		// Convert ground normal into model space.
		Maths::Normal3 modelGroundNormal = Maths::MakeNormal3(m_stepControllers[i].GetGroundNormal() * worldToModelMatrix);
				
		// Set up IK target to raise up foot in proportion to ground height.
		Maths::Mat43 delta = Maths::MakeMat43(Maths::Const<Maths::Mat33>::Identity(), Maths::MakeVector3(0.f, localHeightDelta, 0.f));
		
		C_IkChain& ikChain = *m_legIkChains[i];
		U32 targetIndex = m_legIkTargetIndices[i];
		
		ikChain.SetTargetPositionOffset(targetIndex, Maths::MakeVector3(0.f, localHeightDelta, 0.f));
		ikChain.SetTargetRotationOffset(targetIndex, CalculateShortestRotationQuaternion(upDir, modelGroundNormal));

		ikChain.SetTargetWeight(targetIndex, ikWeight);
	}
}

Maths::Vector3	C_WalkController::GetHipOffset() const
{
	return m_hipOffset;
}

#ifndef CFG_GOLD
void C_WalkController::DebugRender(DbUtil::I_DebugRender* debugRender, const Maths::Mat43& modelWorldMatrix) const
{
	debugRender->SetDepthEnable(false);
	for (U32 i = 0; i < m_numLegs; ++i)
	{
		m_stepControllers[i].DebugRender(debugRender, modelWorldMatrix);
	}
	debugRender->SetDepthEnable(true);
}
#endif


} // end Animation
