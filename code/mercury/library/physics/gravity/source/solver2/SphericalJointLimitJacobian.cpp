#include "stdafx.h"
#include "gravity/solver2/Jacobian.h"
#include "gravity/sharedphysics/RigidBody.h"

namespace Solver2
{
	void C_SphericalJointLimitJacobian::CalculateBVector( const Maths::Vector6& bAV1, const Maths::Vector6& bAV2, const F32& slop, const F32& rcpTime)
	{
		
		using namespace Maths;
		const VF32 baum=MakeVF32(m_Baumgarte/* * rcpTime*/);
		const Vector6 correction(_m_Error * baum);
		_m_BVector = correction - (_m_J1 * bAV1);
		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_BVector -=(_m_J2*bAV2); 
		}
	}

	
	void C_SphericalJointLimitJacobian::SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint)
	{
		
		using namespace Maths;
		//Spu::MemBaseMarker marker;
		const Vector6 zero(Const<Vector6>::Zero());
		const Maths::VF32 wStart(MakeVF32(wStartCoeffient));
		//SharedPhysics::C_Constraint* spuConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		Vector6 warmStartValue=MakeVector6(pConstraint->m_WarmStartValues);//, spuConstraint->m_WarmStartValues[1], spuConstraint->m_WarmStartValues[2]);
		const Vector6 lambda(warmStartValue * wStart);
		_m_Lambda = Select(Finite(lambda), lambda, zero);

	}


	//Maths::Vector6 C_SphericalJointLimitJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector6 PCREF diagonalRecip)
	//{

	//	using namespace Maths;

	//	Vector6 difV=_m_BVector -(_m_J1 * imjtLamV1);

	//	//if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
	//	{
	//		difV -=(_m_J2 * imjtLamV2);
	//	}

	//	difV *=diagonalRecip;

	//	const Vector6 lam(_m_Lambda);
	//	const Vector6 temp(lam + difV);
	//	const Vector6 zero=Const<Vector6>::Zero();

	//	const Vector6 newLambda = Max(zero, temp);

	//	difV = newLambda - lam;
	//	_m_Lambda = newLambda;

	//	return difV;
	//

	//	/*const Vector6 absDif(Abs(difV));
	//	const VF32 newTotalDif = totalDif + Element<0>(absDif) + Element<1>(absDif)
	//		+ Element<2>(absDif) + Element<3>(absDif) + Element<4>(absDif) + Element<5>(absDif);

	//	return newTotalDif;*/
	//}  

};

