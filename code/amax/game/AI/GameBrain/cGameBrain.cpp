#include "stdafx.h"
#include <GameBrain/cGameBrain.h>
#include <data/idata.h>
#include <controlsstate/icontrolstate.h>
#include <pidcontrollerai/ipidcontrollerai.h>
#include <vehiclestate/ivehiclestate.h>
#include <vehiclelogic/ivehiclelogic.h>
#include <accelbrakecontroller/iaccelbrakecontroller.h>
#include <trackdata/itrackdata.h>
#include <avoidance/iavoidance.h>
#include <paramsai/iparamsai.h>
#include <splinefollower/isplinefollower.h>
#include <splinetracker/isplinetracker.h>
#include <speedtracker/ispeedtracker.h>
#include <debugTools/idebugai.h>
#include <recovery/irecoveryai.h>
#include <rubberbanding/irubberbandingai.h>
#include <driverai/igamebrain.h>
#include <Perks/PerkAndSkillUsage/iPerkSkillUsage.h>
#include <perks/PerkLogic/iPerkLogic.h>
#include <gameservertime/cgameservertime.h>
#include <perks/PerkSystem/iPerkSystem.h>


namespace airacingtech
{
	CGameBrain::CGameBrain()
	{
		m_pIData = 0;
		m_pIPerkSkillUsage = 0;
		m_pIPerkLogic = 0;
		m_pIPerkSystem = 0;
	}

	CGameBrain::~CGameBrain()
	{

	}

