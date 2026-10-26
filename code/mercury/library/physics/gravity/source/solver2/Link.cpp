#include <stdafx.h>
#include "gravity/solver2/Featherstone.h"

//using namespace Maths;

namespace Solver2
{

	void C_Joint::IniSpatialRelativeMatrix()
	{
		using namespace Maths;

		SharedPhysics::C_RigidBody& inBody = m_pInBoundLink->m_pBody->GetBody();
		SharedPhysics::C_RigidBody& outBody = m_pOutBoundLink->m_pBody->GetBody();


		const Mat33 outInverse = outBody.m_OrientationMatrix.InverseRotation();
		const Mat33 inInverse = inBody.m_OrientationMatrix.InverseRotation();

		const Mat33 zeroMat = Const<Mat33>::Zero();

		
		const Vector3 offsetInToOut = (outBody.m_Position - inBody.m_Position) * outInverse; // in outBound's local space
		//Construct the skew matrix of offsetInToOut
		const Mat33 skewMatInToOut = MakeMat33_SkewSymmetric(offsetInToOut);
		const Mat33 skewRotationInToOut = - skewMatInToOut* m_InToOut;
	

		m_spatialMatInToOut=MakeMat66(m_InToOut, zeroMat, skewRotationInToOut, m_InToOut);


		const Vector3 offsetOutToIn =  (inBody.m_Position - outBody.m_Position) * inInverse; // in inBound's local space
		const Mat33 skewMatOutToIn =MakeMat33_SkewSymmetric(offsetOutToIn);
		const Mat33 skewRotationOutToIn = - skewMatOutToIn * m_OutToIn ;

		m_spatialMatOutToIn = MakeMat66(m_OutToIn, zeroMat, skewRotationOutToIn, m_OutToIn);
		 
	}

	void C_Joint::IniRelativeMatrix()
	{
		using namespace Maths;

		SharedPhysics::C_RigidBody& inBody = m_pInBoundLink->m_pBody->GetBody();
		SharedPhysics::C_RigidBody& outBody = m_pOutBoundLink->m_pBody->GetBody();

		//matrix from in to out
		const Mat33 relMatInToOut =inBody.m_OrientationMatrix * outBody.m_OrientationMatrix.InverseRotation();
		m_InToOut=Transpose(relMatInToOut);


		//matrix from out to in
		const Mat33 relMatOutToIn =  outBody.m_OrientationMatrix * inBody.m_OrientationMatrix.InverseRotation();
		m_OutToIn = Transpose(relMatOutToIn);
		 
	}

	void C_Link::CompVelocity(const C_Link& inBound, const C_Joint& joint)
	{
		using namespace Maths;

		SharedPhysics::C_RigidBody& inBody= inBound.m_pBody->GetBody();
		SharedPhysics::C_RigidBody& outBody = m_pBody->GetBody();
		 
		const Mat33 inverse(outBody.m_OrientationMatrix.InverseRotation());

		const Vector3 inLinVel= ExtractHiVector3(inBound.m_spatialVelocity); // in inbound local space
		const Vector3 inAngVel=ExtractLoVector3(inBound.m_spatialVelocity);//in inbound local space

		
		const Vector3 r((outBody.m_Position - inBody.m_Position)*inverse); //transfore into the local space


		Vector3 angVel =joint.m_InToOut * inAngVel ;
		Vector3 linVel = (joint.m_InToOut * inLinVel ) + (r^angVel); // the inbody's linear velocity in outbody local frame
		

		if(joint.type == PRISMATICJOINT)
		{
			linVel = linVel + joint.m_scalarVelocity * joint.m_jointAxis;
		}
		if(joint.type == REVOLUTEJOINT)
		{
			angVel = angVel + joint.m_scalarVelocity*joint.m_jointAxis;
			linVel = linVel + joint.m_scalarVelocity*ExtractHiVector3(joint.m_spatialJointAxis);
		}

	
		m_spatialVelocity =MakeVector6(angVel, linVel); // in out body's local space

	}

