#include "stdafx.h"

#include "ui/UIDefines.h"
#include "ui/UiInterface.h"
#include "ui/Graphics/Localisation/loc.h"
#include "ui/Graphics/Font/Font.h"

#ifdef PLATFORM_PS3
#include <wchar.h>
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", off)
#endif

#ifndef CFG_GOLD
Debug::Bool gUiLocalisationDisableStringLookup("Ui.Localisation.DisableStringLookup", false);
Debug::Bool gUiLocalisationDisableIdLookup("Ui.Localisation.DisableIdLookup", false);
#endif


namespace UI
{

// Singleton object which contains all the sorcery strings
C_TextStringID::C_Loc C_TextStringID::m_Loc;

#if 1
void C_TextStringID::C_Loc::Load(const Sys::C_FilePath& filename, const char* lang, Sys::I_FileSystem* fs, const S32 handle )
{
	if ( !lang )
		lang = "en";	// default to english

	// Check if handle already exists
	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		if( (*it)->Handle() == handle )
		{
			LOGERROR( g_Log, DBSTR "Handle aleady exists" );
			return;
		}
	}

	C_LocHackItem* pItem = new C_LocHackItem( filename, lang, fs, handle );

	if( !pItem )
	{
		LOGFATAL( g_Log, DBSTR "Unable to create loc item" );
		return;
	}

	// Push in reverse order so packs added last are searched first
	m_LocList.PushFront( pItem );
	
	m_Initialised = true;
	m_TableValid = true;
}

bool C_TextStringID::C_Loc::LoadFromMemory( const U8* pMemory, const U32 size, const S32 handle )
{
	// Check if handle already exists
	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		if( (*it)->Handle() == handle )
		{
			LOGERROR( g_Log, DBSTR "Handle aleady exists" );
			return false;
		}
	}
	
	// these assume single language
	C_LocHackItem* pItem = new C_LocHackItem( "", pMemory, size, handle );

	if( !pItem )
	{
		LOGFATAL( g_Log, DBSTR "Unable to create loc item");
		return false;
	}

	// Push in reverse order so packs added last are searched first
	m_LocList.PushFront( pItem );

	m_Initialised = true;
	m_TableValid = true;

	return true;
}

void C_TextStringID::C_Loc::Reload( )
{
	if(!m_Initialised)
	{
		LOGFATAL( g_Log, DBSTR " Attempting to reload loc strings when they've not been initialised\n");
		return;
	}
	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		(*it)->LocHack().Free();
		(*it)->Load();
	}

	// Rebuild the lua table
	if(m_pState)
		CreateLuaEnums(m_pState);

	m_TableValid = false;
}

void C_TextStringID::C_Loc::CreateLuaEnums( lua_State *pState )
{
	if(!m_Initialised)
	{
		LOGFATAL( g_Log, DBSTR " Attempting to create lua enums when loc strings have not been loaded\n");
		return;
	}
	assert(pState);

	// Create a new thread (for doing global table searches)
	if( !m_pState )
	{
		m_pState = lua_newthread( pState );
		lua_pushvalue(pState, -1);
		lua_setglobal(pState,"lua_thread");
		lua_pop(pState,1);
	}

	// Create new text table
	lua_newtable(pState);
	lua_pushvalue(pState,-1);
	lua_setglobal(pState,LUA_TEXT_TABLE_NAME);
	
	const U32 uBuffLength = 512;
	char buff[ uBuffLength ];

	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		XtLocHack& lh = (*it)->LocHack();
		XtU32 count = lh.Lookup_Count();

		for(XtU32 i = 0; i < count; i++)
		{
			const char *name = lh.Lookup_Name(i);

			// PR - sorting out the loc string for each platform

			// - check for postfix 360, PS3, PC and chop it off and push only if correct platform
			memset( buff, 0, sizeof( buff ) );

			S32 sLen = strlen( name );

			if ( !strcmp( name + sLen - 4, "_360" ) || !strcmp( name + sLen - 4, "_PS3" ) || !strcmp( name + sLen - 3, "_PC" ) )
			{
			#ifdef PLATFORM_PS3
				if ( !strcmp( name + sLen - 4, "_PS3" ) )
				{
					STRCPY( buff, uBuffLength, name );
					buff[ sLen - 4 ] = '\0';
					lua_pushstring( pState, buff );
					lua_pushinteger( pState, lh.FindId( name ) ); 
					lua_rawset( pState, -3 );
				}
			#endif
			#ifdef PLATFORM_PC
				if ( !strcmp( name + sLen - 3, "_PC" ) )
				{
					STRCPY( buff, uBuffLength, name );
					buff[ sLen - 3 ] = '\0';
					lua_pushstring( pState, buff );
					lua_pushinteger( pState, lh.FindId( name ) ); 
					lua_rawset( pState, -3 );
				}
			#endif
			}
			else
			{
				// just push it in
				lua_pushstring(pState, name); 
				//lua_pushnumber(pState, lh.FindId(name));		// This is causing last byte to be zeroed out on the pc??
				lua_pushinteger(pState, lh.FindId(name)); 
				lua_rawset(pState,-3);
			}
		}
	}
	// Restore the lua stack position
	lua_pop(pState,1);
}

