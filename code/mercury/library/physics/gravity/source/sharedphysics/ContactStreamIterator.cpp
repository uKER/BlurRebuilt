#include "stdafx.h"
#include "gravity/sharedphysics/ContactStreamIterator.h"
#include "gravity/sharedphysics/ContactStream.h"
#include "gravity/solver2/constraintgraphmanager.h"
#include "gravity/sharedphysics/RigidBody.h"
#include "gravity/sdk/Body.h"
#include "gravity/sdk/Scene.h"
#include <system/perf.h>

namespace SharedPhysics
{

	C_ContactStream::~C_ContactStream()
	{
#ifndef PLATFORM_SPU

		if(m_pStream)
			BizAlignedFree(m_pStream);

		m_pStream = NULL;
		m_pCurrentWritePoint = NULL;
		m_pStreamEnd = NULL;
#endif
	}


	C_ContactStreamIterator::C_ContactStreamIterator(C_ContactStream* pStream, Solver2::C_ConstraintGraphManager* pManager, const Maths::HF32 PCREF timeStep) : m_pStream(pStream), 
		m_pCurrentPointer(pStream->m_pStream), m_pCurrentResult(NULL), 
		m_pCurrentPatch(NULL), m_pCurrentPoint(NULL), m_pNextPatch(NULL),
		m_pNextPoint(NULL), m_pManager(pManager), m_PatchCount(0), m_PointCount(0),
		m_Dt(timeStep)
	{
	}

	C_ContactStreamIterator::C_ContactStreamIterator(C_ConstContactStreamPtr* pStream, Solver2::C_ConstraintGraphManager* pManager, const Maths::HF32 PCREF timeStep) : m_pStream(pStream->m_pStream),
		m_pCurrentPointer(pStream->m_pCurrentReadPoint), m_pCurrentResult(NULL), m_pCurrentPatch(NULL),
		m_pCurrentPoint(NULL), m_pNextPatch(NULL),
		m_pNextPoint(NULL), m_pManager(pManager), m_PatchCount(0), m_PointCount(0), m_Dt(timeStep)
	{
	}

	C_ContactStreamIterator::C_ContactStreamIterator(const C_ContactStreamIterator& iter) : m_pStream(iter.m_pStream),
		m_pCurrentPointer(iter.m_pStream->m_pStream), m_pCurrentResult(iter.m_pCurrentResult), m_pCurrentPatch(iter.m_pCurrentPatch),
		m_pCurrentPoint(iter.m_pCurrentPoint), m_pNextPatch(iter.m_pNextPatch),
		m_pNextPoint(iter.m_pNextPoint), m_pManager(iter.m_pManager), m_PatchCount(iter.m_PatchCount), m_PointCount(iter.m_PointCount), m_Dt(iter.m_Dt)
	{
	}

	C_ContactStreamIterator& C_ContactStreamIterator::operator = (const C_ContactStreamIterator& iter)
	{
		m_pStream = iter.m_pStream;
		m_pCurrentPointer = iter.m_pStream->m_pStream;
		m_pCurrentResult = iter.m_pCurrentResult; 
		m_pCurrentPatch = iter.m_pCurrentPatch;
		m_pCurrentPoint = iter.m_pCurrentPoint; 
		m_pNextPatch = iter.m_pNextPatch;
		m_pNextPoint = iter.m_pNextPoint; 
		m_pManager = iter.m_pManager; 
		m_PatchCount = iter.m_PatchCount; 
		m_PointCount = iter.m_PointCount; 
		m_Dt = iter.m_Dt;

		return *this;
	}


