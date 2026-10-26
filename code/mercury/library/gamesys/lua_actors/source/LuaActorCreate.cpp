#include "LuaActorsPch.h"

#include <lua_actors/LuaActorCreate.h>

#include <lapi.h>
#include <lua_actors/LuaActor.h>
#include <lua_actors/LuaActorUtils.h>
#include <shared_actors/XtSharedLevelActorParams.h>


namespace LuaActor
{

C_LuaActorCreate::C_LuaActorCreate(const XtType* pLeafClassXtType, void* pLeafClassObject) :
	m_pLeafClassXtType(pLeafClassXtType), 
	m_LeafClassOffset(0)
{
	assert(pLeafClassXtType != NULL);
	assert(pLeafClassObject != NULL);

	m_LeafClassOffset = reinterpret_cast<const U8*>(pLeafClassObject) - reinterpret_cast<const U8*>(this);
}

C_LuaActorCreate::~C_LuaActorCreate()
{
}

const XtField* C_LuaActorCreate::FindNamedField(const char* fieldName, const XtType* pXtType, const S32 offsetCurrent, S32& offsetFinal)
{
	const XtField* pXtField = XtFindFieldByName(pXtType, fieldName);

	if (pXtField != NULL)
	{
		offsetFinal = offsetCurrent;
	}
	else
	{
		for (U32 i = 0; i < pXtType->baseCount; ++i)
		{
			const XtBase& base = pXtType->bases[i];
			const XtField* pBaseXtField = FindNamedField(fieldName, base.type, offsetCurrent + base.offset, offsetFinal);

			if (pBaseXtField != NULL)
			{
				if (pXtField == NULL)
				{
					pXtField = pBaseXtField;
				}
				else
				{
					LOGERROR(GetDebugLogger(), DBSTR "ambiguous field %s in base classes of %s", fieldName, pXtType->name);
					pXtField = NULL;
					break;
				}
			}
		}
	}

	return pXtField;
}

template <typename T_DataType>
const T_DataType* C_LuaActorCreate::GetDataType(const XtType* pXtType, const void* pData)
{
	if (pXtType == XtGetType<T_DataType>())
	{
		return reinterpret_cast<const T_DataType*>(pData);
	}

	for (U32 i = 0; i < pXtType->baseCount; ++i)
	{
		const XtBase& base = pXtType->bases[i];
		const void* pBaseData = reinterpret_cast<const U8*>(pData) + base.offset;

		const T_DataType* pDataType = GetDataType<T_DataType>(base.type, pBaseData);
		if (pDataType != NULL)
		{
			return pDataType;
		}
	}

	return NULL;
}

bool C_LuaActorCreate::PushUserType(lua_State* pLuaState, const C_LuaActor* pLuaActor, const XtType* pXtType, const void* pData)
{
	// component creation data
	{
		const Component::I_ComponentCreator* pComponentCreator = GetDataType<Component::I_ComponentCreator>(pXtType, pData);
		if (pComponentCreator != NULL)
		{
			lua_pushlightuserdata(pLuaState, const_cast<Component::I_ComponentCreator*>(pComponentCreator));
			return true;
		}
	}

	// actor parameter
	{
		const Actors::C_XtParamActor* pActorParam = GetDataType<Actors::C_XtParamActor>(pXtType, pData);
		if (pActorParam != NULL)
		{
			PushActorHandle(pLuaState, pActorParam->GetActorHandle(pLuaActor));
			return true;
		}
	}

	// orientation parameter
	{
		const Actors::C_XtParamOrientation* pOrientParam = GetDataType<Actors::C_XtParamOrientation>(pXtType, pData);
		if (pOrientParam != NULL)
		{
			PushHMat43(pLuaState, pOrientParam->GetOrientation(pLuaActor));
			return true;
		}
	}

	// position parameter
	{
		const Actors::C_XtParamPosition* pPosParam = GetDataType<Actors::C_XtParamPosition>(pXtType, pData);
		if (pPosParam != NULL)
		{
			PushHVec3(pLuaState, pPosParam->GetPosition(pLuaActor));
			return true;
		}
	}

	// vector parameter
	{
		const Actors::C_XtParamVector* pVecParam = GetDataType<Actors::C_XtParamVector>(pXtType, pData);
		if (pVecParam != NULL)
		{
			PushHVec3(pLuaState, pVecParam->GetVector(pLuaActor));
			return true;
		}
	}

	// float parameter
	{
		const Actors::C_XtParamFloat* pFloatParam = GetDataType<Actors::C_XtParamFloat>(pXtType, pData);
		if (pFloatParam != NULL)
		{
			lua_pushnumber(pLuaState, static_cast<lua_Number>(pFloatParam->GetFloat(pLuaActor)));
			return true;
		}
	}

	// integer parameter
	{
		const Actors::C_XtParamInteger* pIntParam = GetDataType<Actors::C_XtParamInteger>(pXtType, pData);
		if (pIntParam != NULL)
		{
			lua_pushnumber(pLuaState, static_cast<lua_Number>(pIntParam->GetInteger(pLuaActor)));
			return true;
		}
	}

	// boolean parameter
	{
		const Actors::C_XtParamBool* pBoolParam = GetDataType<Actors::C_XtParamBool>(pXtType, pData);
		if (pBoolParam != NULL)
		{
			lua_pushboolean	(pLuaState,pBoolParam->GetBool(pLuaActor) ? 1 : 0);
			return true;
		}
	}

	// unknown type
	return false;
}

bool C_LuaActorCreate::PushData(lua_State* pLuaState, const C_LuaActor* pLuaActor, const XtField* pXtField, const void* pData)
{
	const XtType* pXtType = pXtField->type;

	if (pXtField->array != 0)
	{
		LOGERROR(GetDebugLogger(), DBSTR "%s could not be pushed, array types are not supported", pXtField->name);
		return false;
	}

	switch (pXtField->behaviour)
	{
	case kXtFbAtom:
		// handle by atom type
		assert(pXtType->behaviour == kXtTbAtom);
		switch (pXtType->atom)
		{
		case kXtBool:
			{
				const bool* pValue = reinterpret_cast<const bool*>(pData);
				lua_pushboolean(pLuaState, *pValue ? 1 : 0);
			}
			return true;

		case kXtI8:
			{
				const S8* pValue = reinterpret_cast<const S8*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtI16:
			{
				const S16* pValue = reinterpret_cast<const S16*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtI32:
			{
				const S32* pValue = reinterpret_cast<const S32*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtI64:
			{
				const S64* pValue = reinterpret_cast<const S64*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtU8:
			{
				const U8* pValue = reinterpret_cast<const U8*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtU16:
			{
				const U16* pValue = reinterpret_cast<const U16*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtU32:
			{
				const U32* pValue = reinterpret_cast<const U32*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtU64:
			{
				const U64* pValue = reinterpret_cast<const U64*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtF32:
			{
				const F32* pValue = reinterpret_cast<const F32*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtF64:
			{
				const F64* pValue = reinterpret_cast<const F64*>(pData);
				lua_pushnumber(pLuaState, static_cast<lua_Number>(*pValue));
			}
			return true;

		case kXtSz8:
			{
				const XtSz8* pValue = reinterpret_cast<const XtSz8*>(pData);
				lua_pushstring(pLuaState, pValue->string);
			}
			return true;

		case kXtSz16:
		case kXtLocId:
		default:
			break;
		}
		break;

	case kXtFbEnum:
		// find the enum label with this value
		assert(pXtType->behaviour == kXtTbEnum);
		{
			const XtU32 value = *reinterpret_cast<const XtU32*>(pData);

			for (U32 i = 0; i < pXtType->fieldCount; ++i)
			{
				const XtField* pLabel = &pXtType->fields[i];

				if ((pLabel->behaviour == kXtFbImplEnumLabel) && (pLabel->value == value))
				{
					lua_pushstring(pLuaState, pLabel->name);
					return true;
				}
			}
		}
		break;

	case kXtFbFlags:
		// combine all flag labels with matching bit patterns
		assert(pXtType->behaviour == kXtTbFlags);
		{
			const XtU32 value = *reinterpret_cast<const XtU32*>(pData);

			const char* delimiter = " | ";
			Btl::String result;

			for (U32 i = 0; i < pXtType->fieldCount; ++i)
			{
				const XtField* pLabel = &pXtType->fields[i];

				if ((pLabel->behaviour == kXtFbImplFlagLabel) && ((pLabel->value & value) == pLabel->value))
				{
					if (result.Empty())
					{
						result = pLabel->name;
					}
					else
					{
						result += delimiter;
						result += pLabel->name;
					}
				}
			}

			lua_pushlstring(pLuaState, result.Chars(), result.Length());
			return true;
		}
		break;

	case kXtFbStruct:
		// is it a user-defined type we can send to Lua
		if (PushUserType(pLuaState, pLuaActor, pXtType, pData))
		{
			return true;
		}
		break;

	case kXtFbPtr:
		// is it a user-defined type we can send to Lua
		{
			const void* const* ppData = reinterpret_cast<const void* const*>(pData);
			if ((*ppData != NULL) && PushUserType(pLuaState, pLuaActor, pXtType, *ppData))
			{
				return true;
			}
		}
		break;

	case kXtFbHandle:
		// handle is struct of {record, ...} so output the record
		{
			const XtRecord* pXtRecord = reinterpret_cast<const XtRecord*>(pData);
			lua_pushlightuserdata(pLuaState, const_cast<XtRecord*>(pXtRecord));
			return true;
		}
		break;

	case kXtFbLabel:
		// not supported
		break;

	default:
		// unknown
		break;
	}

	LOGERROR(GetDebugLogger(), DBSTR "%s could not be pushed, unsupported field behaviour", pXtField->name);
	return false;
}

bool C_LuaActorCreate::PushNamedData(lua_State* pLuaState, const C_LuaActor* pLuaActor, const char* memberName) const
{
	S32 offset = 0;
	const XtField* pXtField = FindNamedField(memberName, m_pLeafClassXtType, m_LeafClassOffset, offset);

	if (pXtField != NULL)
	{
		offset += pXtField->offset;

		if (PushData(pLuaState, pLuaActor, pXtField, reinterpret_cast<const U8*>(this) + offset))
		{
			return true;
		}
	}

	LOGERROR(GetDebugLogger(), DBSTR "failed to push %s for actor %s", memberName, GetName());
	lua_pushnil(pLuaState);
	return false;
}

} // namespace LuaActor
