#include "stdafx.h"
#include "gravity/solver2/InvertMJT.h"
#include <system/perf.h>

namespace Solver2
{
	void C_SphericalJointInvertMJT::CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2)
	{
		//PERF_EVENT("SphericalJoint_InvertMJT");
		imjtLamV1 +=(_m_IMJT1 * _m_Difference);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			imjtLamV2 +=(_m_IMJT2 * _m_Difference);
		}
	}

	void C_SphericalJointInvertMJT::CalculateDiagonalVector(Solver2::C_Jacobian* pSpuJacobian)
	{
		using namespace Maths;

		//Spu::MemBaseMarker marker;

		//Solver2::C_SphericalJointJacobian* j=(Solver2::C_SphericalJointJacobian*)Spu::FetchAlloc(m_Jacobian, sizeof(C_SphericalJointJacobian));

		Solver2::C_SphericalJointJacobian* j=(Solver2::C_SphericalJointJacobian*) pSpuJacobian;
		/*_m_Diagonal = Maths::DiagonalMultiply(j->_m_J1,  _m_IMJT1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			_m_Diagonal += Maths::DiagonalMultiply(j->_m_J2, _m_IMJT2);
		}

		Vector3 eps = CopySign(MakeVector3(Maths::Const<Vec4F32>::Epsilon()), _m_Diagonal);
		_m_Diagonal = Rcp(_m_Diagonal + eps) * m_Scale;*/

		const Maths::Vector3 diagonal = Maths::DiagonalMultiply(j->_m_J1,  _m_IMJT1) + Maths::DiagonalMultiply(j->_m_J2, _m_IMJT2);

		const Vector3 eps = CopySign(MakeVector3(Maths::Const<VF32>::Epsilon()), diagonal);
		_m_Diagonal = Rcp(diagonal + eps) * m_Scale;
	}
};
