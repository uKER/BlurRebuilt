#ifndef CONSTRAINTDESCRIPTORS_H
#define CONSTRAINTDESCRIPTORS_H

//#include "gravity/sdk/Body.h"
#include "gravity/sdk/ConstraintDescriptor.h"
#include <gravity/sdk/ShareDeserialiser.h>
#include "gravity/sharedphysics/GraphNode.h"
//#include "gravity/solver2/Constraint.h"
//#include "gravity/solver2/ConstraintGraphManager.h"


namespace Solver2
{
	class C_ConstraintGraphManager;
}

namespace SharedPhysics
{
	class C_Spring;
}

namespace Gravity
{

class C_Body;
class C_Scene;

//template <class Deserialiser>
class SphericalJointDescriptor : public C_ConstraintDescriptors
{
public:
	SphericalJointDescriptor(): C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_Spherical), m_BreakError(Maths::Constants<Maths::HF32>::Max()),
		m_AnchorPoint1(Maths::UNDEFINED), m_AnchorPoint2(Maths::UNDEFINED)
	{
	}

	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	Maths::HF32 m_BreakError;

#ifndef GRAVITY_EXPORTER
	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
#endif

	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);

	void SetBreakError(const Maths::HF32 PCREF breakError)
	{
		m_BreakError = breakError;
	}


	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		deserial >> m_AnchorPoint1 >> m_AnchorPoint2 >> m_BreakError;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_AnchorPoint2 << m_BreakError;
	}
};

class SphericalJointLimitDescriptor : public C_ConstraintDescriptors
{
public:
	SphericalJointLimitDescriptor(): C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_SphericalLimit),
		m_AnchorPoint1(Maths::UNDEFINED), m_AnchorPoint2(Maths::UNDEFINED), m_LowJointLimit(Maths::UNDEFINED),
		m_HighJointLimit(Maths::UNDEFINED)
	{
	}



	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	Maths::HVec3 m_LowJointLimit;
	Maths::HVec3 m_HighJointLimit;

	Maths::Quaternion m_rotationalOffset;
	SharedPhysics::C_Constraint::E_AxisOrder m_AxisOrder;

#ifndef GRAVITY_EXPORTER
	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
	virtual void Import(Deserialiser& deserial);
#endif

	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);
	//void DefineAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);
	void DefineLimits(const Maths::HVec3 PCREF lowLimit, const Maths::HVec3 PCREF highLimit);
	
//	{
//#if !CFG_CONVERTER
//		ImportBasic(deserial);
//
//		Maths::HVec3 lowJointLimit(Maths::UNDEFINED);
//		Maths::HVec3 highJointLimit(Maths::UNDEFINED);
//		deserial >> m_AnchorPoint1 >> m_AnchorPoint2 >> lowJointLimit >> highJointLimit;
//
//		//m_LowJointLimit and m_HighJointLimit is based on radian
//		DefineLimits(lowJointLimit, highJointLimit);
//#endif
//	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		//m_LowJointLimit and m_HighJointLimit is based on degree
		serial << m_AnchorPoint1 << m_AnchorPoint2 << m_LowJointLimit << m_HighJointLimit;
	}
};

class RotationalLimitDescriptor : public C_ConstraintDescriptors
{
public:
	RotationalLimitDescriptor(): C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_RotationalLimit),
		m_LowJointLimit(Maths::UNDEFINED), m_HighJointLimit(Maths::UNDEFINED)
	{
	}

	Maths::HVec3 m_LowJointLimit;
	Maths::HVec3 m_HighJointLimit;
	
	Maths::Quaternion m_rotationalOffset;
	SharedPhysics::C_Constraint::E_AxisOrder m_AxisOrder;

#ifndef GRAVITY_EXPORTER
	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
	virtual void Import(Deserialiser& deserial);
#endif

	void DefineLimits(const Maths::HVec3 PCREF lowLimit, const Maths::HVec3 PCREF highLimit);

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		//m_LowJointLimit and HighJointLimit in degree
		serial << m_LowJointLimit << m_HighJointLimit;
	}
};

