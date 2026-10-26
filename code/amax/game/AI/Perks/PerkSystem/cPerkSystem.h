#ifndef _PERKSYSTEM_AI_C_H
#define _PERKSYSTEM_AI_C_H

#ifdef _MSC_VER
#pragma once
#endif

#include <perks/PerkSystem/iPerkSystem.h>
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <ai/AI_AmaxSetup.h>


namespace Game
{
	namespace Actors
	{
		class C_VehicleFirePickupMsg;
		class C_VehicleLogicActor;
	}
}


namespace airacingtech
{
	class IData;
	class ISystemAI;
	class IGameServerTime;
	class IRaceDirector;
	class ITrackDataAI;



	BEGIN_COMPONENT_DEC(CPerkSystem, IPerkSystem)

	public:

		void AddPerkObject(AIAmax::AIPerk aiPerk)
		{
			m_Perks.PushBack(aiPerk);
		}

		int FindPerkObjectIndex(Actor::C_ActorHandle &handle)
		{
			int index = -1;
			
			for(int i=0; i<m_Perks.Size(); i++)
			{
				if (m_Perks[i].m_actorHandle == handle)
				{
					index = i;
					break;
				}
			}

			return index;
		}

		void DeletePerkObject(Actor::C_ActorHandle &handle)
		{
			int index = FindPerkObjectIndex(handle);
			if (index != -1)
			{
				m_Perks.EraseByIndex(index);
			}
		}

		inline int GetNumPerks(){return m_Perks.Size();}

		inline AIAmax::AIPerk &GetPerk(int index)
		{
			return m_Perks[index]; 
		}

		inline void SetAIPerkUsage(bool v){m_aiDontUsePerks = v;}
		inline bool GetAIPerkUsage(){return m_aiDontUsePerks;}


		void PreUpdate();
		void Update();
		void PostUpdate();
		
		bool RequestToUsePerk(int perk, int indexOfAI);


		int GetEvaluateAiIndex(){return m_EvalauteAI;}

		
		int GetUpperBatchLimit(){return m_upperBatchLimit;}
		int GetLowerBatchLimit(){return m_lowerBatchLimit;}
	


		void AddFireMessage(Game::Actors::C_VehicleLogicActor *pLogicActor, Game::Actors::C_VehicleFirePickupMsg *pMessage)
		{
			m_FireMessages.Add(pLogicActor, pMessage);
		}
		

		void FireMessages();

	private:

		void CachedInterfaces();

		bool IsPlayerTooFarInfront(float &dist);
		void CheckIfPLayerTooFarInfront();
		bool AreCarsGreaterThan(int numCars);
		bool IsPerkOnTrack(int perkType);
		void GrantAndFirePerk(int index, E_PICKUP_TYPE firePerkType, int targetIndex);
		void CheckPlayerWith(E_PICKUP_TYPE firePerkType);

		Btl::Vector<AIAmax::AIPerk>	m_Perks;
		bool						m_aiDontUsePerks;
		int							m_EvalauteAI;

		AIAmax::AIFireMessages				m_FireMessages;

		ITrackDataAI				*m_pTrackData;
		IRaceDirector				*m_pIRaceDirector;
		ISystemAI					*m_pISystem;
		IGameServerTime				*m_pIGameServerTime;
		float						m_lastTime;
		float						m_CheckPlayerLastTime;
		float						m_CheckPlayerLastTimeShunt;
		float						m_CheckPlayerLastTimeShock;
		float						m_IncrementShockTime;
		float						m_IncrementShuntTime;
		bool						m_ShockOnTrack;
		bool						m_ShuntOnTrack;
		int							m_upperBatchLimit;
		int							m_lowerBatchLimit;

	END_COMPONENT_DEC
}



#endif //_PERKSYSTEM_AI_C_H