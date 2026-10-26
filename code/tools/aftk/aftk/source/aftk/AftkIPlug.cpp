#include <stdafx.h>

using namespace Aftk;

//#pragma optimize("", off)

I_Plug::I_Plug(void(*pOnChangedCallback)(void* pThis))
{
	m_pSourceConnection = 0;
	m_pParent = 0;
	m_logicalIndex = -1;
	m_pOnChangedCallback = pOnChangedCallback;
}

I_Plug::~I_Plug()
{
	ClearThis();
}

const I_Plug& I_Plug::operator =(const I_Plug& rhs)
{
	if (m_pSourceConnection == rhs.m_pSourceConnection)
		return *this;
	else if (m_pSourceConnection)
	{
		m_pSourceConnection->DeleteConnectedPlugPtr(this);
	}

	m_pSourceConnection = rhs.m_pSourceConnection;
	m_pSourceConnection->AddConnectedPlugPtr(this);

	return *this;
}

bool I_Plug::operator==(const I_Plug &rhs) const
{
	return IsEqualToAll(rhs);
}

bool I_Plug::operator!= (const I_Plug &rhs) const
{
	return !((*this) == rhs);
}

bool I_Plug::operator< (const I_Plug &rhs) const
{
	return true;
}

I_Object* I_Plug::GetConnectionRoot() const
{
	if (!m_pSourceConnection)
		return m_pParent;

	if (m_pSourceConnection && m_pSourceConnection->HasType(E_ObjType_IPlug))
		//Recurse...
		return ((I_Plug*)m_pSourceConnection)->GetConnectionRoot();
	
	return m_pSourceConnection;
}
	
void I_Plug::SetSourceConnectionPtr(I_Object* pSourceConnection)
{
	//Throw an assert as this is quite an unusual situation. 
	//Are you sure this is what you want to be doing? - having a connection to our parent node.
//	AFTK_ASSERT(pSourceConnection != m_pParent);	// [Jan] commented this out because it's not harmful according to Ken

	if (m_pSourceConnection)
		m_pSourceConnection->DeleteConnectedPlugPtr(this);

	m_pSourceConnection = pSourceConnection;
	if (m_pSourceConnection)
		m_pSourceConnection->AddConnectedPlugPtr(this);

	OnChanged();
}


const I_Object* I_Plug::GetSourceConnectionPtr() const
{
	return m_pSourceConnection;
}

I_Object* I_Plug::GetSourceConnectionPtr()
{
	return m_pSourceConnection;
}

S32 I_Plug::GetLogicalIndex() const
{
	return m_logicalIndex;
}

