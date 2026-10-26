#ifndef LUAACTORCONTEXT_C25628EF_B5EE_4E6F_BBF2_70E91BDB5ABC_H
#define LUAACTORCONTEXT_C25628EF_B5EE_4E6F_BBF2_70E91BDB5ABC_H

#ifdef _MSC_VER
#pragma once
#endif

#include <lua_actors/LuaActorDeclare.h>
#include <misc_shared/context/I_Anything.h>
#include <misc_shared/uid/FourCC.h>


namespace LuaActor
{

class I_LuaActorContext : public Sys::C_Anything<FOURCC_U32('LUAC')>
{
public:
	virtual lua_State* GetLuaState() const = 0;
	virtual const C_LuaActorMessageFactory& GetLuaActorMessageFactory() const = 0;

};

} // namespace LuaActor


#endif // LUAACTORCONTEXT_C25628EF_B5EE_4E6F_BBF2_70E91BDB5ABC_H
