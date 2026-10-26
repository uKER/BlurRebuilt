#ifndef NEXUS_CORE_MAYA_TASKS_H
#define NEXUS_CORE_MAYA_TASKS_H

#ifdef NEXUS_MAYA

namespace Nexus
{
	class MayaTasks
	{
	public:
		static Tu::Status ExecuteMel(const char* pCommand);

	private:
		static void Register(lua_State* pLuaState);

		static int ExecuteMelLua(lua_State* pLuaState);

		friend class Core;
	};
}

#endif

#endif