void C_TextStringID::C_Loc::Destroy( const S32 handle )
{
	if(!m_Initialised)
	{
		LOGFATAL( g_Log, DBSTR " Attempting to destroy loc strings when they're not initialised\n");
		return;
	}

	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		if( (*it)->Handle() == handle )
		{
			SAFE_DELETE_PTR( *it );
			m_LocList.Erase( it );
			return;
		}
	}

	LOGERROR( g_Log, DBSTR " Failed to find loc handle : %d to destroy ", handle );
}

void C_TextStringID::C_Loc::DestroyAll()
{
	if(!m_Initialised)
	{
		LOGFATAL( g_Log, DBSTR " Attempting to destroy loc strings when they're not initialised\n");
		return;
	}

	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); )
	{
		SAFE_DELETE_PTR( *it );
		it = m_LocList.Erase( it );
	}

	m_Initialised = false;
}

#ifndef CFG_GOLD
Debug::Bool	g_uiDebugShowStringIDS("UI.Localisation.ShowStringIDS", false );
#endif

const char *C_TextStringID::Name( )
{
	if( IsLocString() == false )
		return NULL;

	return m_Loc.FindName( m_Value );
}

const WCHAR *C_TextStringID::String( )
{
	bool fRefresh = false;

#ifndef CFG_GOLD
	// Always do lookup if we've ever switched on showing IDS strings
	static bool s_uiDebugAlwaysClearStrings = false;
	if(g_uiDebugShowStringIDS)
		s_uiDebugAlwaysClearStrings = true;

	if(s_uiDebugAlwaysClearStrings)
		m_pString = NULL;
#endif
	// Table has changed don't rely on the pointer
	if(!m_Loc.TableValid())
		m_pString = NULL;

	// Do lookup
	if (!m_pString)
		fRefresh = true;
#if 0
	else if (m_Generation != UILip::GetGeneration())
		fRefresh = true;

	if (fRefresh && !UILip::IsDirty())
		m_Generation = UILip::GetGeneration();
#endif

	if (fRefresh)
		Find();

#if defined OLD_USE_LIP && defined _DEBUG
	if ( m_pString == NULL )
	{
		CHAR achMsg[128];
		sprintf_s(achMsg,128,"Failed to load string with id: %u\n", m_Value);
		OutputDebugStringA(achMsg);
	}
#endif

	return m_pString ? m_pString : L"";
}

const WCHAR* C_TextStringID::C_Loc::FindText(XtU32 id)
{
#ifndef CFG_GOLD
	#define MAX_IDS_STRING		(250)
	static C_WideString<MAX_IDS_STRING> s_uiDebugStringBuffer;

	if(g_uiDebugShowStringIDS && m_pState)
	{
		lua_getglobal(m_pState,LUA_TEXT_TABLE_NAME);
		if(lua_type(m_pState, -1) == LUA_TTABLE)
		{
			const lua_Integer num = (lua_Integer)id;
			const char *key_name = NULL;
			const char *table_error = "TABLE_ERROR";

			lua_pushnil(m_pState);
			while(lua_next(m_pState,-2)!=0 && key_name==NULL)
			{
				if(lua_type(m_pState,-1) == LUA_TNUMBER)
				{
					if(lua_tointeger(m_pState,-1) == num)
					{
						if(lua_type(m_pState,-2) == LUA_TSTRING)
							key_name = lua_tostring(m_pState,-2);
					}
				}
				lua_pop(m_pState,1); // Pop off the next item 
			}
			lua_pop(m_pState,-1); // Pop off nil

			s_uiDebugStringBuffer.Format( L"%S", key_name);

			return s_uiDebugStringBuffer;
		}
		lua_pop(m_pState,-1); // Pop off UIText
	}
#endif
	assert(m_Initialised);	

#ifdef OLD_USE_LIP
    if (id >= IDS_LANGUAGE_FIRST)
    {
        const WCHAR* lang = UILip::GetLanguage(UILip::IdToIndex(id));

        if (lang) return lang;
    }
#endif

#ifndef CFG_GOLD
	if (gUiLocalisationDisableIdLookup)
		return L"BANANA";
#endif

	
	// If we've got a multiplayers loc file loaded
	if (m_pLocMultiplayerTS) 
	{
		XtLocHack& lh = m_pLocMultiplayerTS->LocHack();

		const wchar_t* p = lh.FindText(id, true);
		if( p )
			return p;
	}

	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		XtLocHack& lh = (*it)->LocHack();

		const wchar_t* p = lh.FindText(id,true);
		if( p )
			return p;
	}

	return L"";
}

