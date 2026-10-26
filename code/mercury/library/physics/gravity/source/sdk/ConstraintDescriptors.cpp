#include "Stdafx.h"
#include "gravity/solver2/Constraint.h"
#include "gravity/solver2/ConstraintGraphManager.h"
//#include "gravity/solver2/RevoluteJointLimitDefinition.h"

#include "gravity/sdk/Body.h"
#include "gravity/sdk/scene.h"
#include "gravity/sdk/ConstraintDescriptors.h"

#include "gravity/sharedphysics/AllocationManager.inl"
#include "gravity/sharedphysics/SortedArray.inl"
#include "gravity/sharedphysics/Spring.h"




namespace Gravity
{
	/*
		SphericalJointDescriptor
	*/
#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* SphericalJointDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		Solver2::C_SphericalConstraint* sphericalConstraint=manager.m_SphericalConstraints.GetNextFreeItem();
		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_GraphNode* pNodeA(pBodyA->GetGraphNode());
		SharedPhysics::C_GraphNode* pNodeB(pBodyB->GetGraphNode());

		if(pEdge->m_pInboundNode == pNodeA)
		{
			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint2;
			sphericalConstraint->pNodeA = pNodeA;
			sphericalConstraint->pNodeB = pNodeB;
		}
		else
		{
			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint2;
			sphericalConstraint->pNodeB = pNodeA;
			sphericalConstraint->pNodeA = pNodeB;
		}
		sphericalConstraint->SetBaumgarte(m_Baumgarte);
		sphericalConstraint->m_BreakErrorSq = m_BreakError;
		pEdge->AddConstraintPoint(pPoint);
		pPoint->m_pConstraint = sphericalConstraint;

		return sphericalConstraint;

	}
#endif

	
	void SphericalJointDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1 = anchor1;
		m_AnchorPoint2 = anchor2;
	}
	/*
		SphericalJointLimitDescriptor
	*/
#ifndef GRAVITY_EXPORTER
	SharedPhysics::C_Constraint* SphericalJointLimitDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		//const Maths::VF32 zero(Maths::Const<Maths::VF32>::Zero());
		//Solver2::C_SphericalLimitProxyConstraint* pProxy = manager.m_SphericalLimitProxyConstraints.GetNextFreeItem();
		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_SphericalLimitConstraint* limitConstraint=manager.m_SphericalLimitConstraints.GetNextFreeItem();
		Solver2::C_SphericalConstraint* spherical=manager.m_SphericalConstraints.GetNextFreeItem();

		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint2(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		
		const Maths::HVec3 posA(pBodyA->Position());
		const Maths::HVec3 posB(pBodyB->Position());

	/*	const Maths::Quaternion rot1(pBodyA->GetBody().m_Orientation);
		const Maths::Quaternion rot2(pBodyB->GetBody().m_Orientation);*/

		const Maths::Mat33 rot1(pBodyA->GetBody().m_OrientationMatrix);
		const Maths::Mat33 rot2(pBodyB->GetBody().m_OrientationMatrix);

		SharedPhysics::C_GraphNode* pNodeA = pBodyA->GetGraphNode();
		SharedPhysics::C_GraphNode* pNodeB = pBodyB->GetGraphNode();
		
		if(pEdge->m_pInboundNode == pNodeA)
		{

			limitConstraint->m_rotationalOffSet = rot2 * Inverse(rot1);//rot1 * Inverse(rot2); 
			limitConstraint->pNodeA = pNodeA;
			limitConstraint->pNodeB = pNodeB;

			spherical->m_AnchorPoint1 = m_AnchorPoint1;
			spherical->m_AnchorPoint2 = m_AnchorPoint2;
			spherical->pNodeA = pNodeA;
			spherical->pNodeB = pNodeB;

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;
		}
		else
		{
			limitConstraint->m_rotationalOffSet = rot1 * Inverse(rot2);//rot2 * Inverse(rot1);
			limitConstraint->pNodeB = pNodeA;
			limitConstraint->pNodeA = pNodeB;

			spherical->m_AnchorPoint2 = m_AnchorPoint1;
			spherical->m_AnchorPoint1 = m_AnchorPoint2;
			spherical->pNodeB = pNodeA;
			spherical->pNodeA = pNodeB;

			pProxy->pNodeB = pNodeA;
			pProxy->pNodeA = pNodeB;
		}
		limitConstraint->SetBaumgarte(m_Baumgarte);
		limitConstraint->m_LowJointLimit = m_LowJointLimit;
		limitConstraint->m_HighJointLimit = m_HighJointLimit;
		limitConstraint->m_AxisOrder = m_AxisOrder;

		spherical->SetBaumgarte(m_Baumgarte);
		
		pEdge->AddConstraintPoint(pPoint1);
		pPoint1->m_pConstraint = spherical;
		
		pEdge->AddConstraintPoint(pPoint2);
		pPoint2->m_pConstraint = limitConstraint;

		/*pProxy->m_pSphericalConstraint = spherical;
		pProxy->m_pSphericalLimitConstraint= limitConstraint;*/

		pProxy->AddConstraint(spherical);
		pProxy->AddConstraint(limitConstraint);
		spherical->pRoot = pProxy;
		limitConstraint->pRoot = pProxy;
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_SphericalLimit);
		return pProxy;

	}

	void SphericalJointLimitDescriptor::Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);

		Maths::HVec3 lowJointLimit(Maths::UNDEFINED);
		Maths::HVec3 highJointLimit(Maths::UNDEFINED);
		deserial >> m_AnchorPoint1 >> m_AnchorPoint2 >> lowJointLimit >> highJointLimit;

		//m_LowJointLimit and m_HighJointLimit is based on radian
		DefineLimits(lowJointLimit, highJointLimit);
	}

