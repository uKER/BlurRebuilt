#ifndef CONSTRAINT_DESC_H
#define CONSTRAINT_DESC_H

#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/sdk/ShareDeserialiser.h"

namespace Solver2
{
	class C_ConstraintGraphManager;
}


namespace Gravity
{
class C_Body;
//template <class Deserialiser>
class C_ConstraintDescriptors
{
public:
	C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_ConstraintType type): m_Type(type), m_Scale(1.f), m_Baumgarte(10.f), 
		m_MaskCollisions(false), m_MaskProxy(false)
	{
	}

	virtual ~C_ConstraintDescriptors()
	{
	}
#ifndef GRAVITY_EXPORTER
	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const = 0;
#endif

	void ImportBasic(Deserialiser& deserial)
	{
		deserial >> m_Id >> m_PhysicsBodyAID >>  m_PhysicsBodyBID >> m_PhysicsBodyAState >> m_PhysicsBodyBState >> m_Baumgarte >> m_MaskCollisions >> m_MaskProxy;
	}

	virtual void Import(Deserialiser& deserial)
	{
		//deserial >> m_Type;
		//deserial >> m_physicsBodyAID >> m_PhysicsBodyBID >> m_PhysicsBodyAState >> m_PhysicsBodyBState >> m_Baumgarte;
	}

	void SetScale(F32 scale)
	{
		m_Scale = scale;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << (U32)m_Type << m_Id << m_PhysicsBodyAID <<  m_PhysicsBodyBID << m_PhysicsBodyAState << m_PhysicsBodyBState << m_Baumgarte << m_MaskCollisions << m_MaskProxy;
	}

	void SetBaumgarte(const F32 baumgarte)
	{
		m_Baumgarte = baumgarte;
	}

	SharedPhysics::C_Constraint::E_ConstraintType m_Type; //what type of joints
	S32 m_Id;
	S32 m_PhysicsBodyAID;
	S32 m_PhysicsBodyBID;
	/*
		Active
		Kinematic
		Sleep
		Passive/Static
	*/
	U32 m_PhysicsBodyAState;
	U32 m_PhysicsBodyBState;
	F32 m_Scale;
	F32 m_Baumgarte;
	U32 m_MaskCollisions; //masking the collision between two neighbour bodies
	U32 m_MaskProxy; //masking the collision between two bodies
};

}

#endif
