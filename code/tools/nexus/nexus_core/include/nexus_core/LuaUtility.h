#ifndef NEXUS_CORE_LUA_UTILITY_H
#define NEXUS_CORE_LUA_UTILITY_H

namespace Nexus
{
	class LuaUtility
	{
	public:
		inline static void GetArguments(StringArray& arguments, lua_State* pLuaState)
		{
			S32 size = lua_gettop(pLuaState);
			arguments.Resize(size);
			U32 insertionPoint = size-1;
			//We need to reverse the lua stacks argument order.
			for (S32 i = 0; i < size; i++)
			{
				const char* pStr = lua_tostring(pLuaState, -1);
				arguments[insertionPoint] = pStr;
				lua_pop(pLuaState, 1);
				insertionPoint--;
			}
		}
	};
}

#endif