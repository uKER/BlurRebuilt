#ifndef SOLVER2_CONSTRAINT_H
#define SOLVER2_CONSTRAINT_H

#include "gravity/sharedphysics/GraphNode.h"
#include "gravity/sharedphysics/SortedArray.h"


namespace SharedPhysics
{
	struct C_ContactStreamPoint;
	class C_Spring;
}

namespace Solver2
{
class C_Jacobian;
class C_InvertMJT;
class C_ContactConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_ContactConstraint>
{
public:

	C_ContactConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_Contact),
		m_ContactPoint(Maths::UNDEFINED), m_Normal(Maths::UNDEFINED), m_Friction(Maths::UNDEFINED), m_Coefficients(Maths::UNDEFINED)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);

	void SetCoefficients(const Maths::HF32 PCREF penDepth, const Maths::HVec3 PCREF friction, 
		const Maths::HF32 PCREF restitution)
	{
		m_Coefficients = Maths::HVec2(penDepth, restitution);
		m_Friction = friction;
	}
	

	void SetPenetrationDepth(const Maths::HF32 PCREF penDepth)
	{
		m_Coefficients.X(penDepth);
	}

	void SetFriction(const Maths::HVec3 PCREF friction)
	{
		m_Friction = friction;
	}

	void SetRestitution(const Maths::HF32 PCREF restitution)
	{
		m_Coefficients.Y(restitution);
	}

	Maths::HF32 GetPenetrationDepth()
	{
		return m_Coefficients.X();
	}

	Maths::HVec3 GetFriction()
	{
		return m_Friction;
	}

	Maths::HF32 GetRestitution()
	{
		return m_Coefficients.Y();
	}

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}

	virtual void DebugRender(DbUtil::I_DebugRender* pRender)
	{
	}

	virtual void Serialise(){};
	virtual void Deserialise(){};

	Maths::HVec3 m_ContactPoint;
	Maths::C_Normal m_Normal;
	Maths::HVec3 m_Friction;
	Maths::HVec2 m_Coefficients;

	SharedPhysics::C_ContactStreamPoint* m_pContactPoint; //To write the contact data back to...
};


class C_DistanceConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_DistanceConstraint>
{
public:

	C_DistanceConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_Distance),
		m_AnchorPoint1(Maths::Constants<Maths::HVec3>::Zero()), m_AnchorPoint2(Maths::Constants<Maths::HVec3>::Zero()),
		m_Slop(Maths::Constants<Maths::HF32>::Zero()), m_DistanceLimit(Maths::Constants<Maths::HF32>::Zero()), m_DynamicScale(Maths::Constants<Maths::HF32>::Zero()),
		m_BreakError(Maths::Constants<Maths::HF32>::Max())
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);


	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}

	virtual bool ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset);

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	void DebugRender( DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorAW, const Maths::HVec3 PCREF anchorBW);



	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_AnchorPoint2 << m_Slop << m_DistanceLimit << m_DynamicScale << m_BreakError;
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		serial >> m_AnchorPoint1 >> m_AnchorPoint2 >> m_Slop >> m_DistanceLimit >> m_DynamicScale >> m_BreakError;
	}

	Maths::HVec3 m_AnchorPoint1; //which is the anchor point of rigidbody1, define in the body space and use to linked two rigid bodies together
	Maths::HVec3 m_AnchorPoint2; //which is the anchor point of rigidbody2, define in the body space and use to linked two rigid bodies together
	Maths::HF32 m_Slop;
	Maths::HF32 m_DistanceLimit;
	Maths::HF32 m_DynamicScale;
	Maths::HF32 m_BreakError;

	//SharedPhysics::C_ContactStreamPoint* m_pContactPoint; //To write the contact data back to...
};

class C_SphericalConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_SphericalConstraint>
{
public:

