#include "Stdafx.h"

#include "GameDataFinder.h"

#include <ui/Graphics/Localisation/Loc.h>
#include <ui/Graphics/SimpleTimer.h>
#include <ui/Game/UILUA.h>
#include <ui/Game/StringStore.h>
#include <ui/Game/UIVariableStore.h>
#include <ui/Game/UILocaliser.h>

#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameApp.h>
#include <Game/Game.h>
#include <Game/GameWorld/GameWorld.h>
#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#ifndef CFG_GOLD
#include <GameLogic/Replays/ReplayDirector.h>
#endif // #ifndef CFG_GOLD

#include <GameLogic\RaceRules\RaceRulesManager.h>
#include <GameLogic\Profile\GameProfileManager.h>

#include <AI\AI_Amax.h>

#include <RaceBook\RaceBookManager.h>

#include "../Game/GameActors/Vehicles/VehiclePerformanceModifiers.h"

#include <ui/Game/Profiles/ProfileInterface.h>
#include <GameLogic\Profile\GameProfileManager.h>
#include <ui/UiHelpers.h>
#include "Ui/UiMessages.h"

#include "GameLogic/SinglePlayer/SinglePlayer.h"

#include <Gamelogic\ScoreboardManager.h>

//#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>

#include <actor_system/ActorSys.h>

#include <GameLogic\ContentServer\ContentServerManager.h>

#include <Network/Core.h>
#include <Network/Core/NetScoreboards.h>

#include <Gamelogic\Facebook\FacebookManager.h>
#include <Gamelogic\Twitter\TwitterManager.h>
#include <Gamelogic\SocialNetworking\SocialNetworkManager.h>

#include <GameLogic/SubtitleManager.h>

#include <GameLogic\ContentServer\LinkCodeManager.h>

#include <GameLogic\FriendsChallenges\FriendChallenges.h>
#include <GameLogic\SocialNetworking\FriendsManager.h>
#include <GameLogic\FriendsChallenges\MessagingManager.h>

#include <Gamelogic\SocialNetworking\EventOwnerManager.h>
#include <GameLogic/Profile/MultiplayerProfile.h>

#include <Display/Device/DeviceDebug.h>	// [Blur2PC] Display::g_Resolutions (graphics options labels)

#include "bdLobby\bdLobbyCommon\bdAuthCommon.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif
namespace UI
{

void C_GameDataFinder::Init()
{
	SetSubscriberSystem( &gButtons.GetSubscriberSystem() );
		AddSubscriber( E_UiGameMsg_RaceStartsIn );	

	m_RaceStartsInTime = -1.0f;
}

void C_GameDataFinder::InitMessaging()
{
	if (!C_GameApp::GetInstance()->GetRaceBookLayer() || 
		!C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer() ||
		!C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld() ||
		!C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList())
	{
		BIZ_ASSERT(!"C_GameDataFinder::InitMessaging() - Trying to access NULL pointer - check your flow!");
		return;
	}
	//SetSubscriberSystem( &C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->GetActorList()->GetSubscriberSystem() );
	//AddSubscriber( E_VehicleLogicMsg_PickUpInfo );	
}

void C_GameDataFinder::RemoveMessaging()
{
}

bool C_GameDataFinder::MessageHandler(const Messaging::C_MessageBase* pMessage)
{
	bool handledMessage = true;
	
	switch( pMessage->Type() )
	{
	case E_UiGameMsg_RaceStartsIn:
		m_RaceStartsInTime = (F32)Messaging::MessageCast<C_UiMessageData>( pMessage )->m_Args[ 0 ];
		break;
		
		default:
			handledMessage = false;
			break;
	}

	return handledMessage;
}


const WCHAR* C_GameDataFinder::GetVariableText( const char* pVariableName )
{
	m_VariableText.Clear();

	if ( !strncmp( pVariableName, "NUM_", 4 ) )
	{
		pVariableName += 4;

		// just print a number
		S32 sNumber = (S32)ParseIndex( pVariableName, "" );

		if ( sNumber != -1 )
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( sNumber ) );
	}
	else if ( !strncmp( pVariableName, "PERCENTAGE_", 11 ) )
	{
		pVariableName += 11;

		// just print a number
		S32 sNumber = (S32)ParseIndex( pVariableName, "" );

		if ( sNumber != -1 )
			m_VariableText.Format( L"%s%%", C_UILocaliser::Instance().ConvertNumberToCommas( sNumber ) );
	}
	else
	if ( !strncmp( pVariableName, "POS_", 4 ) )
	{
		pVariableName += 4;

		// just print a position
		S32 sNumber = (S32)ParseIndex( pVariableName, "" );

		if ( sNumber != -1 )
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( sNumber ) );
	}
	else
	if ( !strncmp( pVariableName, "TIME_", 5 ) )
	{
		pVariableName += 5;

		m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( (F32)ParseIndex( pVariableName, "" ) ) );
	}
	else
	if ( !strncmp( pVariableName, "STR_", 4 ) )
	{
#ifndef CFG_GOLD
		// print exactly as it is.
		pVariableName += 4;

		m_VariableText.Format( L"**%S**", pVariableName );
#endif // CFG_GOLD
	}
	else
	if ( !strncmp( pVariableName, "PROGRESS_", 9 ) )
	{
		pVariableName += 9;

		S32 sValue = (S32)ParseIndexAdvance( pVariableName, "" );
		S32 sLimit = (S32)ParseIndexAdvance( pVariableName, "_" );

		UI::C_WideString16 str1( UI::C_UILocaliser::Instance().ConvertNumberToCommas( sValue ) );
		UI::C_WideString16 str2( UI::C_UILocaliser::Instance().ConvertNumberToCommas( sLimit ) );

		m_VariableText.Format( L"%s/%s", str1.Chars(), str2.Chars() );
	}
	else
	if ( !strncmp( pVariableName, "VEHICLE_", 8 ) )
	{
		pVariableName += 8;

		U32 vehicleId = (U32)ParseIndexAdvance( pVariableName, "" );

		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)vehicleId ];

		if ( !pVehicle || !pVehicle->manufacturer )
			return L"";

		if ( !strcmp( pVariableName, "_FULL_NAME" ) )
			m_VariableText.Format( L"%s %s", UI::C_TextStringID( pVehicle->manufacturer->loc_string.id ).String(), UI::C_TextStringID( pVehicle->loc_string.id ).String() );
	}
	else
	if ( !strncmp( pVariableName, "PHOTO_", 6 ) )
	{
		pVariableName += 6;

		if ( !strncmp( pVariableName, "SLOT_NAME_", 10 ) )
		{
			pVariableName += 10;

			S32 sSlot = (S32)ParseIndex( pVariableName, "" );
		
			m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_SLOT_NAME" ).String(), sSlot + 1 );
			
		}
		else if( !strncmp( pVariableName, "SLOT_INFO_", 10 ) )
		{
			pVariableName += 10;

			S32 sSlot = (S32)ParseIndex( pVariableName, "" );
			S32 sPhotoSlot = -1;

			for ( S32 i = 0; i < Game::C_ContentServerManager::Instance().GetNumSearchResults(); i++ )
			{
				bdFileMetaData* md = Game::C_ContentServerManager::Instance().GetMetaData( i );

				if ( md && ( md->m_fileSlot == sSlot ) )
				{
					sPhotoSlot = i;
					break;
				}
			}

			if ( sPhotoSlot == -1 )
			{
				m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_SLOT_EMPTY_INFO" ).String() );
			}
			else
			{
				time_t time = Game::C_ContentServerManager::Instance().GetMetaData( sPhotoSlot )->m_createTime;

				Time::C_DecomposedTime dec_time( 0 );
				dec_time.m_sec 		= gmtime( &time )->tm_sec;
				dec_time.m_min 		= gmtime( &time )->tm_min;
				dec_time.m_hour 	= gmtime( &time )->tm_hour;
				dec_time.m_mday 	= gmtime( &time )->tm_mday;
				dec_time.m_mon		= gmtime( &time )->tm_mon;
				dec_time.m_year		= gmtime( &time )->tm_year;
				dec_time.m_wday		= gmtime( &time )->tm_wday;
				dec_time.m_yday		= gmtime( &time )->tm_yday;
				dec_time.m_isdst	= gmtime( &time )->tm_isdst;

				m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_SLOT_INFO" ).String(), UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( dec_time ) );
			}
		}
		else
		if ( !strncmp( pVariableName, "SEARCH_NAME_", 12 ) )
		{
			pVariableName += 12;

			S32 sSlot = (S32)ParseIndex( pVariableName, "" );

			m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_SEARCH_NAME" ).String(), sSlot + 1 );

		}
		else
		if ( !strncmp( pVariableName, "SEARCH_INFO_", 12 ) )
		{
			pVariableName += 12;

			S32 sSlot = (S32)ParseIndex( pVariableName, "" );

			bdFileMetaData* md = Game::C_ContentServerManager::Instance().GetMetaData( sSlot );

			if ( !md )
				return L"";

			time_t time = md->m_createTime;

			Time::C_DecomposedTime dec_time( 0 );
			dec_time.m_sec		= gmtime( &time )->tm_sec;
			dec_time.m_min		= gmtime( &time )->tm_min;
			dec_time.m_hour		= gmtime( &time )->tm_hour;
			dec_time.m_mday		= gmtime( &time )->tm_mday;
			dec_time.m_mon		= gmtime( &time )->tm_mon;
			dec_time.m_year 	= gmtime( &time )->tm_year;
			dec_time.m_wday 	= gmtime( &time )->tm_wday;
			dec_time.m_yday		= gmtime( &time )->tm_yday;
			dec_time.m_isdst	= gmtime( &time )->tm_isdst;

			UI::C_WideString32 owner_name( md->m_ownerName );

			m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_INFO" ).String(), owner_name.Chars() ); //UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( dec_time ) );
		}
		else if ( !strncmp( pVariableName, "SEARCH_PREVIEW_TITLE_", 21 ) )
		{
			pVariableName += 21;

			S32 sSlot = (S32)ParseIndex( pVariableName, "" );

			bdFileMetaData* md = Game::C_ContentServerManager::Instance().GetMetaData( sSlot );

			if ( !md )
				return L"";

			time_t time = md->m_createTime;

			Time::C_DecomposedTime dec_time( 0 );
			dec_time.m_sec		= gmtime( &time )->tm_sec;
			dec_time.m_min		= gmtime( &time )->tm_min;
			dec_time.m_hour		= gmtime( &time )->tm_hour;
			dec_time.m_mday		= gmtime( &time )->tm_mday;
			dec_time.m_mon		= gmtime( &time )->tm_mon;
			dec_time.m_year 	= gmtime( &time )->tm_year;
			dec_time.m_wday 	= gmtime( &time )->tm_wday;
			dec_time.m_yday		= gmtime( &time )->tm_yday;
			dec_time.m_isdst	= gmtime( &time )->tm_isdst;

			UI::C_WideString32 owner_name( md->m_ownerName );

			m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_SLOT_INFO" ).String(), UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( dec_time ) );
		}
		else
		if ( !strncmp( pVariableName, "LEADERBOARD_DOWNLOAD_", 21 ) )
		{
			pVariableName += 21;

			S32 sSlot = (S32)ParseIndex( pVariableName, "" );

			bdContentStreamingStatsInfo* si = Game::C_ContentServerManager::Instance().GetLeaderboardRow( sSlot );

			if ( !si )
				return L"";

			UI::C_WideString128 name( si->m_fileOwnerName );
			
			m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_BY_DOWNLOAD" ).String(), si->m_rank, name.Chars(), si->m_rating );
		}
		else
		if ( !strncmp( pVariableName, "LEADERBOARD_VOTE_", 17 ) )
		{
			pVariableName += 17;

			S32 sSlot = (S32)ParseIndex( pVariableName, "" );

			Game::csVotingStatsInfo* si = Game::C_ContentServerManager::Instance().GetVotingLeaderboardRow( sSlot );

			if ( !si )
				return L"";

			UI::C_WideString128 name( si->m_fileOwnerName );

			m_VariableText.Format( UI::C_TextStringID( "CMN_PHOTO_BY_VOTE" ).String(), si->m_rank, name.Chars(), si->m_rating / 100000.0f );
		}
	}
	else
	if ( !strcmp( pVariableName, "CONTENT_SERVER_INFO" ) )
	{
		static C_TextStringID s_InfoText( "POP_CONTENT_SERVER_DATA_INFO" );

		m_VariableText.Format( s_InfoText.String(), Game::C_ContentServerManager::Instance().GetPercentageFinished(), Game::C_ContentServerManager::Instance().GetDataRate() / 1024.f );
	}
	else
	if ( !strcmp( pVariableName, "SUBTITLE" ) )
	{
		m_VariableText.Format(Game::C_SubtitleManager::Instance().GetSubtitle());
	}
	else
	if ( !strcmp( pVariableName, "LINK_CODE" ) )
	{
		UI::C_WideString80 str(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mWebsiteLinkCode.Chars());
		m_VariableText.Format(L"%s",str.Chars());

		m_VariableText.Insert(L"-", 5);
		m_VariableText.Insert(L"-", 11);
		m_VariableText.Insert(L"-", 17);
	}
	else
	if ( !strcmp( pVariableName, "321GO_MESSAGE" ) )
	{
		AmaxEvent *ev = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();

	}
	else
	if ( !strcmp( pVariableName, "RACE_STARTS_IN" ) )
	{
		static C_TextStringID s_GoText( "IDS_CMN_GO" );
		if( m_RaceStartsInTime == -1 )
		{
			m_VariableText = s_GoText.String();
		}
		else
		if( m_RaceStartsInTime >= 0 )
		{
			static C_TextStringID m_sCountdownText( "HUD_COUNTDOWN" );

			S64 time = m_RaceStartsInTime * 1000;
			S32 mins = (S32)( time / 60000.0f );
			S32 secs = (S32)( ( time - ( mins * 60000.0f ) ) / 1000.0f );
			S32 msecs = (S32)( time - ( mins * 60000.0f ) - ( secs * 1000.0f ) );
			msecs /= 10;

//			m_VariableText.Format( m_sCountdownText.String(), secs, UI::C_UILocaliser::Instance().GetNumberSeperator(), msecs );
			m_VariableText.Format( m_sCountdownText.String(), secs, '.', msecs ); // #33395, #36937
		}
	}
#ifdef PLATFORM_PC
	else
	if ( !strncmp( pVariableName, "OPTION_", 7 ) )
	{
		// [Blur2PC] retail Blur PC graphics options labels (Blur.exe 1.2, 0x00c50005): resolution names
		// ("1920x1080"), multisample names ("4x"; "0x" shows as Off) and the confirm-settings countdown
		pVariableName += 7;

		if ( !strncmp( pVariableName, "RESOLUTION", 10 ) )
		{
			pVariableName += 10;
			const Display::C_Resolution* pResolution = Display::g_Resolutions.GetResolution( (S32)ParseIndex( pVariableName, "" ) );
			if ( pResolution )
				m_VariableText.Format( L"%S", pResolution->m_Name.Chars() );
		}
		else
		if ( !strncmp( pVariableName, "AATYPE", 6 ) )
		{
			pVariableName += 6;
			const Display::C_MultisampleOption* pMultisample = Display::g_Resolutions.GetMultisample( (S32)ParseIndex( pVariableName, "" ) );
			if ( pMultisample )
			{
				if ( !strcmp( pMultisample->m_Name.Chars(), "0x" ) )
					m_VariableText = C_TextStringID( "CMN_OFF" ).String();
				else
					m_VariableText.Format( L"%S", pMultisample->m_Name.Chars() );
			}
		}
		else
		if ( !strncmp( pVariableName, "COUNTDOWN", 9 ) )
		{
			pVariableName += 9;
			m_VariableText.Format( C_TextStringID( "POP_CONFIRM_VIDEO_SETTINGS" ).String(), (S32)ParseIndex( pVariableName, "" ) );
		}
	}
