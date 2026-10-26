#ifndef DMAN_MUTANTVALUE_H
#define DMAN_MUTANTVALUE_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DMan
{

class C_MutantValue
{
public:

	C_MutantValue() : m_valueType(E_DataType_String) 
	{
		m_stringVal = wxT("");
	}

	C_MutantValue(bool val) : m_valueType(E_DataType_Bool) 
	{
		m_boolVal = val;
	}

	C_MutantValue(U32 val) : m_valueType(E_DataType_UInt) 
	{
		m_uintVal = val;
	}

	C_MutantValue(int val) : m_valueType(E_DataType_Int) 
	{
		m_intVal = val;
	}

	C_MutantValue(long val) : m_valueType(E_DataType_Long) 
	{
		m_longVal = val;
	}

	C_MutantValue(float val) : m_valueType(E_DataType_Float) 
	{
		m_floatVal = val;
	}

	C_MutantValue(double val) : m_valueType(E_DataType_Double) 
	{
		m_doubleVal = val;
	}
	
	C_MutantValue(const wxString& val) : m_valueType(E_DataType_String) 
	{
		m_stringVal = val;
	}

	C_MutantValue(void* pointer) : m_valueType(E_DataType_Pointer)
	{
		m_pointer = pointer;
	}

	C_MutantValue(E_DataType type, const C_MutantValue& src) : m_valueType(type)
	{
		switch(type)
		{
		case E_DataType_Bool:
			m_boolVal = bool(src);
			break;
		case E_DataType_UInt:
			m_uintVal = U32(src);
			break;
		case E_DataType_Int:
			m_intVal = int(src);
			break;
		case E_DataType_Long:
			m_longVal = long(src);
			break;
		case E_DataType_Float:
			m_floatVal = float(src);
			break;
		case E_DataType_Double:
			m_doubleVal = double(src);
			break;
		case E_DataType_String:
			m_stringVal = wxString(src);
			break;
		case E_DataType_Pointer:
			m_pointer = (void*)src;
			break;
		}
	}

	E_DataType GetType() const
	{
		return m_valueType;
	}

	operator bool() const
	{
		switch(m_valueType)
		{
		case E_DataType_Bool:
			return m_boolVal;
		case E_DataType_UInt:
			return (m_uintVal != 0);
		case E_DataType_Int:
			return (m_intVal != 0);
		case E_DataType_Long:
			return (m_longVal != 0L);
		case E_DataType_Float:
			return (m_floatVal != 0.0f);
		case E_DataType_Double:
			return (m_doubleVal != 0.0);
		case E_DataType_String:
		{
#ifdef UNICODE
			std::wistringstream from(m_stringVal.GetData());
#else
			std::stringstream from(m_stringVal.GetData());
#endif
			bool to;
			if(!(from >> to))
			{
				if(m_stringVal.Len() > 0)
					return true;
				else
					return false;
			}
			return to;
		}
		case E_DataType_Pointer:
			return m_pointer != NULL;
		default:
			return false;
		}
	}

	operator U32() const
	{
		switch(m_valueType)
		{
		case E_DataType_Bool:
			return (U32)m_boolVal;
		case E_DataType_UInt:
			return m_uintVal;
		case E_DataType_Int:
			return (U32)m_intVal;
		case E_DataType_Long:
			return (U32)m_longVal;
		case E_DataType_Float:
			return (U32)m_floatVal;
		case E_DataType_Double:
			return (U32)m_doubleVal;
		case E_DataType_String:
		{
#ifdef UNICODE
			std::wistringstream from(m_stringVal.GetData());
#else
			std::stringstream from(m_stringVal.GetData());
#endif
			U32 to;
			if(!(from >> to))
			{
				return 0;
			}
			return to;
		}
		case E_DataType_Pointer:
			return (U32)m_pointer;
		default:
			return 0;
		}
	}

	operator int() const
	{
		switch(m_valueType)
		{
		case E_DataType_Bool:
			return (int)m_boolVal;
		case E_DataType_UInt:
			return (int)m_uintVal;
		case E_DataType_Int:
			return m_intVal;
		case E_DataType_Long:
			return (int)m_longVal;
		case E_DataType_Float:
			return (int)m_floatVal;
		case E_DataType_Double:
			return (int)m_doubleVal;
		case E_DataType_String:
		{
#ifdef UNICODE
			std::wistringstream from(m_stringVal.GetData());
#else
			std::stringstream from(m_stringVal.GetData());
#endif
			int to;
			if(!(from >> to))
			{
				return 0;
			}
			return to;
		}
		default:
			return 0;
		}
	}

	operator long() const
	{
		switch(m_valueType)
		{
		case E_DataType_Bool:
			return (long)m_boolVal;
		case E_DataType_UInt:
			return (long)m_uintVal;
		case E_DataType_Int:
			return (long)m_intVal;
		case E_DataType_Long:
			return m_longVal;
		case E_DataType_Float:
			return (long)m_floatVal;
		case E_DataType_Double:
			return (long)m_doubleVal;
		case E_DataType_String:
		{
#ifdef UNICODE
			std::wistringstream from(m_stringVal.GetData());
#else
			std::stringstream from(m_stringVal.GetData());
#endif
			long to;
			if(!(from >> to))
			{
				return -1L;
			}
			return to;
		}
		case E_DataType_Pointer:
			return (long)m_pointer;
		default:
			return -1L;
		}
	}

	operator float() const
	{
		switch(m_valueType)
		{
		case E_DataType_Bool:
			return (float)m_boolVal;
		case E_DataType_UInt:
			return (float)m_uintVal;
		case E_DataType_Int:
			return (float)m_intVal;
		case E_DataType_Long:
			return (float)m_longVal;
		case E_DataType_Float:
			return m_floatVal;
		case E_DataType_Double:
			return (float)m_doubleVal;
		case E_DataType_String:
		{
#ifdef UNICODE
			std::wistringstream from(m_stringVal.GetData());
#else
			std::stringstream from(m_stringVal.GetData());
#endif
			float to;
			if(!(from >> to))
			{
				return 0.0f;
			}
			return to;
		}
		default:
			return 0.0f;
		}
	}

	operator double() const
	{
		switch(m_valueType)
		{
		case E_DataType_Bool:
			return (double)m_boolVal;
		case E_DataType_UInt:
			return (double)m_uintVal;
		case E_DataType_Int:
			return (double)m_intVal;
		case E_DataType_Long:
			return (double)m_longVal;
		case E_DataType_Float:
			return (double)m_floatVal;
		case E_DataType_Double:
			return m_doubleVal;
		case E_DataType_String:
		{
#ifdef UNICODE
			std::wistringstream from(m_stringVal.GetData());
#else
			std::stringstream from(m_stringVal.GetData());
#endif
			double to;
			if(!(from >> to))
			{
				return 0.0;
			}
			return to;
		}
		default:
			return 0.0;
		}
	}

	operator wxString() const
	{
#ifdef UNICODE
		std::wstringstream conv;
#else
		std::stringstream conv;
#endif
		
		switch(m_valueType)
		{
		case E_DataType_Bool:
			conv << m_boolVal;
			break;
		case E_DataType_UInt:
			conv << m_uintVal;
			break;
		case E_DataType_Int:
			conv << m_intVal;
			break;
		case E_DataType_Long:
			conv << m_longVal;
			break;
		case E_DataType_Float:
			conv << m_floatVal;
			break;
		case E_DataType_Double:
			conv << m_doubleVal;
			break;
		case E_DataType_String:
			return m_stringVal;
			break;
		case E_DataType_Pointer:
			conv << (U32)m_pointer;
		default:
			break;
		}

		return wxString(conv.str());
	}

	operator void*() const
	{
		if(m_valueType == E_DataType_Pointer)
			return m_pointer;
		else
			return NULL;
	}

	friend bool operator<(const C_MutantValue& lhs, const C_MutantValue& rhs);
	friend bool operator<=(const C_MutantValue& lhs, const C_MutantValue& rhs);
	friend bool operator>(const C_MutantValue& lhs, const C_MutantValue& rhs);
	friend bool operator>=(const C_MutantValue& lhs, const C_MutantValue& rhs);
	friend bool operator==(const C_MutantValue& lhs, const C_MutantValue& rhs);
	friend bool operator!=(const C_MutantValue& lhs, const C_MutantValue& rhs);

private:
	enum E_DataType m_valueType;
	
	bool		m_boolVal;
	U32			m_uintVal;
	int			m_intVal;
	long		m_longVal;
	float		m_floatVal;
	double		m_doubleVal;
	wxString 	m_stringVal;
	void*		m_pointer;
};

} //namespace DMan

#endif //DMAN_MUTANTVALUE_H