#include "LuaSharedPch.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", off )
#endif

#include <lua_shared/LuaHelper.h>

extern "C"
{
#include <lua.h>
#include <lstate.h>
#include <lgc.h>
#include <lauxlib.h>
}

#include <core/Core.h>
#include <system/DebugLog.h>
#include <system/memory/MemoryTracker.h>

#include "system/time/Time.h"

namespace Lua
{

static Debug::Logger& g_Log = Debug::GetLogger("mercury.lua");

void AddDataMember(lua_State *pState, const char *member, S32 value)
{
	lua_pushstring(pState, member); 
	lua_pushinteger(pState, value); 
	lua_rawset(pState,-3);
}

void AddDataMember(lua_State *pState, const char *member, U32 value)
{
	lua_pushstring(pState, member); 
	lua_pushinteger(pState, value); 
	lua_rawset(pState,-3);
}
// Removed because of possible data loss
/*
void AddDataMember(lua_State *pState, const char *member, U64 value)
{
	lua_pushstring(pState, member); 
	lua_pushinteger(pState, value); 
	lua_rawset(pState,-3);
}

void AddDataMember(lua_State *pState, const char *member, S64 value)
{
	lua_pushstring(pState, member); 
	lua_pushinteger(pState, value); 
	lua_rawset(pState,-3);
}
*/

void AddDataMember(lua_State *pState, const char *member, F32 value)
{
	lua_pushstring(pState, member); 
	lua_pushnumber(pState, value); 
	lua_rawset(pState,-3);
}

void AddDataMember(lua_State *pState, const char *member, F64 value)
{
	lua_pushstring(pState, member); 
	lua_pushnumber(pState, value); 
	lua_rawset(pState,-3);
}

void AddDataMember(lua_State *pState, const char *member, bool value)
{
	lua_pushstring(pState, member); 
	lua_pushboolean(pState, value); 
	lua_rawset(pState,-3);
}

void AddDataMember(lua_State *pState, const char *member, const char *str)
{
	lua_pushstring(pState, member); 
	lua_pushstring(pState, str); 
	lua_rawset(pState,-3);
}

void AddDataMember(lua_State *pState, const char *member, void *data)
{
	lua_pushstring(pState, member); 
	lua_pushlightuserdata(pState, data); 
	lua_rawset(pState,-3);
}

const S8 GetDataMember(lua_State *pState, const char *member, S8 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (S8)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}

const U8 GetDataMember(lua_State *pState, const char *member, U8 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (U8)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}

const U16 GetDataMember(lua_State *pState, const char *member, U16 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (U16)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}

const S16 GetDataMember(lua_State *pState, const char *member, S16 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (S16)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}

const S32 GetDataMember(lua_State *pState, const char *member, S32 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (S32)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}

const U32 GetDataMember(lua_State *pState, const char *member, U32 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (U32)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}
/*
const S64 GetDataMember(lua_State *pState, const char *member, S64 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (S64)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}

const U64 GetDataMember(lua_State *pState, const char *member, U64 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (U64)lua_tointeger(pState,-1);
	lua_pop(pState,1);

	return value;
}
*/

const F32 GetDataMember(lua_State *pState, const char *member, F32 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (F32)lua_tonumber(pState,-1);
	lua_pop(pState,1);

	return value;
}

const F64 GetDataMember(lua_State *pState, const char *member, F64 &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TNUMBER )
		value = (F64)lua_tonumber(pState,-1);
	lua_pop(pState,1);

	return value;
}

const bool GetDataMember(lua_State *pState, const char *member, bool &value, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TBOOLEAN )
		value = lua_toboolean(pState,-1)!=0;
	lua_pop(pState,1);

	return value;
}

const char* GetDataMember(lua_State *pState, const char *member, const char *&str, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || lua_type(pState, -1) == LUA_TSTRING )
		str = lua_tostring(pState,-1);
	lua_pop(pState,1);

	return str;
}

const void* GetDataMember(lua_State *pState, const char *member, void *&data, const bool safe )
{
	lua_pushstring(pState, member);
	lua_rawget(pState,-2);
	if( !safe || (lua_type(pState, -1) == LUA_TLIGHTUSERDATA || lua_type(pState, -1) == LUA_TUSERDATA) )
		data = lua_touserdata(pState,-1);
	lua_pop(pState,1);

	return data;
}

