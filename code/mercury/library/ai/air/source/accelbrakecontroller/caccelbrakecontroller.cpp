#include "stdafx.h"
#include <accelbrakecontroller/caccelbrakecontroller.h>
#include <vehiclestate/ivehiclestate.h>
#include <controlsstate/icontrolstate.h>


//#define ACCEL_BRAKE_OPTIMISE_OFF


#ifdef _MSC_VER
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#ifdef DEBUG_AI
#pragma optimize("", off)
#pragma message("ACCEL_BRAKE_OPTIMISE_OFF is enabled")
#endif
#endif
#endif


static Debug::Logger &g_DebugLoggerAccelBrakeController = Debug::GetLogger("oldtech.ai.accelbrakecontroller");

Debug::Float g_TCLS("OldAI.Car.TC", 0.9f, Debug::Range(1.0f, 0.0f), Debug::Step(0.001f));
Debug::Float g_ABSLS("OldAI.Car.ABS", 0.75f, Debug::Range(1.0f, 0.0f), Debug::Step(0.001f));


Debug::Bool g_DebugEnableNudgeSwerve("OldAI.Debug.Damage.Nudge",false);

namespace airacingtech
{
	using namespace fuzzy_lib_one;
	using namespace fuzzy_lib_two;

#define FUZZY_DEBUG 0

#if FUZZY_DEBUG
	static CObjectId Throttle("Throttle");
	static CObjectId AngularVeloicty("AngularVeloicty");
	static CObjectId LatSlip("LapSlip");
	static CObjectId DistToSpline("DistToSpline");

	// low
	static CObjectId ThrottleLow("ThrottleLow");
	static CObjectId AngularVelocityLow("AngularVeloictyLow");
	static CObjectId LatSlipLow("LapSlipLow");
	static CObjectId DistToSplineLow("DistToSplineLow");

	// medium
	static CObjectId ThrottleMedium("ThrottleMedium");
	static CObjectId AngularVeloictyMedium("AngularVeloictyMedium");
	static CObjectId LatSlipMedium("LapSlipMedium");

	// high
	static CObjectId ThrottleHigh("ThrottleHigh");
	static CObjectId AngularVeloictyHigh("AngularVeloictyHigh");
	static CObjectId LatSlipHigh("LapSlipHigh");
	static CObjectId DistToSplineHigh("DistToSplineHigh");
#endif


	CAccelBrakeController::CAccelBrakeController() :m_DirectInput(false),m_pIVehicleState(0), m_pIData(0), m_handbrakeCount(0), m_startAIRace(false)
	{
		m_accelInc = 0.0f;
		m_brakeInc = 0.0f;
		m_hbrakeInc = -1.0f;
		m_accel = 0.0f;
		m_brake = 0.0f;
		m_startTime = -1;
		m_revState = REV_NONE;
		m_revTimer = 0;
		m_revTimerTarget = 0;
		m_nudgedState = NUDGE_STATE_NORMAL;

#if FUZZY_DEBUG		
		FuzzyVariable &throttleVar = m_fzPolicyThrottle.CreateFLV(Throttle);
		FuzzyVariable &angVelVar = m_fzPolicyThrottle.CreateFLV(AngularVeloicty);
		FuzzyVariable &LatSlipVar = m_fzPolicyThrottle.CreateFLV(LatSlip);
		//FuzzyVariable &DistToSplineVar = m_fzPolicyThrottle.CreateFLV(DistToSpline);



		FzSet AngVelLowSet = angVelVar.AddLeftShoulderSet(AngularVelocityLow, 0.0f, 0.25f, 0.75f);
		FzSet AngVelMediumSet = angVelVar.AddTriangularSet(AngularVeloictyMedium, 0.25f, 0.75f, 1.25f);
		FzSet AngVelHighSet = angVelVar.AddRightShoulderSet(AngularVeloictyHigh, 0.75f , 1.25f , 2.0f );

		//const float maxLatSlip  = 0.5f;
		FzSet LatSlipLowSet = LatSlipVar.AddLeftShoulderSet(LatSlipLow,0.0f, 0.25f , 0.6f );
		FzSet LatSlipHighSet = LatSlipVar.AddRightShoulderSet(LatSlipHigh, 0.25f , 0.6f , 1.0f );

		FzSet ThrottleLowSet = throttleVar.AddLeftShoulderSet(ThrottleLow, 0.0f, 0.15f, 0.5f);
		FzSet ThrottleMediumSet = throttleVar.AddTriangularSet(ThrottleMedium, 0.15f, 0.5f, 0.85f);
		FzSet ThrottleHighSet = throttleVar.AddRightShoulderSet(ThrottleHigh, 0.5f, 0.85f, 1.0f);


		//FzSet DistToSplineLowSet = DistToSplineVar.AddLeftShoulderSet(DistToSplineLow,0.0f, 1.0f , 4.0f );
		//FzSet DistToSplineHighSet = DistToSplineVar.AddRightShoulderSet(DistToSplineHigh, 2.0f , 4.0f , 10.0f );



		m_fzPolicyThrottle.AddRule(FzAND(AngVelLowSet, LatSlipLowSet), ThrottleHighSet);
		m_fzPolicyThrottle.AddRule(FzAND(AngVelLowSet, LatSlipHighSet), ThrottleHighSet); 

		m_fzPolicyThrottle.AddRule(FzAND(AngVelMediumSet, LatSlipLowSet), ThrottleHighSet);
		m_fzPolicyThrottle.AddRule(FzAND(AngVelMediumSet, LatSlipHighSet), ThrottleHighSet);

		m_fzPolicyThrottle.AddRule(FzAND(AngVelHighSet, LatSlipLowSet), ThrottleHighSet);
		m_fzPolicyThrottle.AddRule(FzAND(AngVelHighSet, LatSlipHighSet), ThrottleLowSet);

		//m_fzPolicyThrottle.Fuzzify(AngularVeloicty, av);
		//m_fzPolicyThrottle.Fuzzify(LatSlip, ls);
		//float dfzThrottle = m_fzPolicyThrottle.DeFuzzify(Throttle);
#endif
	}

