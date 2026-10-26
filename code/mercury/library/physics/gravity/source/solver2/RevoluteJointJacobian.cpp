#include "stdafx.h"
#include "gravity/solver2/Jacobian.h"

namespace Solver2
{

	void C_RevoluteJointJacobian::CalculateBVector(const Maths::Vector6& bAV1, const Maths::Vector6& bAV2, const F32& slop, const F32& rcpTime)
	{
		using namespace Maths;

		const VF32 baum=MakeVF32(m_Baumgarte/**rcpTime*/);
		
		const Vector2 stabiliser=_m_Error*baum;

		_m_BVector = stabiliser - (_m_J1 * bAV1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_BVector -=(_m_J2 * bAV2);
		}
	}

	void C_RevoluteJointJacobian::SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint)
	{
		using namespace Maths;
		//Spu::MemBaseMarker marker;
		const Vector2 zero(Const<Vector2>::Zero());
		const Maths::VF32 wStart(MakeVF32(wStartCoeffient));
		//SharedPhysics::C_Constraint* spuConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		Vector2 warmStartValue=MakeVector2(pConstraint->m_WarmStartValues);//, spuConstraint->m_WarmStartValues[1], spuConstraint->m_WarmStartValues[2]);
		const Vector2 lambda(warmStartValue * wStart);
		_m_Lambda = Select(Finite(lambda), lambda, zero);
	}

	
	//Maths::Vector2 C_RevoluteJointJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector2 PCREF diagonalRecip)
	//{
	//	using namespace Maths;
	//	
	//	Vector2 difV=_m_BVector - (_m_J1 * imjtLamV1);

	//	if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	{
	//		difV -= (_m_J2 * imjtLamV2);
	//	}

	//	difV *= diagonalRecip;

	//	//dif = difV;

	//	//const Vector2 absDif = Abs(difV);

	//	_m_Lambda +=difV;

	//	//const VF32 newTotDif = totalDif + Element<0>(absDif) + Element<1>(absDif);

	//	//return newTotDif;
	//	return difV;
	//}
};