	void C_ContactStreamIterator::NormaliseContacts(const Maths::HF32 PCREF maxRelativeMass, 
		const Maths::HF32 PCREF assumedInfiniteRelativeMass)
	{
		//Work out if an object is greater than assumedInfiniteRelativeMass...

		SharedPhysics::C_RigidBody* pRigidA = m_pCurrentResult->m_pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = m_pCurrentResult->m_pNodeB->m_pRigidBody;

		if(!m_pCurrentResult->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | 
			SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			const Maths::HF32 massA(pRigidA->Mass());
			const Maths::HF32 massB(pRigidB->Mass());
			//Test to see if we need to do mass scaling!
			const Maths::HF32 bigMass(Maths::Max(massA, massB));
			const Maths::HF32 smallMass(Maths::Min(massA, massB));

			const Maths::HF32 scale(bigMass/smallMass);

			if(scale > assumedInfiniteRelativeMass)
			{
				//Need to set the larger one to zero!
				if(massA > massB)
				{
					m_pCurrentResult->m_ObjectAMassScale = 0.f;
					m_pCurrentResult->m_ObjectAInertiaScale = 0.f;
				}
				else
				{
					m_pCurrentResult->m_ObjectBMassScale = 0.f;
					m_pCurrentResult->m_ObjectBInertiaScale = 0.f;
				}
			}
			else if(scale > maxRelativeMass)
			{
				const Maths::HF32 newScale(scale/maxRelativeMass);
				if(massA > massB)
				{
					Maths::Store(newScale, &m_pCurrentResult->m_ObjectAMassScale);
					Maths::Store(newScale, &m_pCurrentResult->m_ObjectAInertiaScale);

				}
				else
				{
					Maths::Store(newScale, &m_pCurrentResult->m_ObjectBMassScale);
					Maths::Store(newScale, &m_pCurrentResult->m_ObjectBInertiaScale);
				}
			}
		}
	}


