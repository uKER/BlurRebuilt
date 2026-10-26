#include "stdafx.h"

#include <Network/Misc/NetDefines.h>

#include <System/Serialise/FileSystemGlobals.h>

#include <ui/Game/UILocaliser.h>
#include <ui/Graphics/Localisation/Loc.h>

#include <GameLogic/GameData/LocData.h>
#include <GameSystems.h>

static void LoadMultiplayerTSLocFromDisk(void*)
{
	char buf[128];
	const char* pLanguageCode = UI::C_UILocaliser::Instance().GetLanguageCode();

	sprintf_s(buf, lengthof(buf), "gamedata\\xt\\Loc\\%s.loc", TITLE_STORAGE_LOC_UI_TABLE);
	if (Sys::GetDefaultFileSystem()->FileExists(buf))
		UI::C_TextStringID::LocTable().LoadMultiplayerTSFromDisk( buf, pLanguageCode );
}

Debug::Bool g_DebugLoadMultiplayerTSLocFromDiskOnStartup( "Multiplayer.Localisation.LoadMultiplayerTSLocFromDiskOnStartup", false, Debug::Persist() );
Debug::Function gDebugLoadMultiplayerTSLocFromDisk( "Multiplayer.Localisation.LoadMultiplayerTSLocFromDisk", LoadMultiplayerTSLocFromDisk, 0 );


XT_TYPE( C_LocData, "UIGameLoc" )
	XT_CALLBACK( C_LocData::SorceryCallback )
	XT_FIELD( m_UITableNames, "UITables" )
	XT_FIELD( m_GameTableNames, "GameTables" )
XT_END_TYPE

C_LocData C_LocData::m_sInstance;

Btl::String GetFileErrorString()
{
	Btl::String msg;
	const WCHAR* pString = UI::C_TextStringID( "PRO_DIRTY_DISC_ERROR" ).String();
	msg.Printf("%S", pString);
	return msg;
}

void C_LocData::Init()
{
	if ( m_bInitialised )
		return;

	MEMORY_TRACKER_PUSH("ui.loc");

	const char* pLanguageCode = UI::C_UILocaliser::Instance().GetLanguageCode();

	//--------------------------------------------------------------------------------------
	// Load in the UI loc table and push it into lua
	//--------------------------------------------------------------------------------------
	if ( !m_UITableNames.elements )
	{
		LOGERROR( UI::g_Log, DBSTR "No loc tables found - eek! - trying to load 'Common.loc' for the default language" );
		UI::C_TextStringID::LocTable().Load( "gamedata\\xt\\Loc\\Common.loc", NULL, NULL, 0 );
	}
	else
	{
		char buf[128];

		// load them all in
		for ( S32 i = 0; i < m_UITableNames.count; i++ )
		{
			sprintf_s(buf, lengthof(buf), "gamedata\\xt\\Loc\\%s.loc", m_UITableNames[i].Text());
			if (Sys::GetDefaultFileSystem()->FileExists(buf))
				UI::C_TextStringID::LocTable().Load( buf, pLanguageCode, NULL, i );
		}
	}

	UI::C_TextStringID::LocTable().CreateLuaEnums( UI::g_pUILua->GetState() );

	//--------------------------------------------------------------------------------------
	// Load in the game loc tables here so that they dont get pushed into lua. As that would be bad
	//--------------------------------------------------------------------------------------
	if ( m_GameTableNames.elements )
	{
		char buf[128];

		// load them all in
		for ( S32 i = 0; i < m_GameTableNames.count; i++ )
		{
			sprintf_s(buf, lengthof(buf), "gamedata\\xt\\Loc\\%s.loc", m_GameTableNames[i].Text());
			if (Sys::GetDefaultFileSystem()->FileExists(buf))
				UI::C_TextStringID::LocTable().Load( buf, pLanguageCode, NULL, m_UITableNames.count + i );
		}
	}

	// initialise the localiser
	UI::C_UILocaliser::Instance().Init();

	Sys::SetFileReadErrorStringCallback(&GetFileErrorString);

	Game::C_GameSystems::Instance().MountPaksByLanguageCode(pLanguageCode);

	m_bInitialised = true;

	if( true == g_DebugLoadMultiplayerTSLocFromDiskOnStartup )
		LoadMultiplayerTSLocFromDisk(NULL);

	MEMORY_TRACKER_POP("ui.loc");
}

void C_LocData::Update()
{

}

bool C_LocData::ReplaceLoc( const char *locName, const U8* pMemory, const U32 size )
{
	if ( !m_bInitialised )
		return false;

	for ( S32 i = 0; i < m_UITableNames.count; ++i )
	{
		if( 0 == strcmp(locName, m_UITableNames[i].Text() ) )
		{
			UI::C_TextStringID::LocTable().Destroy( i );

			return UI::C_TextStringID::LocTable().LoadFromMemory( pMemory, size, i );
		}		
	}

	LOGERROR( UI::g_Log, DBSTR "Loc file: %s was not found in UI Tables.", locName );

	return false;
}


void C_LocData::Delete()
{
	UI::C_TextStringID::LocTable().DestroyAll();
	m_bInitialised = false;
}


// sorcery callback
void C_LocData::SorceryCallback( XtEventType type, XtRecord* pRecord, C_LocData* pLoc, XtCallbackContext* )
{
	switch ( type )
	{
		case kXtEtConstruct:
			// set the global instance
			C_LocData::Instance( *pLoc );
			break;

		case kXtEtDestruct:
			//
			break;
	}
}
