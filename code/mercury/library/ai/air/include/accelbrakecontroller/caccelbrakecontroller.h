#ifndef _ACCELBRAKECONTROLLER_AI_C_H
#define _ACCELBRAKECONTROLLER_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif


#include <accelbrakecontroller/iaccelbrakecontroller.h>
#include <fuzzylib/fuzzylibone/fuzzyone.h>
#include <fuzzylib/fuzzylibtwo/FuzzyPolicy.h>






namespace airacingtech
{
	class IVehicleState;
	class IData;

	BEGIN_COMPONENT_DEC(CAccelBrakeController, IAccelBrakeController)

	public:
	

		void CachedInterfaces();
		void Update();

		void ControlWithSliding();
		void ControlWithoutSliding();
		void HoldAIOnTheBrakes();

		void SetControlsDirectly(bool d, float a, float b, float s, bool h) 
		{
			m_DirectInput = d;
			m_accel = a;
			m_brake = b;
			m_steering = s;
			m_handbrake = h;
		}

		float Accel();
		float Brake();
		float ABS(float b);
		float TC(float accel);
		float TCAWD();
		float TCFWD();
		float TCRWD();
		int Gear();
		float AlterSpeed();



	protected:


		float DrivenLateralSlip();
		float DrivenLongSlip();

		float RearLongSlip();
		float FrontLongSlip();
		float FrontLateralSlip();
		float RearLateralSlip();

		IVehicleState *m_pIVehicleState;
		IData *m_pIData;

		float m_accel;
		float m_brake;
		float m_steering;
		bool m_handbrake;
		float m_handbrakeCount;


		float m_accelInc;
		float m_brakeInc;
		float m_hbrakeInc;
		

		bool	m_startAIRace;
		bool	m_DirectInput;



		int	m_HandBrakeState;



		fuzzy_lib_two::FuzzyPolicy		m_fzPolicyThrottle;

		PID								m_PIDThrottle;

		int								m_startTime;

		void QuickRestart();

		

		REV_STATE m_revState;
		int	m_revTimerTarget;
		int m_revTimer;


		
		void Nudged();

		enum NUDGE_STATE
		{
			NUDGE_STATE_NORMAL = 0,
			NUDGE_STATE_SWERVE_RIGHT,
			NUDGE_STATE_SWERVE_LEFT,
			NUDGE_STATE_SWERVE_LEFT_THEN_RIGHT,
			NUDGE_STATE_SWERVE_RIGHT_THEN_LEFT,
			NUDGE_STATE_RETURN_TO_NORMAL
		};

		NUDGE_STATE m_nudgedState;
		float		m_impactSize;
		int			m_swerveTime;
		int			m_startSwerveTime;


	END_COMPONENT_DEC
}




#endif //_ACCELBRAKECONTROLLER_AI_C_H
