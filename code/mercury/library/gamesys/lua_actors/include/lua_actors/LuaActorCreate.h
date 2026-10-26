#ifndef LUAACTORCREATE_438A85D5_E19D_4179_A97A_C0EFD7341A61_H
#define LUAACTORCREATE_438A85D5_E19D_4179_A97A_C0EFD7341A61_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorCreation.h>
#include <lua_actors/LuaActorDeclare.h>


namespace LuaActor
{

class C_LuaActorCreate : public Actor::C_ActorCreationData
{
public:
	virtual ~C_LuaActorCreate();

	bool PushNamedData(lua_State* pLuaState, const C_LuaActor* pLuaActor, const char* memberName) const;

protected:
	explicit C_LuaActorCreate(const XtType* pLeafClassXtType, void* pLeafClassObject);

private:
	const XtType*	m_pLeafClassXtType; ///< type descriptor for leaf class
	S32				m_LeafClassOffset; ///< offset from 'this' to get to base address of leaf class object

	static const XtField* FindNamedField(const char* fieldName, const XtType* pXtType, const S32 offsetCurrent, S32& offsetFinal);

	template <typename T_DataType>
	static const T_DataType* GetDataType(const XtType* pXtType, const void* pData);

	static bool PushData(lua_State* pLuaState, const C_LuaActor* pLuaActor, const XtField* pXtField, const void* pData);
	static bool PushUserType(lua_State* pLuaState, const C_LuaActor* pLuaActor, const XtType* pXtType, const void* pData);

};

} // namespace LuaActor


#endif // LUAACTORCREATE_438A85D5_E19D_4179_A97A_C0EFD7341A61_H
