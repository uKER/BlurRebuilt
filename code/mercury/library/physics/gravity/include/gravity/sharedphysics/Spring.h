#ifndef SPRING_H
#define SPRING_H

#ifdef _MSC_VER
#pragma once
#endif

#include "gravity/sharedphysics/AllocationManager.h"
#include "gravity/sharedphysics/GraphNode.h"

namespace SharedPhysics
{
class C_GraphNode;
class C_RigidBody;


class C_Spring : public C_BlockItem<C_Spring>
{
public:
	C_Spring(): m_RotationalOffset(Maths::UNDEFINED), m_LocalContactA(Maths::UNDEFINED),
		m_LocalContactB(Maths::UNDEFINED), m_LinearSpringCoefficient(Maths::UNDEFINED), 
		m_LinearDampingCoefficient(Maths::UNDEFINED),m_AngularSpringCoefficient(Maths::UNDEFINED), 
		m_AngularDampingCoefficient(Maths::UNDEFINED), m_RestLength(Maths::UNDEFINED), m_pRigidA(NULL),
		m_pRigidB(NULL), m_pNodeA(NULL), m_pNodeB(NULL)
	{
	}

	~C_Spring()
	{
	}


	NOALIAS inline bool operator < (const C_Spring& other)
	{
		return (m_pNodeA->m_Index < other.m_pNodeA->m_Index) || 
			(m_pNodeA->m_Index == other.m_pNodeA->m_Index && m_pNodeB->m_Index < other.m_pNodeB->m_Index);
	}

	NOALIAS inline bool operator <= (const C_Spring& other)
	{
		return (m_pNodeA->m_Index < other.m_pNodeA->m_Index) || 
			(m_pNodeA->m_Index == other.m_pNodeA->m_Index && m_pNodeB->m_Index <= other.m_pNodeB->m_Index);
	}

	NOALIAS inline bool operator > (const C_Spring& other)
	{
		return (m_pNodeA->m_Index > other.m_pNodeA->m_Index) || 
			(m_pNodeA->m_Index == other.m_pNodeA->m_Index && m_pNodeB->m_Index > other.m_pNodeB->m_Index);
	}

	NOALIAS inline bool operator >= (const C_Spring& other)
	{
		return (m_pNodeA->m_Index > other.m_pNodeA->m_Index) || 
			(m_pNodeA->m_Index == other.m_pNodeA->m_Index && m_pNodeB->m_Index < other.m_pNodeB->m_Index);
	}

	NOALIAS inline bool operator == (const C_Spring& other)
	{
		return (m_pNodeA->m_Index == other.m_pNodeA->m_Index && m_pNodeB->m_Index == other.m_pNodeB->m_Index);
	}

	NOALIAS inline bool operator != (const C_Spring& other)
	{
		return (m_pNodeA->m_Index != other.m_pNodeA->m_Index || m_pNodeB->m_Index != other.m_pNodeB->m_Index);
	}

	void SetLinearCoefficient(const Maths::HF32 PCREF springCoefficient, const Maths::HF32 PCREF dampingCoefficient)
	{
		m_LinearSpringCoefficient = springCoefficient;
		m_LinearDampingCoefficient = dampingCoefficient;
	}

	void SetAngularCoefficient(const Maths::HF32 PCREF springCoefficient, const Maths::HF32 PCREF dampingCoefficient)
	{
		m_AngularSpringCoefficient = springCoefficient;
		m_AngularDampingCoefficient = dampingCoefficient;
	}
	
	/*
		normal should be bodyA point to bodyB, the spring force get out is for bodyB, bodyA's spring force is on the opposite direction with the bodyB's sprint force
	*/
	Maths::HVec3 GetSpringForce(Maths::HVec3& bodyBTorque);
	
	Maths::C_Quaternion m_RotationalOffset;
	Maths::HVec3 m_LocalContactA; //the point, in local space of bodyB, spring attached to bodyA
	Maths::HVec3 m_LocalContactB; //the point, in local space of bodyB, spring attached to bodyB
	Maths::HF32 m_LinearSpringCoefficient;	//spring coefficient
	Maths::HF32 m_LinearDampingCoefficient; //damping coefficient
	Maths::HF32 m_AngularSpringCoefficient;	//spring coefficient
	Maths::HF32 m_AngularDampingCoefficient; //damping coefficient
	Maths::HF32 m_RestLength;
	SharedPhysics::C_RigidBody* m_pRigidA;
	SharedPhysics::C_RigidBody* m_pRigidB;
	SharedPhysics::C_GraphNode* m_pNodeA;
	SharedPhysics::C_GraphNode* m_pNodeB;
};


}

#endif
