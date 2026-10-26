#include "stdafx.h"

#include <system/serialise/C_FileSystem.h>
#include <system/serialise/TextSerialise.h>

#include <Game/Game.h>
#include <GameSystems.h>
#include <GameLogic/Gamedata/GameData.h>
#include <GameLogic/Gamedata/GameVehicleData.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>

#include <GameLogic/Layers/RaceBookLayer.h>
#include <GameLogic/Layers/AmaxWorldLayer.h>
#include <GameApp.h>
#include <Game/GameWorld/GameWorld.h>


#include <GameApp.h>

#include <GameLogic/GameModes/GameModeDefines.h>
#include <GameLogic/GameModes/GameModeDebug.h>
#include <GameLogic/AchievementsManager.h>

#include <framework/application/LoadDebugVars.h>
#include <RaceBook/Data/RaceBookData.h>

#ifndef CFG_GOLD
extern Debug::Bool g_debugSuperSpecialCutSceneVideoMode;
#endif

Debug::Int g_debugDebugForceVehiclePlayerTextureDetail("Vehicle.LOD.DebugPlayerDetail", 10, Debug::Range(0, 10), Debug::Step(1));
Debug::Int g_debugDebugForceVehicleAiTextureDetail("Vehicle.LOD.DebugAiDetail", 7, Debug::Range(0, 10), Debug::Step(1));

