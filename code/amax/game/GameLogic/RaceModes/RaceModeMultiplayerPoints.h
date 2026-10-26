#ifndef RACEMODE_MULTIPLAYER_POINTS_H
#define RACEMODE_MULTIPLAYER_POINTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <GameLogic/RaceModes/RaceModeMultiplayerRacing.h>
#include <Ui\game\Multiplayer\MultiplayerDefines.h>

namespace Game
{

	class C_RaceModeMultiplayer_Points : public C_RaceModeMultiplayer_Racing
	{
	public:
		void										Initialise(Game::C_LevelState *pLevelState) OVERRIDE;
		void										MpMode_Update();
		void										MpMode_SortResults();

		void										StartLevel();

		bool										MessageHandler(const Messaging::C_MessageBase* pMessage);

	private:

		bool										AddModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, U8 p_VehicleIndex);	//	Add any racemode-specific to the end of the vehicle data net message
		bool										HandleModeSpecificNetMessageBasicVehicleData(RacerNet::C_NetMessage& netMsg, S8 p_VehicleIndex);	//	Read any racemode-specific from the end of the vehicle data net message

		void										CheckTarget();
		void										UpdateLocalFans();

		U32											m_PointsTarget;				//	Target number of points (if applicable)
	};


} // namespace Game


#endif	//	RACEMODE_MULTIPLAYER_POINTS_H