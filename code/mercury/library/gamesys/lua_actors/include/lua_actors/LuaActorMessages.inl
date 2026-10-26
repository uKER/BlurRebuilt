#include <actor_system/ActorDeclare.h>
#include <lua_actors/LuaActorUtils.h>
#include <misc_shared/messaging/MessageValue.h>
#include <xt/XtGame.h>


namespace LuaActorMessages_Impl
{

// default type info
template <typename T_DataType>
const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const T_DataType*)
{
	LOGERROR(LuaActor::GetDebugLogger(), DBSTR "unsupported data type for LuaActor message data");
	return NULL;
};

// boolean type info
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const bool*);

// numeric type info
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const U8*);
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const U16*);
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const U32*);
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const S8*);
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const S16*);
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const S32*);
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const F32*);

// string type info
class C_LuaActorMessageStringInfo : public LuaActor::I_LuaActorMessageDataTypeInfo
{
public:
	C_LuaActorMessageStringInfo(const U32 bufferSize) : m_BufferSize(bufferSize) {}

	virtual U32 GetSize() const {return sizeof(char) * m_BufferSize;}

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const;
	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const;

private:
	U32 m_BufferSize;

};

template <U32 t_BufferSize>
const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const char (*)[t_BufferSize])
{
	static C_LuaActorMessageStringInfo typeInfo(t_BufferSize);
	return &typeInfo;
}

// vector type info
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const Maths::HVec3*);

// matrix type info
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const Maths::HMat43*);

// actor handle type info
extern const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const Actor::C_ActorHandle*);

// xt handle type info
class C_LuaActorMessageXtHandleInfoBase : public LuaActor::I_LuaActorMessageDataTypeInfo
{
protected:
	explicit C_LuaActorMessageXtHandleInfoBase(const XtType* pXtType) : m_pXtType(pXtType) {}

	void PushRecord(lua_State* pLuaState, XtRecord* pRecord) const;
	XtRecord* ToRecord(lua_State* pLuaState, const int index) const;

private:
	const XtType* m_pXtType;

};

template <typename T_RecordType>
class C_LuaActorMessageXtHandleInfo : public C_LuaActorMessageXtHandleInfoBase
{
public:
	typedef XtHandle<T_RecordType> C_XtHandle;

	C_LuaActorMessageXtHandleInfo() : C_LuaActorMessageXtHandleInfoBase(XtGetType<T_RecordType>()) {}

	virtual U32 GetSize() const {return sizeof(C_XtHandle);}

	virtual void PushValue(lua_State* pLuaState, const void* pMessageData) const
	{
		const C_XtHandle* pValue = reinterpret_cast<const C_XtHandle*>(pMessageData);
		PushRecord(pLuaState, pValue->record);
	}

	virtual void ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
	{
		C_XtHandle* pValue = reinterpret_cast<C_XtHandle*>(pMessageData);
		*pValue = C_XtHandle(ToRecord(pLuaState, index));
	}

};

template <typename T_RecordType>
const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const XtHandle<T_RecordType>*)
{
	static C_LuaActorMessageXtHandleInfo<T_RecordType> typeInfo;
	return &typeInfo;
}

// message value type info
class C_LuaActorMessageValueInfoBase : public LuaActor::I_LuaActorMessageDataTypeInfo
{
protected:
	explicit C_LuaActorMessageValueInfoBase(const LuaActor::I_LuaActorMessageDataTypeInfo* pValueTypeInfo) : m_pValueTypeInfo(pValueTypeInfo) {}

	void PushValueType(lua_State* pLuaState, const void* pValueData) const;
	void ToValueType(lua_State* pLuaState, const int index, void* pValueData) const;

private:
	const LuaActor::I_LuaActorMessageDataTypeInfo* m_pValueTypeInfo;

};

template <typename T_DataType>
class C_LuaActorMessageValueInfo : public C_LuaActorMessageValueInfoBase
{
	typedef Messaging::C_MessageValueStorage<T_DataType> C_Storage;

	C_LuaActorMessageValueInfo() : C_LuaActorMessageValueInfoBase(GetTypeInfo(reinterpret_cast<const T_DataType*>(NULL))) {}

	virtual U32 GetSize() const {return sizeof(C_Storage);}

	virtual bool PushValue(lua_State* pLuaState, const void* pMessageData) const
	{
		const C_Storage* pStorage = reinterpret_cast<const C_Storage*>(pMessageData);
		const T_DataType value = pStorage->Get();
		PushValueType(pLuaState, &value);
	}

	virtual bool ToValue(lua_State* pLuaState, const int index, void* pMessageData) const
	{
		C_Storage* pStorage = reinterpret_cast<C_Storage*>(pMessageData);
		T_DataType value;
		ToValueType(pLuaState, index, &value);
		pStorage->Set(value);
	}

};

template <typename T_DataType>
const LuaActor::I_LuaActorMessageDataTypeInfo* GetTypeInfo(const Messaging::C_MessageValueStorage<T_DataType>*)
{
	static C_LuaActorMessageValueInfo<T_DataType> typeInfo;
	return &typeInfo;
}

} // namespace LuaActorMessage_Impl


namespace LuaActor
{

template <class T_DataType>
inline const I_LuaActorMessageDataTypeInfo* C_LuaActorMessageReflection::MakeTypeInfo(const T_DataType& data)
{
	return LuaActorMessage_Impl::GetTypeInfo(&data);
}

template <class T_MessageType>
void C_LuaActorMessageFactory::ReflectMessage(const char* messageName)
{
	const Messaging::TMsgType messageType = T_MessageType::StaticType();

#if LUA_ACTORS_CHECKS
	if (!IsMessageReflected(messageType, messageName))
#endif
	{
		C_LuaActorMessageReflection* pReflection = new C_LuaActorMessageReflection(messageType, messageName, sizeof(T_MessageType));
		T_MessageType::ReflectMembers(*pReflection);
		ReflectMessage(pReflection);
	}
}

} // namespace LuaActor
