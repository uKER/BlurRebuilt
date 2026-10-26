#include <stdafx.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <algorithm>

namespace EditorLib
{

I_Plug::I_Plug(void(*pOnChangedCallback)(I_Plug* pThis))
{
	m_pSourceConnection = 0;
	m_pParent = 0;
	m_pOnChangedCallback = pOnChangedCallback;
}

I_Plug::~I_Plug()
{
	ClearThis();
}

const I_Plug& I_Plug::operator =(const I_Plug& rhs)
{
	//If we're connected to something then disconnect if we're to be connected to something
	//different.
	if (m_pSourceConnection && (m_pSourceConnection != rhs.m_pSourceConnection))
	{
		//If we're not connected to a material DG in any way then copy conection info.
		m_pSourceConnection = rhs.m_pSourceConnection;
	}

	return *this;
}

void I_Plug::SetSourceConnection(I_Plug* pSourceConnection)
{
	//Throw an assert as this is quite an unusual situation. 
	//Are you sure this is what you want to be doing? - having a connection to our parent node.
	m_pSourceConnection = pSourceConnection;
	OnChanged();
}

const I_Plug* I_Plug::GetSourceConnection() const
{
	return m_pSourceConnection;
}

I_Plug* I_Plug::GetSourceConnection()
{
	return m_pSourceConnection;
}

E_DataType I_Plug::GetCompoundType() const
{
	U32 nChildren = GetNumChildren();
	if ((nChildren < 2) || (nChildren > 4))
		return E_DataType_Unknown;

	E_DataType t = GetChild(0)->GetDataType();

	for (U32 i = 1; i < nChildren; i++)
	{
		if (GetChild(i)->GetDataType() != t)
			return E_DataType_Unknown;
	}
	if (GetNumChildren() == 4)
	{
		switch(t)
		{
		case E_DataType_U32:
			return E_DataType_Vector4U32;
		case E_DataType_U64:
			return E_DataType_Vector4U64;
		case E_DataType_S32:
			return E_DataType_Vector4S32;
		case E_DataType_S64:
			return E_DataType_Vector4S64;
		case E_DataType_F32:
			return E_DataType_Vector4F32;
		case E_DataType_F64:
			return E_DataType_Vector4F64;
		default:
			break;
		}
	}
	else if (GetNumChildren() == 3)
	{
		switch(t)
		{
		case E_DataType_U32:
			return E_DataType_Vector3U32;
		case E_DataType_U64:
			return E_DataType_Vector3U64;
		case E_DataType_S32:
			return E_DataType_Vector3S32;
		case E_DataType_S64:
			return E_DataType_Vector3S64;
		case E_DataType_F32:
			return E_DataType_Vector3F32;
		case E_DataType_F64:
			return E_DataType_Vector3F64;
		default:
			break;
		}
	}
	else
	{
		switch(t)
		{
		case E_DataType_U32:
			return E_DataType_Vector2U32;
		case E_DataType_U64:
			return E_DataType_Vector2U64;
		case E_DataType_S32:
			return E_DataType_Vector2S32;
		case E_DataType_S64:
			return E_DataType_Vector2S64;
		case E_DataType_F32:
			return E_DataType_Vector2F32;
		case E_DataType_F64:
			return E_DataType_Vector2F64;
		default:
			break;
		}
	}

	return E_DataType_Unknown;
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

	if (m_pParent->IsA(E_ObjType_IPlug))
	{
		I_Plug* pPlug = (I_Plug*)m_pParent;
		for (U32 i = 0; i < pPlug->GetNumChildren(); i++)
		{
			if (pPlug->GetChild(i) == this)
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

Util::C_String I_Plug::GetPath() const
{
	Util::C_String finalString;
	if (m_pParent)
	{
		m_pParent->GetPath(finalString);
		finalString = finalString + ".";
	}
	finalString += GetName();
	return finalString;
}

I_DgObject* I_Plug::GetParent()
{
	return m_pParent;
}

const I_DgObject* I_Plug::GetParent() const
{
	return m_pParent;
}

bool I_Plug::IsA(E_ObjType type) const
{
	if (I_DgObject::IsA(type))
		return true;
	if (type == E_ObjType_IPlug) 
		return true;

	return false;
}

const I_DgObject* I_Plug::GetTopLevelParent() const
{
	const I_Plug* pPlug = this;
	//Find top level parent.
	while(pPlug->GetParent()->IsA(E_ObjType_IPlug))
	{
		pPlug = (I_Plug*)pPlug->GetParent();
	}
	//Set top level parent.
	return pPlug->GetParent();
}

void I_Plug::Clear()
{
	ClearThis();
}

void I_Plug::ClearThis()
{
	m_pSourceConnection = 0;
}

void I_Plug::Disconnect()
{
	m_pSourceConnection = 0;
}

void I_Plug::SetParent(I_Plug* pParent)
{
	m_pParent = pParent;
}

}
