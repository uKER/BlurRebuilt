#ifndef LUA_LUAHELPER_H
#define LUA_LUAHELPER_H

#ifdef _MSC_VER
#pragma once
#endif

// extern "C" 
// {
// 	#include <lua.h>
// }
struct lua_State;

namespace Lua
{

void AddDataMember(lua_State *pState, const char *member, S32 value);
void AddDataMember(lua_State *pState, const char *member, U32 value);
// void AddDataMember(lua_State *pState, const char *member, U64 value);
// void AddDataMember(lua_State *pState, const char *member, S64 value);
void AddDataMember(lua_State *pState, const char *member, F32 value);
void AddDataMember(lua_State *pState, const char *member, F64 value);
void AddDataMember(lua_State *pState, const char *member, bool value);
void AddDataMember(lua_State *pState, const char *member, const char *str);
void AddDataMember(lua_State *pState, const char *member, void *data);

const S8	GetDataMember(lua_State *pState, const char *member, S8 &value, const bool safe = false );
const U8	GetDataMember(lua_State *pState, const char *member, U8 &value, const bool safe = false );
const U16	GetDataMember(lua_State *pState, const char *member, U16 &value, const bool safe = false );
const S16	GetDataMember(lua_State *pState, const char *member, S16 &value, const bool safe = false );
const S32	GetDataMember(lua_State *pState, const char *member, S32 &value, const bool safe = false );
const U32	GetDataMember(lua_State *pState, const char *member, U32 &value, const bool safe = false );
// const S64	GetDataMember(lua_State *pState, const char *member, S64 &value, const bool safe = false );
// const U64	GetDataMember(lua_State *pState, const char *member, U64 &value, const bool safe = false );
const F32	GetDataMember(lua_State *pState, const char *member, F32 &value, const bool safe = false );
const F64	GetDataMember(lua_State *pState, const char *member, F64 &value, const bool safe = false );
const bool	GetDataMember(lua_State *pState, const char *member, bool &value, const bool safe = false );
const char*	GetDataMember(lua_State *pState, const char *member, const char *&str, const bool safe = false );
const void*	GetDataMember(lua_State *pState, const char *member, void *&data, const bool safe = false );
// Templated version to call above with temporary
template <typename Type>
const Type GetDataMember(lua_State *pState, const char* member )
{
	Type value = 0;
	return GetDataMember( pState, member, value, true );
}

const int DataMemberType(lua_State *pState, const char *key);

void DumpStack(lua_State *pState);
void *MemoryAllocator (void *ud, void *old_ptr, size_t old_size, size_t new_size);

void* LuaAlloc( const U32 size );
void LuaFree( void* ptr );

// a generic call function
const bool Call( lua_State* pState, const char* pFuncName, const char* pArgFormat = NULL, ... );

void TimedGarbageCollect(lua_State *pState, U32 usec);

} // namespace Lua

#endif // LUA_LUAHELPER_H
