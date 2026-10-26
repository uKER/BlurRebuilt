#include "stdafx.h"
#include <file_tag_db/GenericAttribute.h>

using namespace FileTag;

C_GenericAttribute::C_GenericAttribute() 
{
	_OneTimeInit();
}

C_GenericAttribute::~C_GenericAttribute()
{
	Clear();
}

C_GenericAttribute::C_GenericAttribute(const C_GenericAttribute &rhs) 
{
	_OneTimeInit();
	*this = rhs;
}

void C_GenericAttribute::SetValue(const C_GenericAttribute& rhs)
{
	switch (rhs.m_type) 
	{
	case E_DataType_S32: {
		S32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_DataType_S64: {
		S64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_DataType_F32: {
		F32 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}
	case E_DataType_F64: {
		F64 value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}

	case E_DataType_Boolean: {
		bool value;
		rhs.GetValue(value);
		SetValue(value);
		break;
	}

	case E_DataType_String:
	{
		SetValue(*(Util::C_String*)rhs.m_pValue, E_DataType_String);
		break;
	}

	case E_DataType_StringEnum:
	{
		SetValue(*(Util::C_String*)rhs.m_pValue, E_DataType_StringEnum);
		break;
	}

	case E_DataType_StringFilename:
	{
		SetValue(*(Util::C_String*)rhs.m_pValue, E_DataType_StringFilename);
		break;
	}

	default:
		m_pValue = 0;
		m_type = E_DataType_Null;
		break;
	}

	m_type = rhs.m_type;
}

const C_GenericAttribute& C_GenericAttribute::operator =(const C_GenericAttribute &rhs)
{
	Clear();

	m_name = rhs.m_name;

	SetValue(rhs);

	return *this;
}

bool C_GenericAttribute::operator==(const C_GenericAttribute &rhs) const
{
	if (m_name == rhs.m_name &&
		m_type == rhs.m_type) 
	{
		switch (m_type) 
		{
			case E_DataType_S32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(S32)) != 0)
					return false;
				break;
			}
			case E_DataType_S64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(S64)) != 0)
					return false;
				break;
			}
			case E_DataType_F32: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(F32)) != 0)
					return false;
				break;
			}
			case E_DataType_F64: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(F64)) != 0)
					return false;
				break;
			}
			case E_DataType_Boolean: {
				if (memcmp(m_pValue, rhs.m_pValue, sizeof(bool)) != 0)
					return false;
				break;
			}
			case E_DataType_String: 
			case E_DataType_StringEnum:
			case E_DataType_StringFilename: 
			{
				Util::C_String* pLhs = (Util::C_String*)m_pValue;
				Util::C_String* pRhs = (Util::C_String*)rhs.m_pValue;
				if (*pLhs != *pRhs) {
					return false;
				}
				break;
			}
			default:
				break;
		}
	} else {
		return false;
	}
	
	return true;
}

bool C_GenericAttribute::operator!= (const C_GenericAttribute &rhs) const 
{
	if ( *this == rhs ) {
		return false;
	} else {
		return true;
	}
}

bool C_GenericAttribute::operator<(const C_GenericAttribute &rhs) const 
{
	if (memcmp((const void*)this, (const void*) &rhs, sizeof(C_GenericAttribute)) < 0)
	{
		return true;
	} else {
		return false;
	}
}

void C_GenericAttribute::SetValue(S32 value)
{
	if (m_type == E_DataType_S32)
	{
		(*(S32*)m_pValue) = value;
		return;
	}
	Clear();
	m_pValue = new S32(value);
	m_type = E_DataType_S32;
}

void C_GenericAttribute::SetValue(S64 value)
{
	if (m_type == E_DataType_S64)
	{
		(*(S64*)m_pValue) = value;
		return;
	}
	Clear();
	m_pValue = new S64(value);
	m_type = E_DataType_S64;
}

void C_GenericAttribute::SetValue(F32 value) 
{
	if (m_type == E_DataType_F32)
	{
		(*(F32*)m_pValue) = value;
		return;
	}
	Clear();
	m_pValue = new F32(value);
	m_type = E_DataType_F32;
}

void C_GenericAttribute::SetValue(F64 value)
{
	if (m_type == E_DataType_F64)
	{
		(*(F64*)m_pValue) = value;
		return;
	}
	Clear();
	m_pValue = new F64(value);
	m_type = E_DataType_F64;
}

void C_GenericAttribute::SetValue(bool value) 
{
	if (m_type == E_DataType_Boolean)
	{
		(*(bool*)m_pValue) = value;
		return;
	}
	Clear();
	m_pValue = new bool(value);
	m_type = E_DataType_Boolean;
}

void C_GenericAttribute::SetValue(const Util::C_String& value, E_DataType stringDataType) 
{
	if (!(
		stringDataType == E_DataType_String || 
		stringDataType == E_DataType_StringEnum ||
		stringDataType == E_DataType_StringFilename))
	{
		assert(!"Invalid string type provided!");
		return;
	}

	if (m_type == E_DataType_String || 
		m_type == E_DataType_StringEnum || 
		m_type == E_DataType_StringFilename)
	{
		(*(Util::C_String*)m_pValue) = value;
		return;
	}
	Clear();
	m_pValue = new Util::C_String(value);

	m_type = stringDataType;
}

const Util::C_String& C_GenericAttribute::GetName() const 
{
	return m_name;
}

Util::C_String& C_GenericAttribute::GetName() 
{
	return m_name;
}

C_GenericAttribute::E_DataType C_GenericAttribute::GetDataType() const 
{
	return m_type;
}

void C_GenericAttribute::Clear() 
{
	if (m_pValue) 
	{
		switch (m_type)
		{
			case E_DataType_S64:
			{
				delete (S64*)m_pValue;
				break;
			}

			case E_DataType_S32:
			{
				delete (S32*)m_pValue;
				break;
			}

			case E_DataType_F32:
			{
				delete (F32*)m_pValue;
				break;
			}

			case E_DataType_F64:
			{
				delete (F64*)m_pValue;
				break;
			}

			case E_DataType_Boolean:
			{
				delete (bool*)m_pValue;
				break;
			}

			case E_DataType_String:
			case E_DataType_StringEnum:
			case E_DataType_StringFilename:
			{
				delete (Util::C_String*)m_pValue;
				break;
			}
				
			case E_DataType_Null:
			default:
				break;
		}
	}

	m_pValue = 0;
	m_type = E_DataType_Null;
}

void C_GenericAttribute::GetValue(S32& value) const
{
	value = *(S32*)m_pValue;
}

void C_GenericAttribute::GetValue(S64& value) const
{
	value = *(S64*)m_pValue;
}

void C_GenericAttribute::GetValue(F32& value) const
{
	value = *(F32*)m_pValue;
}

void C_GenericAttribute::GetValue(F64& value) const
{
	value = *(F64*)m_pValue;
}

void C_GenericAttribute::GetValue(bool& value) const
{
	value = *(bool*)m_pValue;
}

void C_GenericAttribute::GetValue(Util::C_String& value) const
{
	value = *(Util::C_String*)m_pValue;
}

void C_GenericAttribute::_OneTimeInit()
{
	m_pValue = 0;
	m_type = E_DataType_Null;
}

void C_GenericAttribute::_SetValue(void* pValue, E_DataType type)
{
	m_pValue = pValue;
	m_type = type;
}

