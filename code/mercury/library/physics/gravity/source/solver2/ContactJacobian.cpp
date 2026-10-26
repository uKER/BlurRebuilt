#include "stdafx.h"
#include "gravity/solver2/Jacobian.h"


namespace Solver2
{

	void C_ContactJacobian::CalculateBVector(const Maths::Vector6& bAV1, const Maths::Vector6& bAV2, const F32& slop, const F32& rcpTime)
	{
		//b=-J(v+time*Invert(M)*f)=-J*bAuxiliary, which is the formula to caculate b. However, if we want to has error correction, we are using
		//the Baumgarte scheme to push the bodies apart when they overlap. v+ax=0(v=-ax);//v is velocity, x is center of mass, a is a tunable parameter
		//and governs the speed of penetration resolution. No position stabilization is necessary for friction constraints. Therefore, like coefficient
		//of restitution, which just apply in the first row(the contact normal direction), the baumgarte parameter just applys to the first row

		//cofficient of resititution should be just applied to the first row of the jacobian, the reason for that is because friction should not 
		//has bounceness


		using namespace Maths;
	
		const VF32 zero(Maths::Const<VF32>::Zero());
		const VF32 baumgarteVF=MakeVF32(m_Baumgarte);
		const VF32 slopVF=MakeVF32(slop);
		/*const VF32 cOfR(MakeVF32(m_CofRSetting));  */
		const VF32 cOfR(W(m_CoefficientSetting));
		VF32 _pen(_m_Error);

		const VF32 pen = Maths::Select(_pen < slopVF, zero, _pen );

		/*_m_BVector =-(_m_J1*bAV1);

		if(!TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Sleeping_Or_Static))
		{
			_m_BVector -=(_m_J2*bAV2);
		}

		const VF32 value(pen*m_Baumgarte + cOfR*X(_m_BVector));
		SetX(&_m_BVector, value);*/

		const Vector3 bVec = ((-_m_J1*bAV1) - (_m_J2*bAV2));

		const VF32 value(pen*m_Baumgarte + cOfR*X(bVec));
		_m_BVector = MakeVector3(value, Y(bVec), Z(bVec));
	}



	void  C_ContactJacobian::SetLambda(const F32& wStartCoeffient, SharedPhysics::C_Constraint* pConstraint)
	{
		using namespace Maths;
		const Vector3 zero(Const<Vector3>::Zero());
		const Maths::VF32 wStart(MakeVF32(wStartCoeffient));
		//SharedPhysics::C_Constraint* spuConstraint = (SharedPhysics::C_Constraint*)Spu::FetchAlloc(m_pConstraint, sizeof(SharedPhysics::C_Constraint));
		const Vector3 warmStartValue=MakeVector3(pConstraint->m_WarmStartValues);//, spuConstraint->m_WarmStartValues[1], spuConstraint->m_WarmStartValues[2]);
		const Vector3 lambda(warmStartValue * wStart);
		_m_Lambda = Select(Finite(lambda), lambda, zero);

	}

	//Maths::Vector3 C_ContactJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector3 PCREF diagonalRecip)
	//{
	//	
	//	using namespace Maths;
	//
	//	//const VF32 coefficient = MakeVF32(m_Coefficient);

	//	const VF32 coefficient(X(m_CoefficientSetting));
	//	const VF32 zero = Maths::Const<VF32>::Zero();
	//	const VF32 max(Maths::Const<VF32>::Max());

	//	const Vector3 vec1(_m_J1*imjtLamV1);
	//	
	//	/*Vector3 difV(_m_BVector - vec1);

	//	if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	{
	//		difV -=(_m_J2*imjtLamV2);
	//	}

	//	difV*=diagonalRecip;*/

	//	Vector3 difV(((_m_BVector - vec1) - ((_m_J2*imjtLamV2))) * diagonalRecip);

	//	/*if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
	//	{
	//		difV -=(_m_J2*imjtLamV2);
	//	}

	//	difV*=diagonalRecip;*/

	//	const Vector3 lam(_m_Lambda);

	//	const Vector3 unclampedLambda = lam + difV;

	//	const VF32 upper = coefficient * Maths::Max(X(lam), zero);

	//	const Vector3 lower = MakeVector3(zero, -upper, -upper);
	//	const Vector3 higher = MakeVector3(max, upper, upper);

	//	const Vector3 newLambda = Maths::Max(lower, Maths::Min(unclampedLambda, higher));
	//	const Vector3 newDif=newLambda - lam;
	//	//const Vector3 absDif(Abs(newDif));

	//	/*const Maths::VF32 totDif = totalDif + X(absDif) + Y(absDif) + Z(absDif);
	//	dif = newDif;*/
	//	_m_Lambda=newLambda;
	//	return newDif;

	//}


};