E_GenericAttributeType I_Plug::GetCompoundType() const
{
	U32 nChildren = GetNumChildren();
	if ((nChildren < 2) || (nChildren > 4))
		return Aftk::E_GenericAttributeType_Unknown;

	E_GenericAttributeType t = GetChildPtr(0)->GetDataType();

	for (U32 i = 1; i < nChildren; i++)
	{
		if (GetChildPtr(i)->GetDataType() != t)
			return E_GenericAttributeType_Unknown;
	}
	if (GetNumChildren() == 4)
	{
		switch(t)
		{
		case E_GenericAttributeType_U8:
			return E_GenericAttributeType_Vector4U8;
		case E_GenericAttributeType_U16:
			return E_GenericAttributeType_Vector4U16;
		case E_GenericAttributeType_U32:
			return E_GenericAttributeType_Vector4U32;
		case E_GenericAttributeType_U64:
			return E_GenericAttributeType_Vector4U64;
		case E_GenericAttributeType_S8:
			return E_GenericAttributeType_Vector4S8;
		case E_GenericAttributeType_S16:
			return E_GenericAttributeType_Vector4S16;
		case E_GenericAttributeType_S32:
			return E_GenericAttributeType_Vector4S32;
		case E_GenericAttributeType_S64:
			return E_GenericAttributeType_Vector4S64;
		case E_GenericAttributeType_F32:
			return E_GenericAttributeType_Vector4F32;
		case E_GenericAttributeType_F64:
			return E_GenericAttributeType_Vector4F64;
		default:
			break;
		}
	}
	else if (GetNumChildren() == 3)
	{
		switch(t)
		{
		case E_GenericAttributeType_U8:
			return E_GenericAttributeType_Vector3U8;
		case E_GenericAttributeType_U16:
			return E_GenericAttributeType_Vector3U16;
		case E_GenericAttributeType_U32:
			return E_GenericAttributeType_Vector3U32;
		case E_GenericAttributeType_U64:
			return E_GenericAttributeType_Vector3U64;
		case E_GenericAttributeType_S8:
			return E_GenericAttributeType_Vector3S8;
		case E_GenericAttributeType_S16:
			return E_GenericAttributeType_Vector3S16;
		case E_GenericAttributeType_S32:
			return E_GenericAttributeType_Vector3S32;
		case E_GenericAttributeType_S64:
			return E_GenericAttributeType_Vector3S64;
		case E_GenericAttributeType_F32:
			return E_GenericAttributeType_Vector3F32;
		case E_GenericAttributeType_F64:
			return E_GenericAttributeType_Vector3F64;
		default:
			break;
		}
	}
	else
	{
		switch(t)
		{
		case E_GenericAttributeType_U8:
			return E_GenericAttributeType_Vector2U8;
		case E_GenericAttributeType_U16:
			return E_GenericAttributeType_Vector2U16;
		case E_GenericAttributeType_U32:
			return E_GenericAttributeType_Vector2U32;
		case E_GenericAttributeType_U64:
			return E_GenericAttributeType_Vector2U64;
		case E_GenericAttributeType_S8:
			return E_GenericAttributeType_Vector2S8;
		case E_GenericAttributeType_S16:
			return E_GenericAttributeType_Vector2S16;
		case E_GenericAttributeType_S32:
			return E_GenericAttributeType_Vector2S32;
		case E_GenericAttributeType_S64:
			return E_GenericAttributeType_Vector2S64;
		case E_GenericAttributeType_F32:
			return E_GenericAttributeType_Vector2F32;
		case E_GenericAttributeType_F64:
			return E_GenericAttributeType_Vector2F64;
		default:
			break;
		}
	}

	return Aftk::E_GenericAttributeType_Unknown;
}

bool I_Plug::IsConnected() const
{
	if (m_pSourceConnection)
		return true;

	return false;
}

bool I_Plug::IsPlugChild() const
{
	if (!m_pParent)
		return false;

	if (m_pParent->HasType(E_ObjType_IPlug))
	{
		I_Plug* pPlug = (I_Plug*)m_pParent;
		for (U32 i = 0; i < pPlug->GetNumChildren(); i++)
		{
			if (pPlug->GetChildPtr(i) == this)
				return true;
		}
	}

	return false;
}

bool I_Plug::IsCompound() const
{
	if (GetNumChildren() == 0)
		return false;

	return true;
}

void I_Plug::GetLocationIndexList(C_PlugLogicalIndexList& locationIndexList, const I_Object* pObject)
{
	if (!pObject)
		return;

	if ((pObject->HasType(E_ObjType_Shader)))
	{
		locationIndexList.indices[locationIndexList.nIndices] = 
			((C_ShaderNode*)pObject)->GetNodeIndex();
		locationIndexList.nIndices++;
	}
	else if (pObject->GetType() == E_ObjType_IPlug)
	{
		locationIndexList.indices[locationIndexList.nIndices] = 
			((I_Plug*)pObject)->GetLogicalIndex();
		locationIndexList.nIndices++;

		//Recurse...
		GetLocationIndexList(locationIndexList, ((I_Plug*)pObject)->GetParentPtr());
	}
}

std::string I_Plug::GetPath() const
{
	std::string finalString;
	if (m_pParent)
	{
		finalString = m_pParent->GetPath();
		finalString += ".";
	}
	finalString += GetName();
	return finalString;
}

I_Object* I_Plug::GetParentPtr()
{
	return m_pParent;
}

const I_Object* I_Plug::GetParentPtr() const
{
	return m_pParent;
}