#endif

	void SphericalJointLimitDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1 = anchor1;
		m_AnchorPoint2 = anchor2;
	}

	void SphericalJointLimitDescriptor::DefineLimits(const Maths::HVec3 PCREF lowLimit, const Maths::HVec3 PCREF highLimit)
	{
		const Maths::HF32 ninety(SharedPhysics::Constants::Ninety());
		const Maths::HVec3 vNinety(ninety);
		const Maths::HVec3 absLow(Maths::Abs(lowLimit));
		const Maths::HVec3 absHigh(Maths::Abs(highLimit));
		BIZ_ASSERT(absLow.Any_LT(vNinety) != 0);
		BIZ_ASSERT(absHigh.Any_LT(vNinety) != 0);

		if(absLow.Y().LT(ninety) & absHigh.Y().LT(ninety))
		{
			m_AxisOrder = SharedPhysics::C_Constraint::E_AxisOrder_XYZ;
		}
		else if(absLow.X().LT(ninety) & absHigh.X().LT(ninety))
		{
			m_AxisOrder = SharedPhysics::C_Constraint::E_AxisOrder_ZXY;
		}
		else
		{
			m_AxisOrder = SharedPhysics::C_Constraint::E_AxisOrder_XZY;
		}

		Maths::HF32 pi(Maths::Constants<Maths::HF32>::Pi());
		Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 twoPiR(pi*two);
		const Maths::HF32 twoPiD(360.f);

		//Change the limit from degree to radian
		m_LowJointLimit=(lowLimit/twoPiD)*twoPiR;
		m_HighJointLimit=(highLimit/twoPiD)*twoPiR;
		
	}


	//void SphericalJointLimitDescriptor::DefineAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	//{
	//	m_JointAxis1 = jointAxis1;
	//	m_JointAxis2 = jointAxis2;
	//}
	void RotationalLimitDescriptor::DefineLimits(const Maths::HVec3 PCREF lowLimit, const Maths::HVec3 PCREF highLimit)
	{
		const Maths::HF32 ninety(SharedPhysics::Constants::Ninety());
		const Maths::HVec3 vNinety(ninety);
		const Maths::HVec3 absLow(Maths::Abs(lowLimit));
		const Maths::HVec3 absHigh(Maths::Abs(highLimit));
		BIZ_ASSERT(absLow.Any_LT(vNinety) != 0);
		BIZ_ASSERT(absHigh.Any_LT(vNinety) != 0);

		if(absLow.Y().LT(ninety) & absHigh.Y().LT(ninety))
		{
			m_AxisOrder = SharedPhysics::C_Constraint::E_AxisOrder_XYZ;
		}
		else if(absLow.X().LT(ninety) & absHigh.X().LT(ninety))
		{
			m_AxisOrder = SharedPhysics::C_Constraint::E_AxisOrder_ZXY;
		}
		else
		{
			m_AxisOrder = SharedPhysics::C_Constraint::E_AxisOrder_XZY;
		}


		Maths::HF32 pi(Maths::Constants<Maths::HF32>::Pi());
		Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 twoPiR(pi*two);
		const Maths::HF32 twoPiD(360.f);

		//Change the limit from degree to radian
		m_LowJointLimit=(lowLimit/twoPiD)*twoPiR;
		m_HighJointLimit=(highLimit/twoPiD)*twoPiR;

	}

	

#ifndef GRAVITY_EXPORTER


	void RotationalLimitDescriptor::Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);

		Maths::HVec3 lowJointLimit(Maths::UNDEFINED);
		Maths::HVec3 highJointLimit(Maths::UNDEFINED);
		deserial >> lowJointLimit >> highJointLimit;

		//define m_LowJointLimit and m_HighJointLimit in radian based on lowJointLimit and highJointLimit in degree 
		DefineLimits(lowJointLimit, highJointLimit);
	}


	SharedPhysics::C_Constraint* RotationalLimitDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		
		Solver2::C_SphericalLimitConstraint* limitConstraint=manager.m_SphericalLimitConstraints.GetNextFreeItem();
		
		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		

		const Maths::Mat33 rot1(pBodyA->GetBody().m_OrientationMatrix);
		const Maths::Mat33 rot2(pBodyB->GetBody().m_OrientationMatrix);

		SharedPhysics::C_GraphNode* pNodeA = pBodyA->GetGraphNode();
		SharedPhysics::C_GraphNode* pNodeB = pBodyB->GetGraphNode();
		
		if(pEdge->m_pInboundNode == pNodeA)
		{

			limitConstraint->m_rotationalOffSet = rot2 * Inverse(rot1);//rot1 * Inverse(rot2); 
			limitConstraint->pNodeA = pNodeA;
			limitConstraint->pNodeB = pNodeB;
		}
		else
		{
			limitConstraint->m_rotationalOffSet = rot1 * Inverse(rot2);//rot2 * Inverse(rot1);
			limitConstraint->pNodeB = pNodeA;
			limitConstraint->pNodeA = pNodeB;

		}
		limitConstraint->SetBaumgarte(m_Baumgarte);

		////Change the limit from degree to radian
		limitConstraint->m_LowJointLimit=m_LowJointLimit;
		limitConstraint->m_HighJointLimit=m_HighJointLimit;

		limitConstraint->m_AxisOrder = m_AxisOrder;
		
		pEdge->AddConstraintPoint(pPoint);
		pPoint->m_pConstraint = limitConstraint;

		return limitConstraint;

	}
#endif


#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* DistanceConstraintDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		Solver2::C_DistanceConstraint* distanceConstraint=manager.m_DistanceConstraints.GetNextFreeItem();
		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_GraphNode* pNodeA(pBodyA->GetGraphNode());
		SharedPhysics::C_GraphNode* pNodeB(pBodyB->GetGraphNode());

		if(pEdge->m_pInboundNode == pNodeA)
		{
			distanceConstraint->m_AnchorPoint1 = m_AnchorPoint1;
			distanceConstraint->m_AnchorPoint2 = m_AnchorPoint2;
			distanceConstraint->pNodeA = pNodeA;
			distanceConstraint->pNodeB = pNodeB;
		}
		else
		{
			distanceConstraint->m_AnchorPoint2 = m_AnchorPoint1;
			distanceConstraint->m_AnchorPoint1 = m_AnchorPoint2;
			distanceConstraint->pNodeB = pNodeA;
			distanceConstraint->pNodeA = pNodeB;
		}
		distanceConstraint->SetBaumgarte(m_Baumgarte);
		distanceConstraint->m_DistanceLimit = m_DistanceLimit;
		
		pEdge->AddConstraintPoint(pPoint);
		pPoint->m_pConstraint = distanceConstraint;

		return distanceConstraint;

	}
