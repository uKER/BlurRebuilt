#ifndef LUA_VARIABLE_HELPER_H
#define LUA_VARIABLE_HELPER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Game
{

class LuaVariableHelper
{
public:
	void CreateLuaTables();

	static LuaVariableHelper& Instance() {return s_instance;}

private:
	static LuaVariableHelper s_instance;
};

} // namespace Game

#endif // LUA_VARIABLE_HELPER_H
