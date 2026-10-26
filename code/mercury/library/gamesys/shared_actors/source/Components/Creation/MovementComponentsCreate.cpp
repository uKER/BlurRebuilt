#include "SharedActorsPch.h"

#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

#include <shared_actors/XtActorBase.h>


// Create Info
XT_ABSTRACT_TYPE(Components::C_MovementCreateInfo, "MovementComponentBase")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_pParentActorCreate, "ParentActorCreate")
XT_END_TYPE

XT_TYPE(Components::C_MovementAttacherCreateInfo, "AttacherMovementComponent")
XT_FIELD(m_DopplerSpeed, "DopplerSpeed")
XT_BASE(Components::C_MovementCreateInfo)
XT_END_TYPE

XT_TYPE(Components::C_MovementTranslatorCreateInfo, "TranslatorMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
XT_END_TYPE

XT_TYPE(Components::C_MovementRotatorCreateInfo, "RotatorMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
XT_END_TYPE

XT_TYPE(Components::C_MovementRotTransCreateInfo, "RotTransMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
XT_END_TYPE

XT_TYPE(Components::C_MovementSpinnerCreateInfo, "SpinnerMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
XT_FIELD(m_InitialRPM, "InitialRpm")
XT_END_TYPE

XT_TYPE(Components::C_MovementWheelCreateInfo, "WheelMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
XT_FIELD(m_Radius, "Radius")
XT_END_TYPE

XT_TYPE(Components::C_MovementConstrainerCreateInfo, "ConstrainerMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
XT_END_TYPE

XT_TYPE(Components::C_MovementAnimationSlaveCreateInfo, "AnimationSlaveMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
XT_END_TYPE

XT_TYPE(Components::C_MovementSplineCreateInfo, "SplineMovementComponent")
	XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_pSplineActorCreate, "Spline")
	XT_FIELD(m_MoveSpeed, "MoveSpeed")
	XT_FIELD(m_Rotation, "RotationAngle")
	XT_FIELD(m_Loop, "Loop")
	XT_FIELD(m_StartImmediately, "StartImmediately")
	XT_FIELD(m_StartFromNearestPoint, "StartFromNearestPoint")
XT_END_TYPE

XT_TYPE(Components::C_MovementKinematicSplineCreateInfo, "KinematicSplineMovementComponent")
	XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_pSplineActorCreate, "Spline")
	XT_FIELD(m_StartSpeed, "StartSpeed")
	XT_FIELD(m_MinSpeed, "MinSpeed")
	XT_FIELD(m_MaxSpeed, "MaxSpeed")
	XT_FIELD(m_Acceleration, "Acceleration")
	XT_FIELD(m_BrakeAtEnd, "BrakeAtEnd")
	XT_FIELD(m_Loop, "Loop")
	XT_FIELD(m_StartImmediately, "StartImmediately")
	XT_FIELD(m_StartFromNearestPoint, "StartFromNearestPoint")
XT_END_TYPE

XT_TYPE(Components::C_MovementKinematicSplineTrackerCreateInfo, "KinematicSplineTrackerMovementComponent")
	XT_BASE(Components::C_MovementKinematicSplineCreateInfo)
XT_END_TYPE

XT_TYPE(Components::C_MovementKinematicSplineSlaveCreateInfo, "KinematicSplineSlaveMovementComponent")
	XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_pSplineActorCreate, "Spline")
	XT_FIELD(m_pSlaveMaster, "SlaveMaster")
	XT_FIELD(m_LastSlave, "LastSlave")
XT_END_TYPE

XT_TYPE(Components::C_MovementRelativeToBoneCreateInfo, "BoneRelativeMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_BoneName, "BoneName")
	XT_FIELD(m_TranslationOffset, "TranslationOffset")
	XT_FIELD(m_RotationOffset, "RotationOffset")
	XT_FIELD(m_IgnoreBoneRotation, "IgnoreBoneRotation")
XT_END_TYPE

XT_TYPE(Components::C_MovementAimSplineCreateInfo, "AimSplineMovementComponent")
XT_BASE(Components::C_MovementSplineCreateInfo)
	XT_FIELD(m_pTargetActor, "TargetActor")
	XT_FIELD(m_TrackingSpeed, "TrackingSpeed")
XT_END_TYPE

XT_TYPE(Components::C_MovementAimerCreateInfo, "AimerMovementComponent")
XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_pTargetActor, "TargetActor")
	XT_FIELD(m_TrackingSpeed, "TrackingSpeed")
	XT_FIELD(m_LimitAngle, "LimitAngle")
	XT_FIELD(m_LimitAngleAmount, "LimitAngleAmount")
XT_END_TYPE

XT_TYPE(Components::C_MovementBobbingBoatCreateInfo, "BobbingBoatMovementComponent")
	XT_BASE(Components::C_MovementCreateInfo)
	XT_FIELD(m_MaxPitchScale, "MaxPitchScale")
	XT_FIELD(m_MaxRollScale, "MaxRollScale")
	XT_FIELD(m_PitchTimerOffset, "PitchTimerOffset")
	XT_FIELD(m_PitchTimerScale, "PitchTimerScale")
	XT_FIELD(m_RollTimerOffset, "RollTimerOffset")
	XT_FIELD(m_RollTimerScale, "RollTimerScale")
XT_END_TYPE

XT_LIBRARY(SharedMovementComponentCreation)
XT_REGISTER_TYPE(Components::C_MovementCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementAttacherCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementTranslatorCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementRotatorCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementRotTransCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementSpinnerCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementWheelCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementConstrainerCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementAnimationSlaveCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementSplineCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementKinematicSplineCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementKinematicSplineTrackerCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementKinematicSplineSlaveCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementRelativeToBoneCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementAimerCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementAimSplineCreateInfo)
XT_REGISTER_TYPE(Components::C_MovementBobbingBoatCreateInfo)
XT_END_LIBRARY




namespace Components
{

C_MovementCreateInfo::C_MovementCreateInfo()
	: I_GameComponentCreator()
	, m_pParentActorCreate(NULL)
{
}

C_MovementCreateInfo::~C_MovementCreateInfo()
{
}

bool C_MovementCreateInfo::IsParented() const
{
	return m_pParentActorCreate != NULL;
}

Actor::C_ActorHandle C_MovementCreateInfo::ParentHandle() const
{
	return m_pParentActorCreate->GetActorHandle();
}

} // namespace Components
