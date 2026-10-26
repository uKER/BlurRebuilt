#ifndef _DRIVER_AI_C_H
#define _DRIVER_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <driverai/idriverai.h>
#include <data/idata.h>



namespace airacingtech
{

	BEGIN_COMPONENT_DEC(CDriverAI, IDriverAI)

		void Update();
		void PreUpdate();
		void PostUpdate();

		void CalculateSteerState();
		void CalculateCommonVehicleData();
		void UpdateExcitementLevel();
		void UpdateSliding();
		void UpdatePressure();
		void FollowSpline();
		void QuickRestart();
		void RubberBanding();



		void UpdateIntro(DataState &myDataState);
		void UpdateGame(DataState &myDataState);
		void UpdateOutro(DataState &myDataState);

		void PreUpdateIntro(DataState &myDataState);
		void PreUpdateGame(DataState &myDataState);
		void PreUpdateOutro(DataState &myDataState);
		
		void PostUpdateIntro(DataState &myDataState);
		void PostUpdateGame(DataState &myDataState);
		void PostUpdateOutro(DataState &myDataState);

	private:
		IData	*m_pIData;
		
	END_COMPONENT_DEC

	
}




#endif //_DRIVER_AI_C_H
