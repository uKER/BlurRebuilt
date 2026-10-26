#ifndef _MODE_DESTRUCTION_AI_C_H
#define _MODE_DESTRUCTION_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Mode/Destruction/iModeDestruction.h>
#include "AI_AmaxSetup.h"

struct AmaxRaceModeDestruction;

namespace airacingtech
{
	class IGameServerTime;
	class IData;
	class AITrackDataRoutes;
	class ITrackDataAI;

	BEGIN_COMPONENT_DEC(CModeDestruction, IModeDestruction)

	public:

		static const int MAX_DESTRUCTION_ROUTES = 10;
		static const int MAX_VEHICLES_IN_WAVE = 4;
		static const int DESTRUCTION_SAFE_SPAWN_DIST = 150;

		void CachedInterfaces();
		void PreUpdate();
		void Update();
		void PostUpdate();
#ifndef CFG_GOLD
		virtual void DebugRender();
#endif 

		enum DestructionAIType 
		{
			DAIT_None,
			DAIT_Easy,
			DAIT_Medium,
			DAIT_Hard,
			DAIT_VHard,
		};

	private:
		
	
		struct DestructionAIData
		{
			IData *				m_Data;
			DestructionAIType	m_Type;
			int					m_wave;
			int					m_wreckedCycles;
			bool				m_waveCompletable;
		};
		int FindInActivePool(IData *pIDataToDelete);
		void InitialiseMode();
		void PopulatePool();
		void DeActivatePool();
		void DeActivateVehicle(IData *pIDataToDelete, bool wasActive = false);
		bool ActivateWave(int waveIndex);
		void ActivateVehicle(DestructionAIType type, int waveIndex, int rubberBandingIndex, const Maths::HVec3& position, Maths::HVec3& direction, F32 velocity);
		void ReassignToWave(int oldWave, int newWave);
		bool CheckForAnyActive(int waveIndex);
		void CheckForCompletedWaves();
		void RemoveVehiclesBehind();
		int FindFurthestAhead(float &furthestDist);
		void PlaceVehicles(airacingtech::AITrackDataRoutes* pLogicRoute,const airacingtech::AITrackDataRoutes* pRoute, int segment, int waveIndex, DestructionAIType type, Maths::HF32 offsetLateral, bool paddingVehicle = false);
		DestructionAIType GetIthVehicleTypeInWave( int waveIndex, int vehicleIndex );
		
		bool  CanTrackAccomodateBox(AITrackDataRoutes* desiredRoute, IData *pIData, F32 frontDistanceAhead, F32 behindDistanceAhead, F32 width ) const;
		void GetFormationStartAndEndRequired(E_DestructionMode_Wave_Formation formation, int numVehicles, F32& startOffset, F32 &endOffset) const;
		F32 GetFormationWidthRequired(E_DestructionMode_Wave_Formation formation, int numVehicles) const;
		void GetFormationOffsets(E_DestructionMode_Wave_Formation formation,int index, int numVehicles, Maths::HF32& offsetNormal, Maths::HF32& offsetLateral) const;
		void Check();

		void AddVehicleToWorld( int vehicleIdx );
		void RemoveVehicleFromWorld( int vehicleIdx );

		void DisableAutomaticHOGPlacementForAI();
		void CheckForManualHOGPlacement();

		IGameServerTime						*m_pIGameServerTime;
		ITrackDataAI						*m_pITrackData;

		Btl::Vector<DestructionAIData>		m_AIlist;	
		Btl::Vector<DestructionAIData>		m_AIPool;
		Btl::Vector<DestructionAIData>		m_AIPoolActive;
		
		int									m_currentBalancingIndex;
		int									m_currentWave;
		int									m_lastWave;
		const AmaxRaceModeDestruction*		m_DestructionModeSetupData;

		bool								m_startRace;
		float								m_lastTime;
		
	END_COMPONENT_DEC
}



#endif //_MODE_DESTRUCTION_AI_C_H