	CAccelBrakeController::~CAccelBrakeController()
	{
		m_accelInc = 0.0f;
		m_brakeInc = 0.0f;
		m_hbrakeInc = -1.0f;
		m_accel = 0.0f;
		m_brake = 0.0f;
		m_startTime = -1;
		m_handbrakeCount = 0;
		m_startAIRace = false;
		m_DirectInput = false;
		m_revState = REV_NONE;
		m_revTimer = 0;
		m_revTimerTarget = 0;
	}


	void CAccelBrakeController::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_ACCEL_BRAKE_CONTROLLER_AI, CID_ACCEL_BRAKE_CONTROLLER_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_ACCEL_BRAKE_CONTROLLER_AI, CAccelBrakeController::CreateMe, CAccelBrakeController::DestroyMe, CHash("AccelBrakeController"));
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_ACCEL_BRAKE_CONTROLLER_AI, MT_POST_INIT_AI_SYSTEM);
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_ACCEL_BRAKE_CONTROLLER_AI, MT_QUICK_RESTART);
		
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_ACCEL_BRAKE_CONTROLLER_AI, MT_COLLISION);
	
		GlobalsComponent::GetObjectManager().SubscribeToMessageType(CID_ACCEL_BRAKE_CONTROLLER_AI, MT_STOP_AI);

		
	
	}

	IComponent *CAccelBrakeController::CreateMe()
	{
		return new CAccelBrakeController;
	}

	bool CAccelBrakeController::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CAccelBrakeController::Init(CObjectId oid, CParameterNode &paramNode)
	{
		return true;
	}

	void CAccelBrakeController::Deinit(void)
	{

	}

	EMessageResult CAccelBrakeController::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		switch (msg.mType)
		{

	

		case MT_QUICK_RESTART:
			{
				QuickRestart();
			}
			break;


		case MT_POST_INIT_AI_SYSTEM:
			{
				msgRet =  MR_TRUE;
				CachedInterfaces();
			}
			break;

		case MT_STOP_AI:
			{
				SetControlsDirectly(false, 0.0f,0.0f,0.0f,false);
			}
			break;
	

		default:
			break;
		}

		return msgRet;
	}

	void CAccelBrakeController::QuickRestart()
	{
		m_accelInc = 0.0f;
		m_brakeInc = 0.0f;
		m_hbrakeInc = -1.0f;
		m_accel = 0.0f;
		m_brake = 0.0f;
		m_startTime = -1;
		m_handbrakeCount = 0;
		m_startAIRace = false;
		m_DirectInput = false;
		m_revState = REV_NONE;
		m_revTimer = 0;
		m_revTimerTarget = 0;
	}

	void CAccelBrakeController::CachedInterfaces()
	{
		// cache all the interfaces needed for the ai...

		CObjectId id = GetObjectId();

		if (!m_pIVehicleState)
			m_pIVehicleState = static_cast<IVehicleState *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_VEHICLE_STATE));
		if (!m_pIData)
			m_pIData = static_cast<IData *>(GlobalsComponent::GetObjectManager().QueryInterface(id, IID_DATA_AI));

	}

	EComponentTypeId CAccelBrakeController::GetComponentTypeId(void)
	{
		return CID_ACCEL_BRAKE_CONTROLLER_AI;
	}

	/* Compute gear */
	int CAccelBrakeController::Gear()
	{
		//If all of the above didn't apply, simply return the current gear. You have to put the new constants at the beginning of driver.cpp. 
		const float SHIFT = 0.9f;         /* [-] (% of rpmredline) */
		const float SHIFT_MARGIN = 4.0f;  /* [m/s] */

		int gear = m_pIVehicleState->GetState()->GetGear();

		if (gear <= 0) 
			return 1;

		//If we are in neutral or reverse shift to the first gear. 

		float gr_up = m_pIVehicleState->GetState()->GetGearRatios(gear);
		float rpmMax = m_pIVehicleState->GetState()->GetMaxRevs();
		float rpmlimit = m_pIVehicleState->GetState()->GetRevLimit();
		float vehiclespeed = m_pIVehicleState->GetState()->GetSpeed();


		float omega = rpmMax / gr_up;
		float wr = m_pIVehicleState->GetState()->GetWheel(2)->GetRadius();

		if (omega*wr*SHIFT < vehiclespeed) 
		{
			return gear + 1;
			//Shift up if the allowed speed for the current gear (omega*wr*SHIFT) is exceeded. 
		} 
		else 
		{
			float gr_down = m_pIVehicleState->GetState()->GetGearRatios(gear-1);
			omega = rpmMax/gr_down;
			if (gear > 1 && omega*wr*SHIFT > vehiclespeed + SHIFT_MARGIN) 
			{
				return gear - 1;

				//If the current gear is greater than one (we don't want to shift to neutral here), check if the current speed (plus a constant) is lower than the allowed speed with the next lower gear. If that's true, shift down. We add the constant to avoid oscillating up- and down-shifting. 
			}
		}
		return gear;
	}





	float CAccelBrakeController::Accel()
	{
		float accel = 0.0f;
		float trackspeed = m_pIData->GetState().m_trackSpeed;
		float vehiclespeed = m_pIVehicleState->GetState()->GetSpeed();
		const float ACCEL_MARGIN = 2.0f; // could change this slightly

		int gear = m_pIVehicleState->GetState()->GetGear();
		float gearratio = m_pIVehicleState->GetState()->GetGearRatios(gear);
		float rpmMax = m_pIVehicleState->GetState()->GetMaxRevs();
		float rpmlimit = m_pIVehicleState->GetState()->GetRevLimit();

		if (trackspeed > vehiclespeed + ACCEL_MARGIN)
		{
			accel = 1.0f;
		}
		else
		{

			float wheelRadius = m_pIVehicleState->GetState()->GetWheel(2)->GetRadius();
			accel = trackspeed / wheelRadius * gearratio / rpmlimit;

			//accel = 1.0f;
		}

		return accel;
	}

	float CAccelBrakeController::Brake()
	{
		return 1.0f;
	}


	float CAccelBrakeController::ABS(float b)
	{
		float ABS_SLIP = g_ABSLS;//0.8f;        /* [-] range [0.95..0.6] */
		const float ABS_MINSPEED = 3.0f;    /* [m/s] */

		float brake = b;
		float trackspeed = m_pIData->GetState().m_trackSpeed;
		float vehiclespeed = m_pIVehicleState->GetState()->GetSpeed();

		if (vehiclespeed < ABS_MINSPEED)
		{
			brake = b;
		}
		else
		{

			float slip = 0.0f;

			// decide drive type....
			if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven() && m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
			{
				//slip = vehiclespeed  / TCAWD();

				//slip = (RearLongSlip() + FrontLongSlip()) / 2.0f;

				slip = TCAWD();
			}
			else if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven())
			{
				//slip = vehiclespeed  / TCRWD();

				//slip = RearLongSlip();


				slip = TCRWD();
			}
			else if (m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
			{
				//slip = vehiclespeed  / TCFWD();

				//slip = FrontLongSlip();

				slip = TCFWD();
			}

			// need to check this across the vehicles.....and in the weather........
			float maxSlip = m_pIVehicleState->GetState()->GetAmaxCarData().m_tyreSlip;
			if (slip > maxSlip)
			{
				brake = 1.0f; // test
				//brake -= (Maths::Abs(slip) * 0.5f); // 3 was good.. might need to set it higer
				//brake = Maths::Max(0.0f, brake);
			}

			//brake = 1.0f;
#if 0
			int i;
			float slip = 0.0;
			for (i = 0; i < 4; i++) 
			{
				float wheelRotation = m_pIVehicleState->GetState()->GetWheel(i)->GetAngularRotation();
				float radius =  m_pIVehicleState->GetState()->GetWheel(i)->GetRadius();

				slip += wheelRotation * radius / vehiclespeed;
			}
			slip /= 4.0f;

			if (slip < ABS_SLIP) 
				brake = brake*slip;
#endif

#if 0
			if (Maths::Abs(trackspeed - vehiclespeed) < 1.0f)
			{
				int gear = m_pIVehicleState->GetState()->GetGear();
				float gearratio = m_pIVehicleState->GetState()->GetGearRatios(gear);
				float rpmMax = m_pIVehicleState->GetState()->GetMaxRevs();
				float rpmlimit = m_pIVehicleState->GetState()->GetRevLimit();

				float wheelRadius = m_pIVehicleState->GetState()->GetWheel(2)->GetRadius();
				brake = trackspeed / wheelRadius * gearratio / rpmlimit;
			}
			else
			{
			}
#endif


		}



		return brake;
	}

	float CAccelBrakeController::TCAWD()
	{
		float t_c = (TCFWD() + TCRWD()) / 2.0f;
		return t_c;
	}

	float CAccelBrakeController::TCFWD()
	{
		//float wheelRotation0 = Maths::Max(0.000001f,Maths::Abs(m_pIVehicleState->GetState()->GetWheel(0)->GetAngularRotation()));
		//float wheelRotation1 = Maths::Max(0.000001f,Maths::Abs(m_pIVehicleState->GetState()->GetWheel(1)->GetAngularRotation()));
		//float radius =  m_pIVehicleState->GetState()->GetWheel(0)->GetRadius();
		//float t_c = (wheelRotation0 + wheelRotation1) * radius / 2.0f; 
		//return t_c;
		float wheelSpin = Maths::Abs(m_pIVehicleState->GetState()->GetWheel(0)->GetWheelSpin() +  m_pIVehicleState->GetState()->GetWheel(1)->GetWheelSpin() ) * 0.5f;

		return wheelSpin;
	}

	float CAccelBrakeController::TCRWD()
	{
		//float wheelRotation2 = Maths::Max(0.000001f,Maths::Abs(m_pIVehicleState->GetState()->GetWheel(2)->GetAngularRotation()));
		//float wheelRotation3 =  Maths::Max(0.000001f,Maths::Abs(m_pIVehicleState->GetState()->GetWheel(3)->GetAngularRotation()));
		//float radius =  m_pIVehicleState->GetState()->GetWheel(2)->GetRadius();
		//float t_c = (wheelRotation2 + wheelRotation3) * radius / 2.0f; 
		//return t_c;

		float wheelSpin = Maths::Abs(m_pIVehicleState->GetState()->GetWheel(2)->GetWheelSpin() +  m_pIVehicleState->GetState()->GetWheel(3)->GetWheelSpin() ) * 0.5f;

		return wheelSpin;
	}


	float CAccelBrakeController::DrivenLateralSlip()
	{
		float lateralSlip = 0.0f;
		if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven() && m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
		{
			// 4 wheel drive...
			lateralSlip = m_pIVehicleState->GetState()->GetWheel(0)->GetLateralSlip() + m_pIVehicleState->GetState()->GetWheel(1)->GetLateralSlip()
				+ m_pIVehicleState->GetState()->GetWheel(2)->GetLateralSlip() + m_pIVehicleState->GetState()->GetWheel(3)->GetLateralSlip();
			lateralSlip /= 4.0f;
		}
		else if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven())
		{
			// rear wheel drive	...
			lateralSlip = m_pIVehicleState->GetState()->GetWheel(2)->GetLateralSlip() + m_pIVehicleState->GetState()->GetWheel(3)->GetLateralSlip();
			lateralSlip /= 2.0f;
		}
		else if (m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
		{
			// front wheel drive...	
			lateralSlip = m_pIVehicleState->GetState()->GetWheel(0)->GetLateralSlip() + m_pIVehicleState->GetState()->GetWheel(1)->GetLateralSlip();
			lateralSlip /= 2.0f;
		}

		return lateralSlip;
	}

	float CAccelBrakeController::DrivenLongSlip()
	{
		float LongSlip = 0.0f;
		if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven() && m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
		{
			// 4 wheel drive...
			LongSlip = m_pIVehicleState->GetState()->GetWheel(0)->GetLongSlip() + m_pIVehicleState->GetState()->GetWheel(1)->GetLongSlip()
				+ m_pIVehicleState->GetState()->GetWheel(2)->GetLongSlip() + m_pIVehicleState->GetState()->GetWheel(3)->GetLongSlip();
			LongSlip /= 4.0f;
		}
		else if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven())
		{
			// rear wheel drive	...
			LongSlip = m_pIVehicleState->GetState()->GetWheel(2)->GetLongSlip() + m_pIVehicleState->GetState()->GetWheel(3)->GetLongSlip();
			LongSlip /= 2.0f;
		}
		else if (m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
		{
			// front wheel drive...	
			LongSlip = m_pIVehicleState->GetState()->GetWheel(0)->GetLongSlip() + m_pIVehicleState->GetState()->GetWheel(1)->GetLongSlip();
			LongSlip /= 2.0f;
		}

		return LongSlip;
	}


	float CAccelBrakeController::FrontLongSlip()
	{
		float LongSlip = 0.0f;
		LongSlip = m_pIVehicleState->GetState()->GetWheel(0)->GetLongSlip() + m_pIVehicleState->GetState()->GetWheel(1)->GetLongSlip();
		LongSlip /= 2.0f;
		return LongSlip;
	}

	float CAccelBrakeController::RearLongSlip()
	{
		float LongSlip = 0.0f;

		LongSlip = m_pIVehicleState->GetState()->GetWheel(2)->GetLongSlip() + m_pIVehicleState->GetState()->GetWheel(3)->GetLongSlip();
		LongSlip /= 2.0f;
		return LongSlip;
	}

	float CAccelBrakeController::FrontLateralSlip()
	{
		float lateralSlip = 0.0f;

		lateralSlip = m_pIVehicleState->GetState()->GetWheel(0)->GetLateralSlip() + m_pIVehicleState->GetState()->GetWheel(1)->GetLateralSlip();
		lateralSlip /= 2.0f;
		return lateralSlip;
	}


	float CAccelBrakeController::RearLateralSlip()
	{
		float lateralSlip = 0.0f;

		lateralSlip = m_pIVehicleState->GetState()->GetWheel(2)->GetLateralSlip() + m_pIVehicleState->GetState()->GetWheel(3)->GetLateralSlip();
		lateralSlip /= 2.0f;
		return lateralSlip;
	}




	float CAccelBrakeController::TC(float accel)
	{
		// 0.8 is very good
		float TRACTION_CONTROL_LIMITED_SLIP = g_TCLS;//0.4f;//0.9;        /* [-] range [0.95..0.3] */
		const float TRACTION_CONTROL_LIMITED_MINSPEED = 3.0;    /* [m/s] */

		float vehiclespeed = m_pIVehicleState->GetState()->GetSpeed();
		float tc = accel;

		if (vehiclespeed < TRACTION_CONTROL_LIMITED_MINSPEED) 
		{
			tc = accel;
		}
		else
		{
			float slip = 0.0f;

			// decide drive type....
			if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven() && m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
			{
				//slip = vehiclespeed  / TCAWD();
				slip = TCAWD();	
			}
			else if (m_pIVehicleState->GetState()->GetWheel(2)->IsDriven())
			{
				//slip = vehiclespeed  / TCRWD();
				slip = TCRWD();
			}
			else if (m_pIVehicleState->GetState()->GetWheel(0)->IsDriven())
			{
				//slip = vehiclespeed  / TCFWD();
				slip = TCFWD();
			}

			

			float maxSlip = m_pIVehicleState->GetState()->GetAmaxCarData().m_tyreSlip;

			//float wheelSpin = (m_pIVehicleState->GetState()->GetWheel(2)->GetWheelSpin() +  m_pIVehicleState->GetState()->GetWheel(3)->GetWheelSpin() ) * 0.5f;
			float SlipNoise = 1.0f;//1.25f; // 1.5 works well, 1.75 is still in controll, 2.0 spins out
			
			//if (slip > maxSlip*SlipNoise || Maths::Abs(m_pIVehicleState->GetState()->GetAngularVelocity().Y()) > 0.98f) 
			if (Maths::Abs(m_pIVehicleState->GetState()->GetAngularVelocity().Y()) > 0.98f) 
			{
				//tc = 0.5f; // normal driving.....
				tc = 0.5f; // 0.1f;
			}

		}

		

		return tc;
	}




	void CAccelBrakeController::ControlWithSliding()
	{
		ControlState *pControlState = m_pIData->GetControlState()->GetState();
		assert(pControlState);


		float diffSp = m_pIData->GetState().m_DiffTrackSpeed;

		if ((m_pIData->GetState().m_slideHandBrake == true))
		{

			if ((m_handbrake == false) && (m_hbrakeInc < 0.0f))
			{
				m_handbrake = true;
				m_hbrakeInc = 200.0f; // 100.0f
			}
		}



		//float tc = TC(1.0f);
		float latSlip = Maths::Abs(DrivenLateralSlip());
		float longSlip = Maths::Abs(DrivenLongSlip());
		float angVelYAbs = Maths::Abs(m_pIData->GetState().m_angVelY);	


		float angVelDiff = m_pIData->GetState().m_angVelY - m_pIData->GetState().m_angVelYLast;
		//printf("AngVelDiff :%f\n", angVelDiff);
		//printf("LongSlip :%f, LatSlip :%f\n", longSlip, latSlip);


		bool sliding = false;
		float percentageSlideRatio = 0.0f;
		if (m_pIVehicleState)
		{
			float zVel = m_pIVehicleState->GetState()->GetVelocity() |  m_pIVehicleState->GetState()->GetZaxis();
			float xVel = m_pIVehicleState->GetState()->GetVelocity() |  m_pIVehicleState->GetState()->GetXaxis();
			percentageSlideRatio = Maths::Abs((xVel / zVel));

			if (latSlip > 0.2f) // 0.2
			{
				sliding = true;	
				if ((percentageSlideRatio > /*0.55f*/ 0.55f) || (angVelYAbs > 1.55f))// 8 ok
				{
					m_handbrake = false;
					m_hbrakeInc = -1.0f;
				}
			}

		}

#if FUZZY_DEBUG
		// as an optimisation,we could store the fuzzy variable, it would save the lookup
		m_fzPolicyThrottle.Fuzzify(AngularVeloicty, angVelYAbs);
		m_fzPolicyThrottle.Fuzzify(LatSlip, /*percentageSlideRatio*/latSlip);
#endif	



		if (diffSp < 0.0f || sliding) 
		{
			m_accel += 0.1f;
			m_brake = 0.0f;
			m_brake = Maths::Max(m_brake, 0.0f);
			m_accel = Maths::Min(m_accel, 1.0f);

			float slideAngle = m_pIData->GetState().m_SlideSteeringAngle;
			float steeringAngle = m_pIData->GetState().m_SteeringAngle;

#if 1
			m_accel = 0.65f;
#if FUZZY_DEBUG
			float dfzThrottle = m_fzPolicyThrottle.DeFuzzify(Throttle);	
			m_accel = dfzThrottle;
#endif
			if (Maths::Abs(slideAngle) < 40.5) // 20 or higher...
			{
				m_handbrake = false;
				m_hbrakeInc = -1.0f;
			}



#else
			if (sliding)
			{


				if (Maths::Abs(slideAngle) < 22.5) // 20 or higher...
				{
					m_accel = 1.0f;
					m_handbrake = false;
				}
				else 
				{
					if (angVelYAbs > 1.2f) // 0.9 worked
						m_accel -= 0.11f;  // 0.2f
				}


			}
			else
			{
				m_accel = 1.0f;	
			}

#endif

			m_accel = Maths::Max(m_accel, 0.0f);
		}
		else if (diffSp > 0.0f)
		{
			m_accel = 0.0f;
			m_brake += 0.1f;
			m_accel = Maths::Max(m_accel, 0.0f);
			m_brake = Maths::Min(m_brake, 1.0f);

			if ((m_pIData->GetState().m_TryAndSlide == true) && (m_handbrake == true))
			{

				m_brake = 0.0f;
				m_accel = 0.0f; 
			}
		}
		else
		{
			m_brake = 0.0f;
			m_accel = 0.0f; 
		}


		// set the controls...

		pControlState->SetAccel(m_accel);
		pControlState->SetBrake(m_brake);
		pControlState->SetHandbrake(m_handbrake);


		m_hbrakeInc -= 1.0f;	
		if (m_hbrakeInc < 0.0f)
		{
			m_handbrake = false;
		}


	}

	float CAccelBrakeController::AlterSpeed()
	{
		float diffSpeedTrack = m_pIData->GetState().m_DiffTrackSpeed = m_pIData->GetVehicleState()->GetState()->GetSpeed() - m_pIData->GetState().m_trackSpeed;

		if (m_pIData->GetVehicleState()->GetState()->GetSpeed() > 20.0f)
		{
#if 0			
			ISplineFollowingAI *pSplineFollowing = m_pIData->GetSplineFollowing();
			if (pSplineFollowing)
			{
				INavigationTrackerAI *pNavTRacker = m_pIData->GetNAvigationTracker();
				if (pNavTRacker)
				{
					float c = pNavTRacker->GetRoute(0)->m_curvature;
					if (c > 0.002f) // 0.003f
					{
						// right hand turn
						if (pSplineFollowing->GetDistToSpline() < -0.25f)
						{
							float fzSide = FuzzyRightShoulder(Maths::Abs(pSplineFollowing->GetDistToSpline()), 0.0f, 2.5f);
							// need to adjust the speed
							diffSpeedTrack += (fzSide * 7.0f); // 5.0f
						}
					}
					else if (c < -0.002f)
					{
						// left hand turn
						if (pSplineFollowing->GetDistToSpline() > 0.25f)
						{
							// need to adjust the speed
							float fzSide = FuzzyRightShoulder(Maths::Abs(pSplineFollowing->GetDistToSpline()), 0.0f, 2.5f);
							// need to adjust the speed
							diffSpeedTrack += (fzSide * 7.0f); //5.0f
						}
					}
				}

			}
#endif
		}
		

		return diffSpeedTrack;
	}

	void CAccelBrakeController::ControlWithoutSliding()
	{
		m_handbrake = false;

		//float diffSpeedTrack = m_pIData->GetState().m_DiffTrackSpeed = m_pIData->GetVehicleState()->GetState()->GetSpeed() - m_pIData->GetState().m_trackSpeed;
		//float diffSpeedTrack = m_pIData->GetState().m_DiffTrackSpeed;

		//float frontLatSlip = FrontLateralSlip();
		//float frontLongSlip = FrontLongSlip();

		//float rearLatSlip = RearLateralSlip();
		//float rearLongSlip = RearLongSlip();

		//printf("Front Lat slip  :%f\n", frontLatSlip);
		//printf("Front Long slip :%f\n", frontLongSlip);
		//printf("Rear  Lat slip  :%f\n", rearLatSlip);
		//printf("Rear  Long slip :%f\n", rearLongSlip);


		float diffSpeedTrack = AlterSpeed();

		if (diffSpeedTrack < -1.0f) // this could be a driver parameter
		//if (diffSpeedTrack < 0.0f)
		{
#if 0
			if (diffSpeedTrack > -3.0f)
			{
				m_accel += 0.1f;
				m_brake = 0.0f;

				m_accel = TC(m_accel);
				//m_brake = Maths::Max(m_brake, 0.0f);
				m_accel = Maths::Min(m_accel, 1.0f);
			}
			else
			{
				m_accel = 1.0f;
				m_accel = TC(m_accel);
				m_brake = 0.0f;
				m_brake = Maths::Max(m_brake, 0.0f);
				m_accel = Maths::Min(m_accel, 1.0f);
			}
#endif

			m_accel += 0.05f;
			m_accel = Maths::Min(m_accel, 1.0f);
			m_brake = 0.0f;

			//m_accel = 1.0f;
			//m_accel = TC(m_accel);
			//m_brake = Maths::Max(m_brake, 0.0f);
			m_accel = Maths::Min(m_accel, 1.0f);


		}
		else if (diffSpeedTrack  > 2.0f) // 1.0f
		//else if (diffSpeedTrack  > 0.0f)
		{
			
			if (diffSpeedTrack < 3.0f)
			{
				//m_brake = 1.0f;//1.0f;	// 0.95f
				m_brake += 0.1f; // 0.0225 doesn`t work for hairpins.....
				m_accel = 0.0f;

				m_brake = Maths::Max(m_brake, 0.75f);
				m_brake = Maths::Min(m_brake, 1.0f);
			}
			else
			{
				m_brake = 1.0f;	
				m_accel = 0.0f;
			}

			m_brake = ABS(1.0f);
			//m_brake = 1.0f;
		}
		else
		{
			m_brake -= 0.1f;
			m_accel -= 0.01f;
			m_brake = Maths::Max(m_brake, 0.0f);
			m_accel = Maths::Max(m_accel, 0.0f);
			//m_brake = 0.0f;
			//m_accel = 0.0f; 
		}


		ControlState *pControlState = m_pIData->GetControlState()->GetState();
		assert(pControlState);
		pControlState->SetPitch(0.0f);

		bool onGround = false;


/*
		if (!m_pIVehicleState->GetState()->GetWheel(0)->GetOnGround() &&
			!m_pIVehicleState->GetState()->GetWheel(1)->GetOnGround() &&
			!m_pIVehicleState->GetState()->GetWheel(2)->GetOnGround() &&
			!m_pIVehicleState->GetState()->GetWheel(3)->GetOnGround() 
			)
		{
			
			m_accel = 1.0f;
			m_brake = 0.0f;
			pControlState->SetSteering(0.0f);

			
			// set the pitch, very simple, should really be on a pid....or spring dampner
			float y = m_pIVehicleState->GetState()->GetZaxis().Y();
			if (y < 0.0f)
				pControlState->SetPitch(-1.0f); 
			else if (y > 0.2f)
				pControlState->SetPitch(1.0f);
		}
*/		

		// set the controls...
		int gear = Gear();

		

		pControlState->SetAccel(m_accel);
		pControlState->SetBrake(m_brake);
		pControlState->SetHandbrake(m_handbrake);
		pControlState->SetGear(gear);


		//Nudged();

	}

