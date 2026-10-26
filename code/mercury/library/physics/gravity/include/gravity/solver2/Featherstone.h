#ifndef SOLVER2_FEATHERSTONE_H
#define SOLVER2_FEATHERSTONE_H

#include "gravity/sdk/Body.h"
#include <Core/Maths/HVec/New/IncludePhysics.h>
#include <Core/Maths/HVec/New/IncludePhysics.inl>
/*
	Featherstone's algorithm:
	Given the position and velocity of the n joints, the external forces acting on the linkage, and the forces and torques
	being applied by the joint actuators

	Find:
	1.Accoring to the joint position and velocity, calculate each link's velocity and the joint acceleration
	3.According to the joint acceleration to calculate each link's acceleration
*/
namespace Solver2
{
enum JointTyne
{
	PRISMATICJOINT	=	0,
	REVOLUTEJOINT
};

class C_Link;

/*
	Each joint is corresponding to a link( joint i - link i). Therefore, joint's position and velocity should be set in
	the corresponding link's local space
*/
class C_Joint
{
public:
	C_Joint() : m_pNextJoint(NULL), m_pInBoundLink(NULL), m_pOutBoundLink(NULL)
	{
		Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
		m_scalarVelocity = zero;
		m_scalarAcceleration = zero;
		m_spatialscalarForce= zero;

	}


	~C_Joint(){}

	void SetJointAxis(Maths::Vector3 PCREF jointAxis)
	{
		m_jointAxis=jointAxis;
	}

	void SetScalarVelocity(const Maths::VF32 PCREF scalarVelocity)
	{
		m_scalarVelocity=scalarVelocity;
	}

	void IniSpatialJointAxis(const Maths::Vector3 PCREF d)
	{
		using namespace Maths;

		if(type == PRISMATICJOINT)
		{
			const Vector3 zero = Const<Vector3>::Zero(); 
			m_spatialJointAxis= MakeVector6(zero, m_jointAxis);
			m_spatialJointAxisTranspose = MakeVector6(m_jointAxis, zero);
		}
		if(type == REVOLUTEJOINT)
		{
			Vector3 temp = d^m_jointAxis;
			m_spatialJointAxis = MakeVector6(m_jointAxis, temp);
			m_spatialJointAxisTranspose = MakeVector6(temp, m_jointAxis);
		}
	}

	void SetLinks(C_Link* inBoundLink, C_Link* outBoundLink)
	{
		m_pInBoundLink = inBoundLink;
		m_pOutBoundLink = outBoundLink;
	}

	void IniSpatialRelativeMatrix();

	void IniRelativeMatrix();


	Maths::Mat66 m_spatialMatInToOut;
	Maths::Mat66 m_spatialMatOutToIn;
	Maths::Mat33 m_InToOut;
	Maths::Mat33 m_OutToIn;

	//Maths::HVec3 m_spatialJointAxis1;	//if prismatic joint, 0. if revolute joint, m_jointAxis;
	//Maths::HVec3 m_spatialJointAxis2;   //if prismatic joint, it is m_jointAxis. If revolute joint, it is m_jointAixs X m_d
	Maths::Vector6 m_spatialJointAxis;
	Maths::Vector6 m_spatialJointAxisTranspose;
	Maths::Vector6 m_Is;	//outboundJoint's articulated inertia * spatial joint axis
	Maths::Vector6 m_TransposeSI; //spatial joint axis's transpose * outboundJoint's articulate inertia
	Maths::Vector3 m_jointAxis;			//joint's axis in the correspoinding outbound link's local space

	Maths::VF32 m_spatialscalarForce;		//joint actuator force, input data
	Maths::VF32 m_scalarVelocity;			//input data
	Maths::VF32 m_scalarAcceleration;
	Maths::VF32 m_recip_sIs;		//1/T(jointSpatialAxis)*LinkSpatialInertia*jointSpatialAxis
	Maths::VF32 m_qsZIC;	//q - T(jointSpatialAxis)*(Z + LinkSpatailInertia*c);

