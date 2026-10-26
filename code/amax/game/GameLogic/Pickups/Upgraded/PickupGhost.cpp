#include "stdafx.h"

#include <GameLogic/Pickups/Upgraded/PickupGhost.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>

#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceLogicLayer.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/InGameDialogueManager/InGameDialogueManager.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>

#include <RaceBook\RaceBookManager.h>

#include <Vehicle/VehicleModel.h>

//extern Debug::Float g_debugLowSpeedNitrousBoost;

namespace Game
{
	bool C_PickupFireGhost::Update(bool p_IsMultiplayer)
	{
		RLPickupType* pPickup				= (RLPickupType*)C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore()->GetPickupType(E_PICKUP_TYPE_GHOST);
		float frameTime						= C_GameApp::GetInstance()->GetTimeStep();
		C_RaceLogicLayer* pLayer			= C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		Actors::C_VehicleLogicActor* pActor	= pLayer->GetLogicVehicleActor(mFiringVehicle);
		Actors::C_VehicleActor* pVehActor	= pActor->GetGraphicalActor(mFiringVehicle);

		if (!pPickup || mLevel >= pPickup->mLevels.count)
			return true;

		RLGhostBalancingData* pGhostData = (RLGhostBalancingData *)pPickup->mLevels[(int)mLevel]->mBalancingData;

		if(!mTimer)
		{
			pActor->mGhostPoppingActive = true;
			pActor->Ghost(3.0f);
			mMatrix = pLayer->mVehiclePositions[mFiringVehicle].mMatrix;
			SendFirePerkMessage();
		}

		mTimer += frameTime * pGhostData->mSpeed;

		if(mTimer > 3.f)
		{
			return true;
		}

		//TEMP HACK 
		RLBalancingGraph *pGraph = &pPickup->mLevels[(int)mLevel]->mBalancingGraph;

		RLHermiteKeyFrame keyFrame;
		//RLHermiteCurve effectCurve;
		Curve::C_HermiteCurve effectCurve;

		//Build the effect curve
		for(int j = 0; j < pGraph->mEffectGraph.count; j++)
		{
			keyFrame = pGraph->mEffectGraph[j];

			effectCurve.AddKeyFrame(keyFrame.mTime, 
				keyFrame.mValue, 
				keyFrame.mInTangent, 
				keyFrame.mOutTangent);
		}
		//DEBUG DEBUG DEBUG

		Game::Actors::VehiclePerformanceModifiers mods;
		mods.Initialise();
		float boost=effectCurve.Evaluate(mTimer) * pGraph->mMaxBoost;

		Game::Actors::C_VehicleActor *pGActor = pActor->GetGraphicalActor(mFiringVehicle);
		float speedInMetresPerSecond = abs( Magnitude( pGActor->GetVehicleInput()->m_vVelocity ) );
		float mph= (speedInMetresPerSecond * 3600.f) / 1609.f;

		mph=Maths::Clamp(mph,0.f,60.f);

		if(60.f-mph)
		{
			float low_speed_boost=(60.f-mph) * 0.03f;
			boost *= low_speed_boost;
		}

		mods.mBoost.Set(true, boost);

		Game::Actors::C_VehicleUpdatePerformanceModifiersMsg perfModMsg(pActor->GetHandle(), mFiringVehicle, mods);
		/*[ChadW] - BroadcastMessage_CleanUp*///pActor->GetActorList()->BroadcastMessage(&perfModMsg);
		pActor->SendMessageToActor(pActor->GetGraphicalActor(mFiringVehicle)->GetHandle(), &perfModMsg);

		pActor->mPickupInfo.mCurrentlyUsingType=E_PICKUP_TYPE_GHOST;

		return false;
	}
}