#endif
	else
	/*--------------------------------------------------------------------------------------
		REPLAY Data
	--------------------------------------------------------------------------------------*/
	if ( strncmp( pVariableName, "REPLAY", 6 ) == 0 )
	{
		pVariableName += 6;

		if ( strncmp( pVariableName, "_TIME", 5 ) == 0 )
		{
			pVariableName += 5;
		#ifndef CFG_GOLD
			F32 replayTime = Game::C_ReplayDirector::Instance().GetReplayTime(); // milli-seconds
			m_VariableText.Format( L"Time : %s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime( replayTime, false ) );
		#endif // CFG_GOLD
		}
	}

	/*--------------------------------------------------------------------------------------
		HUD Data
	--------------------------------------------------------------------------------------*/
	else if ( !strncmp( pVariableName, "HUD", 3 ) )
	{
		// Offset to removed "HUD" and the "."
		pVariableName += 4;

		// Store race logic core for ease of use.
		Game::Actors::RaceLogicCore* pRLC = C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetRaceLogicCore();
		if( !pRLC )
			return m_VariableText = L"****BROKEN****";

		//DEBUG DEBUG DEBUG
		int viewportIndex = 0;
		U8 vehicleIndex = pRLC->GetViewportFocusVehicleIndex(viewportIndex);
		//DEBUG DEBUG DEBUG

		const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);

		if ( !strcmp( pVariableName, "HUD_VERSION_NAME" ) )
		{
			//Just a debug value so we know which version the debug hud is at, I'm going to do it as names 'cos I'm bored with numbers, besides it works for MS...
			m_VariableText = L"v. Curmudgeon";
		}
		else
		if ( !strcmp( pVariableName, "NEW_PERK_TYPE" ) )
		{
			switch(mCurrentPickupType)
			{
				case E_PICKUP_TYPE_NITROUS:
					m_VariableText.Format( L"~~{A}~~ Nitrous");
					break;
				case E_PICKUP_TYPE_SHOCK:
					m_VariableText.Format( L"~~{A}~~ Shock");
					break;
				case E_PICKUP_TYPE_SHUNT:
					m_VariableText.Format( L"~~{A}~~ Shunt");
					break;
				default:
					m_VariableText.Format( L"~~{A}~~ - - - -");
					break;
			}
		}
		else
		if ( !strcmp( pVariableName, "NEW_PERK_SHIELD" ) )
		{
			m_VariableText.Format( L"~~{X}~~ Shield");
		}
		else
		if ( !strcmp( pVariableName, "NEW_PERK_POWER_BAR" ) )
		{
			m_VariableText.Format( L"Power : ");

			for(int a=0;a<mPowerLevel;a++)
			{
				C_WideString32	temp_str;

				temp_str.Format( L"%0.2f ",mPower[a]);
				m_VariableText.Add( temp_str);
			}
		}
		else
		if ( !strcmp( pVariableName, "SPEED" ) )
		{
			float speed = abs(pLogicActor->GetPositionInfo().mSpeedInMph);

			m_VariableText.Format( L"%03d", (U16)speed );
		}
		else
		if ( !strcmp( pVariableName, "SPEED_MPH" ) )
		{
			float speed = abs(pLogicActor->GetPositionInfo().mSpeedInMph);

			static C_TextStringID s_MphText( "IDS_CMN_MPH" );

			m_VariableText.Format( L"%d %s", (U16)speed, s_MphText.String() );
		}
		else
		if ( !strcmp( pVariableName, "GEAR" ) )
		{
			static C_TextStringID s_GearText( "IDS_CMN_GEAR" );

			S8 gear = pRLC->GetVehicleGear(vehicleIndex);
			if(gear > 0)
				m_VariableText.Format( L"%d %s", gear, s_GearText.String() );
			else if(gear == 0)
				m_VariableText.Format( L"N %s", s_GearText.String() );
			else if(gear < 0)
				m_VariableText.Format( L"R %s", s_GearText.String() );
		}
		else
		if ( !strcmp( pVariableName, "LAP" ) )
		{
			U8 lap = pLogicActor->GetLapInfo().mCurrentLap;
			U8 laps = Game::C_Game::GetInstance()->GetLevelState().mLaps;

			if(lap > laps)
				lap = laps;

			static C_TextStringID s_LapFormatText( "CMN_HUD_LAP_FORMAT" );

			if(laps == 99) //[PK] hack, need to sotrt out a proper system for infinite laps
				m_VariableText.Format( L"--" );
			else
				m_VariableText.Format( s_LapFormatText.String(), lap, laps );
		}
		else
		if ( !strcmp( pVariableName, "GET_READY" ) )
		{
			static C_TextStringID s_GetReady( "CMN_GET_READY" );

			float logicalDistanceToStart = 1.0f - pLogicActor->GetPositionInfo().mLogicPosition;

			U32 distance = (U32)(logicalDistanceToStart * AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline(1));
			if( distance < 100 )
				m_VariableText = s_GetReady;
		}
		else
		if ( !strcmp( pVariableName, "STARLINE_DISTANCE" ) )
		{
			static C_TextStringID s_DistanceFormatText( "CMN_HUD_DISTANCE_D" );

			float logicalDistanceToStart = 1.0f - pLogicActor->GetPositionInfo().mLogicPosition;
			if( pLogicActor->GetPositionInfo().mLogicPosition > 0 )
			{
				U32 distance = (U32)(logicalDistanceToStart * AIAmax::AmaxAIR::Instance().DistanceFromStartOfSpline(1));

				m_VariableText.Format( s_DistanceFormatText.String(), distance );
			}
		}
		else
		if ( !strcmp( pVariableName, "CURRENT_POSITION" ) )
		{
			U8 racePosition = pLogicActor->GetPositionInfo().mRacePosition;

			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToPosition( racePosition ) );
		}
		else
		if ( !strcmp( pVariableName, "POSITION" ) )
		{
			U8 racePosition = pLogicActor->GetPositionInfo().mRacePosition;

			static C_TextStringID s_GamePositionText( "IDS_GAME_POSITION" );

			m_VariableText.Format( s_GamePositionText.String(), C_UILocaliser::Instance().ConvertNumberToPosition( racePosition ) );
		}
		else
		if ( !strcmp( pVariableName, "FINAL_POSITION" ) )
		{
			const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

			if ( results.mCars[vehicleIndex].mStatistics.mWrecked )
			{
				m_VariableText.Format( L"%s", C_TextStringID( "IDS_GAME_WRECKED" ).String() );
			}
			else
				m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( (S64)results.mCars[vehicleIndex].mPosition ) );
		}
		else
		if ( !strcmp( pVariableName, "FINAL_POSITION_NUM" ) )
		{
			const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();
			m_VariableText.Format( L"%d", results.mCars[vehicleIndex].mPosition );
		}
		else
		if ( !strcmp( pVariableName, "FINAL_POSITION_NUM_SUFFIX" ) )
		{
			const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();
			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToPosition( (S64)results.mCars[vehicleIndex].mPosition, true, NULL, NULL, false, true ) );
		}
		else
		if ( !strcmp( pVariableName, "FANS" ) )
		{
			const VehicleScoringInfo& scoring = pLogicActor->GetScoring();
			if(scoring.mAllFansGained)
				m_VariableText.Format( L"MAX" );
			else
				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( scoring.mFinalScore ) );
		}
		/*else if ( !strcmp( pVariableName, "LAP_TIME" ) )
		{
			////Check if we've just completed a lap
			//F32 time = 0.f,
			//	splitDifference = 0.f;
			//bool showSplit = false;

			//if( pRLC->IsVehicleRaceStarted(vehicleIndex) )
			//{
			//	if(pRLC->IsVehicleDisplayingLapEnd(vehicleIndex))
			//	{
			//		//Show the lap time as it was at the end of the last lap
			//		time = pRLC->GetVehicleLapTimeAtLastSplitOrLapEnd(vehicleIndex);
			//		splitDifference = pRLC->GetVehicleSplitDifference(vehicleIndex);

			//		if(pLogicActor->GetLapInfo().mCurrentLap > 1)
			//			showSplit = true;
			//	}
			//	else if(pRLC->IsVehicleDisplayingSplit(vehicleIndex))
			//	{
			//		//Show the lap time as it was at the end of the last lap
			//		time = pRLC->GetVehicleLapTimeAtLastSplitOrLapEnd(vehicleIndex);
			//		splitDifference = pRLC->GetVehicleSplitDifference(vehicleIndex);

			//		if(pLogicActor->GetLapInfo().mCurrentLap > 1)
			//			showSplit = true;
			//	}
			//	else
			//	{
			//		time = pRLC->GetVehicleCurrentLapTime(vehicleIndex);
			//	}

			//	if(!showSplit)
			//	{
			//		m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime( time ) );
			//	}
			//	else
			//	{
			//		C_WideString64 strLapTime(C_UILocaliser::Instance().ConvertFloatSecondsToTime(time));
			//		C_WideString64 strSplitTime(C_UILocaliser::Instance().ConvertFloatSecondsToTime(splitDifference));

			//		//m_VariableText.Format( L"%s %s", strLapTime.Chars(), strSplitTime.Chars());
			//		m_VariableText.Format( L"%s", strSplitTime.Chars());
			//	}
			//}
			//else
			//{
			//	m_VariableText.Format( L"%s", L" -- : --" );
			//}
		}*/
		/*else if ( !strcmp( pVariableName, "LAST_LAP_TIME" ) )
		{
			//static C_TextStringID s_LastLapTime( "IDS_GAME_LAST_LAP_TIME_T" );
			//if( pRLC->IsVehicleRaceStarted(vehicleIndex) && pLogicActor->GetLapInfo().mCurrentLap > 1 )	//We need to be on the second lap to have recorded a 'last' lap time
			//{
			//	F32 time = pRLC->GetVehicleLastLapTime(vehicleIndex);
			//	m_VariableText.Format( s_LastLapTime.String(), C_UILocaliser::Instance().ConvertFloatSecondsToTime( time ) );
			//}
			//else
			//{
			//	m_VariableText.Format( s_LastLapTime.String(), L"--:--" );
			//}
		}*/
		/*else if ( !strcmp( pVariableName, "FASTEST_LAP_TIME" ) )
		{
			//static C_TextStringID s_FastestLapTime( "IDS_GAME_FASTEST_LAP_TIME_T" );
			//if( pRLC->IsVehicleRaceStarted(vehicleIndex) && pLogicActor->GetLapInfo().mCurrentLap > 1 )	//We need to be on the second lap to have recorded a 'fastest' lap time
			//{
			//	F32 time = pRLC->GetVehicleFastestLapTime(vehicleIndex);
			//	m_VariableText.Format( s_FastestLapTime.String(), C_UILocaliser::Instance().ConvertFloatSecondsToTime( time ) );
			//}
			//else
			//{
			//	m_VariableText.Format(s_FastestLapTime.String(), L"--:--" );
			//}
		}*/
		/*else if ( !strcmp( pVariableName, "LAST_AND_FASTEST_LAP_TIME" ) )
		{
			//if ( pRLC->IsVehicleRaceStarted( vehicleIndex ) && pLogicActor->GetLapInfo().mCurrentLap > 1 )	// need to be on second lap
			//{
			//	F32 fFastestTime = pLogicActor->GetLapInfo().mBestLapTime;
			//	F32 fLastTime = pRLC->GetVehicleLapTimeAtLastSplitOrLapEnd( vehicleIndex );

			//	static C_TextStringID s_LastLapTimeText( "IDS_GAME_LAST_LAP_TIME_T" );

			//	// last lap
			//	if ( pRLC->IsVehicleDisplayingLapEnd( vehicleIndex ) )
			//	{
			//		m_VariableText.Format( s_LastLapTimeText.String(), C_UILocaliser::Instance().ConvertFloatSecondsToTime( fLastTime ) );
			//		m_VariableText += L" - ";
			//	}

			//	// fastest lap
			//	C_WideString256 string2;
			//	string2.Format( s_LastLapTimeText.String(), C_UILocaliser::Instance().ConvertFloatSecondsToTime( fFastestTime ) );
			//	
			//	m_VariableText += string2;
			//}
		}*/
		/*else if( !strcmp( pVariableName, "SCORE" ) )
		{
			//static C_TextStringID s_ScoreText( "IDS_GAME_SCORE_S" );

			//int score = pRLC->GetVehicleScore(vehicleIndex);
			//m_VariableText.Format( s_ScoreText.String(), C_UILocaliser::Instance().ConvertNumberToCommas( score ) );
		}*/
		/*else if( !strcmp( pVariableName, "XP_TYPE" ) )
		{
			//int score = 0;
			//if( pRLC->IsVehicleRaceStarted(vehicleIndex) )	//We need to be on the second lap to have recorded a 'fastest' lap time
			//	score = pRLC->GetVehicleScore(vehicleIndex);

			//Game::C_RaceBookLayer* pRBLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			//if(pRBLayer)
			//{
			//	m_VariableText.Format( pRBLayer->GetRaceModePtr()->GetScoreString().Chars() );
			//}
		}*/
		/*else if( !strcmp( pVariableName, "RULE_PASSED" ) )
		{
			Game::C_RaceBookLayer* pRBLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			if(pRBLayer)
			{
				m_VariableText.Format( pRBLayer->GetRaceModePtr()->GetRulePassedString(vehicleIndex).Chars() );
			}
		}*/
		/*else if( !strcmp( pVariableName, "XP" ) )
		{
			//int score = 0;
			//if( pRLC->IsVehicleRaceStarted(vehicleIndex) )	//We need to be on the second lap to have recorded a 'fastest' lap time
			//	score = pRLC->GetVehicleScore(vehicleIndex);

			//m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( score ) );
		}*/
		/*else if( !strcmp( pVariableName, "DRIFT_ANGLE" ) )
		{
			//static C_TextStringID s_DriftAngleText( "IDS_GAME_DRIFT_ANGLE_D" );
			//m_VariableText.Format( s_DriftAngleText.String(), (int)pRLC->GetVehicleDriftAngle(vehicleIndex) );
		}*/
		/*else if( !strcmp( pVariableName, "DOT_UP" ) )
		{
			//static C_TextStringID s_GameDotUpText( "IDS_GAME_DOT_UP_F" );
			//m_VariableText.Format( s_GameDotUpText.String(), pRLC->GetVehicleDotUp(vehicleIndex) );
		}*/
		/*else if( !strcmp( pVariableName, "ANGULAR_VELOCITY" ) )
		{
			//Maths::HVec3 angularVelocity = pRLC->GetVehicleAngularVelocity(vehicleIndex);

			//float x = 0.f, y = 0.f, z = 0.f;
			//if( abs(angularVelocity.Z()) > 0.01f)
			//	z = angularVelocity.Z();
			//if( abs(angularVelocity.Y()) > 0.01f)
			//	y = angularVelocity.Y();
			//if( abs(angularVelocity.X()) > 0.01f)
			//	x = angularVelocity.X();

			//m_VariableText.Format( C_TextStringID( "IDS_GAME_ANGULAR_VELOCITY" ).String(), z, y, x );
		}*/
		/*else if( !strcmp( pVariableName, "SLIP_AMOUNT" ) )
		{
			//m_VariableText.Format( C_TextStringID( "IDS_GAME_SLIP_AMOUNT_D" ).String(), pRLC->GetVehicleSlipAmount(vehicleIndex) );
		}*/
		else if ( !strcmp( pVariableName, "WRONG_WAY" ) )
		{
			m_VariableText.Clear();
			
			bool wrongWay = pLogicActor->GetPositionInfo().mGoingWrongWay;
			if ( wrongWay )
				m_VariableText.Format( L"%s", C_TextStringID( "IDS_GAME_WRONG_WAY" ).String() );
		}
		/*else if ( !strcmp( pVariableName, "FATAL_DAMAGE" ) )
		{
			//bool fatalDamage = pRLC->IsVehicleFatallyDamaged(vehicleIndex);
			//if ( fatalDamage )
			//	m_VariableText.Format( L"%s", C_TextStringID( "IDS_GAME_FATAL_DAMAGE" ).String() );
		}*/
		/*else if ( !strcmp( pVariableName, "WRECKED" ) )
		{
			//m_VariableText.Clear();

			//bool fatalDamage = pRLC->IsVehicleFatallyDamaged(vehicleIndex);
			//if ( fatalDamage )
			//	m_VariableText.Format( L"%s", C_TextStringID( "IDS_GAME_WRECKED" ).String() );
		}*/
		else if ( !strcmp( pVariableName, "VIEWING_VEHICLE_INDEX" ) )
		{
			//The vehicle index we're currently chasing (DEBUG)
			m_VariableText.Format( C_TextStringID( "IDS_GAME_VIEWING_VID_D" ).String(), vehicleIndex );
		}
		/*else if ( !strcmp( pVariableName, "AIR_DISTANCE" ) )
		{
			//How far we've flown
			//m_VariableText.Format( C_TextStringID( "IDS_GAME_AIR_DISTANCE_D" ).String(), (int)pRLC->GetVehicleAirDistance(vehicleIndex) );
		}*/
		/*else if ( !strcmp( pVariableName, "DRIFT_DISTANCE" ) )
		{
			//How far we've drifted
			//m_VariableText.Format( C_TextStringID( "IDS_GAME_DRIFT_DISTANCE_D" ).String(), (int)pRLC->GetVehicleDriftDistance(vehicleIndex) );
		}*/
		/*else if ( !strcmp( pVariableName, "WHEELS_ON_GROUND" ) )
		{
			//How many wheels are on the ground
			//m_VariableText.Format( C_TextStringID( "IDS_GAME_WHEELS_ON_GROUND_D" ).String(), pRLC->GetVehicleWheelsOnGround(vehicleIndex) );
		}*/
		/*else if ( !strcmp( pVariableName, "WAYPOINT_INFO" ) )
		{
			//int waypointIndex = pRLC->GetVehicleWaypointIndex(vehicleIndex);
			//float accurateWaypointIndex = pRLC->GetVehicleAccurateWaypoint(vehicleIndex);
			//int trackPercentage = (int)(pRLC->GetVehicleTrackPercentage(vehicleIndex) * 100.f);

			//m_VariableText.Format( C_TextStringID( "IDS_GAME_WAYPOINT_INFO" ).String(), waypointIndex, accurateWaypointIndex, trackPercentage);
		}*/
		/*else if ( !strcmp( pVariableName, "SPLIT_INFO" ) )
		{
			//int nextSplitIndex = pRLC->GetVehicleNextSplitIndex(vehicleIndex);
			//int distanceToNextSplit = (int)pRLC->GetVehicleDistanceToNextSplit(vehicleIndex);

			//int previousSplitIndex = pRLC->GetVehiclePreviousSplitIndex(vehicleIndex);
			//int distanceToPreviousSplit = (int)pRLC->GetVehicleDistanceToPreviousSplit(vehicleIndex);

			//m_VariableText.Format( C_TextStringID( "IDS_GAME_SPLIT_INFO" ).String(), nextSplitIndex, distanceToNextSplit, previousSplitIndex, distanceToPreviousSplit );
		}*/
		/*else if ( !strcmp( pVariableName, "RACE_LAP_SPLIT_INFO" ) )
		{
			
			//int lapNo = pLogicActor->GetLapInfo().mCurrentLap;
			//int realLapNo = pRLC->GetVehicleRealLap(vehicleIndex);

			//int raceSplitNo = pRLC->GetVehicleCurrentRaceSplit(vehicleIndex);
			//int realRaceSplitNo = pRLC->GetVehicleRealRaceSplit(vehicleIndex);
			//
			//static C_TextStringID s_GameRaceLapSplit( "IDS_GAME_RACE_LAP_SPLIT_INFO" );

			//m_VariableText.Format( s_GameRaceLapSplit.String(), lapNo, realLapNo, raceSplitNo, realRaceSplitNo );
		}*/
		/*else if ( !strcmp( pVariableName, "DFS_INFO" ) )
		{
			//float distanceFromStartLine = pRLC->GetVehicleDistanceFromStartLine(vehicleIndex);
			//int racePercentage = (int)(pRLC->GetVehicleRacePercentage(vehicleIndex) * 100.f);
			//int wrongWayDistance  = (int)pRLC->GetVehicleWrongWayDistance(vehicleIndex);

			//C_WideString16 details = L"";
			//if(pRLC->IsVehicleGoingWrongWay(vehicleIndex))
			//	details = L"**WRONG WAY**";

	
			//m_VariableText.Format( C_TextStringID( "IDS_GAME_DFS_INFO" ).String(), distanceFromStartLine, racePercentage, wrongWayDistance, details.Chars() );
		}*/
		/*else if(strcmp(pVariableName, "CAR_AHEAD") == 0)
		{
			//C_WideString128 details = L" - ";

			////Check we're not in first (so there's definitely someone ahead)
			//if(pLogicActor->GetPositionInfo().mRacePosition > 1)
			//{
			//	//See if there is a vehicle ahead
			//	int vehicleIndexAhead = pRLC->GetVehicleIndexAhead(vehicleIndex);
			//	SplitTimeData	lastSplitData, 
			//					vehicleAheadsSplitDataAtOurLastSplit;

			//	lastSplitData.Initialise();
			//	vehicleAheadsSplitDataAtOurLastSplit.Initialise();
			//	int raceSplitNumber = -1;

			//	if(vehicleIndexAhead >= 0)
			//	{
			//		////Get the race split number that we've just crossed
			//		//lastSplitData = pRLC->GetVehicleLastSplitTimeData(vehicleIndex);
			//		////Get the race split number of the split we just crossed
			//		//raceSplitNumber = lastSplitData.mRaceSplitNumber;
			//		////Now get the split data from the vehicleAhead 
			//		//vehicleAheadsSplitDataAtOurLastSplit = pRLC->GetVehicleSplitTimeDataAtRaceSplitNumber(vehicleIndexAhead, lastSplitData.mRaceSplitNumber);
			//	}						

			//	//Work out if we've just completed a split
			//	bool displayLapOrSplit = false;
			//	if( pRLC->IsVehicleDisplayingLapEnd(vehicleIndex) || pRLC->IsVehicleDisplayingSplit(vehicleIndex) )
			//		displayLapOrSplit = true;
			//	
			//	//If there's a car ahead we want to show an accurate time to it based upon the race time the vehicle ahead crossed the split
			//	if( displayLapOrSplit 
			//		&& 
			//		vehicleIndexAhead >= 0 
			//		&& 
			//		vehicleAheadsSplitDataAtOurLastSplit.mRaceSplitNumber >= 0
			//		&&
			//		lastSplitData.mRaceSplitNumber >= 0)
			//	{
			//		//We've just crossed a split so show the actual gap as the vehicle's crossed this race split
			//		float timeGap = lastSplitData.mRaceTime - vehicleAheadsSplitDataAtOurLastSplit.mRaceTime;

			//		details.Format(L"%3.02f s", timeGap);
			//	}
			//	else
			//	{
			//		//We haven't just crossed a split so just show the normal display

			//		//float distanceToCarAhead = pRLC->GetVehicleDistanceToVehicleAhead(vehicleIndex);
			//		//float timeToCarAhead = pRLC->GetVehicleTimeToVehicleAhead(vehicleIndex);

			//		//int vehicleAheadIndex = pRLC->GetVehicleIndexAhead(vehicleIndex);
			//		//int lapsBetween = pRLC->GetVehicleLapsBetween(vehicleIndex, vehicleAheadIndex);

			//		//if(lapsBetween != 0)
			//		//{
			//		//	if(timeToCarAhead == 0.f)
			//		//		details.Format(L"(%d lap) %3.1f m (-.-- s)", lapsBetween, distanceToCarAhead);							
			//		//	else
			//		//		details.Format(L"(%d lap) %3.1f m (%3.02f s)", lapsBetween, distanceToCarAhead, timeToCarAhead);
			//		//}
			//		//else
			//		//{
			//		//	if(timeToCarAhead == 0.f)
			//		//		details.Format(L"%3.1f m (-.-- s)", distanceToCarAhead);
			//		//	else
			//		//		details.Format(L"%3.1f m (%3.02f s)", distanceToCarAhead, timeToCarAhead);
			//		//}
			//	}
			//}

			//m_VariableText.Format( C_TextStringID( "IDS_GAME_CAR_AHEAD" ).String(), details.Chars() );
		}*/
		/*else if(strcmp(pVariableName, "CAR_BEHIND") == 0)
		{
			//C_WideString128 details = L" - ";

			////Make sure we're not in last place (so there's someone behind us)
			//if(pLogicActor->GetPositionInfo().mRacePosition < (U8)Game::C_Game::GetInstance()->GetLevelState().mNumCars)
			//{
			//	//See if there is a vehicle ahead
			//	int vehicleIndexBehind = pRLC->GetVehicleIndexBehind(vehicleIndex);
			//	SplitTimeData	ourSplitDataAtVehicleBehindsLastSplit, 
			//					vehicleBehindsLastSplitData;

			//	ourSplitDataAtVehicleBehindsLastSplit.Initialise();
			//	vehicleBehindsLastSplitData.Initialise();
			//	int raceSplitNumber = -1;

			//	if(vehicleIndexBehind >= 0)
			//	{
			//		//Get the split data the vehicle behind has last crossed
			//		vehicleBehindsLastSplitData = pRLC->GetVehicleLastSplitTimeData(vehicleIndexBehind);
			//		//Get the data's race split number
			//		raceSplitNumber = vehicleBehindsLastSplitData.mRaceSplitNumber;
			//		//Now get the split data from our store when we crossed this race split
			//		ourSplitDataAtVehicleBehindsLastSplit = pRLC->GetVehicleSplitTimeDataAtRaceSplitNumber(vehicleIndex, raceSplitNumber);
			//	}						

			//	//Work out if the vehicle behind's just completed a split
			//	bool displayLapOrSplit = false;
			//	if( pRLC->IsVehicleDisplayingLapEnd(vehicleIndexBehind) || pRLC->IsVehicleDisplayingSplit(vehicleIndexBehind) )
			//		displayLapOrSplit = true;

			//	//See if the vehicle behind has recently passed a split
			//	if( displayLapOrSplit 
			//		&& 
			//		vehicleIndexBehind >= 0 
			//		&& 
			//		vehicleBehindsLastSplitData.mRaceSplitNumber >= 0
			//		&&
			//		ourSplitDataAtVehicleBehindsLastSplit.mRaceSplitNumber >= 0)
			//	{
			//		//We've just crossed a split so show the actual gap as the vehicle's crossed this race split
			//		float timeGap = vehicleBehindsLastSplitData.mRaceTime - ourSplitDataAtVehicleBehindsLastSplit.mRaceTime;

			//		details.Format(L"%3.02f s", timeGap);
			//	}
			//	else
			//	{
			//		float distanceToCarBehind = pRLC->GetVehicleDistanceToVehicleBehind(vehicleIndex);
			//		float timeToCarBehind = pRLC->GetVehicleTimeToVehicleBehind(vehicleIndex);
			//		int vehicleBehindIndex = pRLC->GetVehicleIndexBehind(vehicleIndex);
			//		int lapsBetween = pRLC->GetVehicleLapsBetween(vehicleIndex, vehicleBehindIndex);

			//		
			//		if(lapsBetween != 0)
			//		{
			//			if(timeToCarBehind == 0.f)
			//			{
			//				details.Format(L"(%d lap) %3.1f m (-.-- s)", lapsBetween, distanceToCarBehind);
			//			}
			//			else
			//			{
			//				details.Format(L"(%d lap) %3.1f m (%3.02f s)", lapsBetween, distanceToCarBehind, timeToCarBehind);
			//			}
			//		}
			//		else
			//		{
			//			if(timeToCarBehind == 0.f)
			//			{
			//				details.Format(L"%3.1f m (-.-- s)", distanceToCarBehind);
			//			}
			//			else
			//			{
			//				details.Format(L"%3.1f m (%3.02f s)", distanceToCarBehind, timeToCarBehind);
			//			}
			//		}
			//	}
			//}

			//m_VariableText.Format( C_TextStringID( "IDS_GAME_CAR_BEHIND" ).String(), details.Chars() );
		}*/
		else if(strcmp(pVariableName, "PICKUP_INFO0") == 0)
		{
			FormatPickupInfo(m_VariableText, pRLC, vehicleIndex, 0);
		}
		else if(strcmp(pVariableName, "PICKUP_INFO1") == 0)
		{
			FormatPickupInfo(m_VariableText, pRLC, vehicleIndex, 1);
		}
		else if(strcmp(pVariableName, "PICKUP_INFO2") == 0)
		{
			FormatPickupInfo(m_VariableText, pRLC, vehicleIndex, 2);
		}
		/*else if(strcmp(pVariableName, "PICKUP") == 0)
		{
			//const RLPickupPod pickupPod = pRLC->GetVehiclePickupPod(vehicleIndex);
		
			//C_WideString32 pickupName = L"NONE";
			//C_WideString32 levelName = L"";
			//float progress = 0.f;

			//m_VariableText.Clear();

			//if(pickupPod.meType > E_PICKUP_TYPE_NONE)
			//{
			//	const RLPickupType *pPickupType = pRLC->GetPickupType(pickupPod.meType);
			//	pickupName = C_TextStringID( pPickupType->mLoc_string.id ).String();

			//	switch(pickupPod.meLevel)
			//	{
			//	case E_PICKUP_LEVEL_NONE:
			//		{
			//			levelName = L"Level 0";
			//			break;
			//		}
			//	case E_PICKUP_LEVEL_ONE:
			//		{
			//			levelName = L"Level 1";
			//			break;
			//		}
			//	case E_PICKUP_LEVEL_TWO:
			//		{
			//			levelName = L"Level 2";
			//			break;
			//		}
			//	case E_PICKUP_LEVEL_THREE:
			//		{
			//			levelName = L"Level 3";
			//			break;
			//		}
			//	}
			//	progress = pickupPod.mPercentProgress;
			//	
			//	m_VariableText.Format( L"%s (%s)", pickupName.Chars(), levelName.Chars());
			//}
			//
		}*/
		/*else if(strcmp(pVariableName, "DAMAGE_INFO") == 0)
		{
			//
			//C_WideString128 damageInfo = L"";
			//C_WideString32 extraInfo = L"";
			//C_WideString8 damageLevel = L"";
			//C_WideString32 lastEvent = L"";

			//const Game::Actors::C_VehicleLogicActor* pLogicActor = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer()->GetLogicVehicleActor(vehicleIndex);

			//S32 damagePoints = pLogicActor->GetCurrentHealth();
			//S32 maxDamagePoints = pLogicActor->GetMaxHealth();

			//float damagePercent = 0.f;
			//if(maxDamagePoints > 0.f)
			//{
			//	damagePercent = ((float)damagePoints)/((float)maxDamagePoints);
			//	damagePercent *= 100;
			//}
			//enum E_HEALTH_LEVEL eDamageLevelZone = pLogicActor->GetCurrentHealthLevel();

			//switch(eDamageLevelZone)
			//{
			//case E_HEALTH_LEVEL_LOW:
			//	{
			//		damageLevel = L"L";
			//		break;
			//	}
			//case E_HEALTH_LEVEL_MEDIUM:
			//	{
			//		damageLevel = L"M";
			//		break;
			//	}
			//case E_HEALTH_LEVEL_HIGH:
			//	{
			//		damageLevel = L"H";
			//		break;
			//	}
			//}

			//RLDamageEvent lastDamageEvent = pRLC->GetVehicleLastDamageEvent(vehicleIndex);
			//switch(lastDamageEvent.meImpactLevel)
			//{
			//case E_IMPACT_LEVEL_LIGHT:
			//	lastEvent = "Lgt";
			//	break;
			//case E_IMPACT_LEVEL_MEDIUM:
			//	lastEvent = "Med";
			//	break;
			//case E_IMPACT_LEVEL_HEAVY:
			//	lastEvent = "Hvy";
			//	break;
			//default:
			//	lastEvent = "-";
			//	break;
			//}

			//if( pRLC->IsVehicleFatallyDamaged(vehicleIndex) )
			//{
			//	extraInfo = L"*DEAD*";
			//}
			//else if( pRLC->IsVehicleInvincible(vehicleIndex) )
			//{
			//	extraInfo = L"*INVINCIBLE*";
			//}
			//
			//damageInfo.Format(L" P: %d(%d%%) Lst:%s (%s DMG) %s", damagePoints, (int)damagePercent, lastEvent.Chars(), damageLevel.Chars(), extraInfo.Chars());

			//m_VariableText.Format( C_TextStringID( "IDS_GAME_DAMAGE_INFO" ).String(), damageInfo.Chars() );
		}*/
	}
	else if ( !strncmp( pVariableName, "RULES", 5 ) )
	{
		// Offset to removed "RULES" and the "."
		pVariableName += 6;

		/*if ( !strncmp( pVariableName, "PRERACE", 7 ) )
		{
			S32 sIndex = (S32)ParseIndex( pVariableName, "PRERACE_" );

			//if ( sIndex != -1 )
			//{
			//	m_VariableText.Format(L"%s", Game::C_RaceRulesManager::Instance().GetPassRuleString(sIndex - 1, Game::RACE_MODE_RULE_STRING_TYPE_PRERACE).Chars() );
			//}
		}
		else*/ /*if ( !strcmp( pVariableName, "HUD" ) )
		{
			//m_VariableText.Clear();
			//for(int a=0;a<Game::C_RaceRulesManager::Instance().GetNumPassRules();a++)
			//{
			//	m_VariableText.Add( Game::C_RaceRulesManager::Instance().GetPassRuleString(a, Game::RACE_MODE_RULE_STRING_TYPE_HUD));
			//	m_VariableText.Add( L"~~/n~~" );
			//}
		}
		else*/ if ( !strncmp( pVariableName, "RESULTS", 7 ) )
		{
			S32 sIndex = (S32)ParseIndex( pVariableName, "RESULTS_" );
			const Game::C_LevelResult& results = Game::C_Game::GetInstance()->GetLevelResult();

			// -------- DEBUG ---------
			Game::Actors::RaceLogicCore* pRLC = GetRLC();

			if ( pRLC )
			{
				int viewportIndex = 0;
				U8 vehicleIndex = pRLC->GetViewportFocusVehicleIndex(viewportIndex);
				// -------- DEBUG ---------

				if ( sIndex != -1 )
				{
					Game::C_RaceRulesManager::Instance().GetPassRuleString( vehicleIndex, m_VariableText, sIndex - 1, Game::RACE_MODE_RULE_STRING_TYPE_RESULTS );
				}
			}
		}
	}
	/*else if ( !strncmp( pVariableName, "PRERACE", 7 ) )
	{
		//// Offset to removed "RULES" and the "."
		//pVariableName += 8;

		//if(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEvent->GetType() != RaceBook::E_RaceBookEventType_Sorcery)
		//	return L"";

		//U32 eventId = ((RaceBook::C_RaceBookSorceryEvent*)Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mCurrentEvent)->mAmaxEventID;
		//AmaxEvent *ev = C_GameEventData::Instance().mEvents[(GAMEDATA_ID)eventId];

		//if(ev)
		//{
		//	if ( !strcmp( pVariableName, "CASH_BONUS" ) )
		//	{
		//		m_VariableText.Clear();
		//		for(int a=0;a<ev->cash_reward.count;a++)
		//		{
		//			C_WideString512	temp_str;
		//			temp_str.Format(L"%s : $%d    --    ",UI::C_UILocaliser::Instance().ConvertNumberToPosition(a+1),ev->cash_reward[a]);
		//			
		//			m_VariableText.Add( temp_str);
		//		}
		//	}
		//	if ( !strcmp( pVariableName, "XP_BONUS" ) )
		//	{
		//		m_VariableText.Clear();
		//		for(int a=0;a<ev->xp_reward.count;a++)
		//		{
		//			C_WideString512	temp_str;
		//			temp_str.Format(L"%s : %dxp    --    ",UI::C_UILocaliser::Instance().ConvertNumberToPosition(a+1),ev->xp_reward[a]);
		//			
		//			m_VariableText.Add( temp_str);
		//		}
		//	}
		//}
	}*/
	else if ( strncmp( pVariableName, "REWARDS_INDEX_", 14 ) == 0 ) 
	{
		pVariableName += 14;

		const S32 playerIndex = (S32)ParseIndexAdvance( pVariableName, "" );

		const S32 vehicleIndex = GetPlayerVehicleIndex( playerIndex );
		const Game::Actors::C_VehicleLogicActor* pLogicActor = GetVehicleLogicActor( vehicleIndex );

		if ( pLogicActor )
		{
			//'Fans Coming towards you at the end of the race' code
			//const Components::C_VehicleComponent_Scoring* pScoring = (Components::C_VehicleComponent_Scoring*)pLogicActor->QueryInterface(Components::E_IF_VehicleComponent_Scoring);

			//const S32 rewardIndex = (S32)ParseIndex( pVariableName, "_" );
			//const Components::AmaxScoreBuffer& item = pScoring->AllScoresBuffer()[rewardIndex];

			//static C_TextStringID sfan_text( "CMN_FAN" );
			//static C_TextStringID sfans_text( "CMN_FANS" );

			//m_VariableText.Format( L"%s %i %s", item.mString.Chars(), item.mFansAdded, ( item.mFansAdded > 1 ) ? sfan_text.String() : sfans_text.String() );
			////FormatRaceRewards( m_VariableText, item.mBase, item.mModifier );
		}
	}
	else 
	if ( !strncmp( pVariableName, "LOBBY_RACESUMMARY", 17 ) )
	{
		pVariableName += 17;

		Game::C_SocialNetworkManager& socialNetworkManager = Game::C_SocialNetworkManager::Instance();

		if( !strncmp( pVariableName, "_NEMESIS", 8 ) )
		{
			pVariableName += 8;

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				S8 NemesisId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mMyNemesis;
				if(NemesisId != -1)
				{
					if(!socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
					{
						m_VariableText.Format(L"%s",socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[NemesisId].mName);
					}	
				}							
			}

		}
		else if( !strncmp( pVariableName, "_PREY", 5 ) )
		{
			pVariableName += 5;

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				S8 PreyId = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mMyPrey;
				if(PreyId != -1)
				{
					if(!socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
					{
						m_VariableText.Format(L"%s",socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[PreyId].mName);
					}	
				}							
			}

		}
		else if( !strncmp( pVariableName, "_HITCOUNT_NEMESIS", 17 ) )
		{
			pVariableName += 17;

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				S32 NemesisHitCount = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNumTimesHitByNemesis;
				if(NemesisHitCount != 0)
				{									
					m_VariableText.Format( C_TextStringID( "MP_LOBBY_RACESUMMARY_HIT_COUNTER" ).String() ,UI::C_UILocaliser::Instance().ConvertNumberToCommas(NemesisHitCount) );

				}							
			}

		}
		else if( !strncmp( pVariableName, "_HITCOUNT_PREY", 14 ) )
		{
			pVariableName += 14;

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				S32 PreyHitCount = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNumTimesHitPrey;
				if(PreyHitCount != 0)
				{									
					m_VariableText.Format( C_TextStringID( "MP_LOBBY_RACESUMMARY_HIT_COUNTER" ).String() ,UI::C_UILocaliser::Instance().ConvertNumberToCommas(PreyHitCount) );

				}							
			}
		}
		else if( !strncmp( pVariableName, "_ROW_POSITION", 13 ) )
		{
			pVariableName += 13;
			
			const S32 row = (S32)ParseIndex( pVariableName, "" );

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;
	

				if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
				{
					if(row <= socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNumCars)
					{
						for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
							vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
						{
							if( (vehicleIterator) != NULL && (vehicleIterator)->mPosition == row)
							{	
								//Get the position
								m_VariableText.Format(L"%s",UI::C_UILocaliser::Instance().ConvertNumberToCommas((vehicleIterator)->mPosition));
								//stop looping through the vector
								break;
							}
						}
					}
				}				
			}

		}
		else if( !strncmp( pVariableName, "_ROW_PLAYER_RANK", 16 ) )
		{
			pVariableName += 16;

			const S32 row = (S32)ParseIndex( pVariableName, "" );

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;

				if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
				{		

					if(row <= socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNumCars)
					{

						for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
							vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
						{
							if((vehicleIterator) != NULL && (vehicleIterator)->mPosition == row)
							{	
								//TODO Get the rank
								m_VariableText.Format(L"%s",UI::C_UILocaliser::Instance().ConvertNumberToCommas((vehicleIterator)->mRank + 1));
								//stop looping through the vector
								break;
							}
						}
					}
				}
			}

		}
		else if( !strncmp( pVariableName, "_ROW_GAMERTAG", 13 ) )
		{
			pVariableName += 13;

			const S32 row = (S32)ParseIndex( pVariableName, "" );

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;

				if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
				{
					if(row <= socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNumCars)
					{
						for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
							vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
						{
							if((vehicleIterator) != NULL && (vehicleIterator)->mPosition == row)
							{	
								//Get the gamertag
								m_VariableText.Format(L"%s",(vehicleIterator)->mName);
								//stop looping through the vector
								break;
							}
						}
					}
				}
			}

		}
		else if( !strncmp( pVariableName,"_ROW_STAT" , 9 ) )
		{
			pVariableName += 9;

			const S32 row = (S32)ParseIndex( pVariableName, "" );

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{
				Btl::Vector<Game::AmaxSocialNetworkRaceVehicleStore, Btl::VectorParameters_Array<MAX_VEHICLES> >::Iterator vehicleIterator;

				if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
				{
					if(row <= socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mNumCars)
					{
						for ( vehicleIterator = socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Begin(); 
							vehicleIterator != socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.End(); vehicleIterator++)
						{
							if((vehicleIterator) != NULL && (vehicleIterator)->mPosition == row)
							{	
								//Get the position
								if(socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mRaceType == UI::E_RaceType_Destruction)
								{//time
									m_VariableText.Format(L"%s",UI::C_UILocaliser::Instance().ConvertNumberToCommas((vehicleIterator)->mStatistic));
								}
								else
								{//points
									m_VariableText.Format(L"%s",UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime((vehicleIterator)->mStatistic));
								}
								
								//stop looping through the vector
								break;
							}
						}
					}
				}
			}
		}
		else if ( strncmp( pVariableName, "_AWARD_OWNER", 12 ) == 0 )
		{
			Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

			const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
			const Game::AwardSelection& awardSelection = awards.GetAwardSelection();

			pVariableName += 12;
			const S32 index = (S32)ParseIndex( pVariableName, "" ) - 1;

			const U8 vehicleIndex = awardSelection.m_Owners[index];


			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{				
				if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
				{
					m_VariableText.Format( socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[vehicleIndex].mName );
				}
			}

			
		}
		else if ( strncmp( pVariableName, "_AWARD_RANK", 11 ) == 0 )
		{
			Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
			Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

			const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
			const Game::AwardSelection& awardSelection = awards.GetAwardSelection();

			pVariableName += 11;
			const S32 index = (S32)ParseIndex( pVariableName, "" ) - 1;

			const U8 vehicleIndex = awardSelection.m_Owners[index];

			if ( !socialNetworkManager.mRaceHistoryStore.Empty() )
			{				
				if( !socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles.Empty() )
				{
					m_VariableText.Format( L"%d", socialNetworkManager.mRaceHistoryStore[socialNetworkManager.GetPrevRaceHistoryStoreIndex()].mVehicles[vehicleIndex].mRank + 1 );
				}
			}

		}
		else if ( strncmp( pVariableName, "_CHALLENGE_MULTIPLE", 19 ) == 0 )
		{

			pVariableName += 19;

			const S32 numChallengesCompleted = (S32)ParseIndex( pVariableName, "" );
		
			m_VariableText.Format( C_TextStringID( "MP_LOBBY_RACESUMMARY_CHALLENGE_MULTIPLE" ).String(), UI::C_UILocaliser::Instance().ConvertNumberToCommas(numChallengesCompleted)  );

		}

	}
	else 
	if ( !strncmp( pVariableName, "MINI_SCOREBOARD", 15 ) )
	{
		pVariableName += 16;

		if ( strncmp( pVariableName, "ROW_GAMERTAG", 12 ) == 0 )
		{
			pVariableName += 12;
			
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentFriendRowAmount() && row < Game::MAX_FRIENDS_SCOREBOARD_ROWS )
			{
				m_VariableText.Format( L"%s", Game::C_ScoreboardManager::Instance().GetFriendsRowData(row).m_Userdata.m_Name );
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_FANS_EARNED", 18 ) == 0 )
		{
			pVariableName += 18;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentFriendRowAmount() && row < Game::MAX_FRIENDS_SCOREBOARD_ROWS )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetFriendsRowData( row );
				if (Game::eSPFansEarned < data.m_Columns.Size())
					m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[Game::eSPFansEarned].m_Data.m_S64));
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_COMPLETION_TIME", 22 ) == 0 )
		{
			pVariableName += 22;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentFriendRowAmount() && row < Game::MAX_FRIENDS_SCOREBOARD_ROWS )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetFriendsRowData( row );
				if (Game::eSPCompTime < data.m_Columns.Size())
					m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(data.m_Columns[Game::eSPCompTime].m_Data.m_S64 / 100.0f));
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_WRECKS", 13) == 0 )
		{
			pVariableName += 13;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentFriendRowAmount() && row < Game::MAX_FRIENDS_SCOREBOARD_ROWS )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetFriendsRowData( row );
				if (Game::eSPWrecks < data.m_Columns.Size())
					m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[Game::eSPWrecks].m_Data.m_S64));
			}
		}
	}
	else 
	if ( !strncmp( pVariableName, "SCOREBOARD", 10 ) )
	{


		const int GAMERANK = 0;
		const int TOTALFANS = 1;
		const int TOTALTIMEPLAYED = 2;
		const int RACEPOINTS = 3;
		const int TOPTHREE = 4;
		const int TOTALRACES = 5;
		const int RACEWINS = 6;
		const int POWERUPHITS = 7;
		const int POWERUPSFIRED = 8;
		const int CARSWRECKED = 9;
		const int LEGENDLEVEL = 10;
		const int TIMETOACHIEVELEGEND = 11;

		pVariableName += 11;
		if ( strncmp( pVariableName, "TITLE", 5 ) == 0 )
		{
			//skip the underscore
			pVariableName += 6;

			WCHAR* Title = NULL;
			WCHAR* Filter = NULL;

			if ( strncmp( pVariableName, "FANS", 4 ) == 0 )
			{
				//skip the underscore
				pVariableName += 5;
				Title = (WCHAR*) C_TextStringID( "MP_SCOREBOARD_NEW_FANS_TOTAL" ).String();
			}
			else if ( strncmp( pVariableName, "POINTS", 6 ) == 0 )
			{
				//skip the underscore
				pVariableName += 7;
				Title = (WCHAR*) C_TextStringID( "MP_SCOREBOARD_NEW_TOTAL_POINTS" ).String();
			}
			else if ( strncmp( pVariableName, "POWERUPS", 8 ) == 0 )
			{
				//skip the underscore
				pVariableName += 9;
				Title = (WCHAR*) C_TextStringID( "MP_SCOREBOARD_TOTAL_POWERUPS" ).String();
			}
			else if ( strncmp( pVariableName, "LEGENDS", 7 ) == 0 )
			{
				//skip the underscore
				pVariableName += 8;
				Title = (WCHAR*) C_TextStringID( "MP_SCOREBOARD_NEW_LEGENDS" ).String();
			}


			if ( strncmp( pVariableName, "FRIENDS", 7 ) == 0 )
			{
				pVariableName += 7;
				Filter = (WCHAR*)C_TextStringID( "MP_SCOREBOARD_FILTER_FRIENDS" ).String();
			}
			else if ( strncmp( pVariableName, "YOU", 3 ) == 0 )
			{
				pVariableName += 3;
				Filter = (WCHAR*)C_TextStringID( "MP_SCOREBOARD_FILTER_YOU" ).String();
			}
			else if ( strncmp( pVariableName, "TOP", 3 ) == 0 )
			{
				pVariableName += 3;
				Filter = (WCHAR*)C_TextStringID( "MP_SCOREBOARD_FILTER_TOP" ).String();
			}
			else if ( strncmp( pVariableName, "LOBBY", 5 ) == 0 )
			{
				pVariableName += 5;
				Filter = (WCHAR*)C_TextStringID( "MP_SCOREBOARD_FILTER_LOBBY" ).String();
			}
			

			if(Title != NULL && Filter != NULL)
			{
				m_VariableText.Format( L"%s %s", Title, Filter );
			}

		}
		else if ( strncmp( pVariableName, "OWNER_GAMERTAG", 14 ) == 0 )
		{
			pVariableName += 14;
			
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				//m_VariableText.Format( L"%s", Game::C_ScoreboardManager::Instance().GetScoreboardOwnerProfile()->m_Name );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_GAMERTAG", 12 ) == 0 )
		{
			pVariableName += 12;
			
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				m_VariableText.Format( L"%s", Game::C_ScoreboardManager::Instance().GetCurrentRowData(row).m_Userdata.m_Name );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_POSITION", 12 ) == 0 )
		{
			pVariableName += 12;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(Game::C_ScoreboardManager::Instance().GetCurrentRowData(row).m_Rank));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_SP_SESSION_POSITION", 23 ) == 0 )
		{
			pVariableName += 23;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentFriendRowAmount() && row < Game::MAX_FRIENDS_SCOREBOARD_ROWS )
			{
				//Stop it displaying "0" in rank, but empty strings for the gamertag and data
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetFriendsRowData( row );
				if (wcslen(data.m_Userdata.m_Name) != 0)
					m_VariableText.Format( L"%d", Game::C_ScoreboardManager::Instance().GetFriendsRowData(row).m_Rank );
			}
		
		}
		else
		if ( strncmp( pVariableName, "ROW_SP_SESSION_RANK", 19 ) == 0 )
		{
			pVariableName += 19;
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;

			Btl::Vector<Game::SessionScoreboardData>& Data = Game::C_ScoreboardManager::Instance().GetSessionScoreboardData();
			if( row < Data.Size())
			{
				m_VariableText.Format(L"%d", (Data[row].m_Rank + 1) );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_SP_SESSION_GAMERTAG", 23 ) == 0 )
		{
			pVariableName += 23;
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;

			Btl::Vector<Game::SessionScoreboardData>& Data = Game::C_ScoreboardManager::Instance().GetSessionScoreboardData();
			if( row < Data.Size())
			{
				m_VariableText.Format(L"%s", Data[row].m_Name );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_SP_SESSION_POINTS", 21 ) == 0 )
		{
			pVariableName += 21;
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;

			Btl::Vector<Game::SessionScoreboardData>& Data = Game::C_ScoreboardManager::Instance().GetSessionScoreboardData();
			if( row < Data.Size())
			{
				m_VariableText.Format(L"%d", Data[row].m_RacePointsThisSession );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_SP_SESSION_WINS", 19 ) == 0 )
		{
			pVariableName += 19;
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;

			Btl::Vector<Game::SessionScoreboardData>& Data = Game::C_ScoreboardManager::Instance().GetSessionScoreboardData();
			if( row < Data.Size())
			{
				m_VariableText.Format(L"%d",Data[row].m_RaceWinsThisSession );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_SP_SESSION_RACES", 20 ) == 0 )
		{
			pVariableName += 20;
			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;

			Btl::Vector<Game::SessionScoreboardData>& Data = Game::C_ScoreboardManager::Instance().GetSessionScoreboardData();
			if( row < Data.Size())
			{
				m_VariableText.Format(L"%d", Data[row].m_RacesPlayedThisSession );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_PLAYER_RANK", 15 ) == 0 )
		{
			pVariableName += 15;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas((Game::C_ScoreboardManager::Instance().GetCurrentRowData(row).m_Columns[GAMERANK].m_Data.m_S64 + 1)));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_TOTAL_FANS", 14 ) == 0 )
		{
			pVariableName += 14;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[TOTALFANS].m_Data.m_S64));
			}
		}	
		else
		if ( strncmp( pVariableName, "ROW_WIN_RACE_RATIO", 18 ) == 0 )
		{
			pVariableName += 18;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%0.2f%%", (data.m_Columns[RACEWINS].m_Data.m_S64/data.m_Columns[TOTALRACES].m_Data.m_S64)*100.0f );
			}
		}	
		else
		if ( strncmp( pVariableName, "ROW_POWERUP_HIT_RATIO", 21 ) == 0 )
		{
			pVariableName += 21;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				if(data.m_Columns[POWERUPSFIRED].m_Data.m_S64 > 0.0f)
				{
					m_VariableText.Format(L"%0.2f%%", (float)((float)data.m_Columns[POWERUPHITS].m_Data.m_S64/(float)data.m_Columns[POWERUPSFIRED].m_Data.m_S64)*100.0f );
				}
				else
				{	
					m_VariableText.Format(L"%0.2f%%", 0.0f);
					
				}
			}
		}	
		else
		if ( strncmp( pVariableName, "ROW_TOTAL_RACE_TIME", 19 ) == 0 )
		{
			pVariableName += 19;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );

				S64 TotalSeconds = ( data.m_Columns[ TOTALTIMEPLAYED ].m_Data.m_S64 / SYS_USEC_PER_SEC );
				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTimeDHM( TotalSeconds ) );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_RACEPOINTS", 14 ) == 0 )
		{
			pVariableName += 14;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[RACEPOINTS].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_TOPTHREE", 12 ) == 0 )
		{
			pVariableName += 12;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[TOPTHREE].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_TOTALRACES", 14 ) == 0 )
		{
			pVariableName += 14;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[TOTALRACES].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_RACEWINS", 12 ) == 0 )
		{
			pVariableName += 12;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[RACEWINS].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_POWERUPHITS", 15 ) == 0 )
		{
			pVariableName += 15;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[POWERUPHITS].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_POWERUPSFIRED", 17 ) == 0 )
		{
			pVariableName += 17;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[POWERUPSFIRED].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_CARSWRECKED", 15 ) == 0 )
		{
			pVariableName += 15;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[CARSWRECKED].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_LEGENDLEVEL", 15 ) == 0 )
		{
			pVariableName += 15;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[LEGENDLEVEL].m_Data.m_S64));
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_TIMETOACHIEVELEGEND", 23 ) == 0 )
		{
			pVariableName += 23;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );

				S64 TotalSeconds = ( data.m_Columns[ TIMETOACHIEVELEGEND ].m_Data.m_S64 );
				m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTimeDHM( TotalSeconds ) );
			}
		}
		else
		if ( strncmp( pVariableName, "ROW_FIELD", 9 ) == 0 )
		{
			pVariableName += 9;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );

				switch( Game::C_ScoreboardManager::Instance().GetLookAtScoreboard() )
				{
					case RacerNet::eMpScoreboardPerkRatio :
						{
							m_VariableText.Format( L"%0.3f%%", data.m_Rating/10000.f );
							break;
						}
					case RacerNet::eMpScoreboardTotalWins :
					case RacerNet::eMpScoreboardTotalFans :
					case RacerNet::eMpScoreboardLegends:
					case RacerNet::eMpScoreboardLobby:
						{
							m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertNumberToCommas( data.m_Rating ) );
							break;
						}
				}
			}
		}
		else 
		if ( !strncmp( pVariableName, "TITLE", 5 ) )
		{
			switch(Game::C_ScoreboardManager::Instance().GetLookAtScoreboard())
			{
				case RacerNet::eMpScoreboardPerkRatio:
					m_VariableText.Format(C_TextStringID( "MP_SCOREBOARD_TOTAL_POWERUPS" ).String());
					break;
				case RacerNet::eMpScoreboardTotalWins:
					m_VariableText.Format(C_TextStringID( "MP_SCOREBOARD_NEW_TOTAL_POINTS" ).String());
					break;
				case RacerNet::eMpScoreboardTotalFans:
					m_VariableText.Format(C_TextStringID( "MP_SCOREBOARD_NEW_FANS_TOTAL" ).String());
					break;
				case RacerNet::eMpScoreboardLegends:
					m_VariableText.Format(C_TextStringID( "MP_SCOREBOARD_NEW_LEGENDS" ).String());
					break;
				case RacerNet::eMpScoreboardLobby:
					m_VariableText.Format(C_TextStringID( "MP_SCOREBOARD_NEW_LOBBY" ).String());
					break;
				default:
					m_VariableText.Clear();
					break;
			}
		}
		else 
		if ( !strncmp( pVariableName, "TOTAL", 5 ) )
		{
			m_VariableText.Format(C_TextStringID( "MP_LEADERBOARD_SIZE_D" ).String(),Game::C_ScoreboardManager::Instance().GetCurrentScoreboardSize());
		}
		else
		if(strncmp( pVariableName, "ROW_SP_FANS_EARNED", 18 ) == 0 )
		{
			pVariableName += 18;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[Game::eSPFansEarned].m_Data.m_S64));
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_LAP_TIME", 15 ) == 0 )
		{
			pVariableName += 15;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(data.m_Columns[Game::eSPLapTime].m_Data.m_S64 / 100.0f));
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_COMPLETION_TIME", 22 ) == 0 )
		{
			pVariableName += 22;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(data.m_Columns[Game::eSPCompTime].m_Data.m_S64 / 100.0f));
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_WRECKS", 13) == 0 )
		{
			pVariableName += 13;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				m_VariableText.Format(L"%s", UI::C_UILocaliser::Instance().ConvertNumberToCommas(data.m_Columns[Game::eSPWrecks].m_Data.m_S64));
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_VEHICLE_ID", 17 ) == 0 )
		{
			pVariableName += 17;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				S64 VehicleId = data.m_Columns[Game::eSPVehicleId].m_Data.m_S64;

				AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)VehicleId ];

				if(pVehicle)
				{
					if ( pVehicle->locStringFull.id != 0 )
					{
						m_VariableText.Format(L"%s", UI::C_TextStringID(pVehicle->locStringFull.id).String());  
					}
					else
					{
						m_VariableText.Format(L"%s %s", UI::C_TextStringID(pVehicle->manufacturer->loc_string.id).String(), UI::C_TextStringID(pVehicle->loc_string.id).String());
					}
				}
				else
				{
					m_VariableText.Format(L"ERROR");

				}				
			}
		}
		else
		if(strncmp( pVariableName, "ROW_SP_DIFFICULTY_LEVEL", 23 ) == 0 )
		{
			pVariableName += 23;

			const S32 row = (S32)ParseIndex( pVariableName, "" ) - 1;
			if( row < Game::C_ScoreboardManager::Instance().GetCurrentRowAmount() )
			{
				Scoreboard::RowData data = Game::C_ScoreboardManager::Instance().GetCurrentRowData( row );
				Game::GAME_DIFFICULTY difficulty = (Game::GAME_DIFFICULTY)data.m_Columns[Game::eSPDifficultyLevel].m_Data.m_S64;

				switch(difficulty)
				{
				case Game::GAME_DIFFICULTY_VERY_EASY:
					{
						m_VariableText.Format(C_TextStringID( "CMN_DIFFICULTY_1" ).String());
					}
					break;
					
				case Game::GAME_DIFFICULTY_EASY:
					{
						m_VariableText.Format(C_TextStringID( "CMN_DIFFICULTY_2" ).String());
					}
					break;

				case Game::GAME_DIFFICULTY_NORMAL: 
					{
						m_VariableText.Format(C_TextStringID( "CMN_DIFFICULTY_3" ).String());
					}
					break;

				case Game::GAME_DIFFICULTY_HARD:
					{
						m_VariableText.Format(C_TextStringID( "CMN_DIFFICULTY_4" ).String());
					}
					break;

				case Game::GAME_DIFFICULTY_HARDCORE:
					{
						m_VariableText.Format(C_TextStringID( "CMN_DIFFICULTY_5" ).String());
					}
					break;

				default:
					{
						//shouldn't really be getting here!
						m_VariableText.Format(C_TextStringID( "CMN_DIFFICULTY_2" ).String());
					}
					break;
					
				}			
			}
		}
		if(strncmp( pVariableName, "ROW_BLANK", 9 ) == 0 )
		{
			pVariableName += 9;			
			m_VariableText.Format(L"--");
		}		
	}
	else if ( !strncmp( pVariableName, "UNLOCKS", 7 ) )
	{
		pVariableName += 8;

		if(!strncmp( pVariableName, "FANS_", 5 ) )
		{
			pVariableName += 5;	

			S32 sNumber = (S32)ParseIndex( pVariableName, "" );
			if ( sNumber != -1 )
				m_VariableText.Format( C_TextStringID( "SP_SGOAL_PAR" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( sNumber ) );
		}
		else if(!strncmp( pVariableName, "CURRENT_RANK_", 13 ) )
		{
			pVariableName += 13;

			U32 sID = (U32)ParseIndex( pVariableName, "" );
			m_VariableText.Format( L"%s", C_TextStringID( sID ).String() );
			
		}
		else if(!strncmp( pVariableName, "NEXT_RANK_", 10 ) )
		{
			pVariableName += 10;

			S32 sNumber = (S32)ParseIndexAdvance( pVariableName, "" );
			if( sNumber != -1 && !strncmp( pVariableName, "_TO_", 4 ) )
			{
				pVariableName += 4;
				U32 sID = (U32)ParseIndex( pVariableName, "" );
				m_VariableText.Format( C_TextStringID( "SP_NEXT_RANK" ).String(), C_UILocaliser::Instance().ConvertNumberToCommas( sNumber ), C_TextStringID( sID ).String() );
			}

		}
	}
	else if ( !strncmp( pVariableName, "REWARDS", 7 ) )
	{
		// Offset to removed "REWARDS" and the "."
		pVariableName += 8;

		const Game::C_LevelState& levelState = Game::C_Game::GetInstance()->GetLevelState();
		const Game::C_LevelResult& results=Game::C_Game::GetInstance()->GetLevelResult();
		int pos=results.mCars[0].mPosition;

		AmaxEvent *ev = Game::C_GameProfileManager::Instance().GetPrimaryProfile()->GetCurrentEvent();

		if(ev)
		{
			int total_xp=RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[0].mCharacterID)->mLifetimeStatistics.mXpPoints;
			int on_track_xp=results.mCars[0].mScore;

			int position_xp = 0;
			int position_cash = 0;

			//if( pos < ev->xp_reward.count )
			//	position_xp=ev->xp_reward[pos-1];

			//if( pos < ev->cash_reward.count )
			//	position_cash=ev->cash_reward[pos-1];

			int total_cash=RaceBook::C_RaceBookManager::Instance().GetProfileByID((GAMEDATA_ID)levelState.mCars[0].mCharacterID)->mCash;

			if ( !strcmp( pVariableName, "PREVIOUS_XP" ) )
			{
				m_VariableText.Format(L"%d",total_xp-on_track_xp-position_xp);
			}
			else if ( !strcmp( pVariableName, "ON_TRACK_XP" ) )
			{
				m_VariableText.Format(L"%d",on_track_xp);
			}
			else if ( !strcmp( pVariableName, "POSITION_XP" ) )
			{
				m_VariableText.Format(L"%d",position_xp);
			}
			else if ( !strcmp( pVariableName, "TOTAL_XP" ) )
			{
				m_VariableText.Format(L"%d",total_xp);
			}
			else if ( !strcmp( pVariableName, "PREVIOUS_CASH" ) )
			{
				m_VariableText.Format(L"$%d",total_cash-position_cash);
			}
			else if ( !strcmp( pVariableName, "POSITION_CASH" ) )
			{
				m_VariableText.Format(L"$%d",position_cash);
			}
			else if ( !strcmp( pVariableName, "TOTAL_CASH" ) )
			{
				m_VariableText.Format(L"$%d",total_cash);
			}
			else if ( !strcmp( pVariableName, "LEVEL" ) )
			{
				m_VariableText.Format( L"Level: %d", RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mLevel);
			}
			else if ( !strcmp( pVariableName, "NEXT_LEVEL_AT" ) )
			{
				int level=RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mLevel;

				if(level == C_RaceBookData::Instance().mDefaults[0]->experience_per_level.count)
					m_VariableText.Format( L"Maximum Level Reached");
				else
					m_VariableText.Format( L"Next Level @ %dxp", C_RaceBookData::Instance().mDefaults[0]->experience_per_level[level+1]);
			}
			else if ( !strcmp( pVariableName, "LEVELLED_UP" ) )
			{
				if(RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mHasLeveledUp)
					m_VariableText.Format( L"Congratulations you are now Level %d",RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mLevel);
				else
					m_VariableText.Format( L" ");
			}
			else if ( !strcmp( pVariableName, "CAR" ) )
			{
				m_VariableText.Clear();

				if(Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mUnlockedCar)
				{
					m_VariableText.Format(L"You have won the %s",UI::C_TextStringID(C_RaceBookData::Instance().mVehicles[(GAMEDATA_ID)Game::C_GameProfileManager::Instance().GetPrimaryProfile()->mUnlockedCar]->loc_string.id).String());
				}
			}
			else if ( !strcmp( pVariableName, "FANS_ADDED" ) )
			{
				int new_fans=RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mNumFans-RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mNumPreviousFans;
				m_VariableText.Format(L"%d",new_fans);
			}
			else if ( !strcmp( pVariableName, "FANS_DONATIONS" ) )
			{
				float new_cash=RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mBankBalance-RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mPreviousBankBalance;
				m_VariableText.Format(L"= $%s", C_UILocaliser::Instance().ConvertNumberToCommas((S64)new_cash) );
			}
		}
	}
	else if ( !strncmp( pVariableName, "FLOW", 4 ) )
	{
		// Offset to removed "FLOW" and the "."
		pVariableName += 5;

		if ( !strcmp( pVariableName, "STORY_STAGE" ) )
		{
			m_VariableText.Format( L"Story Stage: n/a");
		}
		else if ( !strcmp( pVariableName, "XP" ) )
		{
			m_VariableText.Format( L"XP: %d", RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mLifetimeStatistics.mXpPoints);
		}
		else if ( !strcmp( pVariableName, "CASH" ) )
		{
			m_VariableText.Format( L"Cash: %d", RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mCash);
		}
		else if ( !strcmp( pVariableName, "LEVEL" ) )
		{
			m_VariableText.Format( L"Level: %d", RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mLevel);
		}
		else if ( !strcmp( pVariableName, "NEXT_LEVEL_AT" ) )
		{
			int level=RaceBook::C_RaceBookManager::Instance().GetPlayerProfile()->mLevel;
			m_VariableText.Format( L"Next Level @ %dxp", C_RaceBookData::Instance().mDefaults[0]->experience_per_level[level+1]);
		}
	}
	else
	if( !strncmp( pVariableName, "TIME", 4 ) )
	{
		pVariableName += 5;

		Time::C_DecomposedTime n( Time::GetCurrentTime() );

		if ( !strcmp( pVariableName, "DATE" ) )
		{
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertSystemTimeToDateOnly( n ) );
		}
		if ( !strcmp( pVariableName, "TIME" ) )
		{
			m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertSystemTimeToTimeOnly( n ) );
		}
	}
	else
	if (strncmp( pVariableName, "TWITTER_USERNAME", 16) == 0 )
	{
		m_VariableText.Format( L"%s", Game::C_TwitterManager::Instance().GetUsername() );
	}
	else
	if (strncmp( pVariableName, "TWITTER_PASSWORD_ASTERISK", 25) == 0 )
	{
		m_VariableText.Format(L"");
		for(size_t a=0; a < wcslen(Game::C_TwitterManager::Instance().GetPassword()); a++)
		{
			m_VariableText.Add(L"*");
		}
	}
	else
	if (strncmp( pVariableName, "TWITTER_PASSWORD", 16) == 0 )
	{
		m_VariableText.Format( L"%s", Game::C_TwitterManager::Instance().GetPassword() );
	}
	else
	if ( !strncmp( pVariableName, "TWITTER_MSG_TEXT_DIRECT", 23 ) )
	{
		m_VariableText = Game::C_SocialNetworkManager::Instance().mCurrentMessage.Chars();
	}
	else
	if ( !strncmp( pVariableName, "TWITTER_MSG_TEXT", 16 ) )
	{
		if ( Game::C_SocialNetworkManager::Instance().mCurrentMessage.Length() == 0 )
			return L"";

		static C_TextStringID id( "RBC_TWITTER_MSG" );
		m_VariableText.Format( id.String(), Game::C_SocialNetworkManager::Instance().mCurrentMessage.Chars() );
	}
	else
	if (strncmp( pVariableName, "TWITTER_MSG", 11) == 0 )
	{
		pVariableName += 11;

		const int index = (int)ParseIndex( pVariableName, "" );

		m_VariableText = Game::C_SocialNetworkManager::Instance().GetTweetString( index );
	}
	else
	if ( !strncmp( pVariableName, "TWITTER_ITEM_TEXT", 17 ) )
	{
		m_VariableText.Format( L"%s", Game::C_SocialNetworkManager::Instance().GetCurrentItemText() );
	}
	else
	if (strncmp( pVariableName, "BLURB_NORMAL_MSG", 16) == 0 )
	{
		pVariableName += 16;

		const int index = (int)ParseIndexAdvance( pVariableName, "" );

		if ( !strcmp( pVariableName, "_NAME" ) )
			m_VariableText = Game::C_FriendChallenges::Instance().GetStoreName( index );
		else
		if ( !strcmp( pVariableName, "_DESC" ) )
			m_VariableText = Game::C_FriendChallenges::Instance().GetStoreDesc( index );
		else
		if ( !strcmp( pVariableName, "_SENDER" ) )
			m_VariableText = Game::C_FriendChallenges::Instance().GetStoreSender( index );
		else
		if ( !strcmp( pVariableName, "_ATTEMPTS" ) )
			m_VariableText = Game::C_FriendChallenges::Instance().GetStoreAttempts( index );
		else
		if ( !strcmp( pVariableName, "_TIME_SENT" ) )
			m_VariableText = Game::C_FriendChallenges::Instance().GetStoreTimeSent( index );
	}
	else
	if (strncmp( pVariableName, "BLURB_SERVER_MSG", 16) == 0 )
	{
		pVariableName += 16;

		const int index = (int)ParseIndexAdvance( pVariableName, "" );
		
		if ( Game::C_MessagingManager::Instance().GetCurrentTask() != Game::MESSAGING_TASK_TYPE_NOTHING )
			return L"";

		if ( !strcmp( pVariableName, "_NAME" ) )
			m_VariableText = Game::C_SocialNetworkManager::Instance().GetBlurbServerName( index );
		else
		if ( !strcmp( pVariableName, "_DESC" ) )
			m_VariableText = Game::C_SocialNetworkManager::Instance().GetBlurbServerDesc( index );
		else
		if ( !strcmp( pVariableName, "_SENDER" ) )
			m_VariableText = Game::C_SocialNetworkManager::Instance().GetBlurbServerSender( index );
		else
		if ( !strcmp( pVariableName, "_DAYS_LEFT" ) )
			m_VariableText = Game::C_SocialNetworkManager::Instance().GetBlurbServerDaysLeft( index );
		else
		if ( !strcmp( pVariableName, "_TIME_SENT" ) )
			m_VariableText = Game::C_SocialNetworkManager::Instance().GetBlurbServerTimeSent( index );
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_USERNAME", 17) == 0 )
	{
		m_VariableText.Format( L"%s", Game::C_FacebookManager::Instance().GetUsername() );
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_PASSWORD_ASTERISK", 26) == 0 )
	{
		m_VariableText.Format(L"");
		for(size_t a=0; a < wcslen(Game::C_FacebookManager::Instance().GetPassword()); a++)
		{
			m_VariableText.Add(L"*");
		}
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_PASSWORD", 17) == 0 )
	{
		m_VariableText.Format( L"%s", Game::C_FacebookManager::Instance().GetPassword() );
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_MSG_USERNAME", 21) == 0 )
	{
		pVariableName += 21;

		const int index = (int)ParseIndex( pVariableName, "" );

		if(index < Game::C_SocialNetworkManager::Instance().GetNumSocialItems())
		{
			//Game::AmaxSocialNetworkItem *sni=Game::C_SocialNetworkManager::Instance().GetSocialItem(index);

			//m_VariableText.Format( L"~00000000~%s~r %s", 
			//	C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ).Name(), 
			//	sni->mText.Chars() );
		}
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_MSG", 12) == 0 )
	{
		pVariableName += 12;

		const int index = (int)ParseIndex( pVariableName, "" );

		m_VariableText= Game::C_SocialNetworkManager::Instance().mCurrentMessage.Chars();
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_STORY_USERNAME", 23) == 0 )
	{
		m_VariableText.Format( C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ).Name() );
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_STORY_NAME", 19) == 0 )
	{
		pVariableName += 19;

		const int index = (int)ParseIndex( pVariableName, "" );

		m_VariableText=Game::C_SocialNetworkManager::Instance().GetCurrentItemText();
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_STORY_CAPTION", 22) == 0 )
	{
		pVariableName += 22;

		const int index = (int)ParseIndex( pVariableName, "" );

		const AmaxSocialNetworkingItem *sni_d=Game::C_SocialNetworkManager::Instance().GetCurrentItem();

		if(sni_d)
			m_VariableText.Format( C_TextStringID( sni_d->facebook_story.caption.id ).String() );
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_STORY_DESCRIPTION", 26) == 0 )
	{
		pVariableName += 26;

		const int index = (int)ParseIndex( pVariableName, "" );

		const AmaxSocialNetworkingItem *sni_d=Game::C_SocialNetworkManager::Instance().GetCurrentItem();

		if(sni_d)
			m_VariableText.Format( C_TextStringID( sni_d->facebook_story.description.id ).String() );
	}
	else
	if (strncmp( pVariableName, "FACEBOOK_STORY_INFO", 19) == 0 )
	{
		pVariableName += 19;

		const int index = (int)ParseIndex( pVariableName, "" );

		static C_TextStringID id( "RBU_FACEBOOK_INFO" );
		m_VariableText.Format( id.String(), index, Game::C_SocialNetworkManager::Instance().GetNumSocialItems());
	}

	else
	if (strncmp( pVariableName, "FACEBOOK_PUBLISH_INFO", 21) == 0 )
	{
		float timeout=Game::C_FacebookManager::Instance().GetTimeout();

		if(Game::C_FacebookManager::Instance().HasPublisherFile())
		{
			if(timeout)
			{
				static C_TextStringID id( "RBC_FACEBOOK_UNLOCKS_IN" );
				m_VariableText.Format( id.String(), timeout );
			}
			else
			{
				static C_TextStringID id( "RBC_FACEBOOK" );
				m_VariableText.Format( id.String() );
			}
		}
	}
	else
	if (strncmp( pVariableName, "TWITTER_PUBLISH_INFO", 20) == 0 )
	{
		float timeout=Game::C_TwitterManager::Instance().GetTimeout();

		if(Game::C_TwitterManager::Instance().HasPublisherFile())
		{
			if(timeout)
			{
				static C_TextStringID id( "RBC_TWITTER_UNLOCKS_IN" );
				m_VariableText.Format( id.String(), timeout );
			}
			else
			{
				static C_TextStringID id( "RBC_TWITTER" );
				m_VariableText.Format( id.String() );
			}
		}
	}
	else
	if (strncmp( pVariableName, "SHARE_BUTTON", 20) == 0 )
	{
		C_TextStringID id( "INFO_LB_SHARE" );

		const U8 primaryUserIndex = SysProfile::C_Manager::Instance().GetPrimaryPadNo();
		const SysProfile::FullProfile& fp = UI::C_ProfileInterface::Instance().GetPadProfile(primaryUserIndex);

		bool online_possible = false;

#if defined ( PLATFORM_PS3 )
		
		online_possible = SysProfile::C_Manager::Instance().NetworkPlatformAccountAvailable() & NetPlay::C_Core::Instance().ValidLocalAddress();

#elif defined ( PLATFORM_PC )

		char* userName = Game::C_GameProfileManager::Instance().GetDemonwareUserName();
		char* password = Game::C_GameProfileManager::Instance().GetDemonwarePassword();
		if (userName != NULL && password != NULL)
		{
			online_possible = true;
		}

#endif

		if(fp.m_CommunicationsPrivilege && (Game::C_TwitterManager::Instance().HasPublisherFile() || 
			Game::C_FacebookManager::Instance().HasPublisherFile() || online_possible) )
		{
			m_VariableText.Format( id.String() );
		}
		else
		{
			return L"";
		}
	}

	else
	if ( strncmp( pVariableName, "BEST_LAP", 8 ) == 0 )
	{
		pVariableName += 8;

		const int index = (int)ParseIndex( pVariableName, "" );

		m_VariableText.Format( L"%s", C_UILocaliser::Instance().ConvertFloatSecondsToTime(GetRaceLogicLayer()->GetLogicVehicleActor(index)->GetLapInfo().mBestLapTime));
	}
	else
	if( !strcmp( pVariableName, "FAN_NUM_OF" ) )
	{
		Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if( pLayer )
		{
			static C_TextStringID id( "HUD_CURRENT_POS_OF" );
			m_VariableText.Format( id.String(), pLayer->mNumFanPickups );
		}
	}
	else
	if( !strcmp( pVariableName, "FAN_PICKUPS" ) )
	{
		Game::C_RaceLogicLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer()->GetRaceLogicLayer();
		if( pLayer )
			m_VariableText.Format( L"%d", pLayer->GetLogicVehicleActor(0)->GetPickupInfo().mNumFanPickupsCollected.Size() );
	}
	else
	if ( strncmp( pVariableName, "FRIEND_DEMAND_STORE_", 20 ) == 0 )
	{
		pVariableName += 20;

		if( !strncmp( pVariableName, "DESCRIPTION_", 12 ) )
		{
			pVariableName += 12;

			const int index = (int)ParseIndex( pVariableName, "" );

			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			m_VariableText.Format( L"\"%s\"",fc->mDescription );
		}
		else if( !strncmp( pVariableName, "NAME_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndex( pVariableName, "" );

			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			if(fc->mEventID == 0 )
			{
				m_VariableText.Format( UI::C_TextStringID( "FDE_FRIEND_DEMAND_SLOT_EMPTY_INFO" ).String(), index + 1 );
			}
			else
			{
				m_VariableText.Format( Game::C_FriendChallenges::Instance().GetStoreName(index) );
			}
		}
		else if ( !strncmp( pVariableName, "THE_DEMAND_", 11 ) )
		{
			static C_TextStringID id( "FDE_CHALLENGE" );

			pVariableName += 11;

			const int index = (int)ParseIndex( pVariableName, "" );

			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			UI::C_WideString64 str1(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(3,-1,fc));
			UI::C_WideString64 str2(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(1,-1,fc));
			UI::C_WideString64 str3(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(0,-1,fc));
			UI::C_WideString64 str4(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(2,-1,fc));

			m_VariableText.Format( id.String(), UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ).Name(), str1.Chars(), str2.Chars(), str3.Chars(), str4.Chars());
		}
		else if( !strncmp( pVariableName, "ATTEMPTS_", 9 ) )
		{
			pVariableName += 9;

			const int index = (int)ParseIndex( pVariableName, "" );

			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( fc->mAttempts ) );
		}
		else if( !strncmp( pVariableName, "SENDER_", 7 ) )
		{
			pVariableName += 7;

			const int index = (int)ParseIndex( pVariableName, "" );

			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			m_VariableText.Format( fc->mChallenger );
		}
		else if( !strncmp( pVariableName, "DATE_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndex( pVariableName, "" );

			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			const Time::C_DecomposedTime time( fc->mDate );
			m_VariableText.Format( L"Sent: %s",UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( time ) );
		}
		else if ( !strncmp( pVariableName, "CRITERIA_TITLE_", 15 ) )
		{
			pVariableName += 15;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );

			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetCriteriaTitleString(index, -1, fc) );
		}
		else if ( !strncmp( pVariableName, "CRITERIA_VAR_", 13 ) )
		{
			pVariableName += 13;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );

			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetCriteriaVariableString(index, -1, fc) );
		}
		
		else if ( !strncmp( pVariableName, "PARAM_TITLE_", 12 ) )
		{
			pVariableName += 12;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );
			
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamTitleString(index, -1, fc) );
		}
		else if ( !strncmp( pVariableName, "PARAM_VAR_", 10 ) )
		{
			pVariableName += 10;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );
			
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamVariableString(index, -1, fc) );
		}
		else if ( !strncmp( pVariableName, "PARAM_SWITCH_", 13 ) )
		{
			pVariableName += 13;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			const Game::FriendChallenge *fc=&Game::C_FriendChallenges::Instance().mFriendDemandStore[index];

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );
			
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamSwitchString(index, -1, fc) );
		}
	}
	else
	if ( strncmp( pVariableName, "FRIEND_DEMAND_MESSAGE_", 22 ) == 0 )
	{
		pVariableName += 22;

		if( !strncmp( pVariableName, "DESCRIPTION_", 12 ) )
		{
			pVariableName += 12;

			const int index = (int)ParseIndex( pVariableName, "" );

			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			if(fc)
				m_VariableText.Format( L"\"%s\"",fc->mDescription );
		}
		else if( !strncmp( pVariableName, "NAME_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndex( pVariableName, "" );

			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			if(fc)
				m_VariableText.Format( L"%s's \"%s\"",fc->mChallenger, fc->mName );
		}
		else if ( !strncmp( pVariableName, "THE_DEMAND_", 11 ) )
		{
			static C_TextStringID str_id( "FDE_CHALLENGE" );

			pVariableName += 11;

			const int index = (int)ParseIndex( pVariableName, "" );

			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			UI::C_WideString64 str1(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(3,-1,fc));
			UI::C_WideString64 str2(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(1,-1,fc));
			UI::C_WideString64 str3(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(0,-1,fc));
			UI::C_WideString64 str4(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(2,-1,fc));

			if(fc)
				m_VariableText.Format( str_id.String(), UI::C_ProfileInterface::Instance().GetPadProfile( SysProfile::C_Manager::Instance().GetPrimaryPadNo() ).Name(), str1.Chars(), str2.Chars(), str3.Chars(), str4.Chars());
		}
		else if( !strncmp( pVariableName, "SENDER_", 7 ) )
		{
			pVariableName += 7;

			const int index = (int)ParseIndex( pVariableName, "" );

			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			if(fc)
				m_VariableText.Format( fc->mChallenger );
		}
		else if( !strncmp( pVariableName, "DATE_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndex( pVariableName, "" );

			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			if(fc)
			{
				const Time::C_DecomposedTime time( fc->mDate );
				m_VariableText.Format( L"Sent: %s", UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( time ) );
			}
		}
		else if ( !strncmp( pVariableName, "CRITERIA_TITLE_", 15 ) )
		{
			pVariableName += 15;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );
	
			if(fc)
				m_VariableText.Format( Game::C_FriendChallenges::Instance().GetCriteriaTitleString(index, -1, fc) );
		}
		else if ( !strncmp( pVariableName, "CRITERIA_VAR_", 13 ) )
		{
			pVariableName += 13;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );

			if(fc)
				m_VariableText.Format( Game::C_FriendChallenges::Instance().GetCriteriaVariableString(index, -1, fc) );
		}
		
		else if ( !strncmp( pVariableName, "PARAM_TITLE_", 12 ) )
		{
			pVariableName += 12;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );
			
			if(fc)
				m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamTitleString(index, -1, fc) );
		}
		else if ( !strncmp( pVariableName, "PARAM_VAR_", 10 ) )
		{
			pVariableName += 10;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );
			
			if(fc)
				m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamVariableString(index, -1, fc) );
		}
		else if ( !strncmp( pVariableName, "PARAM_SWITCH_", 13 ) )
		{
			pVariableName += 13;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(index)->m_mailBody;

			pVariableName++;
			index = (int)ParseIndexAdvance( pVariableName, "" );
			
			if(fc)
			{
				if(fc->mParameters[index].mModifier ==  Game::FRIEND_CHALLENGE_MODIFIER_NOT_USED)
				{
					static C_TextStringID id( "FDE_NO_MODIFIERS" );
					m_VariableText.Format( id.String() );
				}
				else
				{
					m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamSwitchString(index, -1, fc) );
				}
			}
		}
	}
	else
	if ( strncmp( pVariableName, "FRIEND_DEMAND_RECHALLENGE_", 26 ) == 0 )
	{
		pVariableName += 26;

		if ( !strncmp( pVariableName, "PRIMARY", 7 ) )
		{
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetPrimaryReChallengeString() );
		}
		else if ( !strncmp( pVariableName, "MEDAL", 5 ) )
		{
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetMedalReChallengeString() );
		}
		else if ( !strncmp( pVariableName, "ATTEMPTS", 8 ) )
		{
			const Game::FriendChallenge *fc=Game::C_FriendChallenges::Instance().GetAttemptingFriendDemand();
			m_VariableText.Format( L"%d", fc->mAttempts );
		}
		else if ( !strncmp( pVariableName, "SECONDARY_TITLE", 15 ) )
		{
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetSecondaryTitleReChallengeString() );
		}
		else if ( !strncmp( pVariableName, "SECONDARY_SWITCH", 16 ) )
		{
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetSecondarySwitchReChallengeString() );
		}
		else if ( !strncmp( pVariableName, "SECONDARY_VAR", 13 ) )
		{
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetSecondaryVarReChallengeString() );
		}
		else if ( !strncmp( pVariableName, "DESC", 4 ) )
		{
			//PK 101, I�ve beaten your challenge in 2 attempts. I re-challenge you to get 2,459 Fans (369 more than you) and a Gold Medal (same as you), whilst still using the Focus RS on Medium difficulty.
			//%s, I�ve beaten your challenge in %d attempts. I re-challenge you to get %s and a %d, whilst still using the %s on %s difficulty.

			static C_TextStringID id( "FDE_RECHALLENGE" );

			const Game::FriendChallenge *fc=Game::C_FriendChallenges::Instance().GetAttemptingFriendDemand();

			UI::C_WideString64 friend_str(fc->mChallenger);
			UI::C_WideString128 primary_str(Game::C_FriendChallenges::Instance().GetPrimaryReChallengeString());
			UI::C_WideString64 medal_str(Game::C_FriendChallenges::Instance().GetMedalReChallengeString());
			UI::C_WideString64 vehicle_str(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(3,-1,fc));
			UI::C_WideString64 diff_str(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(1,-1,fc));

			m_VariableText.Format( id.String(), friend_str.Chars(), fc->mAttempts, primary_str.Chars(), medal_str.Chars(), vehicle_str.Chars(), diff_str.Chars());
		}
	}
	else
	if ( strncmp( pVariableName, "RACE_HISTORY_", 13 ) == 0 )
	{
		pVariableName += 13;

		if ( !strncmp( pVariableName, "TITLE_", 6 ) )
		{
			pVariableName += 6;

			const int index = (int)ParseIndex( pVariableName, "" );

			if(Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
			{
				if(C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRoute])
				{
					m_VariableText.Format( L"%s (%s)",UI::C_TextStringID(C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRoute]->loc_string.id).String(),
														UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mDate ) );
				}
			}
		}
		else if ( !strncmp( pVariableName, "CITY_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndex( pVariableName, "" );

			if(Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
			{
				if(C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRoute])
				{
					m_VariableText.Format( L"%s",UI::C_TextStringID(C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRoute]->city->loc_string.id).String() );
				}
			}
		}
		else if ( !strncmp( pVariableName, "ROUTE_", 6 ) )
		{
			pVariableName += 6;

			const int index = (int)ParseIndex( pVariableName, "" );

			if(Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
			{
				if(C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRoute])
				{
					m_VariableText.Format( L"%s",UI::C_TextStringID(C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRoute]->loc_string.id).String() );
				}
			}
		}
		else if ( !strncmp( pVariableName, "TIME_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndex( pVariableName, "" );

			if(Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
			{
				if(C_GameData::Instance().mRoutes[(GAMEDATA_ID)Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRoute])
				{
					m_VariableText.Format( L"%s",UI::C_UILocaliser::Instance().ConvertSystemTimeToDate( Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mDate ) );
				}
			}
		}
		else if ( !strncmp( pVariableName, "MODE_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndex( pVariableName, "" );

			switch( Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mRaceType )
			{
				case E_RaceType_Racing			: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_RACING" ).String() );			break;
				case E_RaceType_Elimination		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_ELIMINATION" ).String() );		break;
				case E_RaceType_Points			: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_POINTS" ).String() );			break;
				case E_RaceType_Destruction		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_DESTRUCTION" ).String() );		break;
				case E_RaceType_Hardcore		: m_VariableText.Format( C_TextStringID( "MP_RACE_TYPE_HARDCORE" ).String() );			break;
			}
		}
		else if ( !strncmp( pVariableName, "AWARD_", 6 ) )
		{
			pVariableName += 6;

			const int index = (int)ParseIndexAdvance( pVariableName, "" );

			if ( !strncmp( pVariableName, "_NAME_", 6 ) )
			{
				pVariableName += 6;

				const int award_num = (int)ParseIndex( pVariableName, "" );

				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

				const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
				
				UI::E_MultiplayerAward award_type=Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mPlayerAwards[award_num];

				if(award_type != UI::E_MultiplayerAward_None)
				{
					const MultiplayerAwardData* pAward = awards.GetAward(award_type).GetData();
					
					C_TextStringID id( pAward->mAwardName.id );
					m_VariableText.Format( id.String() );
				}
			}
			else if ( !strncmp( pVariableName, "_DESC_", 6 ) )
			{
				pVariableName += 6;

				const int award_num = (int)ParseIndex( pVariableName, "" );

				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

				const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
				
				UI::E_MultiplayerAward award_type=Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mPlayerAwards[award_num];

				if(award_type != UI::E_MultiplayerAward_None)
				{
					const MultiplayerAwardData* pAward = awards.GetAward(award_type).GetData();
					
					C_TextStringID id( pAward->mAwardDescription.id );
					m_VariableText.Format( id.String() );
				}
			}
			else if ( strncmp( pVariableName, "_AMOUNT_", 8 ) == 0 )
			{
				pVariableName += 8;
				const int award_num = (int)ParseIndex( pVariableName, "" );

				Game::C_RaceBookLayer* pLayer = C_GameApp::GetInstance()->GetRaceBookLayer();
				Game::C_GameModeMultiPlayer* pGameMode = (Game::C_GameModeMultiPlayer*)pLayer->GetGameModePtr();

				const Game::C_MultiplayerAwards& awards = pGameMode->GetRaceAwards();
				
				UI::E_MultiplayerAward award_type=Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mPlayerAwards[award_num];
				F32 award_value = Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mPlayerAwardValues[award_num];

				if(award_type != UI::E_MultiplayerAward_None)
				{
					const MultiplayerAwardData* pAward = awards.GetAward(award_type).GetData();

					switch( pAward->mAwardStat )
					{
						case E_MultiplayerStat_Accuracy:
						{
							m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_PERCENTRAGE" ).String(), award_value );
							break;
						}
						case E_MultiplayerStat_TopSpeed:
						{
							m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_SPEED" ).String(), (S32)award_value );
							break;
						}
						case E_MultiplayerStat_DistanceTravelled:
						case E_MultiplayerStat_LongestDrift:
						case E_MultiplayerStat_LongestJump:
						{
							m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_DISTANCE" ).String(), (S32)award_value );
							break;
						}
						case E_MultiplayerStat_KillStreak :
						case E_MultiplayerStat_DeathStreak :
						{
							m_VariableText.Format( C_TextStringID( "MP_AWARD_FORMAT_STREAK" ).String(), (S32)award_value );
							break;
						}
						case E_MultiplayerStat_RaceTime:
						case E_MultiplayerStat_FastestLap:
						case E_MultiplayerStat_SlowestLap:
						case E_MultiplayerStat_TimeInvisibleWithSilentRunning:
						case E_MultiplayerStat_LongestTimeNotDamaged:
						case E_MultiplayerStat_LongestTimeWithoutBeingWrecked:
						case E_MultiplayerStat_TimeInFirstPlace:
						{
							m_VariableText.Format( C_UILocaliser::Instance().ConvertFloatSecondsToTime( award_value ) );
							break;
						}
						default :
						{
							// Default is numbers with commas
							m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( (S32)award_value ) );
							break;
						}
					}
				}
			}
		}
		else if ( !strncmp( pVariableName, "GRID_", 5 ) )
		{
			pVariableName += 5;

			const int index = (int)ParseIndexAdvance( pVariableName, "" );
			
			if(Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
			{
				const Game::AmaxSocialNetworkRaceStore asn_rs=Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1];

				if ( !strncmp( pVariableName, "_POS_", 5 ) )
				{
					pVariableName += 5;

					const int grid_pos = (int)ParseIndex( pVariableName, "" );

					if(grid_pos-1 < Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
					{
						m_VariableText.Format( L"%s.", UI::C_UILocaliser::Instance().ConvertNumberToPosition(grid_pos) );
					}
				}
				else if ( !strncmp( pVariableName, "_NAME_", 6 ) )
				{
					pVariableName += 6;

					const int grid_pos = (int)ParseIndex( pVariableName, "" );
					
					if(grid_pos-1 < Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
					{
						for(int a=0;a<asn_rs.mNumCars;a++)
						{
							if(asn_rs.mVehicles[a].mPosition == grid_pos)
							{
								m_VariableText.Format( L"%s", asn_rs.mVehicles[a].mName );
							}
						}
					}
				}
				else if ( !strncmp( pVariableName, "_TIME_", 6 ) )
				{
					pVariableName += 6;
					
					const int grid_pos = (int)ParseIndex( pVariableName, "" );
					
					if(grid_pos-1 < Game::C_SocialNetworkManager::Instance().mRaceHistoryStore[index-1].mNumCars)
					{
						for(int a=0;a<asn_rs.mNumCars;a++)
						{
							if(asn_rs.mVehicles[a].mPosition == grid_pos)
							{
								//display "points" in motor mash
								if( asn_rs.mRaceType == UI::E_RaceType_Destruction )
								{
									m_VariableText.Format( L"%.0f", asn_rs.mVehicles[a].mStatistic );
								}
								else if(asn_rs.mVehicles[a].mStatistic)
								{
									m_VariableText.Format( L"%s", UI::C_UILocaliser::Instance().ConvertFloatSecondsToTime(asn_rs.mVehicles[a].mStatistic) );
								}
								else
								{
									static C_TextStringID id( "CMN_DNF" );
									m_VariableText.Format( id.String() );
								}
							}
						}

					}
				}
			}
		}
	}
	else
	if ( strncmp( pVariableName, "FRIEND_DEMAND_STATS_", 20 ) == 0 )
	{
		pVariableName += 20;

		if ( !strncmp( pVariableName, "SENT_YOU_", 9 ) )
		{
			pVariableName += 9;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
			if(stats_slot != -1)
				m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mSent );
		}
		else if ( !strncmp( pVariableName, "SENT_THEM_", 10 ) )
		{
			pVariableName += 10;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mSent );
		}
		else if ( !strncmp( pVariableName, "WON_YOU_", 8 ) )
		{
			pVariableName += 8;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
			if(stats_slot != -1)
				m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted );
		}
		else if ( !strncmp( pVariableName, "WON_THEM_", 9 ) )
		{
			pVariableName += 9;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted );
		}
		else if ( !strncmp( pVariableName, "KOS_YOU_", 8 ) )
		{
			pVariableName += 8;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
			if(stats_slot != -1)
				m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mKOs );
		}
		else if ( !strncmp( pVariableName, "KOS_THEM_", 9 ) )
		{
			pVariableName += 9;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mKOs );
		}
		else if ( !strncmp( pVariableName, "MOST_ATTEMPTS_YOU_", 18 ) )
		{
			pVariableName += 18;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
			if(stats_slot != -1)
				m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mMostAttempts );
		}
		else if ( !strncmp( pVariableName, "MOST_ATTEMPTS_THEM_", 17 ) )
		{
			pVariableName += 17;
			
			const int index = (int)ParseIndex( pVariableName, "" );
			m_VariableText.Format( L"%d", Game::C_FriendChallenges::Instance().mTaleOfTheTape.mMostAttempts );
		}
		else if ( !strncmp( pVariableName, "AVG_ATTEMPTS_YOU_", 17 ) )
		{
			pVariableName += 17;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			int stats_slot=Game::C_FriendChallenges::Instance().GetStatSlot(index);
			
			if(stats_slot != -1)
			{
				float avg=0.f;
				if(Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted)
				{
					avg=(float)Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mTotalAttempts / (float)Game::C_FriendChallenges::Instance().mFriendDemandStatsStore[stats_slot].mCompleted; 
					m_VariableText.Format( L"%0.3f", avg );
				}
				else
				{
					m_VariableText.Format( L"0");
				}
			}
		}
		else if ( !strncmp( pVariableName, "AVG_ATTEMPTS_THEM_", 16 ) )
		{
			pVariableName += 16;
			
			const int index = (int)ParseIndex( pVariableName, "" );

			float avg=0.f;
			if(Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted)
			{
				avg=(float)Game::C_FriendChallenges::Instance().mTaleOfTheTape.mTotalAttempts / (float)Game::C_FriendChallenges::Instance().mTaleOfTheTape.mCompleted; 
				m_VariableText.Format( L"%0.3f", avg );
			}
			else
			{
				m_VariableText.Format( L"0");
			}
		}
	}
	else
	if ( strncmp( pVariableName, "FRIEND_COMPARE_MSG", 18 ) == 0 )
	{
		static C_TextStringID id( "POP_FRIEND_COMPARE_CHOSEN_MSG" );

		SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromOnlineID(Game::C_EventOwnerManager::Instance().GetFriend());
		if(prof)
		{
			m_VariableText.Format(id.String(), prof->Name(), prof->Name());
		}
	}
	else
	if ( strncmp( pVariableName, "LEGEND_RESET", 12 ) == 0 )
	{
		static C_TextStringID id( "POP_LEGEND_RESET" );
		
		Game::C_GameProfile* pGameProfile = Game::C_GameProfileManager::Instance().GetPrimaryProfile();
		if(pGameProfile)
		{
			Game::C_MultiplayerProfile* pMultiplayerProfile = pGameProfile->GetMultiplayerProfile();
			if(pMultiplayerProfile)
			{
				S32 nextLegendLevel = pMultiplayerProfile->GetLegendLevel()+1;
				MultiplayerLevel& legendLevelData = C_MultiplayerData::Instance().m_pConfig->m_LegendLevelingData[nextLegendLevel];

				// Find car unlocked at this level (if more than one.. *shouldnt ever be* use first for now)
				XtId vehicleId = legendLevelData.mVehicleUnlocks[0]->ID;
				const AmaxVehicle* vehicle = C_MultiplayerData::Instance().FindVehicle(vehicleId);
				if(vehicle)
				{
					// Format the string with the next legend level and name of the next legend car to unlock!
					if(vehicle->locStringFull.id != 0)
					{
						m_VariableText.Format(id.String(), nextLegendLevel , C_TextStringID( vehicle->locStringFull.id ).String() );
					}
					else
					{
						// Do manufacturers name + car name
						const WCHAR* manufacturer = C_TextStringID( vehicle->manufacturer->loc_string.id ).String();
						const WCHAR* vehName      = C_TextStringID( vehicle->loc_string.id ).String();
						C_WideString128	temp_str;
						
						temp_str.Format( L"%s %s", manufacturer, vehName );
						m_VariableText.Format(id.String(), nextLegendLevel , temp_str.Chars()  );

					}
				}
			}
		}
	}
	else
	if ( strncmp( pVariableName, "FRIEND_DEMAND_", 14 ) == 0 )
	{
		pVariableName += 14;

		if ( !strncmp( pVariableName, "EVENT", 5 ) )
		{
			static C_TextStringID id( "FDE_FRIEND_DEMAND" );
			m_VariableText.Format( id.String() );
		}
		else if ( !strncmp( pVariableName, "CRITERIA_TITLE_", 15 ) )
		{
			pVariableName += 15;

			const int index = (int)ParseIndex( pVariableName, "" );

			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetCriteriaTitleString(index) );
		}
		else if ( !strncmp( pVariableName, "CRITERIA_VAR_", 13 ) )
		{
			pVariableName += 13;

			const int index = (int)ParseIndex( pVariableName, "" );
			
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetCriteriaVariableString(index) );
		}
		else if ( !strncmp( pVariableName, "PARAM_TITLE_", 12 ) )
		{
			pVariableName += 12;

			const int index = (int)ParseIndex( pVariableName, "" );
			
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamTitleString(index) );
		}
		else if ( !strncmp( pVariableName, "PARAM_VAR_", 10 ) )
		{
			pVariableName += 10;

			const int index = (int)ParseIndex( pVariableName, "" );
			
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamVariableString(index) );
		}
		else if ( !strncmp( pVariableName, "PARAM_SWITCH_", 13 ) )
		{
			pVariableName += 13;

			const int index = (int)ParseIndex( pVariableName, "" );
			
			m_VariableText.Format( Game::C_FriendChallenges::Instance().GetParamSwitchString(index) );
		}
		else if( !strncmp( pVariableName, "DESCRIPTION", 11 ) )
		{
				m_VariableText.Format( Game::C_FriendChallenges::Instance().GetFriendDemandDescription() );
		}
		else if( !strncmp( pVariableName, "NAME", 4 ) )
		{
				m_VariableText.Format( Game::C_FriendChallenges::Instance().GetFriendDemandName() );
		}
		else if ( !strncmp( pVariableName, "FRIEND_", 7 ) )
		{
			pVariableName += 7;
				
			const int index = (int)ParseIndex( pVariableName, "" );

			SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(index);
			if(prof)
			{
				m_VariableText.Format(prof->Name());
			}
		}	
		else if ( !strncmp( pVariableName, "INFO_", 5 ) )
		{
			pVariableName += 5;

			int index = (int)ParseIndexAdvance( pVariableName, "" );

			pVariableName++;
			int index1 = (int)ParseIndexAdvance( pVariableName, "" );

			static C_TextStringID id( "FDE_FRIEND_DEMAND_INFO" );
			m_VariableText.Format( id.String(), index, index1 );
		}
		else if ( !strncmp( pVariableName, "THE_DEMAND_HELP", 15 ) )
		{
			static C_TextStringID id( "FDE_NAME" );

			WCHAR *friend_str=L"your friends";
			if(Game::C_FriendChallenges::Instance().NumFriendsAdded() == 1)
			{
				for(int a=0;a<Game::MAX_FRIEND_DEMAND_FRIENDS;a++)
				{
					int slot=Game::C_FriendChallenges::Instance().GetFriendSlot(a);
					if(slot != -1)
					{
						SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(slot);
						if(prof)
						{
							friend_str=prof->Name();
						}
						break;
					}
				}
			}

			m_VariableText.Format( id.String(), friend_str );
		}
		else if ( !strncmp( pVariableName, "THE_DEMAND", 10 ) )
		{
			static C_TextStringID id( "FDE_CHALLENGE" );

			WCHAR *friend_str=L"Friends";
			if(Game::C_FriendChallenges::Instance().NumFriendsAdded() == 1)
			{
				for(int a=0;a<Game::MAX_FRIEND_DEMAND_FRIENDS;a++)
				{
					int slot=Game::C_FriendChallenges::Instance().GetFriendSlot(a);
					if(slot != -1)
					{
						SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(slot);
						if(prof)
						{
							friend_str=prof->Name();
						}
						break;
					}
				}
			}

			UI::C_WideString64 str1(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(3));
			UI::C_WideString64 str2(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(1));
			UI::C_WideString64 str3(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(0));
			UI::C_WideString64 str4(Game::C_FriendChallenges::Instance().GetCriteriaVariableString(2));

			m_VariableText.Format( id.String(), friend_str, str1.Chars(), str2.Chars(), str3.Chars(), str4.Chars());
		}
		else if ( !strncmp( pVariableName, "RESULT_HELP", 11 ) )
		{
			Game::C_LevelResult& levelResult = Game::C_Game::GetInstance()->GetLevelResult();
			if(Game::C_FriendChallenges::Instance().IsAttemptingFriendDemand())
			{
				const Game::FriendChallenge *fc=Game::C_FriendChallenges::Instance().GetAttemptingFriendDemand();


				if(Game::C_FriendChallenges::Instance().IsAttemptingFriendDemand())
				{
					if(levelResult.mFriendDemandComplete)
					{
						static C_TextStringID id( "FDE_YOU_ARE_BETTER_THAN" );
						m_VariableText.Format( id.String(), fc->mChallenger);
					}
					else
					{
						static C_TextStringID id( "FDE_YOU_ARE_NOT_BETTER_THAN" );
						m_VariableText.Format( id.String(), fc->mChallenger);
					}
				}
			}
		}
		else if ( !strncmp( pVariableName, "ACTIVE_COUNT_", 13 ) )
		{
			pVariableName += 13;

			int index = (int)ParseIndexAdvance( pVariableName, "" );

			int count=0;

			SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(index);
			if(prof)
			{
				for(int a=0;a<Game::MAX_FRIEND_DEMANDS;a++)
				{
					if(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mEventID != 0)
					{
#ifdef PLATFORM_PS3
						bool identical_ids=Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[a] == bdAuthUtility::getUserID(prof->m_Uid.handle.data);
						if(!wcscmp(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mChallenger, prof->Name()) || identical_ids)
#else
						bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid,Game::C_FriendChallenges::Instance().mFriendDemandUserIDStore[a]) ? true : false;
						if(!wcsicmp(Game::C_FriendChallenges::Instance().mFriendDemandStore[a].mChallenger, prof->Name()) || identical_ids)
#endif
						{
							count++;
						}
					}
				}
			}

			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( count ) );
		}
		else if ( !strncmp( pVariableName, "NEW_COUNT_", 10 ) )
		{
			pVariableName += 10;

			int index = (int)ParseIndexAdvance( pVariableName, "" );
			int count=0;

			SysProfile::BaseProfile *prof=Game::C_FriendsManager::Instance().GetFriendProfileFromID(index);

			if(prof)
			{
				for(U32 i = 0; i < Game::C_MessagingManager::Instance().GetNumberOfMails(); i++)
				{
					Game::FriendChallenge *fc=(Game::FriendChallenge *)Game::C_MessagingManager::Instance().GetMailBody(i)->m_mailBody;

					if(Game::C_FriendsManager::Instance().GetFriendProfileFromID(index))
					{
#ifdef PLATFORM_PS3
						bool identical_ids=Game::C_MessagingManager::Instance().GetMailHeader(i)->m_senderID == bdAuthUtility::getUserID(prof->m_Uid.handle.data);
						if(fc && (!wcscmp(fc->mChallenger, prof->Name()) || identical_ids) )
#else
						bool identical_ids=XOnlineAreUsersIdentical(prof->m_Uid,Game::C_MessagingManager::Instance().GetMailHeader(i)->m_senderID) ? true : false;
						if(fc && (!wcsicmp(fc->mChallenger, prof->Name()) || identical_ids) )
#endif
						{
							count++;
						}
					}
				}
			}

			m_VariableText.Format( C_UILocaliser::Instance().ConvertNumberToCommas( count ) );
		}
		else if ( !strncmp( pVariableName, "TOTAL_NEW_COUNT", 15 ) )
		{
			static C_TextStringID id( "FDE_NEW_MAIL" );

			if(Game::C_MessagingManager::Instance().GetNumberOfMails() || Game::C_MessagingManager::Instance().IsThereNewMail())
			{
				m_VariableText.Format( id.String() );
			}
		}
	}

	// unhandled
	if ( !m_VariableText.Length()  )
	{
		//LOGWARN( g_Log, DBSTR "name \"%s\" not found", pVariableName );
		return L"";
	}

	return m_VariableText;
}