	Maths::Vector3 m_position;	//joint's world position
	Maths::Vector3 m_InBoundD;  // (inbound.position - m_position)*inbound.inverseMatrix, in inbound's local space
	Maths::Vector3 m_OutBoundD; // (outBound.position - m_position)*outbound.inverseMatrix, in outbound's local space

	U32 type;
	U32 m_index;
	C_Link* m_pInBoundLink;
	C_Link* m_pOutBoundLink;
	C_Joint* m_pNextJoint;
};

enum BodyPart
{
	HEAD,
	TORSO,
	HIP,
	LEFTARMTOP,
	LEFTARMBOTTOM,
	RIGHTARMTOP,
	RIGHTARMBOTTOM,
	LEFTLEGTOP,
	LEFTLEGBOTTOM,
	RIGHTLEGTOP,
	RIGHTLEGBOTTOM
};
/*
	Each link is corresponded to a body
*/
class C_Link
{
public:
	C_Link() : 
			m_spatialAI(Maths::Const<Maths::Mat66>::Zero()),
			m_spatialAZA(Maths::Const<Maths::Vector6>::Zero()),
			m_spatialVelocity(Maths::Const<Maths::Vector6>::Zero()),
			m_spatialAcceleration(Maths::Const<Maths::Vector6>::Zero()),
			m_spatialCoriolis(Maths::Const<Maths::Vector6>::Zero()),
			m_pFirstJoint(NULL),
			m_pBody(NULL)
			{
			}
	~C_Link()
	{
	}



	void CompVelocity(const C_Link& other, const C_Joint& joint);
	void IniSpatialIsolatedInertia();
	void IniSpatialIsolatedZeroAcceleration();
	void CompCoriolis(const C_Link& other, const C_Joint& joint);
	void CompArticulatedSpatialInertiaAndZeroAcceleration(C_Link& other, C_Joint& joint);
	void CompJointAndLinkAcceleration(C_Link& other, C_Joint& joint, const Maths::VF32 PCREF time);
	
	U32 m_index;

	Maths::Mat66 m_spatialAI; //spacial articulated inertia
	Maths::Vector6 m_spatialAZA;//spatial arciculated zero inertia
	Maths::Vector6 m_spatialVelocity;//first 3 element angular velocity, last 3 element linear velocity
	Maths::Vector6 m_spatialAcceleration;//first 3 element angular acceleration, lat 3 element linear velocity
	Maths::Vector6 m_spatialCoriolis;

	C_Joint* m_pFirstJoint;
	Gravity::C_Body* m_pBody;
	BodyPart m_Type;
};

class C_MultiBody
{
public:


	C_MultiBody()
	{
	}


	~C_MultiBody()
	{
		/*if(m_pSortedJoints)
			delete [] m_pSortedJoints;*/
		//m_pSortedJoints = NULL;
	}

	S32 GetNumJoints()
	{
		return m_Joints.m_numElements;
	}

	S32 GetNumLinks()
	{
		return m_Links.m_numElements;
	}


	C_Link* AddLink(C_Link* link)
	{
		return m_Links.AddToEndElement(link);
	}

	C_Joint* AddJoint(C_Joint* joint)
	{
		return m_Joints.AddToEndElement(joint);
	}

	void SetJointVelocity(const U32 index, Maths::VF32 PCREF velocity);

	//This method is called in each time step
	void FwdDynamic(const Maths::VF32 PCREF time);

	//Call these method when we set up the links
	void Connect(C_Link* in, C_Link* out, C_Joint& joint);
	void EnumerateJointIndices(C_Link** pStack);
	void RearrangeJoint();
	void InitiLInksSpatialIsolatedInertia();



	SharedPhysics::SortedArray<C_Joint > m_Joints;
	//SharedPhysics::SortedArray<C_Joint* > m_pSortedJoints;
	SharedPhysics::SortedArray<C_Link > m_Links;
	C_Joint** m_pSortedJoints;

private:

	void CalcualteSpatialRelativeTransformation();
	void CompLinkVelocities();
	void InitLinks();
	void CompSpatialInertiaAndZeroAcceleration();
	void CompJointAndSpatialAcceleration( const Maths::VF32 PCREF time);

};


};

#endif