class DistanceConstraintDescriptor : public C_ConstraintDescriptors
{
public:
	DistanceConstraintDescriptor(): C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_Distance),
		m_AnchorPoint1(Maths::UNDEFINED), m_AnchorPoint2(Maths::UNDEFINED), m_DistanceLimit(Maths::UNDEFINED)
	{
	}

	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	Maths::HF32 m_DistanceLimit;

#ifndef GRAVITY_EXPORTER
	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
#endif

	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);
	void DefineDistanceLimit(const Maths::HF32 PCREF distanceLimit);

	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		deserial >> m_AnchorPoint1 >> m_AnchorPoint2;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_AnchorPoint2;
	}
};

class FixedJointDescriptor : public C_ConstraintDescriptors
{
public:
	FixedJointDescriptor(): C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_Fixed),
		m_AnchorPoint1(Maths::UNDEFINED), m_AnchorPoint2(Maths::UNDEFINED)
	{
	}
	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	
	
#ifndef GRAVITY_EXPORTER

	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
#endif


	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);

	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		deserial >> m_AnchorPoint1 >> m_AnchorPoint2;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_AnchorPoint1  << m_AnchorPoint2 ;
	}

};


class FixedRotationalDescriptor : public C_ConstraintDescriptors
{
public:
	FixedRotationalDescriptor(): C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_FixedRotational)
	{
	}

#ifndef GRAVITY_EXPORTER

	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
#endif
};


class RevoluteJointDescriptor : public C_ConstraintDescriptors
{
public:
	RevoluteJointDescriptor(): C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_Revolute), 
	  m_AnchorPoint1(Maths::UNDEFINED), m_AnchorPoint2(Maths::UNDEFINED), m_JointAxis1(Maths::UNDEFINED),
		  m_JointAxis2(Maths::UNDEFINED), m_BreakError(Maths::Constants<Maths::HF32>::Max())
	{
	}
	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	Maths::HVec3 m_JointAxis1;
	Maths::HVec3 m_JointAxis2;
	Maths::HF32 m_BreakError;
	
#ifndef GRAVITY_EXPORTER
	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
#endif


	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);

	void DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);

	void SetBreakError(const Maths::HF32 PCREF breakError)
	{
		m_BreakError = breakError;
	}

	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);

		deserial >> m_AnchorPoint1 >> m_JointAxis1 >> m_AnchorPoint2 >> m_JointAxis2 >> m_BreakError;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_JointAxis1 << m_AnchorPoint2 << m_JointAxis2 << m_BreakError;
	}
	

};


class RevoluteJointLimitDescriptor : public C_ConstraintDescriptors
{
public:
	RevoluteJointLimitDescriptor() : C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_RevoluteLimit),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_AnchorPoint1(Maths::UNDEFINED),
		m_AnchorPoint2(Maths::UNDEFINED), m_LowJointLimit(Maths::UNDEFINED), m_HighJointLimit(Maths::UNDEFINED)
	{
	}
	Maths::HVec3 m_JointAxis1;
	Maths::HVec3 m_JointAxis2;
	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	Maths::HF32  m_LowJointLimit;
	Maths::HF32  m_HighJointLimit;
	

#ifndef GRAVITY_EXPORTER

	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;
	virtual void Import(Deserialiser& deserial);
#endif

	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);

	void DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);

	/*
		When we sets up the revolute joint limits, make sure the the lowLimit > highLimit.
	*/
	void DefineJointLimit(const Maths::HF32 PCREF lowLimit, const Maths::HF32 PCREF highLimit);


	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_AnchorPoint2 << m_JointAxis1  << m_JointAxis2 << m_LowJointLimit << m_HighJointLimit;
	}
};


class PrismaticJointDescriptor : public C_ConstraintDescriptors
{
public:
	PrismaticJointDescriptor() : C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_Prismatic), 
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED)
	{
	}
	Maths::HVec3 m_JointAxis1;
	Maths::HVec3 m_JointAxis2;
	
#ifndef GRAVITY_EXPORTER

	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;

