#ifndef GAMEDATAFINDER_H
#define GAMEDATAFINDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <ui/Game/UIDataFinder.h>
#include <ui/Game/UIScreenControl.h>
#include <ui/Game/UIVariableStore.h>

#include <gamelogic\racelogiccore\data\RLEnums.h>
#include "GameLogic/RaceLogicCore/Data/RLDefinitions.h"

#include "Game\GameActors\Components\Messages\VehicleLogicMessages.h"

#include <misc_shared/Messaging/MessageMultiHandler.h>

namespace Game
{
	namespace Actors
	{
		class RaceLogicCore;
	};
};

namespace UI
{

class C_GameDataFinder : public C_UIBaseGameDataFinder, public Messaging::C_MessageMultiHandler
{
public:
	static C_GameDataFinder&	Instance()						{ static C_GameDataFinder gameDataFinder; return gameDataFinder; }

private:
	C_GameDataFinder() : C_UIBaseGameDataFinder( "GAME_" )		{  }
	C_GameDataFinder( const C_UIDataFinder& o )					{  }

public:
	void						Init();
	void						InitMessaging();
	void						RemoveMessaging();

	const WCHAR*				GetVariableText( const char* pVariableName );

	void						FormatRaceRewards( UI::C_WideString512& variableText, const AMAX_SCORING_EVENT eType, const S32 sModifier );

	void						FormatPickupInfo(UI::C_WideString512& variableText, const Game::Actors::RaceLogicCore *pRLC, const U8 vehicleIndex, const U8 pickupPodSlotIndex);

	bool MessageHandler(const Messaging::C_MessageBase* pMessage);

	//const C_VehicleLogicPickUpInfoMsg&	GetPickupData( const S32 sVehicleIndex ) { return m_PickupData[ sVehicleIndex ]; }

private:
	// pickup data
	//Btl::Vector< C_VehicleLogicPickUpInfoMsg, Btl::VectorParameters_Array< MAX_VEHICLES > >		m_PickupData;

	E_PICKUP_TYPE				mCurrentPickupType;
	unsigned char				mPowerLevel;
	Btl::Vector<float>			mPower;

	F32							m_RaceStartsInTime;
};

}; //namespace UI

#endif /* GAMEDATAFINDER_H */