#endif

	
	void DistanceConstraintDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1 = anchor1;
		m_AnchorPoint2 = anchor2;
	}

	void DistanceConstraintDescriptor::DefineDistanceLimit(const Maths::HF32 PCREF distanceLimit)
	{
		m_DistanceLimit = distanceLimit;
	}

#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* FixedJointDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		using namespace Maths;

		//Solver2::C_FixedConstraint* pProxy = manager.m_FixedProxyConstraints.GetNextFreeItem();

		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_SphericalConstraint* sphericalConstraint=manager.m_SphericalConstraints.GetNextFreeItem();
		Solver2::C_FixedRotationalConstraint* fixedConstraint=manager.m_FixedConstraints.GetNextFreeItem();

		fixedConstraint->SetScale(m_Scale);
		sphericalConstraint->SetScale(m_Scale);
		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint2(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_GraphNode* pNodeA(pBodyA->GetGraphNode());
		SharedPhysics::C_GraphNode* pNodeB(pBodyB->GetGraphNode());

		SharedPhysics::C_RigidBody& bodyA = pBodyA->GetBody();
		SharedPhysics::C_RigidBody& bodyB = pBodyB->GetBody();

		//const VF32 two(Const<VF32>::Two());
		//const VF32 zero(Const<VF32>::Zero());
		const Quaternion rot1(pBodyA->GetBody().m_Orientation);
		const Quaternion rot2(pBodyB->GetBody().m_Orientation);

		if(pEdge->m_pInboundNode == pNodeA)
		{
			
			fixedConstraint->pNodeA = pNodeA;
			fixedConstraint->pNodeB = pNodeB;
			fixedConstraint->m_rotationalOffSet = Maths::NearestDivide(rot2, rot1);//Maths::NearestDifference(rot1, rot2);

			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint2;
			sphericalConstraint->pNodeA = pNodeA;
			sphericalConstraint->pNodeB = pNodeB;

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;
		}
		else
		{
			
			fixedConstraint->pNodeB = pNodeA;
			fixedConstraint->pNodeA = pNodeB;
			fixedConstraint->m_rotationalOffSet = Maths::NearestDivide(rot1, rot2);//Maths::NearestDifference(rot2, rot1);
			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint2;
			sphericalConstraint->pNodeB = pNodeA;
			sphericalConstraint->pNodeA = pNodeB;
			pProxy->pNodeB = pNodeA;
			pProxy->pNodeA = pNodeB;
		}
		
		fixedConstraint->SetBaumgarte(m_Baumgarte);
		sphericalConstraint->SetBaumgarte(m_Baumgarte);
		pEdge->AddConstraintPoint(pPoint1);
		pEdge->AddConstraintPoint(pPoint2);
		pPoint1->m_pConstraint = fixedConstraint;
		pPoint2->m_pConstraint = sphericalConstraint;
		/*pProxy->m_pFixedConstraint = fixedConstraint;
		pProxy->m_pSphericalConstraint = sphericalConstraint;*/
		fixedConstraint->pRoot = pProxy;
		sphericalConstraint->pRoot = pProxy;
		pProxy->AddConstraint(fixedConstraint);
		pProxy->AddConstraint(sphericalConstraint);
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_Fixed);
		return pProxy;
	}

#endif

	void FixedJointDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1 = anchor1;
		m_AnchorPoint2 = anchor2;
	}


#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* FixedRotationalDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		using namespace Maths;

		Solver2::C_FixedRotationalConstraint* fixedConstraint=manager.m_FixedConstraints.GetNextFreeItem();

		fixedConstraint->SetScale(m_Scale);
		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_GraphNode* pNodeA(pBodyA->GetGraphNode());
		SharedPhysics::C_GraphNode* pNodeB(pBodyB->GetGraphNode());

		SharedPhysics::C_RigidBody& bodyA = pBodyA->GetBody();
		SharedPhysics::C_RigidBody& bodyB = pBodyB->GetBody();

		//const VF32 two(Const<VF32>::Two());
		//const VF32 zero(Const<VF32>::Zero());
		const Quaternion rot1(pBodyA->GetBody().m_Orientation);
		const Quaternion rot2(pBodyB->GetBody().m_Orientation);

		if(pEdge->m_pInboundNode == pNodeA)
		{
			
			fixedConstraint->pNodeA = pNodeA;
			fixedConstraint->pNodeB = pNodeB;
			//fixedConstraint->m_rotationalOffSet = Maths::NearestDifference(rot1, rot2);
			fixedConstraint->m_rotationalOffSet = Maths::NearestDivide(rot2, rot1);
		}
		else
		{
			
			fixedConstraint->pNodeB = pNodeA;
			fixedConstraint->pNodeA = pNodeB;
			//fixedConstraint->m_rotationalOffSet = Maths::NearestDifference(rot2, rot1);
			fixedConstraint->m_rotationalOffSet = Maths::NearestDivide(rot1, rot2);
		}
		
		fixedConstraint->SetBaumgarte(m_Baumgarte);
		pEdge->AddConstraintPoint(pPoint);
		pPoint->m_pConstraint = fixedConstraint;
		return fixedConstraint;
	}

#endif




	/*
		RevoluteJointLimitDescriptor
	*/
