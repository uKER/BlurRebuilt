#include "SharedActorsPch.h"
#include "shared_actors/ActorCreation/ConstraintActorCreate.h"

#include <actor_system/ActorSerialisation.h>
#include <misc_shared/context/ContextualStream.h>


// XT declarations
XT_TYPE(Actors::TAxisType, "TAxisName")
	XT_OPTION(Actors::E_AxisX, "AxisX")
	XT_OPTION(Actors::E_AxisY, "AxisY")
	XT_OPTION(Actors::E_AxisZ, "AxisZ")
XT_END_TYPE

XT_TYPE(Actors::TBoolExtents, "TBoolExtents")
	XT_FIELD(m_Limit,	"Limit")
	XT_FIELD(m_Min,		"Min")
	XT_FIELD(m_Max,		"Max")
XT_END_TYPE


XT_TYPE(Actors::C_ConstraintTypeBase, "ConstraintTypeBase")
XT_END_TYPE

XT_TYPE(Actors::C_FixedConstraint, "FixedConstraint")
	XT_BASE(Actors::C_ConstraintTypeBase)
XT_END_TYPE

XT_TYPE(Actors::C_SphericalConstraint, "SphericalConstraint")
	XT_BASE(Actors::C_ConstraintTypeBase)
	XT_FIELD(m_Limit,				"Limit")
	XT_FIELD(m_AxisLowLimitDegs,	"AxisLowLimitDegs")
	XT_FIELD(m_AxisHighLimitDegs,	"AxisHighLimitDegs")
	XT_FIELD(m_CreateSpring,		"CreateSpring")
	XT_FIELD(m_SpringCoeff,			"SpringCoeff")
	XT_FIELD(m_SpringDamping,		"SpringDamping")
	XT_FIELD(m_BodyAMassScale,		"BodyAMassScale")
XT_END_TYPE

XT_TYPE(Actors::C_DistanceConstraint, "DistanceConstraint")
	XT_BASE(Actors::C_ConstraintTypeBase)
	XT_FIELD(m_Distance, "Distance")
XT_END_TYPE

XT_TYPE(Actors::C_SliderConstraint, "SliderConstraint")
	XT_BASE(Actors::C_ConstraintTypeBase)
	XT_FIELD(m_Axis,			"Axis")
	XT_FIELD(m_Extents,			"Extents")
	XT_FIELD(m_CreateSpring,	"CreateSpring")
	XT_FIELD(m_SpringCoeff,		"SpringCoeff")
	XT_FIELD(m_SpringDamping,	"SpringDamping")
	XT_FIELD(m_RestLength,		"RestLength")
XT_END_TYPE

XT_TYPE(Actors::C_HingeConstraint, "HingeConstraint")
	XT_BASE(Actors::C_ConstraintTypeBase)
	XT_FIELD(m_Axis,			"Axis")
	XT_FIELD(m_Extents,			"Extents")
	XT_FIELD(m_CreateSpring,	"CreateSpring")
	XT_FIELD(m_SpringCoeff,		"SpringCoeff")
	XT_FIELD(m_SpringDamping,	"SpringDamping")
//?	XT_FIELD(m_AngularLimit,	"AngularLimit")
XT_END_TYPE

XT_TYPE(Actors::C_SorceryConstraintActorCreate, "ConstraintActor")
	XT_BASE(Actors::C_PosActorCreate)
	XT_FIELD(m_ActorA,		"ActorA")
	XT_FIELD(m_ActorB,		"ActorB")
	XT_FIELD(m_Constraint,	"Constraint")
	XT_FIELD(m_OnBroken,	"OnBroken")
XT_END_TYPE

XT_LIBRARY(ConstraintActorCreate)
	XT_REGISTER_TYPE(Actors::C_ConstraintTypeBase)
	XT_REGISTER_TYPE(Actors::C_FixedConstraint)
	XT_REGISTER_TYPE(Actors::C_SphericalConstraint)
	XT_REGISTER_TYPE(Actors::C_DistanceConstraint)
	XT_REGISTER_TYPE(Actors::C_SliderConstraint)
	XT_REGISTER_TYPE(Actors::C_HingeConstraint)
	XT_REGISTER_TYPE(Actors::C_SorceryConstraintActorCreate)
	XT_REGISTER_TYPE(Actors::TAxisType)
	XT_REGISTER_TYPE(Actors::TBoolExtents)
XT_END_LIBRARY


namespace Actors
{

SharedPhysics::C_Constraint::E_ConstraintType C_ConstraintActorCreate::ConstraintType() const
{
	return ConstraintInfo()->ConstraintType();
}


void C_DynamicConstraintActorCreate::Serialise(Sys::C_ContextualStream& stream) const
{
	C_ConstraintActorCreate::Serialise(stream);

	stream << m_DynWorldMatrix;
	stream << m_ActorHandleA;
	stream << m_ActorHandleB;
}


void C_DynamicConstraintActorCreate::Deserialise(Sys::C_ContextualStream& stream)
{
	C_ConstraintActorCreate::Deserialise(stream);

	stream >> m_DynWorldMatrix;
	stream >> m_ActorHandleA;
	stream >> m_ActorHandleB;
}


} //namespace Actors