bool I_Plug::HasType(E_ObjType type) const
{
	if (I_Object::HasType(type))
		return true;
	if (type == E_ObjType_IPlug) 
		return true;

	return false;
}

void I_Plug::Output(const I_Plug& plug)
{
	std::string parentName;
	{
		if (plug.IsPlugChild())
		{
			AFTK_DO_BEGIN << "    " AFTK_DO_INFO_END;
			const I_Object* pParentObject = plug.GetParentPtr();

			if (pParentObject->HasType(E_ObjType_IPlug))
			{
				const I_Plug* pParentPlug = (const I_Plug*)pParentObject;
				parentName = pParentPlug->GetName();
			}
			else if (pParentObject->HasType(E_ObjType_Node))
			{
				const C_Node* pParentNode = (const C_Node*)pParentObject;
				parentName = pParentNode->GetName();
			}
		}
	}

	AFTK_DO_BEGIN << plug.GetName() AFTK_DO_INFO_END;
	if (parentName.length() > 0)
	{
		AFTK_DO_BEGIN << " parent: " << parentName AFTK_DO_INFO_END;
	}

	if (plug.IsCompound())
	{
		AFTK_DO_BEGIN << " [COMPOUND] " AFTK_DO_INFO_END;

		E_GenericAttributeType compType = plug.GetCompoundType();
		AFTK_DO_BEGIN << " Compound type : " << compType << "\n" AFTK_DO_INFO_END;

		for (U32 i = 0; i < plug.GetNumChildren(); i++)
		{
			const I_Plug* pChild = plug.GetChildPtr(i);
			AFTK_ASSERT(pChild);
			Output(*pChild);
		}
	}
    
	if (plug.IsConnected())
	{
		const I_Object* pSourceConnection = plug.GetSourceConnectionPtr();

		//If we're connected to the output state of a shader node...
		if (pSourceConnection->HasType(E_ObjType_Node))
		{
			const C_Node* pSourceConnectionNode = (const C_Node*)pSourceConnection;
			AFTK_DO_BEGIN << " -> " << pSourceConnectionNode->GetPath() << " [***]output state[***]" << "\n" AFTK_DO_INFO_END;
		}
		//Else if we're connected to a plug...
		else if (pSourceConnection->HasType(E_ObjType_IPlug))
		{
			AFTK_DO_BEGIN << " -> ***plug connection*** " << pSourceConnection->GetPath() << "\n" AFTK_DO_INFO_END;
		}
		else
		{
			AFTK_DO_BEGIN << "Unknown source connection type.\n" AFTK_DO_INFO_END;
		}
	}
	else
	{
		// Not connected to anything
		AFTK_DO_BEGIN << " -> value " AFTK_DO_INFO_END;
		switch (plug.GetDataType())
		{
			case E_GenericAttributeType_F64:
			{
				F64 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << (F32)val << "\n" AFTK_DO_INFO_END;
				break;
			}
			case E_GenericAttributeType_F32:
			{
				F32 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}
			case E_GenericAttributeType_U8:
			{
				U8 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}
			case E_GenericAttributeType_U16:
			{
				U16 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}
			case E_GenericAttributeType_U32:
			{
				U32 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}
			case E_GenericAttributeType_S8:
			{
				S8 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}
			case E_GenericAttributeType_S16:
			{
				S16 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}
			case E_GenericAttributeType_S32:
			{
				S32 val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}

			case E_GenericAttributeType_Boolean: 
			{
				std::string val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}

			case E_GenericAttributeType_String: 
			{
				std::string val;
				plug.GetValue(val);
				AFTK_DO_BEGIN << val << "\n" AFTK_DO_INFO_END;
				break;
			}

			case E_GenericAttributeType_Null:
			{
				AFTK_DO_BEGIN << "NULL\n" AFTK_DO_INFO_END;
				break;
			}

			default:
				AFTK_DO_BEGIN << "UNKNOWN" << "\n" AFTK_DO_INFO_END;
				break;
		};
	}
}


bool I_Plug::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool I_Plug::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToThis(rhs, flags);
}