namespace Game
{

const int DEBUG_FILE_VERSION=5;		//if the Debug Menu has changed significantly and you need it to auto-wipe ... then increment this
int gCurrentDebugFileVersion=0;

#ifndef CFG_GOLD
bool gAllowSavingDebugMenuValues = true;	// Set to false if Automation is running
#endif

Debug::Logger &g_GameModeDebugLogger( Debug::GetLogger("GameMode.Debug") );

Debug::Int g_debugPerkSlotOveride("Cheats.Debug_PerkSlotOveride",3,Debug::Persist());

void C_GameModeDebug::Initialise()
{
	SetInternalNameAndID("Debug",E_GameMode_Debug);
}

void C_GameModeDebug::Update()
{

}

void C_GameModeDebug::Start()
{
	LoadDebugValues();

	if(gCurrentDebugFileVersion != DEBUG_FILE_VERSION)
	{
		C_Game::GetInstance()->GetLevelState().Clear();
	}
}

Debug::Bool g_debugLoadLargestVehicles("Game.Debug.LoadLargestVehicles", false);
Debug::Int g_debugNumAdditionalCars("Game.Debug.NumAdditionalCars", 0, Debug::Range(0, 20), Debug::Step(1));

void C_GameModeDebug::SetupRace(void *data)

{
	lua_State *pState=(lua_State *)data;
	Game::C_LevelState& levelState = C_Game::GetInstance()->GetLevelState();


	if(!lua_istable(pState,1) )
	{
		//DBPrint("C_GameModeDebug::SetupRace(table) - incorrect arguments\n");
		return;
	}

	lua_settop(pState,1);
	
	levelState.mEvent = 0;

	// Get the City and Route
	Lua::GetDataMember(pState,"City",levelState.mCity,true);
	Lua::GetDataMember(pState,"Route",levelState.mRoute,true);   

	Lua::GetDataMember(pState,"Laps",levelState.mLaps,true);   

	Lua::GetDataMember(pState,"Environment",levelState.mEnvironmentOveride,true);

	Lua::GetDataMember(pState,"RandomCars",levelState.mRandomCars,true);

	Lua::GetDataMember(pState,"Viewports",levelState.mNumViewports,true);

	// Get and Set the RaceMode
	Lua::GetDataMember(pState,"RaceMode",(S32&)levelState.mRaceMode,true);
	C_GameApp::GetInstance()->GetRaceBookLayer()->SetRaceMode(levelState.mRaceMode);


	//DANGER! HARD-CODED MAGIC NUMBERS AHEAD!
	U32 vehicleXtIds[20];
	vehicleXtIds[0] = 0x36D0BCE5;//Dodge_Challenger_Race
	vehicleXtIds[1] = 0xC5269E58;//BMW_1SeriesTii_Race
	vehicleXtIds[2] = 0x662362C5;//AstonMartin_V12VantageRS_Stock
	vehicleXtIds[3] = 0x685AB3BC;//Toyota_SupraTRD_Stock
	vehicleXtIds[4] = 0x2A463054;//Ford_F150_OffRoad
	vehicleXtIds[5] = 0x04B7702E;//Mitsubishi_EVOVI_Tuner
	vehicleXtIds[6] = 0x4A84D6D4;//Nissan_350ZNismo_Stock
	vehicleXtIds[7] = 0xA81889E8;//Hennessey_ViperVenom1000Coupe_Stock
	vehicleXtIds[8] = 0xB32BB9F1;//Bowler_Nemesis_Stock
	vehicleXtIds[9] = 0xFC1ADB45;//Audi_S3_Stock
	vehicleXtIds[10] = 0x288DED6C;//Mazda_RX8_Tuner
	vehicleXtIds[11] = 0xFFFE3216;//Nissan_350Z_Drift
	vehicleXtIds[12] = 0x895082BF;//Audi_R8V10_Stock
	vehicleXtIds[13] = 0xB6F2BF35;//Corvette_C3_Rat
	vehicleXtIds[14] = 0xEF0DA366;//Mazda_RX8_Smooth
	vehicleXtIds[15] = 0x427E762F;//Audi_TT_Stock
	vehicleXtIds[16] = 0x6A3E40BA;//Chevrolet_Camaro_Rat
	vehicleXtIds[17] = 0xA1898922;//BMW_1SeriesTii_Stock
	vehicleXtIds[18] = 0x7FE6A554;//Ford_GT_Stock
	vehicleXtIds[19] = 0xBA0EB318;//Volkswagen_Beetle_OffRoad

	//Car stuff
	Lua::GetDataMember(pState,"NumberOfCars",C_Game::GetInstance()->GetLevelState().mNumCars,true);
	C_Game::GetInstance()->GetLevelState().mNumAdditionalCars = (int)g_debugNumAdditionalCars;
	for(U32 a=0;a<MAX_LEVELSTATE_CARS;a++)
	{
		char temp[128];
		sprintf_s(temp, lengthof(temp), "Manufacturer%d",a+1);
		Lua::GetDataMember(pState,temp,C_Game::GetInstance()->GetLevelState().mCars[a].mManufacturerID,true);

		sprintf_s(temp, lengthof(temp), "VehicleType%d",a+1);
		Lua::GetDataMember(pState,temp,C_Game::GetInstance()->GetLevelState().mCars[a].mCarStyle,true);

		sprintf_s(temp, lengthof(temp), "Vehicle%d",a+1);
		Lua::GetDataMember(pState,temp,C_Game::GetInstance()->GetLevelState().mCars[a].mCarLocID,true);

		if (a >= C_Game::GetInstance()->GetLevelState().mNumCars)
		{
			C_Game::GetInstance()->GetLevelState().mCars[a].mStartDisabled = true;
			C_Game::GetInstance()->GetLevelState().mCars[a].mCutSceneOnly = true;			
		}
		else
		{
			C_Game::GetInstance()->GetLevelState().mCars[a].mStartDisabled = false;
			C_Game::GetInstance()->GetLevelState().mCars[a].mCutSceneOnly = false;			
		}
		//work out the actual carID. as we now have style on the same models
		//at this point mCarLocID is actually the loc string (the only unique number in the record)
		if(C_Game::GetInstance()->GetLevelState().mCars[a].mCarLocID)
		{
			bool found_vehicle=false;
			if (g_debugLoadLargestVehicles)
			{
				for(int v=0;v<C_GameVehicleData::Instance().mVehicles.Number();v++)
				{
					AmaxVehicle *veh=C_GameVehicleData::Instance().mVehicles[v];
					if (veh->ID == vehicleXtIds[a])
					{
						levelState.mCars[a].mCarID=veh->ID;
						found_vehicle=true;
						break;
					}
				}
			}
			if (!found_vehicle)
			{
				for(int v=0;v<C_GameVehicleData::Instance().mVehicles.Number();v++)
				{
					AmaxVehicle *veh=C_GameVehicleData::Instance().mVehicles[v];

					if(veh->loc_string.id == levelState.mCars[a].mCarLocID)
					{
						if(veh->style->style == levelState.mCars[a].mCarStyle)
						{
							levelState.mCars[a].mCarID=veh->ID;
							found_vehicle=true;
							break;
						}
					}
				}
			}
			BIZ_ASSERT(found_vehicle);
		}

		sprintf_s(temp, lengthof(temp), "Control%d",a+1);
		Lua::GetDataMember(pState,temp,levelState.mCars[a].mController,true); 

		sprintf_s(temp, lengthof(temp), "Character%d",a+1);
		Lua::GetDataMember(pState,temp,levelState.mCars[a].mCharacterID,true);
		if (levelState.mCars[a].mCharacterID == 0)
			levelState.mCars[a].mCharacterID = C_RaceBookData::Instance().mCharacters[0]->ID; // assume default

		levelState.mCars[a].mNameAboveCar.Format(L"Character %d",a);

		levelState.mCars[a].mStartPosition = a;
		
		levelState.mCars[a].mUpgrades.DebugValues();

		if (levelState.mCars[a].mController == INVALID_CONTROLLER)
		{
			levelState.mCars[a].mControlType = E_VEHICLE_CONTROL_TYPE_AI;
			levelState.mCars[a].mHuman = false;
			levelState.mCars[a].mAutoGears = true;
		}
		else
		{
			levelState.mCars[a].mControlType = E_VEHICLE_CONTROL_TYPE_PLAYER;
			levelState.mCars[a].mHuman = true;
			levelState.mCars[a].mAutoGears = true;
		}

#ifndef CFG_GOLD
		if (g_debugSuperSpecialCutSceneVideoMode)
			levelState.mCars[a].mTextureDetail = INT_MAX;
		else
#endif
		{		
			if (levelState.mCars[a].mControlType == E_VEHICLE_CONTROL_TYPE_PLAYER)
				levelState.mCars[a].mTextureDetail = g_debugDebugForceVehiclePlayerTextureDetail;
			else
				levelState.mCars[a].mTextureDetail = g_debugDebugForceVehicleAiTextureDetail;			
		}
	}

	levelState.mAIController = RLDatabase::Instance().mRLSystems[0]->mAIPickupController->mControllers[0].mController.Object();

	if(levelState.mRandomCars)
	{
		for(unsigned int a=0;a<levelState.mNumCars;a++)
		{
			int rv=rand()%C_GameVehicleData::Instance().mVehicles.Number();
			AmaxVehicle *v=C_GameVehicleData::Instance().mVehicles[rv];

			while(v->do_not_show_in_game)
			{
				rv=rand()%C_GameVehicleData::Instance().mVehicles.Number();
				v=C_GameVehicleData::Instance().mVehicles[rv];
			}

			BIZ_ASSERT(v);
			BIZ_ASSERT(v->manufacturer);
			BIZ_ASSERT(v->style);

			levelState.mCars[a].mCarLocID=v->loc_string.id;
			levelState.mCars[a].mCarID=v->ID;
			levelState.mCars[a].mManufacturerID=v->manufacturer->ID;
			levelState.mCars[a].mCarStyle=v->style->style;
		}
	}

	//C_AchievementsManager::Instance().PreRace();
	levelState.Finished();

	
}

void C_GameModeDebug::StartRace()
{
	EnableChangeViewOnAllViewports();
	SaveDebugValues();
	C_GameApp::GetInstance()->GetRaceBookLayer()->GetAmaxWorldLayer()->GetGameWorld()->SetGlobalFade(1.0f);
}

void C_GameModeDebug::InterpretRaceResults()
{

}

struct LoadSaveInfo
{
	bool					saving;
	Sys::TextSerialiser		*serial;
	Util::C_String			line;
};

static bool LoadSaveGeneral(LoadSaveInfo& lsi)
{
	if (lsi.saving)
	{
		BIZ_ASSERT(lsi.serial != NULL);
		Sys::TextSerialiser& serial = *(lsi.serial);

		serial << "\t<Value name=\"City\" value=\"" << C_Game::GetInstance()->GetLevelState().mCity << "\" />\n";
		serial << "\t<Value name=\"Route\" value=\"" << C_Game::GetInstance()->GetLevelState().mRoute << "\" />\n";
		serial << "\t<Value name=\"NumCars\" value=\"" << C_Game::GetInstance()->GetLevelState().mNumCars << "\" />\n";
		serial << "\t<Value name=\"Laps\" value=\"" << C_Game::GetInstance()->GetLevelState().mLaps << "\" />\n";
		serial << "\t<Value name=\"RandomCars\" value=\"" << C_Game::GetInstance()->GetLevelState().mRandomCars << "\" />\n";
		serial << "\t<Value name=\"Environment\" value=\"" << C_Game::GetInstance()->GetLevelState().mEnvironmentOveride << "\" />\n";
		serial << "\t<Value name=\"Viewports\" value=\"" << C_Game::GetInstance()->GetLevelState().mNumViewports << "\" />\n";
		serial << "\t<Value name=\"DebugFileVersion\" value=\"" << DEBUG_FILE_VERSION << "\" />\n";

		return true;
	}
	else // Loading
	{
		Util::C_String name = Framework::FieldValue(lsi.line, "name");
		Util::C_String value = Framework::FieldValue(lsi.line, "value");

		if (!strcmp(name, "City"))
		{
			C_Game::GetInstance()->GetLevelState().mCity = strtoul(value, NULL, 10);
			return true;
		}
		else if (!strcmp(name, "Route"))
		{
			C_Game::GetInstance()->GetLevelState().mRoute = strtoul(value, NULL, 10);
			return true;
		}
		else if (!strcmp(name, "NumCars"))
		{
			C_Game::GetInstance()->GetLevelState().mNumCars = strtoul(value, NULL, 10);
			return true;
		}
		else if (!strcmp(name, "Laps"))
		{
			C_Game::GetInstance()->GetLevelState().mLaps = strtoul(value, NULL, 10);
			return true;
		}
		else if (!strcmp(name, "RandomCars"))
		{
			C_Game::GetInstance()->GetLevelState().mRandomCars = strtoul(value, NULL, 10);
			return true;
		}
		else if (!strcmp(name, "Enivronment"))
		{
			C_Game::GetInstance()->GetLevelState().mEnvironmentOveride = strtoul(value, NULL, 10);
			return true;
		}
		else if (!strcmp(name, "Viewports"))
		{
			C_Game::GetInstance()->GetLevelState().mNumViewports = strtoul(value, NULL, 10);
			return true;
		}
		else if (!strcmp(name, "DebugFileVersion"))
		{
			gCurrentDebugFileVersion = strtoul(value, NULL, 10);
			return true;
		}
	}

	return false;
}

static bool LoadSaveCar(LoadSaveInfo& lsi, int iCar)
{
	if (lsi.saving)
	{
		BIZ_ASSERT(lsi.serial != NULL);
		BIZ_ASSERT(iCar != -1);

		Sys::TextSerialiser& serial = *(lsi.serial);

		serial << "\t<Value name=\"Car_Manufacturer\" index=\"" << iCar << "\" value=\"" << C_Game::GetInstance()->GetLevelState().mCars[iCar].mManufacturerID << "\" />\n";
		serial << "\t<Value name=\"Car_LocID\" index=\"" << iCar << "\" value=\"" << C_Game::GetInstance()->GetLevelState().mCars[iCar].mCarLocID << "\" />\n";
		serial << "\t<Value name=\"Car_Style\" index=\"" << iCar << "\" value=\"" << C_Game::GetInstance()->GetLevelState().mCars[iCar].mCarStyle << "\" />\n";
		serial << "\t<Value name=\"Car_Controller\" index=\"" << iCar << "\" value=\"" << C_Game::GetInstance()->GetLevelState().mCars[iCar].mController << "\" />\n";
		serial << "\t<Value name=\"Car_Character\" index=\"" << iCar << "\" value=\"" << C_Game::GetInstance()->GetLevelState().mCars[iCar].mCharacterID << "\" />\n";

		return true;
	}
	else // Loading
	{
		Util::C_String name = Framework::FieldValue(lsi.line, "name");

		// Is this a car value?
		if (!strncmp(name, "Car_", 4))
		{
			Util::C_String index = Framework::FieldValue(lsi.line, "index");
			Util::C_String value = Framework::FieldValue(lsi.line, "value");

			iCar = atoi(index);

			if ((iCar >= 0) && (iCar < MAX_LEVELSTATE_CARS))
			{
				if (!strcmp(name, "Car_Manufacturer"))
				{
					C_Game::GetInstance()->GetLevelState().mCars[iCar].mManufacturerID = strtoul(value, NULL, 10);
					return true;
				}
				else if (!strcmp(name, "Car_LocID"))
				{
					C_Game::GetInstance()->GetLevelState().mCars[iCar].mCarLocID = strtoul(value, NULL, 10);
					return true;
				}
				else if (!strcmp(name, "Car_Style"))
				{
					C_Game::GetInstance()->GetLevelState().mCars[iCar].mCarStyle = strtoul(value, NULL, 10);
					return true;
				}
				else if (!strcmp(name, "Car_Controller"))
				{
					C_Game::GetInstance()->GetLevelState().mCars[iCar].mController = strtoul(value, NULL, 10);
					return true;
				}
				else if (!strcmp(name, "Car_Character"))
				{
					C_Game::GetInstance()->GetLevelState().mCars[iCar].mCharacterID = strtoul(value, NULL, 10);
					return true;
				}
			}
		}
	}

	return false;
}

void C_GameModeDebug::LoadDebugValues()
{
	using namespace Util;

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();

	const char* kDebugXmlFile = "debug_frontend.xml";
	if (!fs.FileExists(kDebugXmlFile))
		return;

	Sys::I_File* file = fs.Open(kDebugXmlFile);
	if (file == 0)
		return;

	Sys::TextDeserialiser deserial(*file);

	LoadSaveInfo lsi;
	lsi.saving = false;

	do
	{
		lsi.line = "";
		deserial >> lsi.line;

		if (LoadSaveGeneral(lsi))
			continue;
		else if (LoadSaveCar(lsi, -1))
			continue;
	}
	while (file->GetPosition() < file->Length());

	delete file;
	file = 0;
}

void C_GameModeDebug::SaveDebugValues() const
{
	#ifndef CFG_GOLD
	if (!gAllowSavingDebugMenuValues)
		return;

	Sys::I_FileSystem& fs = *Sys::GetDefaultFileSystem();
	
	Sys::C_OpenParams params;
	params.m_IsCritical = false;
	Sys::I_File* file = fs.Open("debug_frontend.xml", Sys::E_FileAccess_Write, params);
	if (file == 0)
		return;
	
	Sys::TextSerialiser serial(*file);

	LoadSaveInfo lsi;
	lsi.saving = true;
	lsi.serial = &serial;

	serial << Util::C_String("<Values>\n");
	{
		LoadSaveGeneral(lsi);

		for (unsigned int iCar=0; iCar < MAX_LEVELSTATE_CARS; iCar++)
			LoadSaveCar(lsi, iCar);
	}
	serial << Util::C_String("</Values>\n");

	delete file;
	file = 0;
	#endif
}

} // namespace Game
