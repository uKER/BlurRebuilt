#include "stdafx.h"

#ifdef NEXUS_MAYA

#include <nexus_core/core.h>

#include <Maya/MGlobal.h>
#include <Maya/MStatus.h>

namespace Nexus
{
	Tu::Status MayaTasks::ExecuteMel(const char* pCommand)
	{
		if (!Nexus::g_pCore->IsMayaInitialised())
		{
			Tu::Status tuStatus = Nexus::g_pCore->InitialiseMaya();
			if (!tuStatus) 
				return tuStatus;
		}

		MStatus mStatus;
		mStatus = MGlobal::executeCommand(pCommand, true, false);
		if (!mStatus)
		{
			return Tu::Status(Tu::Status::FAILURE, mStatus.errorString().asChar());
		}

		return Tu::Status::SUCCESS;
	}

	int MayaTasks::ExecuteMelLua(lua_State* pLuaState)
	{
		const char* pStr = lua_tostring(pLuaState, 1);
		Tu::Status status = ExecuteMel(pStr);
		lua_pop(pLuaState, 1);

		//Return result
		return GlobalTasks::ProcessResult(pLuaState, status);
	}

	void MayaTasks::Register(lua_State* pLuaState)
	{
		//Register Lua functions
		lua_register( pLuaState, "MExecuteMel", &MayaTasks::ExecuteMelLua);
	}
}

#endif