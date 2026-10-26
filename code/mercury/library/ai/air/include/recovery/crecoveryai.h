#ifndef _RECOVERYAI_C_H
#define _RECOVERYAI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <recovery/irecoveryai.h>

namespace airacingtech
{
	class IData;
	class IVehicleState;
	class IControlState;
	class IGameServerTime;
	class ResetInfo;

	BEGIN_COMPONENT_DEC(CRecoveryAI, IRecoveryAI)

	public:

		enum RecoveryState
		{
			DETECT_RECOVERY = 0,
			RECOVERY,
			RECOVERY_COMPLETE,
		};


		void Update();
		inline int GetState(){return m_state;}
		inline void SetState(int state){m_state = (RecoveryState)state;}

		inline ResetInfo *GetResetInfo(){return m_pResetInfo;}
		inline int GetRuleFired(){return m_ruleFired;}

		void QuickRestart();

		inline void SetLastPollTime(float t){m_lastPollTime = t;}
		inline float GetLastPollTime(){return m_lastPollTime;}

	private:

		void CachedInterfaces();
		void DetectRecovery();

		

		
		ResetInfo		*m_pResetInfo;
	
		RecoveryState	m_state;
		float			m_lastTime;
		float			m_lastPollTime;
		float			m_NoRouteLastTime;
		bool			m_NoRoute;
		
		Maths::HVec3	m_lastPosition;
		int				m_ruleFired;

		IData			*m_pIData;
		IVehicleState	*m_pIVehicle;
		IControlState	*m_pIControl;
		IGameServerTime *m_pIGameServerTime;

	END_COMPONENT_DEC
}


#endif //_RECOVERYAI_C_H