#endif
	
	void DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);

	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		deserial  >> m_JointAxis1  >> m_JointAxis2;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_JointAxis1 << m_JointAxis2;
	}

};

class PrismaticJointLimitDescriptor : public C_ConstraintDescriptors
{
public:
	PrismaticJointLimitDescriptor() : C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_PrismaticLimit),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_LowJointLimit(Maths::UNDEFINED), 
		m_HighJointLimit(Maths::UNDEFINED)
	{
	}
	Maths::HVec3 m_JointAxis1;
	Maths::HVec3 m_JointAxis2;
	Maths::HF32  m_LowJointLimit;
	Maths::HF32  m_HighJointLimit;

#ifndef GRAVITY_EXPORTER

	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;

#endif

	void DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);

	/*
	  When we set up the prismatic joint limit, make sure the lowLimit is set to be negative and highLimit
	  is set to be positive. The reason for that is because the lowLimit is the limit boundary on the opposite
	  side of the joint axis, therefore, the sign distance should be negative, vice-versa for the highLimit 
	*/
	void DefineJointLimit(const Maths::HF32 PCREF lowLimit, const Maths::HF32 PCREF highLimit);


	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		deserial >> m_JointAxis1 >> m_JointAxis2 >> m_LowJointLimit >> m_HighJointLimit;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_JointAxis1 << m_JointAxis2 << m_LowJointLimit << m_HighJointLimit;
	}
	
}; 


class CarWheelDescriptor : public C_ConstraintDescriptors
{
public:
	CarWheelDescriptor() : C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_CarWheel),
		m_RevoluteJointAxis1(Maths::UNDEFINED), m_RevoluteJointAxis2(Maths::UNDEFINED), m_PrismaticJointAxis1(Maths::UNDEFINED),
		m_PrismaticJointAxis2(Maths::UNDEFINED), m_PrismaticLowJointLimit(Maths::UNDEFINED), m_PrismaticHighJointLimit(Maths::UNDEFINED)
	{
	}

	Maths::HVec3 m_RevoluteJointAxis1;
	Maths::HVec3 m_RevoluteJointAxis2;
	Maths::HVec3 m_PrismaticJointAxis1;
	Maths::HVec3 m_PrismaticJointAxis2;
	Maths::HF32 m_PrismaticLowJointLimit;
	Maths::HF32 m_PrismaticHighJointLimit;

#ifndef GRAVITY_EXPORTER

	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;

#endif

	void DefineRevoluteJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);

	void DefinePrismaticJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);

	void DefinePrismaticLimits(const Maths::HF32 PCREF lowLimit, const Maths::HF32 PCREF highLimit);


	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		deserial >> m_RevoluteJointAxis1 >> m_RevoluteJointAxis2 >> m_PrismaticJointAxis1 >> m_PrismaticJointAxis2 >> m_PrismaticLowJointLimit >> m_PrismaticHighJointLimit;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_RevoluteJointAxis1 << m_RevoluteJointAxis2 << m_PrismaticJointAxis1 << m_PrismaticJointAxis2 << m_PrismaticLowJointLimit << m_PrismaticHighJointLimit;
	}

};


class TwoRotationalProxyDescriptor : public C_ConstraintDescriptors
{
public:
	TwoRotationalProxyDescriptor() : C_ConstraintDescriptors(SharedPhysics::C_Constraint::E_Type_TwoRotational),
		m_JointAxis1(Maths::UNDEFINED), m_JointAxis2(Maths::UNDEFINED), m_AnchorPoint1(Maths::UNDEFINED),
		m_AnchorPoint2(Maths::UNDEFINED)
	{
	}
	Maths::HVec3 m_JointAxis1;
	Maths::HVec3 m_JointAxis2;
	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	Maths::Quaternion m_rotationalOffset;

#ifndef GRAVITY_EXPORTER

	virtual SharedPhysics::C_Constraint* CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const;

#endif

	void DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2);

	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);
	
	//void DefineOffsetAngle(const Maths::HF32 PCREF angle);

	virtual void Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		deserial >> m_AnchorPoint1 >> m_JointAxis1 >> m_AnchorPoint2 >> m_JointAxis2 ;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_JointAxis1 << m_AnchorPoint2 << m_JointAxis2 ;
	}
};

