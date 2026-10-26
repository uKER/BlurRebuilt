#ifndef _DATA_AI_C_H
#define _DATA_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <data/idata.h>


namespace airacingtech
{
	class IVehicleState;
	class ISplineTrackerAI;
	class IPIDControllerAI;
	class IControlState;
	class IAccelBrakeController;
	class IAvoidance;
	class IParamsAI;
	class ISplineTracker;
	class ISplineFollower;
	class ISpeedTracker;
	class IVehicleLogic;
	class IDebugAI;
	class IRecoveryAI;
	class IRubberbandingAI;
	class IGameBrain;

	BEGIN_COMPONENT_DEC(CData, IData)
	
		void CachedInterfaces();

		inline IVehicleLogic *GetVehicleLogic() const {return m_state.m_pVehicleLogic;}
		inline IVehicleState *GetVehicleState() const {return m_state.m_pIVehicleState;}
		inline IPIDControllerAI  *GetPIDController() const {return m_state.m_pIPIDController;}
		inline IControlState *GetControlState() const {return m_state.m_pIControlState;}
		inline IAccelBrakeController *GetAccelBrakeController() const {return m_state.m_pIAccelBrakeController;}
		inline DataState &GetState(){return m_state;}
		inline IAvoidance *GetAvoidance() const {return m_state.m_pIAvoidance;}
		inline IParamsAI *GetParams() const {return m_state.m_pIParams;}
		inline ISplineTracker	*GetSplineTracker() const {return m_state.m_pISplineTracker;}
		inline ISplineFollower *GetSplineFollower() const {return m_state.m_pISplineFollower;}
		inline ISpeedTracker *GetSpeedTracker() const {return m_state.m_pISpeedTracker;}
		inline IDebugAI *GetDebugAI() const {return m_state.m_pIDebugAI;}
		inline IRecoveryAI *GetRecovery() const {return m_state.m_pIRecovery;}
		inline IRubberbandingAI *GetRubberBanding() const {return m_state.m_pIRubberBanding;}
		inline IIntroOutroSplineAI *GetIntroOutroSplineAI() const {return m_state.m_pIntroOutroSplineAI;}
		inline IGameBrain *GetGameBrainAI() const {return m_state.m_pIGameBrainAI;}
		
		const bool IsPlayer() const ;
	
	
		void SetOverrideSteerPerk(bool Override, float offset, float dist, float secs)
		{
			// needs to be set every frame
			m_state.m_overidePerkSteering.m_setOverride = Override;
			m_state.m_overidePerkSteering.m_offset = offset;
			m_state.m_overidePerkSteering.m_dist = dist;
			m_state.m_overidePerkSteering.m_secs = secs;
		}
		
		inline bool IsInIntro() const
		{
			if (m_state.m_gameState == DataState::INTRO )
				return true;
			else
				return false;
		}
	
		inline bool IsInGame() const
		{
			if (m_state.m_gameState == DataState::GAME )
				return true;
			else
				return false;
		}

		inline bool IsInOutro() const
		{
			if (m_state.m_gameState == DataState::OUTRO )
				return true;
			else
				return false;
		}	



	protected:

		DataState m_state;

	END_COMPONENT_DEC
}




#endif //_DATA_AI_C_H