const int DataMemberType(lua_State *pState, const char *key)
{
	int result;
	lua_pushstring(pState, key);
	lua_rawget(pState, -2);  /* get background[key] */

	result=lua_type(pState, -1);
	lua_pop(pState, 1);  /* remove number */

	return result;
}

// To be defined by the game if DumpStack() is used
extern void OutputString(const char *str);

// output a string from LUA - used for syntax errors etc.
void DumpStack(lua_State *pState)
{
	int top = lua_gettop(pState);
	
	LOGERROR( g_Log, "Lua stack dump\n");

	while(top)
	{
		LOGERROR( g_Log, "%d - ",top);
		
		switch(lua_type(pState, top))
		{
		case LUA_TNUMBER:
			LOGERROR( g_Log, "number %f\n",lua_tonumber(pState,top));
			OutputString("number\n");
			break;
		case LUA_TTABLE:
			LOGERROR( g_Log, "table %x\n",lua_topointer(pState,top));
			OutputString("table\n");
			break;
		case LUA_TSTRING:
			LOGERROR( g_Log, "string \"%s\"\n",lua_tostring(pState,top));
			OutputString("string\n");
			break;
		case LUA_TBOOLEAN:
			LOGERROR( g_Log, "boolean %d\n",lua_toboolean(pState,top));
			OutputString("boolean\n");
			break;

		case LUA_TNIL:
			LOGERROR( g_Log, "nil\n");
			OutputString("nil\n");
			break;

		case LUA_TLIGHTUSERDATA:
			LOGERROR( g_Log, "light user data\n");
			OutputString("light user data\n");
			break;

		case LUA_TFUNCTION:
			LOGERROR( g_Log, "function %x\n",lua_topointer(pState,top));
			OutputString("function\n");
			break;

		case LUA_TUSERDATA:
			LOGERROR( g_Log, "user data\n");
			OutputString("user data\n");
			break;

		case LUA_TTHREAD:
			LOGERROR( g_Log, "thread\n");
			OutputString("thread\n");
			break;

		default:
			LOGERROR( g_Log, "unknown\n");
			OutputString("unknown\n");
			break;
		}

		top--;
	}
}

void *MemoryAllocator (void *ud, void *old_ptr, size_t old_size, size_t new_size) 
{
	MEMORY_TRACKER_PUSH_BY_ID((U32)ud);

	if (new_size == 0) // free memory
	{
		if(old_ptr)
			LuaFree( old_ptr );
		MEMORY_TRACKER_POP_BY_ID((U32)ud);
		return NULL;
	}
	else
	{
		// new size is less than old size, return old pointer
		if(new_size < old_size)
		{
			MEMORY_TRACKER_POP_BY_ID((U32)ud);
			return old_ptr;
		}

		/* ANSI requires that realloc(NULL, size) == malloc(size) */
		if(old_ptr == NULL)
		{
			void *mem = LuaAlloc(new_size);
			if(mem)
			{
				memset(mem,0,new_size);	// TODO: Remove
			}
			MEMORY_TRACKER_POP_BY_ID((U32)ud);
			return mem;
		}
		else	// re-allocate to a bigger sized block 
		{
			void *new_ptr = LuaAlloc(new_size);
			if(new_ptr)
			{
				memset(new_ptr,0,new_size);	// TODO: Remove
				memcpy(new_ptr,old_ptr,old_size);
				LuaFree(old_ptr);
			}
			MEMORY_TRACKER_POP_BY_ID((U32)ud);
			return new_ptr;
		}

		MEMORY_TRACKER_POP_BY_ID((U32)ud);
		return old_ptr;	
	}

	MEMORY_TRACKER_POP_BY_ID((U32)ud);
	return NULL;
}

