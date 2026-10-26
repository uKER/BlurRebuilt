#include "stdafx.h"
#include <Perks/PerkMine/cPerkMine.h>
#include <data/idata.h>
#include <avoidance/iavoidance.h>
#include <gameservertime/cgameservertime.h>
#include <GameLogic/Pickups/PickupBase.h>

Debug::Float g_debugAIPerksMineOptimumWidth("AmaxAI.Perks.Mine.OptimumWidth", 10.f);
Debug::Float g_debugAIPerksMineOptimumCurve("AmaxAI.Perks.Mine.OptimumCurve", 0.007f);
Debug::Float g_debugAIPerksMineOptimumDist("AmaxAI.Perks.Mine.OptimumDist", 1.f); 
Debug::Float g_debugAIPerksMineNarrowCurvedDenominator("AmaxAI.Perks.Mine.NarrowCurvedDenominator", 4.f); 
Debug::Float g_debugAIPerksMineOptimumDropTime("AmaxAI.Perks.Mine.OptimumDropTime", 1.f); 

Debug::Float g_debugAIPerksMineOptimumOpponentDistance("AmaxAI.Perks.Mine.OptimumOpponentDistance", 30.f); 
Debug::Float g_debugAIPerksMineMissDotVehicleDrop("AmaxAI.Perks.Mine.MissDotVehicleDrop", 0.05f);

Debug::Float g_debugAIPerksMineMissDotVehicle("AmaxAI.Perks.Mine.MissDotVehicle", 0.006f);
Debug::Float g_debugAIPerksMineMissDotProjectile("AmaxAI.Perks.Mine.MissDotProjectile", 0.006f);

Debug::Float g_debugAIPerksMineWeightingDistance("AmaxAI.Perks.Mine.WeightingDistance", 0.5f);
Debug::Float g_debugAIPerksMineWeightingAngle("AmaxAI.Perks.Mine.WeightingAngle", 0.5f);

Debug::Float g_debugAIPerksMineMaxRange("AmaxAI.Perks.Mine.MaxRange", 20.f);
Debug::Float g_debugAIPerksMineMaxReverseRange("AmaxAI.Perks.Mine.MaxReverseRange", 35.f);


namespace airacingtech
{
	CPerkMine::CPerkMine()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pSplineTracker = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;
		m_TimeStamp = -1.0f;