	C_SphericalConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_Spherical),
		m_AnchorPoint1(Maths::Constants<Maths::HVec3>::Zero()), m_AnchorPoint2(Maths::Constants<Maths::HVec3>::Zero()),
		m_Slop(Maths::Constants<Maths::HF32>::Zero()), m_BreakErrorSq(Maths::Constants<Maths::HF32>::Max())
	{
	}

    #if 0 //defined( PLATFORM_SPU )
	C_SphericalConstraint( const Spu::C_ConstructVirtualTable& cvt ) : C_Constraint( cvt ) {}
	#endif
	
	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}
    
    #if !defined( PLATFORM_SPU )
    
	virtual bool ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset);

	#endif

	virtual void PostStabilise();

	virtual void PostStabilise(Gravity::C_Body* pBodyA, Gravity::C_Body* pBodyB);

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_AnchorPoint2 << m_Slop << m_BreakErrorSq;
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		serial >> m_AnchorPoint1 >> m_AnchorPoint2 >> m_Slop >> m_BreakErrorSq;
	}

	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorPointAW, const Maths::HVec3 PCREF anchorPointBW);
	
	
	Maths::HVec3 m_AnchorPoint1; //which is the anchor point of rigidbody1, define in the body space and use to linked two rigid bodies together
	Maths::HVec3 m_AnchorPoint2; //which is the anchor point of rigidbody2, define in the body space and use to linked two rigid bodies together
	Maths::HF32 m_Slop;
	Maths::HF32 m_BreakErrorSq;
	//SharedPhysics::C_ContactStreamPoint* m_pContactPoint; //To write the contact data back to...
};

class C_SphericalLimitConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_SphericalLimitConstraint>
{
public:

	C_SphericalLimitConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_SphericalLimit), m_AxisOrder(SharedPhysics::C_Constraint::E_AxisOrder_XYZ),
		m_LowJointLimit(Maths::UNDEFINED), m_HighJointLimit(Maths::UNDEFINED)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);


	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}

	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_LowJointLimit << m_HighJointLimit << Maths::HMat33(m_rotationalOffSet);
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		Maths::HMat33 m;
		serial >> m_LowJointLimit >> m_HighJointLimit >> m;
		m_rotationalOffSet = m;
	}


	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorPointAW, const Maths::HVec3 PCREF anchorPointBW);

	
	Maths::HVec3 m_LowJointLimit;	//in radian
	Maths::HVec3 m_HighJointLimit; // in radian

	Maths::Mat33 m_rotationalOffSet;

	SharedPhysics::C_Constraint::E_AxisOrder m_AxisOrder;
};



class C_RevoluteConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_RevoluteConstraint>
{
public:

	C_RevoluteConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_Revolute), 
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_BreakErrorSq(Maths::Constants<Maths::HF32>::Max())
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}

	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_JointAxis1 << m_JointAxis2 << Maths::C_Quaternion(m_rotationalOffSet) << m_BreakErrorSq;
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		Maths::C_Quaternion q;
		serial >> m_JointAxis1 >> m_JointAxis2 >> q >> m_BreakErrorSq;
		m_rotationalOffSet = q;
	}

	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB);

	
	Maths::C_Normal m_JointAxis1;	//the joint axis in the rigidbody1's local space and need to be unit vector
	Maths::C_Normal m_JointAxis2;	//the joint axis in the rigidbody2's loca space and need to be unit vector
	Maths::HF32 m_BreakErrorSq;
	Maths::Quaternion m_rotationalOffSet;
};


class C_PrismaticConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_PrismaticConstraint>
{
public:

	C_PrismaticConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_Prismatic),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_OffSetA(Maths::UNDEFINED), m_OffSetB(Maths::UNDEFINED)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	virtual bool ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset);
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}
	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}
	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_JointAxis1 << m_JointAxis2 << m_OffSetA << m_OffSetB;
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		serial >> m_JointAxis1 >> m_JointAxis2 >> m_OffSetA >> m_OffSetB;
	}

	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB);
	
	Maths::C_Normal m_JointAxis1;	//the joint axis in the rigidbody1's local space and need to be unit vector
	Maths::C_Normal m_JointAxis2;	//the joint axis in the rigidbody2's loca space and need to be unit vector
	//SharedPhysics::C_ContactStreamPoint* m_pContactPoint; //To write the contact data back to...
	Maths::HVec3 m_OffSetA, m_OffSetB; //Inverse(R(j))*(centerMass(j) - centerMass(i)), vector in j's local space
};