const bool Call( lua_State* pState, const char* pFuncName, const char* pArgFormat, ... )
{
	if ( !pState || !pFuncName || !pFuncName[ 0 ] )
	{
		LOGERROR( g_Log, "Lua::Call() - invalid arguements" );
		return false;
	}

	S32 sNumArgs = 0;
	S32 sNumResults = 0;

	va_list v;
	va_start( v, pArgFormat );

	// set function
	lua_getglobal( pState, pFuncName );

	if ( lua_type( pState, -1 ) == LUA_TFUNCTION ) // type check
	{
		bool bArgsDone = false;

		// set args
		while( pArgFormat && *pArgFormat && !bArgsDone )
		{
			switch ( *pArgFormat )
			{
				case 'i':
					lua_pushnumber( pState, va_arg( v, S32 ) );
					break;
				case 'u':
					lua_pushnumber( pState, va_arg( v, U32 ) );
					break;
				case 'f':
					lua_pushnumber( pState, va_arg( v, F64 ) ); // F32 is promoted to F64 in va_arg
					break;
				case 'd':
					lua_pushnumber( pState, va_arg( v, F64 ) );
					break;
				case 'b':
					lua_pushboolean( pState, va_arg( v, S32 ) ); // bool is promoted to S32 in va_arg
					break;
				case 's':
					lua_pushstring( pState, va_arg( v, const char* ) );
					break;
				case '>':
					bArgsDone = true;
					break;
				default:
					LOGERROR( g_Log, "Lua::Call() - invalid arg type : %c", *pArgFormat );
					break;
			}

			if ( *pArgFormat != '>' )
				sNumArgs++;

			pArgFormat++;
		}

		// number of results is the remaining string
		sNumResults = pArgFormat ? strlen( pArgFormat ) : 0;

		// call the function
		if ( lua_pcall( pState, sNumArgs, sNumResults, 0 ) )
		{
			LOGERROR( g_Log, "Lua::Call() - Lua error : %s", lua_tostring( pState, -1 ) );
			lua_pop( pState, 1 );
			return false;
		}

		// grab the results
		S32 sStackPos = -sNumResults;	// stack index of first result

		while( pArgFormat && *pArgFormat )
		{
			switch ( *pArgFormat )
			{
				case 'i':
					if ( lua_type( pState, sStackPos ) == LUA_TNUMBER )
						*va_arg( v, S32* ) = (S32)lua_tonumber( pState, sStackPos );
					else
						LOGERROR( g_Log, "Lua::Call() - incorrect result type" );
					break;
				case 'u':
					if ( lua_type( pState, sStackPos ) == LUA_TNUMBER )
						*va_arg( v, U32* ) = (U32)lua_tonumber( pState, sStackPos );
					else
						LOGERROR( g_Log, "Lua::Call() - incorrect result type" );
					break;
				case 'f':
					if ( lua_type( pState, sStackPos ) == LUA_TNUMBER )
						*va_arg( v, F32* ) = (F32)lua_tonumber( pState, sStackPos );
					else
						LOGERROR( g_Log, "Lua::Call() - incorrect result type" );
					break;
				case 'd':
					if ( lua_type( pState, sStackPos ) == LUA_TNUMBER )
						*va_arg( v, F64* ) = (F64)lua_tonumber( pState, sStackPos );
					else
						LOGERROR( g_Log, "Lua::Call() - incorrect result type" );
					break;
				case 'b':
					if ( lua_type( pState, sStackPos ) == LUA_TBOOLEAN )
						*va_arg( v, bool* ) = ( lua_toboolean( pState, sStackPos ) != 0 );
					else
						LOGERROR( g_Log, "Lua::Call() - incorrect result type" );
					break;
				case 's':
					if ( lua_type( pState, sStackPos ) == LUA_TSTRING )
						*va_arg( v, const char** ) = lua_tostring( pState, sStackPos );
					else
						LOGERROR( g_Log, "Lua::Call() - incorrect result type" );
					break;
				default:
					LOGERROR( g_Log, "Lua::Call() - invalid result type : %c", *pArgFormat );
					break;
			}

			pArgFormat++;
			sStackPos++;	// move up the stack
		}

		// pop results
		lua_pop( pState, sNumResults );

		// done
		va_end( v );
	}
	else
	{
		//LOGERROR( g_Log, "Lua::Call() - function doesn't exist : %s", pFuncName );
		lua_pop( pState, 1 );
		return false;
	}

	return true;
}

void TimedGarbageCollect(lua_State* pState, U32 usec)
{
	Time::C_Time start = Time::GetPerformanceCounter();

	for ( ;; ) {
		luaC_step(pState);

		if (pState->l_G->gcstate == GCSpause)
			break;

		if ( usec )
		{
			Time::C_Time total = Time::GetPerformanceCounter() - start;
			if ( total >= usec )
				break;
		}
	}
}

#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize( "", on )
#endif

} // namespace Lua
