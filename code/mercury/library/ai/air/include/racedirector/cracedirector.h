#ifndef _RACEDIRECTOR_C_H                      
#define _RACEDIRECTOR_C_H               

#ifdef _MSC_VER        
#pragma once          
#endif

#include <racedirector/iracedirector.h>
#include <vector>
#include <vehiclelogic/cvehiclelogic.h> // very bad [Darren] should really use the interface


namespace airacingtech          
{

	class EnterDynamicJunctionMSG;
	class SetRouteMSG;
	class IData;


	BEGIN_COMPONENT_DEC(CRaceDirector, IRaceDirector)
	
	public:
		

		void CachedInterfaces();
		void PreUpdate();
		void Update();
		void PostUpdate();
		void QuickRestart();
		void InitRace();
		void HandleJunctionMessages(EnterDynamicJunctionMSG *pJunctionMessage);
		void HandleSetRoute(SetRouteMSG *pMessage);

		

		inline AIEventBalancing *GetEventBalancing(){return &m_AIEventBalancing;}

		IData *GetAIAtPosition(int rank){return m_AIlist[rank - 1];}

	private:

		void UpdateTrackLogic();
		void SortByLogic();
		void UpdateJunctionRules();
		void CalculateAllRouteSpeeds();
		void CalculateSpeed(int carNo);
		void CalculateTheFrictions(float speed , IData *pIData, float &assumedDeceleration, float &assumedAccel);
		//void SetupSpeedData(int carNo);
		void SortByTrackPosition();

		
		// Car sorting comparison functors
		struct SortCarsByLogic
		{
			bool operator ()(const IData *pCar1, const IData *pCar2)
			{
				airacingtech::IVehicleLogic *pVehicleLogic1 = pCar1->GetVehicleLogic();
				airacingtech::IVehicleLogic *pVehicleLogic2 = pCar2->GetVehicleLogic();

				airacingtech::LogicState *pLogicState1 = pVehicleLogic1->GetLogicState();
				airacingtech::LogicState *pLogicState2 = pVehicleLogic2->GetLogicState();

				float pos1 = pLogicState1->m_logicPositionCombined;
				float pos2 = pLogicState2->m_logicPositionCombined;

				return (pos1 > pos2);
			}
		};


		// Car sorting comparison functors
		struct SortCarsByTrackPosition
		{
			bool operator ()(const IData *pCar1, const IData *pCar2)
			{
				airacingtech::IVehicleLogic *pVehicleLogic1 = pCar1->GetVehicleLogic();
				airacingtech::IVehicleLogic *pVehicleLogic2 = pCar2->GetVehicleLogic();

				airacingtech::LogicState *pLogicState1 = pVehicleLogic1->GetLogicState();
				airacingtech::LogicState *pLogicState2 = pVehicleLogic2->GetLogicState();

				float pos1 = pLogicState1->m_logicPosition;
				float pos2 = pLogicState2->m_logicPosition;

				return (pos1 < pos2);
			}
		};

		




		ITrackDataAI	*m_pTrackData;
		IGameServerTime	*m_pIGameServerTime;

		// using this due to the sort feature, I`m not sure if it`s in btl
		std::vector<IData *>	m_AIlist;
		std::vector<IData *>	m_AIlistTrackPosition;
	
		float					m_JunctionLastTime;
		float					m_TrackLogicLastTime;

		



		AIEventBalancing		m_AIEventBalancing;
	
	END_COMPONENT_DEC

}




#endif //_RACEDIRECTOR_C_H
