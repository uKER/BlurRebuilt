#include "stdafx.h"

#include <DialogueMan/Misc/MutantValue.h>

namespace DMan
{

bool operator<(const C_MutantValue& lhs, const C_MutantValue& rhs) 
{
	//assume lhs datatype is the same as rhs datatype
	//assert( lhs.GetType() == rhs.GetType() );
	
	E_DataType dataType = lhs.GetType();
	
	switch(dataType)
	{
	case E_DataType_Bool:
		return (bool)lhs < (bool)rhs;
	case E_DataType_UInt:
		return (U32)lhs < (U32)rhs;
	case E_DataType_Int:
		return (int)lhs < (int)rhs;
	case E_DataType_Long:
		return (long)lhs < (long)rhs;
	case E_DataType_Float:
		return (float)lhs < (float)rhs;
	case E_DataType_Double:
		return (double)lhs < (double)rhs;
	case E_DataType_String:
		return (wxString)lhs < (wxString)rhs;
	case E_DataType_Pointer:
		return (void*)lhs < (void*)rhs;
	default:
		return false;
	}
	return false;
}

bool operator<=(const C_MutantValue& lhs, const C_MutantValue& rhs) 
{
	//assume lhs datatype is the same as rhs datatype
	//assert( lhs.GetType() == rhs.GetType() );
	
	E_DataType dataType = lhs.GetType();
	
	switch(dataType)
	{
	case E_DataType_Bool:
		return (bool)lhs <= (bool)rhs;
	case E_DataType_UInt:
		return (U32)lhs <= (U32)rhs;
	case E_DataType_Int:
		return (int)lhs <= (int)rhs;
	case E_DataType_Long:
		return (long)lhs <= (long)rhs;
	case E_DataType_Float:
		return (float)lhs <= (float)rhs;
	case E_DataType_Double:
		return (double)lhs <= (double)rhs;
	case E_DataType_String:
		return (wxString)lhs <= (wxString)rhs;
	case E_DataType_Pointer:
		return (void*)lhs <= (void*)rhs;
	default:
		return false;
	}
	return false;
}

bool operator>(const C_MutantValue& lhs, const C_MutantValue& rhs) 
{
	//assume lhs datatype is the same as rhs datatype
	//assert( lhs.GetType() == rhs.GetType() );
	
	E_DataType dataType = lhs.GetType();
	
	switch(dataType)
	{
	case E_DataType_Bool:
		return (bool)lhs > (bool)rhs;
	case E_DataType_UInt:
		return (U32)lhs > (U32)rhs;
	case E_DataType_Int:
		return (int)lhs > (int)rhs;
	case E_DataType_Long:
		return (long)lhs > (long)rhs;
	case E_DataType_Float:
		return (float)lhs > (float)rhs;
	case E_DataType_Double:
		return (double)lhs > (double)rhs;
	case E_DataType_String:
		return (wxString)lhs > (wxString)rhs;
	case E_DataType_Pointer:
		return (void*)lhs > (void*)rhs;
	default:
		return false;
	}
	return false;
}

bool operator>=(const C_MutantValue& lhs, const C_MutantValue& rhs) 
{
	//assume lhs datatype is the same as rhs datatype
	//assert( lhs.GetType() == rhs.GetType() );
	
	E_DataType dataType = lhs.GetType();
	
	switch(dataType)
	{
	case E_DataType_Bool:
		return (bool)lhs >= (bool)rhs;
	case E_DataType_UInt:
		return (U32)lhs >= (U32)rhs;
	case E_DataType_Int:
		return (int)lhs >= (int)rhs;
	case E_DataType_Long:
		return (long)lhs >= (long)rhs;
	case E_DataType_Float:
		return (float)lhs >= (float)rhs;
	case E_DataType_Double:
		return (double)lhs >= (double)rhs;
	case E_DataType_String:
		return (wxString)lhs >= (wxString)rhs;
	case E_DataType_Pointer:
		return (void*)lhs >= (void*)rhs;
	default:
		return false;
	}
	return false;
}

bool operator==(const C_MutantValue& lhs, const C_MutantValue& rhs) 
{
	//assume lhs datatype is the same as rhs datatype
	//assert( lhs.GetType() == rhs.GetType() );
	
	E_DataType dataType = lhs.GetType();
	
	switch(dataType)
	{
	case E_DataType_Bool:
		return (bool)lhs == (bool)rhs;
	case E_DataType_UInt:
		return (U32)lhs == (U32)rhs;
	case E_DataType_Int:
		return (int)lhs == (int)rhs;
	case E_DataType_Long:
		return (long)lhs == (long)rhs;
	case E_DataType_Float:
		return (float)lhs == (float)rhs;
	case E_DataType_Double:
		return (double)lhs == (double)rhs;
	case E_DataType_String:
		return (wxString)lhs == (wxString)rhs;
	case E_DataType_Pointer:
		return (void*)lhs == (void*)rhs;
	default:
		return false;
	}
	return false;
}

bool operator!=(const C_MutantValue& lhs, const C_MutantValue& rhs) 
{
	//assume lhs datatype is the same as rhs datatype
	//assert( lhs.GetType() == rhs.GetType() );
	
	E_DataType dataType = lhs.GetType();
	
	switch(dataType)
	{
	case E_DataType_Bool:
		return (bool)lhs != (bool)rhs;
	case E_DataType_UInt:
		return (U32)lhs != (U32)rhs;
	case E_DataType_Int:
		return (int)lhs != (int)rhs;
	case E_DataType_Long:
		return (long)lhs != (long)rhs;
	case E_DataType_Float:
		return (float)lhs != (float)rhs;
	case E_DataType_Double:
		return (double)lhs != (double)rhs;
	case E_DataType_String:
		return (wxString)lhs != (wxString)rhs;
	case E_DataType_Pointer:
		return (void*)lhs != (void*)rhs;
	default:
		return false;
	}
	return false;
}

}