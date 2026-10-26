#include "stdafx.h"
#include <Perks/PerkShield/cPerkShield.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <gameservertime/cgameservertime.h>

Debug::Float g_debugAIPerksShieldBargeRangeExtension("AI.Perks.Shield.BargeRangeExtension", 1.2f);
Debug::Float g_debugAIPerksShieldMissDot("AI.Perks.Shield.MissDot", 0.006f);

Debug::Float g_debugAIPerksShieldMaxRangeStatic("AI.Perks.Shield.MaxRangeStatic", 15.f);
Debug::Float g_debugAIPerksShieldMaxRangeDynamic("AI.Perks.Shield.MaxRangeDynamic", 8.f);

Debug::Float g_debugAIPerksShieldWeightingDistance("AI.Perks.Shield.WeightingDistance", 0.5f);
Debug::Float g_debugAIPerksShieldWeightingAngle("AI.Perks.Shield.WeightingAngle", 0.5f);


namespace airacingtech
{
	CPerkShield::CPerkShield()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;
		m_pLevelState = 0;

		Reset();
	}

	CPerkShield::~CPerkShield()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;
		m_pLevelState = 0;

		Reset();
	}

	void CPerkShield::Reset()
	{
		for( int i = 0; i < m_VehicleEvaluationValues.Size(); i++ )
			m_VehicleEvaluationValues[i] = 1.f;

		m_ProjectileEvaluationValues.Clear();
	}

	void CPerkShield::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_SHIELD, CID_PERK_SHIELD);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_SHIELD, CPerkShield::CreateMe, CPerkShield::DestroyMe, CHash("PerkShield"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_SHIELD, MT_POST_INIT_AI_SYSTEM);
	}

	IComponent *CPerkShield::CreateMe()
	{
		return new CPerkShield;
	}

	bool CPerkShield::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkShield::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkShield::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkShield::HandleMessage(const CMessage &msg)
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


	EComponentTypeId CPerkShield::GetComponentTypeId(void)
	{
		return CID_PERK_SHIELD;
	}

	void CPerkShield::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

		m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));

		m_pIPerkSkillUsage = static_cast<IPerkSkillUsage *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERKS_SKILL_USAGE));

		m_pLevelState = &Game::C_Game::GetInstance()->GetLevelState();

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

	
	void CPerkShield::Evaluate()
	{
		if (m_pIAvoidance && m_pIData && m_pIGameServerTime && m_pIPerkSkillUsage)
		{
			Reset();

			CheckPickups();
			CheckProjectiles();
		}
	}

	void CPerkShield::CheckPickups()
	{
		float lookAheadTime = 0.2f;
		float eval = 2.f;

		Maths::HVec3 vehPredictPos	= m_pVehActor->GetMatrix().AW();
		m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, vehPredictPos, lookAheadTime);

		AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();


		for( int i = 1; i <= 32; i *= 2)
		{
			AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(i);

			if(pAvoidCar && pAvoidCar->IsValid())
			{
				int avoidIdx										= pAvoidCar->m_pIData->GetState().m_index;
				Game::Actors::C_VehicleLogicActor* pAvoidLogicActor	= m_pRaceLogicLayer->GetLogicVehicleActor(avoidIdx);
				Game::Actors::C_VehicleActor* pAvoidVehActor		= pAvoidLogicActor->GetGraphicalActor(avoidIdx);
				VehiclePickupInfo& pickupInfo						= pAvoidLogicActor->GetPickupInfo();


				if (pAISkillz && pAISkillz->m_Shield.m_Defensive < 0.25f)
				{
					if (i==32)
					{
						m_VehicleEvaluationValues[avoidIdx] = -pAvoidCar->m_splineZDist / 50.0f;
					}
					else
					{
						m_VehicleEvaluationValues[avoidIdx] = pAvoidCar->m_splineZDist / 50.0f;
					}
				}
				else
				{
					if(pickupInfo.mPickupStoreIndex >= 0 && pickupInfo.mPickupStoreIndex < pickupInfo.mPickupStore.Size())
					{
						// Maybe someday we will pre-empt some more types here.
						switch(pickupInfo.mPickupStore[pickupInfo.mPickupStoreIndex].mType)
						{
						case E_PICKUP_TYPE_BARGE:
							{
								if (pAISkillz)
								{
									if (pAISkillz->m_Shield.m_Defensive >= 0.25f)
									{
										float bargeRadius = m_pLevelState->mCars[avoidIdx].mPickupInfo.BargeInfo().mRadius * g_debugAIPerksShieldBargeRangeExtension;

										Maths::HVec3 avoidVehPos = pAvoidVehActor->GetMatrix().AW();
										m_pIPerkSkillUsage->PredictVehiclePos(pAvoidVehActor, avoidVehPos, lookAheadTime);

										float distToVehicle = Maths::Distance(vehPredictPos, avoidVehPos);
										m_VehicleEvaluationValues[avoidIdx] = distToVehicle / bargeRadius;
									}
								}
								else
								{
									float bargeRadius = m_pLevelState->mCars[avoidIdx].mPickupInfo.BargeInfo().mRadius * g_debugAIPerksShieldBargeRangeExtension;

									Maths::HVec3 avoidVehPos = pAvoidVehActor->GetMatrix().AW();
									m_pIPerkSkillUsage->PredictVehiclePos(pAvoidVehActor, avoidVehPos, lookAheadTime);

									float distToVehicle = Maths::Distance(vehPredictPos, avoidVehPos);
									m_VehicleEvaluationValues[avoidIdx] = distToVehicle / bargeRadius;
								}
							}
							break;

						case E_PICKUP_TYPE_BLINDFIRE:
							{

								if (pAISkillz)
								{
									if (pAISkillz->m_Shield.m_Defensive >= 0.5f)
									{
										// infront or behind only
										if (i==1 || i==2 || i==4)
										{
											m_VehicleEvaluationValues[avoidIdx] = pAvoidCar->m_splineZDist / 50.0f;
										}
										else
										{
											if (i==32)
											{
												m_VehicleEvaluationValues[avoidIdx] = -pAvoidCar->m_splineZDist / 50.0f;
											}
										}
									}
								}
								else
								{
									// infront or behind only
									if (i==1 || i==2 || i==4)
									{
										m_VehicleEvaluationValues[avoidIdx] = pAvoidCar->m_splineZDist / 50.0f;
									}
									else
									{
										if (i==32)
										{
											m_VehicleEvaluationValues[avoidIdx] = -pAvoidCar->m_splineZDist / 50.0f;
										}
									}
								}	
							}
							break;


						case E_PICKUP_TYPE_BOMB:
							{
								if (pAISkillz)
								{
									if (pAISkillz->m_Shield.m_Defensive >= 0.75f)
									{
										if (i==1 || i==2 || i==4)
										{
											m_VehicleEvaluationValues[avoidIdx] = -pAvoidCar->m_splineZDist / 50.0f;
										}
									}
								}
								else
								{
									if (i==1 || i==2 || i==4)
									{
										m_VehicleEvaluationValues[avoidIdx] = -pAvoidCar->m_splineZDist / 50.0f;
									}
								}


							}
							break;


						case E_PICKUP_TYPE_SHUNT:
							{
								if (pAISkillz)
								{
									if (pAISkillz->m_Shield.m_Defensive >= 0.75f)
									{
										if (i==32)
										{
											m_VehicleEvaluationValues[avoidIdx] = -pAvoidCar->m_splineZDist / 50.0f;
										}
									}
								}
								else
								{
									if (i==32)
									{
										m_VehicleEvaluationValues[avoidIdx] = -pAvoidCar->m_splineZDist / 50.0f;
									}
								}


							}
							break;

						default:
							break;

						}

					}
				}
			}
		}	
	}

	void CPerkShield::CheckProjectiles()
	{
		Maths::HVec3 vehPos = m_pVehActor->GetMatrix().AW();
		Maths::HVec3 vehVelocity = m_pVehActor->GetVehiclePhysics()->GetVelocity();

		AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();

		for( int i = 0; i < m_pIPerkSkillUsage->GetNumProjectiles(); i++ )
		{
			Game::C_PickupBase* pPickup = m_pIPerkSkillUsage->GetProjectile(i);

			if(pPickup)
			{
				Maths::HVec3 projPos	= pPickup->GetMatrix().AW();	
				float distToVehicle		= Maths::Distance(vehPos, projPos);

				float angleEval = 2.f;
				float distEval = 2.f;

				AITrackableData& data = pPickup->GetAITrackableData();

				if(data.mVelocity.Length() > 0.f)
				{
					angleEval = m_pIPerkSkillUsage->PredictWillHitTarget(projPos, data.mVelocity, vehPos, g_debugAIPerksShieldMissDot * data.mRadius);
					distEval	= distToVehicle / g_debugAIPerksShieldMaxRangeDynamic;
				}
				else
				{
					// static mines and shocks I presume ?
					angleEval = m_pIPerkSkillUsage->PredictWillHitTarget(vehPos, vehVelocity, projPos, g_debugAIPerksShieldMissDot * data.mRadius);
					distEval	= distToVehicle / g_debugAIPerksShieldMaxRangeStatic;
				}

				float eval	= (angleEval * g_debugAIPerksShieldWeightingAngle) + (distEval * g_debugAIPerksShieldWeightingDistance);


				if (pAISkillz)
				{
					if (pAISkillz->m_Shield.m_Defensive < 0.25f)
					{
						if (pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
						{

						}
						else
						{
							eval = 999.0f;
						}
						
					}
					else if (pAISkillz->m_Shield.m_Defensive < 0.5f)
					{
						if (pPickup->GetType() == E_PICKUP_TYPE_SHOCK || pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
						{

						}
						else
						{
							eval = 999.0f;
						}
					}
					else if (pAISkillz->m_Shield.m_Defensive < 0.75f)
					{
						if (pPickup->GetType() == E_PICKUP_TYPE_SHOCK || pPickup->GetType() == E_PICKUP_TYPE_SHUNT || pPickup->GetType() == E_PICKUP_TYPE_BLINDFIRE)
						{

						}
						else
						{
							eval = 999.0f;
						}
					}
					else if (pAISkillz->m_Shield.m_Defensive < 1.0f)
					{
						if (pPickup->GetType() == E_PICKUP_TYPE_SHOCK || pPickup->GetType() == E_PICKUP_TYPE_SHUNT || 
							pPickup->GetType() == E_PICKUP_TYPE_BLINDFIRE || pPickup->GetType() == E_PICKUP_TYPE_BOMB)
						{

						}
						else
						{
							eval = 999.0f;
						}
					}
				

				}


				m_ProjectileEvaluationValues.PushBack(eval);
			}
		}
	}
}




