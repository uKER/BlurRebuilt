#include "stdafx.h"
#include <Perks/PerkBolt/cPerkBolt.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <Perks/PerkAndSkillUsage/iPerkSkillUsage.h>
#include <gameservertime/cgameservertime.h>

#include <GameApp.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <GameLogic\Pickups\PickupBase.h>
#include <vehiclestate/ivehiclestate.h>

Debug::Float g_debugAIPerksBoltMaxRange("AmaxAI.Perks.Bolt.MaxRange", 100.f);
Debug::Float g_debugAIPerksBoltMissDotVehicle("AmaxAI.Perks.Bolt.MissDotVehicle", 0.006f);
Debug::Float g_debugAIPerksBoltMissDotProjectile("AmaxAI.Perks.Bolt.MissDotProjectile", 0.006f);
Debug::Float g_debugAIPerksBoltWeightingDistance("AmaxAI.Perks.Bolt.WeightingDistance", 2.f);
Debug::Float g_debugAIPerksBoltWeightingAngle("AmaxAI.Perks.Bolt.WeightingAngle", 1.f);

namespace airacingtech
{
	CPerkBolt::CPerkBolt()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;

		Reset();
	}

	CPerkBolt::~CPerkBolt()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;

		Reset();
	}

	void CPerkBolt::Reset()
	{
		for( int i = 0; i < m_VehicleEvaluationValues.Size(); i++ )
			m_VehicleEvaluationValues[i].mEval = 2.f;

		m_ProjectileEvaluationValues.Clear();
	}

	void CPerkBolt::RegisterComponentType()
	{
		
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_BOLT, CID_PERK_BOLT);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_BOLT, CPerkBolt::CreateMe, CPerkBolt::DestroyMe, CHash("PerkBolt"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_BOLT, MT_POST_INIT_AI_SYSTEM);	
	}

	IComponent *CPerkBolt::CreateMe()
	{
		return new CPerkBolt;
	}

	bool CPerkBolt::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkBolt::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkBolt::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkBolt::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{

		case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;
				CachedInterfaces();	
			}
			break;

		default:
			break;
		}


		return msgRet;
	}


	EComponentTypeId CPerkBolt::GetComponentTypeId(void)
	{
		return CID_PERK_BOLT;
	}

	void CPerkBolt::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

		m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));

		m_pIPerkSkillUsage = static_cast<IPerkSkillUsage *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERKS_SKILL_USAGE));

		m_pRaceLogicLayer	= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();

		if(m_pRaceLogicLayer)
		{
			m_pLogicActor	= m_pRaceLogicLayer->GetLogicVehicleActor(m_pIData->GetState().m_index);

			if(m_pLogicActor)
			{
				m_pVehActor = m_pLogicActor->GetGraphicalActor(m_pIData->GetState().m_index);
			}
		}
	}

	void CPerkBolt::Evaluate()
	{
		Reset();

		if (m_pIAvoidance && m_pIData && m_pIGameServerTime && m_pIPerkSkillUsage && m_pRaceLogicLayer && m_pLogicActor && m_pVehActor)
		{
			Maths::HVec3 vehPos			= m_pVehActor->GetMatrix().AW();
			Maths::HVec3 vehForwardDir	= m_pVehActor->GetMatrix().AZ();
			float maxRange				= g_debugAIPerksBoltMaxRange;
			Maths::HVec3 dirToCheck;
			float speedToUse;

			float normalSpeed	= 300.f;
			float reverseSpeed	= 300.f;

			const RLPickupType* pPickup = RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BLINDFIRE);
			if(pPickup && pPickup->mNormal && pPickup->mReversed)
			{
				RLBlindfireBalancingData* pNormalBalancingData = (RLBlindfireBalancingData*)pPickup->mNormal->mBalancingData;
				RLBlindfireBalancingData* pReverseBalancingData = (RLBlindfireBalancingData*)pPickup->mReversed->mBalancingData;

				if(pNormalBalancingData)
					normalSpeed = pNormalBalancingData->mSpeed;
			
				if(pReverseBalancingData)
					reverseSpeed = pReverseBalancingData->mSpeed;
			}

			// was 0.5 and 3.0f radius, this just missed.
			// 0.5 and 2.0 hit and nearly misses
			// 0.5 and 1.0, really good
			// 0.5 and 0.5, really precise and should always be a direct hit.

			float vehicleRadiusF = 2.0f;
			const float minvehicleRadiusF = 1.0f;
			const float maxVehicleRadiusF = 7.0f;//5.0f
			const float diffF = maxVehicleRadiusF - minvehicleRadiusF;

			float vehicleRadiusB = 2.0f;
			const float minvehicleRadiusB = 1.0f;
			const float maxVehicleRadiusB = 7.0f;//5.0f; 
			const float diffB = maxVehicleRadiusB - minvehicleRadiusB;

			AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();
			
			if (pAISkillz)
			{
				vehicleRadiusF = maxVehicleRadiusF - (pAISkillz->m_Blindfire.m_Offensive * diffF);
				vehicleRadiusB = maxVehicleRadiusB - (pAISkillz->m_Blindfire.m_Defensive * diffB);
			}

			float VehicleRadiusToUse = vehicleRadiusF;

			int myGroup = m_pIPerkSkillUsage->GetGroup(m_pIData);

			for( int i = 1; i <= 32; i *= 2)
			{
				AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);
				
				if(pAvoidCar && pAvoidCar->IsValid())
				{

					int avoidGroup = m_pIPerkSkillUsage->GetGroup(pAvoidCar->m_pIData);

					bool evaluate = true;
					if (myGroup == 1 || avoidGroup == 2 || (myGroup == avoidGroup && myGroup != 0))
					{
						evaluate = false;
					}

					if (evaluate)
					{
						int avoidIdx									= pAvoidCar->m_pIData->GetState().m_index;
						Game::Actors::C_VehicleActor* pAvoidVehActor	= m_pLogicActor->GetGraphicalActor(avoidIdx);
						Game::Actors::C_VehicleLogicActor *pLogicActor	= m_pRaceLogicLayer->GetLogicVehicleActor(avoidIdx);
						if (pLogicActor && !pLogicActor->IsGhosted())
						{
							if(i <= 4) // Ahead
							{
								speedToUse = normalSpeed;
								dirToCheck = vehForwardDir;
							}
							else if(i == 32) // Behind
							{
								speedToUse = reverseSpeed;
								dirToCheck = -vehForwardDir;
								VehicleRadiusToUse = vehicleRadiusF;

								// If the skill is beginner, then no backwards firing.....
								if (pAISkillz && pAISkillz->m_Blindfire.m_Offensive < 0.25f)
									continue;
							}
							else 
								continue;

							const float lookAhead = 1.0f; 
							const float lookAhead2 = lookAhead - 0.2f; 

							Maths::HVec3 velocityAvoid = pAvoidCar->m_pIVehicleState->GetState()->GetVelocity();
							Maths::HVec3 positionAvoid = pAvoidCar->m_pIVehicleState->GetState()->GetPosition();
							Maths::HVec3 positionAvoidPred  = positionAvoid + (velocityAvoid * lookAhead2);

							Maths::HVec3 velocityBolt = dirToCheck * speedToUse;
							Maths::HVec3 positionBolt = vehPos;
							Maths::HVec3 positionBoltPred = positionBolt + (velocityBolt * lookAhead);

							float u0,u1;
							positionBolt.Y(0.0f);	positionBoltPred.Y(0.0f);	positionAvoid.Y(0.0f);	positionAvoidPred.Y(0.0f);

							bool hit = airacingtech::SphereSphereSweep(0.5f, positionBolt, positionBoltPred, VehicleRadiusToUse, positionAvoid, positionAvoidPred, u0,u1, lookAhead);

							if (u0 < 0.0f)
							{
								u0 = 1000.0f;
							}

							float eval = u0;

							int avoidIdx = pAvoidCar->m_pIData->GetState().m_index;
							m_VehicleEvaluationValues[avoidIdx].mEval = eval;
							m_VehicleEvaluationValues[avoidIdx].mFireBackwards = (i == 32);
						}
					}
				}
			}


