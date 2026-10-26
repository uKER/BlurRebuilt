#include "Stdafx.h"

#include <ui/UIDefines.h>
#include <ui/Game/SaveSystem/SaveSystem.h>
#include <ui/Game/UILua.h>
#include <ui/Graphics/Localisation/loc.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

namespace UI
{

// --------------------------
// C_SaveInfoInterfaceProfile
// --------------------------
class C_SaveInfoInterfaceProfile : public C_SaveInfoInterface
{
public:
	C_SaveInfoInterfaceProfile()
		: C_SaveInfoInterface()
	{
	#ifdef PLATFORM_XENON
		m_SaveRoot = "SAVE";
		m_bRoamable = true;
	#endif
	#ifdef PLATFORM_PS3
		m_TitleId = "ABCD12345";
		m_FilePath = "MemSavePS3";
		m_uSizeIcon0 = 30554;
		m_uSizeIcon1 = 1718272;
		m_uSizePic1 = 715488;
		m_uSizeSnd0 = 851224;
		m_ListName = "REPLAY";

		SetParamSFO_Title( L"Test" );							// TODO - localise
		SetParamSFO_Detail( L"Test Save Data Detail Info" );	// TODO - localise
		SetSubtitle( L"Save Game" );							// TODO - localise
		SetListNewDataTitle( L"Create New Data" );				// TODO - localise

		SetSecureFieldId( "WE <3 PS3 ... ??" );
	#endif
	#ifdef PLATFORM_PC
		m_SaveRoot = "SAVE";
		m_EnumerationFileExtension = "save";
	#endif
	}

public:
	void			SetupFiles()
					{
						m_FileInfo.PushBack( 
							C_SaveFileInfo(
								  200 * 1024											// m_sFileSize
								, "PROFILE1"											// m_FileName
							#ifdef PLATFORM_XENON
								, "SAVE:\\savegame"										// m_InternalName
								, UI::C_TextStringID( "PRO_SAVE_GAME_NAME" ).String()	// m_DisplayName
							#endif
							)
						);

						m_FileInfo.PushBack( 
							C_SaveFileInfo(
								  100 * 1024											// m_sFileSize
								, "PROFILE2"											// m_FileName
							#ifdef PLATFORM_XENON
								, "SAVE:\\savegame"										// m_InternalName
								, UI::C_TextStringID( "PRO_SAVE_GAME_NAME" ).String()	// m_DisplayName
							#endif
								, true													// m_bOptional
							)
						);
					}

	bool			Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
					{
						return true;
					}

	bool			Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize, const S32 sUserData )
					{
						return true;
					}
};

static C_SaveInfoInterfaceProfile* s_pSaveInfoInterface = NULL;

static void CheckForSaveInfoInterface()
{
	if ( !s_pSaveInfoInterface )
	{
		s_pSaveInfoInterface = new C_SaveInfoInterfaceProfile();
		s_pSaveInfoInterface->SetupFiles();
	}
}

// ClearGameProfile
static S32 ClearGameProfile( lua_State* pState )
{
	if ( lua_type( pState, 1 ) != LUA_TNUMBER )
	{
		g_pUILua->StackDump( pState, "ClearGameProfile( padNo ) - incorrect arguments" );
		return 0;
	}

	S32 sPadNo = (S32)lua_tonumber( pState, 1 );

	// TODO - reset the game profile

	return 0;
}

// SetProfileSaveInfoInterface
static S32 SetProfileSaveInfoInterface( lua_State* pState )
{
	CheckForSaveInfoInterface();

	s_pSaveInfoInterface->m_eType = E_UiSaveInfoType_Default;
#if PLATFORM_XENON || PLATFORM_PC
	bool bCheckOverwrites = ( ( lua_type( pState, 1 ) == LUA_TBOOLEAN ) && ( lua_toboolean( pState, 1 ) != 0 ) );
	s_pSaveInfoInterface->m_bCheckOverwrites = bCheckOverwrites;
#endif
	C_SaveSystem::Instance().SaveInfoInterface( s_pSaveInfoInterface );
	return 0;
}

// SetProfileSaveInfoInterfaceEnumerate
static S32 SetProfileSaveInfoInterfaceEnumerate( lua_State* pState )
{
	CheckForSaveInfoInterface();

	s_pSaveInfoInterface->m_eType = E_UiSaveInfoType_Enumerate;
	S32 sEnumeration = ( lua_type( pState, 1 ) == LUA_TNUMBER ) ? ( (S32)lua_tonumber( pState, 1 ) - 1 ) : -1;
	s_pSaveInfoInterface->m_sEnumeration = sEnumeration;
	C_SaveSystem::Instance().SaveInfoInterface( s_pSaveInfoInterface );
	return 0;
}

// the library
static const luaL_reg game_profile_lib[] = 
{
	{ "ClearGameProfile",						ClearGameProfile },
	{ "SetProfileSaveInfoInterface",			SetProfileSaveInfoInterface },
	{ "SetProfileSaveInfoInterfaceEnumerate",	SetProfileSaveInfoInterfaceEnumerate },

	// end of list
	{ NULL, NULL }
};

LUALIB_API int luaopen_game_profile( lua_State* pState )
{
	luaL_openlib( pState, "GameProfile", game_profile_lib, 0 );
	return 1;
}

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on)
#endif