#ifndef GRAVITY_EXPORTER


	SharedPhysics::C_Constraint* RevoluteJointLimitDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		//BIZ_ASSERT(m_LowJointLimit < m_HighJointLimit);
		//const Maths::VF32 zero(Maths::Const<Maths::VF32>::Zero());
		//Solver2::C_RevoluteLimitProxyConstraint* pProxy = manager.m_RevoluteLimitProxyConstraints.GetNextFreeItem();
		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_RevoluteLimitConstraint* limitConstraint=manager.m_RevoluteLimitConstraint.GetNextFreeItem(); 
		Solver2::C_RevoluteConstraint* revolute=manager.m_RevoluteConstraints.GetNextFreeItem();
		Solver2::C_SphericalConstraint* spherical=manager.m_SphericalConstraints.GetNextFreeItem();

		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint0(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint2(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		
		const Maths::HVec3 posA(pBodyA->Position());
		const Maths::HVec3 posB(pBodyB->Position());

		const Maths::Quaternion rot1(pBodyA->GetBody().m_Orientation);
		const Maths::Quaternion rot2(pBodyB->GetBody().m_Orientation);

		SharedPhysics::C_GraphNode* pNodeA = pBodyA->GetGraphNode();
		SharedPhysics::C_GraphNode* pNodeB = pBodyB->GetGraphNode();
		
		if(pEdge->m_pInboundNode == pNodeA)
		{

			revolute->m_JointAxis1 = m_JointAxis1;
			revolute->m_JointAxis2 = m_JointAxis2;
			revolute->m_rotationalOffSet = Maths::NearestDifference(rot1, rot2);
			revolute->pNodeA = pNodeA;
			revolute->pNodeB = pNodeB;

			limitConstraint->m_JointAxis1   = m_JointAxis1;
			limitConstraint->m_JointAxis2   = m_JointAxis2;
			limitConstraint->m_rotationalOffSet = Maths::NearestDifference(rot1, rot2);
			//limitConstraint->m_rotationalOffSet = Maths::NearestDivide(rot2, rot1);
			
			limitConstraint->pNodeA = pNodeA;
			limitConstraint->pNodeB = pNodeB;

			spherical->m_AnchorPoint1 = m_AnchorPoint1;
			spherical->m_AnchorPoint2 = m_AnchorPoint2;
			spherical->pNodeA = pNodeA;
			spherical->pNodeB = pNodeB;

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;
		}
		else
		{
			revolute->m_JointAxis2 = m_JointAxis1;
			revolute->m_JointAxis1 = m_JointAxis2;
			revolute->m_rotationalOffSet = Maths::NearestDifference(rot2, rot1);
			revolute->pNodeB = pNodeA;
			revolute->pNodeA = pNodeB;

			limitConstraint->m_JointAxis2   = m_JointAxis1;
			limitConstraint->m_JointAxis1   = m_JointAxis2;
			limitConstraint->m_rotationalOffSet = Maths::NearestDifference(rot2, rot1);
			//limitConstraint->m_rotationalOffSet = Maths::NearestDivide(rot1, rot2);
			limitConstraint->pNodeB = pNodeA;
			limitConstraint->pNodeA = pNodeB;

			spherical->m_AnchorPoint2 = m_AnchorPoint1;
			spherical->m_AnchorPoint1 = m_AnchorPoint2;
			spherical->pNodeB = pNodeA;
			spherical->pNodeA = pNodeB;

			pProxy->pNodeB = pNodeA;
			pProxy->pNodeA = pNodeB;
		}

		
		limitConstraint->SetBaumgarte(m_Baumgarte);
		limitConstraint->m_LowJointLimit= m_LowJointLimit;
		limitConstraint->m_HighJointLimit= m_HighJointLimit;

		revolute->SetBaumgarte(m_Baumgarte);
		spherical->SetBaumgarte(m_Baumgarte);
		
		pEdge->AddConstraintPoint(pPoint0);
		pPoint0->m_pConstraint = revolute;

		pEdge->AddConstraintPoint(pPoint1);
		pPoint1->m_pConstraint = spherical;

		pEdge->AddConstraintPoint(pPoint2);
		pPoint2->m_pConstraint = limitConstraint;

		/*pProxy->m_pSphericalConstraint = spherical;
	 	pProxy->m_pRevoluteConstraint = revolute;
		pProxy->m_pLimitConstraint = limitConstraint;*/
		revolute->pRoot = pProxy;
		spherical->pRoot = pProxy;
		limitConstraint->pRoot = pProxy;

		pProxy->AddConstraint(revolute);
		pProxy->AddConstraint(spherical);
		pProxy->AddConstraint(limitConstraint);
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_RevoluteLimit);
		return pProxy;

	}

	void RevoluteJointLimitDescriptor::Import(Deserialiser& deserial)
	{
		ImportBasic(deserial);
		Maths::HF32 lowJointLimit(Maths::UNDEFINED);
		Maths::HF32 highJointLimit(Maths::UNDEFINED);
		deserial >> m_AnchorPoint1 >> m_AnchorPoint2 >> m_JointAxis1 >> m_JointAxis2 >> lowJointLimit >> highJointLimit;


		DefineJointLimit(lowJointLimit, highJointLimit);
	}



#endif
	

	void RevoluteJointLimitDescriptor::DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	{
		m_JointAxis1=jointAxis1;
		m_JointAxis2=jointAxis2;
	}

	void RevoluteJointLimitDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1 = anchor1;
		m_AnchorPoint2 = anchor2;
	}

	/*
		When we sets up the revolute joint limits, make sure the the lowLimit > highLimit.
	*/

	void RevoluteJointLimitDescriptor::DefineJointLimit(const Maths::HF32 PCREF lowLimit, const Maths::HF32 PCREF highLimit)
	{
		//m_LowJointLimit=lowLimit;
		//m_HighJointLimit=highLimit;
		//BIZ_ASSERT(lowLimit < highLimit);

		Maths::HF32 pi(Maths::Constants<Maths::HF32>::Pi());
		Maths::HF32 two(Maths::Constants<Maths::HF32>::Two());
		const Maths::HF32 twoPiR(pi*two);
		const Maths::HF32 twoPiD(360.f);

		//Change the limit from degree to radian
		m_LowJointLimit=(lowLimit/twoPiD)*twoPiR;
		m_HighJointLimit=(highLimit/twoPiD)*twoPiR;

	}


	/*
		PrismaticJointDescriptor
	*/
