#ifndef NEXUS_CORE_GLOBAL_TASKS_H
#define NEXUS_CORE_GLOBAL_TASKS_H

namespace Nexus
{
	class GlobalTasks
	{
	public:
		static void Print(const std::string& text);

		static int ProcessResult(lua_State* pLuaState, const Tu::Status& status);

	private:
		static int SetStatusLua(lua_State* pLuaState);

		static int FileExistsLua(lua_State* pLuaState);

		static int PrintLua(lua_State* pLuaState);

		static int ExitLua(lua_State* pLuaState);

		static void Register(lua_State* pLuaState);

		friend class Core;
	};
}

#endif
