#include "stdafx.h"

#include <Game/Game.h>
#include <ui/Game/UILua.h>
#include <Ui\Graphics\UIColourClasses.h>

#include <GameLogic/GameData/GameEventData.h>
#include <GameLogic/RaceModes/RaceModes.h>
#include <RaceBook/Data/RaceBookData.h>

#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/GameData/GameEventData.h>

#include "RaceBook/RaceBookManager.h"

#include <vehicle\db\VehicleXt.h>

#include "Ui/UiHelpers.h"

#include <GameLogic\StickerManager.h>
#include <Game\Achievements\AchievementSystem.h>
#include <GameLogic\AchievementsManager.h>

extern Debug::Logger &gLUAMessageLogger;

#ifndef CFG_GOLD
Debug::Int g_DebugCheatShopFansOverride( "RaceBook.DebugShopFansOverride", -1, Debug::Range( -1, 100000 ) );
#endif

Debug::Bool g_debugShowDevCities("Debug.ShowDevCities", false, Debug::Persist());

namespace Game
{

static int GetCities( lua_State* pState )
{
	bool show_all=false;
	if(lua_isboolean(pState,1) )
	{
		show_all=lua_toboolean(pState,1) ? true : false;
	}

	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < C_GameData::Instance().mCities.Number(); i++)
	{
		AmaxCity *city = C_GameData::Instance().mCities[C_GameData::Instance().mCities.mSortedData[i]];

		if(city && ((!city->devOnly || g_debugShowDevCities) || show_all) )
		{
			lua_newtable(pState);

			Lua::AddDataMember(pState,"debug_tag",city->tag);
			Lua::AddDataMember(pState,"name",city->loc_string.id);
			Lua::AddDataMember(pState,"id",city->ID);
			Lua::AddDataMember(pState,"icon_name",city->icon_name);

			lua_rawseti(pState,-2,index);
			index++;
		}
	}

	return 1;
}

static int GetCityData( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetCityData( id ) - invalid id" );
		return 0;
	}

	AmaxCity* pCity = C_GameData::Instance().mCities[(GAMEDATA_ID)(U32)lua_tointeger( pState, 1 )];

	if ( !pCity )
		return 0;

	lua_newtable(pState);

	Lua::AddDataMember(pState,"debug_tag",	pCity->tag);
	Lua::AddDataMember(pState,"name",		pCity->loc_string.id);
	Lua::AddDataMember(pState,"id",			pCity->ID);
	Lua::AddDataMember(pState,"icon_name",	pCity->icon_name);

	return 1;
}

static int GetWinConditions( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < C_GameEventData::Instance().mWinConditions.Number(); i++)
	{
		AmaxWinCondition *wc = C_GameEventData::Instance().mWinConditions[i];

		lua_newtable(pState);

		Lua::AddDataMember(pState,"name",wc->loc_string.id);
		Lua::AddDataMember(pState,"id",wc->ID);

		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static int GetCharacters( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < C_RaceBookData::Instance().mCharacters.Number(); i++)
	{
		RaceBookCharacter *c = C_RaceBookData::Instance().mCharacters[C_RaceBookData::Instance().mCharacters.mSortedData[i]];

		lua_newtable(pState);

		Lua::AddDataMember(pState,"name", c->loc_string.id);
		Lua::AddDataMember(pState,"id", c->ID);
		Lua::AddDataMember(pState,"tag", c->tag);

		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static int GetRoutes( lua_State* pState )
{
	if(!lua_isnumber(pState,1) )
	{
		//DBPrint("GetRoutes(int cityId) - incorrect arguments\n");
		return 0;
	}

	//Create our route table
	lua_newtable(pState);

	//Get the cityId from the stack
	U32 cityId = (U32)lua_tointeger(pState,1);

	//We appear to have to go through each route even though
	int index=0;
	for(int a=0;a<C_GameData::Instance().mRoutes.Number();a++)
	{
		AmaxRoute *route=C_GameData::Instance().mRoutes[C_GameData::Instance().mRoutes.mSortedData[a]];

		if(route && route->city->ID == cityId)
		{
			//Create a new table for the route
			lua_newtable(pState);
			
			Lua::AddDataMember(pState,"name",route->loc_string.id);
			Lua::AddDataMember(pState,"debug_tag",route->tag);
			Lua::AddDataMember(pState,"id",route->ID);
			Lua::AddDataMember(pState,"route_number",route->route_number);

			lua_rawseti(pState,-2,index+1);
			index++;
		}
	}

	return 1;
}

static int GetRouteData( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetRouteData( id ) - invalid id" );
		return 0;
	}

	AmaxRoute* pRoute=C_GameData::Instance().mRoutes[(GAMEDATA_ID)(U32)lua_tointeger( pState, 1 )];

	if ( !pRoute )
		return 0;

	lua_newtable(pState);

	Lua::AddDataMember(pState,"debug_tag",	pRoute->tag);
	Lua::AddDataMember(pState,"name",		pRoute->loc_string.id);
	Lua::AddDataMember(pState,"id",			pRoute->ID);

	return 1;
}

static int GetManufacturers( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < C_GameVehicleData::Instance().mManufacturers.Number(); i++)
	{
		AmaxManufacturer *manu = C_GameVehicleData::Instance().mManufacturers[C_GameVehicleData::Instance().mManufacturers.mSortedData[i]];

		int num_Cars=0;
		for(int a=0;a<C_GameVehicleData::Instance().mVehicles.Number();a++)
		{
			AmaxVehicle *veh=C_GameVehicleData::Instance().mVehicles[C_GameVehicleData::Instance().mVehicles.mSortedData[a]];

			if(veh->manufacturer->ID == manu->ID)
			{
				num_Cars++;
			}
		}

		if(num_Cars)
		{
			lua_newtable(pState);

			Lua::AddDataMember(pState,"name",manu->loc_string.id);
			Lua::AddDataMember(pState,"id",manu->ID);

			lua_rawseti(pState,-2,index);
			index++;
		}
	}

	return 1;
}

