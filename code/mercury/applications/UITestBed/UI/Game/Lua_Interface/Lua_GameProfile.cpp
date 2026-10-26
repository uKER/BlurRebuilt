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
#ifdef PLATFORM_XENON
public:
	const char*		SaveRoot()			{ return "SAVE"; }
	const char*		InternalName()		{ return "SAVE:\\savegame"; }
	const char*		FileName()			{ return "testprofile.save"; }
	const WCHAR*	DisplayName()		{ return UI::C_TextStringID( "IDS_PROFILE_SAVE_GAME_NAME" ).String(); }
	const bool		Roamable()			{ return true; }
	const bool		CheckOverwrites()	{ return false; }
#elif PLATFORM_PS3
public:
	virtual const char*		TitleID()			{ return "ABCD12345"; }
	virtual const char*		ParamSFO_Title()	{ return "Test"; }
	virtual const char*		ParamSFO_Detail()	{ return "Test Save Data Detail Info"; }
	virtual const char*		Subtitle()			{ return "Save Game"; }
	virtual Util::C_String	FilePath()			{ return "Image\\MemSavePS3"; }
	const S32				SizeIcon0()			{ return 30554; }
	const S32				SizeIcon1()			{ return 1718272; }
	const S32				SizePic1()			{ return 715488; }
	const S32				SizeSnd0()			{ return 851224; }
	const char*				SecureFieldId()		{ return "WE <3 PS3 ... ??"; }
#elif PLATFORM_PC
public:
	const char*				SaveRoot()			{ return "Save"; }
	const char*				FileName()			{ return "Testprofile.save"; }
	const bool				CheckOverwrites()	{ return false; }
#endif

public:
	const U32		BufferSize()		{ return 200 * 1024; }

	bool			Load( const S32 sPadNo, U8* pBuffer, const U32 uDataSize )	{ return true; }
	bool			Save( const S32 sPadNo, U8* pBuffer, const U32 uDataSize )	{ return true; }
};

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
	static C_SaveInfoInterfaceProfile saveInfoInterface;

	C_SaveSystem::Instance().SaveInfoInterface( &saveInfoInterface );

	return 0;
}

// the library
static const luaL_reg game_profile_lib[] = 
{
	{ "ClearGameProfile",				ClearGameProfile },
	{ "SetProfileSaveInfoInterface",	SetProfileSaveInfoInterface  },

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