#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* PrismaticJointDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		//Solver2::C_PrismaticProxyConstraint* pProxy = manager.m_PrismaticProxyConstraints.GetNextFreeItem();

		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_PrismaticConstraint* prismaticConstraint=manager.m_PrismaticConstraint.GetNextFreeItem();
		Solver2::C_FixedRotationalConstraint* fixedConstraint = manager.m_FixedConstraints.GetNextFreeItem();

		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());
		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint2(manager.m_PersistentConstraintPoints.GetNextFreeItem());

		const Maths::HVec3 posA(pBodyA->Position());
		const Maths::HVec3 posB(pBodyB->Position());

		SharedPhysics::C_GraphNode* pNodeA = pBodyA->GetGraphNode();
		SharedPhysics::C_GraphNode* pNodeB = pBodyB->GetGraphNode();
		
		if(pEdge->m_pInboundNode == pBodyA->GetGraphNode())
		{
			prismaticConstraint->m_JointAxis1   = m_JointAxis1;
			prismaticConstraint->m_JointAxis2   = m_JointAxis2;
			prismaticConstraint->m_OffSetA=pBodyB->Orientation() * (posB - posA);
			prismaticConstraint->m_OffSetB=pBodyA->Orientation() * (posA - posB);
			fixedConstraint->pNodeA = pNodeA;
			fixedConstraint->pNodeB = pNodeB;

			fixedConstraint->m_rotationalOffSet = Maths::NearestDifference(pBodyA->GetBody().m_Orientation, pBodyB->GetBody().m_Orientation);

			prismaticConstraint->pNodeA = pNodeA;
			prismaticConstraint->pNodeB = pNodeB;

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;


			//prismaticConstraint->m_OffSet=pBodyA->Orientation() * (posA - posB);
		}
		else
		{
			prismaticConstraint->m_JointAxis2   = m_JointAxis1;
			prismaticConstraint->m_JointAxis1   = m_JointAxis2;
			prismaticConstraint->m_OffSetA=pBodyA->Orientation() * (posA - posB);
			prismaticConstraint->m_OffSetB=pBodyB->Orientation() * (posB - posA);
			prismaticConstraint->pNodeB = pNodeA;
			prismaticConstraint->pNodeA = pNodeB;

			fixedConstraint->m_rotationalOffSet = Maths::NearestDifference(pBodyB->GetBody().m_Orientation, pBodyA->GetBody().m_Orientation);
			fixedConstraint->pNodeB = pNodeA;
			fixedConstraint->pNodeA = pNodeB;

			prismaticConstraint->pNodeB = pNodeA;
			prismaticConstraint->pNodeA = pNodeB;

			pProxy->pNodeB = pNodeA;
			pProxy->pNodeA = pNodeB;

		

			//prismaticConstraint->m_OffSet=pBodyB->Orientation() * (posB - posA);
		}
		prismaticConstraint->SetBaumgarte(m_Baumgarte);
		fixedConstraint->SetBaumgarte(m_Baumgarte);
		pEdge->AddConstraintPoint(pPoint1);
		pEdge->AddConstraintPoint(pPoint2);
		pPoint1->m_pConstraint = fixedConstraint;
		pPoint2->m_pConstraint = prismaticConstraint;
		/*pProxy->m_pPrismaticConstraint = prismaticConstraint;
		pProxy->m_pFixedConstraint = fixedConstraint;*/
		fixedConstraint->pRoot = pProxy;
		prismaticConstraint->pRoot = pProxy;
		pProxy->AddConstraint(fixedConstraint);
		pProxy->AddConstraint(prismaticConstraint);
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_Prismatic);
		return pProxy;

	}

#endif
	

	void PrismaticJointDescriptor::DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	{
		m_JointAxis1=jointAxis1;
		m_JointAxis2=jointAxis2;
	}


	/*
		PrismaticJointLimitDescriptor
	*/
#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* PrismaticJointLimitDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		//Solver2::C_PrismaticLimitProxyConstraint* pProxy = manager.m_PrismaticLimitProxyConstraints.GetNextFreeItem();
		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_PrismaticLimitConstraint* limitConstraint=manager.m_PrismaticLimitConstraint.GetNextFreeItem();
		Solver2::C_FixedRotationalConstraint* fixedConstraint = manager.m_FixedConstraints.GetNextFreeItem();
		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());
		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint2(manager.m_PersistentConstraintPoints.GetNextFreeItem());

		const Maths::HVec3 posA(pBodyA->Position());
		const Maths::HVec3 posB(pBodyB->Position());

		Maths::Quaternion rot1(pBodyA->GetBody().m_Orientation);
		Maths::Quaternion rot2(pBodyB->GetBody().m_Orientation);

		SharedPhysics::C_GraphNode* pNodeA = pBodyA->GetGraphNode();
		SharedPhysics::C_GraphNode* pNodeB = pBodyB->GetGraphNode();
		
		if(pEdge->m_pInboundNode == pBodyA->GetGraphNode())
		{
			limitConstraint->m_JointAxis1   = m_JointAxis1;
			limitConstraint->m_JointAxis2   = m_JointAxis2;
			limitConstraint->m_OffSetA=pBodyB->Orientation() * (posB - posA);
			limitConstraint->m_OffSetB=pBodyA->Orientation() * (posA - posB);
			limitConstraint->pNodeA = pNodeA;
			limitConstraint->pNodeB = pNodeB;

			fixedConstraint->pNodeA = pNodeA;
			fixedConstraint->pNodeB = pNodeB;
			//fixedConstraint->m_rotationalOffSet = Maths::NearestDifference(rot1, rot2);
			fixedConstraint->m_rotationalOffSet = Maths::NearestDivide(rot2, rot1);

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;



			//prismaticConstraint->m_OffSet=pBodyA->Orientation() * (posA - posB);
		}
		else
		{
			limitConstraint->m_JointAxis2   = m_JointAxis1;
			limitConstraint->m_JointAxis1   = m_JointAxis2;
			limitConstraint->m_OffSetA=pBodyA->Orientation() * (posA - posB);
			limitConstraint->m_OffSetB=pBodyB->Orientation() * (posB - posA);
			limitConstraint->pNodeB = pNodeA;
			limitConstraint->pNodeA = pNodeB;

			fixedConstraint->pNodeB = pNodeA;
			fixedConstraint->pNodeA = pNodeB;
			//fixedConstraint->m_rotationalOffSet = Maths::NearestDifference(rot2, rot1);
			fixedConstraint->m_rotationalOffSet = Maths::NearestDivide(rot1, rot2);

			pProxy->pNodeB = pNodeA;
			pProxy->pNodeA = pNodeB;
			//prismaticConstraint->m_OffSet=pBodyB->Orientation() * (posB - posA);
		}
		limitConstraint->SetBaumgarte(m_Baumgarte);
		limitConstraint->m_LowJointLimit= m_LowJointLimit;
		limitConstraint->m_HighJointLimit= m_HighJointLimit;
		pEdge->AddConstraintPoint(pPoint1);
		pEdge->AddConstraintPoint(pPoint2);
		pPoint1->m_pConstraint = fixedConstraint;
		pPoint2->m_pConstraint = limitConstraint;
		
		/*pProxy->m_pFixedConstraint = fixedConstraint;
		pProxy->m_pPrismaticLimitConstraint = limitConstraint;*/
		fixedConstraint->pRoot = pProxy;
		limitConstraint->pRoot = pProxy;
		pProxy->AddConstraint(fixedConstraint);
		pProxy->AddConstraint(limitConstraint);
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_PrismaticLimit);
		return pProxy;

	}

