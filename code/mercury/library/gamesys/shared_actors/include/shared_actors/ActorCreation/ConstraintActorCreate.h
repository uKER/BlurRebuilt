#ifndef _CONSTRAINT_ACTOR_CREATE_
#define _CONSTRAINT_ACTOR_CREATE_

#ifdef _MSC_VER
#pragma once
#endif

#include "actor_system/ActorDeclare.h"
#include "shared_actors/XtActorBase.h"
#include "shared_actors/XtSharedLevelActorParams.h"
#include <gravity/sharedphysics/GraphNode.h>
#include <shared_Actors/SharedActorTypes.h>
#include <shared_actors/XtActorMessages.h>


namespace Actors
{

struct C_ConstraintTypeBase;

// Constraint Creation Base Class
struct C_ConstraintActorCreate : public C_PosActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_Constraint;}
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_Constraint;}

	SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const;

	// virtual data accessors
	virtual const Actor::C_ActorHandle ActorHandleA() const = 0;
	virtual bool ActorBIsNull() const=0;
	virtual const Actor::C_ActorHandle ActorHandleB() const = 0;
	virtual const C_ConstraintTypeBase* ConstraintInfo() const = 0;

	ActorMsg::C_XtSendMessage m_OnBroken;
};



// Auxiliary constraint types & enums

enum TAxisType
{
	E_AxisX=0,
	E_AxisY,
	E_AxisZ,
};

struct TBoolExtents
{
	TBoolExtents() : m_Limit(false) {}
	void Set(F32 min, F32 max) {m_Limit=true; m_Min=min; m_Max=max;}

	bool m_Limit;
	F32 m_Min;
	F32 m_Max;
};


// Sorcery Constraint Creation Data
// --------------------------------

struct C_ConstraintTypeBase
{
	virtual SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const {return SharedPhysics::C_Constraint::E_Type_Invalid;}
};


struct C_FixedConstraint : public C_ConstraintTypeBase
{
	SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const {return SharedPhysics::C_Constraint::E_Type_Fixed;}

};


struct C_SphericalConstraint : public C_ConstraintTypeBase
{
	SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const {return SharedPhysics::C_Constraint::E_Type_Spherical;}
	bool m_Limit;
	XtVec3 m_AxisLowLimitDegs;
	XtVec3 m_AxisHighLimitDegs;
	bool m_CreateSpring;
	F32 m_SpringCoeff;
	F32 m_SpringDamping;
	F32 m_BodyAMassScale;
};


struct C_DistanceConstraint : public C_ConstraintTypeBase
{
	SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const {return SharedPhysics::C_Constraint::E_Type_Distance;}
	F32 m_Distance;
};


struct C_SliderConstraint : public C_ConstraintTypeBase
{
	SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const {return SharedPhysics::C_Constraint::E_Type_Prismatic;}
	TAxisType m_Axis;
	TBoolExtents m_Extents;
	bool m_CreateSpring;
	F32 m_SpringCoeff;
	F32 m_SpringDamping;
 	F32 m_RestLength;
};


struct C_HingeConstraint : public C_ConstraintTypeBase
{
	SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const {return SharedPhysics::C_Constraint::E_Type_Revolute;}
	TAxisType m_Axis;
	TBoolExtents m_Extents;
	bool m_CreateSpring;
	F32 m_SpringCoeff;
	F32 m_SpringDamping;
	F32 m_AngularLimit;
};


struct C_SorceryConstraintActorCreate : public C_ConstraintActorCreate
{
	const Actor::C_ActorHandle ActorHandleA() const {return m_ActorA ? m_ActorA->GetActorHandle() : Actor::C_ActorHandle();}
	bool ActorBIsNull() const {return m_ActorB==NULL;}
	const Actor::C_ActorHandle ActorHandleB() const {assert(m_ActorB); return m_ActorB->GetActorHandle();}
	SharedPhysics::C_Constraint::E_ConstraintType ConstraintType() const {return m_Constraint ? m_Constraint->ConstraintType() : SharedPhysics::C_Constraint::E_Type_SphericalLimit;}
	const C_ConstraintTypeBase* ConstraintInfo() const {assert(m_Constraint); return m_Constraint;}

	C_PosActorCreate	*	m_ActorA;
	C_PosActorCreate	*	m_ActorB;
	C_ConstraintTypeBase *	m_Constraint;
};


// Dynamic Constraint Creation Data
struct C_DynamicConstraintActorCreate : public C_ConstraintActorCreate
{
	C_DynamicConstraintActorCreate() {}

	virtual const Maths::HMat43 GetCreationMatrix() const {return Maths::Load<Maths::HMat43>(&m_DynWorldMatrix);}

	const Actor::C_ActorHandle ActorHandleA() const {return m_ActorHandleA;}
	bool ActorBIsNull() const {return m_ActorHandleB==Actor::C_ActorHandle();}
	const Actor::C_ActorHandle ActorHandleB() const {assert(m_ActorHandleB); return m_ActorHandleB;}

	// From C_ActorCreationData
	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

	MathsStorage::Float43 m_DynWorldMatrix;
	Actor::C_ActorHandle m_ActorHandleA;
	Actor::C_ActorHandle m_ActorHandleB;
};


struct C_DynamicFixedConstraintActorCreate : public C_DynamicConstraintActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_FixedConstraint;}
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_FixedConstraint;}
	const C_ConstraintTypeBase* ConstraintInfo() const {return &m_Constraint;}
	C_FixedConstraint m_Constraint;
};


struct C_DynamicSphericalConstraintActorCreate : public C_DynamicConstraintActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_SphericalConstraint;}
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_SphericalConstraint;}
	const C_ConstraintTypeBase* ConstraintInfo() const {return &m_Constraint;}
	C_SphericalConstraint m_Constraint;
};


struct C_DynamicDistanceConstraintActorCreate : public C_DynamicConstraintActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_DistanceConstraint;}
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_DistanceConstraint;}
	const C_ConstraintTypeBase* ConstraintInfo() const {return &m_Constraint;}
	C_DistanceConstraint m_Constraint;
};


struct C_DynamicSliderConstraintActorCreate : public C_DynamicConstraintActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_SliderConstraint;}
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_SliderConstraint;}
	const C_ConstraintTypeBase* ConstraintInfo() const {return &m_Constraint;}
	C_SliderConstraint m_Constraint;
};


struct C_DynamicHingeConstraintActorCreate : public C_DynamicConstraintActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_HingeConstraint;}
	static Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_HingeConstraint;}
	const C_ConstraintTypeBase* ConstraintInfo() const {return &m_Constraint;}
	C_HingeConstraint m_Constraint;
};


} //namespace Actors

#endif
