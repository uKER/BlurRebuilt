#include <stdafx.h>
#include "gravity/solver2/Featherstone.h"

//using namespace Maths;

namespace Solver2
{

	void C_MultiBody::Connect(C_Link* in, C_Link* out, C_Joint& joint)
	{
		joint.m_pInBoundLink=in;
		joint.m_pOutBoundLink=out;
		joint.m_pNextJoint = in->m_pFirstJoint;
		in->m_pFirstJoint = &joint;
	}

	void C_MultiBody::EnumerateJointIndices(C_Link** pStack)
	{
		S32 size = 1;
		pStack[0] = &m_Links[0];
		S32 index = 0;

		while(size--)
		{
			C_Link* pLink = pStack[size];
			pLink->m_index = index++;

			C_Joint* pJoint = pLink->m_pFirstJoint;
			while(pJoint)
			{
				pStack[size++] = pJoint->m_pOutBoundLink;
				pJoint = pJoint->m_pNextJoint;
			}
		}

		for(S32 a = 0; a < m_Joints.m_numElements; ++a)
		{
			m_Joints[a].m_index = m_Joints[a].m_pOutBoundLink->m_index;
		}
	}

	void C_MultiBody::SetJointVelocity(const U32 index, Maths::VF32 PCREF velocity)
	{
		m_pSortedJoints[index]->m_scalarVelocity = velocity;
	}


	void C_MultiBody::RearrangeJoint()
	{
		m_pSortedJoints = new C_Joint*[m_Joints.m_numElements];
		for(S32 i=0; i<m_Joints.m_numElements; ++i)
		{
			m_pSortedJoints[m_Joints[i].m_index - 1]=&m_Joints[i];
		}
	}

	void C_MultiBody::CalcualteSpatialRelativeTransformation()
	{
		for(S32 i=0; i< m_Joints.m_numElements; ++i)
		{
			m_Joints[i].IniRelativeMatrix();			
		}
	}

	void C_MultiBody::CompLinkVelocities()
	{
		using namespace Maths;

		C_Joint* joint;
		C_Link* outBoundLink;
		C_Link* inBoundLink = m_pSortedJoints[0]->m_pInBoundLink;
		const SharedPhysics::C_RigidBody& rigid = inBoundLink->m_pBody->GetBody();
		const HMat33 inverse = rigid.m_OrientationMatrix.Inverse();
		//Transform the velocity into local space
		const Vector3 angVelLocal = (Vector3)(rigid.m_AngularVelocity * inverse);
		const Vector3 linVelLocal = (Vector3)(rigid.m_LinearVelocity * inverse);

		inBoundLink->m_spatialVelocity = MakeVector6(angVelLocal, linVelLocal);

		for(S32 i=0; i<m_Joints.m_numElements; ++i)
		{
			joint=m_pSortedJoints[i];
			outBoundLink=joint->m_pOutBoundLink;
			outBoundLink->CompVelocity(*joint->m_pInBoundLink, *joint); 
		}
	}

	//we just need to call this method one time in the whole simulation
	void C_MultiBody::InitiLInksSpatialIsolatedInertia()
	{
		C_Link* link;

		for(S32 i=0; i<m_Links.m_numElements; ++i)
		{
			link = &m_Links[i];
			link->IniSpatialIsolatedInertia();
		}
	}

	void C_MultiBody::InitLinks()
	{
		C_Joint* joint;
		C_Link* outBoundLink;
		C_Link* link;
		//Initialize each link's articulated inertia and articulated zero acceleration force to their isolated counterparts
		for(S32 i=0; i<m_Links.m_numElements; ++i)
		{
			link = &m_Links[i];
			//link->IniSpatialIsolatedInertia();
			link->IniSpatialIsolatedZeroAcceleration();
		}

		link = m_pSortedJoints[0]->m_pInBoundLink;
		link->m_spatialCoriolis = Maths::Const<Maths::Vector6>::Zero();

		//This method didn't calculate the coriolis force of link 0
		for(S32 i=0; i<m_Joints.m_numElements; ++i)
		{
			joint=m_pSortedJoints[i];
			outBoundLink=joint->m_pOutBoundLink;
			outBoundLink->CompCoriolis(*joint->m_pInBoundLink, *joint);
		}
	}

	void C_MultiBody::CompSpatialInertiaAndZeroAcceleration()
	{
		C_Joint* joint;
		C_Link* inBoundLink;
		
		S32 size = m_Joints.m_numElements - 1;

		for(S32 i=size; i>=0; --i)
		{
			joint=m_pSortedJoints[i];
			inBoundLink=joint->m_pInBoundLink;
			inBoundLink->CompArticulatedSpatialInertiaAndZeroAcceleration(*joint->m_pOutBoundLink, *joint);
		}
	}

	void C_MultiBody::CompJointAndSpatialAcceleration( const Maths::VF32 PCREF time)
	{
		using namespace Maths;

		C_Joint* joint;
		C_Link* outBoundLink;

		//Calculate the base link's spatial acceleration -Inverse(I)*Z
		joint = m_pSortedJoints[0];
		C_Link* baseLink = joint->m_pInBoundLink;
		Mat66 spatialAIInverse=Inverse(baseLink->m_spatialAI);//waiting for the inverse 66 matrix finish!!!!
		Mat66 identity = spatialAIInverse * baseLink->m_spatialAI;
		baseLink->m_spatialAcceleration = -(spatialAIInverse * baseLink->m_spatialAZA);


		SharedPhysics::C_RigidBody& rigid = baseLink->m_pBody->GetBody();
		rigid.m_LinearVelocity += HVec3(ExtractHiVector3(baseLink->m_spatialAcceleration) * time);
		rigid.m_AngularVelocity += HVec3(ExtractLoVector3(baseLink->m_spatialAcceleration) * time);
		rigid.m_LinearMomentum = rigid.m_LinearVelocity * rigid.m_Mass;
		rigid.m_AngularMomentum = rigid.m_AngularVelocity * rigid.m_InertiaTensor;
		



		for(S32 i=0; i<m_Joints.m_numElements; ++i)
		{
			joint=m_pSortedJoints[i];
			outBoundLink=joint->m_pOutBoundLink;
			outBoundLink->CompJointAndLinkAcceleration(*joint->m_pInBoundLink, *joint, time);
		}
	}

	void C_MultiBody::FwdDynamic( const Maths::VF32 PCREF time)
	{
		using namespace Maths;
		
		CalcualteSpatialRelativeTransformation();

		CompLinkVelocities();
		
		InitLinks();
		
		CompSpatialInertiaAndZeroAcceleration();
		
		CompJointAndSpatialAcceleration(time);
	}

};
