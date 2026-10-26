#include "stdafx.h"
#include "gravity/solver2/Jacobian.h"

namespace Solver2
{

	void C_FixedRotationalJacobian::CalculateBVector(const Maths::Vector6& bAV1, const Maths::Vector6& bAV2, const F32& slop, const F32& rcpTime)
	{
		
		using namespace Maths;
		
		const VF32 baum=MakeVF32(m_Baumgarte);
		
		const Vector3 correction(_m_Error * baum);
		
		_m_BVector = correction - (_m_J1 * bAV1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_BVector -=(_m_J2*bAV2); 
		}
	}
	
	void C_FixedRotationalJacobian::SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint)
	{
		
		using namespace Maths;
		//Spu::MemBaseMarker marker;
		const Vector3 zero(Const<Vector3>::Zero());
		const Maths::VF32 wStart(MakeVF32(wStartCoeffient));
		//SharedPhysics::C_Constraint* spuConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		Vector3 warmStartValue=MakeVector3(pConstraint->m_WarmStartValues);//, spuConstraint->m_WarmStartValues[1], spuConstraint->m_WarmStartValues[2]);
		const Vector3 lambda(warmStartValue * wStart);
		_m_Lambda = Select(Finite(lambda), lambda, zero);
	}

	//Maths::Vector3 C_FixedRotationalJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector3 PCREF diagonalRecip)
	//{
	//	using namespace Maths;
	//	
	//	Vector3 difV=_m_BVector - (_m_J1 * imjtLamV1);

	//	if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	{
	//		difV-=(_m_J2 * imjtLamV2);
	//	}

	//	difV *=diagonalRecip;

	//	/*dif = difV;
	//	Vector3 absDif = Maths::Abs(difV);
	//	const VF32 newTotDif = totalDif + Element<0>(absDif) + Element<1>(absDif) + Element<2>(absDif);*/
	//	_m_Lambda +=difV;
	//	return difV;
	//}
}
