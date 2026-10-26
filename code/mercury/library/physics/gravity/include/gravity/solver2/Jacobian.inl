#include "gravity/solver2/Jacobian.h"


namespace Solver2
{

	__forceinline Maths::Vector3 C_ContactJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector3 PCREF diagonalRecip)
	{
		
		using namespace Maths;
	
		//const VF32 coefficient = MakeVF32(m_Coefficient);

		const VF32 zero(Maths::Const<VF32>::Zero());
		const VF32 max(Maths::Const<VF32>::Max());
		const VF32 frictionU(X(m_CoefficientSetting));
		const VF32 frictionV(Y(m_CoefficientSetting));

		

		const Vector3 vec1(_m_J1*imjtLamV1);

		const Vector3 difV(((_m_BVector - vec1) - ((_m_J2*imjtLamV2))) * diagonalRecip);

		//BIZ_ASSERT(Maths::Finite(difV));

		const Vector3 lam(_m_Lambda);

		const Vector3 unclampedLambda = lam + difV;
		const VF32 clampedLambda(Maths::Max(X(lam), zero));

		const VF32 upper0(frictionU * clampedLambda);
		const VF32 upper1(frictionV * clampedLambda);

		const Vector3 lower = MakeVector3(zero, -upper0, -upper1);
		const Vector3 higher = MakeVector3(max, upper0, upper1);

		const Vector3 newLambda = Maths::Max(lower, Maths::Min(unclampedLambda, higher));
		const Vector3 newDif=newLambda - lam;

		_m_Lambda=newLambda;
		return newDif;

	}

	__forceinline Maths::Vector3 C_ContactJacobian::UpdateStaticLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector3 PCREF diagonalRecip)
	{
		
		using namespace Maths;

		const VF32 frictionU(X(m_CoefficientSetting));
		const VF32 frictionV(Y(m_CoefficientSetting));

		const VF32 zero = Maths::Const<VF32>::Zero();
		const VF32 max(Maths::Const<VF32>::Max());

		const Vector3 vec1(_m_J1*imjtLamV1);

		const Vector3 difV((_m_BVector - vec1) * diagonalRecip);

		//BIZ_ASSERT(Maths::Finite(difV));

		const Vector3 lam(_m_Lambda);

		const Vector3 unclampedLambda = lam + difV;

		const VF32 clampedLambda(Maths::Max(X(lam), zero));

		const VF32 upper0(frictionU * clampedLambda);
		const VF32 upper1(frictionV * clampedLambda);

		const Vector3 lower = MakeVector3(zero, -upper0, -upper1);
		const Vector3 higher = MakeVector3(max, upper0, upper1);

		const Vector3 newLambda = Maths::Max(lower, Maths::Min(unclampedLambda, higher));
		const Vector3 newDif=newLambda - lam;

		_m_Lambda=newLambda;
		return newDif;

	}


	__forceinline Maths::Vector3 C_SphericalJointJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector3 PCREF diagonalRecip)
	{
		using namespace Maths;
		const Vector3 difV=((_m_BVector - (_m_J1 * imjtLamV1) - (_m_J2 * imjtLamV2)) * diagonalRecip);

		//BIZ_ASSERT(Maths::Finite(difV));
		_m_Lambda +=difV;
		return difV;
	}

	__forceinline Maths::Vector4 C_CarWheelJointJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector4 PCREF diagonalRecip)
	{
		using namespace Maths;

		Vector4 difV=_m_BVector -(_m_J1 * imjtLamV1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			difV -=(_m_J2 * imjtLamV2);
		}

		difV *=diagonalRecip;

		//BIZ_ASSERT(Maths::Finite(difV));

		const VF32 zero=Const<VF32>::Zero();
		const VF32 lam2=Element<2>(_m_Lambda);
		const VF32 lam3=Element<3>(_m_Lambda);

		const VF32 newLambda2=Max(zero, lam2 + Element<2>(difV));
		const VF32 newLambda3=Max(zero, lam3 + Element<3>(difV));

		SetElement<2>(&difV, newLambda2-lam2);
		SetElement<3>(&difV, newLambda3-lam3);

		//dif = difV;

		_m_Lambda +=difV;
		SetElement<2>(&_m_Lambda, newLambda2);
		SetElement<3>(&_m_Lambda, newLambda3);

		//const Vector4 absDif(Abs(difV));
		//const VF32 newTotalDif = totalDif + Element<0>(absDif) + Element<1>(absDif)+ Element<2>(absDif) + Element<3>(absDif);

		return difV;
	}

	__forceinline Maths::Vector3 C_DistanceJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector3 PCREF diagonalRecip)
	{
		
		using namespace Maths;
	
		/*Vector3 difV=_m_BVector - (_m_J1 * imjtLamV1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			difV-=(_m_J2 * imjtLamV2);
		}

		difV *=diagonalRecip;*/

		const Vector3 difV=((_m_BVector - (_m_J1 * imjtLamV1) - (_m_J2 * imjtLamV2)) * diagonalRecip);

		//BIZ_ASSERT(Maths::Finite(difV));

		//dif = difV;
		//Vector3 absDif = Abs(difV);
		//const VF32 newTotDif = totalDif + X(absDif) + Y(absDif) + Z(absDif);
		_m_Lambda +=difV;
		//return newTotDif;
		return difV;
	}

	__forceinline Maths::Vector3 C_FixedRotationalJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector3 PCREF diagonalRecip)
	{
		using namespace Maths;
		
		/*Vector3 difV=_m_BVector - (_m_J1 * imjtLamV1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			difV-=(_m_J2 * imjtLamV2);
		}

		difV *=diagonalRecip;*/

		const Vector3 difV=((_m_BVector - (_m_J1 * imjtLamV1) - (_m_J2 * imjtLamV2)) * diagonalRecip);

		//BIZ_ASSERT(Maths::Finite(difV));

		/*dif = difV;
		Vector3 absDif = Maths::Abs(difV);
		const VF32 newTotDif = totalDif + Element<0>(absDif) + Element<1>(absDif) + Element<2>(absDif);*/
		_m_Lambda +=difV;
		return difV;
	}

	__forceinline Maths::Vector2 C_PrismaticJointJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector2 PCREF diagonalRecip)
	{
		using namespace Maths;
		
		Vector2 difV=_m_BVector - (_m_J1 * imjtLamV1);
		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			difV -= (_m_J2 * imjtLamV2);
		}

		difV *= diagonalRecip;

		//BIZ_ASSERT(Maths::Finite(difV));

		//dif = difV;

		_m_Lambda +=difV;

		/*const Vector2 absDif = Maths::Abs(difV);
		const VF32 newTotDif = totalDif + Element<0>(absDif) + Element<1>(absDif);
		return newTotDif;*/
		return difV;
	}

	__forceinline Maths::Vector4 C_PrismaticJointLimitJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector4 PCREF diagonalRecip)
	{
		using namespace Maths;

		Vector4 difV=_m_BVector -(_m_J1 * imjtLamV1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			difV -=(_m_J2 * imjtLamV2);
		}

		difV *=diagonalRecip;

		//BIZ_ASSERT(Maths::Finite(difV));

		const VF32 zero=Const<VF32>::Zero();
		const VF32 lam2=Element<2>(_m_Lambda);
		const VF32 lam3=Element<3>(_m_Lambda);

		const VF32 newLambda2=Max(zero, lam2 + Element<2>(difV));
		const VF32 newLambda3=Max(zero, lam3 + Element<3>(difV));

		SetElement<2>(&difV, newLambda2-lam2);
		SetElement<3>(&difV, newLambda3-lam3);

		//dif = difV;

		_m_Lambda +=difV;
		SetElement<2>(&_m_Lambda, newLambda2);
		SetElement<3>(&_m_Lambda, newLambda3);

		/*const Vector4 absDif(Abs(difV));
		const VF32 newTotalDif = totalDif + Element<0>(absDif) + Element<1>(absDif)+ Element<2>(absDif) + Element<3>(absDif);

		return newTotalDif;*/
		return difV;
	}

	__forceinline Maths::Vector2 C_RevoluteJointJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector2 PCREF diagonalRecip)
	{
		using namespace Maths;

		const Vector2 difV=((_m_BVector - (_m_J1 * imjtLamV1) - (_m_J2 * imjtLamV2)) * diagonalRecip);

		//BIZ_ASSERT(Maths::Finite(difV));

		_m_Lambda +=difV;

		return difV;
	}

	__forceinline Maths::Vector2 C_RevoluteJointLimitJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector2 PCREF diagonalRecip)
	{
		using namespace Maths;

		const Vector2 difV=(_m_BVector -(_m_J1 * imjtLamV1) - (_m_J2 * imjtLamV2)) * diagonalRecip;

		const Vector2 zero=Const<Vector2>::Zero();

		const Vector2 newLambda=Max(zero, _m_Lambda + difV);

		const Vector2 finalDif(newLambda - _m_Lambda);
		
		_m_Lambda = newLambda;

		return finalDif;
	}

	__forceinline Maths::Vector6 C_SphericalJointLimitJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::Vector6 PCREF diagonalRecip)
	{

		using namespace Maths;

		/*Vector6 difV=_m_BVector -(_m_J1 * imjtLamV1);

		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			difV -=(_m_J2 * imjtLamV2);
		}

		difV *=diagonalRecip;*/

		const Vector6 zero(Const<Vector6>::Zero());
		const Vector6 difV=((_m_BVector - (_m_J1 * imjtLamV1) - (_m_J2 * imjtLamV2)) * diagonalRecip);
		//BIZ_ASSERT(Maths::Finite(difV));

		
		const Vector6 tempNewLambda(_m_Lambda + difV);
		//const Vector6 newLambda(Maths::Max(tempNewLambda, zero));
		const Vector6 newLambda(Maths::Select(tempNewLambda > zero, tempNewLambda, zero));
		const Vector6 resultDif = newLambda - _m_Lambda;
		
		_m_Lambda = newLambda; //Kier - This has been commented out intentionally until we can get to the bottom of a bug!

		return resultDif;

	}
 
	__forceinline Maths::VF32 C_TwoRotationalJacobian::UpdateLambda(const Maths::Vector6& imjtLamV1, const Maths::Vector6& imjtLamV2, const Maths::VF32 PCREF diagonalRecip)
	{

		using namespace Maths;

		VF32 difV=_m_BVector - Dot(_m_J1, imjtLamV1);
		if(TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Active))
		{
			difV -= Dot(_m_J2, imjtLamV2);
		}

		difV *= diagonalRecip;

		//BIZ_ASSERT(Maths::Finite(difV));

		_m_Lambda +=difV;

		return difV;
	}

}
