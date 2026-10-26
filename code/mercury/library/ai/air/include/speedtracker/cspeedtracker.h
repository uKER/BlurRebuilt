#ifndef _SPEEDTRACKER_AI_C_H
#define _SPEEDTRACKER_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <speedtracker/ispeedtracker.h>

namespace airacingtech
{

	class ITrackDataAI;
	class IData;
	class IVehicleState;
	class VehicleState;
	class ISplineTracker;

	BEGIN_COMPONENT_DEC(CSpeedTracker, ISpeedTracker)

	public:

		void Update();
		
		void CalculateFrictions(float &speed_Coeff, float &decel_co_eff);

	private:

		void CachedInterfaces();
		

		ITrackDataAI		*m_pTrackData;
		IData				*m_pIData;
		IVehicleState		*m_pIVehicleState;
		VehicleState		*m_pVehState;
		ISplineTracker		*m_pSplineTracker;

		Smoother<float>	 *m_pSpeedSmoother;
	
	END_COMPONENT_DEC



}

#endif //_SPEEDTRACKER_AI_C_H
