#include "stdafx.h"

#include "VLC_Tutorials.h"

#include <GameLogic\Structures\LevelState.h>
#include <Game\Achievements\AchievementSystem.h>

#include <GameLogic\GameData\GameData.h>
#include <GameLogic\AchievementsManager.h>

#include "Ui/SoundEffects.h"
#include <Ui/Game/Gadgets/ButtonMessages.h>
#include <Ui/UiInterface.h>

#include <GameLogic/ActorStudioActors/ActorUtils.h>


//#pragma optimize("", off)



namespace Components
{

bool C_VehicleComponent_Tutorials::CreateImpl()
{
	const I_VehicleComponent_Tutorials_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Tutorials_CreateInfo *>(GetCreator().GetData());

	if (creation_data)
	{
		mVehicleIndex		= creation_data->mVehicleIndex;
		mRaceLogicLayerPtr	= creation_data->mRaceLogicLayerPtr;
	}

//	//AddSubscriber( GameMessaging::E_GameMsg_Update );
//	AddSubscriber( Game::E_PickupMessage_PickupPickedUp );

	return true;
}

//==============================================================================================================

bool C_VehicleComponent_Tutorials::OnMessage(const Messaging::C_MessageBase *pMessage) 
{
//	Note: Component only created for player vehicle in single or debug race
	Game::Actors::C_VehicleLogicActor* vehActor	= ((Game::Actors::C_VehicleLogicActor*)GetActor());


	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
		{
			return true;
		}
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

//==============================================================================================================


} // namespace Components
