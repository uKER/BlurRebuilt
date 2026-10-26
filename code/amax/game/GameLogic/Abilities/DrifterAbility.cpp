#include "DrifterAbility.h"

#include "Game/Game.h"
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>
#include <Game/GameActors/Components/VehicleLogic/Scoring/VLC_NewScoring.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <GameLogic/Structures/LevelStateMultiplayerAbilities.h>
#include <kanzen/camera/Utility.h>


namespace Game
{

const float 	C_DrifterAbility::sDriftIntensitySmoothingUpwards = 0.1f;
const float 	C_DrifterAbility::sDriftIntensitySmoothingDownwards = 0.5f;
const float 	C_DrifterAbility::sDriftPercentageSmoothingUpwards = 0.1f;
const float 	C_DrifterAbility::sDriftPercentageSmoothingDownwards = 0.4f;

C_DrifterAbility::C_DrifterAbility(
)
{
	mpMultiplayerAbilities = 0;
}

void 
C_DrifterAbility::Init(
	U8 vehicleIndex, 
	const Game::C_MultiplayerAbilities* multiplayerAbilities,
	MultiplayerConfig*					multiplayerConfig,
	Game::Actors::C_VehicleLogicActor*  vehLogicActor,
	Components::C_VehicleComponent_NewScoring* scoringComponent,
	Game::C_LevelStateMultiplayerVehicleAbilities* vehicleAbilities
)
{
	mVehicleIndex			= vehicleIndex;
	mpMultiplayerAbilities	= multiplayerAbilities;
	mDrifterSpecialFX		= -1;
	mDriftPercentageVel		= 0.0f;
	mDriftPercentage		= 0.0f;
	mDriftIntensity			= 0.0f;
	mDriftIntensityVel		= 0.0f;
	mpMpConfig				= multiplayerConfig;
	mpVehicleLogicActor		= vehLogicActor;
	mpScoringComponent		= scoringComponent;
	mpMultiplayerVehicleAbilities = vehicleAbilities;
	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( GameMessaging::E_GameMsg_FinalUpdate );
}


F32
C_DrifterAbility::CalculateDriftPercentage(F32 driftDistance)
{
	F32 driftPercentage = 0.0f;
	XtArray<F32>& drifterLengths = mpMpConfig->m_MiscModSettings.m_DrifterLengths;
	for(S8 i = drifterLengths.count-1; i >= 0; i--)
	{
		if(driftDistance > drifterLengths[i])
		{
			if(driftDistance - mpVehicleLogicActor->GetPositionInfo().mDistanceTravelledSinceLastUpdate <= drifterLengths[i])
			{
				driftPercentage = 1.0f;
			}
			else
			{
				if(i == drifterLengths.count-1)
				{
					driftPercentage = 1.0f;
				}
				else
				{
					F32 upperBounds = drifterLengths[i+1];
					F32 lowerBounds = drifterLengths[i];
					driftPercentage = (driftDistance - lowerBounds)/(upperBounds - lowerBounds);
					driftPercentage = Maths::Clamped<F32>(driftPercentage,0.0f,1.0f);
				}
			}
			break;
		}
		else if( i==0 )
		{
			F32 upperBounds = drifterLengths[i];
			F32 lowerBounds = 0.0f;
			driftPercentage = (driftDistance - lowerBounds)/(upperBounds - lowerBounds);
			driftPercentage = Maths::Clamped<F32>(driftPercentage,0.0f,1.0f);
		}
	}	
	return driftPercentage;
	
}
void 
C_DrifterAbility::Update(
	float frameTime
)
{


	F32 driftDistance = mpScoringComponent->GetDriftDistance();
	F32 driftPercentage = 0.0f;
	const Game::C_MultiplayerAbility* drifterAbility = mpMultiplayerAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Drifter);

	if( mpMultiplayerVehicleAbilities->GetAbility(UI::E_MultiplayerVehicleAbilities_Drifter)->GetActive())
	{
		driftPercentage = CalculateDriftPercentage(driftDistance);
	}


	mDriftPercentage = Kanzen::SmoothingFunction(	mDriftPercentage,
													driftPercentage,
													mDriftPercentageVel,
													driftPercentage>mDriftPercentage?sDriftPercentageSmoothingUpwards:sDriftPercentageSmoothingDownwards,
													frameTime);

	F32 desiredIntensity = 0.0f;
	if( driftPercentage > 0.01f )
	{
		desiredIntensity = 1.0f;

		if( mDriftIntensity > 0.05f )
		{

			if(mDrifterSpecialFX == -1)
			{

				const Maths::HMat43& carMatrix = mpVehicleLogicActor->GetGraphicalActor(mVehicleIndex)->GetMatrix();

				if( drifterAbility )
				{
					if(drifterAbility->GetSpecialFX())
					{
						mDrifterSpecialFX = (int)Game::C_PickupsManager::Instance().AllocateUniqueId(mVehicleIndex);

						// Left Wheel
						Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(	drifterAbility->GetSpecialFX(),
							carMatrix,
							mVehicleIndex,
							-1,
							mDrifterSpecialFX, 
							Game::E_InGameSpecialFX_Trigger_Firing, 
							-1.0f, 
							Maths::HVec3(1.0f,1.0f,1.0f), 
							Maths::HVec3(0.0f,0.0f,0.0f),
							true );

						// Left Wheel
						Game::C_InGameSpecialFXManager::Instance().CreateSpecialFX(	drifterAbility->GetSpecialFX(),
							carMatrix,
							mVehicleIndex,
							-1,
							mDrifterSpecialFX,
							Game::E_InGameSpecialFX_Trigger_Firing, 
							-1.0f, 
							Maths::HVec3(1.0f,1.0f,1.0f), 
							Maths::HVec3(0.0f,0.0f,0.0f),
							false );
					}
				}
			}
		}
	}
	else
	{
		if(mDriftIntensity < 0.01f)
		{	
			if(mDrifterSpecialFX!=-1)
			{
				Game::C_InGameSpecialFXManager::Instance().DestroyPerkForced( mDrifterSpecialFX );

				mDrifterSpecialFX =-1;
			}
		}
	}

	// Update the special FX timeline based on our normalised drift distance.
	if( mDrifterSpecialFX != -1)
	{
		// We use the desired drift to modulate the position of the effect.
		F32 driftValue = mDriftPercentage * drifterAbility->GetSpecialFX()[0].mDurationIntro.mEnd * 0.99f;
		// internally it adds on the frame time so take this off
		driftValue -= frameTime;
		Game::C_InGameSpecialFXManager::Instance().SetCurrentTime(mDrifterSpecialFX, driftValue );
		Game::C_InGameSpecialFXManager::Instance().SetIntensity(mDrifterSpecialFX, mDriftIntensity );
	}

	mDriftIntensity = Kanzen::SmoothingFunction(	mDriftIntensity,
													desiredIntensity,
													mDriftIntensityVel,
													desiredIntensity>mDriftIntensity?sDriftIntensitySmoothingUpwards:sDriftIntensitySmoothingDownwards,
													frameTime);


}

bool 
C_DrifterAbility::MessageHandler(
	const Messaging::C_MessageBase *pMessage
) 
{
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_FinalUpdate:
		{
			const GameMessaging::C_FinalUpdateMsg* msg = Messaging::MessageCast< GameMessaging::C_FinalUpdateMsg >(pMessage);
			Update(msg->GetValue());
		}
		break;
	}

	return true;
}

}