#if 0
			for( int i = 0; i < m_pIPerkSkillUsage->GetNumProjectiles(); i++ )
			{
				Game::C_PickupBase* pPickup = m_pIPerkSkillUsage->GetProjectile(i);

				TargetEvaluation targetEval;
				targetEval.mEval = 2.f;

				if(pPickup)
				{
					Game::AITrackableData& data = pPickup->GetAITrackableData();

					if(data.CanShoot(m_pVehActor->GetVehicleIndex()))
					{
						Maths::HVec3 projPos		= pPickup->GetMatrix().AW();	
						Maths::HVec3 projPredictPos	= projPos;

						Maths::HVec3 dirToProj	= projPos - vehPos;
						float projVehDot		= Maths::DotProduct(vehForwardDir, dirToProj);

						// Can't use projected position for this as we need it to work out the projected position
						bool targetForwards = projVehDot > 0;
						if(targetForwards)
						{
							speedToUse = normalSpeed;
							dirToCheck = vehForwardDir;
						}
						else
						{
							speedToUse = reverseSpeed;
							dirToCheck = -vehForwardDir;
						}

						float distToVehicle = Maths::Distance(vehPos, projPredictPos);
						float lookAhead		= distToVehicle / speedToUse;

						m_pIPerkSkillUsage->PredictProjectilePos(pPickup, projPredictPos, lookAhead);

						Maths::HVec3 vehPredictPos = vehPos;
						m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, vehPredictPos, lookAhead);

						float missDot	= g_debugAIPerksBoltMissDotProjectile;
						float angleEval = m_pIPerkSkillUsage->PredictWillHitTarget(vehPredictPos, dirToCheck * distToVehicle, projPredictPos, missDot * data.mRadius);
						float distEval  = distToVehicle / maxRange;
						float eval		= (angleEval * g_debugAIPerksBoltWeightingAngle) + (distEval * g_debugAIPerksBoltWeightingDistance);

						if (pAISkillz && pAISkillz->m_Blindfire.m_Defensive < 0.25f)
						{
							targetEval.mEval = 999.0f;
							targetEval.mFireBackwards = false;
						}
						else if (pAISkillz && pAISkillz->m_Blindfire.m_Defensive < 0.5f)
						{
							targetEval.mEval = eval;
							targetEval.mFireBackwards = !targetForwards;
						}
						else if (pAISkillz && pAISkillz->m_Blindfire.m_Defensive < 0.75f)
						{
							targetEval.mEval = eval;
							targetEval.mFireBackwards = !targetForwards;
						}
						else 
						{
							targetEval.mEval = eval;
							targetEval.mFireBackwards = !targetForwards;
						}
					}
				}

				m_ProjectileEvaluationValues.PushBack(targetEval);
			}