const char* C_TextStringID::C_Loc::FindName( XtU32 id )
{
	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		XtLocHack& lh = (*it)->LocHack();

		const char* p = lh.FindName( id );
		if( p )
			return p;
	}

	return NULL;
}

XtU32 C_TextStringID::C_Loc::FindId(const char* name)
{
	assert(m_Initialised);

#ifndef CFG_GOLD
	if (gUiLocalisationDisableStringLookup)
		return 0;
#endif

	for( Btl::List<C_LocHackItem *>::Iterator it(m_LocList.Begin()); it != m_LocList.End(); ++it )
	{
		XtU32 id = (*it)->LocHack().FindId(name); 
		if( id )
			return id;
	}

	return 0;
}

bool C_TextStringID::C_Loc::LoadMultiplayerTSFromMemory( const U8* pMemory, const U32 size, const char* lang )
{
	if ( !lang )
		lang = "en";	// default to english

	// unload loc table
	if ( m_pLocMultiplayerTS )
		SAFE_DELETE_PTR( m_pLocMultiplayerTS );

	// these assume single language
	m_pLocMultiplayerTS = new C_LocHackItem( lang, pMemory, size, 0 );

	if( !m_pLocMultiplayerTS )
	{
		LOGFATAL( g_Log, DBSTR "Unable to create loc item");
		return false;
	}

	return true;
}


bool C_TextStringID::C_Loc::LoadMultiplayerTSFromDisk( const Sys::C_FilePath& filename, const char* lang )
{
	if ( !lang )
		lang = "en";	// default to english

	// unload loc table
	if ( m_pLocMultiplayerTS ) 
		SAFE_DELETE_PTR( m_pLocMultiplayerTS );

	// load loc table
	m_pLocMultiplayerTS = new C_LocHackItem( filename, lang, NULL, 0 );

	if( !m_pLocMultiplayerTS )
	{
		LOGFATAL( g_Log, DBSTR "Unable to create loc item");
		return false;
	}

	return true;
}

#else

#define MAX_TEMP_STRING 1024
#define MAX_IDS_STRING 64

