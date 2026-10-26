#include "stdafx.h"
#include "gravity/solver2/InvertMJT.h"
//#include <system/perf.h>

namespace Solver2
{

	void C_SphericalJointLimitInvertMJT::CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2)
	{
		//PERF_EVENT("SphericalJointLimit_InvertMJT");
		imjtLamV1 +=(_m_IMJT1 * _m_Difference);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			imjtLamV2 +=(_m_IMJT2 * _m_Difference);
		}
	}

	void C_SphericalJointLimitInvertMJT::CalculateDiagonalVector(Solver2::C_Jacobian* pSpuJacobian)
	{
		using namespace Maths;
		//Spu::MemBaseMarker marker;

		//Solver2::C_SphericalJointLimitJacobian* j=(Solver2::C_SphericalJointLimitJacobian*)Spu::FetchAlloc(m_Jacobian, sizeof(C_SphericalJointLimitJacobian));

		Solver2::C_SphericalJointLimitJacobian* j=(Solver2::C_SphericalJointLimitJacobian*) pSpuJacobian;
		_m_Diagonal = Maths::DiagonalMultiply(j->_m_J1,  _m_IMJT1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			_m_Diagonal += Maths::DiagonalMultiply(j->_m_J2, _m_IMJT2);
		}

		Vector6 eps = CopySign(MakeVector6(Maths::Const<VF32>::Epsilon()), _m_Diagonal);

		_m_Diagonal = Rcp(_m_Diagonal + eps) * m_Scale;
		
	}

};