static int GetVehicles( lua_State* pState )
{
	bool ignore_manu=false;
	if(!lua_isnumber(pState,1) )
	{
		ignore_manu=true;
	}

	//Create our route table
	lua_newtable(pState);

	//Get the cityId from the stack
	U32 manuId = 0;
	if(!ignore_manu)
	{
		manuId = (U32)lua_tointeger(pState,1);
	}

	Btl::Vector< U32, Btl::VectorParameters_Dynamic<> > mCarLocIDs;

	//We appear to have to go through each route even though
	int index=0;
	for(int a=0;a<C_GameVehicleData::Instance().mVehicles.Number();a++)
	{
		AmaxVehicle *veh=C_GameVehicleData::Instance().mVehicles[C_GameVehicleData::Instance().mVehicles.mSortedData[a]];

		if(ignore_manu || veh->manufacturer->ID == manuId)
		{
			bool add=true;
			for(int a=0;a<mCarLocIDs.Size();a++)
			{
				if(mCarLocIDs[a] == veh->loc_string.id)
					add=false;
			}

			if(add)
			{
				//Create a new table for the route
				lua_newtable(pState);
				
				Lua::AddDataMember(pState,"name",veh->loc_string.id);
				Lua::AddDataMember(pState,"id",veh->ID);
				Lua::AddDataMember(pState,"icon_name",veh->m_UiIconName);

				lua_rawseti(pState,-2,index+1);
				index++;

				mCarLocIDs.PushBack(veh->loc_string.id);
			}
		}
	}

	return 1;
}