void C_GameDataFinder::FormatRaceRewards( UI::C_WideString512& variableText, const AMAX_SCORING_EVENT eType, const S32 sModifier )
{
	// TODO : Localise this once i move to the branch
	switch( eType )
	{
		case AMAX_SCORING_EVENT_SHUNT :
		{
			variableText.Format( L"%s", L"Shunt" );
			break;
		}
		case AMAX_SCORING_EVENT_SHOCK :
		{
			variableText.Format( L"%s", L"Shock" );
			break;
		}
		case AMAX_SCORING_EVENT_BARGE :
		{
			variableText.Format( L"%s", L"Barge" );
			break;
		}
		case AMAX_SCORING_EVENT_REPAIR :
		{
			variableText.Format( L"%s", L"Repair" );
			break;
		}
		case AMAX_SCORING_EVENT_SHIELD :
		{
			variableText.Format( L"%s", L"Shield" );
			break;
		}	
		case AMAX_SCORING_EVENT_NITROUS_OVERTAKE :
		{
			variableText.Format( L"%s", L"Nitro Overtake" );
			break;
		}
		case AMAX_SCORING_EVENT_HIT_A_CAR :
		{
			variableText.Format( L"%s", L"Hit a Car" );
			break;
		}
		case AMAX_SCORING_EVENT_TARGET_ACHIEVED :
		{
			variableText.Format( L"%s", L"Achieving Target" );
			break;
		}
	}
}

