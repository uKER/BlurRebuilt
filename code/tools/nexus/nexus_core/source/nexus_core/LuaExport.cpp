#include "stdafx.h"

namespace Nexus
{

void ProcessResult(lua_State* pLuaState)
{
	//Return result
	lua_pushnumber(pLuaState, g_pProgressCallback->TaskResult().ErrorCode());
	lua_pushnumber(pLuaState, g_pProgressCallback->TaskResult().Severity());
	lua_pushstring(pLuaState, g_pProgressCallback->TaskResult().ErrorString().c_str());
	if (!g_pProgressCallback->TaskResult())
	{
		g_pCore->SetExecuteCommandError(true);
		g_pCore->PrintError(g_pProgressCallback->TaskResult().ErrorString());
	}
}

int OpenSceneTaskLua(lua_State* pLuaState)
{
	const char* pStr = lua_tostring(pLuaState, 1);
	lua_pop(pLuaState, 1);
	if (g_pCore->ExecuteCommandError()) return 0;

	StringArray arguments;
	arguments.PushBack(pStr);

	if (g_pProgressCallback)
	{
		g_pProgressCallback->ExecuteTask(OpenSceneTask, arguments);
	}

	//Return result
	ProcessResult(pLuaState);
	return 3;
}

int ValidateSceneTaskLua(lua_State* pLuaState)
{
	if (g_pCore->ExecuteCommandError()) return 0;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		g_pProgressCallback->ExecuteTask(ValidateSceneTask, arguments);
	}
	
	//Return result
	ProcessResult(pLuaState);
	return 3;
}

int RepairSceneTaskLua(lua_State* pLuaState)
{
	if (g_pCore->ExecuteCommandError()) return 0;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		g_pProgressCallback->ExecuteTask(RepairSceneTask, arguments);
	}

	//Return result
	ProcessResult(pLuaState);
	return 3;
}

int WeldMeshPolygonVerticesTaskLua(lua_State* pLuaState)
{
	if (g_pCore->ExecuteCommandError()) return 0;

	StringArray arguments;
	if (g_pProgressCallback)
	{
		g_pProgressCallback->ExecuteTask(WeldMeshPolygonVerticesTask, arguments);
	}

	//Return result
	ProcessResult(pLuaState);
	return 3;
}

int SaveSceneTaskLua(lua_State* pLuaState)
{
	const char* pStr = lua_tostring(pLuaState, 1);
	lua_pop(pLuaState, 1);
	if (g_pCore->ExecuteCommandError()) return 0;

	StringArray arguments;
	arguments.PushBack(pStr);
	if (g_pProgressCallback)
	{
		g_pProgressCallback->ExecuteTask(SaveSceneTask, arguments);
	}

	//Return result
	ProcessResult(pLuaState);
	return 3;
}

int PrintSceneBoundsQuickTaskLua(lua_State* pLuaState)
{
	if (g_pCore->ExecuteCommandError()) return 0;

	PrintSceneBoundsQuickTask();
	return 0;
}

int PrintSceneGraphQuickTaskLua(lua_State* pLuaState)
{
	if (g_pCore->ExecuteCommandError()) return 0;

	PrintSceneGraphQuickTask();
	return 0;
}

int PrintQuickTaskLua(lua_State* pLuaState)
{
	const char* pStr = lua_tostring(pLuaState, 1);
	lua_pop(pLuaState, 1);
	if (g_pCore->ExecuteCommandError()) return 0;

	PrintQuickTask(pStr);
	return 0;
}

}