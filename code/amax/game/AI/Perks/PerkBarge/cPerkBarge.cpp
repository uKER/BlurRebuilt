#include "stdafx.h"
#include <Perks/PerkBarge/cPerkBarge.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <ai/Perks/PerkAndSkillUsage/iPerkSkillUsage.h>
#include <gameservertime/cgameservertime.h>
#include <utilityai/utilityai.h>

#include <GameApp.h>
#include <GameLogic\Layers\RaceBookLayer.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <GameLogic\Pickups\PickupBase.h>
#include <Game/Game.h>


namespace airacingtech
{
	using namespace fuzzy_lib_two;
	
	
	static CObjectId SkillEval("SkillEval");
	static CObjectId Skill("Skill");
	static CObjectId RelativeSpeed("RelativeSpeed");
	
	// low
	static CObjectId SkillEvalLow("SkillEvalLow");
	static CObjectId SkillLow("SkillLow");
	static CObjectId RelativeSpeedLow("RelativeSpeedLow");

	// medium
	static CObjectId SkillEvalMed("SkillEvalMed");
	static CObjectId SkillMed("SkillMed");
	static CObjectId RelativeSpeedMed("RelativeSpeedMed");

	// high
	static CObjectId SkillEvalHigh("SkillEvalHigh");
	static CObjectId SkillHigh("SkillHigh");
	static CObjectId RelativeSpeedHigh("RelativeSpeedHigh");



	CPerkBarge::CPerkBarge()
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


		FuzzyVariable &skillEvalVar = m_fzPolicyEvaluateSkill.CreateFLV(SkillEval);
		FuzzyVariable &skillVar = m_fzPolicyEvaluateSkill.CreateFLV(Skill);
		FuzzyVariable &relSpeedVar = m_fzPolicyEvaluateSkill.CreateFLV(RelativeSpeed);

		FzSet SkillEvalLowSet = skillEvalVar.AddLeftShoulderSet(SkillEvalLow, -10.0f, 0.0f, 0.25f);
		FzSet SkillEvalMediumSet = skillEvalVar.AddTrapezoidSet(SkillEvalMed, 0.0f, 0.25f, 0.5f, 0.75f);
		FzSet SkillEvalHighSet = skillEvalVar.AddRightShoulderSet(SkillEvalHigh, 0.5f , 0.75f , 1.0f );

		FzSet SkillLowSet = skillVar.AddLeftShoulderSet(SkillLow, 0.0f, 0.25f, 0.5f);
		FzSet SkillMediumSet = skillVar.AddTriangularSet(SkillMed, 0.25f, 0.5f, 0.75f);
		FzSet SkillHighSet = skillVar.AddRightShoulderSet(SkillHigh, 0.5f , 0.75f , 1.0f );

		FzSet RelSpeedLowSet = relSpeedVar.AddLeftShoulderSet(RelativeSpeedLow, -20.0f, -10.0f, 0.0f);
		FzSet RelSpeedMediumSet = relSpeedVar.AddTriangularSet(RelativeSpeedMed, -10.0f, 0.0f, 10.0f);
		FzSet RelSpeedHighSet = relSpeedVar.AddRightShoulderSet(RelativeSpeedHigh, 0.0f , 10.0f , 20.0f);

		FzAND lowLow(SkillLowSet,    RelSpeedLowSet);
		m_fzPolicyEvaluateSkill.AddRule(lowLow, SkillEvalMediumSet);

		FzAND mediumLow(SkillMediumSet, RelSpeedLowSet);
		m_fzPolicyEvaluateSkill.AddRule(mediumLow, SkillEvalMediumSet); 

		FzAND highLow(SkillHighSet,   RelSpeedLowSet);
		m_fzPolicyEvaluateSkill.AddRule(highLow, SkillEvalHighSet); 

		//////

		FzAND lowMedium   (SkillLowSet,    RelSpeedMediumSet);
		m_fzPolicyEvaluateSkill.AddRule(lowMedium, SkillEvalMediumSet);

		FzAND mediumMedium(SkillMediumSet, RelSpeedMediumSet);
		m_fzPolicyEvaluateSkill.AddRule(mediumMedium, SkillEvalMediumSet); 

		FzAND highMedium  (SkillHighSet,   RelSpeedMediumSet);
		m_fzPolicyEvaluateSkill.AddRule(highMedium, SkillEvalHighSet); 

		///////

		FzAND lowHigh   (SkillLowSet,    RelSpeedHighSet);
		m_fzPolicyEvaluateSkill.AddRule(lowHigh, SkillEvalMediumSet);

		FzAND mediumHigh(SkillMediumSet, RelSpeedHighSet);
		m_fzPolicyEvaluateSkill.AddRule(mediumHigh, SkillEvalLowSet); 

