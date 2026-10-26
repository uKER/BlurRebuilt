#ifndef REFERENCEFRAME_H
#define REFERENCEFRAME_H

#include "gravity/sdk/Body.h"

namespace Gravity
{
	class C_ReferenceFrame
	{
	public:
		C_ReferenceFrame(C_Body* pBody = NULL): m_pBodyA(pBody), m_pBodyB(NULL)
		{
		}

		~C_ReferenceFrame()
		{
		}

		NOALIAS inline bool operator < (const C_ReferenceFrame& other)
		{
			return m_Index < other.m_Index;
		}

		NOALIAS inline bool operator <= (const C_ReferenceFrame& other)
		{
			return m_Index <= other.m_Index;
		}

		NOALIAS inline bool operator > (const C_ReferenceFrame& other)
		{
			return m_Index > other.m_Index;
		}

		NOALIAS inline bool operator >= (const C_ReferenceFrame& other)
		{
			return m_Index >= other.m_Index;
		}

		NOALIAS inline bool operator == (const C_ReferenceFrame& other)
		{
			return m_Index == other.m_Index;
		}

		NOALIAS inline bool operator != (const C_ReferenceFrame& other)
		{
			return m_Index != other.m_Index;
		}

		void AddReferenceFrame(C_Body* pBody)
		{
			m_pBodyA = pBody;
		}

		//Attach the body to the reference frame, then change the status of the body
		void AddBody(C_Body* otherBody)
		{
			m_pBodyB = otherBody;
			SharedPhysics::C_RigidBody& pBodyA = m_pBodyA->GetBody();
			SharedPhysics::C_RigidBody& pBodyB = m_pBodyB->GetBody();
	
			pBodyB.StoreVelocitiesAndPosition(pBodyA.m_LinearVelocity, pBodyA.m_AngularVelocity, pBodyB.m_Orientation, pBodyB.m_Position);
		}

		void UpdateAttachedBodyState()
		{
		
			SharedPhysics::C_RigidBody& pBodyA = m_pBodyA->GetBody();
			SharedPhysics::C_RigidBody& pBodyB = m_pBodyB->GetBody();

			pBodyB.StoreVelocitiesAndPosition(pBodyA.m_LinearVelocity, pBodyA.m_AngularVelocity, pBodyB.m_Orientation, pBodyB.m_Position);

			
		}
		void ChangeReferenceVelocity(const Maths::HVec3 PCREF velocity)
		{
			using namespace Maths;
			SharedPhysics::C_RigidBody& pBodyA = m_pBodyA->GetBody();
			SharedPhysics::C_RigidBody& pBodyB = m_pBodyB->GetBody();
			pBodyA.m_LinearVelocity = pBodyA.m_LinearVelocity + velocity;
			pBodyB.m_LinearVelocity = pBodyB.m_LinearVelocity;
		}

		C_Body* m_pBodyA;
		C_Body* m_pBodyB;
		U32 m_Index;
	};
};
#endif
