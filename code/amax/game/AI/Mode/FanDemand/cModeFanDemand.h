#ifndef _MODE_FANDEMAND_AI_C_H
#define _MODE_FANDEMAND_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Mode/FanDemand/iModeFanDemand.h>
#include "AI_AmaxSetup.h"

namespace airacingtech
{
	class IGameServerTime;
	class IData;

	BEGIN_COMPONENT_DEC(CModeFanDemand, IModeFanDemand)

	public:

		void CachedInterfaces();
		void PreUpdate();
		void Update();
		void PostUpdate();


		struct ForDeletion
		{
			float	m_time;
			IData	*m_pIData;
		};

	private:

		int FindInActivePool(IData *pIDataToDelete);
		void InitialiseMode();
		void PopulatePool();
		void DeActivatePool();
		void DeActivateVehicle(IData *pIDataToDelete);
		void ActivateVehicles(int numVehicles);
		void ActivateVehicle(Maths::HMat43 worldMatrixPoint, Maths::HVec3 velocity);
		void AddVehicleToWorld(int vehicleIdx);
		void DeActivateVehicle(int i);
		void RemoveVehiclesBehind();
		int FindFurthestAhead(float &furthestDist);
		void PlaceVehicles(IData *pIData, int seperationDist, Maths::HF32 offsetLateral);
		bool LineOfSight(int vehicleIndex, Maths::HVec3 position, Maths::HVec3 targetPosition, Maths::HVec3 &hitPosition);

		void Check();


		IGameServerTime				*m_pIGameServerTime;
		Btl::Vector<IData *>		m_AIlist;
		Btl::Vector<IData *>		m_AIPool;
		Btl::Vector<IData *>		m_AIPoolActive;
		Btl::Vector<int>			m_PatternSequence;
		int							m_currentPattern;
		int							m_lastPattern;


		bool						m_startRace;
		float						m_lastTime;
		
	END_COMPONENT_DEC
}



#endif //_MODE_FANDEMAND_AI_C_H