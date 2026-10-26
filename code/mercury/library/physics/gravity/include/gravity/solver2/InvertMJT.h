#ifndef SOLVER2_INVERTMJT_H
#define SOLVER2_INVERTMJT_H

//#include "core/maths/hvec/new/includeMinimal.h"
#include "gravity/solver2/Jacobian.h"


namespace Solver2
{
class ALIGN16 C_InvertMJT
{
public:
	C_InvertMJT(F32* _m_IMJT1, F32* _m_IMJT2, F32* _m_Diagonal, F32* _m_Difference);

	virtual ~C_InvertMJT()
	{
	}

	/*void SetJacobian(C_Jacobian* _jacobian)
	{
		m_Jacobian=_jacobian;
	}*/

	void SetScale(const Maths::VF32 PCREF scale)
	{
		m_Scale = scale;
	}

	bool TestNodeFlags(const U32& nodeFlagB) const
	{
		return (m_NodeBFlag & nodeFlagB) != 0;
	}


	C_InvertMJT* m_pPPUPtr;

	//C_Jacobian* m_Jacobian;
	/*SharedPhysics::C_GraphNode* m_pRigid1;
	SharedPhysics::C_GraphNode* m_pRigid2; */

	Gravity::C_ConstraintBody* m_pSpuConstraintBody1;
	Gravity::C_ConstraintBody* m_pSpuConstraintBody2;
	U32 m_NodeBFlag;
	Maths::VF32 m_Scale;
};


class C_ContactInvertMJT : public C_InvertMJT
{
public:

	C_ContactInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}
	virtual ~C_ContactInvertMJT()
	{
	}

	void CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2);
	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);

	Maths::Mat63 _m_IMJT1;
	Maths::Mat63 _m_IMJT2;
	Maths::Vector3 _m_Diagonal;//A=J*Invert(M)*Transpose(J); 3x3
	Maths::Vector3 _m_Difference; // bVector-A*Lamba; //bVector =A*Lambda 
};

class C_FixedRotationalInvertMJT : public C_InvertMJT
{
public:
	C_FixedRotationalInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_FixedRotationalInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);

	void CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2);
	void CalculateDiagonalVector(C_Jacobian* jaco);

	Maths::Mat63 _m_IMJT1;
	Maths::Mat63 _m_IMJT2;
	Maths::Vector3 _m_Diagonal;//A=J*Invert(M)*Transpose(J); 6x6
	Maths::Vector3 _m_Difference;

};


class C_DistanceInvertMJT : public C_InvertMJT
{
public:

	C_DistanceInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_DistanceInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2);
	void CalculateDiagonalVector(C_Jacobian* jaco);

	Maths::Mat63 _m_IMJT1;
	Maths::Mat63 _m_IMJT2;
	Maths::Vector3 _m_Diagonal;
	Maths::Vector3 _m_Difference;
};

class C_SphericalJointInvertMJT : public C_InvertMJT
{
public:

	C_SphericalJointInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_SphericalJointInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);

	Maths::Mat63 _m_IMJT1;
	Maths::Mat63 _m_IMJT2;
	Maths::Vector3 _m_Diagonal;
	Maths::Vector3 _m_Difference;
};

class C_SphericalJointLimitInvertMJT : public C_InvertMJT
{
public:
	C_SphericalJointLimitInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_SphericalJointLimitInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);

	Maths::Mat66 _m_IMJT1;
	Maths::Mat66 _m_IMJT2;
	Maths::Vector6 _m_Diagonal;
	Maths::Vector6 _m_Difference;

};

class C_RevoluteJointInvertMJT : public C_InvertMJT
{
public:


	C_RevoluteJointInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_RevoluteJointInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);


	Maths::Mat62 _m_IMJT1;
	Maths::Mat62 _m_IMJT2;
	Maths::Vector2 _m_Diagonal; //A=J*Invert(M)*Transpose(J); 5X5
	Maths::Vector2 _m_Difference;
	
};


class C_PrismaticJointInvertMJT : public C_InvertMJT
{
public:

	C_PrismaticJointInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_PrismaticJointInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);

	Maths::Mat62 _m_IMJT1;
	Maths::Mat62 _m_IMJT2;
	Maths::Vector2 _m_Diagonal; //A=J*Invert(M)*Transpose(J); 3x3
	Maths::Vector2 _m_Difference;

};


class C_CarWheelJointInvertMJT : public C_InvertMJT
{
public:

	C_CarWheelJointInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_CarWheelJointInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(Solver2::C_Jacobian* pSpuJacobian);

	
	Maths::Mat64 _m_IMJT1;
	Maths::Mat64 _m_IMJT2;
	Maths::Vector4 _m_Diagonal; //A=J*Invert(M)*Transpose(J); 3x3
	Maths::Vector4 _m_Difference;
};

class C_PrismaticJointLimitInvertMJT : public C_InvertMJT
{
public:

	C_PrismaticJointLimitInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_PrismaticJointLimitInvertMJT()
	{
	}
	 
	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);


	Maths::Mat64 _m_IMJT1;
	Maths::Mat64 _m_IMJT2;
	Maths::Vector4 _m_Diagonal; //A=J*Invert(M)*Transpose(J); 3x3
	Maths::Vector4 _m_Difference;
};

class C_RevoluteJointLimitInvertMJT : public C_InvertMJT
{
public:
	C_RevoluteJointLimitInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_RevoluteJointLimitInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);

	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);


	Maths::Mat62 _m_IMJT1;
	Maths::Mat62 _m_IMJT2;
	Maths::Vector2 _m_Diagonal; //A=J*Invert(M)*Transpose(J); 3x3
	Maths::Vector2 _m_Difference;
};


class C_TwoRotationalInvertMJT : public C_InvertMJT
{
public:

	C_TwoRotationalInvertMJT(): C_InvertMJT((F32*)(&_m_IMJT1), (F32*)(&_m_IMJT2), (F32*)(&_m_Diagonal), (F32*)(&_m_Difference))
	{
	}

	virtual ~C_TwoRotationalInvertMJT()
	{
	}

	//void CalculateInvertMJTLambda(F32* imjtLam1, F32* imjtLam2);
	void CalculateInvertMJTLambda(Maths::Vector6& imjtLam1, Maths::Vector6& imjtLam2);
	void CalculateDiagonalVector(C_Jacobian* jaco);

	Maths::Vector6 _m_IMJT1;
	Maths::Vector6 _m_IMJT2;
	Maths::VF32 _m_Diagonal;//A=J*Invert(M)*Transpose(J); 
	Maths::VF32 _m_Difference;
};

	
};

#endif