#endif

	void PrismaticJointLimitDescriptor::DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	{
		m_JointAxis1=jointAxis1;
		m_JointAxis2=jointAxis2;
	}

	/*
	  When we set up the prismatic joint limit, make sure the lowLimit is set to be negative and highLimit
	  is set to be positive. The reason for that is because the lowLimit is the limit boundary on the opposite
	  side of the joint axis, therefore, the sign distance should be negative, vice-versa for the highLimit 
	*/

	void PrismaticJointLimitDescriptor::DefineJointLimit(const Maths::HF32 PCREF lowLimit, const Maths::HF32 PCREF highLimit)
	{
		m_LowJointLimit=lowLimit;
		m_HighJointLimit=highLimit;
	}


	/*
		CarWheelDescriptor
	*/
#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* CarWheelDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{

		//Solver2::C_CarFrontWheelProxyConstraint* pProxy=manager.m_CarFrontWheelProxyConstraints.GetNextFreeItem();
		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_CarWheelConstraint* pCarWheelConstraint = manager.m_CarWheelConstraints.GetNextFreeItem();
		Solver2::C_RevoluteConstraint* pRevoluteConstraint = manager.m_RevoluteConstraints.GetNextFreeItem();

		SharedPhysics::C_GraphNode* pNodeA = pBodyA->GetGraphNode();
		SharedPhysics::C_GraphNode* pNodeB = pBodyB->GetGraphNode();

		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pNodeA, pNodeB);
		else
			pEdge = manager.GetConstraintEdge(pNodeB, pNodeA);
		SharedPhysics::C_ConstraintPoint* pPoint0(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		
		const Maths::HVec3 posA(pBodyA->Position());
		const Maths::HVec3 posB(pBodyB->Position());

		const Maths::Quaternion rot1(Maths::C_Quaternion(pBodyA->GetBody().m_OrientationMatrix));
		const Maths::Quaternion rot2(Maths::C_Quaternion(pBodyB->GetBody().m_OrientationMatrix));

		
		if(pEdge->m_pInboundNode == pNodeA)
		{
			pRevoluteConstraint->m_JointAxis1   = m_RevoluteJointAxis1;
			pRevoluteConstraint->m_JointAxis2   = m_RevoluteJointAxis2;
			pRevoluteConstraint->m_rotationalOffSet = Maths::NearestDifference(rot1, rot2);

			pCarWheelConstraint->m_JointAxis1 = m_PrismaticJointAxis1;
			pCarWheelConstraint->m_JointAxis2 = m_PrismaticJointAxis2;
			pCarWheelConstraint->m_LowJointLimit = m_PrismaticLowJointLimit;
			pCarWheelConstraint->m_HighJointLimit = m_PrismaticHighJointLimit;
			pCarWheelConstraint->m_OffSetA = pBodyB->Orientation() * (posB - posA);
			pCarWheelConstraint->m_OffSetB = pBodyA->Orientation() * (posA - posB);


			pCarWheelConstraint->pNodeA = pNodeA;
			pCarWheelConstraint->pNodeB = pNodeB;

			pRevoluteConstraint->pNodeA = pNodeA;
			pRevoluteConstraint->pNodeB = pNodeB;

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;

		}
		else
		{
			pRevoluteConstraint->m_JointAxis2   = m_RevoluteJointAxis1;
			pRevoluteConstraint->m_JointAxis1   = m_RevoluteJointAxis2;
			pRevoluteConstraint->m_rotationalOffSet = Maths::NearestDifference(rot2, rot1);

			pCarWheelConstraint->m_JointAxis2 = m_PrismaticJointAxis1;
			pCarWheelConstraint->m_JointAxis1 = m_PrismaticJointAxis2;
			pCarWheelConstraint->m_LowJointLimit = m_PrismaticLowJointLimit;
			pCarWheelConstraint->m_HighJointLimit = m_PrismaticHighJointLimit;
			pCarWheelConstraint->m_OffSetA = pBodyA->Orientation() * (posA - posB);
			pCarWheelConstraint->m_OffSetB = pBodyB->Orientation() * (posB - posA);


			pRevoluteConstraint->pNodeB = pNodeA;
			pRevoluteConstraint->pNodeA = pNodeB;

			pCarWheelConstraint->pNodeB = pNodeA;
			pCarWheelConstraint->pNodeA = pNodeB;

			pProxy->pNodeB = pNodeA;
			pProxy->pNodeA = pNodeB;
		}

		pRevoluteConstraint->SetBaumgarte(m_Baumgarte);
		pPoint0->m_pConstraint = pRevoluteConstraint;
		pEdge->AddConstraintPoint(pPoint0);

		pCarWheelConstraint->SetBaumgarte(m_Baumgarte);
		pPoint1->m_pConstraint = pCarWheelConstraint;
		pEdge->AddConstraintPoint(pPoint1);

		/*pProxy->m_pCarFrontWheelConstraint = pCarFrontWheelConstraint;
		pProxy->m_pRevoluteConstraint = pRevoluteConstraint;*/
		pRevoluteConstraint->pRoot = pProxy;
		pCarWheelConstraint->pRoot = pProxy;

		pProxy->AddConstraint(pRevoluteConstraint);
		pProxy->AddConstraint(pCarWheelConstraint);
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_CarWheel);
		return pProxy;

	}