	void C_ContactStreamIterator::NormaliseContacts(const Maths::HF32 PCREF assumedInfiniteRelativeMass)
	{
		//Work out if an object is greater than assumedInfiniteRelativeMass...
		SharedPhysics::C_RigidBody* pRigidA = m_pCurrentResult->m_pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = m_pCurrentResult->m_pNodeB->m_pRigidBody;

		if(!m_pCurrentResult->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_Static | 
			SharedPhysics::C_GraphNode::E_Type_KinematicObject))
		{
			const Maths::HF32 massA(pRigidA->Mass());
			const Maths::HF32 massB(pRigidB->Mass());
			//Test to see if we need to do mass scaling!
			const Maths::HF32 bigMass(Maths::Max(massA, massB));
			const Maths::HF32 smallMass(Maths::Min(massA, massB));

			const Maths::HF32 scale(bigMass/smallMass);

			if(scale > assumedInfiniteRelativeMass)
			{
				//Need to set the larger one to zero!
				if(massA > massB)
				{
					m_pCurrentResult->m_ObjectAMassScale = 0.f;
					m_pCurrentResult->m_ObjectAInertiaScale = 0.f;
				}
				else
				{
					m_pCurrentResult->m_ObjectBMassScale = 0.f;
					m_pCurrentResult->m_ObjectBInertiaScale = 0.f;
				}
			}
		}
	}


	void C_ContactStreamIterator::DampBodies(const Maths::HF32 PCREF linearDamp, const Maths::HF32 PCREF angularDamp)
	{
		m_pCurrentResult->m_pNodeA->m_pRigidBody->Damp(linearDamp, angularDamp);
		if(!m_pCurrentResult->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_ZeroInertia))
		{
			m_pCurrentResult->m_pNodeB->m_pRigidBody->Damp(linearDamp, angularDamp);
		}
	}

	void C_ContactStreamIterator::AbsorbEnergy(const Maths::HF32 PCREF damping)
	{
		PERF_EVENT("Gravity.Callback.AbsorbEnergy");
		using namespace Maths;
		//This occurs on a per-contact basis

		const HF32 zero(Maths::Constants<HF32>::Zero());
		SharedPhysics::C_RigidBody* pRigidA = m_pCurrentResult->m_pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = m_pCurrentResult->m_pNodeB->m_pRigidBody;

		const HVec3 normal = Load<HVec3, MathsStorage::Float3>(&m_pCurrentPatch->m_Normal);
		const HVec3 contactPoint = Load<HVec3, MathsStorage::Float3>(&m_pCurrentPoint->m_ContactPoint);

		//const HVec3 pointVelocityA(pRigidA->m_LinearVelocity + ((pRigidA->m_CenterOfMass - contactPoint) ^ pRigidA->m_AngularVelocity));
		//const HVec3 pointVelocityB(pRigidB->m_LinearVelocity + ((pRigidB->m_CenterOfMass - contactPoint) ^ pRigidB->m_AngularVelocity));

		//const HVec3 relativeVelocity(pointVelocityA-pointVelocityB);
		//const Maths::HF32 nDV(relativeVelocity | normal);
		//const HVec3 normalVelocity(Maths::C_Normal(relativeVelocity) * nDV);

		////Normal velocity should be pointing "from A to B"

		//const Maths::HF32 normalVelocityA(Maths::Max(pointVelocityA | normal, zero));
		//const Maths::HF32 normalVelocityB(Maths::Max(-(pointVelocityB | normal), zero));

		//const Maths::HF32 normalVelocityRecipMassA(normalVelocityA * pRigidA->m_ReciprocalMass);
		//Maths::HF32 normalVelocityRecipMassB(normalVelocityB * pRigidB->m_ReciprocalMass);
		//if(m_pCurrentResult->m_pNodeB->HasInfiniteMass())
		//{
		//	normalVelocityRecipMassB = zero;
		//}

		//const Maths::HF32 sumNormalRecipMasses(Maths::Reciprocal(normalVelocityRecipMassA + normalVelocityRecipMassB));

		//const Maths::HF32 bodyALossProportion(normalVelocityRecipMassA * sumNormalRecipMasses);
		//const Maths::HF32 bodyBLossProportion(normalVelocityRecipMassB * sumNormalRecipMasses);

		//const Maths::HVec3 bodyAImpulse(-normalVelocity * bodyALossProportion * damping);
		//const Maths::HVec3 bodyBImpulse(normalVelocity * bodyBLossProportion * damping);

		//pRigidA->ApplyImpulseWorldSpace(bodyAImpulse * pRigidA->m_Mass, contactPoint);
		//pRigidB->ApplyImpulseWorldSpace(bodyBImpulse * pRigidB->m_Mass, contactPoint);

		//const HVec3 contactA = pRigidA->m_CenterOfMass + m_LocalContactA * m_pRigidA->m_OrientationMatrix;
		//const HVec3 contactB = pRigidB->m_CenterOfMass + m_LocalContactB * m_pRigidB->m_OrientationMatrix;

		const HVec3 velocityA(pRigidA->PointVelocity(contactPoint));
		const HVec3 velocityB(pRigidB->PointVelocity(contactPoint));
		const HVec3 relativeV(velocityA - velocityB);
		const HVec3 normVelV(Maths::Max((relativeV | normal), zero) * normal);

		const HVec3 impulse(normVelV*m_Dt*damping);

		pRigidA->ApplyImpulseWorldSpace(-impulse, contactPoint);
		if(m_pCurrentResult->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Static))
		{
			pRigidB->ApplyImpulseWorldSpace(impulse, contactPoint);
		}
	}

	void C_ContactStreamIterator::SetMassRatioA(const F32 massRatio)
	{
		m_pCurrentResult->m_ObjectAMassScale = massRatio;
		m_pCurrentResult->m_ObjectAInertiaScale = massRatio;
	}
	void C_ContactStreamIterator::SetMassRatioB(const F32 massRatio)
	{
		m_pCurrentResult->m_ObjectBMassScale = massRatio;
		m_pCurrentResult->m_ObjectBInertiaScale = massRatio;
	}

	void C_ContactStreamIterator::SetInertiaRatioA(const F32 ratio)
	{
		m_pCurrentResult->m_ObjectAInertiaScale = ratio;
	}

	void C_ContactStreamIterator::SetInertiaRatioB(const F32 ratio)
	{
		m_pCurrentResult->m_ObjectBInertiaScale = ratio;

	}


	void C_ContactStreamIterator::GetImpulseAndFriction(Maths::HF32& impluse, Maths::HVec3& friction, bool forceBodyBActive)
	{
		using namespace Maths;

		const HF32 time(m_Dt);
		const HF32 one(Maths::Constants<HF32>::One());
		SharedPhysics::C_RigidBody* pRigidA = m_pCurrentResult->m_pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = m_pCurrentResult->m_pNodeB->m_pRigidBody;

		HVec3 normal = Load<HVec3, MathsStorage::Float3>(&m_pCurrentPatch->m_Normal);

		//The normal is pointed from bodyA to bodyB
		
		//Should this be point velocity?
		const HVec3 vel(pRigidB->m_LinearVelocity - pRigidA->m_LinearVelocity);
		const HF32 relVel( vel | normal);

		HVec3 contact = Load<HVec3, MathsStorage::Float3>(&m_pCurrentPoint->m_ContactPoint);
		const HVec3 ra(contact - pRigidA->m_CenterOfMass);
		const HVec3 rb(contact - pRigidB->m_CenterOfMass);
		const HVec3 ran( normal ^ ra);
		const HVec3 rbn( normal ^ rb);

		bool bStatic(m_pCurrentResult->m_pNodeB->TestNodeFlags(SharedPhysics::C_GraphNode::E_Type_KinematicObject | SharedPhysics::C_GraphNode::E_Type_Static));
		HMat33 inverseInertiaB(pRigidB->m_InverseInertiaTensor);
		HF32 inverseMassB(pRigidB->ReciprocalMass());
		if(bStatic && !forceBodyBActive)
		{
			inverseInertiaB = Maths::Constants<Maths::HMat33>::Zero();
			inverseMassB = Maths::Constants<Maths::HF32>::Zero();
		}
		SharedPhysics::C_MaterialPair& pair = m_pManager->GetMaterialPair(m_pCurrentPoint->IndexA(), m_pCurrentPoint->IndexB());
		const HF32 restitutionCoefficient(Load<HF32>(&pair.m_RestitutionCoefficient));
		const HF32 frictionCoefficient(Load<HF32>(&pair.m_Friction[0]));


		const HVec3 ranInvert(ran * pRigidA->m_InverseInertiaTensor);
		const HVec3 rbnInvert(rbn * inverseInertiaB);
		const HVec3 a( ra ^ ranInvert);
		const HVec3 b( rb ^ rbnInvert);

		const HF32 t1(normal|a);
		const HF32 t2(normal|b);

		const HF32 recipTime = Reciprocal(time);
		const HF32 recipDenominator = Reciprocal(pRigidA->ReciprocalMass() + inverseMassB + t1 + t2);
		const HF32 numerator = - relVel *(restitutionCoefficient + one);

		impluse = numerator*recipDenominator;
		const HF32 frictionForce = (impluse * recipTime)*frictionCoefficient;
		//Now work out the direction of friction
		const HVec3 fricVelDir(vel - normal * relVel);
		//const Maths::HF32 velMag(fricVelDir.Length()); 
		const HF32 recipVelMag(fricVelDir.ReciprocalLength()); 
		const C_Normal fricDir(fricVelDir*recipVelMag, C_Normal::UNIT_LENGTH_VECTOR);
		friction = -fricDir * frictionForce;		
	}

	C_ContactType C_ContactStreamIterator::GetContactType()
	{
		using namespace Maths;

		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());
		SharedPhysics::C_RigidBody* pRigidA = m_pCurrentResult->m_pNodeA->m_pRigidBody;
		SharedPhysics::C_RigidBody* pRigidB = m_pCurrentResult->m_pNodeB->m_pRigidBody;

		const HVec3 contact = Load<HVec3, MathsStorage::Float3>(&m_pCurrentPoint->m_ContactPoint);
		const HVec3 ra = contact - pRigidA->m_CenterOfMass;
		const HVec3 rb = contact - pRigidB->m_CenterOfMass;

		const HVec3 pa = pRigidA->m_LinearVelocity + (ra ^ pRigidA->m_AngularVelocity);
		const HVec3 pb = pRigidB->m_LinearVelocity + (rb ^ pRigidB->m_AngularVelocity);

		const HVec3 relVel = pb - pa;
		const C_Normal normal(Load<HVec3, MathsStorage::Float3>(&m_pCurrentPatch->m_Normal), C_Normal::UNIT_LENGTH_VECTOR);

		const HF32 t = relVel|normal;
		
		const HF32 eps = Maths::Constants<HF32>::Epsilon();
		if(t.GT(eps)) //seperating
		{
			return SEPERATING;
		}
		else if(t.LT(-eps)) //colliding
		{
			return COLLIDING;
		}
		else
		{
			const HF32 resVF=relVel|normal;
			const HVec3 res(resVF, resVF, resVF);

			const HVec3 temp20(relVel - res);
			const C_Normal temp2(temp20);
			const C_Normal t1(normal^temp2);
			const C_Normal t2(normal^t1);

			const C_Normal t3((t1.m_V + t2.m_V) * half, Maths::C_Normal::UNIT_LENGTH_VECTOR );
			const HF32 tangentialVel = relVel|t3;

			if(Abs(tangentialVel).GT(eps)) //Sliding
				return SLIDING;
			else
				return RESTING;
		}
	}

	bool C_ContactStreamIterator::hasNextContact()
	{
		//If there is no current result loaded, then m_pCurrentPointer MUST point to the start of the stream
		if(m_pCurrentResult)
		{
			m_pCurrentPointer = ((U8*)this->m_pCurrentResult) + m_pCurrentResult->m_OffsetToEnd;
			m_pCurrentResult = (C_ContactStreamResult*)(((U8*)this->m_pCurrentResult) + m_pCurrentResult->m_OffsetToEnd);
		}
		else
		{
			m_pCurrentResult = (C_ContactStreamResult*)m_pCurrentPointer;
		}

		
		while(m_pCurrentResult->m_ValidData == E_DeletedData) //Keep skipping while we've marked these as invalid contacts
		{
			m_pCurrentPointer += m_pCurrentResult->m_OffsetToEnd;
			m_pCurrentResult = (C_ContactStreamResult*)m_pCurrentPointer;
		}
		return m_pCurrentResult->m_ValidData == E_ValidData;
		//TODO - must make sure the contact stream writer clears the memory directly after writing a result
		//so that the data afterwards never matches to the valid/invalid flags (and therefore will be determined
		//to be garbage memory)
	}
	C_ContactStreamResult* C_ContactStreamIterator::nextContact() 
	{
		m_pNextPatch = (C_ContactStreamPatch*)(m_pCurrentResult + 1);
		m_pCurrentPatch = NULL;
		m_PatchCount = m_pCurrentResult->m_NumElements;
		return m_pCurrentResult;
	}

	bool C_ContactStreamIterator::hasNextPatch()
	{
		U8* pCurrentPointer = (U8*)m_pNextPatch;
		while(m_PatchCount > 1 && m_pNextPatch->m_ValidData == E_DeletedData)
		{
			--m_PatchCount;
			pCurrentPointer += m_pNextPatch->m_OffsetToEnd;
			m_pNextPatch = (C_ContactStreamPatch*)pCurrentPointer;
		}
		return m_PatchCount > 0 && m_pNextPatch->m_ValidData == E_ValidData;
	}

	C_ContactStreamPatch* C_ContactStreamIterator::nextPatch() 
	{
		m_pCurrentPatch = m_pNextPatch;
		U8* pCurrentPointer = (U8*)m_pNextPatch;
		m_pNextPatch = (C_ContactStreamPatch*)(pCurrentPointer + m_pNextPatch->m_OffsetToEnd);
		--m_PatchCount;
		m_PointCount = m_pCurrentPatch->m_NumElements;
		m_pNextPoint = (C_ContactStreamPoint*)(m_pCurrentPatch + 1);
		return m_pCurrentPatch;
	}

	bool C_ContactStreamIterator::hasNextPoint()
	{
		//U8* pCurrentPointer = (U8*)m_pCurrentPoint;
		while(m_PointCount > 1 && m_pNextPoint->m_ValidData == E_DeletedData)
		{
			--m_PointCount;
			//pCurrentPointer += sizeof(
			m_pNextPoint++;
		}
		return m_PointCount > 0 && m_pNextPoint->m_ValidData == E_ValidData;
	}

	C_ContactStreamPoint* C_ContactStreamIterator::nextPoint()
	{
		m_pCurrentPoint = m_pNextPoint;
		m_pNextPoint++;
		--m_PointCount;
		return m_pCurrentPoint;
	}

	C_ContactStreamPoint* C_ContactStreamIterator::currentPoint()
	{
		return m_pCurrentPoint;
	}

	void C_ContactStreamIterator::HandleTunnelEventDynamicBodies()
	{
		using namespace Maths;

		const Maths::HF32 one(Maths::Constants<Maths::HF32>::One());
		const Maths::HF32 half(Maths::Constants<Maths::HF32>::Half());

		Gravity::C_Body* pBodyA = m_pCurrentResult->m_pNodeA->m_pBody;
		Gravity::C_Body* pBodyB = m_pCurrentResult->m_pNodeB->m_pBody;

		SharedPhysics::C_RigidBody& bodyA = pBodyA->GetBody();
		SharedPhysics::C_RigidBody& bodyB = pBodyB->GetBody();
		Collision::C_Shape* pShapeA = pBodyA->GetShape();
		Collision::C_Shape* pShapeB = pBodyB->GetShape();
		Gravity::C_Scene* pScene = pBodyA->GetScene();

		const Maths::HVec3 linearVelA(bodyA.m_LinearVelocity);
		const Maths::HVec3 linearVelB(bodyB.m_LinearVelocity);

		////Reset the contact stream to the just entering the contact 
		////just in case game logic has gone and already had a look at it!
		//m_pNextPatch = (C_ContactStreamPatch*)(m_pCurrentResult + 1);
		//m_pCurrentPatch = NULL;
		//m_PatchCount = m_pCurrentResult->m_NumElements;
		//m_pCurrentPoint = NULL;

		const HVec3 transA(linearVelA * m_Dt);
		const HVec3 transB(linearVelB * m_Dt);
		Maths::HF32 lambda(Maths::UNDEFINED);
		Maths::HVec3 normal(Maths::UNDEFINED), contactPoint(Maths::UNDEFINED);

		if(pScene->ContinuousCheckBoundingBox(pShapeA, pShapeB, transA, transB, 
			lambda, normal, contactPoint))
		{
			//lambda should be the t-value for the time of impact...
			if(lambda < one)
			{
				const Maths::HVec3 posA(bodyA.m_Position - (transA * (one - lambda)));
				const Maths::HVec3 posB(bodyB.m_Position - (transB * (one - lambda)));

				bodyA.m_Position = posA;
				bodyB.m_Position = posB;
				normal = Maths::Normalise(posB - posA);

				//Now - we load up the contacts...
				if(hasNextPatch())
				{
					SharedPhysics::C_ContactStreamPatch* pPatch(nextPatch());
					Maths::Store(normal, &pPatch->m_Normal);

					if(hasNextPoint())
					{
						SharedPhysics::C_ContactStreamPoint* pPoint(nextPoint());
						Maths::Store(contactPoint, &pPoint->m_ContactPoint);
						pPoint->m_PenetrationDepth = 0.f;
					}
					while(hasNextPoint())
					{
						SharedPhysics::C_ContactStreamPoint* pPoint(nextPoint());
						pPoint->Invalidate();
					}
				}
				while(hasNextPatch())
				{
					SharedPhysics::C_ContactStreamPatch* pPatch(nextPatch());
					pPatch->Invalidate();
				}
			}
		}
		//Reset the contact stream to the just entering the contact so the 
		//game logic can process it!
		m_pNextPatch = (C_ContactStreamPatch*)(m_pCurrentResult + 1);
		m_pCurrentPatch = NULL;
		m_PatchCount = m_pCurrentResult->m_NumElements;
		m_pCurrentPoint = NULL;
#ifdef USE_PERSISTENT_MANIFOLD
		if(m_pCurrentResult->m_pCollider->m_PersistentCollider)
		{
			SharedPhysics::C_PersistentContactCollider* pCollider = (SharedPhysics::C_PersistentContactCollider*)m_pCurrentResult->m_pCollider;
			pCollider->m_PersistentManifold.m_NumContacts = 0;
		}
#endif
	}


	/*bool C_SingleContactIterator::validResult() 
	{
		return m_pCurrentResult != NULL && m_pCurrentResult->m_ValidData == E_ValidData;
	}*/

	C_ContactStreamResult*  C_SingleContactIterator::result() 
	{
		m_pCurrentPatch = (C_ContactStreamPatch*)(m_pCurrentResult + 1);
		m_PatchCount = m_pCurrentResult->m_NumElements;
		return m_pCurrentResult;
	}

	bool C_SingleContactIterator::hasNextPatch()
	{
		U8* pCurrentPointer = (U8*)m_pCurrentPatch;
		while(m_PatchCount > 1 && m_pCurrentPatch->m_ValidData == E_DeletedData)
		{
			--m_PatchCount;
			pCurrentPointer += m_pCurrentPatch->m_OffsetToEnd;
			m_pCurrentPatch = (C_ContactStreamPatch*)pCurrentPointer;
		}
		return m_PatchCount > 0 && m_pCurrentPatch->m_ValidData == E_ValidData;
	}

	C_ContactStreamPatch*  C_SingleContactIterator::nextPatch()
	{
		C_ContactStreamPatch* pPatch = m_pCurrentPatch;
		U8* pCurrentPointer = (U8*)m_pCurrentPatch;
		m_pCurrentPatch = (C_ContactStreamPatch*)(pCurrentPointer + m_pCurrentPatch->m_OffsetToEnd);
		--m_PatchCount;
		m_PointCount = pPatch->m_NumElements;
		m_pCurrentPoint = (C_ContactStreamPoint*)(pPatch + 1);
		return pPatch;
	}

	bool C_SingleContactIterator::hasNextPoint()
	{
		//U8* pCurrentPointer = (U8*)m_pCurrentPoint;
		while(m_PointCount > 1 && m_pCurrentPoint->m_ValidData == E_DeletedData)
		{
			--m_PointCount;
			//pCurrentPointer += sizeof(
			m_pCurrentPoint++;
		}
		return m_PointCount > 0 && m_pCurrentPoint->m_ValidData == E_ValidData;
	}

	C_ContactStreamPoint* C_SingleContactIterator::nextPoint()
	{
		C_ContactStreamPoint* pPoint = m_pCurrentPoint;
		m_pCurrentPoint++;
		--m_PointCount;
		return pPoint;
	}


	bool C_ParticleContactStreamIterator::HasNextResult()
	{
		if(m_pCurrentPair)
		{
			m_pCurrentPointer = ((U8*)(m_pCurrentPair+1)) + sizeof(C_ParticleContactStorage) * m_pCurrentPair->m_NumContacts;
		}
		m_pCurrentPair = (C_ParticleContactPair*)m_pCurrentPointer;
		
		while(m_pCurrentPair->m_ValidData == E_DeletedData) //Keep skipping while we've marked these as invalid contacts
		{
			m_pCurrentPointer = ((U8*)(m_pCurrentPair+1)) + m_pCurrentPair->m_NumContacts * sizeof(C_ParticleContactPair);
			m_pCurrentPair = (C_ParticleContactPair*)m_pCurrentPointer;
		}
		return m_pCurrentPair->m_ValidData == E_ValidData;
	}
	
	C_ParticleContactPair* C_ParticleContactStreamIterator::NextResult()
	{
		C_ParticleContactPair* pPair = m_pCurrentPair;
		U8* pCurrentPointer = (U8*)m_pCurrentPair;
		//m_pCurrentPair = (C_ClothContactPair*)(pCurrentPointer + pPair->m_NumContacts * sizeof(C_ClothContactStorage));
		m_ContactCount = pPair->m_NumContacts;
		m_pNextContact = (C_ParticleContactStorage*)(pPair + 1);
		return pPair;
	}


	bool C_ParticleContactStreamIterator::HasNextContact()
	{
		while(m_ContactCount > 1 && m_pNextContact->m_ValidData == E_DeletedData)
		{
			--m_ContactCount;
			//pCurrentPointer += sizeof(
			m_pNextContact++;
		}
		return m_ContactCount > 0 && m_pNextContact->m_ValidData == E_ValidData;
	}

	C_ParticleContactStorage* C_ParticleContactStreamIterator::NextContact() 
	{
		m_pCurrentContact = m_pNextContact;
		m_pNextContact++;
		--m_ContactCount;
		return m_pCurrentContact;
	}

	C_SingleContactIterator::C_SingleContactIterator(C_ConstContactStreamPtr* pStream) : m_pStream(pStream->m_pStream),
		m_pCurrentPointer(pStream->m_pCurrentReadPoint), m_pCurrentResult(NULL), m_pCurrentPatch(NULL),
		m_pCurrentPoint(NULL), m_PatchCount(0), m_PointCount(0)
	{
		m_pCurrentResult = (C_ContactStreamResult*)m_pCurrentPointer;
	}

	C_ParticleContactStreamIterator::C_ParticleContactStreamIterator(C_ContactStream* pStream) : m_pStream(pStream), 
		m_pCurrentPointer(pStream->m_pStream), m_pCurrentPair(NULL), m_pCurrentContact(NULL), m_pNextContact(NULL)
	{
	}

	C_ParticleContactStreamIterator::C_ParticleContactStreamIterator(C_ConstContactStreamPtr* pStream) : m_pStream(pStream->m_pStream),
		m_pCurrentPointer(pStream->m_pCurrentReadPoint), m_pCurrentPair(NULL), m_pCurrentContact(NULL), m_pNextContact(NULL)
	{
	}


}
