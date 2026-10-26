#include "stdafx.h"
#include "gravity/solver2/InvertMJT.h"
#include <system/perf.h>

namespace Solver2
{ 

	void C_RevoluteJointLimitInvertMJT::CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2)
	{
		//PERF_EVENT("RevolutelJointLimit_InvertMJT");
		imjtLamV1 +=_m_IMJT1*_m_Difference;
		
		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			imjtLamV2 +=_m_IMJT2*_m_Difference;
		}
	}

	void C_RevoluteJointLimitInvertMJT::CalculateDiagonalVector(Solver2::C_Jacobian* pSpuJacobian)
	{

		using namespace Maths;

		//Spu::MemBaseMarker marker;

		//Solver2::C_RevoluteJointLimitJacobian* jaco=(Solver2::C_RevoluteJointLimitJacobian*)Spu::FetchAlloc(m_Jacobian, sizeof(C_RevoluteJointLimitJacobian));
	
		Solver2::C_RevoluteJointLimitJacobian* jaco=(Solver2::C_RevoluteJointLimitJacobian*)pSpuJacobian;
		_m_Diagonal = DiagonalMultiply(jaco->_m_J1, _m_IMJT1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			_m_Diagonal += DiagonalMultiply(jaco->_m_J2, _m_IMJT2);
		}

		const Vector2 eps=CopySign(MakeVector2(Maths::Const<VF32>::Epsilon()), _m_Diagonal);

		_m_Diagonal = Rcp24(_m_Diagonal + eps) * m_Scale;
	}
};