		Reset();
	}

	CPerkMine::~CPerkMine()
	{
		m_pIData = 0;
		m_pIGameServerTime = 0;
		m_pIAvoidance = 0;
		m_pSplineTracker = 0;
		m_pIPerkSkillUsage = 0;

		m_pRaceLogicLayer = 0;
		m_pLogicActor = 0;
		m_pVehActor = 0;

		Reset();
	}

	void CPerkMine::Reset()
	{
		m_fDropEvaluation = 2.f;

		for( int i = 0; i < m_VehicleEvaluationValues.Size(); i++ )
			m_VehicleEvaluationValues[i].mEval = 2.f;

		m_ProjectileEvaluationValues.Clear();
	}

	void CPerkMine::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_PERK_MINE, CID_PERK_MINE);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_PERK_MINE, CPerkMine::CreateMe, CPerkMine::DestroyMe, CHash("PerkMine"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_PERK_MINE, MT_POST_INIT_AI_SYSTEM);	
	}

	IComponent *CPerkMine::CreateMe()
	{
		return new CPerkMine;
	}

	bool CPerkMine::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CPerkMine::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CPerkMine::Deinit(void)
	{
		return;
	}

	EMessageResult CPerkMine::HandleMessage(const CMessage &msg)
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


	EComponentTypeId CPerkMine::GetComponentTypeId(void)
	{
		return CID_PERK_MINE;
	}

	void CPerkMine::CachedInterfaces()
	{
		CObjectId id = GetObjectId();
		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

		CObjectId gameservertimeID("GameServerTimeSystem");
		m_pIGameServerTime = static_cast<IGameServerTime *>(GlobalsComponent::GetObjectManager().QueryInterface(gameservertimeID, IID_GAMESERVER_TIME));

		m_pIAvoidance = static_cast<IAvoidance *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_AVOIDANCE));

		m_pSplineTracker = static_cast<ISplineTracker *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_SPLINE_TRACKER));

		m_pIPerkSkillUsage = static_cast<IPerkSkillUsage *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERKS_SKILL_USAGE));

		m_pSplineFollower = m_pIData->GetSplineFollower();

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

	void CPerkMine::Evaluate()
	{
		if (m_pIAvoidance && m_pIData && m_pSplineTracker && m_pSplineFollower && m_pIGameServerTime)
		{
			Reset();

			float speed = GetMineSpeed();
	
			EvaluateOpponents(speed);
			EvaluateProjectiles(speed);
			EvaluateDrop();

		}
	}

	void CPerkMine::EvaluateOpponents(float speed)
	{
		Maths::HVec3 vehPos = m_pVehActor->GetMatrix().AW();


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
					if (pLogicActor && !pLogicActor->IsGhosted() && !pLogicActor->IsWreckedOrEliminated())
					{
						Maths::HVec3 avoidVehPos						= pAvoidVehActor->GetMatrix().AW();
						float eval										= 2.f;
						float lookAhead									= 0.2f; // Charge delay time
						Maths::HVec3 predictVehPos						= vehPos;

						m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, predictVehPos, lookAhead);
						m_pIPerkSkillUsage->PredictVehiclePos(pAvoidVehActor, avoidVehPos, lookAhead);

						if(i <= 4) // Ahead
						{
							float distToVehicle = Maths::Distance(predictVehPos, avoidVehPos);
							float angleEval		= m_pIPerkSkillUsage->PredictWillHitTarget(predictVehPos, m_pVehActor->GetMatrix().AZ() * distToVehicle, avoidVehPos, g_debugAIPerksMineMissDotVehicle);

							eval = angleEval;
							if(angleEval < 1.f)
							{
								float distEval  = distToVehicle / g_debugAIPerksMineMaxRange;
								eval			= (angleEval * g_debugAIPerksMineWeightingAngle) + (distEval * g_debugAIPerksMineWeightingDistance);
							}

							if (pAISkillz)
							{
								if (pAISkillz->m_Mine.m_Offensive < 0.25f)
								{
									eval = 999.0f;
								}
								else if (pAISkillz->m_Mine.m_Offensive < 0.5f)
								{

								}
								else if (pAISkillz->m_Mine.m_Offensive < 0.75f)
								{

								}
								else
								{

								}
							}
						}
						else if(i == 32) // Behind
						{
							Maths::HVec3 dir	= Maths::Normalise(predictVehPos - avoidVehPos);
							float dot			= Maths::DotProduct(dir, m_pVehActor->GetMatrix().AZ());

							float inverseDot			= 1.f - dot;
							float inverseMissDot		= 1.f - g_debugAIPerksMineMissDotVehicleDrop;
							float numIncrements			= inverseMissDot / g_debugAIPerksMineMissDotVehicle;

							float angleEval = 2.f;
							if(dot > 0.f)
								angleEval = inverseDot + (inverseDot * numIncrements);

							float dist = Maths::Distance(predictVehPos, avoidVehPos);
							float distEval = dist / g_debugAIPerksMineOptimumOpponentDistance;

							eval = (angleEval * g_debugAIPerksMineWeightingAngle) + (distEval * g_debugAIPerksMineWeightingDistance);

							if (pAISkillz)
							{
								if (pAISkillz->m_Mine.m_Offensive < 0.25f)
								{
									eval = 999.0f;
								}
							}

						}

						m_VehicleEvaluationValues[avoidIdx].mEval = eval;
						m_VehicleEvaluationValues[avoidIdx].mFireBackwards = (i != 32);
					}
				}
			}
		}
	}

	void CPerkMine::EvaluateProjectiles(float speed)
	{
		Maths::HVec3 vehPos = m_pVehActor->GetMatrix().AW();
		Maths::HVec3 vehForwardDir = m_pVehActor->GetMatrix().AZ();
		Maths::HVec3 dirToCheck;

		AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();

		for( int i = 0; i < m_pIPerkSkillUsage->GetNumProjectiles(); i++ )
		{
			Game::C_PickupBase* pPickup = m_pIPerkSkillUsage->GetProjectile(i);

			TargetEvaluation targetEval;
			targetEval.mEval = 2.f;
			targetEval.mFireBackwards = false;

			if(pPickup)
			{
				Game::AITrackableData& data = pPickup->GetAITrackableData();

				if(data.CanShoot(m_pVehActor->GetVehicleIndex()))
				{
					Maths::HVec3 projPos		= pPickup->GetMatrix().AW();	
					Maths::HVec3 projPredictPos	= projPos;

					Maths::HVec3 dirToProj	= projPos - vehPos;
					float projVehDot		= Maths::DotProduct(vehForwardDir, dirToProj);

					float lookAhead	= Maths::Distance(vehPos, projPos) / speed;

					// Can't use projected position for this as we need it to work out the projected position
					bool targetForwards = projVehDot > 0;
					bool doCheck = true;
					if(targetForwards)
					{
						dirToCheck = vehForwardDir;
						m_pIPerkSkillUsage->PredictProjectilePos(pPickup, projPredictPos, lookAhead);
					}
					else
					{
						if(data.mVelocity.Length() == 0.f)
							doCheck = false;
						else
							dirToCheck = -vehForwardDir;

					}

					if(doCheck)
					{
						Maths::HVec3 vehPredictPos = vehPos;
						m_pIPerkSkillUsage->PredictVehiclePos(m_pVehActor, vehPredictPos, lookAhead);

						float distToVehicle = Maths::Distance(vehPos, projPredictPos);
						float angleEval		= m_pIPerkSkillUsage->PredictWillHitTarget(vehPredictPos, dirToCheck, projPredictPos, g_debugAIPerksMineMissDotProjectile * data.mRadius);
						float distEval		= distToVehicle / g_debugAIPerksMineMaxReverseRange;
						float eval			= (angleEval * g_debugAIPerksMineWeightingAngle) + (distEval * g_debugAIPerksMineWeightingDistance);

					

						if (pAISkillz)
						{
							if (pAISkillz->m_Mine.m_Defensive < 0.25f)
							{
								eval = 999.0f;
							}
							else if (pAISkillz->m_Mine.m_Defensive < 0.5f)
							{
								if (pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
								{

								}
								else
								{
									eval = 999.0f;
								}
							}
							else if (pAISkillz->m_Mine.m_Defensive < 0.75f)
							{
								if (pPickup->GetType() == E_PICKUP_TYPE_SHUNT)
								{

								}
								else
								{
									eval = 999.0f;
								}
							}
							else
							{

							}

						}

						targetEval.mEval = eval;
						targetEval.mFireBackwards = targetForwards;	
					}
				}
			}

			m_ProjectileEvaluationValues.PushBack(targetEval);
		}
	}

	void CPerkMine::EvaluateDrop()
	{
		float widthProb = 2.f;
		float curveProb = 2.f;
		float dropProb = 2.f;


		AIPerkSkillzXt *pAISkillz = m_pIPerkSkillUsage->GetSkillz();
		if (pAISkillz)
		{
			float mineTimeStamp = m_pIPerkSkillUsage->GetMineTimeStamp();
			
			if (pAISkillz->m_Mine.m_Defensive < 0.25f)
			{
				float t = mineTimeStamp + 4.0f + airacingtech::RandomInRange(0.0f, 16.0f);
				if(m_pIGameServerTime->GetTime() > t)
				{
					m_fDropEvaluation = 0.01f;
				}
			}
			else if (pAISkillz->m_Mine.m_Defensive < 0.5f)
			{
				float t = mineTimeStamp + airacingtech::RandomInRange(1.0f, 8.0f);
				if(m_pIGameServerTime->GetTime() > t)
				{
					const RouteInfo* pRouteInfo = m_pSplineTracker->GetCurrentRoute();
					if(pRouteInfo && pRouteInfo->m_in)
					{
						AITrackDataRoutes* pRoute = pRouteInfo->m_pRoute;
						if(pRoute)
						{
							int idx				= pRouteInfo->m_index;
							float curvature		= Maths::Abs(pRoute->GetCurvature(idx, AITrackDataLines::TRACK_DATA_LINE_RACING));
							float width			= pRoute->GetWidth(idx);
							float distFromLine	= Maths::Abs(m_pSplineFollower->GetOffset());
							float optimumDist	= Maths::Clamp(distFromLine / g_debugAIPerksMineOptimumDist, 0.f, 2.f);

							widthProb = (width / g_debugAIPerksMineOptimumWidth) + optimumDist;

							if(curvature > 0.f)
								curveProb = (g_debugAIPerksMineOptimumCurve / curvature) + optimumDist;
						}
					}

					// Way more likely to drop on sharp corners.
					if(curveProb < 1.25f && widthProb < 1.25f)
						m_fDropEvaluation = (curveProb + widthProb) / g_debugAIPerksMineNarrowCurvedDenominator;
					else
						m_fDropEvaluation = Maths::Min(widthProb, curveProb);
				}
			}
			else if (pAISkillz->m_Mine.m_Defensive < 0.75f)
			{

				float timeSinceTouched = m_pLogicActor->GetPickupInfo().mTimeSinceTouchedLastPickup;
				if(timeSinceTouched > 0.f)
				{
					m_fDropEvaluation = Maths::Clamp( timeSinceTouched / g_debugAIPerksMineOptimumDropTime, 0.f, 2.f);
				}
			}
			else if (pAISkillz->m_Mine.m_Defensive < 1.0f)
			{
			}

		}
		else
		{
			float timeSinceTouched = m_pLogicActor->GetPickupInfo().mTimeSinceTouchedLastPickup;
			if(timeSinceTouched > 0.f)
				dropProb = Maths::Clamp( timeSinceTouched / g_debugAIPerksMineOptimumDropTime, 0.f, 2.f);

			const RouteInfo* pRouteInfo = m_pSplineTracker->GetCurrentRoute();
			if(pRouteInfo && pRouteInfo->m_in)
			{
				AITrackDataRoutes* pRoute = pRouteInfo->m_pRoute;
				if(pRoute)
				{
					int idx				= pRouteInfo->m_index;
					float curvature		= Maths::Abs(pRoute->GetCurvature(idx, AITrackDataLines::TRACK_DATA_LINE_RACING));
					float width			= pRoute->GetWidth(idx);
					float distFromLine	= Maths::Abs(m_pSplineFollower->GetOffset());
					float optimumDist	= Maths::Clamp(distFromLine / g_debugAIPerksMineOptimumDist, 0.f, 2.f);

					widthProb = (width / g_debugAIPerksMineOptimumWidth) + optimumDist;

					if(curvature > 0.f)
						curveProb = (g_debugAIPerksMineOptimumCurve / curvature) + optimumDist;
				}
			}

			// Way more likely to drop on sharp corners.
			if(curveProb < 1.25f && widthProb < 1.25f)
				m_fDropEvaluation = (curveProb + widthProb) / g_debugAIPerksMineNarrowCurvedDenominator;
			else
				m_fDropEvaluation = Maths::Min(widthProb, curveProb);

			m_fDropEvaluation = Maths::Min(m_fDropEvaluation, dropProb);
		}		
	}

	float CPerkMine::GetMineSpeed()
	{
		const RLPickupType* pPickup = RLDatabase::Instance().GetPickupType(E_PICKUP_TYPE_BLINDFIRE);
		float speed = 0.f;
		if(pPickup && pPickup->mReversed)
		{
			RLBombReverseBalancingData* pData = (RLBombReverseBalancingData*)pPickup->mReversed->mBalancingData;

			if(pData)
			{
				speed = (pData->mScaledSpeed * m_pLogicActor->GetPositionInfo().mSpeedInMph) + pData->mMinSpeed;	
			}
		}

		return speed;
	}
	
}




