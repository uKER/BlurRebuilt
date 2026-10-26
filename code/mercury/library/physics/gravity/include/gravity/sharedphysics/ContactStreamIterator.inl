#ifndef CONTACTSTREAMITERATOR_INL
#define CONTACTSTREAMITERATOR_INL

#include "gravity/sharedphysics/ContactStreamIterator.h"
//#include "gravity/solver2/constraintgraphmanager.h"

namespace SharedPhysics
{

	//NOALIAS inline void C_ContactStreamIterator::NormaliseContacts(const Maths::HF32 PCREF maxRelativeMass, 
	//	const Maths::HF32 PCREF assumedInfiniteRelativeMass)
	//{
	//	//Work out if an object is greater than assumedInfiniteRelativeMass...

	//	SharedPhysics::C_RigidBody* pRigidA = m_pCurrentResult->m_pNodeA->m_pRigidBody;
	//	SharedPhysics::C_RigidBody* pRigidB = m_pCurrentResult->m_pNodeB->m_pRigidBody;

	//	if(!m_pCurrentResult->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | 
	//		SharedPhysics::C_GraphNode::E_Type_KinematicObject))
	//	{
	//		//Test to see if we need to do mass scaling!
	//		const Maths::HF32 bigMass(Maths::Max(pRigidA->m_Mass, pRigidB->m_Mass));
	//		const Maths::HF32 smallMass(Maths::Min(pRigidA->m_Mass, pRigidB->m_Mass));

	//		const Maths::HF32 scale(bigMass/smallMass);

	//		if(scale > assumedInfiniteRelativeMass)
	//		{
	//			//Need to set the larger one to zero!
	//			if(pRigidA->m_Mass > pRigidB->m_Mass)
	//			{
	//				m_pCurrentResult->m_ObjectAMassScale = 0.f;
	//			}
	//			else
	//			{
	//				m_pCurrentResult->m_ObjectBMassScale = 0.f;
	//			}
	//		}
	//		else if(scale > maxRelativeMass)
	//		{
	//			const Maths::HF32 newScale(scale/maxRelativeMass);
	//			if(pRigidA->m_Mass > pRigidB->m_Mass)
	//			{
	//				Maths::Store(newScale, &m_pCurrentResult->m_ObjectAMassScale);
	//			}
	//			else
	//			{
	//				Maths::Store(newScale, &m_pCurrentResult->m_ObjectBMassScale);
	//			}
	//		}
	//	}
	//}

};

#endif