#if 0
	void CAccelBrakeController::Nudged()
	{
		ControlState *pControlState = m_pIData->GetControlState()->GetState();
		assert(pControlState);

		// get the hit list.....

		switch(m_nudgedState)
		{
			case NUDGE_STATE_NORMAL:
				{
					// check for the collision......
					if (g_DebugEnableNudgeSwerve)
					{
						DataState &myDataState = m_pIData->GetState();

						IVehicleState			*pIVehicleStateMY = m_pIData->GetVehicleState();
						VehicleState			*pVehicleStateMy = pIVehicleStateMY->GetState();

						if (myDataState.m_CollisionInfoPrev.m_collisionType == CollisionInfo::VEHICLE_VEHICLE)
						{
							// Seems to be an error in the collision and the collision length....
							float z = myDataState.m_CollisionInfoPrev.m_Position.Z();
							float x = myDataState.m_CollisionInfoPrev.m_Position.X();
							float vehicleLength = m_pIData->GetVehicleState()->GetState()->GetVehicleSize().Z() * 0.5f;
							float impactSize = myDataState.m_CollisionInfoPrev.m_ImpactSize;
							
							// assuming that 0.0f id the rough middle of the car....

							if (z > 1.5f)
							{
								m_impactSize = impactSize;
								int rnd = RandomInRange(15, 30); // 30 was pretty good
								m_swerveTime = m_startSwerveTime = rnd;	// 30 was good....
								m_startSwerveTime /= 2;

								if (x > 0.0f)
								{
									m_nudgedState = NUDGE_STATE_SWERVE_RIGHT_THEN_LEFT;
								}
								else
								{
									m_nudgedState = NUDGE_STATE_SWERVE_LEFT_THEN_RIGHT;
								}
								
							}
							else if (z > 0.0f)
							{

								m_impactSize = impactSize;

								// side to side collision 
								int rnd = RandomInRange(1, 2);
								if (rnd == 1)
								{
									int rnd = RandomInRange(15, 30);
									m_swerveTime = m_startSwerveTime = rnd;	// 30 was good....
									m_startSwerveTime /= 2;

									if (x > 0.0f)
									{
										m_nudgedState = NUDGE_STATE_SWERVE_RIGHT_THEN_LEFT;
									}
									else
									{
										m_nudgedState = NUDGE_STATE_SWERVE_LEFT_THEN_RIGHT;
									}
								}
								else
								{
									int rnd = RandomInRange(5, 20);
									m_swerveTime = m_startSwerveTime = rnd;	// 30 was good....
									//m_startSwerveTime /= 2;

									if (x > -1.0f)
									{
										m_nudgedState = NUDGE_STATE_SWERVE_LEFT;
									}
									else
									{
										m_nudgedState = NUDGE_STATE_SWERVE_RIGHT;
									}
								}

							}
						}
					}
				}
				break;

			case NUDGE_STATE_SWERVE_RIGHT:
				{
					if (m_swerveTime > 0)
					{
						// right
						m_swerveTime--;
						pControlState->SetSteering(-1.0f);	
					}
					else
					{
						m_swerveTime = 60;
						m_nudgedState = NUDGE_STATE_RETURN_TO_NORMAL;
					}
				}
				break;

			case NUDGE_STATE_SWERVE_LEFT:
				{
					if (m_swerveTime > 0)
					{
						// right
						m_swerveTime--;
						pControlState->SetSteering(1.0f);	
					}
					else
					{
						m_swerveTime = 60;
						m_nudgedState = NUDGE_STATE_RETURN_TO_NORMAL;
					}
				}
				break;

		
			case NUDGE_STATE_SWERVE_LEFT_THEN_RIGHT:
				{
					if (m_swerveTime > m_startSwerveTime)
					{
						// left
						m_swerveTime--;
						pControlState->SetSteering(1.0f);
					}
					else if (m_swerveTime > 0)
					{
						// right
						m_swerveTime--;
						pControlState->SetSteering(-1.0f);	
					}
					else
					{
						m_swerveTime = 60;
						m_nudgedState = NUDGE_STATE_RETURN_TO_NORMAL;
					}
				}
				break;

			case NUDGE_STATE_SWERVE_RIGHT_THEN_LEFT:
				{
					if (m_swerveTime > m_startSwerveTime )
					{
						// left
						m_swerveTime--;
						pControlState->SetSteering(-1.0f);
					}
					else if (m_swerveTime > 0)
					{
						// right
						m_swerveTime--;
						pControlState->SetSteering(1.0f);	
					}
					else
					{
						m_swerveTime = 60;
						m_nudgedState = NUDGE_STATE_RETURN_TO_NORMAL;
					}
				}
				break;

			case NUDGE_STATE_RETURN_TO_NORMAL:
				{
					if (m_swerveTime > 0)
					{
						m_swerveTime--;
					}
					else
					{
						m_nudgedState = NUDGE_STATE_NORMAL;
					}
				}
				break;

			default:
				break;
		}

	}
