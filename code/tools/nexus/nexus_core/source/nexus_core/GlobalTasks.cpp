#include "stdafx.h"
#include <tools_common/FileUtilities.h>
#include <nexus_core/core.h>

namespace Nexus
{
	int GlobalTasks::ProcessResult(lua_State* pLuaState, const Tu::Status& status)
	{
		//Return result
		lua_pushnumber(pLuaState, status.ErrorCode());
		lua_pushnumber(pLuaState, status.Severity());
		lua_pushstring(pLuaState, status.ErrorString().c_str());

		if (!status)
			g_pCore->PrintError(status.ErrorString() + std::string("\n"));
		return 3;
	}

	void GlobalTasks::Print(const std::string& text)
	{
		g_pCore->PrintMessage(text);
	}

	int GlobalTasks::PrintLua(lua_State* pLuaState)
	{
		S32 size = lua_gettop(pLuaState);
		if (size != 1)
			return 0;

		const char* pStr = lua_tostring(pLuaState, -1);
		if (!pStr)
			return 0;
		lua_pop(pLuaState, 1);
		Print(pStr);
		return 0;
	}

	int GlobalTasks::SetStatusLua(lua_State* pLuaState)
	{
		S32 size = lua_gettop(pLuaState);
		if (size != 3)
			return size;

		Tu::Status status;
		status.SetErrorCode(Tu::Status::StatusEnum(atoi(lua_tostring(pLuaState, -1))));
		lua_pop(pLuaState, 1);
		status.SetSeverity(atoi(lua_tostring(pLuaState, -1)));
		lua_pop(pLuaState, 1);
		status.SetErrorString(lua_tostring(pLuaState, -1));
		lua_pop(pLuaState, 1);

		g_pCore->SetStatus(status);

		return ProcessResult(pLuaState, Tu::Status(Tu::Status::FAILURE, "Failed to set status."));
	}

	int GlobalTasks::FileExistsLua(lua_State* pLuaState)
	{
		const char* pStr = lua_tostring(pLuaState, -1);
		if (!pStr)
		{
			lua_pop(pLuaState, 1);
			lua_pushnumber(pLuaState, 0);
		}

		if (Tu::FileExists(pStr))
		{
			lua_pop(pLuaState, 1);
			lua_pushnumber(pLuaState, 1);
		}
		else
		{
			lua_pop(pLuaState, 1);
			lua_pushnumber(pLuaState, 0);
		}
		return 1;
	}

	int GlobalTasks::ExitLua(lua_State* pLuaState)
	{
		int exitCode = lua_tonumber(pLuaState, 1);
		lua_pop(pLuaState, 1);

		throw Core::LuaExitException(exitCode);
		return 0;
	}

	void GlobalTasks::Register(lua_State* pLuaState)
	{
		//Register Lua functions
		lua_register(pLuaState, "Print", &GlobalTasks::PrintLua);
		lua_register(pLuaState, "FileExists", &GlobalTasks::FileExistsLua);
		lua_register(pLuaState, "Exit", &GlobalTasks::ExitLua);

		lua_register(pLuaState, "SetStatus", &GlobalTasks::SetStatusLua);
	}
}