	void C_Link::IniSpatialIsolatedInertia()
	{
		using namespace Maths;

		const SharedPhysics::C_RigidBody& rigid = m_pBody->GetBody();

		const Mat33 zero = Const<Mat33>::Zero();

		const Mat33 t1(Maths::Const<Mat33>::Zero());
		const Mat33 br(Maths::Const<Mat33>::Zero());

		const Vector3 rowX = MakeVector3(rigid.m_Mass, 0.f, 0.f);
		const Vector3 rowY = MakeVector3(0.f, rigid.m_Mass, 0.f);
		const Vector3 rowZ = MakeVector3(0.f, 0.f, rigid.m_Mass);
		const Mat33 M=MakeMat33(rowX, rowY, rowZ);
		const Mat33 I(rigid.m_IBody);
		//const Mat33 I(rigid.m_InertiaTensor * rigid.m_OrientationMatrix);

		m_spatialAI = MakeMat66(zero, M, I, zero);
		

	}

	void C_Link::IniSpatialIsolatedZeroAcceleration()
	{
		using namespace Maths;

		const SharedPhysics::C_RigidBody& rigid = m_pBody->GetBody();
		const Vector3 angVel = ExtractLoVector3(m_spatialVelocity);
		
		const Vector3 temp((rigid.m_IBody * angVel) ^ angVel);//theory: w x Iw, implementation: Iw x w
		const Vector3 force=MakeVector3(0.f, rigid.m_Mass * (-9.8f), 0.f); 
		m_spatialAZA = MakeVector6(-force, temp);
		//m_spatialAZA = MakeVector6(Const<Vector3>::Zero(),  temp);


	}

	void C_Link::CompCoriolis(const C_Link& inBound, const C_Joint& joint)
	{
		using namespace Maths;
		const SharedPhysics::C_RigidBody& inBody = inBound.m_pBody->GetBody();
		const SharedPhysics::C_RigidBody& outBody = m_pBody->GetBody();

		
		const Mat33 inverse(outBody.m_OrientationMatrix.InverseRotation());
		
		const Vector3 r((outBody.m_Position - inBody.m_Position)*inverse);

		const Vector3 jointVelocity(joint.m_scalarVelocity*joint.m_jointAxis);//joint axis is already in the link local space

		const Vector3 inAngVel = ExtractLoVector3(inBound.m_spatialVelocity);
		const Vector3 angularVelocity(joint.m_InToOut * inAngVel ); //transfer the other's angularVelocity(world space) into the link's local space
		const Vector3 common1((r ^ angularVelocity) ^ angularVelocity);	//theory w x (w x r)
		const Vector3 common2(jointVelocity ^ angularVelocity);			// theory w ^ jointVecolity

		const VF32 two(Const<VF32>::Two());
		if(joint.type == PRISMATICJOINT)
		{
			const Vector3 zero(Const<Vector3>::Zero());
			const Vector3 temp(common1 + two*common2);
			m_spatialCoriolis = MakeVector6(zero, temp);
		}
		if(joint.type == REVOLUTEJOINT)
		{
			const Vector3 t(joint.m_scalarVelocity * ExtractHiVector3(joint.m_spatialJointAxis));		//jointVelocity x d
			const Vector3 part1(two* (t^angularVelocity));	//2*w x (jointVelocity ^ d)
			const Vector3 part2(t^jointVelocity);			//jointVelocity x (jointVelocity ^ d)
			const Vector3 temp(common1 + part1 + part2);
			m_spatialCoriolis = MakeVector6(common2, temp);
		}
	}