class C_CarWheelConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_CarWheelConstraint>
{
public:

	public:

	C_CarWheelConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_CarWheel),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_OffSetB(Maths::UNDEFINED), m_OffSetA(Maths::UNDEFINED), m_HighJointLimit(Maths::UNDEFINED), m_LowJointLimit(Maths::UNDEFINED)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}
	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_JointAxis1 << m_JointAxis2 << m_OffSetA << m_OffSetB << m_HighJointLimit << m_LowJointLimit ; 
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		serial >> m_JointAxis1 >> m_JointAxis2 >> m_OffSetA >> m_OffSetB >> m_HighJointLimit >> m_LowJointLimit;
	}

	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB);

	virtual bool ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset);
	
	Maths::C_Normal m_JointAxis1;	//the joint axis in the rigidbody1's local space and need to be unit vector
	Maths::C_Normal m_JointAxis2;	//the joint axis in the rigidbody2's loca space and need to be unit vector
	Maths::HVec3 m_OffSetB; //two axis original angle differents
	Maths::HVec3 m_OffSetA; //two axis original angle differents
	Maths::HF32 m_HighJointLimit;
	Maths::HF32 m_LowJointLimit;
};

class C_TwoRotationalConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_TwoRotationalConstraint>
{
public:

	C_TwoRotationalConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_TwoRotational),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}
	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << Maths::C_Quaternion(m_rotationalOffset);
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		Maths::C_Quaternion q;
		serial >> q;
		m_rotationalOffset = q;
	}

	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorPointAW, const Maths::HVec3 PCREF anchorPointBW);
	
	Maths::C_Normal m_JointAxis1;	//the joint axis in the rigidbody1's local space and need to be unit vector
	Maths::C_Normal m_JointAxis2;	//the joint axis in the rigidbody2's loca space and need to be unit vector

	Maths::Quaternion m_rotationalOffset;
};

class C_PrismaticLimitConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_PrismaticLimitConstraint>
{
public:

	C_PrismaticLimitConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_PrismaticLimit),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_OffSetA(Maths::UNDEFINED), m_OffSetB(Maths::UNDEFINED),
		m_LowJointLimit(Maths::UNDEFINED), m_HighJointLimit(Maths::UNDEFINED)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}

	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_JointAxis1 << m_JointAxis2 << m_LowJointLimit << m_HighJointLimit << m_OffSetA << m_OffSetB; 
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		serial >> m_JointAxis1 >> m_JointAxis2 >> m_LowJointLimit >> m_HighJointLimit >> m_OffSetA >> m_OffSetB; 
	}

	virtual bool ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset);

	
	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB);

	Maths::C_Normal m_JointAxis1;	//the joint axis in the rigidbody1's local space and need to be unit vector
	Maths::C_Normal m_JointAxis2;	//the joint axis in the rigidbody2's loca space and need to be unit vector
	//SharedPhysics::C_ContactStreamPoint* m_pContactPoint; //To write the contact data back to...
	Maths::HVec3 m_OffSetA, m_OffSetB; //Inverse(R(j))*(centerMass(j) - centerMass(i)), vector in j's local space

	Maths::HF32	m_LowJointLimit;
	Maths::HF32 m_HighJointLimit;
};

class C_RevoluteLimitConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_RevoluteLimitConstraint>
{
public:

