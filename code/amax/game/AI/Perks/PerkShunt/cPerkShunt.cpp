#include "stdafx.h"
#include <Perks/PerkShunt/cPerkShunt.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <gameservertime/cgameservertime.h>
#include <GameLogic/Pickups/PickupBase.h>
#include <system/isystemai.h>

Debug::Float g_debugAIPerksShuntMinRange("AI.Perks.Shunt.MinRange", 40.f);
Debug::Float g_debugAIPerksShuntMaxReverseRange("AI.Perks.Shunt.MaxReverseRange", 100.f);
Debug::Float g_debugAIPerksShuntSplineFacingDot("AI.Perks.Shunt.SplineFacingDot", 0.8f);
Debug::Float g_debugAIPerksShuntVehicleFacingDot("AI.Perks.Shunt.VehicleFacingDot", 0.98f);

Debug::Float g_debugAIPerksShuntMissDotVehicle("AI.Perks.Shunt.MissDotVehicle", 0.006f);
Debug::Float g_debugAIPerksShuntMissDotProjectile("AI.Perks.Shunt.MissDotProjectile", 0.004f);

Debug::Float g_debugAIPerksShuntWeightingDistance("AI.Perks.Shunt.WeightingDistance", 1.f);
Debug::Float g_debugAIPerksShuntWeightingAngle("AI.Perks.Shunt.WeightingAngle", 1.f);

namespace airacingtech
{
	CPerkShunt::CPerkShunt()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;
		m_pISystem = 0;

