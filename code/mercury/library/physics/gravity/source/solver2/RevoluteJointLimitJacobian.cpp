#include "stdafx.h"
#include "gravity/solver2/Jacobian.h"

namespace Solver2
{ 

	void C_RevoluteJointLimitJacobian::CalculateBVector(const Maths::Vector6& bAV1, const Maths::Vector6& bAV2, const F32& slop, const F32& rcpTime)
	{
		
		using namespace Maths;

		const VF32 baum=MakeVF32(m_Baumgarte/* * rcpTime*/);

		const Vector2 stabiliser=_m_Error*baum;

		_m_BVector = stabiliser - (_m_J1 * bAV1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_BVector -=(_m_J2 * bAV2);
		}
	}

	void C_RevoluteJointLimitJacobian::SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint)
	{
		using namespace Maths;
		//Spu::MemBaseMarker marker;
		const Vector2 zero(Const<Vector2>::Zero());
		const Maths::VF32 wStart(MakeVF32(wStartCoeffient));
		SharedPhysics::C_Constraint* spuConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		Vector2 warmStartValue=zero;//MakeVector2(pConstraint->m_WarmStartValues);//, spuConstraint->m_WarmStartValues[1], spuConstraint->m_WarmStartValues[2]);
		const Vector2 lambda(warmStartValue * wStart);
		_m_Lambda = Select(Finite(lambda), lambda, zero);
	}

	//Maths::Vector2 C_RevoluteJointLimitJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector2 PCREF diagonalRecip)
	//{
	//	
	//	using namespace Maths;

	//	Vector2 difV=_m_BVector -(_m_J1 * imjtLamV1);

	//	//if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
	//	{
	//		difV -=(_m_J2 * imjtLamV2);
	//	}

	//	difV *=diagonalRecip;

	//	const VF32 zero=Const<VF32>::Zero();
	//	const VF32 lam0=Element<0>(_m_Lambda);
	//	const VF32 lam1=Element<1>(_m_Lambda);

	//	const VF32 newLambda0=Max(zero, lam0 + Element<0>(difV));
	//	const VF32 newLambda1=Max(zero, lam1 + Element<1>(difV));


	//	SetElement<0>(&difV, newLambda0-lam0);
	//	SetElement<1>(&difV, newLambda1-lam1);

	//	//dif = difV;

	//	_m_Lambda +=difV;

	//	SetElement<0>(&_m_Lambda, newLambda0);
	//	SetElement<1>(&_m_Lambda, newLambda1);

	//	/*const Vector2 absDif(Abs(difV));
	//	const VF32 newTotalDif = totalDif + Element<0>(absDif) + Element<1>(absDif);

	//	return newTotalDif;*/
	//	return difV;
	//}

};
