
#include "stdafx.h"
#include "gravity/solver2/InvertMJT.h"
#include <system/perf.h>


namespace Solver2
{

	C_InvertMJT::C_InvertMJT(F32* _m_IMJT1, F32* _m_IMJT2, F32* _m_Diagonal, F32* _m_Difference)
	{
		m_Scale = Maths::Const<Maths::VF32>::One();
	}

	void C_ContactInvertMJT::CalculateInvertMJTLambda(Maths::Vector6& imjtLamV1, Maths::Vector6& imjtLamV2)
	{
		//PERF_EVENT("Contact_InvertMJT");
		imjtLamV1 +=(_m_IMJT1*_m_Difference);


		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			imjtLamV2 +=(_m_IMJT2*_m_Difference);
		}
	}

	void C_ContactInvertMJT::CalculateDiagonalVector(Solver2::C_Jacobian* pSpuJacobian)
	{

		using namespace Maths;
		//Spu::MemBaseMarker marker;

		Solver2::C_ContactJacobian* j = (Solver2::C_ContactJacobian*)pSpuJacobian;

		//Solver2::C_ContactJacobian* j=(Solver2::C_ContactJacobian*)Spu::FetchAlloc(m_Jacobian, sizeof(C_ContactJacobian));

		_m_Diagonal = Maths::DiagonalMultiply(j->_m_J1,  _m_IMJT1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			_m_Diagonal += Maths::DiagonalMultiply(j->_m_J2, _m_IMJT2);
		}

		const Vector3 eps=CopySign(MakeVector3(Maths::Const<Vec4F32>::Epsilon()), _m_Diagonal);

		_m_Diagonal = Rcp(_m_Diagonal + eps);

	}


};