		Reset();
	}

	CPerkShunt::~CPerkShunt()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;
		m_pISystem = 0;

		Reset();
	}

	void CPerkShunt::Reset()
	{
		for( int i = 0; i < m_VehicleEvaluationValues.Size(); i++ )
			m_VehicleEvaluationValues[i].mEval = 2.f;

		m_ProjectileEvaluationValues.Clear();
	}

	void CPerkShunt::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_SHUNT, CID_PERK_SHUNT);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_SHUNT, CPerkShunt::CreateMe, CPerkShunt::DestroyMe, CHash("PerkShunt"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_SHUNT, MT_POST_INIT_AI_SYSTEM);
	}

	IComponent *CPerkShunt::CreateMe()
	{
		return new CPerkShunt;
	}

	bool CPerkShunt::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkShunt::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkShunt::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkShunt::HandleMessage(const CMessage &msg)
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


	EComponentTypeId CPerkShunt::GetComponentTypeId(void)
	{
		return CID_PERK_SHUNT;
	}


	void CPerkShunt::CachedInterfaces()
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


		CObjectId systemID("AmaxSystemAI");
		m_pISystem = static_cast<ISystemAI *>(GlobalsComponent::GetObjectManager().QueryInterface(systemID, IID_SYSTEM_AI));


	}

	void CPerkShunt::Evaluate()
	{
		Reset();

		if (m_pIAvoidance && m_pIData && m_pIGameServerTime && m_pIPerkSkillUsage && m_pRaceLogicLayer && m_pLogicActor && m_pVehActor && m_pISystem)
		{
			EvaluateVehiclesAhead();
			EvaluateVehiclesBehind();
			EvaluateProjectiles();
		}
	}

	void CPerkShunt::EvaluateVehiclesAhead()
	{
		// We might already know our shunt target
		int shuntTargetIdx = -1;
		VehiclePickupInfo& pickupInfo = m_pLogicActor->GetPickupInfo();

		int myGroup = 0;
		
		if (m_pIPerkSkillUsage)
		{
			myGroup = m_pIPerkSkillUsage->GetGroup(m_pIData);
		}


		if(pickupInfo.mPickupStoreIndex >= 0 && pickupInfo.mPickupStoreIndex < pickupInfo.mPickupStore.Size() && 
			pickupInfo.mPickupStore[pickupInfo.mPickupStoreIndex].mType == E_PICKUP_TYPE_SHUNT)
		{
			shuntTargetIdx = pickupInfo.mCurrentTargetIdx;
		}
		else
		{
			// We don't do the LoS check, it's expensive and probably unnecessary
			int bestIdx = -1;
			float bestScore = 0.f;
			bool onSpline = false;
			bool atDistance = false;
			for( int i = 0; i < m_pRaceLogicLayer->GetNumVehicles(); i++ )
			{
				float checkAngle = m_pLogicActor->mVehicleProximities[i].mRelativeHeading;
				float score = m_pLogicActor->EvaluateShuntTarget(i, checkAngle, onSpline, atDistance);

				bool evaluate = true;
				if (m_pISystem)
				{
					IData *pIDataTarget = m_pISystem->GetData(i);
					if (pIDataTarget)
					{
						int avoidGroup = m_pIPerkSkillUsage->GetGroup(pIDataTarget);

						if (myGroup == 1 || avoidGroup == 2 || (myGroup == avoidGroup && myGroup != 0))
						{
							evaluate = false;
						}
					}
				}
			
				if(score <= 1.f && (bestIdx == -1 || score < bestScore) && evaluate)
					bestIdx = i;
			}

			shuntTargetIdx = bestIdx;
		}

		if(shuntTargetIdx != -1)
		{

			bool evaluate = true;
			if (m_pISystem)
			{
				IData *pIDataTarget = m_pISystem->GetData(shuntTargetIdx);
				if (pIDataTarget)
				{
					int avoidGroup = m_pIPerkSkillUsage->GetGroup(pIDataTarget);

					if (myGroup == 1 || avoidGroup == 2 || (myGroup == avoidGroup && myGroup != 0))
					{
						evaluate = false;
					}
				}
			}

			if (evaluate)
			{
				float lookAhead			= 0.5f; //Charge delay
				Maths::HVec3 vehDir		= m_pLogicActor->GetPositionInfo().mDirection;
				Maths::HVec3 predictPos = m_pVehActor->GetMatrix().AW();
				m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, predictPos, lookAhead);

				Game::Actors::C_VehicleActor* pTargetVehActor	= m_pLogicActor->GetGraphicalActor(shuntTargetIdx);
				Maths::HVec3 predictTargetPos					= pTargetVehActor->GetMatrix().AW();
				m_pIPerkSkillUsage->PredictVehiclePos(pTargetVehActor, predictTargetPos, lookAhead);

				float distToTarget = Maths::Distance(predictPos, predictTargetPos);
				if(distToTarget < g_debugAIPerksShuntMinRange)
				{
					// We are close so need to check angle to target
					Maths::HVec3 diff	= Maths::Normalise(predictTargetPos - predictPos);
					float dot			= Maths::DotProduct(diff, vehDir);

					if(dot >= 0.f)
					{
						float inverseDot		= 1.f - dot;
						float inverseMissDot	= 1.f - g_debugAIPerksShuntVehicleFacingDot;
						float numIncrements		= g_debugAIPerksShuntVehicleFacingDot / inverseMissDot;

						float angleEval			= inverseDot + (inverseDot * numIncrements);
						float distEval			= distToTarget / g_debugAIPerksShuntMinRange;

						float eval = (angleEval * g_debugAIPerksShuntWeightingAngle) + (distEval * g_debugAIPerksShuntWeightingDistance);

						m_VehicleEvaluationValues[shuntTargetIdx].mEval = eval;
						m_VehicleEvaluationValues[shuntTargetIdx].mFireBackwards = false;
					}
				}
				else
				{
					// Far away, so we should try to face the spline
					ISplineTracker* pSplineTracker	= m_pIData->GetSplineTracker();
					if(pSplineTracker)
					{
						const RouteInfo* pRoute	= pSplineTracker->GetCurrentRoute();
						if(pRoute && pRoute->m_pRoute && pRoute->m_in)
						{
							Maths::HVec3 splineDir;
							pRoute->m_pRoute->GetDirection(pRoute->m_index, splineDir);

							Maths::HVec3 diffVec	= vehDir - splineDir;
							float forwardDot		= Maths::DotProduct(diffVec, vehDir);

							if(forwardDot >= 0.f)
							{
								float inverseDot			= forwardDot;
								float inverseMissDot		= 1.f - g_debugAIPerksShuntSplineFacingDot;
								float numIncrements			= g_debugAIPerksShuntSplineFacingDot / inverseMissDot;

								m_VehicleEvaluationValues[shuntTargetIdx].mEval = forwardDot + (forwardDot * numIncrements);
								m_VehicleEvaluationValues[shuntTargetIdx].mFireBackwards = false;
							}
						}
					}
				}
			}
		}
	}

	void CPerkShunt::EvaluateVehiclesBehind()
	{

		AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();

		int myGroup = 0;

		if (m_pIPerkSkillUsage)
		{
			myGroup = m_pIPerkSkillUsage->GetGroup(m_pIData);
		}


		AvoidCar* pAvoidCar = m_pIAvoidance->GetAvoidCar(32); // Behind
		if(pAvoidCar && pAvoidCar->IsValid() && pAISkillz && pAISkillz->m_Shunt.m_Offensive > 0.25f)
		{

			bool evaluate = true;
			
			int avoidGroup = m_pIPerkSkillUsage->GetGroup(pAvoidCar->m_pIData);

			if (myGroup == 1 || avoidGroup == 2 || (myGroup == avoidGroup && myGroup != 0))
			{
				evaluate = false;
			}
			

			if (evaluate)
			{
				float projSpeed			= 50.f;
				Maths::HVec3 vehPos		= m_pVehActor->GetMatrix().AW();
				Maths::HVec3 vehDir		= m_pLogicActor->GetPositionInfo().mDirection;
				Maths::HVec3 predictPos = m_pVehActor->GetMatrix().AW();

				int avoidIdx									= pAvoidCar->m_pIData->GetState().m_index;
				Game::Actors::C_VehicleActor* pAvoidVehActor	= m_pLogicActor->GetGraphicalActor(avoidIdx);
				Maths::HVec3 avoidVehPos						= pAvoidVehActor->GetMatrix().AW();
				Maths::HVec3 avoidVehPredictPos					= avoidVehPos;

				float distToVehicle = Maths::Distance(vehPos, avoidVehPos);
				float lookAhead		= (distToVehicle / projSpeed);

				m_pIPerkSkillUsage->PredictVehiclePos(pAvoidVehActor, avoidVehPredictPos, lookAhead);
				m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, predictPos, lookAhead);

				float missDot	= g_debugAIPerksShuntMissDotVehicle;
				float angleEval = m_pIPerkSkillUsage->PredictWillHitTarget(predictPos, -vehDir * distToVehicle, avoidVehPredictPos, missDot);
				float eval		= angleEval;

				if(angleEval < 1.f)
				{
					float distEval  = distToVehicle / g_debugAIPerksShuntMaxReverseRange;
					eval			= (angleEval * g_debugAIPerksShuntWeightingAngle) + (distEval * g_debugAIPerksShuntWeightingDistance);
				}

				m_VehicleEvaluationValues[avoidIdx].mEval = eval;
				m_VehicleEvaluationValues[avoidIdx].mFireBackwards = true;
			}
		}
	}

	void CPerkShunt::EvaluateProjectiles()
	{
		AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();
	
		Maths::HVec3 vehPos		= m_pVehActor->GetMatrix().AW();
		Maths::HVec3 vehDir		= m_pLogicActor->GetPositionInfo().mDirection;
		float projSpeed			= 50.f;

		for( int i = 0; i < m_pIPerkSkillUsage->GetNumProjectiles(); i++ )
		{
			float eval = 2.f;
			Game::C_PickupBase* pPickup = m_pIPerkSkillUsage->GetProjectile(i);

			if(pPickup)
			{
				Game::AITrackableData& data = pPickup->GetAITrackableData();

				if(data.CanShoot(m_pVehActor->GetVehicleIndex()))
				{	
					Maths::HVec3 projPos		= pPickup->GetMatrix().AW();
					Maths::HVec3 projPredictPos	= projPos;
					float distToVehicle			= Maths::Distance(vehPos, projPredictPos);
					float lookAhead				= distToVehicle / projSpeed;

					m_pIPerkSkillUsage->PredictProjectilePos(pPickup, projPredictPos, lookAhead);

					Maths::HVec3 vehPredictPos = vehPos;
					m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, vehPredictPos, lookAhead);

					float missDot	= g_debugAIPerksShuntMissDotProjectile;
					float angleEval = m_pIPerkSkillUsage->PredictWillHitTarget(vehPredictPos, -vehDir * distToVehicle, projPredictPos, missDot * data.mRadius);
					float distEval  = distToVehicle / g_debugAIPerksShuntMaxReverseRange;
					eval			= (angleEval * g_debugAIPerksShuntWeightingAngle) + (distEval * g_debugAIPerksShuntWeightingDistance);
				
					if (pAISkillz)
					{
						if (pAISkillz->m_Shunt.m_Defensive < 0.25f)
						{	
							eval = 0.0f;
						}
						else if (pAISkillz->m_Shunt.m_Defensive < 0.5f)
						{
							if (pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
							{

							}
							else
							{
								eval = 999.0f;
							}
						}
						else if (pAISkillz->m_Shunt.m_Defensive < 0.75f)
						{
							if (pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
							{

							}
							else
							{
								eval = 999.0f;
							}
						}
						else if (pAISkillz->m_Shunt.m_Defensive < 1.0f)
						{

						}

					}
				
				}
			}

			


			m_ProjectileEvaluationValues.PushBack(eval);
		}
	}
}