class SpringDescriptor
{
public:

	SpringDescriptor() : m_AnchorPoint1(Maths::UNDEFINED), m_AnchorPoint2(Maths::UNDEFINED), 
		m_LinearSpringCoefficient(Maths::UNDEFINED), m_LinearDampingCoefficient(Maths::UNDEFINED),
		m_AngularSpringCoefficient(Maths::UNDEFINED), m_AngularDampingCoefficient(Maths::UNDEFINED),
		m_RestLength(Maths::UNDEFINED)
	{
	}

	Maths::HVec3 m_AnchorPoint1;
	Maths::HVec3 m_AnchorPoint2;
	Maths::HF32 m_LinearSpringCoefficient;	//spring coefficient
	Maths::HF32 m_LinearDampingCoefficient; //damping coefficient
	Maths::HF32 m_AngularSpringCoefficient;	//spring coefficient
	Maths::HF32 m_AngularDampingCoefficient; //damping coefficient
	Maths::HF32 m_RestLength;

#ifndef GRAVITY_EXPORTER
	SharedPhysics::C_Spring* CreateSpring(Gravity::C_Scene* pScene, C_Body* pBodyA, C_Body* pBodyB) const;
#endif

	void DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2);

	void DefineSpringCoefficients(const Maths::HF32 PCREF linearSpring, const Maths::HF32 linearDamp,
		const Maths::HF32 angularSpring, const Maths::HF32 angularDamp);

	void DefineRestLength(const Maths::HF32 PCREF restLength);

	void Import(Deserialiser& deserial)
	{

		deserial >> m_AnchorPoint1 >> m_AnchorPoint2 >> m_LinearSpringCoefficient 
			>> m_LinearDampingCoefficient >> m_AngularSpringCoefficient >> m_AngularDampingCoefficient >> m_RestLength;
	}

	template <typename Serialiser>
	void Export(Serialiser& serial)
	{
		serial << m_AnchorPoint1 << m_AnchorPoint2 << m_LinearSpringCoefficient 
			<< m_LinearDampingCoefficient << m_AngularSpringCoefficient << m_AngularDampingCoefficient << m_RestLength;
	}
};

//class C_ConstraintProxyDescriptor
//{
//public:
//	C_ConstraintProxyDescriptor(U32 maxNumConstraint = 0): m_maxNumConstraint(numConstraint), m_numConstraint(0)
//	{
//		constraints = new SharedPhysics::C_Constraint*[maxNumConstraint];
//	}
//
//	~C_ConstraintProxyDescriptor()
//	{
//		//The individual constraint is owned by the constraint manager, so we can't delete the constraint here
//		delete [] constraints;
//	}
//
//	void SetConstraint(SharedPhysics::C_Constraint* pConstraints)
//	{
//		m_Constraints[m_numConstraint++] = pConstraints;
//	}
//
//	SharedPhysics::C_Constraint* GetConstraint(U32 index)
//	{
//		BIZ_ASSERT(index < m_maxNumConstraint);
//		return m_Constraints[index];
//	}
//
//	Maths::HVec3 m_AnchorPoint1;
//	Maths::HVec3 m_AnchorPoint2;
//	Maths::HVec3 m_RevoluteJointAxis1;
//	Maths::HVec3 m_RevoluteJointAxis2;
//	Maths::HVec3 m_PrismaticJointAxis1;
//	Maths::HVec3 m_PrismaticJointAxis2;
//	Maths::HF32 m_PrismaticLowJointLimit;
//	Maths::HF32 m_PrismaticHighJointLimit;
//	Maths::HF32 m_RevoluteLowJointLimit;
//	Maths::HF32 m_RevoluteHighJointLimit;
//
//	SharedPhysics::C_Constraint** m_Constraints;
//	U32 m_maxNumConstraint;
//	U32 m_numConstraint;
//};

}

//#include "gravity/sdk/ConstraintDescriptors.inl"

#endif
