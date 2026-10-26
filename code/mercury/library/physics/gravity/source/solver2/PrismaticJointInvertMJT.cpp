#include "stdafx.h"
#include "gravity/solver2/InvertMJT.h"

namespace Solver2
{
	void C_PrismaticJointInvertMJT::CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2)
	{
		imjtLamV1 +=_m_IMJT1*_m_Difference;
		

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			imjtLamV2 +=_m_IMJT2*_m_Difference;
		}
	}

	void C_PrismaticJointInvertMJT::CalculateDiagonalVector(Solver2::C_Jacobian* pSpuJacobian)
	{
		using namespace Maths;

		//Spu::MemBaseMarker marker;

		//Solver2::C_PrismaticJointJacobian* jaco=(Solver2::C_PrismaticJointJacobian*)Spu::FetchAlloc(m_Jacobian, sizeof(C_PrismaticJointJacobian));
	
		Solver2::C_PrismaticJointJacobian* jaco = (Solver2::C_PrismaticJointJacobian*) pSpuJacobian;
		_m_Diagonal = DiagonalMultiply(jaco->_m_J1, _m_IMJT1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_Diagonal += DiagonalMultiply(jaco->_m_J2, _m_IMJT2);
		}

		const Vector2 eps=CopySign(MakeVector2(Const<Vec4F32>::Epsilon()), _m_Diagonal);

		_m_Diagonal = Rcp(_m_Diagonal + eps) * m_Scale;
	}
};
