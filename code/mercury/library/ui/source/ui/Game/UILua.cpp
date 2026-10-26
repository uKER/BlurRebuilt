#include "stdafx.h"

#include "ui/Game/UILua.h"
#include "ui/Game/UIScreenControl.h"
#include "ui/UiInterface.h"

#include <ui/util/uicrashinfo.h>

namespace UI
{

#ifndef CFG_GOLD
#if defined( USER_PEREID ) /*|| defined(USER_WBROWN)*/
Debug::Bool g_debugUseLUAFromHDD("UI.Lua.LoadFromHDD", true, Debug::Persist());
#else
Debug::Bool g_debugUseLUAFromHDD("UI.Lua.LoadFromHDD", false, Debug::Persist());
#endif
#endif

C_UILua* g_pUILua;

#ifndef CFG_GOLD

CrashInfo_s CrashInfo = { 0 };
CyclicDebugLogger<40, 60> g_LuaScreenLogger;

#endif

static int LuaPanicFunction(lua_State *pState)
{
	LOGFATAL(g_Log, "Fatal LUA error - %s\n", lua_tostring(pState,-1));
	return 0;
}

C_UILua::C_UILua()
{
#ifndef CFG_GOLD
	CrashInfo.luaScreenLogger = &g_LuaScreenLogger;
#endif

	m_pScreens = NULL;
	m_pUIData = NULL;
	m_LuaState = NULL;
}

C_UILua::~C_UILua()
{
	if (m_pUIData != NULL)
	{
		delete m_pUIData;
	}
}

void do_console_command(Debug::ConsoleEvent *ev,void *context)
{
	((C_UILua *)context)->	m_ConsoleString+=ev->text;
	((C_UILua *)context)->	m_ConsoleString+="\r\n";
}

void C_UILua::Init(bool full_init, const char *pHDDScriptsDir, void *ptrToLuaScripts )
{
	MEMORY_TRACKER_PUSH("ui.lua");

	mDoingFullInit	= full_init;

#ifdef USE_LUA_PAK_FILE
	if( ptrToLuaScripts )
	{
		U8* curPtr = (U8*)ptrToLuaScripts; 

		// Check header
		if (*((DWORD*)curPtr) == 'BZUI') 
		{
			curPtr+=4;

			mLuaScriptNumber = *((DWORD*)curPtr);
			curPtr += 4;

			assert(m_pScreens == NULL);
			m_pScreens = (LUA_SCREEN_INFO*)curPtr;
			curPtr += sizeof(LUA_SCREEN_INFO) * mLuaScriptNumber;
			
			m_pStringTable	= (char*)curPtr;
		}
		else if (*((DWORD*)curPtr) == 'IUZB') 
		{
			curPtr+=4;

			Ut::NativeToBigEndian::EndianSwap(*((U32*)curPtr));

			mLuaScriptNumber = *((DWORD*)curPtr);
			curPtr += 4;

			assert(m_pScreens == NULL);
			m_pScreens = (LUA_SCREEN_INFO*)curPtr;
			curPtr += sizeof(LUA_SCREEN_INFO) * mLuaScriptNumber;

			for (int iScript=0; iScript < mLuaScriptNumber; iScript++)
			{
				Ut::NativeToBigEndian::EndianSwap(m_pScreens[iScript].screenNameOffset);
				Ut::NativeToBigEndian::EndianSwap(m_pScreens[iScript].dataOffset);
				Ut::NativeToBigEndian::EndianSwap(m_pScreens[iScript].compressedSize);
				Ut::NativeToBigEndian::EndianSwap(m_pScreens[iScript].uncompressedSize);
			}
			
			m_pStringTable	= (char*)curPtr;
		}
		else
		{
			LOGFATAL(g_Log, "Error with Lua pack file");
		}
	}
	else
	{
		LOGFATAL( g_Log, "Pointer to Lua pack file == NULL" );
	}
#endif

	m_HDDScriptsDir = Util::C_String(pHDDScriptsDir);

	//Now init the LUA system
#if LUA_VERSION_NUM >= 501
#ifndef CFG_SPEED
	::Memory::ContextId idLuaUI = MEMORY_TRACKER_CONTEXT_ID("lua.ui");
#else
	U32 idLuaUI = 0;
#endif
	m_LuaState = lua_newstate(Lua::MemoryAllocator, (void*)((U32)idLuaUI));	// new for 5.1
#else
	m_LuaState = lua_open();
#endif

	lua_atpanic(m_LuaState,LuaPanicFunction);

	// Garbage collection now handled by the application loop.
#if PLATFORM_PC
	lua_gc(m_LuaState,LUA_GCSETPAUSE,100);
	lua_gc(m_LuaState,LUA_GCSETSTEPMUL,200);
#endif

	UiInterface()->RegisterLibs( m_LuaState );

	bool fromHdd = false;
#ifndef CFG_GOLD
	fromHdd = g_debugUseLUAFromHDD;
#endif
	g_pUILua->LoadCallFile("UIEnums.lua", fromHdd);

	Debug::RegisterConsole("UiLua",do_console_command,this);

	MEMORY_TRACKER_POP("ui.lua");
}

void C_UILua::LoadCallSetupScript( const U32 initParam, const bool bForceOnlyHDD )
{
	g_pUILua->LoadCallFile("UISetup.lua", bForceOnlyHDD );

	lua_getglobal( m_LuaState, "UISetup" );
	lua_pushnumber( m_LuaState, initParam );
	if(lua_pcall( m_LuaState, 1, 0, 0 ))
	{
		LOGERROR( g_Log, "LUA runtime error : %s - calling global function UISetup", lua_tostring(m_LuaState,-1));
		g_pUILua->StackDump( m_LuaState, "LoadCallSetupScript");
		lua_pop(m_LuaState,1);
	}
}

void C_UILua::End()
{
	lua_close(m_LuaState);	
}

#define LEVELS1	12	/* size of the first part of the stack */
#define LEVELS2	10	/* size of the second part of the stack */

static lua_State *getthread (lua_State *L, int *arg) {
  if (lua_isthread(L, 1)) {
    *arg = 1;
    return lua_tothread(L, 1);
  }
  else {
    *arg = 0;
    return L;
  }
}

// borrowed from db_errorfb : ldlib.c
void C_UILua::DumpCallStack( lua_State *L )
{
  int level;
  int firstpart = 1;  /* still before eventual `...' */
  int arg;
  lua_State *L1 = getthread(L, &arg);
  lua_Debug ar;

#if 0
	if (lua_isnumber(L, arg+2)) 
	{
		level = (int)lua_tointeger(L, arg+2);
		lua_pop(L, 1);
	}
	else
		level = (L == L1) ? 1 : 0;  /* level 0 may be this own function */
	if (lua_gettop(L) == arg)
		lua_pushliteral(L, "");
	else 
	if (!lua_isstring(L, arg+1))
		return 1;  /* message is not a string */
	else
		lua_pushliteral(L, "\n");
#else
	level = (L == L1) ? 1 : 0;
#endif

	LOGDEBUG( g_Log, "CallStack : ");
	while (lua_getstack(L1, level++, &ar)) 
	{
		Util::C_String string;
		Util::C_String format;

		if (level > LEVELS1 && firstpart) 
		{
		  /* no more than `LEVELS2' more levels? */
		  if (!lua_getstack(L1, level+LEVELS2, &ar))
				level--;  /* keep going */
		  else 
		  {
				// lua_pushliteral(L, "\n\t...");  /* too many levels */
				string += "\n\t...";
				while (lua_getstack(L1, level+LEVELS2, &ar))  /* find last levels */
					level++;
		  }
		  firstpart = 0;
		  continue;
		}

		//	lua_pushliteral(L, "\n\t");
		string += "\t";
		lua_getinfo(L1, "Snl", &ar);
		//	lua_pushfstring(L, "%s:", ar.short_src);
		string += format.Format("%s:", ar.short_src);
		if (ar.currentline > 0)
		{
			string += format.Format( "%d:", ar.currentline );
			// lua_pushfstring(L, "%d:", ar.currentline);
		}
		if (*ar.namewhat != '\0')  /* is there a name? */
		{
			string += " in function '";
			string += ar.name;
			string += "'";
			// lua_pushfstring(L, " in function " LUA_QS, ar.name);
		}
		else 
		{
			if (*ar.what == 'm')  /* main? */
			{
				string += " in main chunk";
				// lua_pushfstring(L, " in main chunk");
			}
			else if (*ar.what == 'C' || *ar.what == 't')
			{
				string += " ?";
				//lua_pushliteral(L, " ?");  /* C function or tail call */
			}
			else
			{
				string += format.Format( " in function <%s:%d>", ar.short_src, ar.linedefined );
				// lua_pushfstring(L, " in function <%s:%d>", ar.short_src, ar.linedefined);
			}
		}
		// lua_concat(L, lua_gettop(L) - arg);
		LOGDEBUG( g_Log, string.Chars() );
	}
}

void C_UILua::StackDump( lua_State *L,char *message ) 
{
	int i;
	int top = lua_gettop(L);

	// Show 1 level stack
	luaL_where(L, 1);
	LOGWARN( g_Log, "(%s) : %s\n", lua_tostring(L,-1), message );
	lua_pop(L, 1);

	// Dump stack
	LOGDEBUG( g_Log, "StackDump : " );
	if( top )
	{
		for (i = 1; i <= top; i++) 
		{  /* repeat for each level */
			int t = lua_type(L, i);
			switch (t)
			{
			case LUA_TSTRING:  /* strings */
				LOGDEBUG( g_Log, "\t(%d) string : `%s'",i, lua_tostring(L, i));
				break;

			case LUA_TBOOLEAN:  /* booleans */
				LOGDEBUG( g_Log, "(%d) bool : %s",i,lua_toboolean(L, i) ? "true" : "false");
				break;

			case LUA_TNUMBER:  /* numbers */
				LOGDEBUG( g_Log, "\t(%d) number : %g",i, lua_tonumber(L, i));
				break;

			case LUA_TTABLE:
				LOGDEBUG( g_Log, "\t(%d) table : {}", i );
				break;

			case LUA_TFUNCTION:
				LOGDEBUG( g_Log, "\t(%d) function : ()", i );
				break;

			case LUA_TUSERDATA:
				LOGDEBUG( g_Log, "\t(%d) userdata : *", i );
				break;

			case LUA_TLIGHTUSERDATA:
				LOGDEBUG( g_Log, "\t(%d) lightuserdata : *", i );
				break;

			case LUA_TNIL:
				LOGDEBUG( g_Log, "\t(%d) nil : 0", i );
				break;

			default:  /* other values */
				LOGDEBUG( g_Log, "\t(%d) other : %s",i, lua_typename(L, t));
				break;
			}
		}
	}
	else
	{
		LOGDEBUG( g_Log, "\t[none]" );
	}

	// Show callstack
	DumpCallStack( L );
}

const F32 C_UILua::GetNumberField (const char *key)
{
	F32 result;
	lua_pushstring(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = (F32)lua_tonumber(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */
	return result;
}

const F64 C_UILua::GetDoubleNumberField (const char *key)
{
	F64 result;
	lua_pushstring(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = (F64)lua_tonumber(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */
	return result;
}

const bool C_UILua::GetBooleanField (const char *key)
{
	int result;
	lua_pushstring(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = lua_toboolean(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */

	if(result) return true;

	return false;
}

const F32 C_UILua::GetNumberField (int key)
{
	F32 result;
	lua_pushnumber(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = (F32)lua_tonumber(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */
	return result;
}

const F64 C_UILua::GetDoubleNumberField(int key)
{
	F64 result;
	lua_pushnumber(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = lua_tonumber(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */
	return result;
}

const bool C_UILua::GetBooleanField(int key)
{
	F64 result;
	lua_pushnumber(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = lua_toboolean(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */
	
	if(result) return true;

	return false;
}

const char *C_UILua::GetStringField (const char *key)
{
	const char *result;
	lua_pushstring(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = lua_tostring(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */
	return result;
}

const char *C_UILua::GetStringField (int key)
{
	const char *result;
	lua_pushnumber(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result = lua_tostring(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */
	return result;
}

const int C_UILua::GetFieldType (const char *key)
{
	int result;
	lua_pushstring(m_LuaState, key);
	lua_gettable(m_LuaState, -2);  /* get background[key] */

	result=lua_type(m_LuaState, -1);
	lua_pop(m_LuaState, 1);  /* remove number */

	return result;
}

const int C_UILua::GetFieldTypeFromTable(const char *table,const char *key)
{
	int result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetFieldType(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}


const F32 C_UILua::GetNumberFieldFromTable (const char *table,const char *key)
{
	F32 result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const F32 C_UILua::GetNumberFieldFromTable (const char *table,int key)
{
	F32 result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const F32 C_UILua::GetNumberFieldFromTable (int table,const char *key)
{
	F32 result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const F32 C_UILua::GetNumberFieldFromTable (int table,int key)
{
	F32 result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const bool C_UILua::GetBooleanFieldFromTable (const char *table,const char *key)
{
	bool result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetBooleanField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const bool C_UILua::GetBooleanFieldFromTable (const char *table,int key)
{
	bool result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetBooleanField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const bool C_UILua::GetBooleanFieldFromTable (int table,const char *key)
{
	bool result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetBooleanField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const bool C_UILua::GetBooleanFieldFromTable (int table,int key)
{
	bool result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetBooleanField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const F64 C_UILua::GetDoubleNumberFieldFromTable (const char *table,const char *key)
{
	F64 result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetDoubleNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const F64 C_UILua::GetDoubleNumberFieldFromTable (const char *table,int key)
{
	F64 result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetDoubleNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const F64 C_UILua::GetDoubleNumberFieldFromTable (int table,const char *key)
{
	F64 result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetDoubleNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const F64 C_UILua::GetDoubleNumberFieldFromTable (int table,int key)
{
	F64 result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetDoubleNumberField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const char* C_UILua::GetStringFieldFromTable (const char *table,const char *key)
{
	const char *result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetStringField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const char* C_UILua::GetStringFieldFromTable (const char *table,int key)
{
	const char *result;
	lua_pushstring(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetStringField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const char* C_UILua::GetStringFieldFromTable (int table,const char *key)
{
	const char *result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetStringField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

const char* C_UILua::GetStringFieldFromTable (int table,int key)
{
	const char *result;
	lua_pushnumber(m_LuaState, table);
	lua_gettable(m_LuaState, -2);  /* get table */

	result = GetStringField(key);

	lua_pop(m_LuaState, 1);  /* remove table */

	return result;
}

void C_UILua::SetCurrentPAK(const char *name)
{
	const LUA_SCREEN_INFO* curScreen = m_pScreens;

	for (int iScreen=0; iScreen < mLuaScriptNumber; iScreen++, curScreen++)
	{
		const char* screenName = m_pStringTable + curScreen->screenNameOffset;

		if (!stricmp(screenName, name))
		{
			mCurrentPAK = iScreen;
			return;
		}
	}

	LOGDEBUG(g_Log, "Could not find Lua script %s in PAK file \n", name);
}

#ifndef CFG_GOLD
C_UILua::C_StackCheck::~C_StackCheck()
{
	int top = lua_gettop( m_pState );
	if( top == m_Top )
		return;

	LOGERROR( UI::g_Log, "Lua Stack imbalance of %d", top - m_Top );
}

// Allow strings to be sent straight to the LUA console - designed specifically for the debug console
void C_UILua::ReceiveConsoleString(const char *message)
{
	int status = luaL_dostring(m_LuaState, message);
}

#endif

size_t gLuaPAKFilePosition;

void C_UILua::DoConsole()
{
	if(m_ConsoleString.Length())
	{
		int status=luaL_dostring(m_LuaState, m_ConsoleString.Chars());	

		if(status != 0)
		{
			Debug::ConsoleLn("UiLua",lua_tostring(m_LuaState, -1));
			lua_pop(m_LuaState, 1);  /* remove error message and _ALERT */
		}
		
		m_ConsoleString="";
	}
}


extern "C" size_t GetLuaPAKData(void *buff, size_t size)
{
	if ((S32)gLuaPAKFilePosition != -1)
	{
		int poffset		= g_pUILua->GetCurrentPAKOffset();
		size_t psize	= g_pUILua->GetCurrentPAKSize();

		U8* currentScreenData = g_pUILua->GetPAKScripts() + poffset + gLuaPAKFilePosition;

		if (!gLuaPAKFilePosition)
		{
			g_pUILua->m_zlib.Decompress_Init((U8*)buff, size);
		}
		else
		{
			g_pUILua->m_zlib.Decompress_SetOutputBuffer((U8*)buff, size);
		}

		int previousTotalIn		= g_pUILua->m_zlib.TotalIn();
		int previousTotalOut	= g_pUILua->m_zlib.TotalOut();

		g_pUILua->m_zlib.Decompress(currentScreenData, psize);

		gLuaPAKFilePosition += g_pUILua->m_zlib.TotalIn() - previousTotalIn;

		//send the next 'size' bytes
		//memcpy(buff, currentScreenData, size);
		//gLuaPAKFilePosition += size;
		return (g_pUILua->m_zlib.TotalOut() - previousTotalOut);
	}

	return 0;
}

#undef LUAL_BUFFERSIZE
#define LUAL_BUFFERSIZE 500
#define TEXT_FILE_BLOCK_EOF 0x1a

typedef struct LoadTextData 
{
	//FILE *			f;	// REPLACE
	Sys::I_File*	m_pFile;
	U8				buff[LUAL_BUFFERSIZE];
} LoadTextData;

// chunk reader function
static const char *LoadChunkReader(lua_State *L, void *user_data, size_t *size) 
{
	LoadTextData *ld = (LoadTextData *)user_data;
	
	Sys::BinaryDeserialiser<Ut::LittleEndianToNative> ar(*ld->m_pFile);

	char *cptr = (char*)ld->buff;

	size_t iCount=0;

	for(; (iCount < LUAL_BUFFERSIZE) && (ld->m_pFile->GetPosition() < ld->m_pFile->Length()); iCount++)
	{
		U8 c;

		ar >> c;

		bool file_end = false;

		switch(c)
		{
//			case '!':
			case 255:
			case 0:
			case TEXT_FILE_BLOCK_EOF:
				file_end = true;
				break;		
			default:
				*cptr++ = c;
				break;
		}

		if (file_end == true)
			break;
	}
  
	*size = iCount;
  
	return (iCount > 0) ? (char*)ld->buff : NULL;	// return buffer pointer or NULL if we have finished reading
}

static int FileError(lua_State *L, int fnameindex) 
{
  const char *filename = lua_tostring(L, fnameindex) + 1;
  
#ifndef PLATFORM_PS3
  lua_pushfstring(L, "cannot read %s: %s", filename, strerror(errno));
#else
  lua_pushfstring(L, "cannot read %s", filename);
#endif
  lua_remove(L, fnameindex);

  return LUA_ERRFILE;
}

LUALIB_API int LoadLUAFile(lua_State *L, Sys::I_File* fp, const Util::C_String& shortName) 
{
	LoadTextData ld;
	int status, readstatus;
	int fnameindex = lua_gettop(L) + 1;  // index of filename on the stack 

	lua_pushfstring(L, "@%s", shortName.Chars() );
	ld.m_pFile = fp;

	// call the LUA load function using new chunk reader
	status = lua_load(L, LoadChunkReader, &ld, lua_tostring(L, -1));
  
	readstatus = 0;//ferror(ld.f);	// REPLACE
  
	if (readstatus) 
	{
		lua_settop(L, fnameindex);  // ignore results from `lua_load' 
		return FileError(L, fnameindex);
	}
  
	lua_remove(L, fnameindex);
  
	return status;
}

LUALIB_API int LoadLUAFile(lua_State *L, const Util::C_String& fname, const Util::C_String& shortName)
{
	Sys::I_File * fp = Sys::GetDefaultFileSystem()->Open( fname.Chars() );
	
	if( !fp )
	{
#ifndef CFG_GOLD
		if( g_debugUseLUAFromHDD )
		{
			LOGDEBUG( g_Log, "Failed to load lua file \"%s\" ",fname.Chars() );
		}
		else
#endif
		{
			LOGDEBUG( g_Log, "Failed to load lua file \"%s\" ",fname.Chars() );
		}

		return 0;
	}

	LoadLUAFile(L, fp, shortName);

	delete fp;

	return 1;
}

const int C_UILua::LoadCallFile(const char *fname, bool forceFromHDD)
{
	LOGDEBUG( g_Log,  "Going to screen ... %s\n", fname);

#ifndef CFG_GOLD
	g_LuaScreenLogger.Add(fname);
#endif

#ifdef CFG_GOLD
	return lua_dopak(m_LuaState,fname);
#else // CFG_GOLD
#ifdef USE_LUA_PAK_FILE
	if((g_debugUseLUAFromHDD == false) && (!forceFromHDD))
	{
		const int ret = lua_dopak(m_LuaState,fname);
		if (ret)
		{
			LOGWARN( g_Log,  "Error in Lua file : %s (%d)\n", fname,ret);
			LOGWARN( g_Log, "%s\n", lua_tostring(m_LuaState,-1));
			lua_pop( m_LuaState, 1);  // pop error message from the stack
		}

		return ret;
	}
		
#endif
#endif // CFG_GOLD

	// This is only used on non-mastered versions, use text based LUA files
	Util::C_String fileNameString = m_HDDScriptsDir + Util::C_String(fname);

	lua_pushliteral(m_LuaState, "_TRACEBACK");
	lua_rawget(m_LuaState, LUA_GLOBALSINDEX);  /* get traceback function */
	int err_func = lua_gettop (m_LuaState);

	if( !LoadLUAFile(m_LuaState, fileNameString, fname) )
		return 1;

	const S32 ret = lua_pcall(m_LuaState,0,0,err_func);
	if( ret )
	{
		LOGWARN( g_Log,  "Error in Lua file : %s (%d)\n", fileNameString.Chars(),ret);
		LOGWARN( g_Log, "%s\n", lua_tostring(m_LuaState,-1));
		lua_pop( m_LuaState, 1);  // pop error message from the stack
	}
	lua_pop( m_LuaState, 1);  // pop _TRACBACK off

	return ret;
}

const S32 C_UILua::GetValFromTable(const char *table, const char *name)
{
	S32 ret = -1;
	if(!name || !table)
		return ret;

	lua_getglobal(m_LuaState, table);
	if(lua_isnil(m_LuaState,-1))
	{
		LOGWARN( g_Log,  "%s table not found\n", table);
		lua_pop(m_LuaState,1);
		return ret;
	}

	lua_pushstring(m_LuaState, name );
	lua_rawget(m_LuaState, -2);
	if(lua_isnumber(m_LuaState,-1))
		ret = (S32)lua_tonumber(m_LuaState,-1);
	else
	{
//		LOGWARN( g_Log,  "%s not found in table %s\n", name, table);
	}

	lua_pop(m_LuaState,2);

	return ret;
}

// Grab the text string out of the lua table (if it exists)
const S32 C_UILua::GetStringIDSFromName(const char *name)
{
	const S32 ID = GetValFromTable("UIText",name);

	if(ID == -1)
		return 0;

	return ID;
}

// Grab a font by name
const S32 C_UILua::GetFontFromName(const char *name)
{
	const S32 ID = GetValFromTable(LUA_FONT_TABLE_NAME,name);

	if(ID == -1)
		return 0; // Use font 0

	return ID;
}

extern "C" void SetLuaPAKData(char *name)
{
	g_pUILua->SetCurrentPAK(name);
	gLuaPAKFilePosition=0;
}

}; // namespace UI
