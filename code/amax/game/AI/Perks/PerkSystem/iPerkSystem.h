#ifndef _PERKSYSTEM_AI_I_H
#define _PERKSYSTEM_AI_I_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ComponentSystem/componentlib.h>
#include "AI_AmaxSetup.h"

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

	BEGIN_INTERFACE_DEC(IPerkSystem)

	public:
		
		virtual void FireMessages() = 0;
		virtual void AddFireMessage(Game::Actors::C_VehicleLogicActor *pLogicActor, Game::Actors::C_VehicleFirePickupMsg *pMessage) = 0;
		virtual void AddPerkObject(AIAmax::AIPerk aiPerk) = 0;
		virtual int GetNumPerks() = 0;
		virtual AIAmax::AIPerk &GetPerk(int index) = 0;
		virtual void SetAIPerkUsage(bool v) = 0;
		virtual bool GetAIPerkUsage() = 0;

		virtual void PreUpdate() = 0;
		virtual void Update() = 0;
		virtual void PostUpdate() = 0;
		
		virtual bool RequestToUsePerk(int perk, int indexOfAI) = 0;


		virtual int GetEvaluateAiIndex() = 0;
		virtual int GetUpperBatchLimit() = 0;
		virtual int GetLowerBatchLimit()= 0;
		virtual void DeletePerkObject(Actor::C_ActorHandle &handle) = 0;
	
	END_INTERFACE_DEC

}


#endif //_PERKSYSTEM_AI_I_H