// TODO: [Rog] The way this function processes an INI file isn't great. It relies heavily on everything being "just right" to work. Would be good to fix sometime.
void Loc::Load(lua_State *pState, const Util::C_String& directoryPrefix )
{
// 	WCHAR wc;
	unsigned short wc;

	int count=0;
	WCHAR temp_string[MAX_TEMP_STRING];
	bool end=false;

	num_strings=0;

	Sys::I_FileSystem *fs = Sys::GetDefaultFileSystem();
	Sys::I_File* fp;


#ifdef PLATFORM_PS3
	fp = fs->Open( directoryPrefix + "Ui\\text\\english.ini" );		//English is default
#else
	fp = fs->Open("Ui\\text\\english.ini");		//English is default
#endif

	if(!fp)
	{
		LOGFATAL( g_Log,  "Open failed");
		return;
	}

	Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar(*fp);

	// clear UNICODE header
	ar >> wc;

	// Read first proper character
	ar >> wc;

	// Lua text table creation
	WCHAR ids_string[MAX_IDS_STRING];
	if(pState)
	{
		m_pState = pState;

		lua_newtable(pState);
		lua_pushvalue(pState,-1);
		lua_setglobal(pState,LUA_TEXT_TABLE_NAME);
	}

	while(!end)
	{
		int ids_count = 0;
		bool finished_ids_copy = false;
		memset(ids_string,NULL,sizeof(WCHAR)*MAX_IDS_STRING);

		// clear the define - by reading upto '='
		while (wc != 0x003d)
		{
			// [Rog] Changed so this function actually checks for EOF rather than relying on the underlying serialiser to "hopefully" return 0 in the variable when "out of data".
            if (fp->GetPosition() >= fp->Length())
			{
				end = true;
				break;
			}

			if(!finished_ids_copy)
			{
				if(ids_count < MAX_IDS_STRING)
				{
					ids_string[ids_count++] = wc;
					assert(ids_count <= MAX_IDS_STRING);
				}
				else
				{
					LOGFATAL( g_Log, DBSTR " ids string is too long, truncating to %d chars\n",MAX_IDS_STRING);
				}
			}

			ar >> wc;

			if(wc == 0x0020) // Space
				finished_ids_copy = true;

			// if the letter isn't an IDS ... then quit out
			if (wc < 0x0020) 
			{
				end=true;
				break;
			}
		}

		if (end) continue;

		//Read Next Character
		ar >> wc;

		// clear trailing space if we have them
		while(wc == 0x0020)
		{
			ar >> wc;
		}

		// store the string
		count=0;
		memset(temp_string,NULL,sizeof(WCHAR)*MAX_TEMP_STRING);
		while(wc != 0x000a && wc != 0x000d)
		{
			temp_string[count++] = wc;
			ar >> wc;		
		}

		//take into account the null terminator
		count++;
		assert(count <= MAX_TEMP_STRING);

		// Add to Lua
		if(pState)
		{
			char u8_ids_string_buffer[MAX_TEMP_STRING];
			sprintf_s(u8_ids_string_buffer, MAX_TEMP_STRING, "%S", ids_string);

			lua_pushstring(pState, u8_ids_string_buffer); 
			lua_pushnumber(pState, num_strings); 
			lua_rawset(pState,-3);
		}

		// clear the line feed
		ar >> wc;

		if(wc == 65535) 
		{
			end=true;
			continue;
		}

		strings[num_strings] = new WCHAR[count];

		memset(strings[num_strings], NULL, sizeof(WCHAR)*count);

		wcscpy(strings[num_strings],temp_string);
		num_strings++;

		ar >> wc;
	}
	delete fp;

	// Restore the lua stack position
	if(pState)
		lua_pop(pState,1);
}

#ifndef CFG_GOLD
Debug::Bool	g_uiDebugShowStringIDS("UI.ShowStringIDS", false );
WCHAR s_uiDebugStringBuffer[MAX_IDS_STRING];
#endif

WCHAR *Loc::GetString(int num)
{
	if(num >= 0 && num < num_strings)
	{
#ifndef CFG_GOLD
		if(g_uiDebugShowStringIDS && m_pState)
		{
			lua_getglobal(m_pState,LUA_TEXT_TABLE_NAME);
			if(lua_type(m_pState, -1) == LUA_TTABLE)
			{
				const char *key_name = NULL;
				const char *table_error = "TABLE_ERROR";

				lua_pushnil(m_pState);
				while(lua_next(m_pState,-2)!=0 && key_name==NULL)
				{
					if(lua_type(m_pState,-1) == LUA_TNUMBER)
					{
						if(lua_tonumber(m_pState,-1) == num)
						{
							if(lua_type(m_pState,-2) == LUA_TSTRING)
								key_name = lua_tostring(m_pState,-2);
						}
					}
					lua_pop(m_pState,1); // Pop off the next item 
				}
				lua_pop(m_pState,-1); // Pop off nil

				SWNPRINTF(s_uiDebugStringBuffer, MAX_IDS_STRING, REFORMAT(L"%S"), key_name);

				return s_uiDebugStringBuffer;
			}
			lua_pop(m_pState,-1); // Pop off UIText
		}
#endif
		return(strings[num]);
	}
	else
	{
		return(L"Invalid String!");
	}
}

void Loc::Destroy()
{
	for(int a=0;a<num_strings;a++)
	{
		delete [] strings[a];
		strings[a] = NULL;
	}
}

#endif

}; // namespace UI

#ifdef CFG_ALLOW_OPTIMIZE_OFF_UI
#pragma optimize("", on) 
#endif