		FzAND highHigh  (SkillHighSet,   RelSpeedHighSet);
		m_fzPolicyEvaluateSkill.AddRule(highHigh, SkillEvalLowSet); 



	}

	CPerkBarge::~CPerkBarge()
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

	void CPerkBarge::Reset()
	{
		for( int i = 0; i < m_VehicleEvaluationValues.Size(); i++ )
			m_VehicleEvaluationValues[i] = 1.f;

		m_ProjectileEvaluationValues.Clear();
	}

	void CPerkBarge::RegisterComponentType()
	{
		
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_BARGE, CID_PERK_BARGE);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_BARGE, CPerkBarge::CreateMe, CPerkBarge::DestroyMe, CHash("PerkBarge"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_BARGE, MT_POST_INIT_AI_SYSTEM);	
	}

	IComponent *CPerkBarge::CreateMe()
	{
		return new CPerkBarge;
	}

	bool CPerkBarge::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkBarge::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkBarge::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkBarge::HandleMessage(const CMessage &msg)
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


	EComponentTypeId CPerkBarge::GetComponentTypeId(void)
	{
		return CID_PERK_BARGE;
	}


	void CPerkBarge::CachedInterfaces()
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

		m_pLevelState = &Game::C_Game::GetInstance()->GetLevelState();
	}


	


	void CPerkBarge::Evaluate()
	{
		if (m_pIAvoidance && m_pIData && m_pIGameServerTime && m_pLogicActor && m_pLevelState && m_pVehActor)
		{
			Reset();

			float bargeRadius = m_pLevelState->mCars[m_pIData->GetState().m_index].mPickupInfo.BargeInfo().mRadius;
			float lookAheadTime = 0.2f; // 0.2 changed to 0.5

			Maths::HVec3 vehPredictPos	= m_pVehActor->GetMatrix().AW();
			m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, vehPredictPos, lookAheadTime);

			AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();

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
							Maths::HVec3 avoidVehPos = pAvoidVehActor->GetMatrix().AW();
							m_pIPerkSkillUsage->PredictVehiclePos(pAvoidVehActor, avoidVehPos, lookAheadTime);

							// across the board, only want to barge people who are moving quickly....
							if (pAvoidCar->m_trackSpeedPercentage > 50.0f)
							{
								if (pAISkillz)
								{

									m_fzPolicyEvaluateSkill.Fuzzify(Skill, pAISkillz->m_Barge.m_Offensive);
									m_fzPolicyEvaluateSkill.Fuzzify(RelativeSpeed, pAvoidCar->m_relativeSpeed);
									float dfzSkillEval = m_fzPolicyEvaluateSkill.DeFuzzify(SkillEval);

									// less effective when using the barge....
									if (pAISkillz->m_Barge.m_Offensive < 0.25f)
									{
										bargeRadius += airacingtech::RandomInRange(8.0f, 18.0f);
									}
									else if (pAISkillz->m_Barge.m_Offensive < 0.5f)
									{
										bargeRadius += airacingtech::RandomInRange(5.0f, 12.0f);
									}
									else if (pAISkillz->m_Barge.m_Offensive < 0.75f)
									{
										bargeRadius += airacingtech::RandomInRange(2.0f, 5.0f);
									}
									else if (pAISkillz->m_Barge.m_Offensive < 1.0f)
									{

									}

									float distToVehicle = Maths::Distance(vehPredictPos, avoidVehPos);
									float eval = (distToVehicle / bargeRadius);

									if (pAISkillz->m_Barge.m_Offensive < 0.25f &&
										pAvoidCar->m_pIData->GetState().m_usingNitro == true)	
									{
										eval = 1000.0f;
									}

									if (eval < 0.0f)
									{
										eval = 1000.0f;
									}


									if (eval >= 0.0f && eval <= 1.0f)
									{
										eval *= dfzSkillEval;
									}


									m_VehicleEvaluationValues[avoidIdx] = eval;

								}
								else
								{
									float distToVehicle = Maths::Distance(vehPredictPos, avoidVehPos);
									float eval = distToVehicle / bargeRadius;
									m_VehicleEvaluationValues[avoidIdx] = eval;
								}
							}
						}
					}	
				}
			}

			for( int i = 0; i < m_pIPerkSkillUsage->GetNumProjectiles(); i++ )
			{
				Game::C_PickupBase* pPickup = m_pIPerkSkillUsage->GetProjectile(i);

				float eval = 2.f;
				if(pPickup)
				{
					// use the look ahead as a skill factor ?????????
					lookAheadTime = 2.0f; // I think there is a delay of 0.2 in the firing of a barge perk.

					Maths::HVec3 projPos		= pPickup->GetMatrix().AW();	
					Maths::HVec3 projPredictPos	= projPos;

					
					m_pIPerkSkillUsage->PredictProjectilePos(pPickup, projPredictPos, lookAheadTime);		
					
					Maths::HVec3 vehPos = m_pVehActor->GetMatrix().AW();

					vehPredictPos = vehPos;
					m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, vehPredictPos, lookAheadTime);
			
					float u0,u1;

					projPos.Y(0.0f);	projPredictPos.Y(0.0f);	vehPos.Y(0.0f);	vehPredictPos.Y(0.0f);

					bool hit = airacingtech::SphereSphereSweep(0.5f, projPos, projPredictPos, 4.0f, vehPos, vehPredictPos, u0,u1, lookAheadTime);
					
					if (u0 < 0.0f)
					{
						u0 = 1000.0f;
					}

					float eval = u0;

					if (pAISkillz)
					{
						if (pAISkillz->m_Barge.m_Defensive < 0.25f)
						{
							eval = 999.0f;
						}
						else if (pAISkillz->m_Barge.m_Defensive < 0.5f)
						{
							if (pPickup->GetType() != E_PICKUP_TYPE_SHUNT)
							{
								eval = 999.0f;
							}
							
						}
						else if (pAISkillz->m_Barge.m_Defensive < 0.75f)
						{
							if (pPickup->GetType() != E_PICKUP_TYPE_SHUNT)
							{
								eval = 999.0f;
							}
						}
						else 
						{

						}
					}

					m_ProjectileEvaluationValues.PushBack(eval);
					
				}
				
			}
		}
	}
}




