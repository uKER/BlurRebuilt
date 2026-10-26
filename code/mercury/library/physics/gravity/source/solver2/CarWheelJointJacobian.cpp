#include "stdafx.h"
#include "gravity/solver2/Jacobian.h"

namespace Solver2
{
	void C_CarWheelJointJacobian::CalculateBVector(const Maths::Vector6& bAV1, const Maths::Vector6& bAV2, const F32& slop, const F32& rcpTime)
	{
		using namespace Maths;

		const VF32 baum=MakeVF32(m_Baumgarte/* * rcpTime*/);

		const Vector4 stabiliser=_m_Error * baum;

		_m_BVector = stabiliser - (_m_J1 * bAV1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_BVector -=(_m_J2 * bAV2);
		}
	}

	void C_CarWheelJointJacobian::SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint)
	{
		using namespace Maths;
		
		const Vector4 zero(Const<Vector4>::Zero());
		const Maths::VF32 wStart(MakeVF32(wStartCoeffient));
		//SharedPhysics::C_Constraint* spuConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		Vector4 warmStartValue=MakeVector4(pConstraint->m_WarmStartValues);//, spuConstraint->m_WarmStartValues[1], spuConstraint->m_WarmStartValues[2]);
		const Vector4 lambda(warmStartValue * wStart);
		_m_Lambda = Select(Finite(lambda), lambda, zero);
	}

	//Maths::Vector4 C_CarWheelJointJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector4 PCREF diagonalRecip)
	//{
	//	using namespace Maths;

	//	Vector4 difV=_m_BVector -(_m_J1 * imjtLamV1);

	//	if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	{
	//		difV -=(_m_J2 * imjtLamV2);
	//	}

	//	difV *=diagonalRecip;

	//	const VF32 zero=Const<VF32>::Zero();
	//	const VF32 lam2=Element<2>(_m_Lambda);
	//	const VF32 lam3=Element<3>(_m_Lambda);

	//	const VF32 newLambda2=Max(zero, lam2 + Element<2>(difV));
	//	const VF32 newLambda3=Max(zero, lam3 + Element<3>(difV));

	//	SetElement<2>(&difV, newLambda2-lam2);
	//	SetElement<3>(&difV, newLambda3-lam3);

	//	//dif = difV;

	//	_m_Lambda +=difV;
	//	SetElement<2>(&_m_Lambda, newLambda2);
	//	SetElement<3>(&_m_Lambda, newLambda3);

	//	//const Vector4 absDif(Abs(difV));
	//	//const VF32 newTotalDif = totalDif + Element<0>(absDif) + Element<1>(absDif)+ Element<2>(absDif) + Element<3>(absDif);

	//	return difV;
	//}
};
