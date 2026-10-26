#include "stdafx.h"
#include <driverai/cdriverai.h>
#include <driverai/igamebrain.h>
#include <controlsstate/icontrolstate.h>
#include <system/Perf/PerfEvent.h>


// NOTE : This is really the top level component for the ai driver for the game.. create your own driver component
//TODO [Darren] : Optimise....remove if`s would be the first step, assume all the data and pointers are correct..


// NOTE :	We are going to break this now, but in tiny steps, Intro, Game, Outro structure is legacy, thus we will try to change
//			without too much interuption. Leave this structure in place and touch the game first.


#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", off)
#endif
#endif

static Debug::Logger &g_DebugLoggerAIDriver = Debug::GetLogger("oldtech.ai.driver");


namespace airacingtech
{

#define REMOVED_BITSOF_CODE	1

	CDriverAI::CDriverAI() : m_pIData(0)
	{

	}

	CDriverAI::~CDriverAI()
	{

	}

	void CDriverAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_DRIVER_AI, CID_DRIVER_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_DRIVER_AI, CDriverAI::CreateMe, CDriverAI::DestroyMe, CHash("DriverAI"));

		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DRIVER_AI, MT_UPDATE_AI_DRIVER);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DRIVER_AI, MT_PRE_UPDATE_AI_DRIVER);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DRIVER_AI, MT_POST_UPDATE_AI_DRIVER); 
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DRIVER_AI, MT_POST_INIT_AI_SYSTEM); 
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DRIVER_AI, MT_START_AI_RACE);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DRIVER_AI, MT_QUICK_RESTART);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_DRIVER_AI, MT_START_INTRO);
	}

	IComponent *CDriverAI::CreateMe()
	{
		return new CDriverAI;
	}

	bool CDriverAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}


	bool CDriverAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CDriverAI::Deinit(void)
	{
		m_pIData = 0;
	}

	EMessageResult CDriverAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{

		case MT_QUICK_RESTART:
			{
				QuickRestart();
			}
			break;


		case MT_UPDATE_AI_DRIVER:
			{
				msgRet =  MR_TRUE;
				Update();
			}
			break;

		case MT_PRE_UPDATE_AI_DRIVER:
			{
				msgRet =  MR_TRUE;
				PreUpdate();
			}
			break;


		case MT_POST_UPDATE_AI_DRIVER:
			{
				msgRet =  MR_TRUE;
				PostUpdate();
			}
			break;

		case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;
				if (!m_pIData)
				{
					CObjectId id = GetObjectId();
					m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));		
				}
			}
			break;

		case MT_START_AI_RACE:
			{	
				DataState &myDataState = m_pIData->GetState();
				myDataState.m_startRace = true;
			}
			break;

		case MT_START_INTRO:
			{
				DataState &myDataState = m_pIData->GetState();
				myDataState.m_startIntro = true;
			}
			break;


		case MT_START_OUTRO:
			{
				DataState &myDataState = m_pIData->GetState();
				myDataState.m_startOutro = true;
			}
			break;
	
		default:
			break;
		}

		return msgRet;
	}


	EComponentTypeId CDriverAI::GetComponentTypeId(void)
	{
		return CID_DRIVER_AI;
	}

	void CDriverAI::QuickRestart()
	{
		DataState &myDataState = m_pIData->GetState();
		
		myDataState.m_endRace = false;
		myDataState.m_startRace = false;
	}

	void CDriverAI::UpdateIntro(DataState &myDataState)
	{
		if (myDataState.m_startIntro)
		{
			IIntroOutroSplineAI *pIntro = m_pIData->GetIntroOutroSplineAI();
			if (pIntro)
			{
				pIntro->Update();

				IControlState *pIControl = m_pIData->GetControlState();
				if (pIControl)
				{
					pIControl->GetState()->SetSteering(pIntro->GetSteering());
				}
				
				IAccelBrakeController *pIAccelBrakeController = m_pIData->GetAccelBrakeController();
				if (pIAccelBrakeController)
				{
					pIAccelBrakeController->Update();
				}

			}
		}
	}
	
	void CDriverAI::UpdateGame(DataState &myDataState)
	{
		
		if (myDataState.m_startRace)
		{
			IAvoidance *pAvoidance = m_pIData->GetAvoidance();
			if (pAvoidance)
			{
				pAvoidance->Update();
			} 

			ISplineFollower *pSplineFollower = m_pIData->GetSplineFollower();
			if (pSplineFollower)
			{
				pSplineFollower->Update();

				IControlState *pIControl = m_pIData->GetControlState();
				if (pIControl)
				{
					pIControl->GetState()->SetSteering(pSplineFollower->GetSteering());
				}
			}

			// do the accel and braking component.....
			IAccelBrakeController *pIAccelBrakeController = m_pIData->GetAccelBrakeController();
			if (pIAccelBrakeController)
			{
				pIAccelBrakeController->Update();
			}

			// recovery to be done here ??
			// ai only.......
			if (myDataState.m_pIRecovery && myDataState.m_pIVehicleState && !myDataState.m_pIVehicleState->GetState()->GetPlayer())
			{
				// calculate reset to track for the ai only
				myDataState.m_pIRecovery->Update();
			}
		}

#ifdef PERF_REMOVE_POST_UPDATE_AS_BLUR_DOES_NOT_HAVE_IMPORTANT_CODE_HERE
		if (myDataState.m_pIControlState)
		{
			myDataState.m_pIControlState->GetState()->ClampAllValues();	
		}
#endif
	}

	void CDriverAI::UpdateOutro(DataState &myDataState)
	{
		UpdateIntro(myDataState);	
	}



	void CDriverAI::Update()
	{
		if (m_pIData)
		{

			DataState &myDataState = m_pIData->GetState();

			IGameBrain *pIGameBrain = m_pIData->GetGameBrainAI();

			switch(myDataState.m_gameState)
			{
				case DataState::INTRO:
					{
						UpdateIntro(myDataState);
					}
					break;

				case DataState::GAME:
					{
						//UpdateGame(myDataState);
						if (pIGameBrain)
						{
							pIGameBrain->Update();
						}
					}
					break;

				case DataState::OUTRO:
					{
						UpdateOutro(myDataState);
					}
					break;
				
				default:
					break;
			}
		}
	}



	void CDriverAI::PreUpdateIntro(DataState &myDataState)
	{
		if (myDataState.m_pIControlState)
		{
			// this copies the current controls into the previous controls....
			myDataState.m_pIControlState->PreUpdate();
		}


		ISplineTracker *pSplineTracker = m_pIData->GetSplineTracker();
		if (pSplineTracker)
		{
			pSplineTracker->Update();
		}

		/* // This is not needed for the intro now.....
		ISpeedTracker *pSpeedTracker = m_pIData->GetSpeedTracker();
		if (pSpeedTracker)
		{
			//pSpeedTracker->Update();
		}
		*/

		IVehicleLogic *pVehicleLogic = m_pIData->GetVehicleLogic();
		if (pVehicleLogic)
		{
			pVehicleLogic->Update();
		}

		// must be done here.......
		CalculateCommonVehicleData();

#ifdef PERF_REMOVE_POST_UPDATE_AS_BLUR_DOES_NOT_HAVE_IMPORTANT_CODE_HERE
		if (myDataState.m_pIControlState)
		{
			myDataState.m_pIControlState->GetState()->ClampAllValues();	
		}
#endif
	}

	void CDriverAI::PreUpdateGame(DataState &myDataState)
	{
		PERF_EVENT("AI.PreUpdateGame");

		//if (myDataState.m_pIControlState)
		{
			// this copies the current controls into the previous controls....
			myDataState.m_pIControlState->PreUpdate();
		}

		

		//ISplineTracker *pSplineTracker = m_pIData->GetSplineTracker();
		//if (pSplineTracker)
		{
			//pSplineTracker->Update();
			myDataState.m_pISplineTracker->Update();
		}

		//ISpeedTracker *pSpeedTracker = m_pIData->GetSpeedTracker();
		//if (pSpeedTracker)
		{
			//pSpeedTracker->Update();
			myDataState.m_pISpeedTracker->Update();
		}

		//IVehicleLogic *pVehicleLogic = m_pIData->GetVehicleLogic();
		//if (pVehicleLogic)
		{
			//pVehicleLogic->Update();
			myDataState.m_pVehicleLogic->Update();
		}

		// this can override the track speed.......
		//if (myDataState.m_pIRubberBanding)
		{
			myDataState.m_pIRubberBanding->Update();
		}

		// must be done here.......
		CalculateCommonVehicleData();


		//if (myDataState.m_pIAvoidance)
		{
			myDataState.m_pIAvoidance->PreUpdate();
		}


		// Darren : update the game components.....this is a test..not sure it is the best method to choose..
		CMessage msg;
		msg.mType = MT_PRE_UPDATE_GAME_COMPONENTS;
		msg.mpData = 0;
		GlobalsComponent::GetObjectManager().PostAMessage(GetObjectId(), msg);
		
	}

	void CDriverAI::PreUpdateOutro(DataState &myDataState)
	{
		// Darren: for now
		PreUpdateIntro(myDataState);

#ifdef PERF_REMOVE_POST_UPDATE_AS_BLUR_DOES_NOT_HAVE_IMPORTANT_CODE_HERE
		if (myDataState.m_pIControlState)
		{
			myDataState.m_pIControlState->GetState()->ClampAllValues();	
		}
#endif
	
	}


	void CDriverAI::PreUpdate()
	{

		if (m_pIData)
		{
			DataState &myDataState = m_pIData->GetState();

			IGameBrain *pIGameBrain = m_pIData->GetGameBrainAI();

			switch(myDataState.m_gameState)
			{
				case DataState::INTRO:
					{
						PreUpdateIntro(myDataState);	
					}
					break;

				case DataState::GAME:
					{
						//PreUpdateGame(myDataState);

						if (pIGameBrain)
						{
							pIGameBrain->PreUpdate();
						}
					}
					break;

				case DataState::OUTRO:
					{
						PreUpdateOutro(myDataState);
					}
					break;

				default:
					break;
			}
		}
	}


	void CDriverAI::PostUpdateIntro(DataState &myDataState)
	{
		PostUpdateGame(myDataState);	
	}

	void CDriverAI::PostUpdateGame(DataState &myDataState)
	{
		
#ifndef PERF_REMOVE_POST_UPDATE_AS_BLUR_DOES_NOT_HAVE_IMPORTANT_CODE_HERE
		// clear the collision info... basically the game logic runs first, so the collision messages are a frame behind from the physics or gameloop. ie processed next frame
		//myDataState.m_CollisionInfoPrev = myDataState.m_CollisionInfo;
		//myDataState.m_CollisionInfo.Reset();


		if (myDataState.m_pIControlState)
		{
			myDataState.m_pIControlState->GetState()->ClampAllValues();	
		}

#if NEED_DEBUG_OUTPUT
		// Knock this out in release......
		if (myDataState.m_pIDebugAI)
		{
			myDataState.m_pIDebugAI->Update();
		}
#endif

#endif
	}

	void CDriverAI::PostUpdateOutro(DataState &myDataState)
	{
		PostUpdateGame(myDataState);	
	}


	void CDriverAI::PostUpdate()
	{

#ifndef PERF_REMOVE_POST_UPDATE_AS_BLUR_DOES_NOT_HAVE_IMPORTANT_CODE_HERE


		if (m_pIData)
		{
			DataState &myDataState = m_pIData->GetState();

			IGameBrain *pIGameBrain = m_pIData->GetGameBrainAI();
			
			switch(myDataState.m_gameState)
			{
				case DataState::INTRO:
					{
						PostUpdateIntro(myDataState);
					}
					break;

				case DataState::GAME:
					{
						//PostUpdateGame(myDataState);
						if (pIGameBrain)
						{
							pIGameBrain->PostUpdate();
						}
					}
					break;

				case DataState::OUTRO:
					{
						PostUpdateOutro(myDataState);
					}
					break;

				default:
					break;
			}
		}
#endif
	}

	

	void CDriverAI::CalculateSteerState()
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


	void CDriverAI::CalculateCommonVehicleData()
	{
		PERF_EVENT("AI.Driver.CalculateCommonVehicleData");

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


	
	

	void CDriverAI::UpdatePressure()
	{

	}

	void CDriverAI::UpdateExcitementLevel()
	{
		if (m_pIData)
		{
			//UpdateSliding();
		}
	}



}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif






