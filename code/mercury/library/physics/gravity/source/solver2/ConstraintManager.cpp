#include "stdafx.h"
#include "gravity/solver2/ConstraintManager.h"
#include "gravity/solver2/ConstraintManager.inl"
#include "gravity/solver2/constraint.h"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/solver2/Jacobian.inl"
#include "system/perf.h"


namespace Solver2
{

	void ConstraintManager::CreateJacobianInvMJT(SharedPhysics::C_Constraint* constraint, Gravity::C_ConstraintBody* spuConstraintBody1,
		Gravity::C_ConstraintBody* spuConstraintBody2)
	{
		Spu::MemBaseMarker marker;
		F32* ppuScale = &constraint->m_Scale;
		U32 intVal = Spu::Get((U32*)ppuScale);
		F32 constraintWeight = *((F32*)(&intVal));
		if(constraintWeight <= 0.f)
			return; //Do nothing!
		U32 constraintType = Spu::Get((U32*)&constraint->m_ConstraintType);
		switch(constraintType)
		{
		case SharedPhysics::C_Constraint::E_Type_FixedRotational:
			{
				ConstructJacobian<C_FixedRotationalConstraint, C_FixedRotationalJacobian, C_FixedRotationalInvertMJT>(constraint, spuConstraintBody1, spuConstraintBody2, m_FixedJointPairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_Distance:
			{
				ConstructJacobian<C_DistanceConstraint, C_DistanceJacobian, C_DistanceInvertMJT>(constraint, spuConstraintBody1, spuConstraintBody2, m_DistancePairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_TwoRotational:
			{
				ConstructJacobian<C_TwoRotationalConstraint, C_TwoRotationalJacobian, C_TwoRotationalInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_TwoRotationalPairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_Spherical:
			{
				ConstructJacobian<C_SphericalConstraint, C_SphericalJointJacobian, C_SphericalJointInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_SphericalPairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_SphericalLimit:
			{
				ConstructJacobian<C_SphericalLimitConstraint, C_SphericalJointLimitJacobian, C_SphericalJointLimitInvertMJT>( constraint,
					spuConstraintBody1,spuConstraintBody2, m_SphericalLimitPairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_Revolute:
			{
				ConstructJacobian<C_RevoluteConstraint, C_RevoluteJointJacobian, C_RevoluteJointInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_RevolutePairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_RevoluteLimit:
			{
				ConstructJacobian<C_RevoluteLimitConstraint, C_RevoluteJointLimitJacobian, C_RevoluteJointLimitInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_RevoluteLimitPairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_Prismatic:
			{
				ConstructJacobian<C_PrismaticConstraint, C_PrismaticJointJacobian, C_PrismaticJointInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_PrismaticPairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_PrismaticLimit:
			{
				ConstructJacobian<C_PrismaticLimitConstraint, C_PrismaticJointLimitJacobian, C_PrismaticJointLimitInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_PrismaticLimitPairs);
				break;
			}
		case SharedPhysics::C_Constraint::E_Type_CarWheel:
			{
				ConstructJacobian<C_CarWheelConstraint, C_CarWheelJointJacobian, C_CarWheelJointInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_CarWheelPairs);
				break;
			}
		
		default: // E_Type_Contact
			ConstructJacobian<C_ContactConstraint, C_ContactJacobian, C_ContactInvertMJT>(constraint,
					spuConstraintBody1, spuConstraintBody2, m_ContactPairs);
			break;
		}	
	}  


	void ConstraintManager::StoreWarmStartValues()
	{
		//PERF_EVENT("Gravity.Solver.StoreWarmStartValues");
		StoreWarmStartValues<C_ContactJacobian,				C_ContactInvertMJT>(m_ContactPairs);
		StoreWarmStartValues<C_SphericalJointJacobian,		C_SphericalJointInvertMJT>(m_SphericalPairs);
		StoreWarmStartValues<C_SphericalJointLimitJacobian, C_SphericalJointLimitInvertMJT>(m_SphericalLimitPairs);

		StoreWarmStartValues<C_RevoluteJointLimitJacobian,	C_RevoluteJointLimitInvertMJT>(m_RevoluteLimitPairs);
		StoreWarmStartValues<C_RevoluteJointJacobian,		C_RevoluteJointInvertMJT>(m_RevolutePairs);
		
		StoreWarmStartValues<C_PrismaticJointJacobian,		C_PrismaticJointInvertMJT>(m_PrismaticPairs);
		StoreWarmStartValues<C_PrismaticJointLimitJacobian, C_PrismaticJointLimitInvertMJT>(m_PrismaticLimitPairs);
		StoreWarmStartValues<C_CarWheelJointJacobian,		C_CarWheelJointInvertMJT>(m_CarWheelPairs);
		StoreWarmStartValues<C_FixedRotationalJacobian,		C_FixedRotationalInvertMJT>(m_FixedJointPairs);
		StoreWarmStartValues<C_DistanceJacobian,			C_DistanceInvertMJT>(m_DistancePairs);
		StoreWarmStartValues<C_TwoRotationalJacobian,		C_TwoRotationalInvertMJT>(m_TwoRotationalPairs);
		StoreWarmStartValues<C_ContactJacobian,				C_ContactInvertMJT>(m_StaticContactPairs);

	}

	void ConstraintManager::BVecWarmDiagonal(SharedPhysics::Array<Maths::Vector6>& aAuxiliary/*, SharedPhysics::Array<Maths::Vector6>& bAuxiliary*/)
	{
		//PERF_EVENT("Gravity.Solver.B_Warm_Diagonal");
		BVecWarmDiagonal<C_ContactJacobian,				C_ContactInvertMJT>(m_ContactPairs, aAuxiliary/*, bAuxiliary*/);
		
		BVecWarmDiagonal<C_SphericalJointLimitJacobian,	C_SphericalJointLimitInvertMJT>(m_SphericalLimitPairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_SphericalJointJacobian,		C_SphericalJointInvertMJT>(m_SphericalPairs, aAuxiliary/*, bAuxiliary*/);  

		BVecWarmDiagonal<C_RevoluteJointLimitJacobian,	C_RevoluteJointLimitInvertMJT>(m_RevoluteLimitPairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_RevoluteJointJacobian,		C_RevoluteJointInvertMJT>(m_RevolutePairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_PrismaticJointJacobian,		C_PrismaticJointInvertMJT>(m_PrismaticPairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_PrismaticJointLimitJacobian,	C_PrismaticJointLimitInvertMJT>(m_PrismaticLimitPairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_CarWheelJointJacobian,		C_CarWheelJointInvertMJT>(m_CarWheelPairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_FixedRotationalJacobian,		C_FixedRotationalInvertMJT>(m_FixedJointPairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_DistanceJacobian,				C_DistanceInvertMJT>(m_DistancePairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_TwoRotationalJacobian,		C_TwoRotationalInvertMJT>(m_TwoRotationalPairs, aAuxiliary/*, bAuxiliary*/);
		BVecWarmDiagonal<C_ContactJacobian,				C_ContactInvertMJT>(m_StaticContactPairs, aAuxiliary/*, bAuxiliary*/);
	}

//#pragma optimize("", off)
	//Maths::VF32 ConstraintManager::UpdateConstraints(F32* aAuxiliary, bool solveJoint)
	Maths::VF32 ConstraintManager::UpdateConstraints(SharedPhysics::Array<Maths::Vector6>& aAuxiliary)
	{
		//PERF_EVENT("gravity.solver.Solve.UpdateConstraints");
		Maths::VF32 totalDif(Maths::Const<Maths::VF32>::Zero());
		
		totalDif = UpdateConstraint<C_ContactJacobian, C_ContactInvertMJT, Maths::Vector3>(m_ContactPairs, aAuxiliary, totalDif);

		//totalDif = UpdateStaticConstraint<C_ContactJacobian, C_ContactInvertMJT, Maths::Vector3>(m_StaticContactPairs, aAuxiliary, totalDif);  
		totalDif = UpdateConstraint<C_ContactJacobian, C_ContactInvertMJT, Maths::Vector3>(m_StaticContactPairs, aAuxiliary, totalDif);  

		//if(solveJoint)
		{
			//PERF_EVENT("gravity.solver.Solve.UpdateJoints");
			totalDif = UpdateConstraint<C_SphericalJointLimitJacobian,	C_SphericalJointLimitInvertMJT, Maths::Vector6>(m_SphericalLimitPairs, aAuxiliary, totalDif);
			totalDif = UpdateConstraint<C_SphericalJointJacobian,		C_SphericalJointInvertMJT, Maths::Vector3>(m_SphericalPairs, aAuxiliary, totalDif);
			totalDif = UpdateConstraint<C_RevoluteJointLimitJacobian,	C_RevoluteJointLimitInvertMJT, Maths::Vector2>(m_RevoluteLimitPairs, aAuxiliary, totalDif); 
			totalDif = UpdateConstraint<C_RevoluteJointJacobian,		C_RevoluteJointInvertMJT, Maths::Vector2>(m_RevolutePairs, aAuxiliary, totalDif);
			//totalDif = UpdateConstraint<C_RevoluteJointLimitJacobian,	C_RevoluteJointLimitInvertMJT>(m_RevoluteLimitPairs, aAuxiliary, totalDif); 
			totalDif = UpdateConstraint<C_PrismaticJointJacobian,		C_PrismaticJointInvertMJT, Maths::Vector2>(m_PrismaticPairs, aAuxiliary, totalDif);
			totalDif = UpdateConstraint<C_PrismaticJointLimitJacobian,	C_PrismaticJointLimitInvertMJT, Maths::Vector4>(m_PrismaticLimitPairs, aAuxiliary, totalDif);
			totalDif = UpdateConstraint<C_CarWheelJointJacobian,		C_CarWheelJointInvertMJT, Maths::Vector4>(m_CarWheelPairs, aAuxiliary, totalDif);
			totalDif = UpdateConstraint<C_FixedRotationalJacobian,		C_FixedRotationalInvertMJT, Maths::Vector3>(m_FixedJointPairs, aAuxiliary, totalDif);
			totalDif = UpdateConstraint<C_DistanceJacobian,				C_DistanceInvertMJT, Maths::Vector3>(m_DistancePairs, aAuxiliary, totalDif);
			totalDif = UpdateConstraint<C_TwoRotationalJacobian,		C_TwoRotationalInvertMJT, Maths::VF32>(m_TwoRotationalPairs, aAuxiliary, totalDif);
			/*totalDif = UpdateConstraint<C_SphericalJointLimitJacobian,	C_SphericalJointLimitInvertMJT, Maths::Vector6>(m_SphericalLimitPairs, aAuxiliary, totalDif);*/
		}


		
		
		return totalDif;
	}
//#pragma optimize("", on)

	Maths::VF32 ConstraintManager::UpdateConstraintsNoJoints(SharedPhysics::Array<Maths::Vector6>& aAuxiliary)
	{
		//PERF_EVENT("gravity.solver.Solve.UpdateConstraintsNoJoints");
		Maths::VF32 totalDif(Maths::Const<Maths::VF32>::Zero());
		
		totalDif = UpdateConstraint<C_ContactJacobian, C_ContactInvertMJT, Maths::Vector3>(m_ContactPairs, aAuxiliary, totalDif);
//		totalDif = UpdateStaticConstraint<C_ContactJacobian, C_ContactInvertMJT, Maths::Vector3>(m_StaticContactPairs, aAuxiliary, totalDif);
		totalDif = UpdateConstraint<C_ContactJacobian, C_ContactInvertMJT, Maths::Vector3>(m_StaticContactPairs, aAuxiliary, totalDif);
		
		return totalDif;
	}

	void ConstraintManager::Reset()
	{
		//m_pStackAllocator->Reset();
		
		m_MemoryPool.Reset();

		m_ContactPairs.RestoreDataBlocks();
		m_StaticContactPairs.RestoreDataBlocks();
		m_SphericalLimitPairs.RestoreDataBlocks();
		m_SphericalPairs.RestoreDataBlocks();
		m_RevolutePairs.RestoreDataBlocks();
		m_RevoluteLimitPairs.RestoreDataBlocks();
		m_PrismaticPairs.RestoreDataBlocks();
		m_PrismaticLimitPairs.RestoreDataBlocks();
		m_CarWheelPairs.RestoreDataBlocks();
		m_FixedJointPairs.RestoreDataBlocks();
		m_DistancePairs.RestoreDataBlocks();
		m_TwoRotationalPairs.RestoreDataBlocks();
	}



};