#endif


			for( int i = 0; i < m_pIPerkSkillUsage->GetNumProjectiles(); i++ )
			{
				Game::C_PickupBase* pPickup = m_pIPerkSkillUsage->GetProjectile(i);

				TargetEvaluation targetEval;
				targetEval.mEval = 999.f;
				if(pPickup)
				{
					Game::AITrackableData& data = pPickup->GetAITrackableData();

					if(data.CanShoot(m_pVehActor->GetVehicleIndex()))
					{
						// use the look ahead as a skill factor ?????????
						float lookAheadTime = 2.0f; // I think there is a delay of 0.2 in the firing of a barge perk.

						Maths::HVec3 projPos		= pPickup->GetMatrix().AW();	
						Maths::HVec3 projPredictPos	= projPos;

						Maths::HVec3 dirToProj	= projPos - vehPos;
						float projVehDot		= Maths::DotProduct(vehForwardDir, dirToProj);

						// Can't use projected position for this as we need it to work out the projected position
						bool targetForwards = projVehDot > 0;
						if(targetForwards)
						{
							speedToUse = normalSpeed;
							dirToCheck = vehForwardDir;
						}
						else
						{
							speedToUse = reverseSpeed;
							dirToCheck = -vehForwardDir;
						}

						m_pIPerkSkillUsage->PredictProjectilePos(pPickup, projPredictPos, lookAheadTime);		

						Maths::HVec3 vehPos = m_pVehActor->GetMatrix().AW();

						Maths::HVec3 vehPredictPos = vehPos + ((dirToCheck * speedToUse)  * lookAheadTime);
						
						float u0,u1;

						projPos.Y(0.0f);	projPredictPos.Y(0.0f);	vehPos.Y(0.0f);	vehPredictPos.Y(0.0f);

						bool hit = airacingtech::SphereSphereSweep(0.5f, projPos, projPredictPos, vehicleRadiusB, vehPos, vehPredictPos, u0,u1, lookAheadTime);

						if (u0 < 0.0f)
						{
							u0 = 1000.0f;
						}

						float eval = u0;

						if (pAISkillz && pAISkillz->m_Blindfire.m_Defensive < 0.25f)
						{
							targetEval.mEval = 999.0f;
							targetEval.mFireBackwards = false;
						}
						else if (pAISkillz && pAISkillz->m_Blindfire.m_Defensive < 0.5f)
						{
							
							if (pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
							{
								targetEval.mEval = eval;
								targetEval.mFireBackwards = !targetForwards;
							}
							else
							{
								targetEval.mEval = 999.0f;
								targetEval.mFireBackwards = !targetForwards;
							}
							
						}
						else if (pAISkillz && pAISkillz->m_Blindfire.m_Defensive < 0.75f)
						{
							if (pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
							{
								targetEval.mEval = eval;
								targetEval.mFireBackwards = !targetForwards;
							}
							else
							{
								targetEval.mEval = 999.0f;
								targetEval.mFireBackwards = !targetForwards;
							}
						}
						else 
						{
							targetEval.mEval = eval;
							targetEval.mFireBackwards = !targetForwards;
						}

						m_ProjectileEvaluationValues.PushBack(targetEval);
					}
				}
			}
		}
	}
}