	C_RevoluteLimitConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_RevoluteLimit),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_LowJointLimit(Maths::UNDEFINED), m_HighJointLimit(Maths::UNDEFINED), m_ErrorLimit(Maths::UNDEFINED)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2);

	virtual E_ConstraintState GetConstraintState();

	virtual void Release()
	{
		//Do nothing - this is an internally-managed constraint type.
		//This will need to be done for externally accessible constraint types, however!
	}
	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}

	virtual void PostStabilise()
	{

	}

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << m_JointAxis1 << m_JointAxis2 << m_LowJointLimit << m_HighJointLimit << Maths::C_Quaternion(m_rotationalOffSet) << m_ErrorLimit; 
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		Maths::C_Quaternion q;
		serial >> m_JointAxis1 >> m_JointAxis2 >> m_LowJointLimit >> m_HighJointLimit >> q >> m_ErrorLimit;
		m_rotationalOffSet = q;
	}


	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF positionA, const Maths::HVec3 PCREF jointA, const Maths::HVec3 PCREF positionB, const Maths::HVec3 PCREF jointB);

	Maths::C_Normal m_JointAxis1;	//the joint axis in the rigidbody1's local space and need to be unit vector
	Maths::C_Normal m_JointAxis2;	//the joint axis in the rigidbody2's loca space and need to be unit vector
	//SharedPhysics::C_ContactStreamPoint* m_pContactPoint; //To write the contact data back to...
	Maths::Quaternion m_rotationalOffSet;

	Maths::HF32	m_LowJointLimit;
	Maths::HF32 m_HighJointLimit;
	Maths::HVec2 m_ErrorLimit;
	//Maths::HVec3 m_AnchorPoint1;
	//Maths::HVec3 m_AnchorPoint2;

};

class C_FixedRotationalConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_FixedRotationalConstraint>
{
public:
	C_FixedRotationalConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_FixedRotational)
	{
	}

	void CreateJacobian(C_Jacobian* jaco);
	void CreateInvertMJT(C_InvertMJT* imjt, C_Jacobian* jaco);
	virtual void SetRotationalScale(const F32 scale)
	{
		m_Scale = scale;
	}
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2, bool swap);

	virtual void Release()
	{
	}
	
	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		serial << Maths::C_Quaternion(m_rotationalOffSet);
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		Maths::C_Quaternion q;
		serial >> q;
		m_rotationalOffSet = q;
	}

	virtual void DebugRender(DbUtil::I_DebugRender* pRender);

	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HMat43& matA, const Maths::HMat43& matB);

	Maths::Quaternion m_rotationalOffSet;
};

/*
	if E_ConstraintType == E_Type_SphericalLimit
			m_Constraints[0] = sphericalJoint;
			m_Constraints[1] = sphericalJointLimit;
	if E_ConstraintType == E_Type_Fix
			m_Constraints[0] = fixedRotationalJoint;
			m_Constraints[1] = sphericalJoint;
	if E_ConstraintType == E_Type_Revolute
			m_Constraints[0] = revoluteJoint;
			m_Constraints[1] = sphericalJoint;
	if E_ConstraintType == E_Type_RevoluteLimit
			m_Constraints[0] = revoluteJoint;
			m_Constraints[1] = sphericalJoint;
			m_Constraints[2] = revoluteJointLimit;
	if E_ConstraintType == E_Type_Prismatic
			m_Constraints[0] = fixedRotationalJoint;
			m_Constraints[1] = prismaticJoint;
	if E_ConstraintType == E_Type_PrismaticLimit
			m_Constraints[0] = fixedRotationalJoint;
			m_Constraints[1] = prismaticJointLimit;
	if E_ConstraintType == E_Type_CarWheel
			m_Constraints[0] = revoluteJoint;
			m_Constraints[1] = carWheelJoint;
	if E_ConstraintType == E_Type_TwoRotational
			m_Constraints[0] = sphericalJoint;
			m_Constraints[1] = twoRotational;
*/
class C_ProxyConstraint : public SharedPhysics::C_Constraint, public SharedPhysics::C_BlockItem<C_ProxyConstraint>
{
public:
	C_ProxyConstraint() : SharedPhysics::C_Constraint(SharedPhysics::C_Constraint::E_Type_Proxy)
	{
	}

	void SetConstraintType(SharedPhysics::C_Constraint::E_ConstraintType constraintType)
	{
		m_ProxyConstraintType = constraintType;
	}
	//void PostConstraint(SharedPhysics::C_GraphNode* graphNode1,  SharedPhysics::C_GraphNode* graphNode2, bool swap);

	virtual void Release()
	{
	}

