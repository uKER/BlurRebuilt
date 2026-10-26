#include "stdafx.h"
#include <Game/Game.h>

#include <ui/Graphics/Localisation/Loc.h>

#include <Game/Messaging/GameflowMessages.h>

#include <ui\Game\UIScreenControl.h>
#include <ui\Game\Gadgets\ButtonMessages.h>

#include <ui\Graphics\Font\Font.h>

#include <GameApp.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>

#include <RaceBook\RaceBookManager.h>

#if 0
#include <GameLogic/InGameCarInfoManager.h>

Debug::Bool g_UiShowCarNames( "Vehicle.Names.Render", false );

namespace Game
{

C_InGameCarInfoManager C_InGameCarInfoManager::s_instance;

void C_InGameCarInfoManager::Init()
{
	SetSubscriberSystem( &UI::gButtons.GetSubscriberSystem() );
	AddSubscriber( UI::E_UiMsg_FrameUpdate );

	SetSubscriberSystem( &C_Game::GetInstance()->GetSubscriberSystem() );
	AddSubscriber( E_GameflowMessage_RaceStarted );
	AddSubscriber( E_GameflowMessage_RaceFinished );
	AddSubscriber( E_GameflowMessage_QuitRace );
	AddSubscriber( E_GameflowMessage_QuitAndWinRace );

	mShowNames=false;

	mFontColour.SetByName( "HUD" );
	mFontStylePasses.m_StyleName.string = "HudNew";
	mFontStylePasses.Setup();
}

void C_InGameCarInfoManager::StoreCarCoords(int index, Maths::HVec3 pos, float fontsize, float alpha)
{
	mCarScreenCoords[index]=pos;
	mCarNameHeights[index]=fontsize;
	mCarNameAlphas[index]=alpha;
}

bool C_InGameCarInfoManager::MessageHandler(const  Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;

	switch( pMessage->Type() )
	{
		case E_GameflowMessage_RaceStarted:
			mShowNames=true;
			break;

		case E_GameflowMessage_QuitAndWinRace:
		case E_GameflowMessage_QuitRace:
		case E_GameflowMessage_RaceFinished:
			mShowNames=false;
			break;

		case UI::E_UiMsg_FrameUpdate:
			{
				if(C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() && mShowNames)
				{
					Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();

					for(unsigned int a=0;a<levelState.mNumCars;a++)
					{
						if ( !g_UiShowCarNames )
							break;

						if (levelState.mNetworkRace)
						{
						}
						else
						{
							mFontColour.SetAlpha( U8(mCarNameAlphas[a]*0xff) );
							UI::gFont.DrawText( UI::E_SpriteLayer_1,mCarScreenCoords[a], UI::E_Justify_BottomCentre, levelState.mCars[a].mNameAboveCar, mFontColour, mCarNameHeights[a], mFontStylePasses );
						}
					}
				}
			}

			break;

		default :
			break;
	}

	return handledMessage;
}

}; // namespace Game
#endif