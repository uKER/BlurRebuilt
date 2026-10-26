#include "stdafx.h"
#include "gravity/solver2/InvertMJT.h"

namespace Solver2
{
	void C_DistanceInvertMJT::CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2)
	{
	
		imjtLamV1 +=(_m_IMJT1 * _m_Difference);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			imjtLamV2 +=(_m_IMJT2 * _m_Difference);
		}
	}

	void C_DistanceInvertMJT::CalculateDiagonalVector(Solver2::C_Jacobian* pSpuJacobian)
	{
		
		using namespace Maths;

		//Spu::MemBaseMarker marker;

		//Solver2::C_SphericalJointJacobian* j=(Solver2::C_SphericalJointJacobian*)Spu::FetchAlloc(m_Jacobian, sizeof(C_SphericalJointJacobian));
		Solver2::C_DistanceJacobian* j = (Solver2::C_DistanceJacobian*)pSpuJacobian;

		_m_Diagonal = Maths::DiagonalMultiply(j->_m_J1,  _m_IMJT1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			_m_Diagonal += Maths::DiagonalMultiply(j->_m_J2, _m_IMJT2);
		}

		Vector3 eps = CopySign(MakeVector3(Maths::Const<Vec4F32>::Epsilon()), _m_Diagonal);
		
		_m_Diagonal = Rcp(_m_Diagonal + eps) * m_Scale;//Select(_m_Diagonal < eps, big, Rcp(_m_Diagonal)) * m_Scale;
	}
};

