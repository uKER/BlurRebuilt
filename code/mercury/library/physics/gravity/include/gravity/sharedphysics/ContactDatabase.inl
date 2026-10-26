#include "gravity/sharedphysics/ContactDatabase.h"
#include "gravity/sharedphysics/Collider.h"

namespace SharedPhysics
{
	/*NOALIAS inline void C_ContactDatabase::MakeSleepyContact(C_Collider* pCollider)
	{
		//Push the contents of this C_Collider into both buffers of the sleepy portion of the contact database.
		//NB -- IT IS ASSUMED THAT THE CALLING FUNCTION HAS ALREADY REMOVED THIS COLLIDER FROM THE DYNAMIC PORTION
		//OF THE CONTACT DATABASE, OR THAT THIS CONTACT WAS NEVER IN THE DYNAMIC PORTION OF THE DATABASE

		//C_ContactStreamResult* pOldResult = (C_ContactStreamResult*)pCollider->m_StreamPtr.ReadPoint();
		C_SingleContactIterator iter(pCollider->GetIterator());

		//If not valid contact, bin it!
		if(iter.validResult())
			return;
		C_ContactStreamResult* pOldResult(iter.result());

		C_CollisionResultDB * pResult = m_SleepyManager.GetFreeCollisionResult();
		C_CollisionResultDBReverse * pReverse = m_SleepyManager.GetFreeCollisionResultRv();
		pReverse->m_pResult = pResult;

		pResult->m_WitnessPointLocalA = pCollider->m_WitnessPointA;
		pResult->m_WitnessPointLocalB = pCollider->m_WitnessPointB;
		pResult->m_pNodeA = pCollider->NodeA();
		pResult->m_pNodeB = pCollider->NodeB();

		
		pResult->m_NumPatches = pOldResult->m_NumElements;
		// = (C_ContactStreamPatch*)(pOldResult + 1);
		U8* patchBase = (U8*)(pOldResult + 1);
		//for(U32 a = 0; a < pOldResult->m_NumElements; ++a)
		while(iter.hasNextPatch())
		{
			C_ContactStreamPatch* pOldPatch(iter.nextPatch());
			//if(pOldPatch->m_ValidData == E_ValidData)
			{
				C_ContactPatchDB* pPatch = m_SleepyManager.GetFreeCollisionPatch();
				pPatch->m_Normal = pOldPatch->m_Normal;
				pPatch->m_NumContactPoints = pOldPatch->m_NumElements;

				//Loop through all contacts, then move on to next contact...
				//C_ContactStreamPoint* pOldPoint = (C_ContactStreamPoint*)(pOldPatch + 1);
				//for(U32 b = 0; b < pOldPatch->m_NumElements; ++b)
				while(iter.hasNextPoint())
				{
					C_ContactStreamPoint* pOldPoint(iter.nextPoint());
					if(pOldPoint->m_ValidData == E_ValidData)
					{
						C_ContactPointDB* pPoint = m_SleepyManager.GetFreeCollisionPoint();
						pPoint->m_ContactPoint = pOldPoint->m_ContactPoint;
						pPoint->m_PenetrationDepth = pOldPoint->m_PenetrationDepth;
						pPoint->m_WarmstartForces.v[0] = pOldPoint->m_pConstraint->m_pConstraint->m_WarmStartValues[0];
						pPoint->m_WarmstartForces.v[1] = pOldPoint->m_pConstraint->m_pConstraint->m_WarmStartValues[1];
						pPoint->m_WarmstartForces.v[2] = pOldPoint->m_pConstraint->m_pConstraint->m_WarmStartValues[2];
						pPoint->m_MaterialIndices = pOldPoint->m_MaterialIndices;
						pPoint->m_HashValue = pOldPoint->m_HashValue;
						pPatch->AddContactPoint(pPoint);
					}
				}

				//Add the patch to the result...
				pResult->AddContactPatch(pPatch);
			}
			//patchBase += pOldPatch->m_OffsetToEnd;
			//pOldPatch = (C_ContactStreamPatch*)patchBase;
		}

		//Now insert the results into their respective lists...

		C_ResultPtr ptr(pResult);
		C_ResultRvPtr rvPtr(pReverse);
		m_SleepyContactArray.Insert(&ptr);
		m_SleepyContactRvArray.Insert(&rvPtr);
	}*/

	//NOALIAS inline void C_ContactDatabase::SumKineticEnergies()
	//{
	//	//Loop through all contacts in this database, checking that they are valid. If they are valid, then 
	//	const U32 numContacts = m_pCurrentBuffer->NumColliders();