#endif


	void CAccelBrakeController::HoldAIOnTheBrakes()
	{
		m_revTimer++;
		m_accel = 0.0f;
		m_brake = 0.0f;
		m_handbrake = true;

		ControlState *pControlState = m_pIData->GetControlState()->GetState();
		assert(pControlState);

		switch(m_revState)
		{
			case REV_NONE:
				{
					if (m_revTimer > m_revTimerTarget)
					{
						m_revTimerTarget = RandomInRange(0, 120);

						if (m_revTimerTarget > 80)
						{
							m_revTimerTarget = RandomInRange(60, 100);
						}
						else if (m_revTimerTarget > 40)
						{
							m_revTimerTarget = RandomInRange(20, 60);	
						}
						else
						{
							m_revTimerTarget = RandomInRange(0, 30);
						}

						m_revState = REV_GENTLE;
						m_revTimer = 0;
					}
				}
				break;

			case REV_GENTLE:
				{
					if (m_revTimer < m_revTimerTarget)
					{
						m_accel = 0.4f;	// 0.
					}
					else
					{
						m_revTimerTarget = RandomInRange(0, 60); // blip

						if (m_revTimerTarget > 50)
						{
							m_revTimerTarget = RandomInRange(50, 70); // blip
						} 
						else if (m_revTimerTarget > 30)
						{
							m_revTimerTarget = RandomInRange(20, 40); // blip
						}
						else if (m_revTimerTarget > 10)
						{
							m_revTimerTarget = RandomInRange(10, 20); // blip
						}
						else 
						{
							m_revTimerTarget = RandomInRange(0, 15); // blip
						}


						m_revState = REV_BLIP;
						m_revTimer = 0;
					}
				}
				break;
			
			case REV_BLIP:
				{
					if (m_revTimer < m_revTimerTarget)
					{
						m_accel = 1.0f;
					}
					else
					{
						m_revTimerTarget = RandomInRange(0, 40);
						m_revTimer = 0;
						m_revState = REV_NONE;
					}
				}
				break;

			case REV_FULL:
				{
					m_accel = 1.0f;
				}
				break;
		}

		pControlState->SetAccel(m_accel);
		pControlState->SetBrake(m_brake);
		pControlState->SetHandbrake(m_handbrake);
	}


	void CAccelBrakeController::Update()
	{
		// TODO : Remember to include the override speed here, maybe do a pd controller for the speed and brake....

		if (m_pIVehicleState && m_pIData && m_pIData->GetControlState()) // TODO [Darren] : Take this out for optimisation
		{	
			if (m_DirectInput == true)
			{
				m_pIData->GetControlState()->GetState()->SetAccel(m_accel);
				m_pIData->GetControlState()->GetState()->SetBrake(m_brake);
				m_pIData->GetControlState()->GetState()->SetHandbrake(m_handbrake);
			}
			else
			{

				// Use a form of traction control.....
				ControlWithoutSliding();	
			}
			
		}
	}
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


