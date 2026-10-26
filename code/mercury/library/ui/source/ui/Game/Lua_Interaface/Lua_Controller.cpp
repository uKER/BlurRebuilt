#include "stdafx.h"

#include "ui/Game/UILua.h"

namespace UI
{

// sets primary to -1 which means no pads are active
int ClearAllActivePads(lua_State* pState)
{
	return 0;
}


// Clears all active pads except the primary.
int ClearAllButPrimary(lua_State* pState)
{
	return 0;
}


// Clears all active pads except the primary.
// expects a port number to be passed..
int SetPrimary(lua_State* pState)
{
	assert(lua_isnumber(pState, 1));
	return 0;
}

// returns the controller port index of the primary controller
int GetPrimary(lua_State* pState)
{
	lua_pushnumber(pState, 0 ); // TODO
	return 1;
}

// Activates a specific port for UI reading.
// expects a port number to be passed..
int Activate(lua_State* pState)
{
	assert(lua_isnumber(pState, 1));
	return 0;
}



// DeActivates a specific port from UI reading.
// expects a port number to be passed..
int DeActivate(lua_State* pState)
{
	assert(lua_isnumber(pState, 1));
	return 0;
}


// Locks The UI to a specific port
// mainly used in multi player where some screens need to be locked to the person who selected the screen.
// expects a port number to be passed..
int LockUIToPort(lua_State* pState)
{
	assert(lua_isnumber(pState, 1));
	return 0;
}



// Unlocks the UI so any active pad can be read.
// mainly used in multi player where some screens need to be locked to the person who selected the screen.
int UnLockUI(lua_State* pState)
{
	return 0;
}



// returns the number of active controllers...
int NumActive(lua_State* pState)
{
	lua_pushnumber(pState, 1); // TODO
	return 1;
}


/******************************************************************************************************************************************************
*******************************************************************************************************************************************************/

static const luaL_reg controller_lib[] = {
	{"Clear",						ClearAllActivePads},
	{"ClearAllButPrimary",			ClearAllButPrimary},
	{"SetPrimary",					SetPrimary},
	{"GetPrimary",					GetPrimary},
	{"ActivatePort",				Activate},
	{"DeActivatePort",				DeActivate},
	{"LockUI",						LockUIToPort},
	{"UnLockUI",					UnLockUI},
	{"NumActive",					NumActive},
	{NULL,							NULL}		// end of list
};



LUALIB_API int luaopen_controller(lua_State *pState)
{
	luaL_openlib(pState, "UIPad", controller_lib, 0);
	return 1;
}

}; // namespace UI
