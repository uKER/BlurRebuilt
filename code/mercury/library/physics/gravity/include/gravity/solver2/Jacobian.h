#ifndef SOLVER2_JACOBIAN_H
#define SOLVER2_JACOBIAN_H

#include "gravity/sdk/Body.h"
//#include "gravity/sharedphysics/GraphNode.h"
//#include "gravity/solver2/RevoluteJointLimitDefinition.h"


namespace Solver2
{

//struct J_Vector
//{
//	F32* m_pJ;
//	const U32 width;
//	F32 *operator [] (const U32 index)
//	{
//		return &m_pJ[index * width];
//	}
//};

class ALIGN16 C_Jacobian
{
public:

	C_Jacobian():m_WarmStartValue(0.f), /*m_Coefficient(0.f), m_CoefficientOfResititution(0.f), m_CofRSetting(0.f),*/ m_Baumgarte(0.8f)
	{
	}

	C_Jacobian(F32 warmStart): m_WarmStartValue(warmStart), m_Baumgarte(0.8f)
	{
	}


    virtual ~C_Jacobian(){}

	NOALIAS FORCE_INLINE const F32 GetWarmStartValue() const
	{
		return m_WarmStartValue;
	}

	bool TestNodeFlags(const U32& flag) const
	{
		return (m_NodeBFlag & flag) != 0;
	}

	C_Jacobian* m_pPPUPtr;

	//SharedPhysics::C_GraphNode* m_pRigid1;
	//SharedPhysics::C_GraphNode* m_pRigid2;
	Gravity::C_ConstraintBody* m_pSpuConstraintBody1;
	Gravity::C_ConstraintBody* m_pSpuConstraintBody2;
	U32 m_GraphNodeIslandIndexA;
	U32 m_GraphNodeIslandIndexB;
	SharedPhysics::C_Constraint* m_pConstraint;

	//F32* m_Error; // use to correction the positional or ratational error
	/*F32 m_Coefficient;
	F32 m_CoefficientOfRestitution;	
	F32 m_CofRSetting;*/

	//Maths::Vector3 m_CoefficientSetting; //X - m_Coefficient, Y - m_CoefficientOfRestitution, Z - m_CofRSetting
	F32 m_WarmStartValue;
	F32 m_Baumgarte;
	U32 m_NodeBFlag;


};

class C_ContactJacobian : public C_Jacobian
{
public:
	C_ContactJacobian(): C_Jacobian(0.8f), m_CoefficientSetting(Maths::Const<Maths::Vector4>::Zero())
	{
	}

	virtual ~C_ContactJacobian()
	{
	}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);
	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);
	
	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector3 PCREF diagonal, Maths::Vector3& dif,  const Maths::VF32 PCREF totalDif);

	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector3 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector3 PCREF diagonal);

	Maths::Vector3 UpdateStaticLambda(const Maths::Vector6& imjtLam1, const Maths::Vector3 PCREF diagonal);

	Maths::Mat36 _m_J1;
	Maths::Mat36 _m_J2;
	Maths::Vector3 _m_Lambda;// 3X1
	Maths::Vector3 _m_BVector;//b=J*(v0 + time*Inv(M)*Fext); 3X1
	Maths::VF32 _m_Error; //contact's error is the penetration depth in the contact normal direction
	Maths::Vector4 m_CoefficientSetting; //X - friction coefficient U, Y - friction coefficient V Z - m_CoefficientOfRestitution, Z - m_CofRSetting
	bool m_StaticConstraint;
};

class C_FixedRotationalJacobian : public C_Jacobian
{
public:
	C_FixedRotationalJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_FixedRotationalJacobian()
	{
	}

	//void CalculateBVector( const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);
	
	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);
	
	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector3 PCREF diagonal,  Maths::Vector3& dif,  const Maths::VF32 PCREF totalDif);
	

	void CalculateBVector( const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector3 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector3 PCREF diagonal);
	

	Maths::Mat36 _m_J1;
	Maths::Mat36 _m_J2;
	Maths::Vector3 _m_Lambda;
	Maths::Vector3 _m_BVector;
	Maths::Vector3 _m_Error;
	
};


class C_DistanceJacobian : public C_Jacobian
{
public:


	C_DistanceJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_DistanceJacobian(){}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);

	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);

	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector3 PCREF diagonal, Maths::Vector3& dif,  const Maths::VF32 PCREF totalDif);
	
	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector3 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector3 PCREF diagonal);

	Maths::Mat36 _m_J1;
	Maths::Mat36 _m_J2;
	Maths::Vector3 _m_Lambda;
	Maths::Vector3 _m_BVector;
	Maths::Vector3 _m_Error;


};

class C_SphericalJointJacobian : public C_Jacobian
{
public:
	C_SphericalJointJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_SphericalJointJacobian(){}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);
	
	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);

	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector3 PCREF diagonal, Maths::Vector3& dif,  const Maths::VF32 PCREF totalDif);
	

	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector3 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector3 PCREF diagonal);
	

	Maths::Mat36 _m_J1;
	Maths::Mat36 _m_J2;
	Maths::Vector3 _m_Lambda;
	Maths::Vector3 _m_BVector;
	Maths::Vector3 _m_Error;

};

