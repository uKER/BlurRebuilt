#include "stdafx.h"

#include "ui/Game/UILua.h"
#include "ui/Game/UIScreenControl.h"

namespace UI
{

// prints a string from the scripts
int PrintMe(lua_State* pState)
{
	if  (lua_isstring(pState, 1))
		LOGDEBUG( g_Log, "LUA: %s",  lua_tostring(pState, 1));
	else
		assert(false);
	return 0;
}

int IncrementMemoryTag(lua_State* pState)
{
	if (lua_isstring(pState, 1))
	{
		const char* type = lua_tostring(pState, 1);

		if (!strcmp(type, "ui"))
			Memory::TrackNodes::Instance().IncrementTag(Memory::TrackNodes::TagFrontend);
		else if (!strcmp(type, "game"))
			Memory::TrackNodes::Instance().IncrementTag(Memory::TrackNodes::TagGame);
		else
			BIZ_ASSERT_FAIL("IncrementMemoryTag: Unknown type (%s)", type);
	}
	else
	{
		Memory::TrackNodes::Instance().IncrementTag();
	}

	return 0;
}


int BeginNamedEvent(lua_State* pState)
{
	if (lua_isstring(pState, 1))
	{
		const char* name = lua_tostring(pState, 1);

		Perf::BeginNamedEvent(name);


	}
	return 0;
}

int EndNamedEvent(lua_State* pState)
{
	Perf::EndNamedEvent();


	return 0;
}


static const luaL_reg debug_lib[] = {
	{"Print",					PrintMe},
	{"IncrementMemoryTag",		IncrementMemoryTag},

	{"BeginNamedEvent",			BeginNamedEvent},
	{"EndNamedEvent",			EndNamedEvent},

	{NULL,			NULL}		// end of list
};


LUALIB_API int luaopen_debug2(lua_State *pState)
{
	luaL_openlib(pState,"Debug",debug_lib,0);
	return 1;
}

}; // namespace UI