bool I_Plug::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(E_ObjType_IPlug))
		return false;

	I_Plug& rhs = (I_Plug&) _rhs;

	if (m_pSourceConnection && rhs.m_pSourceConnection)
	{
		if (m_pSourceConnection->GetType() != rhs.m_pSourceConnection->GetType())
			return false;

		//What we're doing here is finding the nodes to which the source connections of this 
		//plug and the rhs plug are parented to. These nodes must be equivilant.
		I_Object* lhsRootObj = GetConnectionRoot();
		I_Object* rhsRootObj = rhs.GetConnectionRoot();

		if ((lhsRootObj) && (!rhsRootObj))
			return false;
		if ((!lhsRootObj) && (rhsRootObj))
			return false;

		if (lhsRootObj->GetType() != rhsRootObj->GetType())
			return false;

		//Special case: If we're connected to a material or shader node then we're actually testing
		//for the equivilance of our two plugs. So instead of looking for their sourceConnection pointers
		//to be identical, we look for them to be refering to identically placed locations.
		if ((lhsRootObj->GetType() == E_ObjType_Material) || (lhsRootObj->HasType(E_ObjType_Shader)))
		{
			//Build a list of location indices.
			C_PlugLogicalIndexList locationIndices;
			GetLocationIndexList(locationIndices, m_pSourceConnection);

			C_PlugLogicalIndexList rhsLocationIndices;
			GetLocationIndexList(rhsLocationIndices, rhs.m_pSourceConnection);

			if (locationIndices.nIndices != rhsLocationIndices.nIndices)
				return false;

			if (memcmp(&locationIndices, &rhsLocationIndices, sizeof(U32) * rhsLocationIndices.nIndices) != 0)
				return false;
		}
		else
		{
			if (m_pSourceConnection != rhs.m_pSourceConnection)
				return false;
		}
	}

	return true;
}

const C_Node* I_Plug::GetTopLevelParentPtr() const
{
	const I_Plug* pPlug = this;
	//Find top level parent.
	while(!pPlug->GetParentPtr()->HasType(E_ObjType_Node))
	{
		AFTK_ASSERT(pPlug);
		AFTK_ASSERT(pPlug->GetParentPtr()->HasType(E_ObjType_IPlug));
		pPlug = (I_Plug*)pPlug->GetParentPtr();
	}
	//Set top level parent.
	return (C_Node*)pPlug->GetParentPtr();
}

void I_Plug::GetLocationInfo(C_PlugLocationInfo& locationInfo) const
{
	const I_Plug* pPlug = this;
	//Find top level parent.
	locationInfo.nIndices = 0;
	while(!pPlug->m_pParent->HasType(E_ObjType_Node))
	{
		AFTK_ASSERT(pPlug);
		AFTK_ASSERT(pPlug->m_pParent->HasType(E_ObjType_IPlug));
		locationInfo.indices[locationInfo.nIndices] = pPlug->m_logicalIndex;
		locationInfo.nIndices++;
		pPlug = (I_Plug*)pPlug->m_pParent;
	}
	//Set top level parent.
	locationInfo.pTopLevelParent = (C_Node*)pPlug->m_pParent;
	locationInfo.indices[locationInfo.nIndices] = pPlug->m_logicalIndex;
	locationInfo.nIndices++;

	//Reverse index array.
	std::reverse(
		&locationInfo.indices[0], 
		&locationInfo.indices[locationInfo.nIndices]);
}

void I_Plug::Clear()
{
	ClearThis();
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS

void I_Plug::ClearThis()
{
	if (m_pSourceConnection)
	{
		m_pSourceConnection->DeleteConnectedPlugPtr(this);
		m_pSourceConnection = 0;
	}
}

void I_Plug::DisconnectFromObject()
{
	m_pSourceConnection = 0;
}

void I_Plug::SetLogicalIndex(S32 instance)
{
	m_logicalIndex = instance;
}

void I_Plug::SetParentPtr(I_Object* pParent)
{
	AFTK_ASSERT(m_logicalIndex != -1);
	m_pParent = pParent;
}

#endif