	//	for(U32 a = 0; a < numContacts; ++a)
	//	{
	//		C_Collider* pCollider = m_pCurrentBuffer->Collider(a);
	//		C_SingleContactIterator pIter(pCollider->GetIterator());
	//		if((pIter.validResult()) & (pCollider->m_pNodeB->TestNodeFlags(C_GraphNode::E_Type_Sleeping)))
	//		{
	//			pCollider->m_pNodeB->m_pRigidBody->m_KineticEnergy += pCollider->m_pNodeA->m_pRigidBody->m_KineticEnergy;
	//		}	
	//	}
	//}

	NOALIAS inline void C_ContactDatabase::SumKineticEnergies(SharedPhysics::C_ContactStream& clothStream)
	{		
		//Loop through all contacts in this database, checking that they are valid. If they are valid, then 
		const U32 numContacts = m_pCurrentBuffer->NumColliders();
		const Maths::HVec3 bigNumber(SharedPhysics::Constants::BigNumber());
		//SharedPhysics::C_MemoryIterator<C_Collider, 64> iter = m_pCurrentBuffer->GetIterator();

		for(U32 a = 0; a < numContacts; ++a)
		{
			C_Collider* pCollider = m_pCurrentBuffer->GetCollider(a);
			C_SingleContactIterator pIter(pCollider->GetIterator());
			if(pIter.validResult())
			{
				if(pCollider->m_pNodeB->TestNodeFlags(C_GraphNode::E_Type_Sleeping_Or_Static))
				{
					pCollider->m_pNodeB->m_pRigidBody->m_KineticEnergy += pCollider->m_pNodeA->m_pRigidBody->m_KineticEnergy;
				}
				else if (pCollider->m_pNodeB->TestNodeFlags(C_GraphNode::E_Type_KinematicObject) & pCollider->m_pNodeA->TestNodeFlags(C_GraphNode::E_Type_Sleeping))
				{
					pCollider->m_pNodeA->m_pRigidBody->m_KineticEnergy = bigNumber;
				}
			}	
		}

		//Now sum up kinetic energy from cloth!!!!
		SharedPhysics::C_ParticleContactStreamIterator iter(&clothStream);
		while(iter.HasNextResult())
		{
			SharedPhysics::C_ParticleContactPair* pPair = iter.NextResult();
			if(pPair->m_pNodePtr->TestNodeFlags(C_GraphNode::E_Type_Sleeping))
			{
				pPair->m_pNodePtr->m_pRigidBody->m_KineticEnergy += pPair->m_pParticlePtr->m_pRigidBody->m_KineticEnergy;
			}
		}
				

	}

	/*NOALIAS inline void C_ContactDatabase::ReleaseSleepyContact(C_CollisionResultDB* pResult)
	{
		C_ContactPatchDB* pPatch(pResult->m_pContactPatches);
		while(pPatch)
		{
			C_ContactPointDB* pPoint(pPatch->m_pContactPoints);
			while(pPoint)
			{
				C_ContactPointDB* pNextPoint(pPoint->m_pNext);
				m_SleepyManager.PutToFree(pPoint);
				pPoint = pNextPoint;
			}
			C_ContactPatchDB* pNextPatch(pPatch->m_pNext);
			m_SleepyManager.PutToFree(pPatch);
			pPatch = pNextPatch;
		}
		S32 position;
		C_CollisionResultDBReverse rev(pResult);
		C_ResultRvPtr ptr(&rev);
		S32 pos = m_SleepyContactRvArray.FindIndex(&ptr, position);
		if(pos >= 0)
		{
			C_CollisionResultDBReverse * pRev(m_SleepyContactRvArray[pos].m_pElement);
			m_SleepyManager.PutToFree(pRev);
			m_SleepyContactRvArray.Remove(pos, 1);
		}
		m_SleepyManager.PutToFree(pResult);
		
	}

	NOALIAS inline void C_ContactDatabase::ReleaseSleepyContact(C_CollisionResultDBReverse* pResultRv)
	{
		C_CollisionResultDB* pResult(pResultRv->m_pResult);
		C_ContactPatchDB* pPatch(pResult->m_pContactPatches);
		while(pPatch)
		{
			C_ContactPointDB* pPoint(pPatch->m_pContactPoints);
			while(pPoint)
			{
				C_ContactPointDB* pNextPoint(pPoint->m_pNext);
				m_SleepyManager.PutToFree(pPoint);
				pPoint = pNextPoint;
			}
			C_ContactPatchDB* pNextPatch(pPatch->m_pNext);
			m_SleepyManager.PutToFree(pPatch);
			pPatch = pNextPatch;
		}
		C_ResultPtr ptr(pResult);
		m_SleepyContactArray.Remove(&ptr);
		m_SleepyManager.PutToFree(pResult);
		m_SleepyManager.PutToFree(pResultRv);
	}*/
};
