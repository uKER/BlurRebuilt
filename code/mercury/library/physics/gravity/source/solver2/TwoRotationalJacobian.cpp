#include "stdafx.h"
#include "gravity/solver2/Jacobian.h"

namespace Solver2
{

	void C_TwoRotationalJacobian::CalculateBVector(const Maths::Vector6& bAV1, const Maths::Vector6& bAV2, const F32& slop, const F32& rcpTime)
	{
		using namespace Maths;

		Spu::MemBaseMarker marker;

		const VF32 baum=MakeVF32(m_Baumgarte/**rcpTime*/);

		const VF32 stabiliser=_m_Error*baum;

		_m_BVector = stabiliser - Dot(_m_J1, bAV1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_BVector -=Dot(_m_J2, bAV2);
		}

	}

	void C_TwoRotationalJacobian::SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint)
	{
		using namespace Maths;
		//Spu::MemBaseMarker marker;
		const VF32 zero(Const<VF32>::Zero());
		const Maths::VF32 wStart(MakeVF32(wStartCoeffient));
		//SharedPhysics::C_Constraint* spuConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		VF32 warmStartValue=MakeVF32(pConstraint->m_WarmStartValues);//, spuConstraint->m_WarmStartValues[1], spuConstraint->m_WarmStartValues[2]);
		const VF32 lambda(warmStartValue * wStart);
		_m_Lambda = Select(Finite(lambda), lambda, zero);
	}

	//Maths::VF32 C_TwoRotationalJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::VF32 PCREF diagonalRecip)
	//{

	//	using namespace Maths;

	//	VF32 difV=_m_BVector - Dot(_m_J1, imjtLamV1);
	//	if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	{
	//		difV -= Dot(_m_J2, imjtLamV2);
	//	}

	//	difV *= diagonalRecip;

	//	//dif = difV;

	//	//const VF32 absDif = Abs(difV);

	//	_m_Lambda +=difV;

	//	//const VF32 newTotDif = totalDif + absDif ;

	//	//return newTotDif;
	//	return difV;
	//}


};