#endif


	void CarWheelDescriptor::DefineRevoluteJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	{
		m_RevoluteJointAxis1 = jointAxis1;
		m_RevoluteJointAxis2 = jointAxis2;
	}

	void CarWheelDescriptor::DefinePrismaticJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	{
		m_PrismaticJointAxis1 = jointAxis1;
		m_PrismaticJointAxis2 = jointAxis2;
	}


	void CarWheelDescriptor::DefinePrismaticLimits(const Maths::HF32 PCREF lowLimit, const Maths::HF32 PCREF highLimit)
	{
		m_PrismaticLowJointLimit = lowLimit;
		m_PrismaticHighJointLimit = highLimit;
	}

	




#ifndef GRAVITY_EXPORTER

	SharedPhysics::C_Constraint* TwoRotationalProxyDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		//Solver2::C_TwoRotationalProxyConstraint* pProxy=manager.m_TwoRotationalProxyConstraints.GetNextFreeItem();

		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_TwoRotationalConstraint* twoRConstraint = manager.m_TwoRotationalConstraints.GetNextFreeItem();
		Solver2::C_SphericalConstraint* sphericalConstraint = manager.m_SphericalConstraints.GetNextFreeItem();


		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());
		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint2(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		

		const Maths::HVec3 posA(pBodyA->Position());
		const Maths::HVec3 posB(pBodyB->Position());

		const Maths::Quaternion rot1(pBodyA->GetBody().m_Orientation);
		const Maths::Quaternion rot2(pBodyB->GetBody().m_Orientation);

		SharedPhysics::C_GraphNode* pNodeA = pBodyA->GetGraphNode();
		SharedPhysics::C_GraphNode* pNodeB = pBodyB->GetGraphNode();
		
		if(pEdge->m_pInboundNode == pBodyA->GetGraphNode())
		{
			twoRConstraint->m_JointAxis1   = m_JointAxis1;
			twoRConstraint->m_JointAxis2   = m_JointAxis2;
			twoRConstraint->pNodeA = pNodeA;
			twoRConstraint->pNodeB = pNodeB;
			//twoRConstraint->m_rotationalOffset = Maths::NearestDifference(rot1, rot2);
			twoRConstraint->m_rotationalOffset = Maths::NearestDivide(rot2, rot1);

			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint2;
			sphericalConstraint->pNodeA = pNodeA;
			sphericalConstraint->pNodeB = pNodeB;

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;
			

			//prismaticConstraint->m_OffSet=pBodyA->Orientation() * (posA - posB);
		}
		else
		{
			twoRConstraint->m_JointAxis2 = m_JointAxis1;
			twoRConstraint->m_JointAxis1 = m_JointAxis2;
			twoRConstraint->pNodeB = pNodeA;
			twoRConstraint->pNodeA = pNodeB;
			//twoRConstraint->m_rotationalOffset = Maths::NearestDifference(rot2, rot1);
			twoRConstraint->m_rotationalOffset = Maths::NearestDivide(rot1, rot2);

			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint2;
			sphericalConstraint->pNodeB = pNodeA;
			sphericalConstraint->pNodeA = pNodeB;

			pProxy->pNodeB = pNodeA;
			pProxy->pNodeA = pNodeB;

		}
		twoRConstraint->SetBaumgarte(m_Baumgarte);
		sphericalConstraint->SetBaumgarte(m_Baumgarte);
		pEdge->AddConstraintPoint(pPoint1);
		pEdge->AddConstraintPoint(pPoint2);
		pPoint1->m_pConstraint = sphericalConstraint;
		pPoint2->m_pConstraint = twoRConstraint;
		
		/*pProxy->m_pTwoRotationalConstraint = twoRConstraint;
		pProxy->m_pSphericalConstraint = sphericalConstraint;*/
		sphericalConstraint->pRoot = pProxy;
		twoRConstraint->pRoot = pProxy;
		pProxy->AddConstraint(sphericalConstraint);
		pProxy->AddConstraint(twoRConstraint);
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_TwoRotational);
		return pProxy;

	}
#endif

	void TwoRotationalProxyDescriptor::DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	{
		m_JointAxis1=jointAxis1;
		m_JointAxis2=jointAxis2;
	}

	void TwoRotationalProxyDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1=anchor1;
		m_AnchorPoint2=anchor2;
	}

	/*void TwoRotationalProxyDescriptor::DefineOffsetAngle(const Maths::HF32 PCREF angle)
	{
		m_OffsetAngle = angle;
	}*/