static int GetRaceModes( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	
	for(int a=0;a<E_RaceMode_MAX;a++)
	{
		lua_newtable(pState);

		Lua::AddDataMember(pState,"name",gRaceModeStore((E_RaceMode)a)->GetName());
		Lua::AddDataMember(pState,"id",(int)gRaceModeStore((E_RaceMode)a)->GetID());

		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static int HasVehicleGotStyle( lua_State* pState )
{
	if(!lua_isnumber(pState,1) || !lua_isnumber(pState,1))
	{
		//DBPrint("GetRoutes(int cityId) - incorrect arguments\n");
		return 0;
	}

	U32 vehLocId = (U32)lua_tointeger(pState,1);
	int style = (int)lua_tonumber(pState,2);

	bool has_style=false;
	for(int a=0;a<C_GameVehicleData::Instance().mVehicles.Number();a++)
	{
		AmaxVehicle *veh=C_GameVehicleData::Instance().mVehicles[C_GameVehicleData::Instance().mVehicles.mSortedData[a]];

		if(veh->loc_string.id == vehLocId && veh->style->style == style)
		{
			has_style=true;
		}
	}

	lua_pushboolean(pState, has_style);

	return 1;
}

static int GetPerks( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes.count; i++)
	{
		RLPickupType *pu=RLDatabase::Instance().mRLSystems[0]->mPickupSystem->mTypes[i];
		
		lua_newtable(pState);

		Lua::AddDataMember(pState,"name",pu->mLoc_string.id);
		Lua::AddDataMember(pState,"id",pu->meType);

		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static void SendToLua( lua_State* pState, AmaxCity* city )
{
	if( !city )
		return;

	Lua::AddDataMember( pState, "city_tag",			city->tag );
	Lua::AddDataMember( pState, "city_id",			city->ID );
	Lua::AddDataMember( pState, "city_name",		city->loc_string.id );
	Lua::AddDataMember( pState, "city_icon_name",	city->icon_name );
}

static void SendToLua( lua_State* pState, AmaxRoute* route )
{
	if( !route )
		return;

	Lua::AddDataMember( pState, "route_tag",		route->tag );
	Lua::AddDataMember( pState, "route_id",			route->ID );
	Lua::AddDataMember( pState, "route_name",		route->loc_string.id );
	Lua::AddDataMember( pState, "route_type",		route->circuitType );

	SendToLua( pState, route->city );
}

static void SendToLua( lua_State* pState, AmaxEvent* ev )
{
	if( !ev )
		return;

	Lua::AddDataMember( pState, "event_tag",		ev->tag );
	Lua::AddDataMember( pState, "event_id",			ev->ID );
	Lua::AddDataMember( pState, "event_name",		ev->loc_string.id );

	SendToLua( pState, ev->route );
}

static int GetEvent( lua_State* pState )
{
	if( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		UI::g_pUILua->StackDump( pState, "GetEvent( id ) - incorrect arguments" );
		return 0;
	}

	XtId id = (XtId)lua_tointeger( pState, 1 );

	AmaxEvent* pEvent = C_GameEventData::Instance().mEvents[ (GAMEDATA_ID)id ];
	if( !pEvent )
	{
		UI::g_pUILua->StackDump( pState, "GetEvent( id ) - invalid id" );
		return 0;
	}

	lua_newtable(pState);
	SendToLua( pState, pEvent );
	
	return 1;
}

static int GetEvents( lua_State* pState )
{
	lua_newtable(pState);

	int index=1;
	for(S32 i = 0; i < C_GameEventData::Instance().mEvents.Number(); i++)
	{
		lua_newtable( pState );
		
		SendToLua( pState, C_GameEventData::Instance().mEvents[ C_GameEventData::Instance().mEvents.mSortedData[ i ] ] );

		lua_rawseti( pState, -2, index);
		index++;
	}

	return 1;
}

static int GetSplitScreenEvents( lua_State* pState )
{
	lua_newtable(pState);
	int index=1;
	for(S32 i = 0; i < C_GameEventData::Instance().mSplitScreenEvents.Number(); i++)
	{
		AmaxSplitScreenEvent *e = C_GameEventData::Instance().mSplitScreenEvents[i];
		
		lua_newtable(pState);

		Lua::AddDataMember(pState,"tag", e->tag);
		Lua::AddDataMember(pState,"name", e->loc_string.id);
		Lua::AddDataMember(pState,"id", e->ID);
		Lua::AddDataMember(pState,"allow_route", e->mAllowRoute);
		Lua::AddDataMember(pState,"allow_car_class", e->mAllowCarClass);
		Lua::AddDataMember(pState,"allow_laps", e->mAllowLaps);
		Lua::AddDataMember(pState,"laps", e->laps);
		Lua::AddDataMember(pState,"allow_pad_ai", e->mAllowPadAIVehicles);
		Lua::AddDataMember(pState,"allow_handicap", e->mAllowHandicap);	 //...or this
		Lua::AddDataMember(pState,"icon_sheet", e->mIconSheet);
		Lua::AddDataMember(pState,"icon_name", e->mIconName);
		Lua::AddDataMember(pState,"mode_name", e->mModeName.id);
		Lua::AddDataMember(pState,"fans_to_unlock", e->mFansToUnlock);

		// route override
		if ( e->route )
		{
			Lua::AddDataMember( pState, "has_route_override", true );
			Lua::AddDataMember( pState, "route_name", e->route->loc_string.id );
			Lua::AddDataMember( pState, "route_id", e->route->route_number );


			if ( e->route->city )
			{
				Lua::AddDataMember( pState, "city_icon", e->route->city->icon_name );
				Lua::AddDataMember( pState, "city_name", e->route->city->loc_string.id );
			}
		}
		else
			Lua::AddDataMember( pState, "has_route_override", false );

		lua_rawseti(pState,-2,index);
		index++;
	}

	return 1;
}

static int GetShopItems( lua_State* pState )
{
	lua_newtable( pState );

	RaceBook::C_RaceBookProfile* pProfile = RaceBook::C_RaceBookManager::Instance().GetPlayerProfile();

	if ( !pProfile )
		return 1;

	S32 sCount = 0;

	S32 sNumFans = pProfile->mNumFans;

#ifndef CFG_GOLD
	if ( g_DebugCheatShopFansOverride != -1 )
		sNumFans = g_DebugCheatShopFansOverride;
#endif	

	Lua::AddDataMember( pState, "total_fans", sNumFans );

	return 1;
}

static S32 POBCheckForUpcomingAchievements( lua_State* pState )
{
	bool AchievementsNearingCompletion = false;

	C_StickerManager& StickerManager = C_StickerManager::Instance();

	for(int i = 0; i < BLUR_STICKER_MAX; i++)
	{
		C_Sticker* pSticker = StickerManager.GetSticker(BLUR_STICKER(i));
		if(pSticker)
		{
			if(pSticker->IsOneShot() == false)//dont want to check one shot stickers
			{
				F32 StickerProgress =  ( pSticker->Progress() / pSticker->Goal() );
				if(StickerProgress > 0.75)
				{
					AchievementsNearingCompletion = true;
					break;
				}
			}
		}
	}

	lua_pushboolean(pState, AchievementsNearingCompletion);
	return 1;
}

static S32 POBGetUpcomingAchievements( lua_State* pState )
{
	bool AchievementsNearingCompletion = false;

	C_StickerManager& StickerManager = C_StickerManager::Instance();
	C_AchievementsManager& AchievementManager = C_AchievementsManager::Instance();

	lua_newtable(pState);

	lua_pushstring(pState, "stickers");
	lua_newtable(pState);

	S32 stickerCount = 0;
	for(int i = 0; i < BLUR_STICKER_MAX; i++)
	{
		C_Sticker* pSticker = StickerManager.GetSticker(BLUR_STICKER(i));
		if(pSticker)
		{
			if(pSticker->IsOneShot() == false && pSticker->IsComplete() == false)//dont want to check one shot stickers
			{
				F32 StickerProgress =  ( pSticker->Progress() / pSticker->Goal() );
				if(StickerProgress >= 0.75)
				{	
					S32 AchievementLocation = 0; 
					S32 stickerIndex = 0;

					if(pSticker->m_pAchievement)
					{
						for(int j = 0; j < (int)AchievementManager.GetNumAchievements(); j++)
						{
						
							if(AchievementManager.mStickerBookLocations[j]->mType == pSticker->m_pAchievement->mType)
							{
								AchievementLocation = j;
							}
						}					

						for(int k = 0; k < AchievementManager.mStickerBookLocations[AchievementLocation]->mStickers.Size(); k++)
						{
							if(AchievementManager.mStickerBookLocations[AchievementLocation]->mStickers[k]->Type() == pSticker->Type())
							{
								stickerIndex = k;
							}
						}

						stickerCount++;

						lua_newtable(pState);						
							Lua::AddDataMember( pState, "achievement_location", AchievementLocation ); 						
							Lua::AddDataMember( pState, "description", pSticker->Description().id ); 
							Lua::AddDataMember( pState, "sticker_progress", StickerProgress );
							Lua::AddDataMember( pState, "progress", pSticker->Progress() ); 
							Lua::AddDataMember( pState, "goal", pSticker->Goal() );
							Lua::AddDataMember( pState, "sticker_index",  stickerIndex ); 
						lua_rawseti( pState, -2, stickerCount);	
					}
				}
			}
		}
	}

	lua_rawset(pState, -3);
	return 1;
}


// GameData_SendVehicleToLua
static void GameData_SendVehicleToLua( lua_State* pState, AmaxVehicle* pVehicle )
{
	Lua::AddDataMember( pState, "id", pVehicle->ID );
	Lua::AddDataMember( pState, "class", pVehicle->carClass );
	Lua::AddDataMember( pState, "tag", pVehicle->tag );
	Lua::AddDataMember( pState, "name", pVehicle->loc_string.id );
	Lua::AddDataMember( pState, "icon", pVehicle->m_UiIconName.Text() );
	Lua::AddDataMember( pState, "sheet", UI::GetVehicleSheet( pVehicle->carClass ) );

	Lua::AddDataMember( pState, "acceleration", pVehicle->m_sUiAcceleration );
	Lua::AddDataMember( pState, "speed", pVehicle->m_sUiSpeed );
	Lua::AddDataMember( pState, "stability", pVehicle->m_sUiStability );
	Lua::AddDataMember( pState, "difficulty", pVehicle->m_sUiDifficulty );
	Lua::AddDataMember( pState, "strength", RLDatabase::Instance().mRLSystems[ 0 ]->mDamageSystem->mParameters->mVehicleHealthGroups[ pVehicle->vehicleHealthGroup ].mBaseHealth / 100 );

	Lua::AddDataMember( pState, "style", pVehicle->m_eUiCarStyle );
	Lua::AddDataMember( pState, "respray", pVehicle->m_bUICanReSpray );

	Lua::AddDataMember( pState, "man_tag", pVehicle->manufacturer->tag );
	Lua::AddDataMember( pState, "man_name", pVehicle->manufacturer->loc_string.id );
	Lua::AddDataMember( pState, "man_texture", pVehicle->manufacturer->mUiTexture.Text() );
	Lua::AddDataMember( pState, "man_texture_square", pVehicle->manufacturer->mUiSquareTexture );

	Lua::AddDataMember( pState, "ability", pVehicle->bossAbilitySlot );
	Lua::AddDataMember( pState, "has_parent", pVehicle->m_ParentVehicle != NULL );

	// full name
	if ( pVehicle->locStringFull.id != 0 )
		Lua::AddDataMember( pState, "name_full", pVehicle->locStringFull.id );	// override
	else
	{
		Btl::String64 string;
		string.Printf( "GAME_VEHICLE_%u_FULL_NAME", pVehicle->ID );
		Lua::AddDataMember( pState, "name_full", string.Chars() );	// datafinder lookup for manufacturer name + car name
	}
}

// GetVehicle
static S32 GetVehicle( lua_State* pState )
{
	if ( lua_type( pState, 1 ) == LUA_TNONE )
	{
		// every car
		lua_newtable( pState );

		for ( S32 i = 0; i < C_GameVehicleData::Instance().mVehicles.Number(); i++ )
		{
			AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ i ];

			if ( !pVehicle )
				continue;

			lua_newtable( pState );
				GameData_SendVehicleToLua( pState, pVehicle );
			lua_rawseti( pState, -2, pVehicle->ID );
		}

		return 1;
	}

	if ( lua_type( pState, 1 ) == LUA_TNUMBER )
	{
		// specific car
		lua_newtable( pState );

		XtId id = (XtId)lua_tointeger( pState, 1 );

		AmaxVehicle* pVehicle = C_GameVehicleData::Instance().mVehicles[ (GAMEDATA_ID)id ];

		if ( !pVehicle )
			return 1;

		GameData_SendVehicleToLua( pState, pVehicle );

		return 1;
	}

	return 0;
}

/**********************************************************************************************************************************************************************/
/**********************************************************************************************************************************************************************/
static const luaL_reg gamedata_lib[] = 
{
	{"GetCities",							GetCities},
	{"GetCityData",							GetCityData},
	{"GetRoutes",							GetRoutes},
	{"GetRouteData",						GetRouteData},
	{"GetWinConditions",					GetWinConditions},
	{"GetCharacters",						GetCharacters},

	{"GetManufacturers",					GetManufacturers},
	{"GetVehicles",							GetVehicles},

	{"HasVehicleGotStyle",					HasVehicleGotStyle},
		
	{"GetRaceModes",						GetRaceModes},

	{"GetPerks",							GetPerks},
	{"GetEvent",							GetEvent},
	{"GetEvents",							GetEvents},
	{"GetSplitScreenEvents",				GetSplitScreenEvents},
	{"GetShopItems",						GetShopItems},

	{"GetVehicle",							GetVehicle},

	{"POBCheckForUpcomingAchievements",		POBCheckForUpcomingAchievements},
	{"POBGetUpcomingAchievements",			POBGetUpcomingAchievements},

	{NULL,						NULL}		// end of list
};  

LUALIB_API int luaopen_gamedata(lua_State *pState)
{
	luaL_openlib(pState,"GameData",gamedata_lib,0);
	return 1;
}

} // end namespace Game