class C_SphericalJointLimitJacobian : public C_Jacobian
{
public:

	C_SphericalJointLimitJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_SphericalJointLimitJacobian(){}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);
	
	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);

	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector6 PCREF diagonal, Maths::Vector6& dif,  const Maths::VF32 PCREF totalDif);

	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector6 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector6 PCREF diagonal);
	

	Maths::Mat66 _m_J1;
	Maths::Mat66 _m_J2;
	Maths::Vector6 _m_Lambda;
	Maths::Vector6 _m_BVector;
	Maths::Vector6 _m_Error;

	

};

class C_TwoRotationalJacobian : public C_Jacobian
{
public:

	C_TwoRotationalJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_TwoRotationalJacobian(){}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);
	
	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);

	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::VF32 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::VF32 PCREF diagonal);
	

	Maths::Vector6 _m_J1;
	Maths::Vector6 _m_J2;
	Maths::VF32 _m_Lambda;
	Maths::VF32 _m_BVector;
	Maths::VF32 _m_Error;

};


class C_RevoluteJointJacobian : public C_Jacobian
{
public:

	C_RevoluteJointJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_RevoluteJointJacobian()
	{
	}

	//void CalculateBVector( const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);
	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);
	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector2 PCREF diagonal, Maths::Vector2& dif,  const Maths::VF32 PCREF totalDif);
	
	void CalculateBVector( const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector2 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector2 PCREF diagonal);

	Maths::Mat26 _m_J1;
	Maths::Mat26 _m_J2;
	Maths::Vector2 _m_Lambda;
	Maths::Vector2 _m_BVector;
	Maths::Vector2 _m_Error;


};



class C_PrismaticJointJacobian : public C_Jacobian
{
public:

	C_PrismaticJointJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_PrismaticJointJacobian()
	{
	}

	//void CalculateBVector( const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);

	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);
	
	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector2 PCREF diagonal, Maths::Vector2& dif,  const Maths::VF32 PCREF totalDif);

	void CalculateBVector( const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector2 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector2 PCREF diagonal);
	
	Maths::Mat26 _m_J1;
	Maths::Mat26 _m_J2;
	Maths::Vector2 _m_Lambda;
	Maths::Vector2 _m_BVector;
	Maths::Vector2 _m_Error;

};


class C_CarWheelJointJacobian : public C_Jacobian
{
public:

	C_CarWheelJointJacobian() : C_Jacobian(0.2f)
	{
	}

	virtual ~C_CarWheelJointJacobian()
	{
	}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);

	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);
	
	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector4 PCREF diagonal, Maths::Vector4& dif,  const Maths::VF32 PCREF totalDif);

	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	
	Maths::Vector4 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector4 PCREF diagonal);

	Maths::Mat46 _m_J1;
	Maths::Mat46 _m_J2;
	Maths::Vector4 _m_Lambda;
	Maths::Vector4 _m_BVector;
	Maths::Vector4 _m_Error;
};



class C_PrismaticJointLimitJacobian : public C_Jacobian
{
public:

	C_PrismaticJointLimitJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_PrismaticJointLimitJacobian()
	{
	}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);

	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);

	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector4 PCREF diagonal, Maths::Vector4& dif,  const Maths::VF32 PCREF totalDif);

	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector4 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector4 PCREF diagonal);


	Maths::Mat46 _m_J1;
	Maths::Mat46 _m_J2;
	Maths::Vector4 _m_Lambda;
	Maths::Vector4 _m_BVector;
	Maths::Vector4 _m_Error;

};

class C_RevoluteJointLimitJacobian : public C_Jacobian
{
public:
	C_RevoluteJointLimitJacobian(): C_Jacobian(0.2f)
	{
	}

	virtual ~C_RevoluteJointLimitJacobian()
	{
	}

	//void CalculateBVector(const F32* __restrict bA1, const F32* __restrict bA2, const F32& slop, const F32& rcpTime);

	void SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint);

	//Maths::VF32 UpdateLambda(const F32* __restrict imjtLam1, const F32* __restrict imjtLam2, const Maths::Vector4 PCREF diagonal, Maths::Vector4& dif,  const Maths::VF32 PCREF totalDif);

	void CalculateBVector(const Maths::Vector6& bA1, const Maths::Vector6& bA2, const F32& slop, const F32& rcpTime);
	Maths::Vector2 UpdateLambda(const Maths::Vector6& imjtLam1, const Maths::Vector6& imjtLam2, const Maths::Vector2 PCREF diagonal);

	Maths::Mat26 _m_J1;
	Maths::Mat26 _m_J2;
	Maths::Vector2 _m_Lambda;
	Maths::Vector2 _m_BVector;
	Maths::Vector2 _m_Error;

};


};

#endif