#ifndef GRAVITY_EXPORTER
	SharedPhysics::C_Constraint* RevoluteJointDescriptor::CreateConstraint(Solver2::C_ConstraintGraphManager& manager, C_Body* pBodyA, C_Body* pBodyB) const
	{
		//Solver2::C_RevoluteProxyConstraint* pProxy = manager.m_RevoluteProxyConstraints.GetNextFreeItem();
		Solver2::C_ProxyConstraint* pProxy = manager.m_ProxyConstraints.GetNextFreeItem();
		Solver2::C_SphericalConstraint* sphericalConstraint=manager.m_SphericalConstraints.GetNextFreeItem();
		Solver2::C_RevoluteConstraint* revoluteConstraint=manager.m_RevoluteConstraints.GetNextFreeItem();
		SharedPhysics::C_ConstraintEdge* pEdge = NULL;
		if(!pBodyA->GetGraphNode()->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | SharedPhysics::C_GraphNode::E_Type_KinematicObject))
			pEdge = manager.GetConstraintEdge(pBodyA->GetGraphNode(), pBodyB->GetGraphNode());
		else
			pEdge = manager.GetConstraintEdge(pBodyB->GetGraphNode(), pBodyA->GetGraphNode());

		SharedPhysics::C_ConstraintPoint* pPoint1(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_ConstraintPoint* pPoint2(manager.m_PersistentConstraintPoints.GetNextFreeItem());
		SharedPhysics::C_GraphNode* pNodeA(pBodyA->GetGraphNode());
		SharedPhysics::C_GraphNode* pNodeB(pBodyB->GetGraphNode());

		const Maths::Quaternion rot1(pBodyA->GetBody().m_Orientation);
		const Maths::Quaternion rot2(pBodyB->GetBody().m_Orientation);

		if(pEdge->m_pInboundNode == pNodeA)
		{
			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint2;
			sphericalConstraint->pNodeA = pNodeA;
			sphericalConstraint->pNodeB = pNodeB;
			revoluteConstraint->m_JointAxis1 = m_JointAxis1;
			revoluteConstraint->m_JointAxis2 = m_JointAxis2;
			revoluteConstraint->m_rotationalOffSet = Maths::NearestDifference(rot1, rot2);
			revoluteConstraint->pNodeA = pNodeA;
			revoluteConstraint->pNodeB = pNodeB;

			pProxy->pNodeA = pNodeA;
			pProxy->pNodeB = pNodeB;
		}
		else
		{
			sphericalConstraint->m_AnchorPoint2 = m_AnchorPoint1;
			sphericalConstraint->m_AnchorPoint1 = m_AnchorPoint2;
			sphericalConstraint->pNodeB = pNodeA;
			sphericalConstraint->pNodeA = pNodeB;
			revoluteConstraint->m_JointAxis1 = m_JointAxis2;
			revoluteConstraint->m_JointAxis2 = m_JointAxis1;
			revoluteConstraint->m_rotationalOffSet = Maths::NearestDifference(rot2, rot1);
			revoluteConstraint->pNodeA = pNodeB;
			revoluteConstraint->pNodeB = pNodeA;

			pProxy->pNodeA = pNodeB;
			pProxy->pNodeB = pNodeA;
		}
		sphericalConstraint->SetBaumgarte(m_Baumgarte);
		revoluteConstraint->SetBaumgarte(m_Baumgarte);
		//sphericalConstraint->m_BreakErrorSq = m_BreakError;
		pEdge->AddConstraintPoint(pPoint1);
		pPoint1->m_pConstraint = revoluteConstraint;

		pEdge->AddConstraintPoint(pPoint2);
		pPoint2->m_pConstraint = sphericalConstraint;

	/*	pProxy->m_pRevoluteConstraint = revoluteConstraint;
		pProxy->m_pSphericalConstraint = sphericalConstraint;*/

		revoluteConstraint->pRoot = pProxy;
		sphericalConstraint->pRoot = pProxy;
		pProxy->AddConstraint(revoluteConstraint);
		pProxy->AddConstraint(sphericalConstraint);
		pProxy->SetConstraintType(SharedPhysics::C_Constraint::E_Type_Revolute);
		return pProxy;
	}
#endif

	void RevoluteJointDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1 = anchor1;
		m_AnchorPoint2 = anchor2;
	}


	void RevoluteJointDescriptor::DefineJointAxis(const Maths::HVec3 PCREF jointAxis1, const Maths::HVec3 PCREF jointAxis2)
	{
		m_JointAxis1=jointAxis1;
		m_JointAxis2=jointAxis2;
	}

	//Springs!!!!

#ifndef GRAVITY_EXPORTER
	SharedPhysics::C_Spring* SpringDescriptor::CreateSpring(Gravity::C_Scene* pScene, C_Body* pBodyA, C_Body* pBodyB) const
	{
		SharedPhysics::C_Spring* pSpring = pScene->CreateSpring();
		pSpring->m_LinearSpringCoefficient = m_LinearSpringCoefficient;
		pSpring->m_LinearDampingCoefficient = m_LinearDampingCoefficient;
		pSpring->m_AngularSpringCoefficient = m_AngularSpringCoefficient;
		pSpring->m_AngularDampingCoefficient = m_AngularDampingCoefficient;
		pSpring->m_RestLength = m_RestLength;
		pSpring->m_LocalContactA = m_AnchorPoint1;
		pSpring->m_LocalContactB = m_AnchorPoint2;
		pSpring->m_pRigidA = &pBodyA->GetBody();
		pSpring->m_pRigidB = &pBodyB->GetBody();
		pSpring->m_pNodeA = pBodyA->GetGraphNode();
		pSpring->m_pNodeB = pBodyB->GetGraphNode();
		pSpring->m_RotationalOffset = Maths::NearestDivide(pSpring->m_pRigidB->m_Orientation, pSpring->m_pRigidA->m_Orientation);
		return pSpring;
	}
#endif

	void SpringDescriptor::DefineSpringCoefficients(const Maths::HF32 PCREF linearSpring, const Maths::HF32 linearDamp,
		const Maths::HF32 angularSpring, const Maths::HF32 angularDamp)
	{
		m_LinearSpringCoefficient = linearSpring;
		m_LinearDampingCoefficient = linearDamp;
		m_AngularSpringCoefficient = angularSpring;
		m_AngularDampingCoefficient = angularDamp;
	}

	void SpringDescriptor::DefineRestLength(const Maths::HF32 PCREF restLength)
	{
		m_RestLength = restLength;
	}

	void SpringDescriptor::DefineAnchorPoints(const Maths::HVec3 PCREF anchor1, const Maths::HVec3 PCREF anchor2)
	{
		m_AnchorPoint1 = anchor1;
		m_AnchorPoint2 = anchor2;
	}
};