	void CGameBrain::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_GAMEBRAIN, CID_GAMEBRAIN);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_GAMEBRAIN, CGameBrain::CreateMe, CGameBrain::DestroyMe, CHash("GameBrain"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_GAMEBRAIN, MT_POST_INIT_AI_SYSTEM); 

		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_GAMEBRAIN, MT_AI_TAKE_CONTROL);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_GAMEBRAIN, MT_AI_RELINQUISH_CONTROL);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_GAMEBRAIN, MT_SET_LOCAL_PLAYER);
	}

	IComponent *CGameBrain::CreateMe()
	{
		return new CGameBrain;
	}

	bool CGameBrain::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CGameBrain::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CGameBrain::Deinit(void)
	{
		return;
	}

	EMessageResult CGameBrain::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{

			case MT_AI_TAKE_CONTROL:
				{
					if (m_pIData)
						m_pIData->GetState().m_aiInControl = true;
				}
				break;

			case MT_AI_RELINQUISH_CONTROL:
				{
					if (m_pIData)
						m_pIData->GetState().m_aiInControl = false;
				}
				break;

			case MT_SET_LOCAL_PLAYER:
				{
					if (m_pIData)
						m_pIData->GetState().m_localPlayer = *((bool*)msg.mpData);
				}
				break;

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


	EComponentTypeId CGameBrain::GetComponentTypeId(void)
	{
		return CID_GAMEBRAIN;
	}

	void CGameBrain::CachedInterfaces()
	{
		//cache pointers....
		CObjectId id = GetObjectId();

		m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));									
		m_pIPerkSkillUsage = static_cast<IPerkSkillUsage *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERKS_SKILL_USAGE));		
		m_pIPerkLogic = static_cast<IPerkLogicAI *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_PERK_LOGIC));		
		
		CObjectId perkSystemID("PerkSystemAISystem");
		m_pIPerkSystem = static_cast<IPerkSystem *>(GlobalsComponent::GetObjectManager().QueryInterface(perkSystemID, IID_PERK_SYSTEM));

	
	}

	void CGameBrain::Update()
	{
		UpdateGame();
	}

	void CGameBrain::UpdateGame()
	{

		DataState &MyDataState = m_pIData->GetState();
		
		if (MyDataState.m_startRace)
		{
			// do the required behaviour.......
			if (MyDataState.m_aiInControl)
				m_pIPerkSkillUsage->Update();

			// TODO : should really create a behaviour tree, but a simple state machine will do for now......time, time, time....
			switch(m_BrainState.m_currentBrainState)
			{
				case BrainState::BRAIN_STATE_AVOIDANCE:
					{
						IAvoidance *pAvoidance = m_pIData->GetAvoidance();
						if (pAvoidance && MyDataState.m_aiInControl)
						{
							pAvoidance->Update();
						} 

						// we can use perks.....
					}
					break;

				case BrainState::BRAIN_STATE_GO_FOR_PERK:
					{
						// we can use perks.......
					}
					break;

				case BrainState::BRAIN_STATE_BLOCKING:
					{
						// we can use perks
					}
					break;
						
				case BrainState::BRAIN_STATE_WANDER:
					{
						// we can use perks.....
					}
					break;

				case BrainState::BRAIN_STATE_NONE:
					{

					}
					break;

				default:
					{
						IAvoidance *pAvoidance = m_pIData->GetAvoidance();
						if (pAvoidance && MyDataState.m_aiInControl)
						{
							pAvoidance->Update();
						} 
					}
					break;
			}
			

			

			ISplineFollower *pSplineFollower = m_pIData->GetSplineFollower();
			if (pSplineFollower)
			{
				pSplineFollower->Update();

				IControlState *pIControl = m_pIData->GetControlState();
				if (pIControl && MyDataState.m_aiInControl)
				{
					pIControl->GetState()->SetSteering(pSplineFollower->GetSteering());
				}
			}

			// do the accel and braking component.....
			IAccelBrakeController *pIAccelBrakeController = m_pIData->GetAccelBrakeController();
			if (pIAccelBrakeController && MyDataState.m_aiInControl)
			{
				pIAccelBrakeController->Update();
			}

			// recovery to be done here ??
			// ai only.......
			if (MyDataState.m_pIRecovery && MyDataState.m_pIVehicleState && !MyDataState.m_pIVehicleState->GetState()->GetPlayer() && MyDataState.m_aiInControl)
			{
				// calculate reset to track for the ai only
				MyDataState.m_pIRecovery->Update();
			}
		}


		// this is not needed
		/*
		if (pMyDataState->m_pIControlState)
		{
			pMyDataState->m_pIControlState->GetState()->ClampAllValues();	
		}
		*/

	}




	void CGameBrain::PreUpdate()
	{
		PreUpdateGame();
	}

	void CGameBrain::PreUpdateGame()
	{
		PERF_EVENT("AI.PreUpdateGame");

		DataState &MyDataState = m_pIData->GetState();

		//if (pMyDataState)
		{
			//if (myDataState.m_pIControlState)
			if (MyDataState.m_aiInControl)
			{
				// this copies the current controls into the previous controls....
				MyDataState.m_pIControlState->PreUpdate();
			}

			//ISplineTracker *pSplineTracker = m_pIData->GetSplineTracker();
			//if (pSplineTracker)
			{
				//pSplineTracker->Update();
				MyDataState.m_pISplineTracker->Update();
			}

			//ISpeedTracker *pSpeedTracker = m_pIData->GetSpeedTracker();
			//if (pSpeedTracker)
			if (MyDataState.m_aiInControl || MyDataState.m_localPlayer)
			{
				//pSpeedTracker->Update();
				MyDataState.m_pISpeedTracker->Update();
			}

			//IVehicleLogic *pVehicleLogic = m_pIData->GetVehicleLogic();
			//if (pVehicleLogic)
			{
				//pVehicleLogic->Update();
				MyDataState.m_pVehicleLogic->Update();
			}

			// this can override the track speed.......
			//if (myDataState.m_pIRubberBanding)
			if (MyDataState.m_aiInControl)
			{
				MyDataState.m_pIRubberBanding->Update();
			}

			// must be done here.......
			//if (MyDataState.m_aiInControl)
				CalculateCommonVehicleData();


			//if (myDataState.m_pIAvoidance)
			if (MyDataState.m_aiInControl)
			{
				MyDataState.m_pIAvoidance->PreUpdate();
			}

			// these are game specific updates.
			if (MyDataState.m_aiInControl)
				m_pIPerkSkillUsage->PreUpdate();

			// this is actually going for a pickup behaviour, needs to be moved.....
			if (MyDataState.m_aiInControl)
				m_pIPerkLogic->PreUpdate();			
		}
	}
	


	void CGameBrain::PostUpdate()
	{
		// not currently being used.......
	}


	void CGameBrain::CalculateCommonVehicleData()
	{
		//PERF_EVENT("AI.Driver.CalculateCommonVehicleData");

		if (m_pIData->GetState().m_pIVehicleState)
		{
			DataState &myDataState = m_pIData->GetState();
			VehicleState *pVehicleStateMy = myDataState.m_pIVehicleState->GetState();
			assert(pVehicleStateMy);

			Maths::HVec3 vel = pVehicleStateMy->GetVelocity();
			Maths::HVec3 velN = vel;
			velN.Normalise();
			if (pVehicleStateMy->GetSpeed() < 1.0f)
			{
				velN = pVehicleStateMy->GetZaxis();
			}

			myDataState.m_slip = vel | pVehicleStateMy->GetXaxis();
			myDataState.m_slipN = velN | pVehicleStateMy->GetXaxis();

			//myDataState.m_speedfrds = vel | pVehicleStateMy->GetZaxis();
			//myDataState.m_speedmps = pVehicleStateMy->GetSpeed();
			//myDataState.m_speedmph = MSTOMPH(m_pIData->GetState().m_speedmps);

			myDataState.m_angVelV = pVehicleStateMy->GetAngularVelocity();
			myDataState.m_angVelVLast = pVehicleStateMy->GetAngularVelocityLast();

			//if (m_pIData->GetSplineFollowing())
			//	myDataState.m_SteerAngle = m_pIData->GetSplineFollowing()->GetAngle();
			//else
			myDataState.m_SteerAngle = 0.0f;

			float whslipAng1,whslipAng2,whslipAng3,whslipAng4;
			float whslide1,whslide2,whslide3, whslide4;

			int noWheels =  pVehicleStateMy->GetNoWheels();

			if (noWheels == 2)
			{
				// TODO : If a bike, need to factor in the some alternative info....
				whslipAng1 = pVehicleStateMy->GetWheel(0)->GetSlipAngle();
				whslipAng2 = pVehicleStateMy->GetWheel(0)->GetSlipAngle();
				whslipAng3 = pVehicleStateMy->GetWheel(1)->GetSlipAngle();
				whslipAng4 = pVehicleStateMy->GetWheel(1)->GetSlipAngle();

				whslide1 = pVehicleStateMy->GetWheel(0)->GetSlideFactorAmax();
				whslide2 = pVehicleStateMy->GetWheel(0)->GetSlideFactorAmax();
				whslide3 = pVehicleStateMy->GetWheel(1)->GetSlideFactorAmax();
				whslide4 = pVehicleStateMy->GetWheel(1)->GetSlideFactorAmax();
			}
			else 
			{
				// assuming 4 wheels
				whslipAng1 = pVehicleStateMy->GetWheel(0)->GetSlipAngle();
				whslipAng2 = pVehicleStateMy->GetWheel(1)->GetSlipAngle();
				whslipAng3 = pVehicleStateMy->GetWheel(2)->GetSlipAngle();
				whslipAng4 = pVehicleStateMy->GetWheel(3)->GetSlipAngle();

				whslide1 = pVehicleStateMy->GetWheel(0)->GetSlideFactorAmax();
				whslide2 = pVehicleStateMy->GetWheel(1)->GetSlideFactorAmax();
				whslide3 = pVehicleStateMy->GetWheel(2)->GetSlideFactorAmax();
				whslide4 = pVehicleStateMy->GetWheel(3)->GetSlideFactorAmax();
			}

			myDataState.m_rearWheelSlideFactorLast = myDataState.m_rearWheelSlideFactor;
			myDataState.m_rearWheelSlideFactor = Maths::Abs((whslide3 + whslide4)) * 0.5f;
			myDataState.m_frontWheelSlideFactor = Maths::Abs((whslide1 + whslide2)) * 0.5f;


			myDataState.m_angVelY = pVehicleStateMy->GetYaxis() | myDataState.m_angVelV;
			myDataState.m_angVelYLast = pVehicleStateMy->GetYaxis() | myDataState.m_angVelVLast;
			myDataState.m_diffAngVel = myDataState.m_angVelY - myDataState.m_angVelYLast;

			myDataState.m_DiffTrackSpeed = pVehicleStateMy->GetSpeed() - myDataState.m_trackSpeed;


			// average the wheels slide between the rear and the the front
			myDataState.m_frontWheelSlip = (whslipAng1 + whslipAng2) * 0.5f;
			myDataState.m_rearWheelSlip = (whslipAng3 + whslipAng4) * 0.5f;

			Maths::HVec3 rightVec = pVehicleStateMy->GetXaxis();
			Maths::C_Normal velDir(vel.Normalised());
			/*Maths::HVec3 velDir =  vel;
			velDir /= velDir.Length();*/
			myDataState.m_alignedAngleDirection = asinf( Maths::ClampedAbs(rightVec | velDir, Maths::HF32(1.f)) );
			assert(_finite(myDataState.m_alignedAngleDirection));



			CalculateSteerState();
#if 0
			LOGDEBUG( g_DebugLoggerAIDriver,"Wheel1 :%f :%f\n", whslide1,whslipAng1);
			LOGDEBUG( g_DebugLoggerAIDriver,"Wheel2 :%f :%f\n", whslide2,whslipAng2);
			LOGDEBUG( g_DebugLoggerAIDriver,"Wheel3 :%f :%f\n", whslide3,whslipAng3);
			LOGDEBUG( g_DebugLoggerAIDriver,"Wheel4 :%f :%f\n", whslide4,whslipAng4);
#endif

		}
	}

	void CGameBrain::CalculateSteerState()
	{
		DataState &myDataState = m_pIData->GetState();

		if (Maths::Abs(myDataState.m_frontWheelSlip) < 0.08f && Maths::Abs(myDataState.m_rearWheelSlip) < 0.08f )
		{
			myDataState.m_SteerState = DataState::NEUTRAL;
		}
		else if (myDataState.m_frontWheelSlip > 0.0f && myDataState.m_rearWheelSlip > 0.0f)
		{
			// same sign
			if (myDataState.m_frontWheelSlip > myDataState.m_rearWheelSlip)
			{
				myDataState.m_SteerState = DataState::UNDER_STEER_SAME;
			}
			else 
			{
				myDataState.m_SteerState = DataState::OVER_STEER_SAME;
			}
		}
		else if (myDataState.m_frontWheelSlip < 0.0f && myDataState.m_rearWheelSlip < 0.0f)
		{
			if (Maths::Abs(myDataState.m_frontWheelSlip) > Maths::Abs(myDataState.m_rearWheelSlip))
			{
				myDataState.m_SteerState = DataState::UNDER_STEER_SAME;
			}
			else 
			{
				myDataState.m_SteerState = DataState::OVER_STEER_SAME;
			}
		}
		else if (myDataState.m_frontWheelSlip > 0.0f && myDataState.m_rearWheelSlip < 0.0f)
		{
			myDataState.m_SteerState = DataState::OVER_STEER_FRONT;
		}
		else if (myDataState.m_frontWheelSlip < 0.0f && myDataState.m_rearWheelSlip > 0.0f)
		{
			myDataState.m_SteerState = DataState::OVER_STEER_REAR;
		}
	}


	
}