	void C_Link::CompArticulatedSpatialInertiaAndZeroAcceleration(C_Link& outBound, C_Joint& joint)
	{
		using namespace Maths;
		//SharedPhysics::C_RigidBody& outBody = outBound.m_pBody->GetBody();
		//SharedPhysics::C_RigidBody& inBody = m_pBody->GetBody();


		joint.IniSpatialRelativeMatrix();
		
		//Calculate out bound joint s'Is(1X6 6X6 6X1) = 1X1, which is a scalar

		
		joint.m_Is= outBound.m_spatialAI * joint.m_spatialJointAxis; //spatial articulate inertia * joint spatial axis
		joint.m_TransposeSI = joint.m_spatialJointAxisTranspose * outBound.m_spatialAI; //joint spatial axis's transpose * spatial articulate inertial

		joint.m_recip_sIs = Rcp(Dot(joint.m_spatialJointAxisTranspose,joint.m_Is));

		const Mat66 IsstI = TensorProduct(joint.m_Is,  joint.m_TransposeSI) * joint.m_recip_sIs;

		const Mat66 temp = outBound.m_spatialAI - IsstI;

		const Mat66 comp = joint.m_spatialMatOutToIn * temp * joint.m_spatialMatInToOut;

		//Calculate the inbound's spatial articulated inertia
		m_spatialAI += comp;

		/*
			Calculate the spatial zero acceleration
		*/
		const Vector6 zIc = outBound.m_spatialAZA + outBound.m_spatialAI * outBound.m_spatialCoriolis;
		VF32 t = Dot(joint.m_spatialJointAxisTranspose, zIc);
		joint.m_qsZIC = joint.m_spatialscalarForce - t;
		
		const VF32 cons = joint.m_qsZIC * joint.m_recip_sIs;

		const Vector6 tempZ = zIc + joint.m_Is*cons;

		m_spatialAZA += joint.m_spatialMatOutToIn * tempZ; 	
	}


	void C_Link::CompJointAndLinkAcceleration(C_Link& inBound, C_Joint& joint, const Maths::VF32 PCREF time)
	{
		using namespace Maths;

		const Vector6 inBoundRelativeAcceleration = joint.m_spatialMatInToOut * inBound.m_spatialAcceleration;

		const Vector6 temp1 = joint.m_TransposeSI * joint.m_spatialMatInToOut;
		const VF32 temp2 = Dot(temp1, inBound.m_spatialAcceleration);

		const VF32 comp = Dot(joint.m_TransposeSI, inBoundRelativeAcceleration);
		joint.m_scalarAcceleration = (joint.m_qsZIC - comp)*joint.m_recip_sIs;
		
		m_spatialAcceleration = inBoundRelativeAcceleration + m_spatialCoriolis + joint.m_scalarAcceleration * joint.m_spatialJointAxis;
		
		SharedPhysics::C_RigidBody& inBody = inBound.m_pBody->GetBody();
		SharedPhysics::C_RigidBody& outBody = m_pBody->GetBody();
		outBody.m_LinearVelocity += HVec3(ExtractHiVector3(m_spatialAcceleration)*outBody.m_OrientationMatrix * time);
		//outBody.m_AngularVelocity += HVec3(ExtractLoVector3(m_spatialAcceleration)*outBody.m_OrientationMatrix * time);
		
		const Vector3 jointPosition = (Vector3)inBody.m_Position + joint.m_InBoundD * inBody.m_OrientationMatrix;

		const Vector3 position = outBody.m_Position + outBody.m_LinearVelocity * HF32(time); //the link's position if don't have constraint

		const Vector3 dir=MakeVector3(Normalise(position - jointPosition));

		const VF32 dist = Length(joint.m_OutBoundD);

		outBody.m_Position = HVec3(jointPosition + dir*dist);

		//outBody.m_Position += outBody.m_LinearVelocity * HF32(time); //the link's position if don't have constraint

		//const Maths::C_Quaternion dif(Maths::C_Quaternion((outBody.m_AngularVelocity) * 0.5f) * outBody.m_Orientation * HF32(time));
		//outBody.m_Orientation = outBody.m_Orientation + dif;
		//outBody.m_Orientation.Normalise();
	
		outBody.StoreVelocitiesAndPosition(outBody.m_LinearVelocity, outBody.m_AngularVelocity, outBody.m_Orientation, outBody.m_Position);

	}
};