void C_GameDataFinder::FormatPickupInfo(UI::C_WideString512& variableText, const Game::Actors::RaceLogicCore *pRLC, const U8 vehicleIndex, const U8 pickupPodSlotIndex)
{
	//const RLPickupPod pickupPod = pRLC->GetVehiclePickupPod(vehicleIndex, pickupPodSlotIndex);

	//C_WideString32 pickupName = L"NONE";
	//C_WideString32 levelName = L"";

	//float progress = 0.f;

	//if(pickupPod.meType > E_PICKUP_TYPE_NONE)
	//{
	//	const RLPickupType *pPickupType = pRLC->GetPickupType(pickupPod.meType);
	//	pickupName = C_TextStringID( pPickupType->mLoc_string.id ).String();

	//	switch(pickupPod.meLevel)
	//	{
	//	case E_PICKUP_LEVEL_NONE:
	//		{
	//			levelName = L"L0";
	//			break;
	//		}
	//	case E_PICKUP_LEVEL_ONE:
	//		{
	//			levelName = L"L1";
	//			break;
	//		}
	//	case E_PICKUP_LEVEL_TWO:
	//		{
	//			levelName = L"L2";
	//			break;
	//		}
	//	case E_PICKUP_LEVEL_THREE:
	//		{
	//			levelName = L"L3";
	//			break;
	//		}
	//	}
	//	progress = pickupPod.mPercentProgress;
	//}

	//variableText.Format( C_TextStringID( "IDS_GAME_PICKUP_INFO" ).String(), pickupPodSlotIndex, pickupName.Chars(), levelName.Chars(), progress, pickupPod.mNoTargetsHit );
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif
