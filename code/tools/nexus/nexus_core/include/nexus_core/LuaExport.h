#ifndef NEXUS_CORE_LUA_EXPORT_H
#define NEXUS_CORE_LUA_EXPORT_H

namespace Nexus
{
	int OpenSceneTaskLua(lua_State* pLuaState);

	int SaveSceneTaskLua(lua_State* pLuaState);

	int ValidateSceneTaskLua(lua_State* pLuaState);

	int RepairSceneTaskLua(lua_State* pLuaState);

	int WeldMeshPolygonVerticesTaskLua(lua_State* pLuaState);

	int PrintSceneBoundsQuickTaskLua(lua_State* pLuaState);

	int PrintSceneGraphQuickTaskLua(lua_State* pLuaState);

	int PrintQuickTaskLua(lua_State* pLuaState);
}

#endif