	template <typename Serialiser>
	void SaveState(Serialiser& serial)
	{
		for(S32 i=0; i< m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SaveState<Serialiser>(serial);
		}
	}

	template <typename Deserialiser>
	void LoadState(Deserialiser& serial)
	{
		for(S32 i=0; i< m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->LoadState<Deserialiser>(serial);
		}
	}


	virtual void DebugRender(DbUtil::I_DebugRender* pRender);
	void DebugRender(DbUtil::I_DebugRender* pRender, const Maths::HVec3 PCREF anchorPointAW, const Maths::HVec3 PCREF anchorPointBW);


	virtual void SetScale(const F32 scale)
	{
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetScale(scale);
		}
		m_Scale = scale;
	}

	virtual void SetRotationalScale(const F32 scale)
	{
		//m_pRevoluteConstraint->SetScale(scale);
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetRotationalScale(scale);
		}
	}

	virtual void PostStabilise()
	{
		//m_pSphericalConstraint->PostStabilise();
		for(S32 i = 0; i < m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->PostStabilise();
		}
	}

	virtual void PostStabilise(Gravity::C_Body* pBodyA, Gravity::C_Body* pBodyB)
	{
		for(S32 i = 0; i < m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->PostStabilise(pBodyA, pBodyB);
		}
	}

#ifndef PLATFORM_SPU
	virtual void SetBodyAMassScale(const Maths::HF32 PCREF scale)
	{
		m_MassScales.X(scale);
		m_MassScales.Z(scale);
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetBodyAMassScale(scale);
		}
	}
	
	virtual void SetBodyBMassScale(const Maths::HF32 PCREF scale)
	{
		m_MassScales.Y(scale);
		m_MassScales.W(scale);
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetBodyBMassScale(scale);
		}
	}

	virtual void SetBodyAInertiaScale(const Maths::HF32 PCREF scale)
	{
		m_MassScales.Z(scale);
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetBodyAInertiaScale(scale);
		}
	}
	
	virtual void SetBodyBInertiaScale(const Maths::HF32 PCREF scale)
	{
		m_MassScales.W(scale);
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetBodyBInertiaScale(scale);
		}
	}

	virtual void NormaliseMassRatios(const Maths::HF32 PCREF maxMassRatio, const Maths::HF32 PCREF assumedInfiniteMassRatio)
	{
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->NormaliseMassRatios(maxMassRatio, assumedInfiniteMassRatio);
		}
	}

#endif

	virtual void SetBaumgarte(F32 baumgarte)
	{
		m_Baumgarte = baumgarte;
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetBaumgarte(baumgarte);
		}
	}

	virtual void SetUserData(void* data)
	{
		m_pUserData = data;
		for(S32 i=0; i<m_Constraints.m_numElements; ++i)
		{
			m_Constraints[i]->SetUserData(data);
		}
	}

	virtual bool ApplyBodyOffset(const Gravity::C_Body* body, const Maths::HVec3 PCREF offset)
	{
		//return m_pSphericalConstraint->ApplyBodyOffset(body, offset);
		//return false;
		for(S32 a = 0; a < m_Constraints.m_numElements; ++a)
		{
			m_Constraints[a]->ApplyBodyOffset(body, offset);
		}
		return true;
	}

	void AddConstraint(SharedPhysics::C_Constraint* pConstraint)
	{
		m_Constraints.AddToEnd(&pConstraint);
	}

	SharedPhysics::C_Constraint* GetConstraint(S32 index)
	{
		BIZ_ASSERT(index < m_Constraints.m_numElements);
		return m_Constraints[index];
	}

	SharedPhysics::C_Constraint::E_ConstraintType m_ProxyConstraintType;
	SharedPhysics::SortedArray<SharedPhysics::C_Constraint*> m_Constraints;
};

#ifndef PLATFORM_SPU
template <typename Serialiser>
void SaveState(SharedPhysics::C_Constraint* pConstraint, Serialiser& serial)
{
	switch(pConstraint->ConstraintType())
	{
	case SharedPhysics::C_Constraint::E_Type_Distance:
		{
			Solver2::C_DistanceConstraint* pDistance = (Solver2::C_DistanceConstraint*)pConstraint;
			pDistance->SaveState<Serialiser>(serial);
		}
		break;
	case SharedPhysics::C_Constraint::E_Type_Spherical:
		{
			Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*) pConstraint;
			pSpherical->SaveState<Serialiser>(serial);
		}
		break;
	case SharedPhysics::C_Constraint::E_Type_Proxy:
		{
			Solver2::C_ProxyConstraint* pProxy = (Solver2::C_ProxyConstraint*)pConstraint;

			switch(pProxy->m_ProxyConstraintType)
			{
			case SharedPhysics::C_Constraint::E_Type_SphericalLimit:
				{
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*) pProxy->m_Constraints[0];
					Solver2::C_SphericalLimitConstraint* pLimit = (Solver2::C_SphericalLimitConstraint*)pProxy->m_Constraints[1];
					pSpherical->SaveState<Serialiser>(serial);
					pLimit->SaveState<Serialiser>(serial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Fixed:
				{
					Solver2::C_FixedRotationalConstraint* pFix = (Solver2::C_FixedRotationalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[1];
					pFix->SaveState<Serialiser>(serial);
					pSpherical->SaveState<Serialiser>(serial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Revolute:
				{
					Solver2::C_RevoluteConstraint* pRevolute = (Solver2::C_RevoluteConstraint*)pProxy->m_Constraints[0];
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[1];
					pRevolute->SaveState<Serialiser>(serial);
					pSpherical->SaveState<Serialiser>(serial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_RevoluteLimit:
				{
					Solver2::C_RevoluteConstraint* pRevolute = (Solver2::C_RevoluteConstraint*)pProxy->m_Constraints[0];
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[1];
					Solver2::C_RevoluteLimitConstraint* pLimit = (Solver2::C_RevoluteLimitConstraint*)pProxy->m_Constraints[2];
					pRevolute->SaveState<Serialiser>(serial);
					pSpherical->SaveState<Serialiser>(serial);
					pLimit->SaveState<Serialiser>(serial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Prismatic:
				{
					Solver2::C_FixedRotationalConstraint* pFix = (Solver2::C_FixedRotationalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_PrismaticConstraint* pPrismatic = (Solver2::C_PrismaticConstraint*)pProxy->m_Constraints[1];
					pFix->SaveState<Serialiser>(serial);
					pPrismatic->SaveState<Serialiser>(serial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_PrismaticLimit:
				{
					Solver2::C_FixedRotationalConstraint* pFix = (Solver2::C_FixedRotationalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_PrismaticLimitConstraint* pPrismaticLimit = (Solver2::C_PrismaticLimitConstraint*)pProxy->m_Constraints[1];
					pFix->SaveState<Serialiser>(serial);
					pPrismaticLimit->SaveState<Serialiser>(serial);

				}
				break;
			case SharedPhysics::C_Constraint::E_Type_CarWheel:
				{
					Solver2::C_RevoluteConstraint* pRevolute = (Solver2::C_RevoluteConstraint*)pProxy->m_Constraints[0];
					Solver2::C_CarWheelConstraint* pCarWheel = (Solver2::C_CarWheelConstraint*)pProxy->m_Constraints[1];
					pRevolute->SaveState<Serialiser>(serial);
					pCarWheel->SaveState<Serialiser>(serial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_TwoRotational:
				{
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_TwoRotationalConstraint* pTwo = (Solver2::C_TwoRotationalConstraint*)pProxy->m_Constraints[1];
					pSpherical->SaveState<Serialiser>(serial);
					pTwo->SaveState<Serialiser>(serial);
				}
				break;
			default:
				break;
			};
		}
		break;
	default:
		break;
	};
}

template <typename Deserialiser>
void LoadState(SharedPhysics::C_Constraint* pConstraint, Deserialiser& deserial)
{
	switch(pConstraint->ConstraintType())
	{
	case SharedPhysics::C_Constraint::E_Type_Distance:
		{
			Solver2::C_DistanceConstraint* pDistance = (Solver2::C_DistanceConstraint*)pConstraint;
			pDistance->LoadState<Deserialiser>(deserial);
		}
		break;
	case SharedPhysics::C_Constraint::E_Type_Spherical:
		{
			Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*) pConstraint;
			pSpherical->LoadState<Deserialiser>(deserial);
		}
		break;
	case SharedPhysics::C_Constraint::E_Type_Proxy:
		{
			Solver2::C_ProxyConstraint* pProxy = (Solver2::C_ProxyConstraint*)pConstraint;

			switch(pProxy->m_ProxyConstraintType)
			{
			case SharedPhysics::C_Constraint::E_Type_SphericalLimit:
				{
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*) pProxy->m_Constraints[0];
					Solver2::C_SphericalLimitConstraint* pLimit = (Solver2::C_SphericalLimitConstraint*)pProxy->m_Constraints[1];
					pSpherical->LoadState<Deserialiser>(deserial);
					pLimit->LoadState<Deserialiser>(deserial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Fixed:
				{
					Solver2::C_FixedRotationalConstraint* pFix = (Solver2::C_FixedRotationalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[1];
					pFix->LoadState<Deserialiser>(deserial);
					pSpherical->LoadState<Deserialiser>(deserial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Revolute:
				{
					Solver2::C_RevoluteConstraint* pRevolute = (Solver2::C_RevoluteConstraint*)pProxy->m_Constraints[0];
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[1];
					pRevolute->LoadState<Deserialiser>(deserial);
					pSpherical->LoadState<Deserialiser>(deserial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_RevoluteLimit:
				{
					Solver2::C_RevoluteConstraint* pRevolute = (Solver2::C_RevoluteConstraint*)pProxy->m_Constraints[0];
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[1];
					Solver2::C_RevoluteLimitConstraint* pLimit = (Solver2::C_RevoluteLimitConstraint*)pProxy->m_Constraints[2];
					pRevolute->LoadState<Deserialiser>(deserial);
					pSpherical->LoadState<Deserialiser>(deserial);
					pLimit->LoadState<Deserialiser>(deserial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_Prismatic:
				{
					Solver2::C_FixedRotationalConstraint* pFix = (Solver2::C_FixedRotationalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_PrismaticConstraint* pPrismatic = (Solver2::C_PrismaticConstraint*)pProxy->m_Constraints[1];
					pFix->LoadState<Deserialiser>(deserial);
					pPrismatic->LoadState<Deserialiser>(deserial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_PrismaticLimit:
				{
					Solver2::C_FixedRotationalConstraint* pFix = (Solver2::C_FixedRotationalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_PrismaticLimitConstraint* pPrismaticLimit = (Solver2::C_PrismaticLimitConstraint*)pProxy->m_Constraints[1];
					pFix->LoadState<Deserialiser>(deserial);
					pPrismaticLimit->LoadState<Deserialiser>(deserial);

				}
				break;
			case SharedPhysics::C_Constraint::E_Type_CarWheel:
				{
					Solver2::C_RevoluteConstraint* pRevolute = (Solver2::C_RevoluteConstraint*)pProxy->m_Constraints[0];
					Solver2::C_CarWheelConstraint* pCarWheel = (Solver2::C_CarWheelConstraint*)pProxy->m_Constraints[1];
					pRevolute->LoadState<Deserialiser>(deserial);
					pCarWheel->LoadState<Deserialiser>(deserial);
				}
				break;
			case SharedPhysics::C_Constraint::E_Type_TwoRotational:
				{
					Solver2::C_SphericalConstraint* pSpherical = (Solver2::C_SphericalConstraint*)pProxy->m_Constraints[0];
					Solver2::C_TwoRotationalConstraint* pTwo = (Solver2::C_TwoRotationalConstraint*)pProxy->m_Constraints[1];
					pSpherical->LoadState<Deserialiser>(deserial);
					pTwo->LoadState<Deserialiser>(deserial);
				}
				break;
			default:
				break;
			};
		}
		break;
	default:
		break;
	};

}

#endif


};

#endif
